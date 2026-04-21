#include "ImuDemoApp.h"
#include <M5Unified.h>

// Solarpunk palette (shared across modes)
static constexpr uint32_t BG      = 0x040F02;
static constexpr uint32_t LIME    = 0x40FF20;
static constexpr uint32_t AMBER   = 0xFFCC00;
static constexpr uint32_t DIM     = 0x1A3010;
static constexpr uint32_t MUTED   = 0x2A5020;
static constexpr uint32_t RED_C   = 0xFF3020;
static constexpr uint32_t BLUE_C  = 0x20A0FF;

void ImuDemoApp::init() {
    Serial.printf("[ImuDemo] init mode=%d\n", (int)_mode);
}

void ImuDemoApp::update(uint32_t deltaMs) {
    _drawAccum += deltaMs;
    if (_drawAccum < 50) return; // 20 fps
    _drawAccum = 0;

    float ax, ay, az, gx, gy, gz;
    M5.Imu.getAccel(&ax, &ay, &az);
    M5.Imu.getGyro(&gx, &gy, &gz);

    switch (_mode) {
    case ImuMode::ACCEL:  _drawAccel(ax, ay, az);  break;
    case ImuMode::GYRO:   _drawGyro(gx, gy, gz);   break;
    case ImuMode::ORIENT: _drawOrient(ax, ay, az);  break;
    }
}

// ── Accelerometer — ball in a box ────────────────────────────────────────────
void ImuDemoApp::_drawAccel(float ax, float ay, float az) {
    if (!_display->acquire(50)) return;
    auto& c = _display->canvas();
    const int W = DisplayManager::W, H = DisplayManager::H;

    c.fillScreen(BG);

    // Title
    c.setTextFont(2);
    c.setTextColor(LIME, BG);
    c.setCursor(6, 4);
    c.print("ACCEL");

    // Box parameters — square, left-aligned
    const int BOX_SZ = 95;
    const int BX = 16, BY = (H - BOX_SZ) / 2;
    // Outer ring with tick marks
    c.drawRect(BX, BY, BOX_SZ, BOX_SZ, MUTED);
    c.drawRect(BX+1, BY+1, BOX_SZ-2, BOX_SZ-2, DIM);
    // Crosshair faint
    c.drawFastHLine(BX, BY + BOX_SZ/2, BOX_SZ, DIM);
    c.drawFastVLine(BX + BOX_SZ/2, BY, BOX_SZ, DIM);

    // Ball position: ax→horizontal (left/right), ay→vertical (up/down)
    // Clamp to ±1.5g, map to box interior with 8px margin
    float nx = ax / 1.5f, ny = -ay / 1.5f; // invert Y so +ay = up
    if (nx < -1) nx = -1; if (nx > 1) nx = 1;
    if (ny < -1) ny = -1; if (ny > 1) ny = 1;
    int ballX = BX + BOX_SZ/2 + (int)(nx * (BOX_SZ/2 - 10));
    int ballY = BY + BOX_SZ/2 + (int)(ny * (BOX_SZ/2 - 10));

    // Ball colour by total magnitude
    float mag = sqrtf(ax*ax + ay*ay + az*az);
    uint32_t ballColor = (mag < 1.2f) ? LIME : (mag < 1.8f ? AMBER : RED_C);

    // Shadow / glow ring
    c.drawCircle(ballX, ballY, 9, DIM);
    c.fillCircle(ballX, ballY, 7, ballColor);

    // Magnitude gauge — thin arc around box
    int barW = (int)((min(mag, 3.0f) / 3.0f) * (BOX_SZ - 4));
    c.fillRect(BX+2, BY + BOX_SZ + 4, barW, 3, ballColor);
    c.drawRect(BX+2, BY + BOX_SZ + 4, BOX_SZ - 4, 3, MUTED);

    // Right panel — numeric readout
    int rx = BX + BOX_SZ + 18;
    c.setTextFont(1);

    auto drawVal = [&](int y, const char* lbl, float v, uint32_t col) {
        c.setTextColor(col, BG);
        c.setCursor(rx, y);
        c.printf("%s", lbl);
        c.setTextColor(0xDCF5D0, BG);
        c.setCursor(rx + 16, y);
        c.printf("%+.2f", v);
    };

    drawVal(22, "X",  ax, RED_C);
    drawVal(36, "Y",  ay, LIME);
    drawVal(50, "Z",  az, BLUE_C);

    // Magnitude readout (large)
    c.setTextFont(4);
    c.setTextColor(ballColor, BG);
    c.setCursor(rx, 68);
    c.printf("%.2fg", mag);

    _display->push();
    _display->release();
}

// ── Gyroscope — radial bar per axis ──────────────────────────────────────────
void ImuDemoApp::_drawGyro(float gx, float gy, float gz) {
    if (!_display->acquire(50)) return;
    auto& c = _display->canvas();
    const int W = DisplayManager::W, H = DisplayManager::H;

    c.fillScreen(BG);

    c.setTextFont(2);
    c.setTextColor(LIME, BG);
    c.setCursor(6, 4);
    c.print("GYRO");

    // Centre of the radar
    int cx = W / 2, cy = H / 2 + 6;
    int maxR = 52;

    // Scale rings
    for (int r = maxR/3; r <= maxR; r += maxR/3) {
        c.drawCircle(cx, cy, r, DIM);
    }
    // Axis lines
    c.drawFastHLine(cx - maxR, cy, maxR*2, DIM);
    c.drawFastVLine(cx, cy - maxR, maxR*2, DIM);

    // Map gyro values to bar length (cap at ±250 dps)
    auto plotBar = [&](float val, float angleDeg, uint32_t col, const char* lbl) {
        float norm = val / 250.0f;
        if (norm > 1) norm = 1; if (norm < -1) norm = -1;
        float rad = angleDeg * 3.14159f / 180.0f;
        float nx = cosf(rad), ny = sinf(rad);
        int len = (int)(fabsf(norm) * maxR);
        // Direction: positive along angle, negative opposite
        float dir = (norm >= 0) ? 1.0f : -1.0f;
        int ex = cx + (int)(nx * len * dir);
        int ey = cy + (int)(ny * len * dir);
        // Thick line (3 pass)
        c.drawLine(cx, cy, ex, ey, col);
        c.drawLine(cx+1, cy, ex+1, ey, col);
        c.drawLine(cx, cy+1, ex, ey+1, col);
        // Endpoint dot
        c.fillCircle(ex, ey, 4, col);
        // Label
        int lx = cx + (int)(nx * (maxR + 8));
        int ly = cy + (int)(ny * (maxR + 8));
        c.setTextFont(1);
        c.setTextColor(col, BG);
        c.setCursor(lx - 3, ly - 4);
        c.print(lbl);
        // Value
        c.setCursor(lx - 3, ly + 4);
        c.printf("%.0f", val);
    };

    plotBar(gx,   0.0f, RED_C,  "X");  // right
    plotBar(gy, 120.0f, LIME,   "Y");  // lower-left
    plotBar(gz, 240.0f, BLUE_C, "Z");  // upper-left

    _display->push();
    _display->release();
}

// ── Orientation — artificial horizon ─────────────────────────────────────────
void ImuDemoApp::_drawOrient(float ax, float ay, float az) {
    if (!_display->acquire(50)) return;
    auto& c = _display->canvas();
    const int W = DisplayManager::W, H = DisplayManager::H;

    // Roll from ax, pitch offset from ay
    // roll: positive ax = right side down
    float roll  =  atan2f(ax, az);           // radians
    float pitch = -ay * (H / 2) / 1.0f;     // px offset for pitch

    int cx = W / 2, cy = H / 2 + (int)pitch;

    // Horizon line: y = cy + (x - cx) * tan(roll)
    // Fill sky (above horizon) and earth (below) using scanlines
    float tanRoll = tanf(roll);

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            float horizY = cy + (x - cx) * tanRoll;
            if (y < (int)horizY) {
                // sky — very dark teal/green (solarpunk sky)
                c.drawPixel(x, y, 0x031A10);
            } else {
                // earth — very dark warm green
                c.drawPixel(x, y, 0x0A0A01);
            }
        }
    }

    // Horizon line — bright lime
    for (int x = 0; x < W; x++) {
        int hy = cy + (int)((x - cx) * tanRoll);
        if (hy >= 0 && hy < H) {
            c.drawFastVLine(x, max(0, hy-1), 3, LIME);
        }
    }

    // Aircraft reference mark (fixed center cross)
    c.fillRect(cx - 30, cy - 1, 24, 3, AMBER);
    c.fillRect(cx +  6, cy - 1, 24, 3, AMBER);
    c.fillCircle(cx, cy, 4, AMBER);

    // Roll angle readout
    c.setTextFont(2);
    c.setTextColor(LIME, 0x031A10);
    c.setCursor(6, 4);
    c.printf("ORIENT  %+.1f°", roll * 180.0f / 3.14159f);

    _display->push();
    _display->release();
}
