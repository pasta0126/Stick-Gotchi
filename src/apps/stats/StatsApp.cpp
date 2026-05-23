#include "StatsApp.h"
#include <Preferences.h>

// Solarpunk palette
static constexpr uint32_t BG     = 0x040F02;
static constexpr uint32_t LIME   = 0x40FF20;
static constexpr uint32_t AMBER  = 0xFFCC00;
static constexpr uint32_t DIM    = 0x1A3010;
static constexpr uint32_t MUTED  = 0x2A5020;
static constexpr uint32_t BLUE_C = 0x20A0FF;
static constexpr uint32_t PALE   = 0xDCF5D0;

static constexpr int W     = DisplayManager::W;
static constexpr int H     = DisplayManager::H;
static constexpr int TAB_H = 16;

static const char* CREATURE_NAMES[] = { "Bytee", "Cthulhu", "Jack", "Lumi" };
static const char* TAB_LABELS[]     = { "DIARIO", "LINAJE" };

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void StatsApp::init() {
    _tab       = StatsTab::DIARY;
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
    prefs.begin("gotchi2", true);
    _data.creatureType      = (CreatureType)prefs.getUChar("ct", (uint8_t)CreatureType::BYTEE);
    _data.totalCompanionMs  = prefs.getUInt("tc", 0);
    _data.interactionsToday = prefs.getUInt("it", 0);
    _data.id.visual_seed    = prefs.getUShort("vs", 0);
    _data.id.generation     = prefs.getUChar("gn", 0);
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
    case StatsTab::DIARY:   _drawDiary(c);   break;
    case StatsTab::LINEAGE: _drawLineage(c); break;
    default: break;
    }

    _display->push();
    _display->release();
}

void StatsApp::_drawTabBar(M5Canvas& c) {
    int tabW = W / 2;
    for (int i = 0; i < 2; i++) {
        bool active = (static_cast<uint8_t>(_tab) == i);
        uint32_t bg = active ? MUTED : BG;
        c.fillRect(i * tabW, 0, tabW, TAB_H, bg);
        c.setTextFont(1);
        c.setTextColor(active ? LIME : MUTED + 0x101010, bg);
        c.drawCenterString(TAB_LABELS[i], i * tabW + tabW / 2, 4);
    }
    c.drawFastHLine(0, TAB_H, W, MUTED);
}

void StatsApp::_drawDiary(M5Canvas& c) {
    int y = TAB_H + 4;

    // Creature name + generation
    const char* cname = ((uint8_t)_data.creatureType < 4)
                        ? CREATURE_NAMES[(uint8_t)_data.creatureType] : "?";
    c.setTextFont(2);
    c.setTextColor(LIME, BG);
    c.setCursor(4, y);
    c.print(cname);

    c.setTextFont(1);
    c.setTextColor(AMBER, BG);
    c.setCursor(4, y + 18);
    c.printf("Gen. %d", _data.id.generation);

    // Visual seed color swatches
    GotchiVisual vis  = decodeVisual(_data.id.visual_seed);
    uint16_t     col1 = hsvToRgb565(vis.hue_primary,   220, 200);
    uint16_t     col2 = hsvToRgb565(vis.hue_secondary, 180, 160);
    c.fillRect(90, y,      26, 26, (uint32_t)col1);
    c.fillRect(118, y + 5, 16, 16, (uint32_t)col2);
    c.drawRect(90, y,      26, 26, MUTED);
    c.drawRect(118, y + 5, 16, 16, MUTED);
    c.setTextFont(1);
    c.setTextColor(DIM + 0x101010, BG);
    c.setCursor(90, y + 28);
    c.printf("0x%04X", _data.id.visual_seed);

    y += 40;
    c.drawFastHLine(0, y, W, MUTED);
    y += 4;

    // Time together
    uint32_t totalSec = _data.totalCompanionMs / 1000;
    uint32_t hours    = totalSec / 3600;
    uint32_t mins     = (totalSec % 3600) / 60;
    c.setTextColor(PALE, BG);
    c.setCursor(4, y);
    if (hours >= 24) {
        c.printf("Juntos: %lud %luh", hours / 24, hours % 24);
    } else {
        c.printf("Juntos: %luh %02lum", hours, mins);
    }
    y += 14;

    // Interactions today
    c.setTextColor(BLUE_C, BG);
    c.setCursor(4, y);
    c.printf("Hoy: %lu interacciones", _data.interactionsToday);
    y += 14;

    // Heritage summary
    c.drawFastHLine(0, y, W, MUTED);
    y += 4;
    c.setTextColor(AMBER, BG);
    c.setCursor(4, y);
    c.printf("Herencia  Humor:+%.1f  Salud:+%.1f",
             _data.heritage.bonus_mood, _data.heritage.bonus_health);
    y += 12;

    // Ancestor count
    int activeAnc = 0;
    for (int i = 0; i < 5; i++) {
        if (_data.ancestors[i].days_lived > 0 || _data.ancestors[i].id.visual_seed != 0)
            activeAnc++;
    }
    c.setTextColor(PALE, BG);
    c.setCursor(4, y);
    c.printf("Ancestros registrados: %d/5", activeAnc);
}

void StatsApp::_drawLineage(M5Canvas& c) {
    constexpr int SPLIT  = 157;
    constexpr int VIAL_X = 163;
    constexpr int VIAL_Y = TAB_H + 4;

    GotchiVisual vis  = decodeVisual(_data.id.visual_seed);
    uint16_t     col1 = hsvToRgb565(vis.hue_primary,   220, 200);
    uint16_t     col2 = hsvToRgb565(vis.hue_secondary, 180, 160);

    c.drawFastVLine(SPLIT, TAB_H, H - TAB_H, MUTED);
    _drawDnaVial(c, VIAL_X, VIAL_Y, col1, col2);

    c.setTextFont(1);
    c.setTextColor(MUTED + 0x1A1A1A, BG);
    c.setCursor(VIAL_X, VIAL_Y + 72);
    c.printf("0x%04X", _data.id.visual_seed);

    int y = TAB_H + 2;
    c.setTextColor(AMBER, BG);
    c.setCursor(4, y);
    c.printf("Gen.%d", _data.id.generation);

    y += 12;
    c.drawFastHLine(0, y, SPLIT, MUTED);
    y += 2;

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

            int mFill = (int)(a.avg_mood_pct   * 44 / 100);
            int hFill = (int)(a.avg_health_pct * 44 / 100);
            c.fillRect(14,  y + 9, mFill, 3, LIME);
            c.drawRect(13,  y + 8, 46,    5, MUTED);
            c.fillRect(64,  y + 9, hFill, 3, BLUE_C);
            c.drawRect(63,  y + 8, 46,    5, MUTED);
        }
        y += 18;
    }
}

void StatsApp::_drawDnaVial(M5Canvas& c, int x, int y,
                              uint16_t col1, uint16_t col2) {
    constexpr int VW     = 34;
    constexpr int CAP_H  = 5;
    constexpr int NECK_H = 4;
    constexpr int BODY_H = 52;

    uint8_t r1 = ((col1 >> 11) & 0x1F) << 3;
    uint8_t g1 = ((col1 >> 5)  & 0x3F) << 2;
    uint8_t b1 =  (col1        & 0x1F) << 3;
    uint32_t glowC = ((uint32_t)(r1/8) << 16) | ((uint32_t)(g1/8) << 8) | (b1/8);
    uint32_t liqC  = ((uint32_t)(r1/5) << 16) | ((uint32_t)(g1/5) << 8) | (b1/5);

    c.drawRect(x - 1, y + CAP_H - 1, VW + 2, NECK_H + BODY_H + 12, glowC);

    int capX = x + 4, capW = VW - 8;
    c.fillRect(capX, y, capW, CAP_H, 0x606060);
    c.drawFastHLine(capX + 1, y + 1, capW - 2, 0xA0A0A0);
    c.drawFastHLine(capX + 1, y + 3, capW - 2, 0x303030);

    int neckX = x + 6, neckW = VW - 12, neckY = y + CAP_H;
    c.fillRect(neckX, neckY, neckW, NECK_H, 0x303030);
    c.drawFastVLine(neckX,             neckY, NECK_H, 0x606060);
    c.drawFastVLine(neckX + neckW - 1, neckY, NECK_H, 0x606060);

    int bodyY = neckY + NECK_H;
    c.fillRect(x, bodyY, VW, BODY_H, 0x080808);
    c.drawRect(x, bodyY, VW, BODY_H, 0x404040);
    c.drawFastVLine(x + 1, bodyY + 1, BODY_H - 2, 0x505050);
    c.drawFastVLine(x + 2, bodyY + 1, BODY_H - 2, 0x282828);
    c.fillRect(x + 3, bodyY + 1, VW - 6, BODY_H - 2, liqC);

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
    for (int hy = period / 2; hy < BODY_H - 2; hy += period) {
        int py = bodyY + 1 + hy;
        c.drawFastHLine(helixCX - helixW + 2, py, helixW * 2 - 4, 0x404040);
        c.drawPixel(helixCX - helixW + 3, py - 1, 0x505050);
        c.drawPixel(helixCX + helixW - 4, py - 1, 0x505050);
        c.drawPixel(helixCX - helixW + 3, py + 1, 0x505050);
        c.drawPixel(helixCX + helixW - 4, py + 1, 0x505050);
    }

    uint32_t rng = (uint32_t)_data.id.visual_seed;
    for (int i = 0; i < 5; i++) {
        rng = rng * 1664525u + 1013904223u;
        int bx2 = x + 3 + (int)((rng & 0xFFu) % (uint32_t)(VW - 6));
        rng = rng * 1664525u + 1013904223u;
        int by2 = bodyY + 3 + (int)(((rng >> 8) & 0xFFu) % (uint32_t)(BODY_H - 6));
        c.drawCircle(bx2, by2, 1, 0x606060);
    }

    int botY = bodyY + BODY_H;
    c.fillRect(x + 1, botY,     VW - 2, 3, 0x303030);
    c.fillRect(x + 3, botY + 3, VW - 6, 3, 0x252525);
    c.fillRect(x + 7, botY + 6, VW - 14, 2, 0x202020);
    uint32_t botGlow = ((uint32_t)(r1/6) << 16) | ((uint32_t)(g1/6) << 8) | (b1/6);
    c.drawFastHLine(x + 9, botY + 7, VW - 18, botGlow);
}
