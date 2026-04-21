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

// ─────────────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);

    // M5Unified hardware init
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(1); // landscape, USB connector on the right
    M5.Display.setBrightness(180);

    // BLE stack (once only)
    BleService::initStack();

    // Core systems
    display.begin();
    buttons.begin(700); // 700 ms long-press threshold

    // Inject dependencies into apps
    gotchiApp.inject(&ble);
    imuDemoApp.inject(&display);

    // Allow any app to request the menu to open via BtnB long press
    gotchiApp.setMenuCallback([]()  { menu.open(); });
    imuDemoApp.setMenuCallback([]() { menu.open(); });

    // Build menu
    menu.begin(buttons, display, apps);
    menu.addItem({ "Stick Gotchi", MenuItemType::APP,
                   []() -> AppBase* { return &gotchiApp; }, nullptr });
    menu.addItem({ "IMU Demo",     MenuItemType::APP,
                   []() -> AppBase* { return &imuDemoApp; }, nullptr });
    menu.addItem({ "Reboot",       MenuItemType::ACTION, nullptr,
                   []() { ESP.restart(); } });

    // AppManager wires button callbacks for the active app
    apps.begin(buttons);

    // Boot directly into Stick Gotchi
    apps.launchApp(&gotchiApp);

    Serial.println("[main] Boot complete");
}

void loop() {
    M5.update(); // polls hardware (buttons, IMU, power)

    uint32_t now = millis();
    static uint32_t lastMs = 0;
    uint32_t delta = now - lastMs;
    lastMs = now;

    // Button updates (A and B go through the callback chain)
    buttons.update();

    // Button C (power button) — always toggles the menu regardless of app state
    if (M5.BtnC.wasPressed()) {
        menu.isOpen() ? menu.close() : menu.open();
    }

    // Delegate main loop to whoever is active
    if (menu.isOpen()) {
        menu.update();
    } else {
        apps.update(delta);
    }
}
