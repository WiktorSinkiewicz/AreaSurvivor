#include "CombatSystem.h"
#include "ResourceManager.h"
#include "Constants.h"
#include <cmath>
#include <algorithm>

void CombatSystem::update(float dt, Player& player, std::vector<std::unique_ptr<Enemy>>& enemies) {
    shootTimer += dt;
    float attackSpeed = player.getAttackSpeed();
    float effectiveShootInterval = Config::SHOOT_INTERVAL / attackSpeed;

    // Dynamiczny rozrzut dla cone shot
    float currentSpread = Config::PROJECTILE_SPREAD + (player.coneShotLevel * 2.0f);

    // aura
    if (player.auraLevel > 0) {
        auraTimer += dt;
        float range = player.getPickupRange() * player.auraRadiusMult;

        auraVisual.setRadius(range);
        auraVisual.setOrigin(range, range);
        auraVisual.setPosition(player.getPosition());
        auraVisual.setFillColor(sf::Color(150, 50, 250, 40)); // Fioletowy przezroczysty cieñ

		if (auraTimer >= 0.5f) { // dmg co 0.5 sekundy
            float auraDmg = (player.getMaxHp() * player.auraDamageMult) * player.getDamageMultiplier();
            for (auto& enemy : enemies) {
                if (checkCircleCollision(player.getPosition(), enemy->getPosition(), range, Config::ENEMY_RADIUS)) {
                    enemy->takeDamage(auraDmg);
                    addDamageText(enemy->getPosition(), auraDmg, false); // Wyœwietlanie dmg aury

                    // Odrzut aury na III poziomie
                    if (player.auraLevel == 3) {
                        sf::Vector2f dir = enemy->getPosition() - player.getPosition();
                        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                        if (dist > 0) enemy->applyKnockback(dir / dist, 15.f);
                    }
                }
            }
            auraTimer = 0.f;
        }
    }
    else {
        auraVisual.setRadius(0); // Ukryj aurê, jeœli gracz jej nie posiada
    }

    for (auto it = echoQueue.begin(); it != echoQueue.end();) {
        it->delayTimer -= dt;
        if (it->delayTimer <= 0.f) {
            int count = player.getProjectileCount();
            for (int i = 0; i < count; ++i) {
                float angleOffset = (i - (count - 1) / 2.f) * currentSpread;
                sf::Vector2f rot = rotateVector(it->direction, angleOffset);
                projectiles.push_back(std::make_unique<Projectile>(player.getPosition(), rot, player.getDamage(), player.getPenetration()));
            }

            it->remainingBursts--;
            if (it->remainingBursts <= 0) it = echoQueue.erase(it);
            else {
                it->delayTimer = 0.15f / attackSpeed;
                ++it;
            }
        }
        else ++it;
    }

    if (shootTimer >= effectiveShootInterval && !enemies.empty()) {
        if (Enemy* target = findClosestEnemy(player.getPosition(), enemies)) {
            sf::Vector2f dir = target->getPosition() - player.getPosition();
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0) dir /= len; // Normalizacja kierunku strza³u

            int count = player.getProjectileCount();
            for (int i = 0; i < count; ++i) {
                float angleOffset = (i - (count - 1) / 2.f) * currentSpread;
                sf::Vector2f rot = rotateVector(dir, angleOffset);
                projectiles.push_back(std::make_unique<Projectile>(player.getPosition(), rot, player.getDamage(), player.getPenetration()));
            }

            // Jeœli gracz ma ulepszenie Echo, dodaj seriê do kolejki
            if (player.multiShotLevel > 0) {
                echoQueue.push_back({ 0.15f / attackSpeed, player.multiShotLevel, dir });
            }
            shootTimer = 0.f;
        }
    }

    for (auto it = projectiles.begin(); it != projectiles.end();) {
        (*it)->update(dt);
        if ((*it)->isOffScreen()) it = projectiles.erase(it);
        else ++it;
    }

    for (auto it = damageTexts.begin(); it != damageTexts.end();) {
        it->lifetime -= dt;
        it->text.move(it->velocity * dt);

        float factor = std::max(0.f, it->lifetime / 0.8f);
        sf::Color currentColor = it->startColor;
        currentColor.a = static_cast<sf::Uint8>(255 * factor); // Zanikanie tekstu
        it->text.setFillColor(currentColor);

        if (it->lifetime <= 0) it = damageTexts.erase(it);
        else ++it;
    }
}

void CombatSystem::checkCollisions(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies) {
    for (auto& proj : projectiles) {
        if (proj->isExpired()) continue;
        for (auto it = enemies.begin(); it != enemies.end(); ++it) {
            if (proj->canHit(it->get()) && checkCircleCollision(proj->getPosition(), (*it)->getPosition(), Config::BULLET_RADIUS, Config::ENEMY_RADIUS)) {
                float dmg = proj->getDamage() * player.getDamageMultiplier();
                bool isCrit = (rand() % 100) < player.getLuck();
                if (isCrit) dmg *= player.critMultiplier;

                sf::Vector2f knockbackDir = (*it)->getPosition() - proj->getPosition();
                float len = std::sqrt(knockbackDir.x * knockbackDir.x + knockbackDir.y * knockbackDir.y);
                if (len > 0) (*it)->applyKnockback(knockbackDir / len, player.getProjectileKnockback());

                (*it)->takeDamage(dmg);
                proj->registerHit(it->get());
                addDamageText((*it)->getPosition(), dmg, isCrit);

                if (player.freezeLevel > 0) {
                    float fChance = player.freezeChance + (player.getLuck() * 0.01f);
                    if ((static_cast<float>(rand() % 100) / 100.f) < fChance) {
                        it->get()->freeze(2.0f);
                        it->get()->getShape().setFillColor(sf::Color(100, 200, 255)); // Kolor lodu
                    }
                }
                if (player.vampireLevel > 0) {
                    float hitChance = player.vampireLevel * 0.1f;
                    if ((static_cast<float>(rand() % 1000) / 1000.f) < hitChance) {
                        player.heal(1.0f); // Leczy o 1 HP
                    }
                }
                if (player.ricochetLevel > 0 && !proj->isExpired()) {
                    // Szukamy najbli¿szego wroga, wykluczaj¹c tego, którego w³aœnie trafiliœmy
                    Enemy* nextTarget = findClosestEnemy(proj->getPosition(), enemies, it->get());

                    if (nextTarget) {
                        sf::Vector2f newDir = nextTarget->getPosition() - proj->getPosition();
                        proj->redirect(newDir); // Odbicie w stronê nowego celu
                    }
                }
            }
        }
    }
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const auto& p) { return p->isExpired(); }), projectiles.end());
}

void CombatSystem::draw(sf::RenderWindow& window) {
    for (auto& proj : projectiles) proj->draw(window);
    for (auto& dt : damageTexts) window.draw(dt.text);
}

void CombatSystem::addDamageText(sf::Vector2f pos, float dmg, bool isCrit) {
    DamageText dt;
    // Pobieramy bezpiecznie czcionkê
    dt.text.setFont(ResourceManager::get().getFont("main"));
    dt.text.setString(std::to_string((int)dmg));

    // Przywracamy rozmiary z Twojej stabilnej wersji
    dt.text.setCharacterSize(isCrit ? 35 : 22);
    dt.startColor = isCrit ? sf::Color(255, 215, 0) : sf::Color::White;
    dt.text.setFillColor(dt.startColor);
    dt.text.setOutlineThickness(0.f);

    sf::FloatRect bounds = dt.text.getLocalBounds();
    dt.text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    dt.text.setPosition(pos);

    dt.lifetime = 0.8f;
    dt.velocity = sf::Vector2f((std::rand() % 100 - 50), -140.f);
    damageTexts.push_back(dt);
}

Enemy* CombatSystem::findClosestEnemy(sf::Vector2f pos, std::vector<std::unique_ptr<Enemy>>& enemies, Enemy* exclude) {
    Enemy* closest = nullptr;
    float minDistSq = 1e9f;
    for (auto& enemy : enemies) {
        if (enemy.get() == exclude) continue;
        float dx = enemy->getPosition().x - pos.x;
        float dy = enemy->getPosition().y - pos.y;
        float distSq = dx * dx + dy * dy;
        if (distSq < minDistSq) { minDistSq = distSq; closest = enemy.get(); }
    }
    return closest;
}

sf::Vector2f CombatSystem::rotateVector(sf::Vector2f v, float angle) {
    float rad = angle * (3.14159f / 180.f);
    return sf::Vector2f(v.x * std::cos(rad) - v.y * std::sin(rad), v.x * std::sin(rad) + v.y * std::cos(rad));
}

bool CombatSystem::checkCircleCollision(sf::Vector2f p1, sf::Vector2f p2, float r1, float r2) {
    float dx = p1.x - p2.x, dy = p1.y - p2.y;
    return (dx * dx + dy * dy) < (r1 + r2) * (r1 + r2);
}