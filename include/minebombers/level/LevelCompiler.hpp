#pragma once

#include "minebombers/level/TileMap.hpp"
#include "minebombers/level/FogMap.hpp"
#include "minebombers/level/TerrainFactory.hpp"
#include "scene/Node.hpp"
#include "scene/3D/Transform3D.hpp"
#include "physics/CollisionData.hpp"
#include "scene/attachment/SpriteAttachment.hpp"
#include "minebombers/behaviors/FogBehaviour.hpp"
#include "minebombers/behaviors/PlayerBehaviour.hpp"
#include "minebombers/behaviors/WallBehavior.hpp"
#include "minebombers/attachments/CollisionMaterialAttachment.hpp"

#include "minebombers/attachments/GunAttachment.hpp"
#include "minebombers/behaviors/ItemNodeBehaviour.hpp"
#include <sstream>

class LevelCompiler {
public:
  static constexpr float CHUNK_SIZE = 8.0f;

  LevelCompiler(Random& rand, b2World& w): _rand(rand), _world(w) {}

  auto materializeLevel(
    std::shared_ptr<TileMap> map,
    std::shared_ptr<Node<Transform3D>> root
  ) -> void {

    auto level = std::make_shared<Node<Transform3D>>("level");
    level->setSleep(true);
    root->addChild(level);

    auto ground = std::make_shared<Node<Transform3D>>("ground");
    ground->setLocalPosition(glm::vec3(0,0,0));
    level->addChild(ground);

    auto obj = std::make_shared<Node<Transform3D>>("obj");
    obj->setLocalPosition(glm::vec3(0,0,2));
    level->addChild(obj);


    auto items = normalGuns();
    auto artifacts = artifactGuns();

    const auto mapWidth = map->getWidth();
    const auto mapHeight = map->getHeight();

    const auto xChunks = ceil(mapWidth / CHUNK_SIZE);
    const auto yChunks = ceil(mapHeight / CHUNK_SIZE);

    for (auto xChunk = 0; xChunk < xChunks; xChunk++) {
      for (auto yChunk = 0; yChunk < yChunks; yChunk++) {
        auto chunkNode = std::make_shared<Node<Transform3D>>(name("chunk", xChunk, yChunk));
        chunkNode->setLocalPosition(glm::vec3(xChunk * CHUNK_SIZE, yChunk * CHUNK_SIZE, 0));
        chunkNode->setSleep(true);
        obj->addChild(chunkNode);
        for (auto x = 0; x < CHUNK_SIZE; x++) {
          for (auto y = 0; y < CHUNK_SIZE; y++) {
            auto totalX = (int)(xChunk * CHUNK_SIZE + x);
            auto totalY = (int)(yChunk * CHUNK_SIZE + y);
            auto tileNode =  std::make_shared<Node<Transform3D>>(name("tile", totalX, totalY));
            tileNode->setLocalPosition(glm::vec3(x, y, 0));
            chunkNode->addChild(tileNode);

            auto item = items[(totalX+totalY)%items.length()];
            auto artifact = artifacts[(totalX+totalY)%artifacts.length()];

            auto terrainType = (*map)[totalX][totalY].getType();

            switch (terrainType) {
              case CAVE_WALL:
              case INDESCTRUCTIBLE_WALL:
              case CONSTRUCTED_WALL:
              case WINDOW: {
                const auto terrain = TerrainFactory::generateTerrain(
                  terrainType, "terrain", _world, map);
                tileNode->addChild(terrain);
                const auto pos = terrain->position();
                const auto& physAttachment = terrain->get<PhysicsAttachment>();
                physAttachment.foreach([&](auto phys) {
                  phys.setPosition(pos.x, pos.y);
                });
                break;
              }
              case ITEM_LOCATION:
                tileNode->addChild(getItem(item, totalX, totalY));
                break;
              case ARTIFACT_LOCATION:
                tileNode->addChild(getItem(artifact, totalX, totalY));
                break;
            }
          }
        }
      }
    }
  }

  auto initFog(
    std::shared_ptr<TileMap> map, std::shared_ptr<FogMap> fogMap,
    std::shared_ptr<Node<Transform3D>> root
  ) -> void {

    fogMap->init(map->getWidth(), map->getHeight());
    auto fogNode = std::make_shared<Node<Transform3D>>("fog");
    root->addChild(fogNode);

    fogNode->setLocalPosition(glm::vec3(0,0,100));
    for (auto x = 0; x < map->getWidth(); x++) {
      for (auto y = 0; y < map->getHeight(); y++) {
        auto node = std::make_shared<Node<Transform3D>>(name("fog", x, y));
        node->addAttachment(getSprite("tiles/fog", -1));
        node->setLocalPosition(glm::vec3(x,y,0));
        node->setSleep(true);
        fogNode->addChild(node);
        (*fogMap)[x][y] = node;
      }
    }
    fogNode->addBehavior<FogBehaviour>(map, fogMap);
  }

  auto materializePlayer(
    std::shared_ptr<TileMap> map,
    std::shared_ptr<Node<Transform3D>> root
  ) -> void{
    auto tile = map->getRandom(PLAYER_SPAWN_POINT, _rand);

    auto node = std::make_shared<Node<Transform3D>>("player");
    node->setLocalPosition(glm::vec3(tile.getX(), tile.getY(), 2));
    node->addAttachment(getSprite("tiles/spriggan_druid", -1));

    auto material_att = std::make_shared<CollisionMaterialAttachment>();
    material_att->bulletRebound = true;
    material_att->isPlayer = true;

    node->addAttachment(material_att);
    auto gun = std::make_shared<GunAttachment>(5.0f, 1.0f, 1, 0.4f, 8.0f, "tiles/sniper_normal");
    node->addAttachment(gun);

    auto physCircle = createPhysCircle(tile.getX(), tile.getY());
    auto physAttachment = std::make_shared<PhysicsAttachment>(physCircle);

    node->addAttachment(physAttachment);

    node->addBehavior<PlayerBehaviour>();
    root->addChild(node);
  }

  auto normalGuns() -> KBVector<std::shared_ptr<GunAttachment>> {
    auto guns = KBVector<std::shared_ptr<GunAttachment>>();
    auto pistol = std::make_shared<GunAttachment>(15.0f, 2.0f, 1, 0.1f, 15.0f, "tiles/pistol_normal");
    auto rifle = std::make_shared<GunAttachment>(8.0f, 6.0f, 1, 0.3f, 12.0f, "tiles/rifle_normal");
    auto shotgun = std::make_shared<GunAttachment>(10.0f, 1.5f, 3, 0.2f, 10.0f, "tiles/shotgun_normal");
    auto sniper = std::make_shared<GunAttachment>(60.0f, 0.5f, 1, 0.0f, 25.0f, "tiles/sniper_normal");
    guns += rifle;
    guns += pistol;
    guns += shotgun;
    guns += sniper;

    return guns;
  }

  auto artifactGuns() -> KBVector<std::shared_ptr<GunAttachment>> {
    auto guns = KBVector<std::shared_ptr<GunAttachment>>();
    auto megaPistol = std::make_shared<GunAttachment>(45.0f, 2.0f, 1, 0.05f, 20.0f, "tiles/pistol_artifact");
    auto killerRifle = std::make_shared<GunAttachment>(20.0f, 10.0f, 1, 0.25f, 20.0f, "tiles/rifle_artifact");
    auto superShotgun = std::make_shared<GunAttachment>(15.0f, 2.5f, 5, 0.15f, 15.0f, "tiles/shotgun_artifact");
    auto uberSniper = std::make_shared<GunAttachment>(140.0f, 0.75f, 1, 0.0f, 45.0f, "tiles/sniper_artifact");
    guns += killerRifle;
    guns += megaPistol;
    guns += superShotgun;
    guns += uberSniper;

    return guns;
  }

  auto createPhysSquare(float x, float y) -> b2Body* {
    b2BodyDef bodyDef;
    bodyDef.position.Set(x, y);
    b2Body* body = _world.CreateBody(&bodyDef);
    b2PolygonShape box;
    box.SetAsBox(0.5f, 0.5f);
    body->CreateFixture(&box, 0.0f);
    return body;
  }
  auto createPhysCircle(float x, float y) -> b2Body* {
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(x, y);
    bodyDef.allowSleep = false;
    b2Body* body = _world.CreateBody(&bodyDef);
    b2CircleShape circleShape;
    circleShape.m_p.Set(0, 0); //position, relative to body position
    circleShape.m_radius = 0.35f;
    body->CreateFixture(&circleShape, 1.0f);
    return body;
  }
private:
  Random& _rand;
  b2World& _world;
  auto getSprite(std::string baseName, int numVariations) -> std::shared_ptr<SpriteAttachment> {
    if (numVariations == -1) {
      return std::make_shared<SpriteAttachment>(baseName);
    }
    std::ostringstream oss;
    oss << baseName << _rand.nextInt(numVariations);
    return std::make_shared<SpriteAttachment>(oss.str());
  }
  auto name(std::string base, int x, int y) -> std::string {
    std::ostringstream oss;
    oss << base << x << "-" << y;
    return oss.str();
  }
  // auto getTerrain(std::string baseSprite, int spriteVariation, float health, int x, int y, std::shared_ptr<TileMap> map) -> std::shared_ptr<Node<Transform3D>> {
  //   auto node = std::make_shared<Node<Transform3D>>(name("obj",x,y));
  //   node->setLocalPosition(glm::vec3(x, y, 0));
  //   node->addAttachment(getSprite(baseSprite, spriteVar));
  //
  //   //node->addBehavior<WallBehavior>();
  //   auto terrainBehaviour = node->addBehavior<TerrainBehaviour>(health, map, x, y);
  //   auto material_att = std::make_shared<CollisionMaterialAttachment>();
  //
  //   material_att->staticMaterial = true;
  //
  //   node->addAttachment(material_att);
  //
  //   auto physBody = createPhysSquare(x, y);
  //
  //   auto physAttachment = std::make_shared<PhysicsAttachment>(physBody);
  //   node->addAttachment(physAttachment);
  //
  //   return node;
  // }
  auto getItem(std::shared_ptr<GunAttachment> gun, int x, int y) -> std::shared_ptr<Node<Transform3D>> {
    auto node = std::make_shared<Node<Transform3D>>(name("item",x,y));
    node->setLocalPosition(glm::vec3(x,y,0));
    node->addAttachment(getSprite(gun->sprite, -1));

    auto itBeh = node->addBehavior<ItemNodeBehaviour>();
    auto material_att = std::make_shared<CollisionMaterialAttachment>();
    material_att->gunItem = Some<std::shared_ptr<GunAttachment>>(gun);
    // material_att->itemLink = itBeh;
    node->addAttachment(material_att);

    auto physBody = createPhysSquare(x, y);

    auto collisionData = new CollisionData("", material_att);
    physBody->SetUserData(collisionData);

    auto physAttachment = std::make_shared<PhysicsAttachment>(physBody);
    node->addAttachment(physAttachment);

    return node;
  }
};
