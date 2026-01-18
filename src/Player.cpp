#include "Player.h"
#include <cmath>

Player::Player() {
    level = 1;
    currentXP = 0;
    maxXP = static_cast<int>(Config::BASE_XP_REQUIRED);

    maxHp = 100.f;
    currentHp = maxHp;
    moveSpeed = 1.0f;
    damage = 1.0f;
    projectileCount = 1;
    penetration = 1;
    pickupRange = Config::BASE_PICKUP_RANGE;
    attackSpeed = Config::BASE_ATK_SPD;
    armor = Config::BASE_ARMOR;
    regen = Config::BASE_REGEN;
    projectileKnockback = Config::BASE_PROJECTILE_KNCK;
    currentRadius = Config::PLAYER_RADIUS;

    shape.setRadius(Config::PLAYER_RADIUS);
    shape.setFillColor(sf::Color(150, 50, 250));
    shape.setOrigin(Config::PLAYER_RADIUS, Config::PLAYER_RADIUS);
    setPosition(Config::LOGICAL_WIDTH / 2.f, Config::LOGICAL_HEIGHT / 2.f);
}

bool Player::addXP(int amount) {
    currentXP += amount;
    if (currentXP >= maxXP) {
        levelUp();
        return true;
    }
    return false;
}

void Player::levelUp() {
    level++;

    // Przenosimy nadmiar XP, aby nic siê nie zmarnowa³o
    currentXP -= maxXP;
    if (currentXP < 0) currentXP = 0;

    // Obliczamy nowy próg punktowy
    // Przyk³ad: Poziom 2 (100 XP) -> Poziom 3 (100 * 1.15 + 40 = 155 XP)
    maxXP = static_cast<int>(maxXP * Config::XP_EXPONENTIAL_GROWTH) + Config::XP_LINEAR_GROWTH;
}

void Player::update(float dt) {
    if (invulnTimer > 0.f) {
        invulnTimer -= dt;
        // Efekt migania
        shape.setFillColor((static_cast<int>(invulnTimer * 10) % 2 == 0) ? sf::Color::White : sf::Color(150, 50, 250));
    }
    else {
        shape.setFillColor(sf::Color(150, 50, 250));
    }
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) movement.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) movement.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) movement.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) movement.x += 1.f;

    if (movement.x != 0 || movement.y != 0) {
        // Normalizacja prêdkoœci ukoœnej
        float len = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        velocity = (movement / len) * (Config::PLAYER_SPEED * moveSpeed);
        move(velocity * dt);
    }
    if (currentHp < maxHp) {
        currentHp += regen * dt;
        if (currentHp > maxHp) currentHp = maxHp;
    }
}

void Player::handleInput(bool blockInput) { /* kod wejscia */ }

void Player::takeDamage(float amount) {
    // Mechanika UNIKU
    if ((rand() % 100) / 100.f < dodgeChance) return;

    if (invulnTimer <= 0.f) {
        float reducedDamage = amount * (10.0f / (10.0f + armor));
        currentHp -= reducedDamage;
        invulnTimer = Config::INVULNERABILITY_TIME;
    }
}