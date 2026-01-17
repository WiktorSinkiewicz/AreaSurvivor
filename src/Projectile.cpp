#include "Player.h"

Player::Player() {
    // Inicjalizacja statystyk - BEZ TEGO GRA NIE ZADZIA£A POPRAWNIE
    level = 1;
    currentXP = 0;
    maxXP = static_cast<int>(Config::BASE_XP_REQUIRED);

    maxHp = 100.f;
    currentHp = maxHp;
    moveSpeed = 1.0f; // 1.0 = 100% prêdkoœci bazowej
    damage = 1.0f;
    projectileCount = 1;
    penetration = 1;

    shape.setRadius(Config::PLAYER_RADIUS);
    shape.setFillColor(sf::Color(150, 50, 250));
    shape.setOrigin(Config::PLAYER_RADIUS, Config::PLAYER_RADIUS);
    setPosition(Config::LOGICAL_WIDTH / 2.f, Config::LOGICAL_HEIGHT / 2.f);
}

void Player::takeDamage(float amount) {
    currentHp -= amount;
    if (currentHp < 0) currentHp = 0;
}

void Player::addXP(int amount) {
    currentXP += amount;
    if (currentXP >= maxXP) levelUp();
}

void Player::levelUp() {
    level++;
    currentXP = 0;
    maxXP += 50;
    // Tutaj w przysz³oœci wywo³asz menu ulepszeñ
}

float Player::getXPProgress() const {
    return static_cast<float>(currentXP) / maxXP;
}

float Player::getHpProgress() const {
    return currentHp / maxHp;
}

void Player::update(float dt) {
    // Ruch z uwzglêdnieniem mno¿nika moveSpeed
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
}

void Player::handleInput(bool blockInput) {
    // Logika wejœcia, jeœli nie jest blokowana przez UI
}