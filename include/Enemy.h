#pragma once
#include "Entity.h"

class Enemy : public Entity {
public:
    Enemy(float startX, float startY, int hp);

    // Ta wersja jest wymagana przez klasê bazow¹ Entity
    void update(float dt) override {}

    // Ta wersja jest u¿ywana w Game.cpp do œledzenia gracza
    void update(float dt, sf::Vector2f targetPos);

    void applyKnockback(sf::Vector2f dir, float force);
    void takeDamage(int amount) { health -= amount; }
    bool isDead() const { return health <= 0; }

private:
    int health;
    sf::Vector2f knockbackVel;
};