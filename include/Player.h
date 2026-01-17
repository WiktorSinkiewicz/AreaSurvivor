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
    void addXP(int amount);

    int getLevel() const { return level; }
    float getXPProgress() const;
    float getHpProgress() const;

    float getDamage() const { return damage; }
    int getProjectileCount() const { return projectileCount; }
    int getPenetration() const { return penetration; }

private:
    void levelUp();

    // Statystyki przetrwania i walki
    float maxHp;
    float currentHp;
    float moveSpeed;      // Mno¿nik prêdkoœci
    float damage;
    int projectileCount;
    int penetration;

    // Statystyki progresji
    int level;
    int currentXP;
    int maxXP;
};