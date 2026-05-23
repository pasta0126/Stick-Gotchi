#pragma once
#include "../../core/AppBase.h"
#include "../../core/DisplayManager.h"
#include "../../gotchi/GotchiDNA.h"
#include "../../gotchi/GotchiLineage.h"
#include "../../gotchi/GotchiBehaviour.h"
#include <M5Unified.h>

enum class StatsTab : uint8_t { DIARY = 0, LINEAGE, COUNT };

struct DiaryData {
    CreatureType creatureType   = CreatureType::BYTEE;
    uint32_t     totalCompanionMs = 0;
    uint32_t     interactionsToday = 0;
    GotchiID     id;
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
    StatsTab        _tab       = StatsTab::DIARY;
    bool            _needsDraw = true;
    DiaryData       _data{};

    void _loadFromNvs();
    void _drawFrame();
    void _drawTabBar(M5Canvas& c);
    void _drawDiary(M5Canvas& c);
    void _drawLineage(M5Canvas& c);
    void _drawDnaVial(M5Canvas& c, int x, int y, uint16_t col1, uint16_t col2);
};
