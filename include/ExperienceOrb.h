#pragma once
#include "Entity.h"

class ExperienceOrb : public Entity {
public:
    ExperienceOrb(sf::Vector2f pos, int xpAmount);
    void update(float dt) override; // Pusta, ruch obs³uguje Game
    int getXpAmount() const { return xpAmount; }
private:
    int xpAmount;
};