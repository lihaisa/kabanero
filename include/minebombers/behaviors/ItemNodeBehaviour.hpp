#pragma once

#include "game/Behavior.hpp"
#include "scene/Node.hpp"
#include "scene/3D/Transform3D.hpp"
#include "service/Services.hpp"
#include "message/event/CollisionEvent.hpp"
#include "message/event/DestroyNodeEvent.hpp"
#include "minebombers/attachments/GunAttachment.hpp"

#include <glm/vec2.hpp>

#include <iostream>
#include <memory>

class ItemNodeBehaviour : public Behavior<Transform3D> {
public:
  ItemNodeBehaviour(Node<Transform3D>* node, std::shared_ptr<GunAttachment> gun) :_gun(gun) {
  }

  auto update(float delta, Node<Transform3D>& node) -> void override {
  }

  auto getGun() -> std::shared_ptr<GunAttachment> {
    return _gun;
  }
private:
  std::shared_ptr<GunAttachment> _gun;
};
