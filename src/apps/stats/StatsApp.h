#pragma once
#include "../../core/AppBase.h"
#include "../../core/DisplayManager.h"
#include "../../gotchi/GotchiDNA.h"
#include "../../gotchi/GotchiLineage.h"
#include <M5Unified.h>

enum class StatsTab : uint8_t { STATUS = 0, LINEAGE, HISTORY, COUNT };

struct CachedStats {
    uint8_t  hunger, energy, health;
    uint8_t  stage, gotchiType;
    uint32_t feedCount, playCount;
    uint8_t  dirtyness;
    bool     lightOn;
    uint32_t stageAgeMs;
    GotchiID       id;
    GotchiAncestor ancestors[5];
    GotchiHeritage heritage;
};

class StatsApp : public AppBase {
public:
    void inject(DisplayManager* display) { _display = display; }

    void init()                       override;
    void update(uint32_t deltaMs)     override;
    void suspend()                    override {}
    void resume()                     override {}
    void destroy()                    override {}
    bool onInput(const InputEvent& e) override;
    const char* getName() const       override { return "Stats"; }

private:
    DisplayManager* _display   = nullptr;
    StatsTab        _tab       = StatsTab::STATUS;
    bool            _needsDraw = true;
    CachedStats     _data{};

    void _loadFromNvs();
    void _drawFrame();
    void _drawTabBar(M5Canvas& c);
    void _drawStatus(M5Canvas& c);
    void _drawLineage(M5Canvas& c);
    void _drawHistory(M5Canvas& c);
    void _drawStatBar(M5Canvas& c, int y, const char* label,
                      uint8_t pct, uint32_t fillColor);
    void _drawDnaVial(M5Canvas& c, int x, int y,
                      uint16_t col1, uint16_t col2);
};
