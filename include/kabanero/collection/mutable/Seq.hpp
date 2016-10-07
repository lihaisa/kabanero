#pragma once

#include <functional>
#include <vector>
#include <list>
#include <iostream>
#include <algorithm>

#include "kabanero/Option.hpp"

template <template <class T, class Allocator = std::allocator<T>> class C, typename T>
class Seq {
public:
  Seq() {}

  Seq(const Seq& copy) : memory(copy.memory) {}

  typedef C<T> Memory;
  typedef typename Memory::iterator iterator;
  typedef typename Memory::const_iterator const_iterator;

  auto length() -> size_t {
    return memory.size();
  }

  template <typename F>
  auto foreach(const F& func) const -> void {
    for (auto const& i : memory) {
      func(i);
    }
  }

  template <typename F>
  auto foreach(F func) -> void {
    for (auto& i : memory) {
      func(i);
    }
  }

  auto begin() -> iterator {
    return memory.begin();
  }

  auto end() -> iterator {
    return memory.end();
  }

  auto begin() const -> const_iterator {
    return memory.cbegin();
  }

  auto end() const -> const_iterator {
    return memory.cend();
  }

  auto operator[](size_t index) -> T& {
    return memory[index];
  }

  auto operator[](size_t index) const -> const T& {
    return memory[index];
  }

  auto operator+=(T elem) -> void {
    memory.push_back(elem);
  };

  template <typename F, typename R = typename std::result_of<F&(T)>::type>
  auto map(F func) const -> Seq<C, R> {
    auto a = Seq<C, R>();
    for (auto const& i : memory) {
      a += func(i);
    }
    return a;
  }

  template <typename F>
  auto fold(T const& start, F func) -> T {
    auto s = start;
    for (auto const& i : memory) {
      s = func(s, i);
    }
    return s;
  }

  template <typename F>
  auto find(F func) const -> const Option<T> {
    const auto it = std::find_if(begin(), end(), func);
    if (it == end()) {
      return Option<T>();
    } else {
      return Some(*it);
    }
  }

  template <typename F>
  auto exists(F func) const -> bool {
    const auto it = std::find_if(begin(), end(), func);
    return it != end();
  }

  template <typename F>
  auto sortBy(F func) const -> Seq<C, T> {

  }

  template <typename F>
  auto filter(F func) const -> Seq<C, T> {
    auto a = Seq<C, T>();
    for (auto const& i : memory) {
      if (func(i)) {
        a += i;
      }
    }
    return a;
  }

  auto remove(const int i) -> const T {
    const auto r = memory[i];
    memory.erase(memory.begin() + i);
    return r;
  }

  auto toVector() -> std::vector<T> {
    return std::vector<T>(begin(), end());
  }

protected:
  Memory memory;
};

template <template <class T, class Allocator> class C, typename T>
auto operator<<(std::ostream& os, const Seq<C, T>& collection) -> std::ostream& {
  os << "[ ";
  for (auto it = collection.begin(); it != collection.end(); it++) {
    os << *it;
    if (it != (--collection.end())) {
      os << ", ";
    }
  }
  os << " ]";
  return os;
}

template<typename T>
class VectorSeq : public Seq<std::vector, T> {
public:
  VectorSeq() : Seq<std::vector, T>() {}
  VectorSeq(const VectorSeq& copy) : Seq<std::vector, T>(copy) {}
};
