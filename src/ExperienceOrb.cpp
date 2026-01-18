#include "ExperienceOrb.h"
#include "Constants.h"

ExperienceOrb::ExperienceOrb(sf::Vector2f pos, int xpAmount) : xpAmount(xpAmount) {
    shape.setRadius(Config::EXP_ORB_RADIUS);
    shape.setFillColor(sf::Color(50, 150, 255)); // Jasny niebieski
    shape.setOrigin(Config::EXP_ORB_RADIUS, Config::EXP_ORB_RADIUS);
    setPosition(pos.x, pos.y);
}

void ExperienceOrb::update(float dt) {}