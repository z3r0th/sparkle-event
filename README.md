# About
Sparkle Event is a simple and lightweight, header-only C++ event system for games and real-time applications.
It provides a flexible observer pattern with support for:

- Standalone functions and lambdas
- Member functions with raw pointers and/or std::weak_ptr
- One-time bindings (BindOnce)
- Cleanup of expired weak references

This library is part of **[Sparkle Project](http://gitlab.com/sparkle-game-engine)** and is designed for game engines and 
gameplay systems that need fast, type-safe event dispatching without relying on external frameworks.

# Sparkle

**[Sparkle Project](http://gitlab.com/sparkle-game-engine)** is a small personal project to learn design and build a UI engine.
The project is currently private.

# Features

- 🚀 Header-only: Just include Event.h.
- 🧩 Type-safe: Events are strongly typed with template arguments.
- 🗑️ Cleanup: Expired weak pointers are removed on Raise.
- 🎯 One-shot events: BindOnce lets you attach callbacks that auto-remove after first execution.
- 🛠️ Binding separation: Bind lambda and member functions without exposing Raise function.

# Reference

| Method                    | Description                              |
|---------------------------|------------------------------------------|
| `GetBinder()`             | Get the binder. Public binding reference |
| `Bind(callback)`          | Bind a standalone lambda or function     |
| `Bind(callback, object*)` | Bind with an object pointer association  |
| `BindOnce(...)`           | Bind a one-time callback                 |
| `Remove(object*)`         | Remove all callbacks tied to object      |
| `RemoveAll()`             | Remove all bindings                      |
| `Cleanup()`               | Cleans up expired weak pointers.         |
| `Raise(args...)`          | Trigger the event                        |
| `Size()`                  | Number of objects observing this event   |
| `CallbackCount()`         | Total number of bound callbacks          |

# Installation

- Requires C++17 or later.

## Simple Copy

Copy Event.h into your project’s include directory. Add #include "Event.h" where needed.

## CMake

```cmake
FetchContent_Declare(
    sparkle-events
    GIT_REPOSITORY https://gitlab.com/sparkle-game-engine/sparkle-event.git
    GIT_TAG main
    FIND_PACKAGE_ARGS
)
set(SPARKLE_BUILD_TESTS OFF CACHE BOOL "" FORCE) # Don't compile tests
set(SPARKLE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # Don't compile examples
FetchContent_MakeAvailable(sparkle-events)

target_link_libraries(MY_PROJECT PRIVATE Sparkle::SparkleEvents)
```

# Examples

---
### Examples from Examples folder

You can run examples in the *examples* folder
- Player Health Update `PlayerHealthUpdate` (weak_ptr and Binder)
- Boss Tutorial `BossTutorial` (BindOnce)
- Menu Buttons `MenuButtons` (Lambda and Static functions)
- Day Night Cycle `DayNightCycle` (Member function)
- Player Weapon Pick `PlayerWeapon` (Remove and RemoveAll) 
---

## 1. Simple Event with Lambda

```c++
#include "Event.h"
using namespace Sparkle;

Event<int> OnScore;

OnScore.Bind([](int score) {
    std::cout << "Score: " << score << std::endl;
});

OnScore(100); // Output: Score: 100
```
### More
- Example: `MenuButtons`

## 2. Binding Member Functions

```c++
struct Score {
    void OnScore(int points) {
        std::cout << "Adding " << points << " points" << std::endl;
    }
};

Score score;
Event<int> OnScoreEvent;

// Bind member function to object
OnScoreEvent.Bind(&Score::OnScore, &score);
OnScoreEvent(25); // Output: Adding 25 points
```
### More
- Example: `DayNightCycle`

## 3. One-Time Binding

```c++
Event OnSpawnEvent;

OnSpawnEvent.BindOnce([]() {
    std::cout << "Spawn only once ";
});

OnSpawnEvent(); // Output: "Spawn only once "
OnSpawnEvent(); // Output: 
```
### More
- Example: `BossTutorial`

## 4. Binder

```c++
class Player {
    private:
        Event<int> OnDamageEvent;
    public:
        EventBinder<int>& OnDamage() { return OnDamageEvent.GetBinder(); }
        void TakeDamage(int amount) { OnDamageEvent(amount); }
};

Player player;
Player.OnDamage().Bind([]{ std::cout << "Player Took " << amount << " Damage" << std::endl; });
// Player.OnDamage().Raise(10) - this is illegal, we only expose the binder and not the event itself. 
Player.TakeDamage(10); // Output: Player Took 10 Damage;
```
### More
- Example: `MenuButtons`

## 5. Weak Pointers

```c++
#include "Event.h"
#include <iostream>
#include <memory>

using namespace Sparkle;

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
```

### Output
```terminaloutput
[Game] Player took 25 damage
[UI] Player HP: 75
[Game] Player took 10 damage
```
Run Example: `PlayerHealthUpdate`

# 6. Remove

```c++
struct Enemy { Event<int> OnDefeated; };
struct LootSystem { void DropLoot(int p){ std::cout << "Dropped loot worth " << p << std::endl; } };

Enemy enemy;
LootSystem loot;
enemy->OnDefeated.Bind(&LootSystem::DropLoot, loot);
enemy->OnDefeated(100); // Output: Dropped loot worth 100
enemy->OnDefeated.Remove(loot);       // Remove loot system
enemy->OnDefeated(50); // Output: 
```

### More
- Example: `PlayerWeapon`

# Tips

- Prefer weak_ptr over raw pointers for safety.
- Avoid high-frequency calls with very large numbers of bindings unless optimized.
- Use BindOnce for callbacks that only need to run once to avoid manual cleanup.

# Changelog

```terminaloutput
v0.9.0 Initial Release
```

# Roadmap 

- Multi Thread safety
- Performance improvements
- Handle to remove specific functions and avoid relying on object pointer

# License

MIT License. Free to use in commercial and non-commercial projects