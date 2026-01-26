#include "UpgradeManager.h"
#include <algorithm>
#include <random>

UpgradeManager::UpgradeManager() {
    initDatabase();
}

void UpgradeManager::initDatabase() {
    commonPool = {
        { "HP", "Witalnosc (+50 HP)", "Wieksze zdrowie.", false, [](Player& p) { p.addMaxHp(Config::UPGRADE_HP_VAL); } },
        { "DMG", "Sila (+1 DMG)", "Wieksze obrazenia bazowe.", false, [](Player& p) { p.addBaseDamage(Config::UPGRADE_DMG_VAL); } },
        { "DMG_P", "Gniew (+10% DMG)", "Wiekszy mnoznik obrazen.", false, [](Player& p) { p.addDmgMultiplier(0.10f); } },
        { "SPD", "Zrecznosc (+10% SPD)", "Szybszy ruch.", false, [](Player& p) { p.addMoveSpd(Config::UPGRADE_SPD_VAL); } },
        { "ATK", "Tempo (+10% ATK SPD)", "Szybszy atak.", false, [](Player& p) { p.addAtkSpd(Config::UPGRADE_ATK_SPD_VAL); } },
        { "LUCK", "Szczescie (+5% LCK)", "Krytyki i lepsze wybory.", false, [](Player& p) { p.addLuck(5.0f); } },
        { "ARMOR", "Pancerz (+2 ARMOR)", "Redukcja obrazen.", false, [](Player& p) { p.addArmor(2.f); } },
        { "REGEN", "Regeneracja (+1 HP/s)", "Szybsze leczenie.", false, [](Player& p) { p.addRegen(1.f); } },
        { "PEN", "Przenikanie (+1 PEN)", "Wieksza penetracja pociskow.", false, [](Player& p) { p.addPenetration(1); } },
		{ "KNOCK", "Odepchniecie (+200 KNCK)", "Silniejsze odpychanie pociskow.", false, [](Player& p) { p.addKnockback(200.f); } },
        { "CONE", "Salwa (+1 PROJ)", "Wystrzeliwujesz wiecej pociskow w stozku.", false, [](Player& p) { p.addConeShot(); } },
        { "MAG", "Magnes (+40 MGN)", "Wiekszy zasieg zbierania XP.", false, [](Player& p) { p.addPickupRange(Config::UPGRADE_PICKUP_VAL); } }
    };

    rareTracks["ECHO"] = {
        { "ECHO", "Echo Wojny I", "Dodatkowa seria.", true, [](Player& p) { p.activateMulti(); } },
        { "ECHO", "Echo Wojny II", "Druga seria.", true, [](Player& p) { p.activateMulti(); } },
        { "ECHO", "Echo Wojny III", "Serie i +20% tempa.", true, [](Player& p) { p.activateMulti(); p.addAtkSpd(0.20f); } }
    };

    rareTracks["RICO"] = {
        { "RICO", "Rykoszet I", "Odbicia (+1 PEN).", true, [](Player& p) { p.addRicochet(1); } },
        { "RICO", "Rykoszet II", "Wiecej odbic (+1 PEN).", true, [](Player& p) { p.addRicochet(1); } },
        { "RICO", "Rykoszet III", "Maks. odbicia (+2 PEN).", true, [](Player& p) { p.addRicochet(2); } }
    };

    rareTracks["GIANT"] = {
        { "GIANT", "Sila Giganta I", "+50 HP i +15% rozmiaru.", true, [](Player& p) { p.addMaxHp(50.f); p.addSize(1.15f); } },
        { "GIANT", "Sila Giganta II", "+100 HP i +15% rozmiaru.", true, [](Player& p) { p.addMaxHp(100.f); p.addSize(1.15f); } },
        { "GIANT", "Sila Giganta III", "+200 HP i +5 pancerza.", true, [](Player& p) { p.addMaxHp(200.f); p.addArmor(5.f); } }
    };

    rareTracks["GHOST"] = {
        { "GHOST", "Duch I", "40% szansy na unik", true, [](Player& p) { p.addDodge(0.40f); } },
        { "GHOST", "Duch II", "70% uniku i predkosc.", true, [](Player& p) { p.addDodge(0.30f); p.addMoveSpd(0.2f); } },
        { "GHOST", "Duch III", "90% uniku i mniejszy rozmiar.", true, [](Player& p) { p.addDodge(0.20f); p.addSize(0.85f); } }
    };

    rareTracks["SNIPE"] = {
        { "SNIPE", "Snajper I", "Krytyki x2.5.", true, [](Player& p) { p.addCritMultiplier(0.5f); } },
        { "SNIPE", "Snajper II", "Krytyki x3.0 i +10% LCK.", true, [](Player& p) { p.addCritMultiplier(0.5f); p.addLuck(10.f); } },
        { "SNIPE", "Snajper III", "Krytyki x4.0 i +2 PEN.", true, [](Player& p) { p.addCritMultiplier(1.0f); p.addPenetration(2); } }
    };

    rareTracks["VAMP"] = {
        { "VAMP", "Wampiryzm I", "10% szans na leczenie przy killu.", true, [](Player& p) { p.addVampire(1); } },
        { "VAMP", "Wampiryzm II", "25% szans na leczenie.", true, [](Player& p) { p.addVampire(1); } },
        { "VAMP", "Wampiryzm III", "50% szans i +2 HP/s regenu.", true, [](Player& p) { p.addVampire(1); p.addRegen(2.f); } }
    };

    rareTracks["THORN"] = {
        { "THORN", "Pancerz Cierniowy I", "Zadaje obrazenia przy dotyku.", true, [](Player& p) { p.activateThorn(); } },
        { "THORN", "Pancerz Cierniowy II", "Podwojne obrazenia cierni.", true, [](Player& p) { p.activateThorn(); } },
        { "THORN", "Pancerz Cierniowy III", "Ciernie daja tez +5 pancerza.", true, [](Player& p) { p.activateThorn(); p.upgradeArmor(); p.upgradeArmor(); } }
    };

    rareTracks["FREEZE"] = {
        { "FREEZE", "Lodowy Dotyk I", "Pociski maja szanse zamrozic wrogow (zalezne od LCK).", true, [](Player& p) { p.addFreezeChance(0.1f); p.freezeLevel = 1; } },
        { "FREEZE", "Lodowy Dotyk II", "Wieksza szansa na mroz i +10 LCK.", true, [](Player& p) { p.addFreezeChance(0.1f); p.addLuck(10.f); p.freezeLevel = 2; } },
        { "FREEZE", "Lodowy Dotyk III", "Zamrozeni wrogowie otrzymuja obrazenia w czasie.", true, [](Player& p) { p.addFreezeChance(0.1f); p.freezeLevel = 3; } }
    };

    rareTracks["AURA"] = {
        { "AURA", "Swieta Aura I", "Zadaje obrazenia wokol gracza (2% Max HP).", true, [](Player& p) { p.addAura(0.02f, 0.5f); p.auraLevel = 1; } },
        { "AURA", "Swieta Aura II", "Zwieksza zasieg i dmg aury", true, [](Player& p) { p.addAura(0.04f, 0.5f); p.auraLevel = 2; } },
        { "AURA", "Swieta Aura III", "Aura zadaje 6% Max HP", true, [](Player& p) { p.addAura(0.06f, 0.5f); p.auraLevel = 3; } }
    };

    rareTracks["RAM"] = {
        { "RAM", "Taran I", "Otrzymujesz 2 pkt mniej obrazen i zadajesz dmg przy zderzeniu.", true, [](Player& p) { p.addRam(2.f, 0.5f); p.ramLevel = 1; } },
        { "RAM", "Taran II", "+5 redukcji dmg i wiekszy dmg od predkosci.", true, [](Player& p) { p.addRam(3.f, 0.5f); p.ramLevel = 2; } },
        { "RAM", "Taran III", "Zderzenie zadaje potworny dmg (zalezy od Max HP).", true, [](Player& p) { p.addRam(5.f, 1.0f); p.ramLevel = 3; } }
    };

    // Inicjalizacja postêpu (wszystko na poziomie 0)
    for (auto const& [id, track] : rareTracks) {
        playerProgress[id] = 0;
    }
}

std::vector<Upgrade> UpgradeManager::generateOptions(int playerLevel, float luck) {
    std::vector<Upgrade> availableNow;

    // 1. Sprawdzamy czy to czas na rzadkie ulepszenia
    bool isRareRoll = (playerLevel > 0 && playerLevel % 10 == 0);

    if (isRareRoll) {
        // Budujemy aktualnie dostêpn¹ pulê rzadkich ulepszeñ
        for (auto const& [id, track] : rareTracks) {
            int currentLevel = playerProgress[id];
            if (currentLevel < 3) { // Jeœli poziom III nie zosta³ osi¹gniêty
                availableNow.push_back(track[currentLevel]);
            }
        }
    }

    // 2. Jeœli pula rzadkich jest pusta lub to zwyk³y poziom, bierzemy zwyk³e
    if (availableNow.empty() || !isRareRoll) {
        availableNow = commonPool;
    }

    // 3. Mieszanie i wybór opcji (Logika szczêœcia)
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(availableNow.begin(), availableNow.end(), g);

    int numOptions = 3 + static_cast<int>(luck / 100.0f);
    if ((std::rand() % 100) < fmod(luck, 100.0f)) numOptions++;

    std::vector<Upgrade> result;
    int actualCount = std::min(numOptions, (int)availableNow.size());
    for (int i = 0; i < actualCount; ++i) {
        result.push_back(availableNow[i]);
    }

    return result;
}

void UpgradeManager::onUpgradeChosen(const Upgrade& chosen) {
    // Jeœli wybrano rzadkie ulepszenie, zwiêkszamy jego poziom w postêpie
    if (chosen.isRare) {
        playerProgress[chosen.id]++;
    }
}