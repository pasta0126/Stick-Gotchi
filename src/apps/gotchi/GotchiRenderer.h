#pragma once
#include <M5Unified.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "GotchiPet.h"
#include "GotchiSprites.h"
#include "../../gotchi/GotchiDNA.h"
#include "../../core/DisplayManager.h"

class GotchiRenderer {
public:
    void inject(DisplayManager* display) { _display = display; }
    void start(GotchiPet* pet);
    void suspend();
    void resume();
    void stop();

    void render();

    void setGaze(float h, float v = 0.0f);
    void setActionBarState(uint8_t selected, bool visible);

private:
    GotchiPet*      _pet      = nullptr;
    DisplayManager* _display  = nullptr;
    M5Canvas*       _canvas   = nullptr;
    bool            _started  = false;
    bool            _suspended = false;

    float       _gazeH    = 0.0f;
    float       _gazeV    = 0.0f;

    float       _posX = 120.0f;
    float       _posY = 62.0f;
    float       _velX = 0.5f;
    float       _velY = 0.2f;
    uint32_t    _moveAccumMs = 0;
    uint32_t    _nextDirChangeMs = 0;

    static constexpr int PLAY_X0 = 4;
    static constexpr int PLAY_X1 = 236;
    static constexpr int PLAY_Y0 = 22;
    static constexpr int PLAY_Y1 = 100;

    uint8_t      _selectedAction = 0;
    bool         _actionBarVisible = true;

    uint32_t    _lastFrameMs  = 0;
    uint8_t     _animFrame    = 0;
    uint32_t    _animAccumMs  = 0;
    static constexpr uint32_t FRAME_INTERVAL_MS = 500;

    TaskHandle_t _taskHandle = nullptr;
    static void  _renderTask(void* arg);

    void _drawFrame();
    void _drawBackground();
    void _drawEmote(Mood mood, int x, int y);
    void _updatePosition(uint32_t deltaMs);
    void _drawStatsBar();
    void _drawActionBar(uint8_t selected, bool visible);
    void _drawSleepZs(int cx, int cy);
    SpriteFrame _selectSprite(LifeStage stage, GotchiBranch branch, uint8_t frame);

    struct SpritePalette {
        uint16_t transparent;
        uint16_t primary;
        uint16_t secondary;
        uint16_t dark;
        uint16_t white;
    };

    SpritePalette _buildPalette(const GotchiVisual& vis, LifeStage stage, GotchiBranch branch);
    void _drawSprite(const uint8_t* data, uint8_t w, uint8_t h,
                     int x, int y, uint8_t scale, const SpritePalette& pal);
};
