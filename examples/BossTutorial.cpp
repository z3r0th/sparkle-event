#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"
#include "Sparkle/Event.h"
#include <iostream>

using namespace Sparkle;

// Using BindOnce feature

/*
 *
 * The BindOnce connection ensures the ShowBossIntro() runs only on the first event.
 * The callback is automatically removed right after execution.
 *
```terminaloutput
[Boss] Appears on screen!
[UI] Showing 'Beware the Boss!' message.
[Boss] Appears on screen!
```
 *
 */

//
// Game scenario: a boss fight introduction
// BindOnce
//
struct Boss {
    Event<> OnFirstSpawn{"OnFirstSpawn"};

    void Spawn() {
        std::cout << "[Boss] Appears on screen!\n";
        OnFirstSpawn(); // Trigger event
    }
};

struct TutorialSystem {
    void ShowBossIntro() {
        std::cout << "[UI] Showing 'Beware the Boss!' message.\n";
    }
};

int main() {
    Boss boss;
    TutorialSystem tutorial;

    // Bind tutorial message to play only once (first spawn)
    boss.OnFirstSpawn.BindOnce(&TutorialSystem::ShowBossIntro, &tutorial);

    // First spawn → triggers the tutorial
    boss.Spawn();

    // Second spawn → tutorial not shown again
    boss.Spawn();

    return 0;
}

#pragma clang diagnostic pop