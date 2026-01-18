#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "Constants.h"
#include "GUIManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
#include "ExperienceOrb.h"

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

    void updateCombat(float dt);
    void updateSpawning(float dt);
    void updateExperience(float dt);
    void updateEntities(float dt);

    void drawWorld();
    void drawUI(ImVec2 screenCenter);

    void checkAllCollisions();
    void resolveEnemyCollisions();
    void spawnEnemy(int hp);
    void resizeView(const sf::RenderWindow& window, sf::View& view);

    Enemy* findClosestEnemy();
    sf::Vector2f rotateVector(sf::Vector2f v, float angleDegrees);
    bool checkCircleCollision(Entity* e1, Entity* e2, float r1, float r2);

    enum class UpgradeType {
        HP, DMG, SPD, PROJ, PEN, PICKUP, ATK_SPD, ARMOR, REGEN, KNCK,
        RARE_MULTI, RARE_RICO, RARE_CONE, RARE_THORN, RARE_GIANT, RARE_GHOST // Dodano to
    };
    struct UpgradeOption {
        UpgradeType type;
        std::string title;
        std::string description;
    };

    sf::RenderWindow window;
    sf::View gameView;
    GUIManager guiManager;
    sf::Clock clock;
    std::unique_ptr<Player> player;

    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<std::unique_ptr<ExperienceOrb>> expOrbs;

    float shootTimer = 0.f;
    float spawnTimer = 0.f;
    float gameTime = 0.f;
    bool isGameOver = false;
    bool isUpgrading = false;
    std::vector<UpgradeOption> availableUpgrades;
    struct PendingEcho {
        float delayTimer;
        int remainingBursts;
        sf::Vector2f direction;
    };
    std::vector<PendingEcho> echoQueue;

    void generateUpgrades();
    void applyUpgrade(UpgradeType type);
};