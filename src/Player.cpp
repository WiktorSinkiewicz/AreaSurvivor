#include "Player.h"
#include <cmath>

Player::Player() {
    level = 1; currentXP = 0;
    maxXP = static_cast<int>(Config::BASE_XP_REQUIRED);
    maxHp = 100.f; currentHp = maxHp;
    moveSpeed = 1.0f; damage = 1.0f;
    projectileCount = 1; penetration = 1;
    pickupRange = Config::BASE_PICKUP_RANGE;
    attackSpeed = Config::BASE_ATK_SPD;
    armor = Config::BASE_ARMOR;
    regen = Config::BASE_REGEN;
    projectileKnockback = Config::BASE_PROJECTILE_KNCK;
    currentRadius = Config::PLAYER_RADIUS;

    // Inicjalizacja mechanik rzadkich
    multiShotLevel = 0; coneShotLevel = 0; ricochetLevel = 0;
    thornLevel = 0; vampireLevel = 0; freezeLevel = 0;
    auraLevel = 0; ramLevel = 0;
    freezeChance = 0.0f; auraDamageMult = 0.0f; auraRadiusMult = 0.0f;
    flatDamageReduction = 0.0f; collisionDamageMult = 0.0f;
    dodgeChance = 0.0f; critMultiplier = 2.0f;

    shape.setRadius(Config::PLAYER_RADIUS);
    shape.setFillColor(sf::Color(150, 50, 250));
    shape.setOrigin(Config::PLAYER_RADIUS, Config::PLAYER_RADIUS);
    setPosition(Config::LOGICAL_WIDTH / 2.f, Config::LOGICAL_HEIGHT / 2.f);

    auraShape.setFillColor(sf::Color(150, 50, 250, 40));
    auraShape.setOutlineThickness(2.f);
    auraShape.setOutlineColor(sf::Color(150, 150, 255, 80));
}

void Player::update(float dt) {
    if (invulnTimer > 0.f) {
        invulnTimer -= dt;
        shape.setFillColor((static_cast<int>(invulnTimer * 10) % 2 == 0) ? sf::Color::White : sf::Color(150, 50, 250));
    }
    else shape.setFillColor(sf::Color(150, 50, 250));

    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) movement.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) movement.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) movement.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) movement.x += 1.f;

    if (movement.x != 0 || movement.y != 0) {
        float len = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        velocity = (movement / len) * (Config::PLAYER_SPEED * moveSpeed);
        move(velocity * dt);
    }
    if (currentHp < maxHp) {
        currentHp += regen * dt;
        if (currentHp > maxHp) currentHp = maxHp;
    }
    if (auraLevel > 0) {
        // Zasiêg aury zale¿y od zasiêgu podnoszenia
        float currentRadius = pickupRange * 0.5f + 50;
        auraShape.setRadius(currentRadius);
        auraShape.setOrigin(currentRadius, currentRadius);
        auraShape.setPosition(shape.getPosition());
    }
}

void Player::takeDamage(float amount) {
    if ((static_cast<float>(rand() % 100) / 100.f) < dodgeChance) return;
    if (invulnTimer <= 0.f) {
        float reducedDamage = amount * (10.0f / (10.0f + armor));
        currentHp -= reducedDamage;
        invulnTimer = Config::INVULNERABILITY_TIME;
    }
}

bool Player::addXP(int amount) {
    currentXP += amount;
    if (currentXP >= maxXP) { levelUp(); return true; }
    return false;
}

void Player::levelUp() {
    level++;
    currentXP -= maxXP; if (currentXP < 0) currentXP = 0;
    maxXP = static_cast<int>(maxXP * Config::XP_EXPONENTIAL_GROWTH) + Config::XP_LINEAR_GROWTH;
}