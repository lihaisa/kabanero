#pragma once

#include <iostream>
// #include "message/event/DestroyNodeEvent.hpp"
#include "minebombers/level/TileMap.hpp"
#include "scene/3D/Transform3D.hpp"
#include "game/Behavior.hpp"
#include "service/Services.hpp"
#include "message/event/DestroyNodeEvent.hpp"
#include "message/event/CollisionEvent.hpp"
#include "game/Behavior.hpp"
#include "scene/Node.hpp"
#include "scene/3D/Transform3D.hpp"
#include "service/Services.hpp"
#include "random/StdLibRandom.hpp"
#include "util/StringUtil.hpp"
#include "minebombers/attachments/GunAttachment.hpp"

class TerrainBehavior : public Behavior<Transform3D> {
public:
  TerrainBehavior(Node<Transform3D>* node, float health, std::shared_ptr<TileMap> map) :
   _health(health),
   _map(map) {

    node->addEventReactor([&, node](CollisionEvent event) {
      const auto& pos = node->position();
      if (event.collisionMaterialAttachment()->collisionDamage > 0.0f) {
        const auto damage = event.collisionMaterialAttachment()->collisionDamage;
        _dmgToTake += damage;
        node->wakeUp();
      }
    });
  }

  auto update(float delta, Node<Transform3D>& node) -> void override {
    auto parentName = node.parent().get().name();
    parentName.erase(0, 4);
    auto coords = split(parentName, '-');
    if (_dmgToTake != 0) {
      _health -= _dmgToTake;
      _dmgToTake = 0;
      if (_health <= 0) {
        Services::messagePublisher()->sendMessage(
          Message(
            "game",
            std::make_shared<DestroyNodeEvent>(node.path())
          )
        );
        const auto& pos = node.position();

        (*_map)[std::atoi(coords[0].c_str())][std::atoi(coords[1].c_str())].setType(OPEN_SIDE);
      }
    }
    node.setSleep(true);
  }

private:
  float _health, _dmgToTake;
  std::shared_ptr<TileMap> _map;
};
