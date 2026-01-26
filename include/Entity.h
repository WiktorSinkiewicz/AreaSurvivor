#pragma once
#include <SFML/Graphics.hpp>

class Entity {
public:
    virtual ~Entity() = default;

    virtual void update(float dt) = 0;

    virtual void draw(sf::RenderWindow& window);

    // Metody pomocnicze (Wrapper na SFML)
    void setPosition(float x, float y);
    void move(const sf::Vector2f& offset);
    sf::FloatRect getGlobalBounds() const;
    sf::Vector2f getPosition() const;
    sf::CircleShape& getShape() { return shape; }

protected:
    sf::CircleShape shape;
    sf::Vector2f velocity;
};