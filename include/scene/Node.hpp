#pragma once

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <Box2D/Box2D.h>

#include "physics/CollisionData.hpp"
#include "exception/EngineException.hpp"
#include "collection/mutable/KBVector.hpp"
#include "collection/mutable/KBMap.hpp"
#include "collection/mutable/KBTypeMap.hpp"
#include "collection/Option.hpp"
#include "scene/Transform.hpp"
#include "scene/NodeAttachment.hpp"
#include "scene/BoundingBox.hpp"
#include "scene/attachment/PhysicsAttachment.hpp"
#include "minebombers/attachments/CollisionMaterialAttachment.hpp"
#include "message/EventHandler.hpp"
#include "service/Services.hpp"
#include <typeinfo>
#include <iostream>
#include <string>
#include <memory>
#include "util/MatrixUtil.hpp"
#include "util/StringUtil.hpp"
#include "glm/gtx/string_cast.hpp"
#include "game/Behavior.hpp"

/**
 * Node interface.
 */
template <typename T>
class Node : public EventHandler, public std::enable_shared_from_this<Node<T>> {
public:
  Node(std::string name) : _name(name), _render(true) {}
  ~Node() {}

  auto isRenderOn() const -> bool { return _render; }
  auto setRenderOn(bool b) -> void { _render = b; }

  auto name() const -> std::string {
    return _name;
  }

  auto parent() const -> const Option<Node> {
    return _parent;
  }

  auto children() const -> const KBMap<std::string, std::shared_ptr<Node>>& {
    return _children;
  }

  auto removeChild(std::string name) const -> const void {
    _children.remove(name);
  }

  auto getNode(std::string path) const -> const Option<std::shared_ptr<Node>> {
    auto i = path.find('/');
    if (i != std::string::npos){
      auto first_token = path.substr(0, i);
      auto rest = path.substr(i+1, path.length());
      auto c = children().get(first_token);
      if(c.isDefined()){
        return c.get()->getNode(rest);
      } else {
        return Option<std::shared_ptr<Node>>();
      }
    } else{
      return _children.get(path);
    }
  }

  auto addChild(std::shared_ptr<Node> child) -> void {
    this->wakeUp();
    _children.insert(child->name(), child);
    child->_setParent(this->shared_from_this());

    auto materialAttachment = child->template getShared<CollisionMaterialAttachment>();
    auto physicsAttachment = child->template get<PhysicsAttachment>();

    if(physicsAttachment.isDefined() && materialAttachment.isDefined()){
      auto collisionData = new CollisionData(child->path(), materialAttachment.get());
      physicsAttachment.get().body()->SetUserData(collisionData);
    }

    _addChildBB(child->boundingBox(), child->localPosition());

    child->_setUpdateFlag();
    if (!child->getAllowSleep()) {
      this->setAllowSleep(false);
    }
  }

  auto addAttachment(std::shared_ptr<NodeAttachment> attachment) -> void {
    _attachments[typeid(*attachment)] = attachment;
  }

  template <typename AttachmentType>
  auto get() const -> Option<AttachmentType> {
     auto attachment =  _attachments.get<AttachmentType>();
    if (attachment.isDefined()) {
      return Some<AttachmentType>(std::dynamic_pointer_cast<AttachmentType>(attachment.get()));
    } else {
      return Option<AttachmentType>();
    }
  }

  template <typename AttachmentType>
  auto getShared() const -> Option<std::shared_ptr<AttachmentType>> {
    auto attachment = _attachments.get<AttachmentType>();
    if (attachment.isDefined()) {
      return Some<std::shared_ptr<AttachmentType>>(std::dynamic_pointer_cast< AttachmentType >( attachment.get() ));
    } else {
      return Option<std::shared_ptr<AttachmentType>>();
    }
  }

  auto transform() const -> typename T::matrixType {
    return _transform.matrix();
  }

  auto worldTransform() const -> typename T::matrixType {
    if (_shouldUpdate) {
      _update();
    }
    return _worldTransform;
  }

  auto position() const -> typename T::vectorType {
    return MatrixUtil::getTransform(worldTransform());
  }

  auto localPosition() const -> const typename T::vectorType {
    return _transform.position();
  }

  auto setLocalPosition(typename T::vectorType v) -> void {
    _transform.setPosition(v);
    _setUpdateFlag();
    if (_parent.isDefined()) {
      _parent.get()._addChildBB(boundingBox(), localPosition());
    }
  }

  auto localScale() const -> const typename T::vectorType {
    return _transform.scale();
  }

  auto setLocalScale(typename T::vectorType v) -> void {
    _transform.setScale(v);
    _setUpdateFlag();
  }

  auto localRotation() const -> const typename T::rotationType {
    return _transform.rotation();
  }

  auto setLocalRotation(typename T::rotationType r) -> void {
    _transform.setRotation(r);
    _setUpdateFlag();
  }

  auto rotation() const -> typename T::rotationType {
    return _parent.map([&](const auto& parentNode) {
      return parentNode.rotation() * this->localRotation();
    }).getOrElse(localRotation());
  }

  auto scale() const -> typename T::vectorType {
    return _parent.map([&](const auto& parentNode) {
      return parentNode.scale() * localScale();
    }).getOrElse(localScale());
  }

  auto path() const -> std::string {
    return _parent.map([&](const auto& parentNode) {
      return parentNode.path() + "/" + this->name();
    }).getOrElse(this->name());
  }

  auto operator==(Node<T> other) -> bool {
    return true;
  }

  auto setSleep(bool val) -> void {
    if (_allowSleep) {
      _isSleeping = true;
    }
  }

  auto getAllowSleep() -> bool {
    return _allowSleep;
  }

  auto setAllowSleep(bool isAllowed) -> void {
    _allowSleep = isAllowed;
    if (!isAllowed && _parent.isDefined()) {
      _parent.get().setAllowSleep(isAllowed);
    }
  }

  auto isSleeping() const -> bool {
    return _isSleeping;
  }

  auto wakeUp() -> void {
    _isSleeping = false;
    if (_parent.isDefined()) {
      _parent.get().wakeUp();
    }
   }

  template <typename BehaviorType, typename... Args>
  auto addBehavior(Args&&... args) -> std::shared_ptr<BehaviorType> {
    auto behavior = std::make_shared<BehaviorType>(this, std::forward<Args>(args)...);
    _behaviors += behavior;
    return behavior;
  }

  auto handleEvent(std::shared_ptr<Event> event) -> void override {
    this->wakeUp();
    if (reactors.contains(typeid(*event))) {
      reactors[typeid(*event)](event);
    }
  }

  auto update(float delta) -> void {
    if (!_isSleeping) {
      auto allSleeping = true;
      _children.values().foreach([&](auto child) {
        child->update(delta);
        if (!child->isSleeping()) {
          allSleeping = false;
        }
      });

      const auto& physAttachment = this->get<PhysicsAttachment>();
      physAttachment.foreach([&](auto phys) {
        auto pos = phys.position();
        if (_parent.isDefined()) {
          const auto& parentPos = _parent.get().position();
          pos.x -= parentPos.x;
          pos.y -= parentPos.y;
        }
        this->setLocalPosition(glm::vec3(pos.x, pos.y, this->localPosition().z));
      });
      _behaviors.foreach([&](auto& behavior) {
        behavior->update(delta, *this);
      });
      setSleep(allSleeping);
    }
  }

  auto getRoot() -> std::shared_ptr<Node> {
    if (_parent.isDefined()) {
      return _parent.get().getRoot();
    } else {
      return this->shared_from_this();
    }
  }

  auto toBeDestroyed() -> bool {
    return _toBeDestroyed;
  }
  auto markToBeDestroyed() -> void {
    _toBeDestroyed = true;
  }

  auto setBoundingBox(BoundingBox box) -> void {
    _localBoundingBox = box;
    _boundingBox.setMax(box);
    if (_parent.isDefined()) {
      _parent.get()._addChildBB(box, localPosition());
    }
  }

  auto boundingBox() -> BoundingBox {
    if (_shouldUpdate) {
      _update();
    }
    return _boundingBox;
  }

  auto localBoundingBox() -> BoundingBox {
    return _localBoundingBox;
  }

protected:
  mutable bool _shouldUpdate = true;

  auto _update() const -> void {
    _worldTransform = _parent.map([&](const auto& parentNode) {
      return parentNode.worldTransform() * _transform.matrix();
    }).getOrElse(_transform.matrix());
    _setUpdateFlag();
    _shouldUpdate = false;
  }

  auto _addChildBB(BoundingBox box, glm::vec3 localPosition) -> void {
    auto bb = box;
    auto pos = localPosition;

    auto right = pos.x + bb.right();
    auto left = pos.x + bb.left();
    auto bottom = pos.y + bb.bottom();
    auto top = pos.y + bb.top();
    auto near = pos.y + bb.near();
    auto far = pos.y + bb.far();

    _boundingBox.setMaxRight(right);
    _boundingBox.setMaxLeft(left);
    _boundingBox.setMaxTop(top);
    _boundingBox.setMaxBottom(bottom);
    _boundingBox.setMaxNear(near);
    _boundingBox.setMaxFar(far);

    if (_parent.isDefined()) {
      _parent.get()._addChildBB(_boundingBox, this->localPosition());
    }
  }

  auto _setUpdateFlag() const -> void {
    if (!_shouldUpdate) {
      _shouldUpdate = true;
      _children.values().foreach([](auto child) {
        child->_setUpdateFlag();
      });
    }
  }

  auto _setParent(std::shared_ptr<Node> parent) -> void {
    _parent = Some(parent);
    this->_setUpdateFlag();
  }

private:
  std::string _name;
  mutable KBMap<std::string, std::shared_ptr<Node>> _children;
  bool _render;
  bool _toBeDestroyed = false;
  bool _isSleeping = false;
  bool _allowSleep = true;
  Option<Node> _parent;
  KBTypeMap<std::shared_ptr<NodeAttachment>> _attachments;
  T _transform;
  BoundingBox _localBoundingBox;
  mutable BoundingBox _boundingBox;
  KBVector<std::shared_ptr<Behavior<T>>> _behaviors;
  mutable typename T::matrixType _worldTransform;
};

template <typename T>
auto operator<<(std::ostream& os, const Node<T>& node) -> std::ostream& {
  os << "Node: " <<
    "  name: " << node.name() << std::endl <<
    "  position: " << glm::to_string(node.position()) << std::endl <<
    "  rotation: " << glm::to_string(node.rotation()) << std::endl <<
    "  scale: " << glm::to_string(node.scale());

  return os;

}
