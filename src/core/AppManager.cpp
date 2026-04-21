#include "AppManager.h"

void AppManager::begin(ButtonManager& buttons) {
    _buttons = &buttons;
}

void AppManager::launchApp(AppBase* app) {
    if (_current) {
        _current->destroy();
    }
    _current = app;
    if (_current) {
        _current->init();
        _registerInputCallback();
    }
}

void AppManager::suspendCurrent() {
    if (_current) _current->suspend();
}

void AppManager::resumeCurrent() {
    if (_current) {
        _current->resume();
        _registerInputCallback();
    }
}

void AppManager::update(uint32_t deltaMs) {
    if (_current) _current->update(deltaMs);
}

void AppManager::_registerInputCallback() {
    _buttons->setCallback([this](const InputEvent& e) {
        if (_current) _current->onInput(e);
    });
}
