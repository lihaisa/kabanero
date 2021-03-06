#pragma once

#include "collection/mutable/Seq.hpp"

/**
 * Standard library vector with additional functional style methods.
 */
template<typename T>
class KBVector : public Seq<std::vector, T> {
public:
  KBVector() : Seq<std::vector, T>() {}
  KBVector(const KBVector& copy) : Seq<std::vector, T>(copy) {}
  KBVector(std::initializer_list<T> init) : Seq<std::vector, T>(init) {}
};
