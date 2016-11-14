#pragma once

#include "message/Event.hpp"

/**
 * Message class.
 * @todo Should address be rather something else than a string?
 */
class Message {
public:
  Message(Event event, std::string address) : _event(event), _address(address) {}
  ~Message() {}

  auto getEvent() -> Event& { return _event; }

private:
  Event _event;
  std::string _address;
};
