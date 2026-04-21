#include "DisplayManager.h"

void DisplayManager::begin() {
    _mutex = xSemaphoreCreateMutex();
    _canvas.createSprite(W, H);
    _canvas.setTextFont(2);
    _canvas.setTextSize(1);
}

bool DisplayManager::acquire(uint32_t timeoutMs) {
    TickType_t ticks = (timeoutMs == portMAX_DELAY)
                           ? portMAX_DELAY
                           : pdMS_TO_TICKS(timeoutMs);
    return xSemaphoreTake(_mutex, ticks) == pdTRUE;
}

void DisplayManager::release() {
    xSemaphoreGive(_mutex);
}

void DisplayManager::push() {
    _canvas.pushSprite(0, 0);
}
