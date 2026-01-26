#include "GUIManager.h"
#include "imguiThemes.h"

void GUIManager::init(sf::RenderWindow& window) {
    ImGui::SFML::Init(window);
    imguiThemes::embraceTheDarkness();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.FontGlobalScale = 2.f;

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg].w = 0.9f;
}

void GUIManager::update(float dt) {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, sf::Color(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, sf::Color(0, 0, 0, 0));
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    ImGui::PopStyleColor(2);
}

void GUIManager::processEvent(sf::RenderWindow& window, const sf::Event& event) {
    ImGui::SFML::ProcessEvent(window, event);
}

void GUIManager::render(sf::RenderWindow& window) {
    ImGui::SFML::Render(window);
}

bool GUIManager::wantsCaptureInput() const {
    return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

void GUIManager::renderDebugPanel(float fps, sf::Vector2f playerPos) {
    ImGui::Begin("Debug Panel");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Position: (%.1f, %.1f)", playerPos.x, playerPos.y);
    ImGui::End();
}

void GUIManager::shutdown() {
    ImGui::SFML::Shutdown();
}