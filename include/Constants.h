#pragma once
#include <SFML/Graphics.hpp>

namespace Config {
    // --- OKNO I WIDOK ---
    const float LOGICAL_WIDTH = 1280.f;
    const float LOGICAL_HEIGHT = 720.f;
    const unsigned int FRAME_LIMIT = 144;

    // --- GRACZ ---
    const float PLAYER_RADIUS = 25.f;
    const float PLAYER_SPEED = 200.f;
    const float INVULNERABILITY_TIME = 0.2f;
    const float BASE_PICKUP_RANGE = 100.f;

    // --- WALKA ---
    const float BULLET_RADIUS = 5.f;
    const float BULLET_SPEED = 500.f;
    const float SHOOT_INTERVAL = 1.f;
    const float PROJECTILE_SPREAD = 10.0f;
    const float KNOCKBACK_FORCE = 150.f;

    // --- STATYSTYKI BAZOWE ---
    const float BASE_ATK_SPD = 1.0f;
    const float BASE_ARMOR = 0.0f;
    const float BASE_REGEN = 0.0f;
    const float BASE_PROJECTILE_KNCK = 50.f; // Bazowa si³a odepchniêcia pociskiem

    // --- WROGOWIE I SKALOWANIE (CO 10 SEKUND) ---
    const float ENEMY_RADIUS = 20.f;
    const float ENEMY_SPEED = 80.f;

    const float SPAWN_INTERVAL_START = 2.0f; // Pocz¹tkowo co 2 sekundy
    const float SPAWN_INTERVAL_MIN = 0.2f;   // Nigdy nie czêœciej ni¿ co 0.2s
    const float SPAWN_REDUCTION_PER_10S = 0.05f; // Co 10 sekund interwa³ skraca siê o 0.1s

    const float ENEMY_BASE_HP = 1.f;
    const float ENEMY_HP_ADD = 0.5f;
    const float ENEMY_HP_PERC = 0.1f;

    const float ENEMY_BASE_DMG = 10.f; // TYLKO JEDNA DEFINICJA JAKO FLOAT
    const float ENEMY_DMG_ADD = 0.5f;
    const float ENEMY_DMG_PERC = 0.05f;

    const float ENEMY_BASE_SPEED = 70.f;
    const float ENEMY_SPEED_ADD = 2.f;

    // --- XP I PROGRESJA ---
    const float XP_PER_KILL_BASE = 25.f;
    const float XP_DROP_SCALING = 5.0f;
    const float BASE_XP_REQUIRED = 100.f;
    const float XP_EXPONENTIAL_GROWTH = 1.05f;
    const int XP_LINEAR_GROWTH = 10;

    // --- KULKI I ULEPSZENIA ---
    const float EXP_ORB_RADIUS = 6.f;
    const float ORB_SPEED = 600.f;
    const float UPGRADE_HP_VAL = 20.f;
    const float UPGRADE_DMG_VAL = 1.f;
    const float UPGRADE_SPD_VAL = 0.2f;
    const float UPGRADE_PICKUP_VAL = 40.f;
    const float UPGRADE_ATK_SPD_VAL = 0.3f;
    const float UPGRADE_ARMOR_VAL = 2.0f;
    const float UPGRADE_REGEN_VAL = 1.f;
    const float UPGRADE_KNCK_VAL = 200.f;

    // --- BALANS RZADKICH ULEPSZEÑ ---
    const float RARE_MULTI_SHOT_DELAY = 0.15f; // OpóŸnienie drugiego strza³u
    const float RARE_RICOCHET_RADIUS = 250.f;  // Zasiêg szukania celu po odbiciu
    const float RARE_THORN_SCALING = 2.0f;     // Mno¿nik pancerza w obra¿eniach cierniowych
    const float RARE_GIANT_HP_MULT = 1.25f;    // +25% Max HP
    const float RARE_DODGE_CHANCE = 0.15f;     // 15% szansy na unik
    const float RARE_GHOST_SPD_BOOST = 1.3f;   // +30% szybkoœci po ulepszeniu Ghost

    // --- KOLORY ---
    const sf::Color COLOR_ARENA = sf::Color(35, 35, 40);
    const sf::Color COLOR_PLAYER = sf::Color(150, 50, 250);
    const sf::Color COLOR_ENEMY = sf::Color::Red;
    const sf::Color COLOR_ORB = sf::Color(50, 150, 255);
}