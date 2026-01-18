#include "Game.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm> // Dla std::shuffle
#include <random>    // Dla std::mt19937

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

        if (event.type == sf::Event::Closed) window.close();

        // Obs³uga nawigacji TYLKO gdy okno ulepszeñ jest otwarte
        if (isUpgrading && event.type == sf::Event::KeyPressed) {
            ImGuiIO& io = ImGui::GetIO();
            if (event.key.code == sf::Keyboard::W) io.AddKeyEvent(ImGuiKey_UpArrow, true);
            else if (event.key.code == sf::Keyboard::S) io.AddKeyEvent(ImGuiKey_DownArrow, true);
            else if (event.key.code == sf::Keyboard::Space) io.AddKeyEvent(ImGuiKey_Space, true);
        }

        // Resetowanie stanu klawiszy po puszczeniu (wa¿ne dla czystoœci sygna³u)
        if (isUpgrading && event.type == sf::Event::KeyReleased) {
            ImGuiIO& io = ImGui::GetIO();
            if (event.key.code == sf::Keyboard::W) io.AddKeyEvent(ImGuiKey_UpArrow, false);
            else if (event.key.code == sf::Keyboard::S) io.AddKeyEvent(ImGuiKey_DownArrow, false);
            else if (event.key.code == sf::Keyboard::Space) io.AddKeyEvent(ImGuiKey_Space, false);
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

    if (isGameOver || isUpgrading) return; // Pauza logiki œwiata
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

    // --- AKTUALIZACJA KULEK EXP ---
    for (auto it = expOrbs.begin(); it != expOrbs.end();) {
        sf::Vector2f orbPos = (*it)->getPosition();
        sf::Vector2f playerPos = player->getPosition();
        float dx = playerPos.x - orbPos.x;
        float dy = playerPos.y - orbPos.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        // 1. Sprawdzamy zasiêg podnoszenia (Magnet)
        if (dist < player->getPickupRange()) {
            sf::Vector2f dir = playerPos - orbPos;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0) {
                (*it)->move((dir / len) * Config::ORB_SPEED * dt);
            }
        }

        // 2. Fizyczny kontakt (Zebranie)
        if (dist < Config::PLAYER_RADIUS + Config::EXP_ORB_RADIUS) {
            if (player->addXP((*it)->getXpAmount())) {
                isUpgrading = true;
                generateUpgrades();
            }
            it = expOrbs.erase(it);
        }
        else {
            ++it;
        }
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
    resolveEnemyCollisions();
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
    // 1. Kolizje POCISK -> WRÓG
    for (auto& proj : projectiles) {
        for (auto it = enemies.begin(); it != enemies.end();) {
            if (proj->canHit(it->get()) && checkCircleCollision(proj.get(), it->get(), Config::BULLET_RADIUS, Config::ENEMY_RADIUS)) {

                (*it)->takeDamage(proj->getDamage());
                proj->registerHit(it->get());

                if ((*it)->isDead()) {
                    expOrbs.push_back(std::make_unique<ExperienceOrb>((*it)->getPosition(), Config::XP_PER_KILL));
                    it = enemies.erase(it); // Usuwamy tylko gdy martwy
                }
                else {
                    ++it; // Jeœli ¿yje, idziemy dalej
                }

                if (proj->isExpired()) break;
            }
            else {
                ++it;
            }
        }
    }

    // Usuwanie zu¿ytych pocisków
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
        [](const auto& p) { return p->isExpired(); }), projectiles.end());

    // 2. Kolizje GRACZ -> WRÓG
    float timeFactor = gameTime / 10.f;
    float currentEnemyDmg = Config::ENEMY_BASE_DMG + (timeFactor * Config::ENEMY_DMG_SCALING);

    for (auto& enemy : enemies) {
        if (checkCircleCollision(player.get(), enemy.get(), Config::PLAYER_RADIUS, Config::ENEMY_RADIUS)) {
            player->takeDamage(currentEnemyDmg);

            if (player->getCurrentHp() <= 0.f) {
                isGameOver = true;
            }

            sf::Vector2f pushDir = enemy->getPosition() - player->getPosition();
            enemy->applyKnockback(pushDir, Config::KNOCKBACK_FORCE);
        }
    }
}

void Game::render() {
    window.clear(sf::Color::Black);
    window.setView(gameView);

    // 1. Rysowanie œwiata (Arena, Wrogowie, Pociski, Gracz)
    sf::RectangleShape arenaBackground(sf::Vector2f(Config::LOGICAL_WIDTH, Config::LOGICAL_HEIGHT));
    arenaBackground.setFillColor(sf::Color(35, 35, 40));
    window.draw(arenaBackground);

    for (const auto& enemy : enemies) enemy->draw(window);
    for (const auto& proj : projectiles) proj->draw(window);
    if (!isGameOver) player->draw(window);
    for (const auto& orb : expOrbs) orb->draw(window);

    // 2. Przygotowanie danych do UI
    ImVec2 screenSize = ImGui::GetMainViewport()->Size;
    // FIX C2065: Tutaj definiujemy niezidentyfikowany wczeœniej screenCenter
    ImVec2 screenCenter = ImVec2(screenSize.x / 2.f, screenSize.y / 2.f);

    if (!isGameOver) {
        // 1. Pozycjonowanie okna HUD na œrodku góry
        ImGui::SetNextWindowPos(ImVec2(screenSize.x / 2.f, 20.f), ImGuiCond_Always, ImVec2(0.5f, 0.f));
        // Dodajemy flagê NoInputs, aby HUD nie blokowa³ myszki podczas walki
        ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);

        const float barW = 450.f; // Nieco szersze paski, by napisy siê mieœci³y
        const float hpHeight = 30.f;
        const float xpHeight = 20.f;

        // --- PASEK HP ---
        ImVec2 hpPos = ImGui::GetCursorPos(); // Zapamiêtujemy pozycjê startow¹ dla tekstu
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        // Rysujemy pasek bez tekstu w œrodku (pusty string "")
        ImGui::ProgressBar(player->getHpProgress(), ImVec2(barW, hpHeight), "");
        ImGui::PopStyleColor();

        // Rêczne wyœrodkowanie tekstu HP nad paskiem
        char hpBuf[64];
        sprintf(hpBuf, "HP: %.0f / %.0f", player->getCurrentHp(), player->getMaxHp());
        ImVec2 hpTextSize = ImGui::CalcTextSize(hpBuf);
        // Ustawiamy kursor na œrodku paska HP
        ImGui::SetCursorPos(ImVec2(hpPos.x + (barW - hpTextSize.x) / 2.f, hpPos.y + (hpHeight - hpTextSize.y) / 2.f));
        ImGui::Text("%s", hpBuf);

        ImGui::Spacing(); // Odstêp miêdzy paskami

        // --- PASEK XP ---
        ImVec2 xpPos = ImGui::GetCursorPos(); // Zapamiêtujemy pozycjê startow¹
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::ProgressBar(player->getXPProgress(), ImVec2(barW, xpHeight), "");
        ImGui::PopStyleColor();

        // Rêczne wyœrodkowanie tekstu XP nad paskiem
        char expBuf[64];
        sprintf(expBuf, "XP: %d / %d", player->getCurrentXp(), player->getMaxXp());
        ImVec2 xpTextSize = ImGui::CalcTextSize(expBuf);
        // Ustawiamy kursor na œrodku paska XP
        ImGui::SetCursorPos(ImVec2(xpPos.x + (barW - xpTextSize.x) / 2.f, xpPos.y + (xpHeight - xpTextSize.y) / 2.f));
        ImGui::Text("%s", expBuf);

        // Informacja o poziomie pod paskami
        ImGui::SetCursorPosX((barW - ImGui::CalcTextSize("Poziom: X").x) / 2.f);
        ImGui::Text("Poziom: %d", player->getLevel());

        ImGui::End();

        // --- OKNO ULEPSZEÑ ---
        if (isUpgrading) {
            ImGui::SetNextWindowFocus(); // Skupienie dla nawigacji klawiatur¹
            ImGui::SetNextWindowPos(screenCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::Begin("UPGRADE", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

            for (auto& opt : availableUpgrades) {
                if (ImGui::Button(opt.title.c_str(), ImVec2(250, 40))) {
                    applyUpgrade(opt.type);
                }
            }
            ImGui::End();
        }
    }
    else {
        // --- EKRAN GAME OVER ---
        ImGui::SetNextWindowPos(screenCenter, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Koniec Gry", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("ZGINALES!");
        ImGui::Text("Czas: %.1f s", gameTime);
        if (ImGui::Button("ZAGRAJ PONOWNIE", ImVec2(200, 50))) restartGame();
        ImGui::End();
    }

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

bool Game::checkCircleCollision(Entity* e1, Entity* e2, float r1, float r2) {
    float dx = e1->getPosition().x - e2->getPosition().x;
    float dy = e1->getPosition().y - e2->getPosition().y;
    return (dx * dx + dy * dy) < (r1 + r2) * (r1 + r2);
}

void Game::generateUpgrades() {
    availableUpgrades.clear();
    std::vector<UpgradeOption> pool = {
        { UpgradeType::HP, "Witalnosc (HP+)", "Wiecej HP" },
        { UpgradeType::DMG, "Sila (DMG+)", "Wiekszy dmg" },
        { UpgradeType::SPD, "Zrecznosc (SPD+)", "Szybszy ruch" },
        { UpgradeType::PROJ, "Arsenal (PROJ+)", "+1 pocisk" },
        { UpgradeType::PEN, "Przebicie (PEN+)", "+1 przebicie" },
        { UpgradeType::PICK, "Magnes (MGN+)", "+zasieg zbierania" }
    };

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(pool.begin(), pool.end(), g); // Naprawa b³êdu C2039

    for (int i = 0; i < 3 && i < pool.size(); i++) {
        availableUpgrades.push_back(pool[i]);
    }
}

void Game::applyUpgrade(UpgradeType type) {
    switch (type) {
    case UpgradeType::HP: player->upgradeHp(); break;
    case UpgradeType::DMG: player->upgradeDmg(); break;
    case UpgradeType::SPD: player->upgradeSpd(); break;
    case UpgradeType::PROJ: player->upgradeProj(); break;
    case UpgradeType::PEN: player->upgradePen(); break;
    case UpgradeType::PICK: player->upgradePickupRange(); break;
    }
    isUpgrading = false; // Powrót do gry
}

void Game::resolveEnemyCollisions() {
    if (enemies.size() < 2) return;

    float radius = Config::ENEMY_RADIUS;
    float minDistance = radius * 2.f;

    for (size_t i = 0; i < enemies.size(); ++i) {
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            sf::Vector2f posA = enemies[i]->getPosition();
            sf::Vector2f posB = enemies[j]->getPosition();

            float dx = posA.x - posB.x;
            float dy = posA.y - posB.y;
            float distSq = dx * dx + dy * dy;

            // Sprawdzamy dystans do kwadratu (optymalizacja - brak sqrt)
            if (distSq < minDistance * minDistance && distSq > 0.f) {
                float dist = std::sqrt(distSq);
                float overlap = minDistance - dist;

                // Kierunek wypchniêcia (znormalizowany wektor ró¿nicy)
                sf::Vector2f pushDir = (sf::Vector2f(dx, dy) / dist);

                // Przesuwamy obu wrogów w przeciwnych kierunkach o po³owê overlapu
                // Mno¿nik 0.5f sprawia, ¿e separacja jest p³ynna
                sf::Vector2f moveVec = pushDir * overlap * 0.5f;

                enemies[i]->move(moveVec);
                enemies[j]->move(-moveVec);
            }
        }
    }
}