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
}