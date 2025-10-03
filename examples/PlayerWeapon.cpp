#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"
#include "Sparkle/Event.h"
#include <iostream>
#include <memory>

using namespace Sparkle;

class Player {
public:
    Event<const std::string&> OnWeaponPicked{"OnWeaponPicked"};

    void PickWeapon(const std::string& weaponName) {
        std::cout << "[Player] Picked up: " << weaponName << "\n";
        OnWeaponPicked(weaponName);
    }
};

class HUD {
public:
    void ShowWeapon(const std::string& weaponName) {
        std::cout << "[HUD] Displaying new weapon: " << weaponName << "\n";
    }
};

class AudioManager {
public:
    void PlayWeaponSound(const std::string& weaponName) {
        std::cout << "[Audio] Playing pickup sound for " << weaponName << "\n";
    }
};

int main() {
    auto player = std::make_shared<Player>();
    auto hud = std::make_shared<HUD>();
    auto audio = std::make_shared<AudioManager>();

    // Bind HUD and AudioManager to the event
    player->OnWeaponPicked.Bind(&HUD::ShowWeapon, std::weak_ptr<HUD>(hud));
    player->OnWeaponPicked.Bind(&AudioManager::PlayWeaponSound, std::weak_ptr<AudioManager>(audio));

    // Player picks up a weapon
    player->PickWeapon("Shotgun");

    // Later in the game: we remove the AudioManager listener (e.g., during mute or shutdown)
    std::cout << "--- Audio system disabled ---\n";
    player->OnWeaponPicked.Remove(std::weak_ptr<AudioManager>(audio));

    player->PickWeapon("Rocket Launcher");

    // On level unload or player death, we clear all
    std::cout << "--- Level reload ---\n";
    player->OnWeaponPicked.RemoveAll();

    player->PickWeapon("Sniper Rifle"); // No listeners left

    return 0;
}

#pragma clang diagnostic pop