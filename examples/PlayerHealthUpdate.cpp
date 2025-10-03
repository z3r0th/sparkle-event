#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"
#include "Sparkle/Event.h"
#include <iostream>
#include <memory>

using namespace Sparkle;

// Using weak_ptr and Binder features

/*
 *
 * The Player class owns an Event<int> named OnHealthChanged.
 * A UIHealthBar subscribes to this event using a weak_ptr.
 * When the UI element is destroyed, its weak_ptr expires automatically.
 * The next time Damage() triggers the event, the expired listener is cleaned up — no crashes, no leaks, no crashes
 *
```terminaloutput
[Game] Player took 25 damage
[UI] Player HP: 75
[Game] Player took 10 damage
Active observers: 0
```
 */

//
// Game scenario: Health bar HUD update
// Weak_ptr and Binder
//
struct UIHealthBar {
    void OnHealthChanged(int newHP) {
        std::cout << "[UI] Player HP: " << newHP << "\n";
    }
};

struct Player {
    int hp{100};
    Event<int> OnHealthChangedEvent{"HealthChanged"};

    void Damage(int amount) {
        hp -= amount;
        std::cout << "[Game] Player took " << amount << " damage\n";
        OnHealthChangedEvent(hp);
    }

    EventBinder<int>& OnHealthChanged() { return OnHealthChangedEvent.GetBinder(); }
};

int main() {
    Player player;
    {
        // UI element created (e.g., on HUD spawn)
        auto ui = std::make_shared<UIHealthBar>();

        // Bind the UI to player's health change event
        player.OnHealthChanged().Bind(&UIHealthBar::OnHealthChanged, std::weak_ptr<UIHealthBar>(ui));

        player.Damage(25); // [Game] Player took 25 damage
        // [UI] Player HP: 75
    } // UI element destroyed (e.g., HUD closed)

    // Next event call is safe — UI expired, callback auto-removed
    player.Damage(10); // [Game] Player took 10 damage
}
#pragma clang diagnostic pop