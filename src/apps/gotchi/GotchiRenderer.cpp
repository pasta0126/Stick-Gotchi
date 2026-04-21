#include "GotchiRenderer.h"

using namespace m5avatar;

// Position offsets center the virtual 320×240 face canvas on the 240×135 display.
static constexpr int   POS_TOP    = -52;
static constexpr int   POS_LEFT   = -40;
static constexpr float FACE_SCALE =  0.6f; // ~192×144 px

void GotchiRenderer::start(GotchiPet* pet) {
    _pet      = pet;
    _started  = true;
    _lastMood = Mood::NEUTRAL;
    _lastTemp = false;
    _avatar.init();
    _avatar.setScale(FACE_SCALE);
    _avatar.setPosition(POS_TOP, POS_LEFT);
    _avatar.setColorPalette(_moodToColorPalette(Mood::NEUTRAL, false));
    _avatar.setExpression(Expression::Neutral);
}

void GotchiRenderer::suspend() {
    if (_started) _avatar.suspend();
}

void GotchiRenderer::resume() {
    if (_started) _avatar.resume();
}

void GotchiRenderer::stop() {
    if (_started) {
        _avatar.stop();
        _started = false;
    }
}

void GotchiRenderer::updateExpression() {
    if (!_pet || !_started) return;
    Mood mood  = _pet->mood();
    bool isTemp = _pet->isTempMood();
    if (mood == _lastMood && isTemp == _lastTemp) return;
    _lastMood = mood;
    _lastTemp = isTemp;
    // Emotes fill the screen; base state stays compact
    _avatar.setScale(isTemp ? 0.85f : 0.60f);
    _avatar.setColorPalette(_moodToColorPalette(mood, isTemp));
    _avatar.setExpression(_moodToExpression(mood));
    _avatar.setMouthOpenRatio(_moodToMouthOpen(mood));
}

void GotchiRenderer::setGaze(float horizontal, float vertical) {
    if (!_started) return;
    _avatar.setRightGaze(vertical, horizontal);
    _avatar.setLeftGaze(vertical, horizontal);
}

// ── Expression mapping ────────────────────────────────────────────────────────

Expression GotchiRenderer::_moodToExpression(Mood mood) {
    switch (mood) {
    case Mood::HAPPY:
    case Mood::EXCITED:
    case Mood::LAUGHING:  return Expression::Happy;
    case Mood::SAD:
    case Mood::SICK:
    case Mood::SCARED:    return Expression::Sad;
    case Mood::SLEEPING:  return Expression::Sleepy;
    case Mood::PENSIVE:
    case Mood::DIZZY:
    case Mood::STARTLED:  return Expression::Doubt;
    case Mood::ANGRY:
    case Mood::ANNOYED:   return Expression::Angry;
    default:              return Expression::Neutral;
    }
}

float GotchiRenderer::_moodToMouthOpen(Mood mood) {
    switch (mood) {
    case Mood::LAUGHING:  return 0.8f;
    case Mood::SCARED:
    case Mood::STARTLED:  return 0.6f;
    case Mood::EXCITED:   return 0.4f;
    default:              return 0.0f;
    }
}

// ── Colour palette ────────────────────────────────────────────────────────────
// Base state (isTemp=false): always white face on black — the gotchi at rest.
// Temp emote (isTemp=true):  vivid colour matching the emotion.

ColorPalette GotchiRenderer::_moodToColorPalette(Mood mood, bool isTemp) {
    ColorPalette p;
    // Default: white face, black background
    p.set(COLOR_PRIMARY,    TFT_WHITE);
    p.set(COLOR_BACKGROUND, TFT_BLACK);

    if (!isTemp) return p; // base/status moods → monochrome

    switch (mood) {
    case Mood::HAPPY:
    case Mood::LAUGHING:
        p.set(COLOR_PRIMARY,    TFT_YELLOW);
        p.set(COLOR_BACKGROUND, 0x2000); // dark amber
        break;
    case Mood::EXCITED:
        p.set(COLOR_PRIMARY,    0xFD20); // orange
        p.set(COLOR_BACKGROUND, 0x2800);
        break;
    case Mood::SAD:
        p.set(COLOR_PRIMARY,    0x7BDF); // steel blue
        p.set(COLOR_BACKGROUND, 0x0008);
        break;
    case Mood::SICK:
        p.set(COLOR_PRIMARY,    0x87E0); // pale green
        p.set(COLOR_BACKGROUND, 0x0200);
        break;
    case Mood::SCARED:
        p.set(COLOR_PRIMARY,    0xDEFB); // pale/ghostly
        p.set(COLOR_BACKGROUND, 0x0809);
        break;
    case Mood::SLEEPING:
        p.set(COLOR_PRIMARY,    0x8C71); // muted blue-gray
        p.set(COLOR_BACKGROUND, 0x0001);
        break;
    case Mood::ANGRY:
    case Mood::ANNOYED:
        p.set(COLOR_PRIMARY,    TFT_RED);
        p.set(COLOR_BACKGROUND, 0x1000); // dark red
        break;
    case Mood::PENSIVE:
        p.set(COLOR_PRIMARY,    0xCE59); // lavender
        p.set(COLOR_BACKGROUND, 0x0801);
        break;
    case Mood::DIZZY:
        p.set(COLOR_PRIMARY,    0xD9DB); // light purple
        p.set(COLOR_BACKGROUND, 0x0802);
        break;
    case Mood::STARTLED:
        p.set(COLOR_PRIMARY,    TFT_WHITE);
        p.set(COLOR_BACKGROUND, 0x0820); // dark teal flash
        break;
    default:
        break;
    }
    return p;
}
