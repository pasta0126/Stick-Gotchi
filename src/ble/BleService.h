#pragma once
#include <NimBLEDevice.h>
#include <functional>

// BLE state payload (7 bytes) sent to the Android companion app.
struct BleGotchiState {
    uint8_t  mood;
    uint8_t  hunger;
    uint8_t  thirst;
    uint8_t  energy;
    uint16_t steps;   // little-endian
    uint8_t  flags;   // bit0 = phoneBatteryLow
};

// Companion app → device commands.
enum class BleCommand : uint8_t {
    FEED  = 0x01,
    DRINK = 0x02,
    PET   = 0x03,
    PLAY  = 0x04,
};

using BleCommandCallback = std::function<void(BleCommand)>;
using BleBatteryCallback = std::function<void(uint8_t level, bool charging)>;
using BleContextCallback = std::function<void(uint8_t hour, int8_t tempC)>;

// ── UUIDs ────────────────────────────────────────────────────────────────────
// Must match the Android companion app exactly.
#define BLE_SVC_UUID  "4fafc201-1fb5-459e-8fcc-c5c9c3319100"
#define BLE_CHR_STATE "beb5483e-36e1-4688-b7f5-ea07361b26a8"  // READ + NOTIFY
#define BLE_CHR_CMD   "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  // WRITE
#define BLE_CHR_BAT   "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  // WRITE
#define BLE_CHR_CTX   "6e400004-b5a3-f393-e0a9-e50e24dcca9e"  // WRITE
// ─────────────────────────────────────────────────────────────────────────────

class BleService {
public:
    // Call once in setup() before any app starts.
    static void initStack();

    // GotchiApp calls start() on init(), stop() on destroy()/suspend().
    void start(const char* deviceName);
    void stop();
    bool isRunning() const { return _running; }

    // Push updated pet state; fires BLE notification if a client is connected.
    void notifyState(const BleGotchiState& state);

    void setCommandCallback(BleCommandCallback cb) { _onCommand = cb; }
    void setBatteryCallback(BleBatteryCallback cb) { _onBattery = cb; }
    void setContextCallback(BleContextCallback cb) { _onContext = cb; }

    bool isConnected() const;

private:
    bool _running = false;

    NimBLEServer*         _server    = nullptr;
    NimBLECharacteristic* _stateChr  = nullptr;
    NimBLECharacteristic* _cmdChr    = nullptr;
    NimBLECharacteristic* _batChr    = nullptr;
    NimBLECharacteristic* _ctxChr    = nullptr;

    BleCommandCallback _onCommand;
    BleBatteryCallback _onBattery;
    BleContextCallback _onContext;

    // NimBLE callback bridges (inner classes)
    class ServerCallbacks;
    class CmdCallbacks;
    class BatCallbacks;
    class CtxCallbacks;
};
