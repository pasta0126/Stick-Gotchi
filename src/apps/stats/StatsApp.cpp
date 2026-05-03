#include "StatsApp.h"
#include <Preferences.h>

// Solarpunk palette (matches ImuDemoApp / GotchiRenderer)
static constexpr uint32_t BG      = 0x040F02;
static constexpr uint32_t LIME    = 0x40FF20;
static constexpr uint32_t AMBER   = 0xFFCC00;
static constexpr uint32_t DIM     = 0x1A3010;
static constexpr uint32_t MUTED   = 0x2A5020;
static constexpr uint32_t RED_C   = 0xFF3020;
static constexpr uint32_t BLUE_C  = 0x20A0FF;
static constexpr uint32_t PALE   = 0xDCF5D0;

static constexpr int W      = DisplayManager::W;   // 240
static constexpr int H      = DisplayManager::H;   // 135
static constexpr int TAB_H  = 16;                  // tab bar height
static constexpr int BAR_X  = 58;                  // stat bar start x
static constexpr int BAR_W  = 134;                 // stat bar width
static constexpr int VAL_X  = 197;                 // value text start x

static const char* STAGE_NAMES[]  = { "EGG", "BABY", "YOUNG", "ADULT" };
static const char* BRANCH_NAMES[] = { "BLOB", "PLANT", "LIBRE" };
static const char* TAB_LABELS[]   = { "STATS", "LINEAGE", "HISTORY" };

static uint32_t barColor(uint8_t pct) {
    if (pct < 30) return RED_C;
    if (pct < 60) return AMBER;
    return LIME;
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void StatsApp::init() {
    _tab       = StatsTab::STATUS;
    _needsDraw = true;
    _loadFromNvs();
}

void StatsApp::update(uint32_t /*deltaMs*/) {
    if (!_needsDraw) return;
    _needsDraw = false;
    _drawFrame();
}

bool StatsApp::onInput(const InputEvent& e) {
    if (e.button == ButtonId::B && e.action == ButtonAction::SHORT_PRESS) {
        _tab = static_cast<StatsTab>(
            (static_cast<uint8_t>(_tab) + 1) %
            static_cast<uint8_t>(StatsTab::COUNT));
        _needsDraw = true;
        return true;
    }
    if (e.button == ButtonId::A && e.action == ButtonAction::SHORT_PRESS) {
        if (_menuCallback) _menuCallback();
        return true;
    }
    if (e.button == ButtonId::B && e.action == ButtonAction::LONG_PRESS) {
        if (_menuCallback) _menuCallback();
        return true;
    }
    return false;
}

// ── NVS load ──────────────────────────────────────────────────────────────────

void StatsApp::_loadFromNvs() {
    Preferences prefs;
    prefs.begin("gotchi", true);
    _data.hunger    = prefs.getUChar("h",  50);
    _data.energy    = prefs.getUChar("e",  50);
    _data.health    = prefs.getUChar("hp", 100);
    _data.stage     = prefs.getUChar("st", 0);
    _data.branch    = prefs.getUChar("br", 0);
    _data.feedCount = prefs.getUInt("fc",  0);
    _data.playCount = prefs.getUInt("pc",  0);
    _data.dirtyness  = prefs.getUChar("di", 0);
    _data.lightOn    = prefs.getBool("li",  true);
    _data.stageAgeMs = prefs.getUInt("sa",  0);
    prefs.end();

    GotchiLineage lineage;
    lineage.load(_data.id, _data.ancestors, _data.heritage);
}

// ── Drawing ───────────────────────────────────────────────────────────────────

void StatsApp::_drawFrame() {
    if (!_display->acquire(100)) return;
    M5Canvas& c = _display->canvas();

    c.fillScreen(BG);
    _drawTabBar(c);

    switch (_tab) {
    case StatsTab::STATUS:  _drawStatus(c);  break;
    case StatsTab::LINEAGE: _drawLineage(c); break;
    case StatsTab::HISTORY: _drawHistory(c); break;
    default: break;
    }

    _display->push();
    _display->release();
}

void StatsApp::_drawTabBar(M5Canvas& c) {
    int tabW = W / 3;
    for (int i = 0; i < 3; i++) {
        bool active = (static_cast<uint8_t>(_tab) == i);
        uint32_t bg  = active ? MUTED : BG;
        uint32_t fg  = active ? LIME  : MUTED + 0x101010;
        c.fillRect(i * tabW, 0, tabW, TAB_H, bg);
        c.setTextFont(1);
        c.setTextColor(active ? LIME : 0x2A5020, bg);
        c.drawCenterString(TAB_LABELS[i], i * tabW + tabW / 2, 4);
    }
    c.drawFastHLine(0, TAB_H, W, MUTED);
}

void StatsApp::_drawStatBar(M5Canvas& c, int y, const char* label,
                             uint8_t pct, uint32_t fillColor) {
    // Label
    c.setTextFont(1);
    c.setTextColor(PALE, BG);
    c.setCursor(4, y + 4);
    c.print(label);

    // Bar background
    c.fillRect(BAR_X,     y + 4, BAR_W, 8, DIM);
    c.drawRect(BAR_X - 1, y + 3, BAR_W + 2, 10, MUTED);

    // Bar fill
    int filled = (int)(pct * BAR_W / 100);
    if (filled > 0) c.fillRect(BAR_X, y + 4, filled, 8, fillColor);

    // Value
    c.setTextColor(fillColor, BG);
    c.setCursor(VAL_X, y + 4);
    c.printf("%3d", pct);
}

void StatsApp::_drawStatus(M5Canvas& c) {
    int y = TAB_H + 2;

    // Identity row
    c.setTextFont(1);
    c.setTextColor(AMBER, BG);
    c.setCursor(4, y);
    const char* stageName  = (_data.stage  < 4) ? STAGE_NAMES[_data.stage]   : "?";
    const char* branchName = (_data.branch < 3) ? BRANCH_NAMES[_data.branch] : "?";
    c.printf("Gen.%d  %s  %s", _data.id.generation, stageName, branchName);

    y += 12;
    c.drawFastHLine(0, y, W, MUTED);
    y += 2;

    // Stat bars
    _drawStatBar(c, y,      "HUNGER", _data.hunger, barColor(_data.hunger));
    _drawStatBar(c, y + 18, "ENERGY", _data.energy, barColor(_data.energy));
    _drawStatBar(c, y + 36, "HEALTH", _data.health, barColor(_data.health));

    y += 56;
    c.drawFastHLine(0, y, W, MUTED);
    y += 4;

    // DNA color swatches (primary + secondary)
    GotchiVisual vis = decodeVisual(_data.id.visual_seed);
    uint16_t col1 = hsvToRgb565(vis.hue_primary,   220, 200);
    uint16_t col2 = hsvToRgb565(vis.hue_secondary, 180, 160);
    c.fillRect(4,  y,     22, 22, (uint32_t)col1);
    c.fillRect(28, y + 4, 14, 14, (uint32_t)col2);
    c.drawRect(4,  y,     22, 22, MUTED);
    c.drawRect(28, y + 4, 14, 14, MUTED);

    // Seed + light state
    c.setTextFont(1);
    c.setTextColor(PALE, BG);
    c.setCursor(46, y);
    c.printf("Seed:0x%04X", _data.id.visual_seed);

    c.setCursor(46, y + 10);
    c.setTextColor(_data.lightOn ? AMBER : MUTED, BG);
    c.printf("Luz:%s", _data.lightOn ? "ON " : "OFF");

    // Dirty bar (compact)
    c.setTextColor(PALE, BG);
    c.setCursor(110, y + 10);
    c.print("Suci:");
    int dBar = (int)(_data.dirtyness * 60 / 100);
    c.fillRect(140, y + 11, dBar, 6, (_data.dirtyness > 70) ? RED_C : MUTED);
    c.drawRect(139, y + 10, 62, 8, MUTED);
}

void StatsApp::_drawDnaVial(M5Canvas& c, int x, int y,
                              uint16_t col1, uint16_t col2) {
    constexpr int VW     = 34;
    constexpr int CAP_H  = 5;
    constexpr int NECK_H = 4;
    constexpr int BODY_H = 52;

    // Decode col1 to RGB for glow calculations
    uint8_t r1 = ((col1 >> 11) & 0x1F) << 3;
    uint8_t g1 = ((col1 >> 5)  & 0x3F) << 2;
    uint8_t b1 =  (col1        & 0x1F) << 3;
    uint32_t glowC = ((uint32_t)(r1/8) << 16) | ((uint32_t)(g1/8) << 8) | (b1/8);
    uint32_t liqC  = ((uint32_t)(r1/5) << 16) | ((uint32_t)(g1/5) << 8) | (b1/5);

    // Outer glow
    c.drawRect(x - 1, y + CAP_H - 1, VW + 2, NECK_H + BODY_H + 12, glowC);

    // Metal cap
    int capX = x + 4, capW = VW - 8;
    c.fillRect(capX, y, capW, CAP_H, 0x606060);
    c.drawFastHLine(capX + 1, y + 1, capW - 2, 0xA0A0A0);
    c.drawFastHLine(capX + 1, y + 3, capW - 2, 0x303030);

    // Neck
    int neckX = x + 6, neckW = VW - 12, neckY = y + CAP_H;
    c.fillRect(neckX, neckY, neckW, NECK_H, 0x303030);
    c.drawFastVLine(neckX,            neckY, NECK_H, 0x606060);
    c.drawFastVLine(neckX + neckW - 1, neckY, NECK_H, 0x606060);

    // Body
    int bodyY = neckY + NECK_H;
    c.fillRect(x, bodyY, VW, BODY_H, 0x080808);
    c.drawRect(x, bodyY, VW, BODY_H, 0x404040);
    c.drawFastVLine(x + 1, bodyY + 1, BODY_H - 2, 0x505050);
    c.drawFastVLine(x + 2, bodyY + 1, BODY_H - 2, 0x282828);
    c.fillRect(x + 3, bodyY + 1, VW - 6, BODY_H - 2, liqC);

    // DNA helix — angular/geometric, two passes (back then front)
    int helixCX = x + VW / 2;
    int helixW  = VW / 2 - 5;
    int period  = 16;
    for (int pass = 0; pass < 2; pass++) {
        uint16_t hcol  = (pass == 0) ? col2 : col1;
        float    phase = (pass == 0) ? 3.14159f : 0.0f;
        int prevHx = -1, prevPy = -1;
        for (int hy = 0; hy < BODY_H - 2; hy += 3) {
            float t  = (float)hy / period * 3.14159f + phase;
            int   hx = helixCX + (int)(sinf(t) * helixW);
            int   py = bodyY + 1 + hy;
            if (prevHx >= 0) c.drawLine(prevHx, prevPy, hx, py, hcol);
            bool extreme = (fabsf(sinf(t)) > 0.85f);
            if (extreme) c.fillRect(hx - 1, py - 1, 3, 3, hcol);
            else         c.drawPixel(hx, py, hcol);
            prevHx = hx; prevPy = py;
        }
    }
    // Angular connectors at crossover points
    for (int hy = period / 2; hy < BODY_H - 2; hy += period) {
        int py = bodyY + 1 + hy;
        c.drawFastHLine(helixCX - helixW + 2, py, helixW * 2 - 4, 0x404040);
        c.drawPixel(helixCX - helixW + 3, py - 1, 0x505050);
        c.drawPixel(helixCX + helixW - 4, py - 1, 0x505050);
        c.drawPixel(helixCX - helixW + 3, py + 1, 0x505050);
        c.drawPixel(helixCX + helixW - 4, py + 1, 0x505050);
    }

    // Bubbles (deterministic from seed)
    uint32_t rng = (uint32_t)_data.id.visual_seed;
    for (int i = 0; i < 5; i++) {
        rng = rng * 1664525u + 1013904223u;
        int bx2 = x + 3 + (int)((rng & 0xFFu) % (uint32_t)(VW - 6));
        rng = rng * 1664525u + 1013904223u;
        int by2 = bodyY + 3 + (int)(((rng >> 8) & 0xFFu) % (uint32_t)(BODY_H - 6));
        c.drawCircle(bx2, by2, 1, 0x606060);
    }

    // Rounded bottom (converging trapezoid)
    int botY = bodyY + BODY_H;
    c.fillRect(x + 1, botY,     VW - 2, 3, 0x303030);
    c.fillRect(x + 3, botY + 3, VW - 6, 3, 0x252525);
    c.fillRect(x + 7, botY + 6, VW - 14, 2, 0x202020);
    uint32_t botGlow = ((uint32_t)(r1/6) << 16) | ((uint32_t)(g1/6) << 8) | (b1/6);
    c.drawFastHLine(x + 9, botY + 7, VW - 18, botGlow);
}

void StatsApp::_drawLineage(M5Canvas& c) {
    // Left panel: ancestor list (0..159). Right panel: DNA vial (162..239).
    constexpr int SPLIT   = 157;
    constexpr int VIAL_X  = 163;
    constexpr int VIAL_Y  = TAB_H + 4;

    GotchiVisual vis  = decodeVisual(_data.id.visual_seed);
    uint16_t     col1 = hsvToRgb565(vis.hue_primary,   220, 200);
    uint16_t     col2 = hsvToRgb565(vis.hue_secondary, 180, 160);

    // ── Right: DNA vial ────────────────────────────────────────────────────
    c.drawFastVLine(SPLIT, TAB_H, H - TAB_H, MUTED);
    _drawDnaVial(c, VIAL_X, VIAL_Y, col1, col2);

    // Seed label under vial
    c.setTextFont(1);
    c.setTextColor(MUTED + 0x1A1A1A, BG);
    c.setCursor(VIAL_X, VIAL_Y + 72);
    c.printf("0x%04X", _data.id.visual_seed);

    // ── Left: header ───────────────────────────────────────────────────────
    int y = TAB_H + 2;
    c.setTextColor(AMBER, BG);
    c.setCursor(4, y);
    c.printf("Gen.%d", _data.id.generation);

    y += 12;
    c.drawFastHLine(0, y, SPLIT, MUTED);
    y += 2;

    // ── Left: ancestor rows ────────────────────────────────────────────────
    for (int i = 0; i < 5 && y + 13 <= H; i++) {
        const GotchiAncestor& a = _data.ancestors[i];
        bool empty = (a.id.generation == 0 && a.id.visual_seed == 0 && a.days_lived == 0);

        if (empty) {
            c.setTextColor(DIM + 0x080808, BG);
            c.setCursor(4, y + 2);
            c.print("- vacio -");
        } else {
            GotchiVisual av = decodeVisual(a.id.visual_seed);
            uint16_t     ac = hsvToRgb565(av.hue_primary, 200, 180);
            c.fillCircle(7, y + 7, 4, (uint32_t)ac);

            c.setTextFont(1);
            c.setTextColor(PALE, BG);
            c.setCursor(14, y);
            const char* dc = (a.cause_of_death == 0) ? "hmb" :
                             (a.cause_of_death == 1) ? "sal" : "otr";
            c.printf("G%d %dd %s", a.id.generation, a.days_lived, dc);

            // Compact mood + health bars
            int mFill = (int)(a.avg_mood_pct   * 44 / 100);
            int hFill = (int)(a.avg_health_pct * 44 / 100);
            c.fillRect(14,      y + 9, mFill, 3, LIME);
            c.drawRect(13,      y + 8, 46,    5, MUTED);
            c.fillRect(64,      y + 9, hFill, 3, BLUE_C);
            c.drawRect(63,      y + 8, 46,    5, MUTED);
        }
        y += 18;
    }
}

void StatsApp::_drawHistory(M5Canvas& c) {
    int y = TAB_H + 2;

    // Header
    c.setTextFont(1);
    c.setTextColor(AMBER, BG);
    c.setCursor(4, y);
    c.printf("Historial — Gen.%d", _data.id.generation);
    y += 12;
    c.drawFastHLine(0, y, W, MUTED);
    y += 3;

    // Time in current stage
    uint32_t stageHours = _data.stageAgeMs / 3600000UL;
    uint32_t stageMins  = (_data.stageAgeMs % 3600000UL) / 60000UL;
    const char* stageName = (_data.stage < 4) ? STAGE_NAMES[_data.stage] : "?";
    c.setTextColor(PALE, BG);
    c.setCursor(4, y);
    c.printf("Etapa: %s  Tiempo: %luh %02lum", stageName, stageHours, stageMins);
    y += 12;

    // Feed / play counters
    c.setCursor(4, y);
    c.printf("Alimentado: %lu  Jugado: %lu", _data.feedCount, _data.playCount);
    y += 12;

    // Feed/play ratio bar
    c.setCursor(4, y);
    c.setTextColor(LIME, BG);
    c.print("F");
    c.setTextColor(BLUE_C, BG);
    c.print(" P");
    uint32_t total = _data.feedCount + _data.playCount;
    int feedW = (total > 0) ? (int)(_data.feedCount * 100 / total) : 50;
    c.fillRect(16, y + 1,         feedW,       8, LIME);
    c.fillRect(16 + feedW, y + 1, 100 - feedW, 8, BLUE_C);
    c.drawRect(15, y,             102,         10, MUTED);
    y += 14;

    c.drawFastHLine(0, y, W, MUTED);
    y += 3;

    // Heritage bonuses
    c.setTextColor(AMBER, BG);
    c.setCursor(4, y);
    c.print("Herencia acumulada:");
    y += 11;

    c.setTextColor(PALE, BG);
    c.setCursor(4, y);
    c.printf("Humor: +%.1f  Salud: +%.1f",
             _data.heritage.bonus_mood, _data.heritage.bonus_health);
    y += 11;

    c.setCursor(4, y);
    c.printf("Eclosion: +%.0f%%  Belleza: %s",
             _data.heritage.hatch_speedup * 100.0f,
             _data.heritage.beauty_bonus ? "SI" : "NO");
    y += 13;

    c.drawFastHLine(0, y, W, MUTED);
    y += 3;

    // Ancestor summary
    int activeAncestors = 0;
    uint32_t totalDays  = 0;
    for (int i = 0; i < 5; i++) {
        if (_data.ancestors[i].days_lived > 0 || _data.ancestors[i].id.visual_seed != 0) {
            activeAncestors++;
            totalDays += _data.ancestors[i].days_lived;
        }
    }
    c.setTextColor(PALE, BG);
    c.setCursor(4, y);
    c.printf("Ancestros: %d/5  Dias totales: %lu", activeAncestors, totalDays);
}
