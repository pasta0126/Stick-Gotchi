#include "GotchiRenderer.h"

void GotchiRenderer::start(GotchiPet* pet, LifeStage stage) {
    _pet     = pet;
    _stage   = stage;
    _started = true;
    _suspended = false;
    _lastFrameMs = 0;
    _animFrame = 0;
    _animAccumMs = 0;

    xTaskCreatePinnedToCore(
        _renderTask,
        "GotchiRender",
        4096,
        this,
        2,
        &_taskHandle,
        0
    );
}

void GotchiRenderer::suspend() {
    _suspended = true;
    if (_taskHandle) vTaskSuspend(_taskHandle);
}

void GotchiRenderer::resume() {
    _suspended = false;
    if (_taskHandle) vTaskResume(_taskHandle);
}

void GotchiRenderer::stop() {
    _started = false;
    if (_taskHandle) {
        vTaskDelete(_taskHandle);
        _taskHandle = nullptr;
    }
}

void GotchiRenderer::render() {
    if (!_pet || !_display || !_started || _suspended) return;
    _drawFrame();
}

void GotchiRenderer::setGaze(float h, float v) {
    _gazeH = h;
    _gazeV = v;
}

void GotchiRenderer::setActionBarState(uint8_t selected, bool visible) {
    _selectedAction = selected;
    _actionBarVisible = visible;
}

void GotchiRenderer::setLifeStage(LifeStage stage) {
    _stage = stage;
}

void GotchiRenderer::_renderTask(void* arg) {
    auto* self = static_cast<GotchiRenderer*>(arg);
    for (;;) {
        if (self->_started && !self->_suspended && self->_pet && self->_display) {
            self->_drawFrame();
        }
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

void GotchiRenderer::_drawFrame() {
    if (!_display || !_display->acquire(100)) return;

    _canvas = &_display->canvas();
    _canvas->fillScreen(TFT_BLACK);

    if (_pet->isDead()) {
        GotchiVisual vis = decodeVisual(_pet->currentID().visual_seed);
        SpritePalette pal = _buildPalette(vis);
        int cx = 120;
        int cy = 62;
        _drawSprite(DEATH_FRAME_SPRITE.data, DEATH_FRAME_SPRITE.w, DEATH_FRAME_SPRITE.h,
                    cx - (DEATH_FRAME_SPRITE.w * 2) / 2, cy - (DEATH_FRAME_SPRITE.h * 2) / 2,
                    2, pal);
    } else {
        _drawStatsBar();

        GotchiVisual vis = decodeVisual(_pet->currentID().visual_seed);
        SpritePalette pal = _buildPalette(vis);

        uint8_t frame = _animFrame;
        _drawBody(vis, frame);

        int cx = 120;
        int cy = 62;
        int scale = 2;
        if (_stage == LifeStage::EGG) scale = 3;
        if (_stage == LifeStage::BABY) scale = 3;

        _drawFace(_pet->mood(), _gazeH, _gazeV, cx, cy, scale);

        if (_pet->isSleeping()) {
            _drawSleepZs(cx, cy);
        }

        _drawActionBar();
    }

    _canvas->pushSprite(0, 0);
    _display->release();

    uint32_t now = millis();
    if (now - _lastFrameMs >= FRAME_INTERVAL_MS) {
        _lastFrameMs = now;
        _animFrame = (_animFrame + 1) % 2;
    }
}

void GotchiRenderer::_drawBody(const GotchiVisual& vis, uint8_t frame) {
    SpritePalette pal = _buildPalette(vis);

    SpriteFrame spriteData;
    int scale = 2;
    int baseX = 120, baseY = 62;

    switch (_stage) {
    case LifeStage::EGG:
        spriteData = frame == 0 ? EGG_FRAMES[0] : EGG_FRAMES[1];
        scale = 3;
        baseX = 120 - (spriteData.w * scale) / 2;
        baseY = 62 - (spriteData.h * scale) / 2;
        _drawSprite(spriteData.data, spriteData.w, spriteData.h, baseX, baseY, scale, pal);
        break;

    case LifeStage::BABY:
        spriteData = frame == 0 ? BABY_FRAMES[0] : BABY_FRAMES[1];
        scale = 3;
        baseX = 120 - (spriteData.w * scale) / 2;
        baseY = 62 - (spriteData.h * scale) / 2;
        _drawSprite(spriteData.data, spriteData.w, spriteData.h, baseX, baseY, scale, pal);
        break;

    case LifeStage::YOUNG:
        spriteData = frame == 0 ? YOUNG_FRAMES[0] : YOUNG_FRAMES[1];
        scale = 2;
        baseX = 120 - (spriteData.w * scale) / 2;
        baseY = 62 - (spriteData.h * scale) / 2;
        _drawSprite(spriteData.data, spriteData.w, spriteData.h, baseX, baseY, scale, pal);
        break;

    case LifeStage::ADULT:
        spriteData = frame == 0 ? ADULT_FRAMES[0] : ADULT_FRAMES[1];
        scale = 2;
        baseX = 120 - (spriteData.w * scale) / 2;
        baseY = 62 - (spriteData.h * scale) / 2;
        _drawSprite(spriteData.data, spriteData.w, spriteData.h, baseX, baseY, scale, pal);
        break;
    }
}

void GotchiRenderer::_drawFace(Mood mood, float gazeH, float gazeV, int cx, int cy, int scale) {
    int eyeOffsetX = scale * 3;
    int eyeOffsetY = scale * 2;

    int eyeL_base_x = cx - eyeOffsetX;
    int eyeR_base_x = cx + eyeOffsetX;
    int eyeY_base = cy - eyeOffsetY;

    int eyeL_x = eyeL_base_x + (int)(gazeH * scale * 1.5f);
    int eyeR_x = eyeR_base_x + (int)(gazeH * scale * 1.5f);
    int eyeY = eyeY_base + (int)(gazeV * scale);

    int eyeSize = scale;

    GotchiVisual vis = decodeVisual(_pet->currentID().visual_seed);
    uint8_t eyeType = vis.eye_type;

    if (mood == Mood::SLEEPING) {
        _canvas->drawLine(eyeL_x - scale, eyeY, eyeL_x + scale, eyeY, TFT_WHITE);
        _canvas->drawLine(eyeR_x - scale, eyeY, eyeR_x + scale, eyeY, TFT_WHITE);
    } else {
        switch (eyeType) {
        case 0:
            _canvas->fillCircle(eyeL_x, eyeY, eyeSize, TFT_WHITE);
            _canvas->fillCircle(eyeR_x, eyeY, eyeSize, TFT_WHITE);
            break;

        case 1: {
            int dy = scale;
            _canvas->fillTriangle(eyeL_x, eyeY - dy, eyeL_x - scale, eyeY + dy / 2, eyeL_x + scale, eyeY + dy / 2, TFT_WHITE);
            _canvas->fillTriangle(eyeR_x, eyeY - dy, eyeR_x - scale, eyeY + dy / 2, eyeR_x + scale, eyeY + dy / 2, TFT_WHITE);
            break;
        }

        case 2: {
            _canvas->fillCircle(eyeL_x - scale / 2, eyeY - scale / 2, scale, TFT_WHITE);
            _canvas->fillCircle(eyeL_x + scale / 2, eyeY - scale / 2, scale, TFT_WHITE);
            _canvas->fillTriangle(eyeL_x - scale, eyeY + scale / 2, eyeL_x + scale, eyeY + scale / 2, eyeL_x, eyeY + scale * 2, TFT_WHITE);

            _canvas->fillCircle(eyeR_x - scale / 2, eyeY - scale / 2, scale, TFT_WHITE);
            _canvas->fillCircle(eyeR_x + scale / 2, eyeY - scale / 2, scale, TFT_WHITE);
            _canvas->fillTriangle(eyeR_x - scale, eyeY + scale / 2, eyeR_x + scale, eyeY + scale / 2, eyeR_x, eyeY + scale * 2, TFT_WHITE);
            break;
        }

        case 3: {
            int armLen = scale + 1;
            _canvas->drawLine(eyeL_x - armLen, eyeY - armLen, eyeL_x + armLen, eyeY + armLen, TFT_WHITE);
            _canvas->drawLine(eyeL_x - armLen, eyeY + armLen, eyeL_x + armLen, eyeY - armLen, TFT_WHITE);
            _canvas->drawLine(eyeL_x, eyeY - armLen - 1, eyeL_x, eyeY + armLen + 1, TFT_WHITE);
            _canvas->drawLine(eyeL_x - armLen - 1, eyeY, eyeL_x + armLen + 1, eyeY, TFT_WHITE);

            _canvas->drawLine(eyeR_x - armLen, eyeY - armLen, eyeR_x + armLen, eyeY + armLen, TFT_WHITE);
            _canvas->drawLine(eyeR_x - armLen, eyeY + armLen, eyeR_x + armLen, eyeY - armLen, TFT_WHITE);
            _canvas->drawLine(eyeR_x, eyeY - armLen - 1, eyeR_x, eyeY + armLen + 1, TFT_WHITE);
            _canvas->drawLine(eyeR_x - armLen - 1, eyeY, eyeR_x + armLen + 1, eyeY, TFT_WHITE);
            break;
        }
        }
    }

    int mouthY = cy + eyeOffsetY + scale;
    int mouthW = scale * 2;

    switch (mood) {
    case Mood::HAPPY:
    case Mood::LAUGHING:
        _canvas->drawArc(cx, mouthY, mouthW, mouthW, 0, 180, TFT_WHITE);
        break;

    case Mood::SAD:
    case Mood::SICK:
        _canvas->drawArc(cx, mouthY - mouthW / 2, mouthW, mouthW, 180, 360, TFT_WHITE);
        break;

    case Mood::SLEEPING:
        break;

    case Mood::ANGRY:
        _canvas->drawLine(cx - mouthW, mouthY - scale, cx, mouthY, TFT_WHITE);
        _canvas->drawLine(cx, mouthY, cx + mouthW, mouthY - scale, TFT_WHITE);
        break;

    case Mood::SCARED:
    case Mood::STARTLED:
        _canvas->fillCircle(cx, mouthY, scale, TFT_WHITE);
        break;

    case Mood::EXCITED:
        _canvas->fillRect(cx - mouthW, mouthY - scale, mouthW * 2, scale * 2, TFT_WHITE);
        break;

    case Mood::NEUTRAL:
    case Mood::PENSIVE:
    case Mood::DIZZY:
    case Mood::ANNOYED:
    default:
        _canvas->drawLine(cx - mouthW, mouthY, cx + mouthW, mouthY, TFT_WHITE);
        break;
    }
}

void GotchiRenderer::_drawStatsBar() {
    _canvas->setTextSize(1);
    _canvas->setTextColor(TFT_WHITE, TFT_BLACK);

    PetStats stats = _pet->stats();
    uint16_t healthColor = stats.health < 20 ? TFT_RED : TFT_WHITE;
    uint16_t hungerColor = stats.hunger < 20 ? TFT_RED : TFT_WHITE;
    uint16_t energyColor = stats.energy < 20 ? TFT_RED : TFT_WHITE;

    _canvas->setTextColor(healthColor, TFT_BLACK);
    _canvas->drawString("H", 4, 5);
    _canvas->setTextColor(TFT_WHITE, TFT_BLACK);
    _canvas->drawString(String(stats.health), 15, 5);

    _canvas->setTextColor(hungerColor, TFT_BLACK);
    _canvas->drawString("F", 45, 5);
    _canvas->setTextColor(TFT_WHITE, TFT_BLACK);
    _canvas->drawString(String(stats.hunger), 55, 5);

    _canvas->setTextColor(energyColor, TFT_BLACK);
    _canvas->drawString("E", 85, 5);
    _canvas->setTextColor(TFT_WHITE, TFT_BLACK);
    _canvas->drawString(String(stats.energy), 95, 5);

    _canvas->setTextColor(TFT_WHITE, TFT_BLACK);
    auto dt = M5.Rtc.getDateTime();
    String timeStr = String(dt.time.hours) + ":" + (dt.time.minutes < 10 ? "0" : "") + String(dt.time.minutes);
    _canvas->drawString(timeStr, 160, 5);

    _canvas->drawString("G" + String(_pet->currentID().generation), 210, 5);
}

void GotchiRenderer::_drawActionBar() {
    if (!_actionBarVisible) return;

    int y_start = 108;
    int y_end = 134;
    int bar_height = y_end - y_start;
    int icon_size = 16;
    int icon_scale = 1;
    int spacing = 240 / 5;

    for (int i = 0; i < 5; i++) {
        int icon_x = (i * spacing) + (spacing - icon_size * icon_scale) / 2;
        int icon_y = y_start + (bar_height - icon_size * icon_scale) / 2;

        uint16_t borderColor = (static_cast<int>(_selectedAction) == i) ? TFT_WHITE : TFT_DARKGREY;

        _canvas->drawRect(icon_x - 1, icon_y - 1, icon_size * icon_scale + 2, icon_size * icon_scale + 2, borderColor);

        uint16_t iconColor = (static_cast<int>(_selectedAction) == i) ? TFT_WHITE : 0x8410;

        switch (i) {
        case 0:
            _canvas->fillRect(icon_x + 4, icon_y + 5, 8, 6, iconColor);
            _canvas->fillRect(icon_x + 2, icon_y + 6, 3, 4, iconColor);
            _canvas->fillRect(icon_x + 11, icon_y + 6, 3, 4, iconColor);
            break;

        case 1:
            _canvas->fillRect(icon_x + 3, icon_y + 3, 10, 10, iconColor);
            _canvas->fillCircle(icon_x + 6, icon_y + 6, 1, TFT_BLACK);
            _canvas->fillCircle(icon_x + 10, icon_y + 6, 1, TFT_BLACK);
            _canvas->fillCircle(icon_x + 8, icon_y + 10, 1, TFT_BLACK);
            break;

        case 2:
            _canvas->fillRect(icon_x + 5, icon_y + 3, 6, 10, iconColor);
            _canvas->fillCircle(icon_x + 5, icon_y + 4, 2, iconColor);
            _canvas->fillCircle(icon_x + 11, icon_y + 4, 2, iconColor);
            break;

        case 3:
            _canvas->fillCircle(icon_x + 8, icon_y + 6, 3, iconColor);
            _canvas->drawLine(icon_x + 8, icon_y + 2, icon_x + 8, icon_y + 1, iconColor);
            _canvas->drawLine(icon_x + 6, icon_y + 4, icon_x + 4, icon_y + 2, iconColor);
            _canvas->drawLine(icon_x + 10, icon_y + 4, icon_x + 12, icon_y + 2, iconColor);
            break;

        case 4:
            _canvas->drawRect(icon_x + 3, icon_y + 5, 10, 8, iconColor);
            _canvas->fillCircle(icon_x + 5, icon_y + 4, 2, iconColor);
            _canvas->fillCircle(icon_x + 11, icon_y + 4, 2, iconColor);
            break;

        default:
            break;
        }
    }
}

void GotchiRenderer::_drawSleepZs(int cx, int cy) {
    int z_x = cx + 20;
    int z_y = cy - 30;
    int z_size = 4;

    for (int i = 0; i < 3; i++) {
        _canvas->drawLine(z_x, z_y - (i * 8), z_x + z_size, z_y + z_size - (i * 8), TFT_WHITE);
        _canvas->drawLine(z_x + z_size, z_y - (i * 8), z_x, z_y + z_size - (i * 8), TFT_WHITE);
    }
}

GotchiRenderer::SpritePalette GotchiRenderer::_buildPalette(const GotchiVisual& vis) {
    SpritePalette pal;

    pal.transparent = 0x0000;
    pal.white = 0xFFFF;

    uint8_t sat_primary = 200;
    uint8_t val_primary = 220;
    uint8_t sat_secondary = 160;
    uint8_t val_secondary = 240;

    if (_stage == LifeStage::EGG) {
        sat_primary = 120;
        val_primary = 180;
        sat_secondary = 100;
        val_secondary = 200;
    } else if (_stage == LifeStage::ADULT) {
        sat_primary = 220;
        val_primary = 240;
        sat_secondary = 180;
        val_secondary = 255;
    }

    pal.primary = hsvToRgb565(vis.hue_primary * 11, sat_primary, val_primary);
    pal.secondary = hsvToRgb565(vis.hue_secondary * 11, sat_secondary, val_secondary);
    pal.dark = hsvToRgb565(vis.hue_primary * 11, 200, 100);

    return pal;
}

void GotchiRenderer::_drawSprite(const uint8_t* data, uint8_t w, uint8_t h,
                                  int x, int y, uint8_t scale, const SpritePalette& pal) {
    for (int py = 0; py < h; py++) {
        for (int px = 0; px < w; px++) {
            uint8_t idx = data[py * w + px];
            if (idx == 0) continue;

            uint16_t color;
            switch (idx) {
            case 1: color = pal.primary; break;
            case 2: color = pal.secondary; break;
            case 3: color = pal.dark; break;
            case 4: color = pal.white; break;
            default: color = pal.transparent; break;
            }

            if (color != pal.transparent) {
                int sx = x + px * scale;
                int sy = y + py * scale;
                _canvas->fillRect(sx, sy, scale, scale, color);
            }
        }
    }
}
