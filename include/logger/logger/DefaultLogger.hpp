#pragma once

#include <string>
#include <iostream>

#include "collection/mutable/KBMap.hpp"
#include "logger/Logger.hpp"

/**
 * DefaultLogger class.
 * @todo Figure out a better way to define colors and headers
 */
class DefaultLogger: public Logger {
public:
  DefaultLogger(): level(DEBUG) {
    colors[DEBUG] = "\u001B[32m";
    colors[INFO] = "\u001B[34m";
    colors[WARN] = "\u001B[33m";
    colors[ERROR] = "\u001B[31m";
    colors[FATAL] = "\u001B[35m";

    headers[DEBUG] = " DEBUG ";
    headers[INFO] = " INFO  ";
    headers[WARN] = " WARN  ";
    headers[ERROR] = " ERROR ";
    headers[FATAL] = " FATAL ";
  }
  ~DefaultLogger() {}

  auto setLevel(const LogLevel l) -> void { level = l; }

  auto debug(const std::string& message) -> void { msg(message, DEBUG); }
  auto info(const std::string& message) -> void { msg(message, INFO); }
  auto warn(const std::string& message) -> void { msg(message, WARN); }
  auto error(const std::string& message) -> void { msg(message, ERROR); }
  auto fatal(const std::string& message) -> void { msg(message, FATAL); }

protected:
  LogLevel level;

  bool enableColor = true;
  std::string colorReset = "\u001B[0m";

  KBMap<LogLevel, std::string> colors;
  KBMap<LogLevel, std::string> headers;

private:
  auto msg(const std::string& message, LogLevel l) -> void {
    if(l >= level){
      if(enableColor){
        printConsole(colors[l] + "[" + headers[l] + "] " + message + "\u001B[0m");
      } else {
        printConsole("[" + headers[l] + "] " + message + "\u001B[0m");
      }
    }
  }

  auto printConsole(const std::string& line) -> void {
    std::cout << line << std::endl;
  }
};
