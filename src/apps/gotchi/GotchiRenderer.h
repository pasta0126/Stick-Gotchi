#pragma once
#include <Avatar.h>
#include "GotchiPet.h"

class GotchiRenderer {
public:
    void start(GotchiPet* pet);
    void suspend();
    void resume();
    void stop();

    // Sync expression + palette when mood changes. Call from GotchiApp::update().
    void updateExpression();

    // Drive eye gaze from IMU tilt: horizontal -1..1 (neg=left, pos=right), vertical -1..1.
    void setGaze(float horizontal, float vertical = 0.0f);

private:
    m5avatar::Avatar _avatar;
    GotchiPet*       _pet      = nullptr;
    Mood             _lastMood = Mood::NEUTRAL;
    bool             _lastTemp = false;
    bool             _started  = false;

    static m5avatar::Expression  _moodToExpression(Mood mood);
    static float                 _moodToMouthOpen(Mood mood);
    // isTemp: true when a transient emote is active → apply colour; false = white on black.
    static m5avatar::ColorPalette _moodToColorPalette(Mood mood, bool isTemp);
};
