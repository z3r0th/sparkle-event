#include "Sparkle/Event.h"
#include <iostream>
#include <string>

using namespace Sparkle;

// Features Lambda Bind and Binder

/*
 * Binding to a lambda function or a static function
 * to quick map button callbacks and interactions
 *
```terminaloutput
[UI] 'Audio' slide to 0.5.
[Game] Audio Volume 0.5
[UI] 'Start Game' clicked.
[Game] Initializing level, loading assets...
[UI] 'Quit' clicked.
[Game] Quitting... Saving progress and closing.
--- Player clicks again ---
[UI] 'Quit' clicked.
```
 */

//
// Example: Game UI button system
// Lambda Bind and Binder
//
class Button {
private:
    std::string label;
    Event<> OnClickEvent{ "OnClick" };

public:
    explicit Button(std::string label) : label(std::move(label)) {}

    void Click() {
        std::cout << "[UI] '" << label << "' clicked." << std::endl;
        OnClickEvent();
    }

    EventBinder<>& OnClick() { return OnClickEvent.GetBinder(); }
};

class SliderButton {
private:
    std::string label;
    Event<float> OnSlideEvent{ "OnSlide" };

public:
    explicit SliderButton(std::string label) : label(std::move(label)) {}

    void Slide(float value) {
        std::cout << "[UI] '" << label << "' slide to " << value << "." << std::endl;
        OnSlideEvent(value);
    }

    EventBinder<float>& OnSlide() { return OnSlideEvent.GetBinder(); }
};

static void AdjustAudio(float volume) {
    std::cout << "[Game] Audio Volume " << volume << std::endl;
}

int main() {
    Button startButton("Start Game");
    Button quitButton("Quit");
    SliderButton audioSlider("Audio");

    // Bind lambda to start button
    startButton.OnClick().Bind([]() {
        std::cout << "[Game] Initializing level, loading assets...\n";
    });

    // Bind lambda to quit button, only once
    quitButton.OnClick().BindOnce([]() {
        std::cout << "[Game] Quitting... Saving progress and closing.\n";
    });

    audioSlider.OnSlide().Bind(&AdjustAudio);

    // Simulate UI interactions
    audioSlider.Slide(0.5);
    startButton.Click();
    quitButton.Click();

    std::cout << "--- Player clicks again ---\n";
    quitButton.Click();  // won't trigger the game method because BindOnce was used

    return 0;
}