#pragma once
#include "Entity.h"
#include "Constants.h"

class Player : public Entity {
public:
    Player();
    void handleInput(bool blockInput);
    void update(float dt) override;
    void takeDamage(float amount);
    bool addXP(int amount);

    float getCurrentHp() const { return currentHp; }
    float getMaxHp() const { return maxHp; }
    int getCurrentXp() const { return currentXP; }
    int getMaxXp() const { return maxXP; }
    int getLevel() const { return level; }
    float getXPProgress() const { return (float)currentXP / maxXP; }
    float getHpProgress() const { return currentHp / maxHp; }
    float getPickupRange() const { return pickupRange; }
    float getDamage() const { return damage; }
    float getArmor() const { return armor; }
    float getAttackSpeed() const { return attackSpeed; }
    int getProjectileCount() const { return projectileCount; }
    int getPenetration() const { return penetration; }
    float getProjectileKnockback() const { return projectileKnockback; }
    float getRadius() const { return currentRadius; }

    void upgradeHp() { maxHp += Config::UPGRADE_HP_VAL; currentHp += Config::UPGRADE_HP_VAL; }
    void upgradeDmg() { damage += Config::UPGRADE_DMG_VAL; }
    void upgradeSpd() { moveSpeed += Config::UPGRADE_SPD_VAL; }
    void upgradeProj() { projectileCount++; }
    void upgradePen() { penetration++; }
    void upgradePickupRange() { pickupRange += Config::UPGRADE_PICKUP_VAL; }
    void upgradeAtkSpd() { attackSpeed += Config::UPGRADE_ATK_SPD_VAL; }
    void upgradeArmor() { armor += Config::UPGRADE_ARMOR_VAL; }
    void upgradeRegen() { regen += Config::UPGRADE_REGEN_VAL; }
    void upgradeKnockback() { projectileKnockback += Config::UPGRADE_KNCK_VAL; }

    // W sekcji public klasy Player:
    void activateMulti() { multiShotLevel++; } // Ka¿dy poziom to +1 dodatkowa fala pocisków
    void activateRico() {
        ricochetLevel++;
        penetration += 1; // Ka¿dy poziom rykoszetu zwiêksza penetracjê o 1, by pocisk móg³ siê odbiæ
    }
    void activateConeShot() {
        coneShotLevel++;
        projectileCount += 1; // Ka¿dy poziom Salwy dodaje 1 dodatkowy pocisk do serii
    }
    void activateThorn() {
        thornLevel++;
    }
    void activateGiant() {
        giantLevel++;
        float bonusHp = maxHp * 0.25f;
        maxHp += bonusHp;
        currentHp += bonusHp;
        regen += 1.0f;

        // Zwiêkszamy rozmiar gracza o 15% przy ka¿dym poziomie
        currentRadius += Config::PLAYER_RADIUS * 0.15f;
        shape.setRadius(currentRadius);
        shape.setOrigin(currentRadius, currentRadius);
    }
    void activateGhost() {
        ghostLevel++;
        dodgeChance += 0.05f; // +5% do uniku za ka¿dym razem
        if (dodgeChance > 0.5f) dodgeChance = 0.5f; // Cap 50%
        moveSpeed += 0.1f;
    }

    int multiShotLevel = 0;
    int ricochetLevel = 0;
    int coneShotLevel = 0;
    int thornLevel = 0;
    int giantLevel = 0;
    int ghostLevel = 0;

private:
    void levelUp();
    float invulnTimer = 0.f;
    float maxHp, currentHp, moveSpeed, damage, pickupRange, attackSpeed, armor, regen, projectileKnockback, dodgeChance, currentRadius;
    int level, currentXP, maxXP, projectileCount, penetration;
};