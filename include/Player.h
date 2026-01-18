#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Constants.h"

class Player : public Entity {
public:
    Player();
    void handleInput(bool blockInput);
    void update(float dt) override;
    void takeDamage(float amount);
    bool addXP(int amount);

    float getCurrentHp() const { return currentHp; }
    float getMaxHp() const { return maxHp; }
    int getCurrentXp() const { return currentXP; }
    int getMaxXp() const { return maxXP; }
    int getLevel() const { return level; }
    float getXPProgress() const;
    float getHpProgress() const;
    float getDamage() const { return damage; }
    int getProjectileCount() const { return projectileCount; }
    int getPenetration() const { return penetration; }
    float getPickupRange() const { return pickupRange; }

    // Metody ulepszeñ statystyk
    void upgradeHp() {
        maxHp += Config::UPGRADE_HP_VAL;
        currentHp += Config::UPGRADE_HP_VAL;
    }
    void upgradeDmg() { damage += Config::UPGRADE_DMG_VAL; }
    void upgradeSpd() { moveSpeed += Config::UPGRADE_SPD_VAL; }
    void upgradeProj() { projectileCount += Config::UPGRADE_PROJ_VAL; }
    void upgradePen() { penetration += Config::UPGRADE_PEN_VAL; }
    void upgradePickupRange() { pickupRange += Config::UPGRADE_PICKUP_VAL; }

private:
    void levelUp();
    float invulnTimer = 0.f;

    float maxHp;
    float currentHp;
    float moveSpeed;
    float damage;
    int projectileCount;
    int penetration;
    float pickupRange;

    int level;
    int currentXP;
    int maxXP;
};