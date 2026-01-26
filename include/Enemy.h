#pragma once
#include "Entity.h"
#include "Constants.h"

class Enemy : public Entity {
public:
    Enemy(float startX, float startY, float hp, float speed, float dmg);

    void update(float dt) override {};
    void update(float dt, sf::Vector2f targetPos);

    void applyKnockback(sf::Vector2f dir, float force);
    void takeDamage(float amount) { health -= amount; }
    bool isDead() const { return health <= 0; }
    float getDamage() const { return damage; }
    void freeze(float duration) {
        if (isFrozen) return;
        isFrozen = true;
        freezeTimer = duration;
        freezeImmunityTimer = duration * Config::FREEZE_IMMUNITY_MULT;
    }

    bool isFrozen = false;
    float freezeTimer = 0.f;
    float freezeImmunityTimer = 0.f;

private:
    float health;
    float speed;
    float damage;
    sf::Vector2f knockbackVel;
};