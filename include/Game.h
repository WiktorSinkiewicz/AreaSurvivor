#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "Constants.h"
#include "GUIManager.h"
#include "Player.h"
#include "Enemy.h"
#include "ExperienceOrb.h"
#include "CombatSystem.h"
#include "UpgradeManager.h"

class Game {
public:
    Game();
    ~Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void restartGame();
    void updateSpawning(float dt);
    void updateExperience(float dt);
    void spawnEnemy(int dummy);
    void resizeView(const sf::RenderWindow& window, sf::View& view);
    void drawWorld();
    void drawUI(ImVec2 screenCenter);
    bool checkCircleCollision(sf::Vector2f p1, sf::Vector2f p2, float r1, float r2);
    void resolveEnemyCollisions();

    UpgradeManager upgradeManager;
    CombatSystem combatSystem;
    GUIManager guiManager;

    sf::RenderWindow window;
    sf::View gameView;
    sf::Clock clock;

    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<ExperienceOrb>> expOrbs;
    std::vector<Upgrade> currentOptions;

    float gameTime = 0.f;
    float spawnTimer = 0.f;
    bool isGameOver = false;
    bool isUpgrading = false;
};