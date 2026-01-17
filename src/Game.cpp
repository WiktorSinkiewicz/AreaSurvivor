#include "Game.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

Game::Game() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    window.create(sf::VideoMode(Config::LOGICAL_WIDTH, Config::LOGICAL_HEIGHT),
        "Arena Survivor", sf::Style::Default);
    window.setFramerateLimit(Config::FRAME_LIMIT);

    gameView.setSize(Config::LOGICAL_WIDTH, Config::LOGICAL_HEIGHT);
    gameView.setCenter(Config::LOGICAL_WIDTH / 2.f, Config::LOGICAL_HEIGHT / 2.f);

    guiManager.init(window);

    restartGame(); // ustawia zmienne pocz¹tkowe
}

Game::~Game() {
    guiManager.shutdown();
}

void Game::restartGame() {
    player = std::make_unique<Player>();
    enemies.clear();
    spawnTimer = 0.f;
    gameTime = 0.f;
    isGameOver = false;
}

void Game::run() {
    while (window.isOpen()) {
        processEvents();
        sf::Time deltaTime = clock.restart();
        float dt = deltaTime.asSeconds();
        if (dt > 0.1f) dt = 0.1f;
        update(dt);
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        guiManager.processEvent(window, event);

        if (event.type == sf::Event::Closed)
            window.close();

        // Zmiana rozmiaru okna
        else if (event.type == sf::Event::Resized) {
            resizeView(window, gameView);
        }
    }
}

// Algorytm Letterboxing (Czarne pasy)
void Game::resizeView(const sf::RenderWindow& window, sf::View& view) {
    float windowRatio = window.getSize().x / (float)window.getSize().y;
    float viewRatio = Config::LOGICAL_WIDTH / Config::LOGICAL_HEIGHT;
    float sizeX = 1;
    float sizeY = 1;
    float posX = 0;
    float posY = 0;

    bool horizontalSpacing = true;
    if (windowRatio < viewRatio)
        horizontalSpacing = false;

    // okno jest za szerokie -> czarne pasy po bokach
    if (horizontalSpacing) {
        sizeX = viewRatio / windowRatio;
        posX = (1 - sizeX) / 2.f;
    }
    // okno jest za wysokie -> czarne pasy gora/dó³
    else {
        sizeY = windowRatio / viewRatio;
        posY = (1 - sizeY) / 2.f;
    }

    view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
}

void Game::update(float dt) {
    ImGui::SFML::Update(window, sf::seconds(dt));
    guiManager.update(dt);

    if (isGameOver) return;

    gameTime += dt;
    spawnTimer += dt;
    shootTimer += dt;

    // --- 1. AUTOMATYCZNE STRZELANIE (Styl Brotato) ---
    if (shootTimer >= Config::SHOOT_INTERVAL && !enemies.empty()) {
        Enemy* target = findClosestEnemy(); // pomocnicza funkcja
        if (target) {
            sf::Vector2f baseDir = target->getPosition() - player->getPosition();
            int count = player->getProjectileCount();

            for (int i = 0; i < count; ++i) {
                float angleOffset = (i - (count - 1) / 2.0f) * 10.0f; // Rozrzut 10 stopni
                sf::Vector2f rotatedDir = rotateVector(baseDir, angleOffset);

                projectiles.push_back(std::make_unique<Projectile>(
                    player->getPosition(),
                    rotatedDir,
                    player->getDamage(),
                    player->getPenetration()
                ));
            }
            shootTimer = 0.f;
        }
    }

    // --- 2. AKTUALIZACJA POCISKÓW ---
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        (*it)->update(dt);
        if ((*it)->isOffScreen()) it = projectiles.erase(it);
        else ++it;
    }

    // --- 3. SPAWNOWANIE WROGÓW ZE SKALOWANIEM HP ---
    if (spawnTimer >= Config::SPAWN_INTERVAL) {
        int scaledHP = 1 + static_cast<int>(gameTime / 10.0f); // +1 HP co 10 sek
        spawnEnemy(scaledHP);
        spawnTimer = 0.f;
    }

    // --- 4. RESZTA LOGIKI ---
    player->handleInput(guiManager.wantsCaptureInput());
    player->update(dt);

    for (auto& enemy : enemies) {
        enemy->update(dt, player->getPosition());
    }

    checkCollisions();
}

void Game::spawnEnemy(int hp) {
    float x = 0, y = 0;
    // losuje krawedz: 0-gora, 1-prawo, 2-do³, 3-lewo
    int side = std::rand() % 4;
    float margin = 50.f; // spawnowanie nieco poza ekranem

    if (side == 0) {
        x = static_cast<float>(std::rand() % (int)Config::LOGICAL_WIDTH);
        y = -margin;
    }
    else if (side == 1) {
        x = Config::LOGICAL_WIDTH + margin;
        y = static_cast<float>(std::rand() % (int)Config::LOGICAL_HEIGHT);
    }
    else if (side == 2) {
        x = static_cast<float>(std::rand() % (int)Config::LOGICAL_WIDTH);
        y = Config::LOGICAL_HEIGHT + margin;
    }
    else {
        x = -margin;
        y = static_cast<float>(std::rand() % (int)Config::LOGICAL_HEIGHT);
    }

    enemies.push_back(std::make_unique<Enemy>(x, y, hp));
}

void Game::checkCollisions() {
    // Kolizje POCISK -> WRÓG
    for (auto& proj : projectiles) {
        for (auto it = enemies.begin(); it != enemies.end();) {
            if (proj->canHit(it->get()) && checkCircleCollision(proj.get(), it->get())) {
                (*it)->takeDamage(proj->getDamage());
                proj->registerHit(it->get()); // Zmniejsza penetracjê i dodaje do listy trafionych

                if ((*it)->isDead()) {
                    player->addXP(Config::XP_PER_KILL);
                    it = enemies.erase(it);
                }
                else {
                    ++it;
                }
            }
            else {
                ++it;
            }
        }
    }
    // Usuñ pociski z penetracj¹ <= 0
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
        [](const auto& p) { return p->isExpired(); }), projectiles.end());

    // Kolizje GRACZ -> WRÓG (Koniec gry)
    for (const auto& enemy : enemies) {
        float dX = player->getPosition().x - enemy->getPosition().x;
        float dY = player->getPosition().y - enemy->getPosition().y;
        float distSq = dX * dX + dY * dY;
        float radiusSum = Config::PLAYER_RADIUS + Config::ENEMY_RADIUS;

        if (distSq < radiusSum * radiusSum) {
            isGameOver = true;
            break;
        }
    }
}

void Game::render() {
    window.clear(sf::Color::Black);
    window.setView(gameView);

    // 1. T³o i Obiekty œwiata
    sf::RectangleShape arenaBackground(sf::Vector2f(Config::LOGICAL_WIDTH, Config::LOGICAL_HEIGHT));
    arenaBackground.setFillColor(sf::Color(35, 35, 40));
    window.draw(arenaBackground);

    for (const auto& enemy : enemies) enemy->draw(window);
    for (const auto& proj : projectiles) proj->draw(window);
    if (!isGameOver) player->draw(window);

    // 2. Interfejs ImGui
    // U¿ywamy ImGui::GetMainViewport(), aby pozycjonowaæ UI wzglêdem FAKTYCZNEGO okna, a nie widoku logicznego
    ImVec2 screenSize = ImGui::GetMainViewport()->Size;
    ImVec2 screenCenter = ImVec2(screenSize.x / 2.f, screenSize.y / 2.f);

    if (!isGameOver) {
        // --- HUD (Paski) ---
        // Pozycjonujemy na œrodku góry, niezale¿nie od rozmiaru okna
        ImGui::SetNextWindowPos(ImVec2(screenSize.x / 2.f, 20.f), ImGuiCond_Always, ImVec2(0.5f, 0.f));
        ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);

        const float barWidth = 350.f;

        // Minimalistyczny poziom
        ImGui::Text("Lvl %d", player->getLevel());

        // Pasek Zdrowia (Czerwony, bez etykiety)
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
        ImGui::ProgressBar(player->getHpProgress(), ImVec2(barWidth, 12.f), "");
        ImGui::PopStyleColor();

        // Pasek XP (Zielony, bez etykiety)
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.1f, 0.7f, 0.1f, 1.0f));
        ImGui::ProgressBar(player->getXPProgress(), ImVec2(barWidth, 8.f), "");
        ImGui::PopStyleColor();

        ImGui::End();
    }
    else {
        // --- EKRAN KONCA GRY ---
        // Œrodkujemy okno dok³adnie na œrodku okna Windows
        ImGui::SetNextWindowPos(screenCenter, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Game Over", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("KONIEC GRY");
        ImGui::Separator();
        ImGui::Text("Przetrwales: %.1f s", gameTime);
        ImGui::Text("Osiagniety poziom: %d", player->getLevel());
        ImGui::Spacing();

        if (ImGui::Button("ZAGRAJ PONOWNIE", ImVec2(200, 50))) {
            restartGame();
        }

        ImGui::End();
    }

    // Opcjonalny panel debugowania
    float fps = 1.0f / clock.restart().asSeconds();
    guiManager.renderDebugPanel(fps, player->getPosition());

    guiManager.render(window);
    window.display();
}


Enemy* Game::findClosestEnemy() {
    Enemy* closest = nullptr;
    float minDistSq = 1e9f; // Du¿a wartoœæ pocz¹tkowa

    for (auto& enemy : enemies) {
        float dx = enemy->getPosition().x - player->getPosition().x;
        float dy = enemy->getPosition().y - player->getPosition().y;
        float distSq = dx * dx + dy * dy;

        if (distSq < minDistSq) {
            minDistSq = distSq;
            closest = enemy.get();
        }
    }
    return closest;
}

sf::Vector2f Game::rotateVector(sf::Vector2f v, float angleDegrees) {
    float radians = angleDegrees * (3.14159f / 180.f);
    float cosA = std::cos(radians);
    float sinA = std::sin(radians);
    return sf::Vector2f(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
}

bool Game::checkCircleCollision(Entity* e1, Entity* e2) {
    float dx = e1->getPosition().x - e2->getPosition().x;
    float dy = e1->getPosition().y - e2->getPosition().y;
    float distSq = dx * dx + dy * dy;

    // Suma promieni pocisku i wroga
    float radiusSum = Config::BULLET_RADIUS + Config::ENEMY_RADIUS;
    return distSq < (radiusSum * radiusSum);
}