#include <M5Unified.h>

#include "core/ButtonManager.h"
#include "core/DisplayManager.h"
#include "core/AppManager.h"
#include "menu/MenuOverlay.h"
#include "ble/BleService.h"
#include "apps/gotchi/GotchiApp.h"
#include "apps/imudemo/ImuDemoApp.h"

// ── Singletons ────────────────────────────────────────────────────────────────
static ButtonManager  buttons;
static DisplayManager display;
static AppManager     apps;
static MenuOverlay    menu;
static BleService     ble;

// ── App instances (static — no heap allocation) ───────────────────────────────
static GotchiApp  gotchiApp;
static ImuDemoApp imuDemoApp;

// ── Icon draw lambdas (solarpunk geometric style) ─────────────────────────────

// Stick Gotchi: stylised face
static void iconGotchi(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int r = sz / 2;
    c.drawCircle(cx, cy, r, col);                                   // head
    c.fillCircle(cx - r / 3, cy - r / 5, 2, col);                  // left eye
    c.fillCircle(cx + r / 3, cy - r / 5, 2, col);                  // right eye
    // smile: three-segment arc approximation
    c.drawLine(cx - r/3, cy + r/5,     cx - r/6, cy + r/3,     col);
    c.drawLine(cx - r/6, cy + r/3,     cx + r/6, cy + r/3,     col);
    c.drawLine(cx + r/6, cy + r/3,     cx + r/3, cy + r/5,     col);
}

// IMU Demo: bar chart with baseline
static void iconImu(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int bw   = max(4, sz / 5);
    int base = cy + sz / 2 - 2;
    int x0   = cx - sz / 2 + 2;
    int gap  = (sz - 3 * bw) / 2;
    int h0   = sz / 2,  h1 = sz * 3 / 4,  h2 = sz / 3;
    c.fillRect(x0,              base - h0, bw, h0, col);
    c.fillRect(x0 + bw + gap,   base - h1, bw, h1, col);
    c.fillRect(x0 + 2*(bw+gap), base - h2, bw, h2, col);
    c.drawFastHLine(cx - sz/2, base + 1, sz, col);
}

// Reboot: circular arrow
static void iconReboot(M5Canvas& c, int cx, int cy, int sz, uint32_t col) {
    int r = sz / 2 - 2;
    // Draw arc as dense dots from 40° to 320°
    for (int a = 40; a <= 320; a += 6) {
        float rad = a * 3.14159f / 180.0f;
        int px = cx + (int)(cosf(rad) * r);
        int py = cy + (int)(sinf(rad) * r);
        c.fillCircle(px, py, 1, col);
    }
    // Arrowhead at ~320° tangent
    float rad = 320.0f * 3.14159f / 180.0f;
    int ax = cx + (int)(cosf(rad) * r);
    int ay = cy + (int)(sinf(rad) * r);
    c.fillTriangle(ax, ay - 6, ax + 6, ay + 1, ax - 4, ay + 4, col);
}

// ─────────────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);

    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(1); // landscape, USB connector on the right
    M5.Display.setBrightness(180);

    BleService::initStack();

    display.begin();
    buttons.begin(700);

    gotchiApp.inject(&ble);
    imuDemoApp.inject(&display);

    // Allow any app to open the menu via BtnB long press
    gotchiApp.setMenuCallback([]()  { menu.open(); });
    imuDemoApp.setMenuCallback([]() { menu.open(); });

    menu.begin(buttons, display, apps);
    menu.addItem({ "Stick Gotchi", MenuItemType::APP,
                   []() -> AppBase* { return &gotchiApp; }, nullptr, iconGotchi });
    menu.addItem({ "IMU Demo",     MenuItemType::APP,
                   []() -> AppBase* { return &imuDemoApp; }, nullptr, iconImu });
    menu.addItem({ "Reboot",       MenuItemType::ACTION, nullptr,
                   []() { ESP.restart(); }, iconReboot });

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
            menu.prevItem();  // C = previous item while menu is open
        } else {
            menu.open();      // C = open menu while app is running (backup)
        }
    }

    if (menu.isOpen()) {
        menu.update();
    } else {
        apps.update(delta);
    }
}
