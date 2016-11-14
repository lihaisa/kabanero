#pragma once

#include <string>
#include <regex>
#include <tuple>
#include "collection/mutable/KBMap.hpp"
#include "collection/mutable/KBVector.hpp"
#include "collection/mutable/KBTypeMap.hpp"
#include "collection/Option.hpp"
#include "exception/ResourceException.hpp"

#include "resource/Loader.hpp"

class ResourceManager {
public:
  // typedef Future<std::shared_ptr<Resource>> FutureResource;

  auto addLoader(std::regex regex, std::shared_ptr<Loader> loader) -> void {
    _loaders += { regex, loader };
  }

  auto load(std::string filePath) -> void {
    auto loaderOption = _loaders.find([&](const auto& pair) {
      std::regex r;
      std::shared_ptr<Loader> l;
      std::tie(r, l) = pair;
      return std::regex_match(filePath, r);
    }).map([&](const auto& pair) {
      std::regex r;
      std::shared_ptr<Loader> l;
      std::tie(r, l) = pair;
      return l;
    });
    if (loaderOption.isEmpty()) {
      throw ResourceException("No matching loader found for file: " + filePath);
    }
    auto loader = loaderOption.get();
    _resources.insert(loader->load(filePath));
  }

  template <typename T>
  auto get(const std::string& filePath) const -> Option<T> {
    Option<KBMap<std::string, std::shared_ptr<Resource>>> m = _resources.get<T>();

    return m.map([&](KBMap<std::string, std::shared_ptr<Resource>> resourceMap) {
      Option<std::shared_ptr<Resource>> resource = resourceMap.get(filePath);
      return *std::dynamic_pointer_cast<T>(resource.get());
    });
  }

private:
  KBVector<std::tuple<std::regex, std::shared_ptr<Loader>>> _loaders;

  KBTypeMap<KBMap<std::string, std::shared_ptr<Resource>>> _resources;

};
