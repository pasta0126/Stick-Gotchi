#include "GotchiRenderer.h"

void GotchiRenderer::start(GotchiPet* pet) {
    _pet     = pet;
    _started = true;
    _suspended = false;
    _lastFrameMs = 0;
    _animFrame = 0;
    _animAccumMs = 0;

    _posX = 120.0f;
    _posY = 62.0f;
    _velX = 0.5f;
    _velY = 0.2f;
    _moveAccumMs = 0;
    _nextDirChangeMs = 2000;

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

void GotchiRenderer::setMiniGame(uint8_t id, uint8_t state, uint8_t frame, bool isHeads) {
    _miniGameId     = id;
    _miniGameState  = state;
    _miniGameFrame  = frame;
    _miniGameIsHeads = isHeads;
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

void GotchiRenderer::_updatePosition(uint32_t deltaMs) {
    // Egg: override with dedicated motion, ignore sleep/physics
    if (_pet->stage() == LifeStage::EGG) {
        if (_animTag == AnimTag::HATCH) {
            _posX = 120.0f + (float)random(-4, 4);
            _posY = 67.0f  + (float)random(-2, 2);
        } else {
            float t = millis() * 0.0015f;
            _posX = 120.0f + sinf(t) * 6.0f;
            _posY = 67.0f  + sinf(t * 1.7f) * 2.5f;
        }
        return;
    }

    if (_pet->isSleeping()) {
        _velX = 0.0f;
        _velY = 0.0f;
        return;
    }

    float baseSpeed = 0.6f;

    switch (_pet->mood()) {
    case Mood::SLEEPING:
        baseSpeed = 0.0f;
        break;
    case Mood::SAD:
        baseSpeed = 0.3f;
        break;
    case Mood::PENSIVE:
        baseSpeed = 0.4f;
        break;
    case Mood::NEUTRAL:
        baseSpeed = 0.6f;
        break;
    case Mood::HAPPY:
        baseSpeed = 1.0f;
        break;
    case Mood::EXCITED:
        baseSpeed = 1.8f;
        break;
    case Mood::SCARED:
        baseSpeed = 2.0f;
        break;
    case Mood::DIZZY:
        baseSpeed = 1.2f;
        break;
    case Mood::SICK:
        baseSpeed = 0.2f;
        break;
    default:
        baseSpeed = 0.6f;
        break;
    }

    if (_pet->mood() == Mood::SICK) {
        _velX += (random(-10, 10) / 100.0f);
        _velY += (random(-10, 10) / 100.0f);
        _velX = max(-baseSpeed, min(baseSpeed, _velX));
        _velY = max(-baseSpeed, min(baseSpeed, _velY));
    }

    if (_pet->mood() == Mood::DIZZY) {
        uint32_t t = millis();
        float angle = (t / 1000.0f) * 3.14159f * 2.0f;
        _velX = cosf(angle) * baseSpeed;
        _velY = sinf(angle) * baseSpeed * 0.5f;
    }

    if (_pet->mood() == Mood::EXCITED) {
        _moveAccumMs += deltaMs;
        if (_moveAccumMs >= 500) {
            _moveAccumMs -= 500;
            float angle = random(0, 628) / 100.0f;
            _velX = cosf(angle) * baseSpeed;
            _velY = sinf(angle) * baseSpeed * 0.7f;
        }
    }

    _moveAccumMs += deltaMs;
    if (_moveAccumMs >= _nextDirChangeMs) {
        _moveAccumMs -= _nextDirChangeMs;
        if (_pet->mood() != Mood::EXCITED && _pet->mood() != Mood::DIZZY) {
            float angle = atan2f(_velY, _velX) + (random(-45, 45) / 100.0f);
            _velX = cosf(angle) * baseSpeed;
            _velY = sinf(angle) * baseSpeed * 0.7f;
        }
        _nextDirChangeMs = 2000 + random(1000, 3000);
    }

    if (_pet->branch() == GotchiBranch::LIBRE) {
        _posY += sinf(millis() * 0.003f) * 0.5f;
    }

    _posX += _velX;
    _posY += _velY;

    if (_posX < PLAY_X0) {
        _posX = PLAY_X0;
        _velX = -_velX + (random(-5, 5) / 100.0f);
    }
    if (_posX > PLAY_X1) {
        _posX = PLAY_X1;
        _velX = -_velX + (random(-5, 5) / 100.0f);
    }

    if (_posY < PLAY_Y0) {
        _posY = PLAY_Y0;
        _velY = -_velY + (random(-5, 5) / 100.0f);
    }
    if (_posY > PLAY_Y1) {
        _posY = PLAY_Y1;
        _velY = -_velY + (random(-5, 5) / 100.0f);
    }
}

void GotchiRenderer::_drawBackground() {
    _canvas->fillRect(0, 18, 240, 90, 0x0640);

    _canvas->drawFastHLine(0, 100, 240, 0x4200);
    _canvas->drawFastHLine(0, 101, 240, 0x2100);

    _canvas->fillRect(8, 92, 2, 8, 0x03E0);
    _canvas->fillRect(4, 90, 4, 2, 0x07E0);
    _canvas->fillRect(9, 87, 4, 2, 0x07E0);

    _canvas->drawLine(228, 25, 232, 29, 0xFFE0);
    _canvas->drawLine(232, 25, 228, 29, 0xFFE0);
    _canvas->drawLine(230, 23, 230, 31, 0xFFE0);

    uint32_t t = millis() / 2000;
    _canvas->fillRect(40 + (t % 8), 35, 1, 1, 0xFFFF);
    _canvas->fillRect(180 - (t % 12), 55, 1, 1, 0xFFFF);
    _canvas->fillRect(120 + (t % 6), 70, 1, 1, 0xFFFF);
}

void GotchiRenderer::_drawEmote(Mood mood, int x, int y) {
    uint16_t bubbleColor = 0xFFFF;
    _canvas->fillRoundRect(x-6, y-12, 12, 10, 2, bubbleColor);
    _canvas->drawRoundRect(x-6, y-12, 12, 10, 2, 0xAAAA);

    _canvas->fillTriangle(x-2, y-2, x+2, y-2, x, y+1, bubbleColor);

    switch (mood) {
    case Mood::HAPPY:
    case Mood::LAUGHING:
        _canvas->fillCircle(x-2, y-8, 2, 0xF800);
        _canvas->fillCircle(x+2, y-8, 2, 0xF800);
        _canvas->fillTriangle(x-3, y-7, x+3, y-7, x, y-4, 0xF800);
        break;
    case Mood::SLEEPING:
        _canvas->drawLine(x-2, y-9, x+2, y-9, 0x001F);
        _canvas->drawLine(x+2, y-9, x-2, y-5, 0x001F);
        _canvas->drawLine(x-2, y-5, x+2, y-5, 0x001F);
        break;
    case Mood::SAD:
        _canvas->fillCircle(x, y-6, 2, 0x001F);
        _canvas->fillTriangle(x-2, y-6, x+2, y-6, x, y-3, 0x001F);
        break;
    case Mood::ANGRY:
    case Mood::ANNOYED:
        _canvas->fillRect(x-3, y-9, 6, 3, 0x8410);
        _canvas->drawLine(x, y-6, x-1, y-4, 0xFFE0);
        _canvas->drawLine(x-1, y-4, x+1, y-4, 0xFFE0);
        break;
    case Mood::SICK:
        _canvas->drawFastHLine(x-2, y-7, 5, 0x07E0);
        _canvas->drawFastVLine(x, y-9, 5, 0x07E0);
        break;
    case Mood::SCARED:
    case Mood::STARTLED:
        _canvas->fillRect(x-1, y-9, 2, 4, 0xFFE0);
        _canvas->fillRect(x-1, y-4, 2, 2, 0xFFE0);
        break;
    case Mood::EXCITED:
        _canvas->drawLine(x, y-9, x, y-5, 0xFFE0);
        _canvas->drawLine(x-3, y-8, x+3, y-6, 0xFFE0);
        _canvas->drawLine(x+3, y-8, x-3, y-6, 0xFFE0);
        break;
    case Mood::PENSIVE:
    case Mood::DIZZY:
        _canvas->drawCircle(x, y-7, 3, 0x8888);
        _canvas->fillRect(x-1, y-8, 2, 2, 0x8888);
        break;
    default:
        _canvas->fillCircle(x, y-7, 1, 0x8888);
        break;
    }
}

void GotchiRenderer::_drawStatsBar() {
    _canvas->fillRect(0, 0, 240, 18, 0x0000);
    _canvas->setTextColor(TFT_WHITE);
    _canvas->setTextSize(1);

    PetStats s = _pet->stats();
    auto col = [](uint8_t v) { return v < 20 ? (uint16_t)TFT_RED : (uint16_t)TFT_WHITE; };

    _canvas->setTextColor(col(s.health));
    _canvas->drawString("HP:" + String(s.health), 2, 4);

    _canvas->setTextColor(col(s.hunger));
    _canvas->drawString("HG:" + String(s.hunger), 50, 4);

    _canvas->setTextColor(col(s.energy));
    _canvas->drawString("EN:" + String(s.energy), 98, 4);

    auto dt = M5.Rtc.getDateTime();
    char timeBuf[6];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", dt.time.hours, dt.time.minutes);
    _canvas->setTextColor(TFT_GREEN);
    _canvas->drawString(timeBuf, 148, 4);

    _canvas->setTextColor(0xAAAA);
    const char* stageStr[] = {"EGG","BABY","YNG","ADLT"};
    char info[12];
    snprintf(info, sizeof(info), "G%d %s",
             _pet->currentID().generation,
             stageStr[(uint8_t)_pet->stage()]);
    _canvas->drawString(info, 190, 4);
}

void GotchiRenderer::_drawActionBar(uint8_t selected, bool visible) {
    if (!visible) return;

    _canvas->fillRect(0, 108, 240, 27, 0x1082);

    const char* labels[] = {"FEED","PLAY","MED","LITE","BATH"};
    for (int i = 0; i < 5; i++) {
        int cx = 24 + i * 48;
        int cy = 121;
        bool active = (i == selected);

        uint16_t bg = active ? 0x07E0 : 0x2104;
        _canvas->fillRoundRect(cx-20, cy-10, 40, 20, 3, bg);
        if (active) _canvas->drawRoundRect(cx-21, cy-11, 42, 22, 3, TFT_WHITE);

        _canvas->setTextColor(active ? TFT_BLACK : 0x8888);
        _canvas->setTextSize(1);
        int tw = strlen(labels[i]) * 6;
        _canvas->drawString(labels[i], cx - tw/2, cy - 3);
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

SpriteFrame GotchiRenderer::_selectSprite(LifeStage stage, GotchiBranch branch, uint8_t frame) {
    if (stage == LifeStage::EGG) {
        if (_animTag == AnimTag::HATCH) {
            const uint8_t* hatchFrames[] = {
                SPR_EGG_HATCH_F0, SPR_EGG_HATCH_F1,
                SPR_EGG_HATCH_F2, SPR_EGG_HATCH_F3
            };
            return SpriteFrame{hatchFrames[frame % EGG_HATCH_FRAMES], EGG_W, EGG_H, 3};
        }
        const uint8_t* idleFrames[] = {SPR_EGG_IDLE_F0, SPR_EGG_IDLE_F1};
        return SpriteFrame{idleFrames[frame % EGG_IDLE_FRAMES], EGG_W, EGG_H, 3};
    }

    if (branch == GotchiBranch::BLOB) {
        if (stage == LifeStage::BABY) {
            return frame == 0 ?
                SpriteFrame{SPR_BLOB_BABY_F0, 16, 16, 3} :
                SpriteFrame{SPR_BLOB_BABY_F1, 16, 16, 3};
        } else if (stage == LifeStage::YOUNG) {
            return frame == 0 ?
                SpriteFrame{SPR_BLOB_YOUNG_F0, 20, 22, 2} :
                SpriteFrame{SPR_BLOB_YOUNG_F1, 20, 22, 2};
        } else {
            return frame == 0 ?
                SpriteFrame{SPR_BLOB_ADULT_F0, 24, 26, 2} :
                SpriteFrame{SPR_BLOB_ADULT_F1, 24, 26, 2};
        }
    } else if (branch == GotchiBranch::PLANT) {
        if (stage == LifeStage::BABY) {
            return frame == 0 ?
                SpriteFrame{SPR_PLANT_BABY_F0, 16, 16, 3} :
                SpriteFrame{SPR_PLANT_BABY_F1, 16, 16, 3};
        } else if (stage == LifeStage::YOUNG) {
            return frame == 0 ?
                SpriteFrame{SPR_PLANT_YOUNG_F0, 20, 22, 2} :
                SpriteFrame{SPR_PLANT_YOUNG_F1, 20, 22, 2};
        } else {
            return frame == 0 ?
                SpriteFrame{SPR_PLANT_ADULT_F0, 24, 26, 2} :
                SpriteFrame{SPR_PLANT_ADULT_F1, 24, 26, 2};
        }
    } else {
        if (stage == LifeStage::BABY) {
            return frame == 0 ?
                SpriteFrame{SPR_LIBRE_BABY_F0, 16, 16, 3} :
                SpriteFrame{SPR_LIBRE_BABY_F1, 16, 16, 3};
        } else if (stage == LifeStage::YOUNG) {
            return frame == 0 ?
                SpriteFrame{SPR_LIBRE_YOUNG_F0, 20, 22, 2} :
                SpriteFrame{SPR_LIBRE_YOUNG_F1, 20, 22, 2};
        } else {
            return frame == 0 ?
                SpriteFrame{SPR_LIBRE_ADULT_F0, 24, 26, 2} :
                SpriteFrame{SPR_LIBRE_ADULT_F1, 24, 26, 2};
        }
    }
}

void GotchiRenderer::_drawFlipCoin() {
    static const uint8_t* const COIN_FRAMES[12] = {
        SPR_COIN_F0, SPR_COIN_F1, SPR_COIN_F2,  SPR_COIN_F3,
        SPR_COIN_F4, SPR_COIN_F5, SPR_COIN_F6,  SPR_COIN_F7,
        SPR_COIN_F8, SPR_COIN_F9, SPR_COIN_F10, SPR_COIN_F11
    };

    // Header
    _canvas->fillRect(0, 0, 240, 18, 0x0000);
    _canvas->setTextColor(0xFEA0);
    _canvas->setTextSize(1);
    _canvas->drawCenterString("FLIP COIN", 120, 4);
    _canvas->setTextColor(0x4208);
    _canvas->drawString("B:salir", 192, 4);

    // Coin sprite — scale 4 (64x64), centered horizontally, y=25
    // Colores exactos de la paleta de la moneda
    SpritePalette pal;
    pal.transparent = 0x0000;
    pal.primary     = 0x7207;  // sombra (113,65,59)
    pal.secondary   = 0xDD2C;  // oro calido (219,164,99)
    pal.dark        = 0xFEA8;  // oro brillante (255,213,65)
    pal.accent      = 0xFFC8;  // highlight (255,252,64)

    constexpr int SCALE = 4;
    constexpr int SW = COIN_W * SCALE;   // 64
    constexpr int SH = COIN_H * SCALE;   // 64
    int coinX = (240 - SW) / 2;          // 88
    int coinY = 28;
    _drawSprite(COIN_FRAMES[_miniGameFrame], COIN_W, COIN_H, coinX, coinY, SCALE, pal);

    // Bottom area
    _canvas->setTextSize(1);
    if (_miniGameState == 2) {  // RESULT
        _canvas->setTextSize(2);
        _canvas->setTextColor(_miniGameIsHeads ? (uint16_t)0xFEA8 : (uint16_t)0xFFFF);
        _canvas->drawCenterString(_miniGameIsHeads ? "CARA" : "CRUZ", 120, 100);
        _canvas->setTextSize(1);
        _canvas->setTextColor(0x4208);
        _canvas->drawCenterString("A: otra vez  B: salir", 120, 122);
    } else {
        _canvas->setTextColor(0x4208);
        _canvas->drawCenterString("A: lanzar  B: salir", 120, 115);
    }
}

void GotchiRenderer::_drawFrame() {
    if (!_display || !_display->acquire(100)) return;

    _canvas = &_display->canvas();
    _canvas->fillScreen(TFT_BLACK);

    if (_miniGameId == 1) {
        _lastFrameMs = millis();  // evita acumulacion de delta al volver al gotchi
        _drawFlipCoin();
        _canvas->pushSprite(0, 0);
        _display->release();
        return;
    }

    if (_pet->isDead()) {
        GotchiVisual vis = decodeVisual(_pet->currentID().visual_seed);
        SpritePalette pal = _buildPalette(vis, LifeStage::EGG, GotchiBranch::BLOB);
        int cx = 120;
        int cy = 62;
        _drawSprite(SPR_DEATH, 16, 16,
                    cx - (16 * 2) / 2, cy - (16 * 2) / 2,
                    2, pal);
    } else {
        _drawBackground();

        uint32_t now = millis();
        uint32_t delta = now - _lastFrameMs;
        if (delta < 16) delta = 16;
        _lastFrameMs = now;
        _animAccumMs += delta;
        if (_animAccumMs >= FRAME_INTERVAL_MS) {
            _animAccumMs -= FRAME_INTERVAL_MS;
            _animFrame = (_animFrame + 1) % 2;
        }
        _updatePosition(delta);

        GotchiVisual vis = decodeVisual(_pet->currentID().visual_seed);
        SpritePalette pal = _buildPalette(vis, _pet->stage(), _pet->branch());

        // Manage egg animation state
        if (_pet->stage() == LifeStage::EGG) {
            if (_pet->isEggHatched()) {
                _animTag = AnimTag::HATCH;
                if (!_hatchDone) {
                    _hatchAccumMs += delta;
                    if (_hatchAccumMs >= HATCH_FRAME_MS) {
                        _hatchAccumMs -= HATCH_FRAME_MS;
                        if (_hatchFrame < EGG_HATCH_FRAMES - 1)
                            _hatchFrame++;
                        else
                            _hatchDone = true;
                    }
                }
            } else {
                _animTag = AnimTag::IDLE;
            }
        }

        uint8_t frameIdx = (_animTag == AnimTag::HATCH) ? _hatchFrame : _animFrame;
        SpriteFrame frame = _selectSprite(_pet->stage(), _pet->branch(), frameIdx);

        int spriteX = (int)_posX - (frame.w * frame.scale) / 2;
        int spriteY = (int)_posY - (frame.h * frame.scale) / 2;
        spriteY = max(PLAY_Y0, min(PLAY_Y1 - frame.h * frame.scale, spriteY));

        _drawSprite(frame.data, frame.w, frame.h, spriteX, spriteY, frame.scale, pal);

        if (_pet->mood() != Mood::NEUTRAL && _pet->stage() != LifeStage::EGG) {
            _drawEmote(_pet->mood(), (int)_posX, spriteY - 2);
        }

        if (_pet->isSleeping() && _pet->stage() != LifeStage::EGG) {
            _drawSleepZs((int)_posX + 10, spriteY);
        }

        _drawStatsBar();
        if (_pet->stage() == LifeStage::EGG && _hatchDone) {
            _drawHatchPrompt();
        } else {
            _drawActionBar(_selectedAction, _actionBarVisible);
        }
    }

    _canvas->pushSprite(0, 0);
    _display->release();

}

GotchiRenderer::SpritePalette GotchiRenderer::_buildPalette(const GotchiVisual& vis, LifeStage stage, GotchiBranch branch) {
    SpritePalette pal;

    pal.transparent = 0x0000;
    pal.accent = 0xFFFF;

    uint8_t sat_primary = 200;
    uint8_t val_primary = 220;
    uint8_t sat_secondary = 160;
    uint8_t val_secondary = 240;

    if (stage == LifeStage::EGG) {
        sat_primary   = 210;
        val_primary   = 200;
        sat_secondary = 180;
        val_secondary = 220;
    } else if (stage == LifeStage::ADULT) {
        sat_primary = 220;
        val_primary = 240;
        sat_secondary = 180;
        val_secondary = 255;
    }

    uint8_t hue_primary   = vis.hue_primary;
    uint8_t hue_secondary = vis.hue_secondary;

    if (branch == GotchiBranch::PLANT) {
        hue_primary = 5 + (millis() / 10000) % 6;
        hue_secondary = 28 + (millis() / 15000) % 4;
    } else if (branch == GotchiBranch::LIBRE) {
        hue_primary = 15 + (millis() / 12000) % 10;
        val_primary = min(255, val_primary + 20);
    }

    pal.primary = hsvToRgb565(hue_primary, sat_primary, val_primary);
    pal.secondary = hsvToRgb565(hue_secondary, sat_secondary, val_secondary);
    pal.dark = hsvToRgb565(hue_primary, 200, 100);

    return pal;
}

void GotchiRenderer::resetHatch() {
    _hatchDone    = false;
    _hatchFrame   = 0;
    _hatchAccumMs = 0;
    _animTag      = AnimTag::IDLE;
    _animFrame    = 0;
}

void GotchiRenderer::_drawHatchPrompt() {
    _canvas->fillRect(0, 108, 240, 27, 0x0861);
    _canvas->setTextColor(TFT_WHITE);
    _canvas->setTextSize(1);
    _canvas->drawCenterString("[ Btn A ]  nuevo huevo", 120, 118);
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
            case 4: color = pal.accent; break;
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
