#include "Game.h"
#include "ResourceManager.h"
#include <imgui.h>
#include <imgui-SFML.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm> 

Game::Game() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    ResourceManager::get().loadFont("main", "resources/fonts/ARIAL.TTF");

    window.create(sf::VideoMode(Config::LOGICAL_WIDTH, Config::LOGICAL_HEIGHT),
        "Arena Survivor", sf::Style::Default);
    window.setFramerateLimit(Config::FRAME_LIMIT);
    window.setKeyRepeatEnabled(false);

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
    expOrbs.clear();
    currentOptions.clear();

    spawnTimer = 0.f;
    gameTime = 0.f;
    isGameOver = false;
    isUpgrading = false;
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
        if (event.type == sf::Event::Resized) resizeView(window, gameView);

        // menu ulepszen
        if (isUpgrading) {
            ImGuiIO& io = ImGui::GetIO();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::W) io.AddKeyEvent(ImGuiKey_UpArrow, true);
                else if (event.key.code == sf::Keyboard::S) io.AddKeyEvent(ImGuiKey_DownArrow, true);
                else if (event.key.code == sf::Keyboard::Space) io.AddKeyEvent(ImGuiKey_Space, true);
            }
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::W) io.AddKeyEvent(ImGuiKey_UpArrow, false);
                else if (event.key.code == sf::Keyboard::S) io.AddKeyEvent(ImGuiKey_DownArrow, false);
                else if (event.key.code == sf::Keyboard::Space) io.AddKeyEvent(ImGuiKey_Space, false);
            }
        }
    }
}

void Game::update(float dt) {
    ImGui::SFML::Update(window, sf::seconds(dt));
    guiManager.update(dt);

    if (isGameOver || isUpgrading) return;

    gameTime += dt;
    window.setMouseCursorVisible(isUpgrading || isGameOver);

    updateSpawning(dt);
    player->update(dt);

    for (auto& enemy : enemies) {
        enemy->update(dt, player->getPosition());
    }

    combatSystem.update(dt, *player, enemies);
    combatSystem.checkCollisions(*player, enemies);
    updateExperience(dt);
    resolveEnemyCollisions();

    // Kolizja Gracz vs Wrogowie
    for (auto& enemy : enemies) {
        if (checkCircleCollision(player->getPosition(), enemy->getPosition(), player->getRadius(), Config::ENEMY_RADIUS)) {

            if (player->ramLevel > 0) {
                float ramBase = (player->getMoveSpeed() * Config::RARE_RAM_SPD_COEFF +
                    player->getMaxHp() * Config::RARE_RAM_HP_COEFF);

                float ramDmg = ramBase * player->collisionDamageMult * player->getDamageMultiplier();
                enemy->takeDamage(ramDmg);
                combatSystem.addDamageText(enemy->getPosition(), ramDmg, false);
            }

            if (player->thornLevel > 0) {
                float thornDmg = (player->getDamage() * 0.5f + player->getArmor() * Config::RARE_THORN_SCALING) * player->thornLevel;
                float finalThorn = thornDmg * player->getDamageMultiplier();
                enemy->takeDamage(finalThorn);
                combatSystem.addDamageText(enemy->getPosition(), finalThorn, false);
            }

            float finalIncomingDmg = std::max(1.f, enemy->getDamage() - player->flatDamageReduction);
            player->takeDamage(finalIncomingDmg);

            if (player->getCurrentHp() <= 0.f) {
                isGameOver = true;
                window.setMouseCursorVisible(true);
                break; // Przerywamy pêtlê, bo gracz zgin¹³
            }

			// odrzut wroga po zderzeniu z graczem
            sf::Vector2f pushDir = enemy->getPosition() - player->getPosition();
            float dist = std::sqrt(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
            if (dist > 0) {
                pushDir /= dist;
                float force = player->getProjectileKnockback();
                enemy->applyKnockback(pushDir, force);
            }
        }
    }

    // Usuwanie martwych wrogow i generowanie XP
    for (auto it = enemies.begin(); it != enemies.end();) {
        if ((*it)->isDead()) {
            float intervals = gameTime / 60.0f;
            int scaledXP = static_cast<int>(Config::XP_ORB_BASE_VALUE * std::pow(Config::XP_ORB_VALUE_GROWTH, intervals));

            expOrbs.push_back(std::make_unique<ExperienceOrb>((*it)->getPosition(), scaledXP));
            it = enemies.erase(it);
        }
        else ++it;
    }
}

void Game::updateExperience(float dt) {
    for (auto it = expOrbs.begin(); it != expOrbs.end();) {
        sf::Vector2f orbPos = (*it)->getPosition();
        float dist = std::sqrt(std::pow(player->getPosition().x - orbPos.x, 2) + std::pow(player->getPosition().y - orbPos.y, 2));

        if (dist < player->getPickupRange()) {
            sf::Vector2f dir = player->getPosition() - orbPos;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0) (*it)->move((dir / len) * Config::ORB_SPEED * dt);
        }

        if (dist < player->getRadius() + Config::EXP_ORB_RADIUS) {
            if (player->addXP((*it)->getXpAmount())) {
                if (!isUpgrading) {
                    isUpgrading = true;
                    // Pobieramy opcje z UpgradeManager
                    currentOptions = upgradeManager.generateOptions(player->getLevel(), player->getLuck());
                }
            }
            it = expOrbs.erase(it);
        }
        else ++it;
    }
}



void Game::render() {
    window.clear(sf::Color::Black);
    window.setView(gameView);

    drawWorld();

    ImVec2 screenCenter(ImGui::GetMainViewport()->Size.x / 2.f, ImGui::GetMainViewport()->Size.y / 2.f);
    drawUI(screenCenter);

    guiManager.render(window);
    window.display();
}

void Game::drawWorld() {
    sf::RectangleShape arena(sf::Vector2f(Config::LOGICAL_WIDTH, Config::LOGICAL_HEIGHT));
    arena.setFillColor(Config::COLOR_ARENA);
    window.draw(arena);

    for (const auto& orb : expOrbs) orb->draw(window);
    for (const auto& enemy : enemies) enemy->draw(window);

    // CombatSystem rysuje pociski i teksty obra¿eñ
    combatSystem.draw(window);

    if (player->auraLevel > 0) {
        window.draw(combatSystem.auraVisual);
    }


    if (!isGameOver) player->draw(window);
}

void Game::drawUI(ImVec2 screenCenter) {
    if (!isGameOver) {
        // HUD (Paski i Czas)
        ImGui::SetNextWindowPos(ImVec2(screenCenter.x, 10.f), ImGuiCond_Always, ImVec2(0.5f, 0.f));

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
        ImGui::End();

		// panel statystyk z lewej
        ImGui::SetNextWindowPos(ImVec2(10.f, 100.f), ImGuiCond_Always);
        if (ImGui::Begin("CharacterPanel", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs)) {

            ImGui::Text("ATK Bazowy: %.1f", player->getDamage());
            ImGui::Text("Mnoznik (Gniew): x%.2f", player->getDamageMultiplier());
            ImGui::Text("Tempo ataku: %.2f", player->getAttackSpeed());
            ImGui::Text("Pociski: %d", player->getProjectileCount());
            ImGui::Text("Penetracja: %d", player->getPenetration());

            ImGui::Spacing(); ImGui::Spacing();

            float armor = player->getArmor();
            float reduction = (armor > 0) ? (armor / (10.f + armor)) * 100.f : 0.f;
            ImGui::Text("Pancerz: %.0f (Redukcja: %.0f%%)", armor, reduction);
            ImGui::Text("Regen: %.1f/s", player->getRegen());

            ImGui::Spacing(); ImGui::Spacing();

            ImGui::Text("Szczescie: %.0f%%", player->getLuck());
            ImGui::Text("Szybkosc: %.0f%%", player->getMoveSpeed() * 100.f);
            ImGui::Text("Magnes: %.0f", player->getPickupRange());

            if (!player->getRareUpgrades().empty()) {
                ImGui::Spacing(); ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "AKTYWNE MODY:");
                for (auto const& [id, title] : player->getRareUpgrades()) {
                    ImGui::BulletText("%s", title.c_str());
                }
            }
        }
        ImGui::End();

        // menu ulepszen
        if (isUpgrading) {
            ImGui::SetNextWindowPos(screenCenter, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowFocus();
            ImGui::Begin("UPGRADE", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

            if (player->getLevel() % 10 == 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "!!! RZADKIE WZMOCNIENIE !!!");
            }
            else {
                ImGui::Text("WYBIERZ WZMOCNIENIE:");
            }

            ImGui::Separator();
            ImGui::Spacing();

            for (auto& opt : currentOptions) {
                if (ImGui::Button(opt.title.c_str(), ImVec2(320, 40))) {
                    // informacja do menedzera o wyborze
                    upgradeManager.onUpgradeChosen(opt);
                    if (opt.isRare) {
                        player->recordRareUpgrade(opt.id, opt.title);
                    }
                    // akcja na graczu z lambdy
                    opt.action(*player);

                    isUpgrading = false;
                    currentOptions.clear();
                    break;
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
            ImGui::End();
        }
    }
    else {
		// game over screen
        ImGui::SetNextWindowPos(screenCenter, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("KONIEC GRY", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "ZGINALES!");
        ImGui::Separator();

        int mins = static_cast<int>(gameTime) / 60;
        int secs = static_cast<int>(gameTime) % 60;
        ImGui::Text("Czas przetrwania: %02d:%02d", mins, secs);
        ImGui::Text("Osiagniety poziom: %d", player->getLevel());

        ImGui::Spacing();
        if (ImGui::Button("ZAGRAJ PONOWNIE", ImVec2(240, 50))) {
            restartGame();
        }
        ImGui::End();
    }
}

void Game::spawnEnemy(int dummy) {
    if (enemies.size() >= Config::MAX_ENEMIES_LIMIT) return;

    float intervals = gameTime / 30.0f;

    // Obliczanie statystyk bazowych
    float hp = Config::E_HP_BASE * std::pow(Config::E_HP_GROWTH, intervals);
    float dmg = Config::E_DMG_BASE * std::pow(Config::E_DMG_GROWTH, intervals);
    float spd = std::min(Config::PLAYER_SPEED_MAX - 30.f,
        Config::E_SPD_BASE * std::pow(Config::E_SPD_GROWTH, intervals));

    // 2. Logika Elity i Bossa
    bool isBoss = (static_cast<int>(gameTime) > 0 && static_cast<int>(gameTime) % static_cast<int>(Config::BOSS_INTERVAL) == 0);
    float eliteChance = Config::ELITE_CHANCE_BASE + (intervals * (Config::ELITE_CHANCE_GROWTH / 2.f));
    bool isElite = !isBoss && (static_cast<float>(rand() % 100) / 100.f) < eliteChance;

    if (isBoss) {
        hp *= 15.0f; // Boss ma 15x wiêcej HP
        dmg *= 2.0f;
        spd *= 0.7f; // Boss jest wolniejszy
    }
    else if (isElite) {
        hp *= Config::ELITE_STAT_MULT;
        dmg *= 1.5f;
    }

    float x, y, margin = 80.f;
    int side = std::rand() % 4;
    if (side == 0) { x = (float)(std::rand() % (int)Config::LOGICAL_WIDTH); y = -margin; }
    else if (side == 1) { x = Config::LOGICAL_WIDTH + margin; y = (float)(std::rand() % (int)Config::LOGICAL_HEIGHT); }
    else if (side == 2) { x = (float)(std::rand() % (int)Config::LOGICAL_WIDTH); y = Config::LOGICAL_HEIGHT + margin; }
    else { x = -margin; y = (float)(std::rand() % (int)Config::LOGICAL_HEIGHT); }

    auto enemy = std::make_unique<Enemy>(x, y, hp, spd, dmg);

    if (isBoss) {
        enemy->getShape().setRadius(Config::ENEMY_RADIUS * 2.5f);
        enemy->getShape().setOrigin(Config::ENEMY_RADIUS * 2.5f, Config::ENEMY_RADIUS * 2.5f);
        enemy->getShape().setOutlineColor(sf::Color::Yellow);
        enemy->getShape().setOutlineThickness(4.f);
    }
    else if (isElite) {
        enemy->getShape().setOutlineColor(sf::Color::White);
        enemy->getShape().setOutlineThickness(2.f);
        enemy->getShape().setFillColor(sf::Color(200, 0, 0));
    }

    enemies.push_back(std::move(enemy));
}


void Game::updateSpawning(float dt) {
    spawnTimer += dt;
    float minutes = gameTime / 60.0f;

    // U¿ywamy ujednoliconych nazw bez aliasów
    float currentInterval = std::max(Config::SPAWN_INTERVAL_MIN,
        Config::SPAWN_INTERVAL_START - (minutes * Config::SPAWN_REDUCTION_RATE));

    if (spawnTimer >= currentInterval) {
        spawnEnemy(0);
        spawnTimer = 0.f;
    }
}

bool Game::checkCircleCollision(sf::Vector2f p1, sf::Vector2f p2, float r1, float r2) {
    return (std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2)) < std::pow(r1 + r2, 2);
}

void Game::resolveEnemyCollisions() {
    if (enemies.size() < 2) return;
    float minDistance = Config::ENEMY_RADIUS * 2.f;
    for (size_t i = 0; i < enemies.size(); ++i) {
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            sf::Vector2f posA = enemies[i]->getPosition();
            sf::Vector2f posB = enemies[j]->getPosition();
            float distSq = std::pow(posA.x - posB.x, 2) + std::pow(posA.y - posB.y, 2);
            if (distSq < minDistance * minDistance && distSq > 0.f) {
                float dist = std::sqrt(distSq);
                sf::Vector2f pushDir = (posA - posB) / dist;
                sf::Vector2f moveVec = pushDir * (minDistance - dist) * 0.5f;
                enemies[i]->move(moveVec);
                enemies[j]->move(-moveVec);
            }
        }
    }
}

void Game::resizeView(const sf::RenderWindow& window, sf::View& view) {
    float windowRatio = window.getSize().x / (float)window.getSize().y;
    float viewRatio = Config::LOGICAL_WIDTH / Config::LOGICAL_HEIGHT;
    float sizeX = 1, sizeY = 1, posX = 0, posY = 0;
    if (windowRatio >= viewRatio) { sizeX = viewRatio / windowRatio; posX = (1 - sizeX) / 2.f; }
    else { sizeY = windowRatio / viewRatio; posY = (1 - sizeY) / 2.f; }
    view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
}