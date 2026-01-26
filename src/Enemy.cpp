#include "Enemy.h"
#include "Constants.h"
#include <cmath>

Enemy::Enemy(float startX, float startY, float hp, float speed, float dmg)
    : health(hp), speed(speed), damage(dmg)
{
    shape.setRadius(Config::ENEMY_RADIUS);
    shape.setFillColor(Config::COLOR_ENEMY);

    shape.setOrigin(Config::ENEMY_RADIUS, Config::ENEMY_RADIUS);
    setPosition(startX, startY);
    knockbackVel = sf::Vector2f(0.f, 0.f);
}


void Enemy::update(float dt, sf::Vector2f playerPos) {
    if (isFrozen) {
        freezeTimer -= dt;
        if (freezeTimer <= 0) {
            getShape().setFillColor(Config::COLOR_ENEMY);
            isFrozen = false;
        }
        return;
    }
    sf::Vector2f dir = playerPos - getPosition();
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    sf::Vector2f moveVec(0.f, 0.f);
    if (len != 0) moveVec = (dir / len) * speed;
    move((moveVec + knockbackVel) * dt);
    knockbackVel *= 0.9f;
}

void Enemy::applyKnockback(sf::Vector2f dir, float force) {
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len != 0) knockbackVel = (dir / len) * force;
}