#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Projectile.h"
#include "Enemy.h"
#include "Player.h"

class CombatSystem {
public:
    void update(float dt, Player& player, std::vector<std::unique_ptr<Enemy>>& enemies);
    void checkCollisions(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies);
    void draw(sf::RenderWindow& window);
    void addDamageText(sf::Vector2f pos, float dmg, bool isCrit);
    sf::CircleShape auraVisual;


private:
    struct DamageText {
        sf::Text text;
        float lifetime;
        sf::Vector2f velocity;
        sf::Color startColor;
    };

    struct PendingEcho {
        float delayTimer;
        int remainingBursts;
        sf::Vector2f direction;
    };

    float shootTimer = 0.f;
    float auraTimer = 0.f;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<DamageText> damageTexts;
    std::vector<PendingEcho> echoQueue;

    Enemy* findClosestEnemy(sf::Vector2f pos, std::vector<std::unique_ptr<Enemy>>& enemies, Enemy* exclude = nullptr);
    sf::Vector2f rotateVector(sf::Vector2f v, float angle);
    bool checkCircleCollision(sf::Vector2f p1, sf::Vector2f p2, float r1, float r2);
};