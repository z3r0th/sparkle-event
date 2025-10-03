#include "Sparkle/Event.h"
#include <iostream>
#include <memory>
#include <string>

using namespace Sparkle;

//
// Example: GameWorld notifying about day/night cycle changes.
//
enum DayNightState
{
    Day,
    Night
};

class GameWorld {
private:
    Event<DayNightState> OnDayNightChangedEvent{"OnDayNightChanged"};

public:
    void SetDay(DayNightState dayState) {
        std::cout << "[World] Time changed: " << (dayState == DayNightState::Day  ? "Day" : "Night") << "\n";
        OnDayNightChangedEvent(dayState);
    }

    EventBinder<DayNightState>& OnDayNightChanged() { return OnDayNightChangedEvent.GetBinder(); }
};

//
// Enemy that reacts to world events.
//
class Enemy : public std::enable_shared_from_this<Enemy> {
private:
    std::string name;
    bool active = true;

    void OnWorldTimeChanged(DayNightState dayState) {
        if (!active) return;

        if (dayState == DayNightState::Day)
            std::cout << "[Enemy] " << name << " hides from the sun.\n";
        else
            std::cout << "[Enemy] " << name << " emerges from the shadows.\n";
    }

public:
    explicit Enemy(std::string name) : name(std::move(name)) {}

    void RegisterToWorld(GameWorld& world) {
        // Capture weak_ptr to prevent extending lifetime
        std::weak_ptr<Enemy> weakSelf = shared_from_this();
        world.OnDayNightChanged().Bind(&Enemy::OnWorldTimeChanged, weakSelf);
    }

    void Destroy() {
        active = false;
        std::cout << "[Enemy] " << name << " destroyed.\n";
    }
};

int main() {
    GameWorld world;

    {
        auto enemy = std::make_shared<Enemy>("Goblin");
        enemy->RegisterToWorld(world);

        world.SetDay(DayNightState::Night); // Goblin reacts
        world.SetDay(DayNightState::Day);  // Goblin reacts again

        // Enemy goes out of scope here
        enemy->Destroy();
    }

    std::cout << "--- Time passes after enemy expired ---\n";
    world.SetDay(DayNightState::Night); // Callback safely detects expired weak_ptr

    return 0;
}
