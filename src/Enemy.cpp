#include "Enemy.h"
#include "Constants.h"
#include <cmath> // do sqrt

Enemy::Enemy(float startX, float startY, int hp) : health(hp) {
    shape.setRadius(Config::ENEMY_RADIUS);
    shape.setFillColor(sf::Color::Red);
    shape.setOrigin(Config::ENEMY_RADIUS, Config::ENEMY_RADIUS);
    setPosition(startX, startY);

    // Mo¿emy ustawiæ prêdkoœæ tutaj lub braæ ze sta³ych w update
}

void Enemy::applyKnockback(sf::Vector2f dir, float force) {
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len != 0) knockbackVel = (dir / len) * force;
}

void Enemy::update(float dt, sf::Vector2f targetPos) {
    sf::Vector2f dir = targetPos - getPosition();
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    sf::Vector2f moveVec(0.f, 0.f);
    if (len != 0) moveVec = (dir / len) * Config::ENEMY_SPEED;

    move((moveVec + knockbackVel) * dt);
    knockbackVel *= 0.9f; // Wygasanie odrzutu
}