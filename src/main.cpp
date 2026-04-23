#include <M5Unified.h>

#include "core/ButtonManager.h"
#include "core/DisplayManager.h"
#include "core/AppManager.h"
#include "menu/MenuOverlay.h"
#include "ble/BleService.h"
#include "apps/gotchi/GotchiApp.h"
#include "apps/gotchi/MiniGames.h"
#include "apps/imudemo/ImuDemoApp.h"

// ── Singletons ────────────────────────────────────────────────────────────────
static ButtonManager  buttons;
static DisplayManager display;
static AppManager     apps;
static MenuOverlay    menu;
static BleService     ble;

// ── App instances ─────────────────────────────────────────────────────────────
static GotchiApp  gotchiApp;
static ImuDemoApp imuDemoApp; // mode is set by menu before launch

// ── Icon lambdas ──────────────────────────────────────────────────────────────

static void iconGotchi(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int r = sz / 2;
    c.drawCircle(cx, cy, r, col);
    c.fillCircle(cx - r/3, cy - r/5, 2, col);
    c.fillCircle(cx + r/3, cy - r/5, 2, col);
    c.drawLine(cx - r/3, cy + r/5, cx - r/6, cy + r/3, col);
    c.drawLine(cx - r/6, cy + r/3, cx + r/6, cy + r/3, col);
    c.drawLine(cx + r/6, cy + r/3, cx + r/3, cy + r/5, col);
}

static void iconBars(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int bw = max(4, sz/5), base = cy + sz/2 - 2;
    int x0 = cx - sz/2 + 2, gap = max(2, (sz - 3*bw) / 2);
    c.fillRect(x0,             base - sz*2/4, bw, sz*2/4, col);
    c.fillRect(x0 + bw + gap,  base - sz*3/4, bw, sz*3/4, col);
    c.fillRect(x0 + 2*(bw+gap),base - sz*1/4, bw, sz*1/4, col);
    c.drawFastHLine(cx - sz/2, base+1, sz, col);
}

static void iconCompass(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int r = sz/2 - 2;
    c.drawCircle(cx, cy, r, col);
    // N arrow (pointing up = north)
    c.fillTriangle(cx, cy - r + 4, cx - 5, cy + 4, cx + 5, cy + 4, col);
    // S dot
    c.fillCircle(cx, cy + r - 4, 2, col);
    // crosshair ticks
    c.drawFastHLine(cx - r, cy, r*2, col);
    c.drawFastVLine(cx, cy - r, r*2, col);
}

static void iconWave(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    // Sine wave icon for gyro
    int pts = 24, w = sz - 4;
    for (int i = 0; i < pts - 1; i++) {
        float t0 = (float)i     / (pts-1) * 2 * 3.14159f;
        float t1 = (float)(i+1) / (pts-1) * 2 * 3.14159f;
        int x0 = cx - w/2 + i     * w / (pts-1);
        int x1 = cx - w/2 + (i+1) * w / (pts-1);
        int y0 = cy - (int)(sinf(t0) * sz/3);
        int y1 = cy - (int)(sinf(t1) * sz/3);
        c.drawLine(x0, y0, x1, y1, col);
    }
}

static void iconCoin(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int r = sz / 2 - 2;
    c.fillCircle(cx, cy, r, col);
    c.fillCircle(cx, cy, r * 5 / 8, (uint32_t)0x000000);
    c.drawCircle(cx, cy, r, col);
}

static void iconReboot(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int r = sz/2 - 2;
    for (int a = 40; a <= 320; a += 6) {
        float rad = a * 3.14159f / 180.0f;
        c.fillCircle(cx + (int)(cosf(rad)*r), cy + (int)(sinf(rad)*r), 1, col);
    }
    float rad = 320.0f * 3.14159f / 180.0f;
    int ax = cx + (int)(cosf(rad)*r), ay = cy + (int)(sinf(rad)*r);
    c.fillTriangle(ax, ay-6, ax+6, ay+1, ax-4, ay+4, col);
}

// ─────────────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);

    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(1);
    M5.Display.setBrightness(180);

    BleService::initStack();
    display.begin();
    buttons.begin(700);

    imuDemoApp.inject(&display);
    gotchiApp.injectRenderer(&display);

    gotchiApp.setMenuCallback([]()  { menu.open(); });
    imuDemoApp.setMenuCallback([]() { menu.open(); });

    menu.begin(buttons, display, apps);

    // ── IMU submenu items ──────────────────────────────────────────────────
    std::vector<MenuItem> imuChildren = {
        { "Accelerometer", MenuItemType::APP,
          []() -> AppBase* { imuDemoApp.setMode(ImuMode::ACCEL); return &imuDemoApp; },
          nullptr, iconBars, {} },
        { "Gyroscope",     MenuItemType::APP,
          []() -> AppBase* { imuDemoApp.setMode(ImuMode::GYRO);  return &imuDemoApp; },
          nullptr, iconWave, {} },
        { "Orientation",   MenuItemType::APP,
          []() -> AppBase* { imuDemoApp.setMode(ImuMode::ORIENT); return &imuDemoApp; },
          nullptr, iconCompass, {} },
    };

    std::vector<MenuItem> gameChildren = {
        { "Lanzar Moneda", MenuItemType::ACTION,
          nullptr,
          []() { gotchiApp.startMiniGame(MiniGameId::FLIP_COIN); },
          iconCoin, {} },
    };

    menu.addItem({ "Stick Gotchi", MenuItemType::APP,
                   []() -> AppBase* { return &gotchiApp; },
                   nullptr, iconGotchi, {} });
    menu.addItem({ "Jugar",        MenuItemType::SUBMENU,
                   nullptr, nullptr, iconCoin, gameChildren });
    menu.addItem({ "IMU Sensors",  MenuItemType::SUBMENU,
                   nullptr, nullptr, iconBars, imuChildren });
    menu.addItem({ "Reboot",       MenuItemType::ACTION,
                   nullptr, []() { ESP.restart(); }, iconReboot, {} });

    apps.begin(buttons);
    apps.launchApp(&gotchiApp);

    Serial.println("[main] Boot complete");
}

void loop() {
    M5.update();

    uint32_t now = millis();
    static uint32_t lastMs = 0;
    uint32_t delta = now - lastMs;
    lastMs = now;

    buttons.update();

    if (M5.BtnC.wasPressed()) {
        if (menu.isOpen()) {
            menu.prevItem();
        } else {
            menu.open();
        }
    }

    if (menu.isOpen()) {
        menu.update();
    } else {
        apps.update(delta);
    }
}
