#include "Game.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm> 
#include <random>    

// --- KONSTRUKTOR I DESTRUKTOR ---
Game::Game() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    window.create(sf::VideoMode(Config::LOGICAL_WIDTH, Config::LOGICAL_HEIGHT),
        "Arena Survivor", sf::Style::Default);
    window.setFramerateLimit(Config::FRAME_LIMIT);
    window.setKeyRepeatEnabled(false); // Zapobiega miganiu menu ulepszeñ

    gameView.setSize(Config::LOGICAL_WIDTH, Config::LOGICAL_HEIGHT);
    gameView.setCenter(Config::LOGICAL_WIDTH / 2.f, Config::LOGICAL_HEIGHT / 2.f);

    guiManager.init(window);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    restartGame();
}

Game::~Game() {
    guiManager.shutdown();
}

void Game::restartGame() {
    player = std::make_unique<Player>();
    enemies.clear();
    projectiles.clear();
    expOrbs.clear();
    spawnTimer = 0.f;
    gameTime = 0.f;
    shootTimer = 0.f;
    isGameOver = false;
    isUpgrading = false;
}

// --- G£ÓWNA PÊTLA ---
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

        if (event.type == sf::Event::Resized) resizeView(window, gameView);

        // Obs³uga nawigacji w menu ulepszeñ
        if (isUpgrading) {
            ImGuiIO& io = ImGui::GetIO();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::W) io.AddKeyEvent(ImGuiKey_UpArrow, true);
                else if (event.key.code == sf::Keyboard::S) io.AddKeyEvent(ImGuiKey_DownArrow, true);
                else if (event.key.code == sf::Keyboard::Space) io.AddKeyEvent(ImGuiKey_Space, true); // FIX C2065
            }
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::W) io.AddKeyEvent(ImGuiKey_UpArrow, false);
                else if (event.key.code == sf::Keyboard::S) io.AddKeyEvent(ImGuiKey_DownArrow, false);
                else if (event.key.code == sf::Keyboard::Space) io.AddKeyEvent(ImGuiKey_Space, false); // FIX C2065
            }
        }
    }
}

// --- SYSTEM AKTUALIZACJI (MODU£Y) ---
void Game::update(float dt) {
    ImGui::SFML::Update(window, sf::seconds(dt));
    guiManager.update(dt);

    if (isGameOver || isUpgrading) return;

    gameTime += dt;
    window.setMouseCursorVisible(isUpgrading || isGameOver);
    updateSpawning(dt);
    updateCombat(dt);
    updateExperience(dt);
    updateEntities(dt);

    resolveEnemyCollisions();
    checkAllCollisions();
}

void Game::updateSpawning(float dt) {
    spawnTimer += dt;

    // 1. Obliczamy wspó³czynnik czasu (co 10 sekund)
    float t_factor = gameTime / 10.0f;

    // 2. Obliczamy aktualny interwa³: Start - (skalowanie * czas)
    float currentSpawnInterval = Config::SPAWN_INTERVAL_START - (t_factor * Config::SPAWN_REDUCTION_PER_10S);

    // 3. Zabezpieczamy, aby interwa³ nie spad³ poni¿ej minimum
    if (currentSpawnInterval < Config::SPAWN_INTERVAL_MIN) {
        currentSpawnInterval = Config::SPAWN_INTERVAL_MIN;
    }

    // 4. Sprawdzamy timer u¿ywaj¹c dynamicznej wartoœci
    if (spawnTimer >= currentSpawnInterval) {
        spawnEnemy(0);
        spawnTimer = 0.f;
    }
}

void Game::updateCombat(float dt) {
    shootTimer += dt;
    float effectiveShootInterval = Config::SHOOT_INTERVAL / player->getAttackSpeed();

    // 1. Obs³uga kolejki Echa (strzelanie z opóŸnieniem)
    for (auto it = echoQueue.begin(); it != echoQueue.end();) {
        it->delayTimer -= dt;
        if (it->delayTimer <= 0.f) {
            // Strza³ z kolejki
            int count = player->getProjectileCount();
            float spread = (player->coneShotLevel > 0) ? (30.0f + player->coneShotLevel * 10.0f) : Config::PROJECTILE_SPREAD;

            for (int i = 0; i < count; ++i) {
                float angleOffset = (i - (count - 1) / 2.0f) * spread;
                sf::Vector2f rotatedDir = rotateVector(it->direction, angleOffset);
                projectiles.push_back(std::make_unique<Projectile>(
                    player->getPosition(), rotatedDir, player->getDamage(), player->getPenetration()));
            }

            it->remainingBursts--;
            if (it->remainingBursts <= 0) it = echoQueue.erase(it);
            else {
                it->delayTimer = 0.15f / player->getAttackSpeed(); // Czas miêdzy echami
                ++it;
            }
        }
        else ++it;
    }

    // 2. Strza³ podstawowy i dodawanie do kolejki
    if (shootTimer >= effectiveShootInterval && !enemies.empty()) {
        if (Enemy* target = findClosestEnemy()) {
            sf::Vector2f dir = target->getPosition() - player->getPosition();

            // Wystrzel pierwsz¹ seriê (podstawow¹)
            int count = player->getProjectileCount();
            float spread = (player->coneShotLevel > 0) ? (30.0f + player->coneShotLevel * 10.0f) : Config::PROJECTILE_SPREAD;
            for (int i = 0; i < count; ++i) {
                float angleOffset = (i - (count - 1) / 2.0f) * spread;
                sf::Vector2f rotatedDir = rotateVector(dir, angleOffset);
                projectiles.push_back(std::make_unique<Projectile>(
                    player->getPosition(), rotatedDir, player->getDamage(), player->getPenetration()));
            }

            // Jeœli gracz ma Echo, dodaj resztê serii do kolejki
            if (player->multiShotLevel > 0) {
                echoQueue.push_back({ 0.15f, player->multiShotLevel, dir });
            }
            shootTimer = 0.f;
        }
    }

    // Ruch pocisków (bez zmian)
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        (*it)->update(dt);
        if ((*it)->isOffScreen()) it = projectiles.erase(it);
        else ++it;
    }
}

void Game::updateExperience(float dt) {
    for (auto it = expOrbs.begin(); it != expOrbs.end();) {
        sf::Vector2f orbPos = (*it)->getPosition();
        sf::Vector2f playerPos = player->getPosition();
        float dx = playerPos.x - orbPos.x;
        float dy = playerPos.y - orbPos.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        // Magnes
        if (dist < player->getPickupRange()) {
            sf::Vector2f dir = playerPos - orbPos;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0) (*it)->move((dir / len) * Config::ORB_SPEED * dt);
        }

        // Podnoszenie
        if (dist < player->getRadius() + Config::EXP_ORB_RADIUS) {
            if (player->addXP((*it)->getXpAmount())) {
                if (!isUpgrading) {
                    isUpgrading = true;
                    generateUpgrades();
                }
            }
            it = expOrbs.erase(it);
        }
        else ++it;
    }
}

void Game::updateEntities(float dt) {
    player->update(dt);
    for (auto& enemy : enemies) {
        enemy->update(dt, player->getPosition());
    }
}

// --- FIZYKA I KOLIZJE ---
void Game::checkAllCollisions() {
    for (auto& proj : projectiles) {
        if (proj->isExpired()) continue;
        for (auto it = enemies.begin(); it != enemies.end();) {
            if (proj->canHit(it->get()) && checkCircleCollision(proj.get(), it->get(), Config::BULLET_RADIUS, Config::ENEMY_RADIUS)) {
                sf::Vector2f knockDir = (*it)->getPosition() - player->getPosition();
                (*it)->applyKnockback(knockDir, player->getProjectileKnockback());
                (*it)->takeDamage(proj->getDamage());
                proj->registerHit(it->get());

                if (player->ricochetLevel > 0 && !proj->isExpired()) {
                    Enemy* nextTarget = nullptr;
                    float minDist = Config::RARE_RICOCHET_RADIUS;
                    for (auto& p : enemies) {
                        if (p.get() == it->get() || !proj->canHit(p.get())) continue;
                        float d = std::sqrt(std::pow(p->getPosition().x - (*it)->getPosition().x, 2) + std::pow(p->getPosition().y - (*it)->getPosition().y, 2));
                        if (d < minDist) { minDist = d; nextTarget = p.get(); }
                    }
                    if (nextTarget) proj->redirect(nextTarget->getPosition() - (*it)->getPosition());
                }

                if ((*it)->isDead()) {
                    float t_factor = gameTime / 10.0f;
                    int bakedXP = static_cast<int>(Config::XP_PER_KILL_BASE + (t_factor * Config::XP_DROP_SCALING));
                    expOrbs.push_back(std::make_unique<ExperienceOrb>((*it)->getPosition(), bakedXP));
                    it = enemies.erase(it);
                }
                else ++it;

                if (proj->isExpired()) break;
            }
            else ++it;
        }
    }
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const auto& p) { return p->isExpired(); }), projectiles.end());

    for (auto& enemy : enemies) {
        if (checkCircleCollision(player.get(), enemy.get(), player->getRadius(), Config::ENEMY_RADIUS)) {
            player->takeDamage(enemy->getDamage());
            if (player->thornLevel > 0) {
                float thornDamage = (player->getDamage() * 0.5f + player->getArmor() * 2.f) * player->thornLevel;
                enemy->takeDamage(thornDamage);
            }
            if (player->getCurrentHp() <= 0.f) isGameOver = true;
            enemy->applyKnockback(enemy->getPosition() - player->getPosition(), Config::KNOCKBACK_FORCE);
        }
    }
}

void Game::resolveEnemyCollisions() {
    if (enemies.size() < 2) return;
    float minDistance = Config::ENEMY_RADIUS * 2.f;
    for (size_t i = 0; i < enemies.size(); ++i) {
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            sf::Vector2f posA = enemies[i]->getPosition();
            sf::Vector2f posB = enemies[j]->getPosition();
            float dx = posA.x - posB.x, dy = posA.y - posB.y;
            float distSq = dx * dx + dy * dy;
            if (distSq < minDistance * minDistance && distSq > 0.f) {
                float dist = std::sqrt(distSq);
                sf::Vector2f pushDir = sf::Vector2f(dx, dy) / dist;
                sf::Vector2f moveVec = pushDir * (minDistance - dist) * 0.5f;
                enemies[i]->move(moveVec);
                enemies[j]->move(-moveVec);
            }
        }
    }
}

// --- RENDEROWANIE ---
void Game::render() {
    window.clear(sf::Color::Black);
    window.setView(gameView);

    drawWorld();

    ImVec2 screenSize = ImGui::GetMainViewport()->Size;
    ImVec2 screenCenter(screenSize.x / 2.f, screenSize.y / 2.f);

    drawUI(screenCenter);

    guiManager.render(window);
    window.display();
}

void Game::drawWorld() {
    // 1. T³o (zawsze pierwsze)
    sf::RectangleShape arena(sf::Vector2f(Config::LOGICAL_WIDTH, Config::LOGICAL_HEIGHT));
    arena.setFillColor(Config::COLOR_ARENA);
    window.draw(arena);

    // 2. Efekty i kulki (pod spodem)
    for (const auto& orb : expOrbs) orb->draw(window);

    // 3. WROGOWIE (teraz na wierzchu pocisków, by ich widzieæ)
    for (const auto& enemy : enemies) enemy->draw(window);

    // 4. Pociski
    for (const auto& proj : projectiles) proj->draw(window);

    // 5. Gracz (zawsze najwy¿ej)
    if (!isGameOver) player->draw(window);
}

void Game::drawUI(ImVec2 screenCenter) {
    if (!isGameOver) {
        // --- 1. HUD (Paski i Czas) ---
        // Ustawiamy pozycjê HUD-a
        ImGui::SetNextWindowPos(ImVec2(screenCenter.x, 10.f), ImGuiCond_Always, ImVec2(0.5f, 0.f));

        // Rozpoczynamy okno HUD
        if (ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs)) {
            const float barW = 450.f;

            // Czas gry
            int mins = static_cast<int>(gameTime) / 60;
            int secs = static_cast<int>(gameTime) % 60;
            char timeBuf[16]; sprintf(timeBuf, "%02d:%02d", mins, secs);
            ImGui::SetCursorPosX((barW - ImGui::CalcTextSize(timeBuf).x) / 2.f);
            ImGui::TextColored(ImVec4(1, 1, 1, 0.8f), "%s", timeBuf);

            // Pasek HP
            ImVec2 hpPos = ImGui::GetCursorPos();
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::ProgressBar(player->getHpProgress(), ImVec2(barW, 30.f), "");
            ImGui::PopStyleColor();
            char hpBuf[64]; sprintf(hpBuf, "HP: %.0f / %.0f", player->getCurrentHp(), player->getMaxHp());
            ImGui::SetCursorPos(ImVec2(hpPos.x + (barW - ImGui::CalcTextSize(hpBuf).x) / 2.f, hpPos.y + (30.f - ImGui::CalcTextSize(hpBuf).y) / 2.f));
            ImGui::Text("%s", hpBuf);

            // Pasek XP
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);
            ImVec2 xpPos = ImGui::GetCursorPos();
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
            ImGui::ProgressBar(player->getXPProgress(), ImVec2(barW, 15.f), "");
            ImGui::PopStyleColor();
            char expBuf[64]; sprintf(expBuf, "XP: %d / %d", player->getCurrentXp(), player->getMaxXp());
            ImGui::SetCursorPos(ImVec2(xpPos.x + (barW - ImGui::CalcTextSize(expBuf).x) / 2.f, xpPos.y + (15.f - ImGui::CalcTextSize(expBuf).y) / 2.f));
            ImGui::Text("%s", expBuf);

            // Poziom
            ImGui::SetCursorPosX((barW - ImGui::CalcTextSize("Poziom: X").x) / 2.f);
            ImGui::Text("Poziom: %d", player->getLevel());
        }
        ImGui::End(); // KONIEC HUD

        // --- 2. MENU ULEPSZEÑ (Osobne okno!) ---
        if (isUpgrading) {
            ImGui::SetNextWindowPos(screenCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowFocus(); // TO POZWOLI KLAWIATURZE DZIA£AÆ OD RAZU

            // 2. Start okna (TYLKO RAZ)
            ImGui::Begin("UPGRADE", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

            if (player->getLevel() % 10 == 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "!!! RZADKIE WZMOCNIENIE !!!");
            }
            else {
                ImGui::Text("WYBIERZ WZMOCNIENIE:");
            }

            ImGui::Separator();
            ImGui::Spacing();

            // 3. Pêtla rysuj¹ca opcje
            for (auto& opt : availableUpgrades) {
                if (ImGui::Button(opt.title.c_str(), ImVec2(320, 40))) {
                    applyUpgrade(opt.type);
                    isUpgrading = false; // Zamyka menu
                }

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 320);
                ImGui::Text("%s", opt.description.c_str());
                ImGui::PopTextWrapPos();
                ImGui::PopStyleColor();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
            }

            ImGui::End(); // KONIEC OKNA
        }
    }
    else {
        // --- 3. GAME OVER ---
        ImGui::SetNextWindowPos(screenCenter, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Koniec Gry", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("ZGINALES!");
        int mins = static_cast<int>(gameTime) / 60;
        int secs = static_cast<int>(gameTime) % 60;
        ImGui::Text("Przetrwales: %02d:%02d", mins, secs);
        if (ImGui::Button("ZAGRAJ PONOWNIE", ImVec2(200, 50))) restartGame();
        ImGui::End();
    }
}

// --- UTILITY ---
void Game::spawnEnemy(int dummy) { // dummy, bo HP liczymy teraz wewn¹trz
    float x, y, margin = 50.f;
    int side = std::rand() % 4;

    // 1. Obliczamy wspó³czynnik czasu (co 10 sekund gry)
    float t_factor = gameTime / 10.0f;

    // 2. Obliczamy statystyki zgodnie z Twoimi wytycznymi
    // HP: (Baza + liniowo) * (1 + procentowo)
    float currentHP = (Config::ENEMY_BASE_HP + (t_factor * Config::ENEMY_HP_ADD))
        * (1.0f + (t_factor * Config::ENEMY_HP_PERC));

    // DMG: (Baza + liniowo) * (1 + procentowo)
    float currentDMG = (Config::ENEMY_BASE_DMG + (t_factor * Config::ENEMY_DMG_ADD))
        * (1.0f + (t_factor * Config::ENEMY_DMG_PERC));

    // SPEED: Tylko liniowo (iloœciowo)
    float currentSPD = Config::ENEMY_BASE_SPEED + (t_factor * Config::ENEMY_SPEED_ADD);

    // 3. Pozycja spawnu
    if (side == 0) { x = (float)(std::rand() % (int)Config::LOGICAL_WIDTH); y = -margin; }
    else if (side == 1) { x = Config::LOGICAL_WIDTH + margin; y = (float)(std::rand() % (int)Config::LOGICAL_HEIGHT); }
    else if (side == 2) { x = (float)(std::rand() % (int)Config::LOGICAL_WIDTH); y = Config::LOGICAL_HEIGHT + margin; }
    else { x = -margin; y = (float)(std::rand() % (int)Config::LOGICAL_HEIGHT); }

    // 4. Tworzymy wroga z obliczonymi statystykami
    enemies.push_back(std::make_unique<Enemy>(x, y, currentHP, currentSPD, currentDMG));
}

Enemy* Game::findClosestEnemy() {
    Enemy* closest = nullptr;
    float minDistSq = 1e9f;
    for (auto& enemy : enemies) {
        float dx = enemy->getPosition().x - player->getPosition().x;
        float dy = enemy->getPosition().y - player->getPosition().y;
        float distSq = dx * dx + dy * dy;
        if (distSq < minDistSq) { minDistSq = distSq; closest = enemy.get(); }
    }
    return closest;
}

sf::Vector2f Game::rotateVector(sf::Vector2f v, float angleDegrees) {
    float radians = angleDegrees * (3.14159f / 180.f);
    return sf::Vector2f(v.x * std::cos(radians) - v.y * std::sin(radians), v.x * std::sin(radians) + v.y * std::cos(radians));
}

bool Game::checkCircleCollision(Entity* e1, Entity* e2, float r1, float r2) {
    float dx = e1->getPosition().x - e2->getPosition().x, dy = e1->getPosition().y - e2->getPosition().y;
    return (dx * dx + dy * dy) < (r1 + r2) * (r1 + r2);
}

void Game::resizeView(const sf::RenderWindow& window, sf::View& view) {
    float windowRatio = window.getSize().x / (float)window.getSize().y;
    float viewRatio = Config::LOGICAL_WIDTH / Config::LOGICAL_HEIGHT;
    float sizeX = 1, sizeY = 1, posX = 0, posY = 0;
    if (windowRatio >= viewRatio) { sizeX = viewRatio / windowRatio; posX = (1 - sizeX) / 2.f; }
    else { sizeY = windowRatio / viewRatio; posY = (1 - sizeY) / 2.f; }
    view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
}

void Game::generateUpgrades() {
    availableUpgrades.clear();
    std::vector<UpgradeOption> pool;

    if (player->getLevel() > 0 && player->getLevel() % 10 == 0) {
        pool = {
            { UpgradeType::RARE_MULTI, "Echo Wojny", "Dodatkowa seria pociskow przy kazdym strzale." },
            { UpgradeType::RARE_RICO, "Rykoszet", "Pociski odbijaja sie miedzy wrogami (+2 PEN)." },
			{ UpgradeType::RARE_CONE, "Salwa", "Wystrzeliwujesz wiecej pociskow w szerszym stozku." }, // przeniesc do normalnych
            { UpgradeType::RARE_THORN, "Pancerz Cierniowy", "Zwieksza obrazenia zadawane przy kontakcie." }, // flat dmg reduction
            { UpgradeType::RARE_GIANT, "Sila Giganta", "+25% Max HP i zwiekszona regeneracja." },
            { UpgradeType::RARE_GHOST, "Duch", "+15% szansy na unik i predkosc ruchu." } // zmniejsz rozmiar
			// inne rzadkie ulepszenia:
            // snajper zwieksza zasieg, zmniejsza rozrzut,zwieksza obrazenia krytyczne
			// bomba - pociski eksploduja przy trafieniu, gracz eksploduje przy kontakcie (zadawane obrazenia zalezne od zdrowia gracza)
			// wampir - leczenie za obrazenia zadane wrogom
			// zamrozenie - szansa na zamrozenie wroga przy trafieniu i kontakcie, szansa zalezna od szczescia
			// berzerker - zwieksza obrazenia i szybkosc ataku przy niskim HP
			// elektromagnes - tworzy pole o zasiegu magnesu zadajace obrazenia zalezne od zdrowia gracza
        };
    }
    else {
        pool = {
            { UpgradeType::HP, "Witalnosc (+50 HP)", "Zwieksza zdrowie." },
            { UpgradeType::DMG, "Sila (+1 DMG)", "Zwieksza obrazenia." },
            { UpgradeType::SPD, "Zrecznosc (+10% SPD)", "Szybszy ruch." },
            { UpgradeType::ATK_SPD, "Tempo (+10% ATK SPD)", "Czestsze strzaly." },
            { UpgradeType::ARMOR, "Pancerz (+2 ARM)", "Redukcja obrazen." },
            { UpgradeType::REGEN, "Regeneracja (+1 HP/s)", "Leczenie w czasie." },
            { UpgradeType::PICKUP, "Magnes (+40 MGN)", "Wiekszy zasieg zbierania." },
            { UpgradeType::PEN, "Przebicie (+1 PEN)", "Pociski przebijaja wrogow." }
            // dmg procentowy
			// szczescie - szansa na krytyczne trafienie i na dodatkowe opcje przy ulepszeniach
        };
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(pool.begin(), pool.end(), g);

    int numToPick = std::min(3, (int)pool.size());
    for (int i = 0; i < numToPick; i++) availableUpgrades.push_back(pool[i]);
}

void Game::applyUpgrade(UpgradeType type) {
    switch (type) {
    case UpgradeType::HP: player->upgradeHp(); break;
    case UpgradeType::DMG: player->upgradeDmg(); break;
    case UpgradeType::SPD: player->upgradeSpd(); break;
    case UpgradeType::PROJ: player->upgradeProj(); break;
    case UpgradeType::PEN: player->upgradePen(); break;
    case UpgradeType::PICKUP: player->upgradePickupRange(); break;
    case UpgradeType::ATK_SPD: player->upgradeAtkSpd(); break;
    case UpgradeType::ARMOR:   player->upgradeArmor(); break;
    case UpgradeType::REGEN:   player->upgradeRegen(); break;
    case UpgradeType::KNCK:    player->upgradeKnockback(); break;
        // RZADKIE:
    case UpgradeType::RARE_MULTI: player->activateMulti(); break;
    case UpgradeType::RARE_RICO:  player->activateRico(); break;
    case UpgradeType::RARE_CONE:  player->activateConeShot(); break;
    case UpgradeType::RARE_THORN: player->activateThorn(); break;
    case UpgradeType::RARE_GIANT: player->activateGiant(); break;
    case UpgradeType::RARE_GHOST: player->activateGhost(); break;
    }
    isUpgrading = false;
}