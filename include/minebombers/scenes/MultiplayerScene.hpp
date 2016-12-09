#pragma once

#include "scene/scene/GameScene.hpp"
#include "service/Services.hpp"
#include "minebombers/level/CaveGenerator.hpp"
#include "minebombers/level/LevelCompiler.hpp"
#include "minebombers/level/LevelCompiler.hpp"
#include "minebombers/behaviors/CameraBehavior.hpp"
#include "minebombers/events/NewGameEvent.hpp"

class MultiplayerScene {
public:
  static auto createScene(int seed, int numPlayers = 1) -> std::shared_ptr<GameScene> {
    const auto& messagePublisher = Services::messagePublisher();
    auto rootNode = std::make_shared<Node<Transform3D>>("world");

    auto scene = std::make_shared<GameScene>("gameScene", rootNode);

    auto w = 128;
    auto h = 128;

    auto caveGen = CaveGenerator(seed, w, h, 4, 3);
    auto tileMap = caveGen.generate();
    auto random = Services::random();

    auto levelCompiler = LevelCompiler(*random, scene->physWorld());
    levelCompiler.materializeLevel(tileMap, rootNode);
    levelCompiler.materializePlayer(tileMap, rootNode);

    auto cameraNode = std::make_shared<Node<Transform3D>>("camera");
    cameraNode->setLocalPosition(glm::vec3(0, 0, 0));
    cameraNode->addBehavior<CameraBehavior>(0.2f);
    auto visibilityAttachment = std::make_shared<VisibilityAttachment>(w, h, tileMap);
    cameraNode->addAttachment(visibilityAttachment);

    rootNode->addChild(cameraNode);

    auto bulletBag = std::make_shared<Node<Transform3D>>("bullets");
    bulletBag->setLocalPosition(glm::vec3(0, 0, 0));
    rootNode->addChild(bulletBag);

    if (numPlayers == 1) {
      SceneView sceneView(rootNode, cameraNode, Viewport(0, 0, 1.0, 1.0));
      scene->addSceneView(sceneView);
    } else if (numPlayers == 2) {
      SceneView sceneView(rootNode, cameraNode, Viewport(0, 0, 0.5, 1.0));
      scene->addSceneView(sceneView);

      SceneView sceneView2(rootNode, cameraNode, Viewport(0.5, 0, 0.5, 1.0));
      scene->addSceneView(sceneView2);
    } else if (numPlayers == 3) {
      SceneView sceneView(rootNode, cameraNode, Viewport(0, 0, 0.5, 0.5));
      scene->addSceneView(sceneView);

      SceneView sceneView2(rootNode, cameraNode, Viewport(0.5, 0, 0.5, 0.5));
      scene->addSceneView(sceneView2);

      SceneView sceneView3(rootNode, cameraNode, Viewport(0.0, 0.5, 0.5, 0.5));
      scene->addSceneView(sceneView3);
    } else {
      SceneView sceneView(rootNode, cameraNode, Viewport(0, 0, 0.5, 0.5));
      scene->addSceneView(sceneView);

      SceneView sceneView2(rootNode, cameraNode, Viewport(0.5, 0, 0.5, 0.5));
      scene->addSceneView(sceneView2);

      SceneView sceneView3(rootNode, cameraNode, Viewport(0.0, 0.5, 0.5, 0.5));
      scene->addSceneView(sceneView3);

      SceneView sceneView4(rootNode, cameraNode, Viewport(0.5, 0.5, 0.5, 0.5));
      scene->addSceneView(sceneView4);
    }

    scene->addEventReactor([messagePublisher](GameInputEvent event) {
      auto action = event.action();
      if (action == NUM_1) {
        std::cout << "Send new game event 1" << std::endl;
        messagePublisher->sendMessage(Message(
          "game",
          std::make_shared<NewGameEvent>(5, 1)
          )
        );
      } else if (action == NUM_2) {
        std::cout << "Send new game event 2" << std::endl;
        messagePublisher->sendMessage(Message(
          "game",
          std::make_shared<NewGameEvent>(15, 2)
          )
        );
      } else if (action == NUM_3) {
        std::cout << "Send new game event 3" << std::endl;
        messagePublisher->sendMessage(Message(
          "game",
          std::make_shared<NewGameEvent>(25, 3)
          )
        );
      } else if (action == NUM_4) {
        std::cout << "Send new game event 4" << std::endl;
        messagePublisher->sendMessage(Message(
          "game",
          std::make_shared<NewGameEvent>(35, 4)
          )
        );
      }
    });

    return scene;
  }
private:
};
