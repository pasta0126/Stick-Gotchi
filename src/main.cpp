#include <M5Unified.h>

#include "core/ButtonManager.h"
#include "core/DisplayManager.h"
#include "core/AppManager.h"
#include "menu/MenuOverlay.h"
#include "menu/GotchiMenuIcons.h"
#include "ble/BleService.h"
#include "apps/gotchi/GotchiApp.h"
#include "apps/gotchi/MiniGames.h"
#include "apps/imudemo/ImuDemoApp.h"
#include "apps/stats/StatsApp.h"

// ── Singletons ────────────────────────────────────────────────────────────────
static ButtonManager  buttons;
static DisplayManager display;
static AppManager     apps;
static MenuOverlay    menu;
static BleService     ble;

// ── App instances ─────────────────────────────────────────────────────────────
static GotchiApp  gotchiApp;
static ImuDemoApp imuDemoApp; // mode is set by menu before launch
static StatsApp   statsApp;

// Icon functions are defined in menu/GotchiMenuIcons.h

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
    statsApp.inject(&display);
    statsApp.setMenuCallback([]() { menu.open(); });

    gotchiApp.setMenuCallback([]()  { menu.open(); });
    imuDemoApp.setMenuCallback([]() { menu.open(); });

    menu.begin(buttons, display, apps);

    // ── IMU submenu items ──────────────────────────────────────────────────
    std::vector<MenuItem> imuChildren = {
        { "Accelerometer", MenuItemType::APP,
          []() -> AppBase* { imuDemoApp.setMode(ImuMode::ACCEL); return &imuDemoApp; },
          nullptr, menuIconBars, {} },
        { "Gyroscope",     MenuItemType::APP,
          []() -> AppBase* { imuDemoApp.setMode(ImuMode::GYRO);  return &imuDemoApp; },
          nullptr, menuIconWave, {} },
        { "Orientation",   MenuItemType::APP,
          []() -> AppBase* { imuDemoApp.setMode(ImuMode::ORIENT); return &imuDemoApp; },
          nullptr, menuIconCompass, {} },
    };

    std::vector<MenuItem> gameChildren = {
        { "Lanzar Moneda", MenuItemType::ACTION,
          nullptr,
          []() { gotchiApp.startMiniGame(MiniGameId::FLIP_COIN); },
          menuIconCoin, {} },
        { "Bola 8", MenuItemType::ACTION,
          nullptr,
          []() { gotchiApp.startMiniGame(MiniGameId::MAGIC_8BALL); },
          menuIcon8Ball, {} },
    };

    menu.addItem({ "Stick Gotchi", MenuItemType::APP,
                   []() -> AppBase* { return &gotchiApp; },
                   nullptr, menuIconGotchi, {} });
    menu.addItem({ "Stats",        MenuItemType::APP,
                   []() -> AppBase* { return &statsApp; },
                   nullptr, menuIconStats, {} });
    menu.addItem({ "Jugar",        MenuItemType::SUBMENU,
                   nullptr, nullptr, menuIconCoin, gameChildren });
    menu.addItem({ "IMU Sensors",  MenuItemType::SUBMENU,
                   nullptr, nullptr, menuIconBars, imuChildren });
    menu.addItem({ "Reboot",       MenuItemType::ACTION,
                   nullptr, []() { ESP.restart(); }, menuIconReboot, {} });

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
