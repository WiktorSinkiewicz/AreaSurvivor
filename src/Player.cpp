#include "Projectile.h"
#include "Constants.h"
#include <cmath>

Projectile::Projectile(sf::Vector2f pos, sf::Vector2f dir, float dmg, int pen)
    : damage(dmg), currentPenetration(pen)
{
    shape.setRadius(Config::BULLET_RADIUS);
    shape.setFillColor(sf::Color::Yellow);
    shape.setOrigin(Config::BULLET_RADIUS, Config::BULLET_RADIUS);

    setPosition(pos.x, pos.y);

    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length != 0) {
        velocity = (dir / length) * Config::BULLET_SPEED;
    }
}

void Projectile::update(float dt) {
    move(velocity * dt);
}

bool Projectile::isOffScreen() const {
    sf::Vector2f pos = getPosition();
    return (pos.x < -100 || pos.x > Config::LOGICAL_WIDTH + 100 ||
        pos.y < -100 || pos.y > Config::LOGICAL_HEIGHT + 100);
}

void Projectile::registerHit(void* enemyPtr) {
    hitEnemies.insert(enemyPtr);
    currentPenetration--;
}

bool Projectile::canHit(void* enemyPtr) {
    return hitEnemies.find(enemyPtr) == hitEnemies.end();
}