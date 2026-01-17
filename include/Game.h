#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "Constants.h"
#include "GUIManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"

class Game {
public:
    Game();
    ~Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void resizeView(const sf::RenderWindow& window, sf::View& view);

    void spawnEnemy(int hp);
    void checkCollisions();
    void restartGame();

    sf::RenderWindow window;
    sf::View gameView;
    GUIManager guiManager;
    std::unique_ptr<Player> player;
    sf::Clock clock;

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    float shootTimer = 0.f;
    float spawnTimer;
    float gameTime;
    bool isGameOver;

    Enemy* findClosestEnemy();
    sf::Vector2f rotateVector(sf::Vector2f v, float angleDegrees);
    bool checkCircleCollision(Entity* e1, Entity* e2);
};