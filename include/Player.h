#pragma once
#include "Entity.h"
#include "Constants.h"

class Player : public Entity {
public:
    Player();
    void update(float dt) override;
    void takeDamage(float amount);
    bool addXP(int amount);
    void heal(float amount) {
        currentHp = std::min(maxHp, currentHp + amount);
    }
    void recordRareUpgrade(const std::string& id, const std::string& title) {
        rareUpgrades[id] = title; // Mapa nadpisze nr ulepszen
    }
    const std::map<std::string, std::string>& getRareUpgrades() const { return rareUpgrades; }

    sf::CircleShape auraShape;
    void updateAuraVisuals();

    // --- GETTERY ---
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
    float getLuck() const { return luck; }
    float getDamageMultiplier() const { return damageMultiplier; }
    float getMoveSpeed() const { return moveSpeed; }
    float getRegen() const { return regen; }

    void addMaxHp(float amount) { maxHp += amount; currentHp += amount; }
    void addBaseDamage(float amount) { damage += amount; }
    void addDmgMultiplier(float amount) { damageMultiplier += amount; }
    void addAtkSpd(float amount) { attackSpeed += amount; }
    void addMoveSpd(float amount) { moveSpeed += amount; }
    void addLuck(float amount) { luck += amount; }
    void addArmor(float amount) { armor += amount; }
    void addRegen(float amount) { regen += amount; }
    void addDodge(float chance) { dodgeChance += chance; }
    void addCritMultiplier(float amount) { critMultiplier += amount; }
    void addPenetration(int amount) { penetration += amount; }
    void addPickupRange(float amount) { pickupRange += amount; }
    void addKnockback(float amount) { projectileKnockback += amount; }
    void addConeShot() {
        coneShotLevel++;
        projectileCount++;
    }

    void addFreezeChance(float amount) { freezeChance += amount; }
    void addAura(float dmg, float range) { auraDamageMult += dmg; auraRadiusMult += range; }
    void addRam(float reduction, float dmg) { flatDamageReduction += reduction; collisionDamageMult += dmg; }
    void addRicochet(int levels) { ricochetLevel += levels; penetration += levels; }
    void addVampire(int levels) { vampireLevel += levels; }
    void addSize(float scale) {
        currentRadius *= scale;
        shape.setRadius(currentRadius);
        shape.setOrigin(currentRadius, currentRadius);
    }
    void activateMulti() { multiShotLevel++; }
    void activateThorn() { thornLevel++; }
    void upgradeArmor() { armor += Config::UPGRADE_ARMOR_VAL; }

    int multiShotLevel = 0, coneShotLevel = 0, ricochetLevel = 0, thornLevel = 0, vampireLevel = 0;
    int freezeLevel = 0, auraLevel = 0, ramLevel = 0;
    float freezeChance = 0.0f, auraDamageMult = 0.0f, auraRadiusMult = 0.0f;
    float flatDamageReduction = 0.0f, collisionDamageMult = 0.0f;
    float critMultiplier = 2.0f, dodgeChance = 0.0f;

private:
    void levelUp();
    float invulnTimer = 0.f;
    float maxHp, currentHp, moveSpeed, damage, pickupRange, attackSpeed, armor, regen, projectileKnockback, currentRadius;
    float luck = 0.0f, damageMultiplier = 1.0f;
    int level, currentXP, maxXP, projectileCount, penetration;
    std::map<std::string, std::string> rareUpgrades;
};