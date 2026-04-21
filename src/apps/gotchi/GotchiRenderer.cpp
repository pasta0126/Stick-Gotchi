#include "GotchiRenderer.h"

using namespace m5avatar;

// The default Avatar Face is designed for a 320×240 canvas.
// M5Stick C Plus2 is 240×135 (landscape).
// Position offsets (unscaled virtual coords) to center the 320×240 face on the display:
//   left = -(320 - 240) / 2 = -40
//   top  = -(240 - 135) / 2 = -52
static constexpr int   POS_TOP   = -52;
static constexpr int   POS_LEFT  = -40;
static constexpr float FACE_SCALE = 0.6f; // ~192×144 px — fills most of 240×135

void GotchiRenderer::start(GotchiPet* pet) {
    _pet      = pet;
    _started  = true;
    _lastMood = Mood::NEUTRAL;
    _avatar.init();
    _avatar.setScale(FACE_SCALE);
    _avatar.setPosition(POS_TOP, POS_LEFT);
    _avatar.setColorPalette(_moodToColorPalette(Mood::NEUTRAL));
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
    Mood mood = _pet->mood();
    if (mood == _lastMood) return;
    _lastMood = mood;
    _avatar.setColorPalette(_moodToColorPalette(mood));
    _avatar.setExpression(_moodToExpression(mood));
    _avatar.setMouthOpenRatio(_moodToMouthOpen(mood));
}

// ── Mood → Avatar expression ──────────────────────────────────────────────────

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

ColorPalette GotchiRenderer::_moodToColorPalette(Mood mood) {
    ColorPalette p;
    // Each case sets face color (PRIMARY) and background (BACKGROUND).
    // WHITE and SHADOW inherit Avatar defaults.
    switch (mood) {
    case Mood::HAPPY:
    case Mood::LAUGHING:
        p.set(COLOR_PRIMARY,    TFT_YELLOW);
        p.set(COLOR_BACKGROUND, 0x2000); // dark amber
        break;
    case Mood::EXCITED:
        p.set(COLOR_PRIMARY,    TFT_ORANGE);
        p.set(COLOR_BACKGROUND, 0x2800); // dark orange
        break;
    case Mood::SAD:
        p.set(COLOR_PRIMARY,    0x7BDF); // steel blue
        p.set(COLOR_BACKGROUND, 0x0008); // midnight blue
        break;
    case Mood::SICK:
        p.set(COLOR_PRIMARY,    0x87E0); // pale green
        p.set(COLOR_BACKGROUND, 0x0200); // dark green
        break;
    case Mood::SCARED:
        p.set(COLOR_PRIMARY,    0xDEFB); // near-white/pale
        p.set(COLOR_BACKGROUND, 0x0809); // dark violet
        break;
    case Mood::SLEEPING:
        p.set(COLOR_PRIMARY,    0x8C71); // muted blue-gray
        p.set(COLOR_BACKGROUND, 0x0001); // near-black blue
        break;
    case Mood::ANGRY:
    case Mood::ANNOYED:
        p.set(COLOR_PRIMARY,    TFT_RED);
        p.set(COLOR_BACKGROUND, 0x1000); // dark red
        break;
    case Mood::PENSIVE:
        p.set(COLOR_PRIMARY,    0xCE59); // lavender
        p.set(COLOR_BACKGROUND, 0x0801); // dark purple
        break;
    case Mood::DIZZY:
        p.set(COLOR_PRIMARY,    0xD9DB); // light purple
        p.set(COLOR_BACKGROUND, 0x0802); // dark purple
        break;
    case Mood::STARTLED:
        p.set(COLOR_PRIMARY,    TFT_WHITE);
        p.set(COLOR_BACKGROUND, 0x0820); // very dark teal
        break;
    default: // NEUTRAL
        p.set(COLOR_PRIMARY,    TFT_WHITE);
        p.set(COLOR_BACKGROUND, TFT_BLACK);
        break;
    }
    return p;
}
