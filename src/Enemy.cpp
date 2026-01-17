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

void Enemy::update(float dt, sf::Vector2f targetPos) {
    sf::Vector2f myPos = getPosition();
    sf::Vector2f direction = targetPos - myPos;

    // Obliczamy d³ugoœæ wektora (dystans)
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    // Normalizacja (¿eby wróg nie teleportowa³ siê do gracza, tylko szed³ z sta³¹ prêdkoœci¹)
    if (length > 0) {
        direction /= length;
        // Ruch: kierunek * prêdkoœæ * czas
        move(direction * Config::ENEMY_SPEED * dt);
    }
}