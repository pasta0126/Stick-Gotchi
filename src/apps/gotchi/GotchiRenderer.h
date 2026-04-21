#pragma once
#include <Avatar.h>
#include "GotchiPet.h"

// Wraps M5Stack-Avatar. The Avatar library manages its own FreeRTOS task.
// suspend() stops the task; resume() restarts it so the menu can draw freely.
class GotchiRenderer {
public:
    void start(GotchiPet* pet);
    void suspend();   // suspends Avatar FreeRTOS task (menu open)
    void resume();    // resumes Avatar FreeRTOS task (menu close)
    void stop();      // stops Avatar task on app destroy

    // Call from GotchiApp::update() to sync expression when mood changes.
    void updateExpression();

private:
    m5avatar::Avatar _avatar;
    GotchiPet*       _pet      = nullptr;
    Mood             _lastMood = Mood::NEUTRAL;
    bool             _started  = false;

    static m5avatar::Expression  _moodToExpression(Mood mood);
    static float                 _moodToMouthOpen(Mood mood);
    static m5avatar::ColorPalette _moodToColorPalette(Mood mood);
};
