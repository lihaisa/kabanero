#pragma once

#include "minebombers/events/PlayerLocationEvent.hpp"
#include "minebombers/events/BulletEvent.hpp"
#include "minebombers/events/UpdateHudEvent.hpp"
#include "minebombers/events/RespawnEvent.hpp"
#include "minebombers/behaviors/BombBehaviour.hpp"
#include "scene/attachment/SpriteAttachment.hpp"
#include "message/event/CreateNodeEvent.hpp"
#include "message/event/GameInputEvent.hpp"
#include "message/event/AudioClipEvent.hpp"
#include "game/Behavior.hpp"
#include "scene/Node.hpp"
#include "scene/3D/Transform3D.hpp"
#include "service/Services.hpp"
#include <Box2D/Box2D.h>
#include "random/StdLibRandom.hpp"
#include "minebombers/data/GunParameters.hpp"
#include "minebombers/util/NodeFactory.hpp"
#include "minebombers/hud/HudEffect.hpp"
// #include "minebombers/data/HudParameters.hpp"

#include <glm/vec2.hpp>
#include <glm/gtx/rotate_vector.hpp>

class PlayerBehaviour : public Behavior<Transform3D> {
public:
  PlayerBehaviour(Node<Transform3D>* node, const std::string& cameraAddress, int playerId = 1, int lives = 3) :
      _cameraAddress(cameraAddress), _playerId(playerId), _lives(lives) {
    node->addEventReactor([&](GameInputEvent event) {
      auto action = event.action();
      auto isPressed = event.isPressed();
      if (hp > 0.0f){
        if (action == UP) {
          moveUp = isPressed;
        } else if (action == RIGHT) {
          moveRight = isPressed;
        } else if (action == DOWN) {
          moveDown = isPressed;
        } else if (action == LEFT) {
          moveLeft = isPressed;
        } else if (action == FIRE_RIGHT) {
          fireRight = isPressed;
        } else if (action == FIRE_DOWN) {
          fireDown = isPressed;
        } else if (action == FIRE_LEFT) {
          _isStrafing = isPressed;
          fireLeft = isPressed;
        } else if (action == FIRE_UP) {
          isFiring = isPressed;
          fireUp = isPressed;
        } else if (action == FIRE) {
          throwBomb = isPressed;
        }
      } else if (action == FIRE) {
        respawn = isPressed;
      }
    });
    node->addEventReactor([&](CollisionEvent event) {
      if (event.collisionMaterialAttachment()->collisionDamage > 0.0f) {
        damageToTake = event.collisionMaterialAttachment()->collisionDamage;
        _collisionVec = event.vec();
      }
      else if (event.collisionMaterialAttachment()->gunParameters.isDefined() && _gunPickupDelay <= 0.0f) {
        Services::logger()->debug("Player got new gun");
        _newGun = event.collisionMaterialAttachment()->gunParameters;
      }
    });
  }

  const float PI = 3.1415927;

  auto update(float delta, Node<Transform3D>& node) -> void override {
    auto pos = node.position();
    if (_gunPickupDelay > 0.0f) {
      _gunPickupDelay -= delta;
    }
    if (_newGun.isDefined()) {
      std::shared_ptr<Node<Transform3D>> crateNode;
      std::shared_ptr<b2BodyDef> bodyDef;
      std::shared_ptr<b2FixtureDef> fixtureDef;

      _gunPickupDelay = 2.0f;

      std::tie(crateNode, bodyDef, fixtureDef) = NodeFactory::createGunCrate(
        std::make_shared<GunParameters>(
          *node.get<GunAttachment>().get().parameters()
        ), round(pos.x), round(pos.y)
      );

      Services::messagePublisher()->sendMessage(
        Message("gameScene",
          std::make_shared<CreateNodeEvent>("world",
            crateNode, bodyDef, fixtureDef
          )
        )
      );

      node.addAttachment(std::make_shared<GunAttachment>(_newGun.get()));
      // _gunName = _newGun.get()->gunName;
      _newGun = Option<std::shared_ptr<GunParameters>>();

      auto gunParams = node.get<GunAttachment>().get().parameters();
      auto gunName = gunParams->gunName;

      if (gunName.compare("Shotgun") == 0 ||
          gunName.compare("Rocket Launcher") == 0 ||
          gunName.compare("Super Shotgun") == 0) {

        auto spriteAttachment = node.get<SpriteAttachment>();
        spriteAttachment.get().setSpriteId("player/survivor-idle_shotgun");

      } else if (
          gunName.compare("Rifle") == 0 ||
          gunName.compare("Sniper") == 0 ||
          gunName.compare("Super Rifle") == 0 ||
          gunName.compare("Super Sniper") == 0) {

        auto spriteAttachment = node.get<SpriteAttachment>();
        spriteAttachment.get().setSpriteId("player/survivor-idle_rifle");
      } else {
        auto spriteAttachment = node.get<SpriteAttachment>();
        spriteAttachment.get().setSpriteId("player/survivor-idle_handgun");
      }

      Services::messagePublisher()->sendMessage(Message(
        "audioPlayer:clip/reload.ogg",
        std::make_shared<AudioClipEvent>(CLIP_PLAY)
      ));
    }
    auto moveDirection = glm::vec2(0, 0);
    if (moveUp) moveDirection.y += 1;
    if (moveDown) moveDirection.y -= 1;
    if (moveRight) moveDirection.x += 1;
    if (moveLeft) moveDirection.x -= 1;
    if (moveDirection.x != 0 && moveDirection.y != 0) {
      moveDirection = glm::normalize(moveDirection);
    }

    if (!(moveDirection.x == 0 && moveDirection.y == 0)) {
      if (!_isStrafing) {
        auto rotationAngle = std::atan2(moveDirection.y, moveDirection.x);
        node.setLocalRotation(glm::angleAxis(rotationAngle, glm::vec3(0, 0, -1)));
        _lookDirection = glm::vec2(moveDirection);
      }
    }

    auto gunParams = node.get<GunAttachment>().get().parameters();

    auto moveSpeed = _playerSpeed * gunParams->walkSpeed;
    if (_isStrafing) {
      moveSpeed *= 0.75f;
    }

    const auto& physAttachment = node.get<PhysicsAttachment>();
    physAttachment.foreach([&](auto phys) {
      const auto& pos = phys.position();
      phys.setVelocity(
        moveDirection.x * moveSpeed,
        moveDirection.y * moveSpeed
      );
    });

    auto playerPos = pos.xy();

    Services::messagePublisher()->sendMessage(Message(
      "gameScene:" + _cameraAddress,
      std::make_shared<PlayerLocationEvent>(playerPos)
    ));

    glm::vec2 fireDirection = _lookDirection;
    _fireDelay -= delta;

    // if (fireUp) fireDirection.y += 1;
    // if (fireDown) fireDirection.y -= 1;
    // if (fireRight) fireDirection.x += 1;
    // if (fireLeft) fireDirection.x -= 1;
    // if (fireDirection.x != 0 && fireDirection.y != 0) {
    //   fireDirection = glm::normalize(fireDirection);
    // }

    if (isFiring && hp > 0.0f && _fireDelay <= 0) {
      // auto gun_att = node.get<GunAttachment>().get();
      auto gunParams = node.get<GunAttachment>().get().parameters();
      // auto gun = gun_att.parameters().get();

      _fireDelay = 1.0f / gunParams->fireRate;
      auto random = Services::random();
      for (auto i = 0; i < gunParams->bulletAmount; i++) {
        auto angle = (random->nextFloat() - 0.5f) * PI ;
        auto spreadFactor = gunParams->accuracy;
        auto rotatedDirection = glm::rotate(fireDirection, angle * spreadFactor);
        // fireDirection.x += (xVar * 2 * spreadFactor) - spreadFactor;
        // fireDirection.y += (yVar * 2 * spreadFactor) - spreadFactor;
        _bulletsShot++;

        std::shared_ptr<Node<Transform3D>> bulletNode;
        std::shared_ptr<b2BodyDef> bodyDef;
        std::shared_ptr<b2FixtureDef> fixtureDef;

        std::tie(bulletNode, bodyDef, fixtureDef) = NodeFactory::createBullet(gunParams);
        // fixtureDef->filter.groupIndex = -1;
        bodyDef->position.Set(
          pos.x + fireDirection.x * 0.4f,
          pos.y + fireDirection.y * 0.4f
        );
        bodyDef->linearVelocity.Set(
          rotatedDirection.x * gunParams->bulletSpeed,
          rotatedDirection.y * gunParams->bulletSpeed
        );

        Services::messagePublisher()->sendMessage(Message("gameScene", std::make_shared<CreateNodeEvent>(
          "world/bullets", bulletNode, bodyDef, fixtureDef
        )));
      }

      Services::messagePublisher()->sendMessage(
        Message(
          "audioPlayer:clip/" + gunParams->fireSound,
          std::make_shared<AudioClipEvent>(CLIP_PLAY)
        )
      );
    }

    if (throwBomb && bombTimer <= 0.0f) {
      bombTimer = 3.0f;
      std::shared_ptr<Node<Transform3D>> bombNode;
      std::shared_ptr<b2BodyDef> bodyDef;
      std::shared_ptr<b2FixtureDef> fixtureDef;

      std::tie(bombNode, bodyDef, fixtureDef) = NodeFactory::createBomb();
      bodyDef->position.Set(pos.x + _lookDirection.x * 0.5, pos.y + _lookDirection.y * 0.5);

      Services::messagePublisher()->sendMessage(
        Message("gameScene",
          std::make_shared<CreateNodeEvent>("world/bullets",
            bombNode, bodyDef, fixtureDef
          )
        )
      );
      Services::messagePublisher()->sendMessage(
        Message("audioPlayer:clip/set_bomb.ogg",
          std::make_shared<AudioClipEvent>(CLIP_PLAY)
        )
      );
    }
    if (bombTimer > 0.0f) {
      bombTimer -= delta;
    }

    if (damageToTake > 0.0f) {
      if (hp >= 0.0f) {
        Services::messagePublisher()->sendMessage(
          Message("audioPlayer:clip/pain.ogg",
            std::make_shared<AudioClipEvent>(CLIP_PLAY)
          )
        );

        auto damageParticle = NodeFactory::createDamageParticle((int)damageToTake);
        auto randomPos = (Services::random()->nextFloat() - 0.5f) * 1.0f;

        damageParticle->setLocalPosition(glm::vec3(pos.x + randomPos, pos.y, pos.z + 1));

        Services::messagePublisher()->sendMessage(
          Message("gameScene",
            std::make_shared<CreateNodeEvent>("world",
              damageParticle, nullptr, nullptr
            )
          )
        );

      }

      hp -= damageToTake;
      damageToTake = 0;

      if (hp <= 0.0f) {
        moveUp = false;
        moveRight = false;
        moveDown = false;
        moveLeft = false;
        fireUp = false;
        fireRight = false;
        fireDown = false;
        fireLeft = false;

        node.addAttachment(std::make_shared<SpriteAttachment>("test-effect/blood_red25"));

        auto meat = NodeFactory::createMeatPieces(pos.xy(), glm::normalize(_collisionVec),  6);

        for (auto m : meat) {
          Services::messagePublisher()->sendMessage(Message("gameScene", std::make_shared<CreateNodeEvent>(
            "world/bullets", std::get<0>(m), std::get<1>(m), std::get<2>(m)
          )));
        }
      }
    }

    if (respawn && _lives > 0) {
      Services::messagePublisher()->sendMessage(Message("gameScene", std::make_shared<RespawnEvent>(
        _playerId, _lives-1
      )));
      Services::messagePublisher()->sendMessage(Message("gameScene", std::make_shared<DestroyNodeEvent>(
        node.path()
      )));
    }

    auto gunAttachment = node.get<GunAttachment>();
    std::string gunName = "";

    if (gunAttachment.isDefined()) {
      gunName = gunAttachment.get().parameters()->gunName;
    }

    if (hp > 0.0f && _lives >= 0) {
      Services::messagePublisher()->sendMessage(Message("gameScene:" + _cameraAddress, std::make_shared<UpdateHudEvent>(
        "Player " + std::to_string(_playerId) + "\nHP: " + std::to_string((int) hp) + "\nLIVES: " + std::to_string(_lives) + "\nGUN: " + gunName + "\n"
      )));
    } else if(hp <= 0.0f && _lives > 0) {
      Services::messagePublisher()->sendMessage(Message("gameScene:" + _cameraAddress, std::make_shared<UpdateHudEvent>(
        "Player " + std::to_string(_playerId) + "\nPress FIRE to respawn "
      )));
    } else {
      Services::messagePublisher()->sendMessage(Message("gameScene:" + _cameraAddress, std::make_shared<UpdateHudEvent>(
        "Player " + std::to_string(_playerId) + "\nGAME OVER"
      )));
    }


    throwBomb = false;
    respawn = false;
  }

private:
  float hp = 100.0f;

  int counter = 0;
  float damageToTake = 0.0f;

  bool moveUp = false;
  bool moveRight = false;
  bool moveDown = false;
  bool moveLeft = false;

  bool fireUp = false;
  bool fireRight = false;
  bool fireDown = false;
  bool fireLeft = false;
  bool isFiring = false;
  bool _isStrafing = false;

  bool respawn = false;

  int _lives;

  int _bulletsShot = 0;

  float _fireDelay = 0;

  bool throwBomb = false;
  float bombTimer = 0.0f;

  float _playerSpeed = 3.0f;
  float _gunPickupDelay = 0.0f;

  std::string _cameraAddress;
  int _playerId;
  glm::vec2 _collisionVec = glm::vec2(0,0);
  glm::vec2 _lookDirection = glm::vec2(1, 0);
  Option<std::shared_ptr<GunParameters>> _newGun = Option<std::shared_ptr<GunParameters>>();
};
