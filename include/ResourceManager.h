#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class ResourceManager {
public:
    static ResourceManager& get();

    // Metody ³adowania i pobierania zasobów
    bool loadFont(const std::string& name, const std::string& path);
    sf::Font& getFont(const std::string& name);

    void loadTexture(const std::string& name, const std::string& path);
    sf::Texture& getTexture(const std::string& name);

private:
    ResourceManager() = default;
    // Mapy przechowuj¹ce za³adowane dane w pamiêci RAM
    std::map<std::string, sf::Font> fonts;
    std::map<std::string, sf::Texture> textures;
};