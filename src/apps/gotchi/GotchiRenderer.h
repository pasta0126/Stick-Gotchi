#pragma once
#include <M5Unified.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "GotchiPet.h"
#include "GotchiSprites.h"
#include "../../gotchi/GotchiDNA.h"
#include "../../core/DisplayManager.h"

enum class LifeStage : uint8_t {
    EGG = 0, BABY = 1, YOUNG = 2, ADULT = 3
};

class GotchiRenderer {
public:
    void inject(DisplayManager* display) { _display = display; }
    void start(GotchiPet* pet, LifeStage stage);
    void suspend();
    void resume();
    void stop();

    void render();

    void setGaze(float h, float v = 0.0f);
    void setActionBarState(uint8_t selected, bool visible);
    void setLifeStage(LifeStage stage);

private:
    GotchiPet*      _pet      = nullptr;
    DisplayManager* _display  = nullptr;
    M5Canvas*       _canvas   = nullptr;
    bool            _started  = false;
    bool            _suspended = false;

    LifeStage   _stage    = LifeStage::EGG;
    float       _gazeH    = 0.0f;
    float       _gazeV    = 0.0f;

    uint8_t      _selectedAction = 0;
    bool         _actionBarVisible = true;

    uint32_t    _lastFrameMs  = 0;
    uint8_t     _animFrame    = 0;
    uint32_t    _animAccumMs  = 0;
    static constexpr uint32_t FRAME_INTERVAL_MS = 500;

    TaskHandle_t _taskHandle = nullptr;
    static void  _renderTask(void* arg);

    void _drawFrame();
    void _drawBody(const GotchiVisual& vis, uint8_t frame);
    void _drawFace(Mood mood, float gazeH, float gazeV, int cx, int cy, int scale);
    void _drawStatsBar();
    void _drawActionBar();
    void _drawSleepZs(int cx, int cy);

    struct SpritePalette {
        uint16_t transparent;
        uint16_t primary;
        uint16_t secondary;
        uint16_t dark;
        uint16_t white;
    };

    SpritePalette _buildPalette(const GotchiVisual& vis);
    void _drawSprite(const uint8_t* data, uint8_t w, uint8_t h,
                     int x, int y, uint8_t scale, const SpritePalette& pal);
};
