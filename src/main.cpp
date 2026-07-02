#include <M5Unified.h>

#include "core/ButtonManager.h"
#include "core/DisplayManager.h"
#include "core/AppManager.h"
#include "home/CarouselHome.h"
#include "home/TileIcons.h"
#include "apps/coinflip/CoinFlipApp.h"
#include "apps/magic8ball/Magic8BallApp.h"
#include "apps/imudemo/ImuDemoApp.h"

// ── Singletons ────────────────────────────────────────────────────────────────
static ButtonManager  buttons;
static DisplayManager display;
static AppManager     apps;

// ── App instances ─────────────────────────────────────────────────────────────
static CarouselHome   carouselHome;
static CoinFlipApp    coinFlipApp;
static Magic8BallApp  magic8BallApp;
static ImuDemoApp     imuDemoApp;

// ── Button C (power) — hard-wired outside ButtonManager, never touches UI ─────
static uint32_t btnCPressStart = 0;
static bool     btnCLongFired  = false;
static constexpr uint32_t BTN_C_LONG_MS = 700;

static void handlePowerButton() {
    uint32_t now = millis();
    bool down = M5.BtnPWR.isPressed();

    if (down) {
        if (btnCPressStart == 0) btnCPressStart = now;
        else if (!btnCLongFired && (now - btnCPressStart) >= BTN_C_LONG_MS) {
            btnCLongFired = true;
            M5.Power.powerOff();
        }
    } else {
        if (btnCPressStart != 0 && !btnCLongFired) {
            ESP.restart();
        }
        btnCPressStart = 0;
        btnCLongFired  = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);

    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(1);
    M5.Display.setBrightness(180);

    display.begin();
    buttons.begin(700);

    coinFlipApp.inject(&display);
    magic8BallApp.inject(&display);
    imuDemoApp.inject(&display);
    carouselHome.inject(&display);
    carouselHome.setAppManager(&apps);

    coinFlipApp.setHomeCallback([]()   { apps.launchApp(&carouselHome); });
    magic8BallApp.setHomeCallback([]() { apps.launchApp(&carouselHome); });
    imuDemoApp.setHomeCallback([]()    { apps.launchApp(&carouselHome); });

    carouselHome.addTile({ "Coin Flip", tileIconCoin,
        []() -> AppBase* { return &coinFlipApp; } });
    carouselHome.addTile({ "Magic 8-Ball", tileIcon8Ball,
        []() -> AppBase* { return &magic8BallApp; } });
    carouselHome.addTile({ "Accelerometer", tileIconAccel,
        []() -> AppBase* { imuDemoApp.setMode(ImuMode::ACCEL); return &imuDemoApp; } });
    carouselHome.addTile({ "Gyroscope", tileIconGyro,
        []() -> AppBase* { imuDemoApp.setMode(ImuMode::GYRO); return &imuDemoApp; } });
    carouselHome.addTile({ "Orientation", tileIconOrient,
        []() -> AppBase* { imuDemoApp.setMode(ImuMode::ORIENT); return &imuDemoApp; } });

    apps.begin(buttons);
    apps.launchApp(&carouselHome);

    Serial.println("[main] Boot complete");
}

void loop() {
    M5.update();

    uint32_t now = millis();
    static uint32_t lastMs = 0;
    uint32_t delta = now - lastMs;
    lastMs = now;

    buttons.update();
    handlePowerButton();
    apps.update(delta);
}
