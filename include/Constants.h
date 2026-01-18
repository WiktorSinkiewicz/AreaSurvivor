#pragma once

namespace Config {
    const float LOGICAL_WIDTH = 1280.f;
    const float LOGICAL_HEIGHT = 720.f;

    const float PLAYER_RADIUS = 25.f;
    const float PLAYER_SPEED = 200.f;
    const unsigned int FRAME_LIMIT = 144;

    // Konfiguracja wrogów
    const float ENEMY_RADIUS = 20.f;
    const float ENEMY_SPEED = 50.f;
    const float SPAWN_INTERVAL = 1.0f;   // Co ile sekund
    // pociski
    const float BULLET_RADIUS = 5.f;
    const float BULLET_SPEED = 400.f;
    const float SHOOT_INTERVAL = 0.5f; // Strza³ co pó³ sekundy
    const float XP_PER_KILL = 25.f;
    const float BASE_XP_REQUIRED = 100.f;
	// Ulepszenia
    const float UPGRADE_HP_VAL = 20.f;
    const float UPGRADE_DMG_VAL = 0.2f;   // +20% obra¿eñ
    const float UPGRADE_SPD_VAL = 0.1f;   // +10% prêdkoœci
    const int UPGRADE_PROJ_VAL = 1;       // +1 pocisk
    const int UPGRADE_PEN_VAL = 1;        // +1 przebicie

    const float INVULNERABILITY_TIME = 0.5f; // Pó³ sekundy niewra¿liwoœci
    const float KNOCKBACK_FORCE = 500.f;    // Si³a odrzutu

    // Skalowanie wrogów
    const float ENEMY_HP_SCALING = 1.0f;    // +1 HP co 10s
    const float ENEMY_DMG_SCALING = 1.0f;   // +1 DMG co 10s
    const float ENEMY_SPEED_SCALING = 5.0f; // +5 speed co 10s
    const int ENEMY_BASE_DMG = 5;

    const float EXP_ORB_RADIUS = 6.f;
    const float BASE_PICKUP_RANGE = 100.f;
    const float UPGRADE_PICKUP_VAL = 40.f;
    const float ORB_SPEED = 500.f; // Prêdkoœæ, z jak¹ kulka leci do gracza
}