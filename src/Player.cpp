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
    // Przenosimy nadmiar XP na kolejny poziom zamiast zerowania
    currentXP -= maxXP;
    if (currentXP < 0) currentXP = 0; // Zabezpieczenie przed b³êdami

    // Mocniejsze skalowanie: ka¿dy poziom wymaga o 20% wiêcej punktów ni¿ poprzedni
    maxXP = static_cast<int>(maxXP * 1.2f) + 50;
}

float Player::getXPProgress() const {
    return static_cast<float>(currentXP) / maxXP;
}

float Player::getHpProgress() const {
    return currentHp / maxHp;
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
}

void Player::handleInput(bool blockInput) {
    // Miejsce na dodatkow¹ logikê sterowania
}

void Player::takeDamage(float amount) {
    if (invulnTimer <= 0.f) {
        currentHp -= amount;
        invulnTimer = Config::INVULNERABILITY_TIME;
        if (currentHp < 0.f) currentHp = 0.f; // Zabezpieczenie przed ujemnym HP
    }
}