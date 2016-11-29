#pragma once

#include <Box2D/Box2D.h>
#include "game/Game.hpp"

class Minebombers : public Game {
public:
  Minebombers() : _physWorld(b2Vec2(0.0f,0.0f)) {}
  auto init() -> void;
  auto update(double delta) -> void override {
    Game::update(delta);
    _phys_elapsed += delta;
    while (_phys_elapsed >= _phys_step) {
      _phys_elapsed -= _phys_step;
      _physWorld.Step(_phys_step, 8, 3);
    }
  }
private:
  b2World _physWorld;
  double _phys_elapsed = 0.0;
  const double _phys_step = 1000.0 / 30.0;
};
