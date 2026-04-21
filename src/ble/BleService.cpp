#include "BleService.h"
#include <Arduino.h>

// ── NimBLE callback implementations ─────────────────────────────────────────

class BleService::ServerCallbacks : public NimBLEServerCallbacks {
public:
    void onConnect(NimBLEServer* /*srv*/) override {
        Serial.println("[BLE] Client connected");
    }
    void onDisconnect(NimBLEServer* srv) override {
        Serial.println("[BLE] Client disconnected — restarting advertising");
        srv->startAdvertising();
    }
};

class BleService::CmdCallbacks : public NimBLECharacteristicCallbacks {
public:
    explicit CmdCallbacks(BleService* svc) : _svc(svc) {}
    void onWrite(NimBLECharacteristic* chr) override {
        if (!_svc->_onCommand) return;
        std::string val = chr->getValue();
        if (val.empty()) return;
        _svc->_onCommand(static_cast<BleCommand>(val[0]));
    }
private:
    BleService* _svc;
};

class BleService::BatCallbacks : public NimBLECharacteristicCallbacks {
public:
    explicit BatCallbacks(BleService* svc) : _svc(svc) {}
    void onWrite(NimBLECharacteristic* chr) override {
        if (!_svc->_onBattery) return;
        std::string val = chr->getValue();
        if (val.size() < 2) return;
        _svc->_onBattery((uint8_t)val[0], (bool)val[1]);
    }
private:
    BleService* _svc;
};

class BleService::CtxCallbacks : public NimBLECharacteristicCallbacks {
public:
    explicit CtxCallbacks(BleService* svc) : _svc(svc) {}
    void onWrite(NimBLECharacteristic* chr) override {
        if (!_svc->_onContext) return;
        std::string val = chr->getValue();
        if (val.size() < 2) return;
        _svc->_onContext((uint8_t)val[0], (int8_t)val[1]);
    }
private:
    BleService* _svc;
};

// ── BleService methods ───────────────────────────────────────────────────────

void BleService::initStack() {
    NimBLEDevice::init(""); // initialise stack once — subsequent calls are no-ops
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); // max TX power
}

void BleService::start(const char* deviceName) {
    if (_running) return;

    NimBLEDevice::setDeviceName(deviceName);
    _server = NimBLEDevice::createServer();
    _server->setCallbacks(new ServerCallbacks());

    NimBLEService* svc = _server->createService(BLE_SVC_UUID);

    // State characteristic — READ + NOTIFY
    _stateChr = svc->createCharacteristic(
        BLE_CHR_STATE,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    // Command characteristic — WRITE (no response for lower latency)
    _cmdChr = svc->createCharacteristic(
        BLE_CHR_CMD,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    _cmdChr->setCallbacks(new CmdCallbacks(this));

    // Phone battery characteristic — WRITE
    _batChr = svc->createCharacteristic(BLE_CHR_BAT, NIMBLE_PROPERTY::WRITE);
    _batChr->setCallbacks(new BatCallbacks(this));

    // Context (hour + temperature) characteristic — WRITE
    _ctxChr = svc->createCharacteristic(BLE_CHR_CTX, NIMBLE_PROPERTY::WRITE);
    _ctxChr->setCallbacks(new CtxCallbacks(this));

    svc->start();

    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(BLE_SVC_UUID);
    adv->setScanResponse(true);
    adv->start();

    _running = true;
    Serial.printf("[BLE] Advertising as \"%s\"\n", deviceName);
}

void BleService::stop() {
    if (!_running) return;
    NimBLEDevice::getAdvertising()->stop();
    if (_server) _server->disconnect(0);
    // NimBLE doesn't support full service teardown easily; just stop advertising.
    _running = false;
    Serial.println("[BLE] Stopped");
}

void BleService::notifyState(const BleGotchiState& state) {
    if (!_running || !_stateChr) return;
    uint8_t buf[7];
    buf[0] = state.mood;
    buf[1] = state.hunger;
    buf[2] = state.thirst;
    buf[3] = state.energy;
    buf[4] = (uint8_t)(state.steps & 0xFF);
    buf[5] = (uint8_t)(state.steps >> 8);
    buf[6] = state.flags;
    _stateChr->setValue(buf, sizeof(buf));
    _stateChr->notify();
}

bool BleService::isConnected() const {
    return _running && _server && (_server->getConnectedCount() > 0);
}
