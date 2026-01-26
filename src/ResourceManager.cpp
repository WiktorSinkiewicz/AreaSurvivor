#include "ResourceManager.h"
#include <iostream>

ResourceManager& ResourceManager::get() {
    static ResourceManager instance;
    return instance;
}

bool ResourceManager::loadFont(const std::string& name, const std::string& path) {
    // Sprawdzamy czy plik w ogóle istnieje na dysku przed ³adowaniem
    sf::Font font;
    if (font.loadFromFile(path)) {
        fonts[name] = font;
        return true;
    }

    // Jeœli nie, spróbujmy samej nazwy (plik obok .exe)
    if (font.loadFromFile("ARIAL.TTF")) {
        fonts[name] = font;
        return true;
    }

    std::cerr << "!!! CRITICAL ERROR: Could not find font face at: " << path << std::endl;
    return false;
}

sf::Font& ResourceManager::getFont(const std::string& name) {
    // Zamiast .at(name), u¿ywamy bezpieczniejszego sprawdzenia, by unikn¹æ image_8c3763.png
    auto it = fonts.find(name);
    if (it != fonts.end()) {
        return it->second;
    }

    // Fallback: Zwracamy pusty obiekt czcionki, ¿eby SFML narysowa³ prostok¹ty zamiast wywalaæ grê
    static sf::Font defaultFont;
    return defaultFont;
}