#pragma once

#include "game/Updateable.hpp"
#include "scene/SceneGraph.hpp"
#include "message/MessageSubscriber.hpp"

/**
 * Scene interface.
 */
template <typename T>
class Scene : public Updateable, public MessageSubscriber {
public:
  Scene() {}
  virtual ~Scene() {}

protected:
  SceneGraph<T> _graph;

};
