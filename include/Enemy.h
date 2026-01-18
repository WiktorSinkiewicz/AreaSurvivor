#pragma once
#include "Entity.h"

class Enemy : public Entity {
public:
    // Konstruktor musi przyjmowaæ te 5 parametrów zgodnie z Game::spawnEnemy
    Enemy(float startX, float startY, float hp, float speed, float dmg);

    void update(float dt) override {};
    void update(float dt, sf::Vector2f targetPos);

    void applyKnockback(sf::Vector2f dir, float force);
    void takeDamage(float amount) { health -= amount; }
    bool isDead() const { return health <= 0; }
    float getDamage() const { return damage; }

private:
    float health;
    float speed;
    float damage;
    sf::Vector2f knockbackVel; // To musi tu byæ!
};