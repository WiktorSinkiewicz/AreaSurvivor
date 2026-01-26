#pragma once
#include <vector>
#include <string>
#include <functional>
#include <map>
#include "Player.h"

struct Upgrade {
    std::string id;
    std::string title;
    std::string description;
    bool isRare;
    std::function<void(Player&)> action;
};

class UpgradeManager {
public:
    UpgradeManager();
    void initDatabase();
    std::vector<Upgrade> generateOptions(int playerLevel, float luck);
    void onUpgradeChosen(const Upgrade& chosen);

private:
    std::vector<Upgrade> commonPool;
    std::map<std::string, std::vector<Upgrade>> rareTracks;
    std::map<std::string, int> playerProgress;
};