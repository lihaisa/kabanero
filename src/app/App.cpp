// NOTE: Enables swizzleing of vectors, but may cause name conflicts
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <sol.hpp>

#include "app/App.hpp"
#include "service/Services.hpp"
#include "logger/DefaultLogger.hpp"
#include "resource/resourceManager/SyncResourceManager.hpp"
#include "message/messagePublisher/DefaultMessagePublisher.hpp"
#include "resource/loader/TextLoader.hpp"
#include "resource/loader/AudioLoader.hpp"
#include "resource/loader/TextureLoader.hpp"
#include "resource/loader/AudioLoader.hpp"
#include "resource/loader/AtlasLoader.hpp"
#include "audio/AudioPlayer.hpp"
#include "input/InputTranslator.hpp"

App::App(std::shared_ptr<Game> game) : _game(game) {
  auto logger = std::make_shared<DefaultLogger>();
  auto resourceManager = std::make_shared<SyncResourceManager>();
  auto messagePublisher = std::make_shared<DefaultMessagePublisher>();

  Services::provideMessagePublisher(messagePublisher);
  Services::provideResourceManager(resourceManager);
  Services::provideLogger(logger);

  logger->debug("Debug test");
  logger->info("Info test");
  logger->warn("Warn test");
  logger->error("Error test");
  logger->fatal("Fatal test");

  sol::state config;
  config.script_file("resources/config.lua");

  logger->info("Loading resources");

  auto resources = config.get<sol::table>("resources");

  auto text_loader = std::make_shared<TextLoader>();
  std::regex text_regex("^.+\\.txt$");
  resourceManager->addLoader(text_regex, text_loader);

  auto audio_loader = std::make_shared<AudioLoader>();
  std::regex audio_regex("^.+\\.ogg$");
  resourceManager->addLoader(audio_regex, audio_loader);

  auto texture_loader = std::make_shared<TextureLoader>();
  std::regex texture_regex("^.+\\.png$");
  resourceManager->addLoader(texture_regex, texture_loader);

  auto atlas_loader = std::make_shared<AtlasLoader>();
  std::regex atlas_regex("^.+\\.atlas$");
  resourceManager->addLoader(atlas_regex, atlas_loader);

  for(auto i = 1; i <= resources.size(); i++){
    logger->debug(resources.get<std::string>(i));
    resourceManager->load(resources.get<std::string>(i));
  }

  // Intervals
  _update_interval = 1.0 / config["update_fps"].get_or(30);
  _draw_interval = 1.0 / config["draw_fps"].get_or(30);

  // Get window parameters from config file and create window
  _window_w = config["window_width"].get_or(800);
  _window_h = config["window_height"].get_or(800);
  _window_name = config["window_name"].get_or<std::string>("window");


  _audioFolderPath = config["audio_folder"].get_or<std::string>("resources/audio/");
  _tilesize = config["tilesize"].get_or(32);
}

auto App::init() -> void {
  Services::logger()->info("Creating game");

  _game->init();
}

auto App::run() -> void {
  sf::RenderWindow window(sf::VideoMode(_window_w, _window_h), _window_name);
  auto audioPlayer = std::make_shared<AudioPlayer>(_audioFolderPath);
  Services::messagePublisher()->addSubscriber(audioPlayer);
  Renderer renderer(window, _tilesize);
  typedef std::chrono::high_resolution_clock Clock;
  typedef std::chrono::milliseconds ms;

  auto last_update_time = Clock::now();
  auto last_draw_time = Clock::now();


  std::shared_ptr<InputTranslator> inputTranslator = std::make_shared<InputTranslator>();

  auto timeAccumulator = 0.0f;

  while (window.isOpen()) {

    auto current_time = Clock::now();

    auto update_delta_ms = std::chrono::duration_cast<ms>(current_time - last_update_time);
    double update_delta = update_delta_ms.count() / 1000.0;

    std::cout << update_delta << std::endl;
    std::cout << _update_interval << std::endl;

    timeAccumulator += update_delta;

    auto draw_delta_ms = std::chrono::duration_cast<ms>(current_time - last_draw_time);
    double draw_delta = draw_delta_ms.count() / 1000.0;

    auto max_steps = 3;
    auto i = 0;

    while (timeAccumulator > _update_interval && i < max_steps) {
      Services::messagePublisher()->publishMessages();
      _game->update(_update_interval);
      timeAccumulator -= _update_interval;
      ++i;
    }
    last_update_time = current_time;
    if (draw_delta_ms.count() > _draw_interval) {
      _game->render(renderer);
    }

    sf::Event event;
    while (window.pollEvent(event)) {
      inputTranslator->processMessage(event);
      if (event.type == sf::Event::Closed) {
        window.close();
      } else if (event.type == sf::Event::Resized) {
        window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
      }
    }
  }
}
