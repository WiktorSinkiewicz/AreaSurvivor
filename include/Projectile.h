#pragma once
#include "Entity.h"
#include <set>

class Projectile : public Entity {
public:
    // Konstruktor przyjmuje: pozycjê startow¹, kierunek lotu, obra¿enia i penetracjê
    Projectile(sf::Vector2f pos, sf::Vector2f dir, float dmg, int pen);

    void update(float dt) override;
    bool isOffScreen() const; // Musi byæ zadeklarowane tutaj!

    bool canHit(void* enemyPtr);
    void registerHit(void* enemyPtr);

    int getPenetration() const { return currentPenetration; }
    float getDamage() const { return damage; }
    bool isExpired() const { return currentPenetration <= 0; }

private:
    float damage;
    int currentPenetration;
    std::set<void*> hitEnemies;
};