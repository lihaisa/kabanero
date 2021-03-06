#pragma once

#include <string>

/**
 * Gun parameters.
 * Contains all values that are needed for gun.
 */
class GunParameters {
public:
  GunParameters(
    float dmg,
    float fr,
    int ba,
    float acc,
    float sped,
    float walkSpeed,
    std::string sprite,
    std::string bltSprite,
    std::string sound,
    bool rocket = false,
    float expSize = 0,
    int shrapnel = 0,
    int bltSize = 0,
    std::string name = "Gun"):
  damage(dmg),
  fireRate(fr),
  bulletAmount(ba),
  accuracy(acc),
  bulletSpeed(sped),
  walkSpeed(walkSpeed),
  sprite(sprite),
  bulletSprite(bltSprite),
  isRocketLauncher(rocket),
  fireSound(sound),
  explosionSize(expSize),
  shrapnelCount(shrapnel),
  bulletSize(bltSize),
  gunName(name)
   {}

  GunParameters(const GunParameters& params) {
    damage = params.damage;
    fireRate = params.fireRate;
    walkSpeed = params.walkSpeed;
    bulletAmount = params.bulletAmount;
    accuracy = params.accuracy;
    bulletSpeed = params.bulletSpeed;
    sprite = params.sprite;
    bulletSprite = params.bulletSprite;
    isRocketLauncher = params.isRocketLauncher;
    explosionSize = params.explosionSize;
    shrapnelCount = params.shrapnelCount;
    bulletSize = params.bulletSize;
    fireSound = params.fireSound;
    gunName = params.gunName;
  }

  float damage;
  float fireRate; // per second
  float walkSpeed;
  int bulletAmount; // bullets per shoot
  float accuracy; //smaller == better
  float bulletSpeed;
  std::string sprite;
  std::string bulletSprite;
  bool isRocketLauncher;
  float explosionSize;
  int shrapnelCount;
  float bulletSize;
  std::string fireSound;
  std::string gunName;
};
