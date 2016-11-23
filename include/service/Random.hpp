#pragma once

class Random {
public:
  Random() {}
  virtual ~Random() {};

  virtual auto seed(int seed) -> void = 0;
  virtual auto nextFloat() -> float = 0;
  virtual auto nextInt(int mod) -> int = 0;
};
class NullRandom : public Random {
public:
  NullRandom() {}

  auto seed(int seed) -> void { }
  auto nextFloat() -> float { return 0.35f; }
  auto nextInt(int mod) -> int { return 4 % mod; }
};
