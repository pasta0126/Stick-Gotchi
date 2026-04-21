#include "ImuDemoApp.h"
#include <M5Unified.h>

void ImuDemoApp::init() {
    Serial.println("[ImuDemo] init");
}

void ImuDemoApp::update(uint32_t deltaMs) {
    _drawAccum += deltaMs;
    if (_drawAccum < 66) return; // ~15 fps is enough for a data display
    _drawAccum = 0;

    float ax, ay, az, gx, gy, gz;
    M5.Imu.getAccel(&ax, &ay, &az);
    M5.Imu.getGyro(&gx, &gy, &gz);
    _drawFrame(ax, ay, az, gx, gy, gz);
}

void ImuDemoApp::_drawFrame(float ax, float ay, float az,
                             float gx, float gy, float gz) {
    if (!_display->acquire(50)) return;

    auto& c = _display->canvas();
    c.fillScreen(0x0D0D1A);

    // Title
    c.setTextFont(2);
    c.setTextColor(TFT_WHITE, 0x0D0D1A);
    c.setCursor(4, 2);
    c.print("IMU Demo  (B=menu)");

    // Accel section
    c.setTextColor(0xAAAAAA, 0x0D0D1A);
    c.setTextFont(1);
    c.setCursor(4, 22);
    c.print("ACCEL (g)");
    _drawBar(4,  32, 100, 12, ax, -2.f, 2.f, TFT_RED,   "X");
    _drawBar(4,  50, 100, 12, ay, -2.f, 2.f, TFT_GREEN, "Y");
    _drawBar(4,  68, 100, 12, az, -2.f, 2.f, TFT_BLUE,  "Z");

    // Gyro section
    c.setTextColor(0xAAAAAA, 0x0D0D1A);
    c.setCursor(4, 86);
    c.print("GYRO (deg/s)");
    _drawBar(4,  96, 100, 12, gx, -250.f, 250.f, TFT_ORANGE,  "X");
    _drawBar(4, 114, 100, 12, gy, -250.f, 250.f, TFT_CYAN,    "Y");
    _drawBar(4, 122, 100, 12, gz, -250.f, 250.f, TFT_MAGENTA, "Z");

    // Numeric readout (right column)
    c.setTextFont(1);
    c.setTextColor(TFT_WHITE, 0x0D0D1A);
    int rx = 120;
    c.setCursor(rx, 22); c.printf("A: %+.2f %+.2f %+.2f", ax, ay, az);
    c.setCursor(rx, 34); c.printf("G: %+5.0f %+5.0f %+5.0f", gx, gy, gz);

    // Magnitude
    float mag = sqrtf(ax*ax + ay*ay + az*az);
    c.setCursor(rx, 50); c.printf("|A|= %.3f g", mag);

    _display->push();
    _display->release();
}

void ImuDemoApp::_drawBar(int x, int y, int w, int h,
                           float val, float minV, float maxV,
                           uint32_t color, const char* label) {
    auto& c = _display->canvas();

    // Background track
    c.fillRect(x + 16, y, w, h, 0x222233);

    // Map val to pixel
    float ratio = (val - minV) / (maxV - minV);
    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;
    int filled = (int)(ratio * w);

    // Zero-line at 50%
    int zeroX = x + 16 + w / 2;
    int barX  = (filled >= w / 2) ? zeroX : x + 16 + filled;
    int barW  = abs(filled - w / 2);
    c.fillRect(barX, y + 1, barW, h - 2, color);

    // Zero marker
    c.drawFastVLine(zeroX, y, h, TFT_WHITE);

    // Label
    c.setTextColor(color, 0x0D0D1A);
    c.setTextFont(1);
    c.setCursor(x, y + 2);
    c.print(label);

    // Value text
    c.setTextColor(TFT_WHITE, 0x0D0D1A);
    c.setCursor(x + 16 + w + 4, y + 2);
    c.printf("%+.2f", val);
}
