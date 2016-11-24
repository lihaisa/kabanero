#pragma once

#include <glm/vec2.hpp>
#include <string>

class Sprite {
public:
  Sprite(
      std::string texturePath,
      glm::vec2 xy,
      glm::vec2 size,
      glm::vec2 origin,
      glm::vec2 offset) :
    _texturePath(texturePath),
    _xy(xy),
    _size(size),
    _origin(origin),
    _offset(offset) {}

  auto texturePath() const -> std::string {
    return _texturePath;
  }

  auto xy() const -> glm::vec2 {
    return _xy;
  }

  auto size() const -> glm::vec2 {
    return _size;
  }

private:
  std::string _texturePath;
  glm::vec2 _xy;
  glm::vec2 _size;
  glm::vec2 _origin;
  glm::vec2 _offset;
};
