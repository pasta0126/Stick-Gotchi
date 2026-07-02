#include "CoinFlipApp.h"
#include "../../render/SpriteBlit.h"
#include "../../generated/sprites_coin.h"

void CoinFlipApp::init() {
    _game      = FlipCoinGame();
    _drawAccum = 0;
}

bool CoinFlipApp::onInput(const InputEvent& e) {
    if (e.button == ButtonId::B && e.action == ButtonAction::LONG_PRESS) {
        if (_homeCallback) _homeCallback();
        return true;
    }
    if (e.button == ButtonId::A && e.action == ButtonAction::SHORT_PRESS) {
        _game.onBtnA();
        return true;
    }
    return false;
}

void CoinFlipApp::update(uint32_t deltaMs) {
    _game.update(deltaMs);

    _drawAccum += deltaMs;
    if (_drawAccum < 33) return; // ~30fps
    _drawAccum = 0;
    _draw();
}

void CoinFlipApp::_draw() {
    if (!_display->acquire(50)) return;
    auto& c = _display->canvas();
    c.fillScreen(0x0000);

    static const uint8_t* const COIN_FRAMES[12] = {
        SPR_COIN_F0, SPR_COIN_F1, SPR_COIN_F2,  SPR_COIN_F3,
        SPR_COIN_F4, SPR_COIN_F5, SPR_COIN_F6,  SPR_COIN_F7,
        SPR_COIN_F8, SPR_COIN_F9, SPR_COIN_F10, SPR_COIN_F11
    };

    c.fillRect(0, 0, 240, 18, 0x0000);
    c.setTextColor(0xFEA0);
    c.setTextSize(1);
    c.drawCenterString("FLIP COIN", 120, 4);
    c.setTextColor(0x4208);
    c.drawString("B: volver", 172, 4);

    SpritePalette pal;
    pal.transparent = 0x0000;
    pal.primary     = 0x7207;  // sombra
    pal.secondary   = 0xDD2C;  // oro calido
    pal.dark        = 0xFEA8;  // oro brillante
    pal.accent      = 0xFFC8;  // highlight

    constexpr int SCALE = 4;
    constexpr int SW = COIN_W * SCALE;
    int coinX = (240 - SW) / 2;
    int coinY = 28;
    drawPaletteSprite(c, COIN_FRAMES[_game.coinFrame()], COIN_W, COIN_H, coinX, coinY, SCALE, pal);

    c.setTextSize(1);
    if (_game.state() == FlipState::RESULT) {
        c.setTextSize(2);
        c.setTextColor(_game.isHeads() ? (uint16_t)0xFEA8 : (uint16_t)0xFFFF);
        c.drawCenterString(_game.isHeads() ? "CARA" : "CRUZ", 120, 100);
        c.setTextSize(1);
        c.setTextColor(0x4208);
        c.drawCenterString("A: otra vez", 120, 122);
    } else {
        c.setTextColor(0x4208);
        c.drawCenterString("A: lanzar", 120, 115);
    }

    _display->push();
    _display->release();
}
