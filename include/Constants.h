#pragma once
#include <SFML/Graphics.hpp>

namespace Config {

    const float LOGICAL_WIDTH = 1280.f;
    const float LOGICAL_HEIGHT = 720.f;
    const unsigned int FRAME_LIMIT = 144;
    const int MAX_ENEMIES_LIMIT = 400;
    const sf::Color COLOR_ARENA = sf::Color(30, 30, 30);

    const float PLAYER_RADIUS = 25.f;
    const float PLAYER_SPEED = 200.f;
    const float PLAYER_SPEED_MAX = 550.f;
    const float BASE_HP_START = 100.f;
    const float INVULNERABILITY_TIME = 0.25f;
    const float BASE_ARMOR = 0.0f;
    const float BASE_REGEN = 0.0f;
    const float BASE_PICKUP_RANGE = 100.f;

    const float BULLET_RADIUS = 5.f;
    const float BULLET_SPEED = 600.f;
    const float SHOOT_INTERVAL = 1.0f;
    const float PROJECTILE_SPREAD = 5.0f;
    const float BASE_ATK_SPD = 1.0f;
    const float BASE_PROJECTILE_KNCK = 80.f;

    const float EXP_ORB_RADIUS = 6.f;
    const float ORB_SPEED = 600.f;
    const float XP_ORB_BASE_VALUE = 50.f;
    const float XP_ORB_VALUE_GROWTH = 1.2f;
    const float BASE_XP_REQUIRED = 100.f;
    const float XP_EXPONENTIAL_GROWTH = 1.04f;
    const int   XP_LINEAR_GROWTH = 40;

    const float ENEMY_RADIUS = 20.f;
    const float SPAWN_INTERVAL_START = 0.8f;
    const float SPAWN_INTERVAL_MIN = 0.05f;
    const float SPAWN_REDUCTION_RATE = 0.10f;

    const float E_HP_BASE = 1.0f;
    const float E_HP_GROWTH = 1.4f;
    const float E_DMG_BASE = 10.f;
    const float E_DMG_GROWTH = 1.2f;
    const float E_SPD_BASE = 70.f;
    const float E_SPD_GROWTH = 1.04f;

    const float BOSS_INTERVAL = 300.f;
    const float ELITE_CHANCE_BASE = 0.05f;
    const float ELITE_CHANCE_GROWTH = 0.02f;
    const float ELITE_STAT_MULT = 4.0f;
    const float UPGRADE_ARMOR_VAL = 2.0f;
    const float UPGRADE_HP_VAL = 50.f;
    const float UPGRADE_DMG_VAL = 1.0f;
    const float UPGRADE_PICKUP_VAL = 40.f;
    const float UPGRADE_SPD_VAL = 0.10f;
    const float UPGRADE_ATK_SPD_VAL = 0.10f;

    const float RARE_RAM_SPD_COEFF = 80.f;
    const float RARE_RAM_HP_COEFF = 0.20f;
    const float RARE_THORN_SCALING = 3.0f;
    const float FREEZE_IMMUNITY_MULT = 1.5f;

    const sf::Color COLOR_ENEMY = sf::Color(255, 0, 0);
    const sf::Color COLOR_FROZEN = sf::Color(0, 191, 255);
    const sf::Color COLOR_ORB = sf::Color(50, 150, 255);

}