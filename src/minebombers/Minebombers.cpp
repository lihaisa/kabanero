#include "minebombers/Minebombers.hpp"

#include "service/Services.hpp"
#include "message/event/AudioClipEvent.hpp"
#include "message/event/AudioTrackEvent.hpp"
#include "minebombers/level/LevelCompiler.hpp"
#include "minebombers/level/CaveGenerator.hpp"
#include "minebombers/behaviors/CameraBehavior.hpp"
#include "minebombers/events/TestEvent.hpp"

auto Minebombers::init() -> void {
  auto messagePublisher = Services::messagePublisher();
  messagePublisher->sendMessage(
    Message(
      "audioPlayer:clip/test_clip.ogg",
      std::make_shared<AudioClipEvent>(CLIP_PLAY)
    )
  );
  messagePublisher->sendMessage(
    Message(
      "audioPlayer:track/jazz",
      std::make_shared<AudioTrackEvent>(TRACK_CHANGE, "resources/audio/local_forecast.ogg")
    )
  );
  messagePublisher->sendMessage(
    Message(
      "audioPlayer:track/jazz",
      std::make_shared<AudioTrackEvent>(TRACK_PLAY)
    )
  );

  auto rootNode = std::make_shared<Node<Transform3D>>("world");

  int seed = 4;

  auto random = StdLibRandom();
  random.seed(seed);

  auto caveGen = CaveGenerator(seed, 64, 64, 4, 3);
  auto tileMap = caveGen.generate();
  auto levelCompiler = LevelCompiler(random);

  rootNode->addChild(levelCompiler.materializeGround(tileMap));
  rootNode->addChild(levelCompiler.materializeObjects(tileMap));

  Services::logger()->debug("num children: " + std::to_string(rootNode->children().values().length()));

  auto cameraNode = std::make_shared<Node<Transform3D>>("camera");
  cameraNode->setLocalPosition(glm::vec3(12, 11, 0));
  cameraNode->addBehavior<CameraBehavior>(1.1f);
  rootNode->addChild(cameraNode);

  auto scene = std::make_shared<GameScene<Transform3D>>("gameScene", rootNode);

  SceneView<Transform3D> sceneView(rootNode, cameraNode, Viewport(0, 0, 1, 1));
  scene->addSceneView(sceneView);

  addScene(scene);

  messagePublisher->sendMessage(
    Message(
      "gameScene:camera",
      std::make_shared<TestEvent>(A)
    )
  );
  messagePublisher->sendMessage(
    Message(
      "gameScene:camera",
      std::make_shared<TestEvent>(B)
    )
  );
}
