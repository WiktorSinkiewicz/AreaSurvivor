#pragma once
#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

class GUIManager {
public:
    void init(sf::RenderWindow& window);
    void update(float dt); // Tu jest ImGui::SFML::Update
    void processEvent(sf::RenderWindow& window, const sf::Event& event);
    void render(sf::RenderWindow& window);

    // Zapobiega strzelaniu gdy klikamy w okno
    bool wantsCaptureInput() const;

    // Funkcja do rysowania interfejsu (HUD, Debug)
    void renderDebugPanel(float fps, sf::Vector2f playerPos);
    void shutdown();
};