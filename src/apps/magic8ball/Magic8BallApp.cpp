#include "Magic8BallApp.h"
#include <M5Unified.h>
#include "../../render/SpriteBlit.h"
#include "../../generated/sprites_8magicball.h"

void Magic8BallApp::init() {
    _game.reset();
    _drawAccum    = 0;
    _imuPollAccum = 0;
    _prevAccMag   = 1.0f;
}

bool Magic8BallApp::onInput(const InputEvent& e) {
    if (e.button == ButtonId::B && e.action == ButtonAction::LONG_PRESS) {
        if (_homeCallback) _homeCallback();
        return true;
    }
    if (e.button == ButtonId::A && e.action == ButtonAction::SHORT_PRESS) {
        if (_game.state() == BallState::RESULT) _game.reset();
        return true;
    }
    return false;
}

void Magic8BallApp::_pollImu(uint32_t deltaMs) {
    _imuPollAccum += deltaMs;
    if (_imuPollAccum < 50) return;
    _imuPollAccum = 0;

    float ax, ay, az;
    M5.Imu.getAccel(&ax, &ay, &az);
    float mag   = sqrtf(ax * ax + ay * ay + az * az);
    float delta = fabsf(mag - _prevAccMag);
    _prevAccMag = mag;
    _game.onAccelDelta(delta);
}

void Magic8BallApp::update(uint32_t deltaMs) {
    _pollImu(deltaMs);
    _game.update(deltaMs);

    _drawAccum += deltaMs;
    if (_drawAccum < 33) return; // ~30fps
    _drawAccum = 0;
    _draw();
}

void Magic8BallApp::_draw() {
    if (!_display->acquire(50)) return;
    auto& c = _display->canvas();

    static const uint8_t* const BALL_FRAMES[4] = {
        SPR_8BALL_F0, SPR_8BALL_F1, SPR_8BALL_F2, SPR_8BALL_F3
    };
    static const char* const RESULTS[13] = {
        "",
        "100% real",
        "Casi seguro",
        "Good vibes",
        "Si",
        "Dale una vuelta",
        "Ahora mismo no",
        "Sigue buscando",
        "Mejor no saberlo",
        "No",
        "Ni de broma",
        "Mala vibra",
        "Imposible",
    };

    c.fillScreen(0x0841);

    c.fillRect(0, 0, 240, 18, 0x0000);
    c.setTextColor(0xFFFF);
    c.setTextSize(1);
    c.drawCenterString("BOLA 8", 120, 4);
    c.setTextColor(0x4208);
    c.drawString("B: volver", 172, 4);

    SpritePalette pal;
    pal.transparent = 0x0841;
    pal.primary     = 0x0000;
    pal.secondary   = 0x2104;
    pal.dark        = 0x52AA;
    pal.accent      = 0xAD55;
    pal.color5      = 0xFFFF;

    constexpr int SCALE = 4;
    constexpr int SW    = BALL8_W * SCALE;
    int ballX = (240 - SW) / 2;
    int ballY = 26;
    drawPaletteSprite(c, BALL_FRAMES[_game.frame()], BALL8_W, BALL8_H, ballX, ballY, SCALE, pal);

    c.setTextSize(1);
    uint8_t state    = (uint8_t)_game.state();
    uint8_t resultId = _game.resultId();

    if (state == 3 && resultId >= 1 && resultId <= 12) {  // RESULT
        uint16_t textColor;
        if      (resultId <= 4)  textColor = 0x07E0;  // verde  (positivo)
        else if (resultId <= 8)  textColor = 0xFFE0;  // amarillo (neutro)
        else                     textColor = 0xF800;  // rojo   (negativo)

        c.fillRect(0, 108, 240, 27, 0x0000);
        c.setTextSize(2);
        c.setTextColor(textColor);
        c.drawCenterString(RESULTS[resultId], 120, 111);
        c.setTextSize(1);
        c.setTextColor(0x4208);
        c.drawCenterString("A: otra vez", 120, 128);
    } else if (state == 0) {  // IDLE
        c.fillRect(0, 108, 240, 27, 0x0000);
        c.setTextColor(0x4208);
        c.drawCenterString("Agita para consultar", 120, 120);
    }

    _display->push();
    _display->release();
}
