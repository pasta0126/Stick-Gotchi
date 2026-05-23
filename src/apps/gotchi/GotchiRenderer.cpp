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
    triggerWake();
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

void GotchiRenderer::setMiniGame(uint8_t id, uint8_t state, uint8_t frame, uint8_t extra) {
    _miniGameId    = id;
    _miniGameState = state;
    _miniGameFrame = frame;
    _miniGameExtra = extra;
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

    GotchiType t = _pet->gotchiType();
    if (t == GotchiType::ENERGY || t == GotchiType::SOUL) {
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
    // Sky gradient: 3 bandas azul oscuro -> azul claro
    _canvas->fillRect(0, 18, 240, 30, 0x4419);  // top sky
    _canvas->fillRect(0, 48, 240, 32, 0x5D1A);  // mid sky
    _canvas->fillRect(0, 80, 240, 15, 0x75DC);  // lower sky

    // Franja de horizonte
    _canvas->fillRect(0, 95, 240, 3, 0x7DEB);

    // Suelo
    _canvas->fillRect(0, 98, 240, 5, 0x5D08);   // cesped
    _canvas->fillRect(0, 103, 240, 5, 0x33C5);  // tierra

    // Sol (esquina superior derecha)
    _canvas->fillRect(213, 20, 14, 14, 0xFEA6);
    _canvas->drawFastHLine(206, 27, 28, 0xFEA6);  // rayo horizontal
    _canvas->drawFastVLine(220, 13, 28, 0xFEA6);  // rayo vertical

    // Nube 1 (izquierda)
    _canvas->fillRect(18, 34, 42, 7, 0xFFFF);
    _canvas->fillRect(26, 28, 26, 6, 0xFFFF);
    _canvas->fillRect(35, 24, 14, 4, 0xFFFF);

    // Nube 2 (centro-derecha)
    _canvas->fillRect(132, 46, 36, 6, 0xFFFF);
    _canvas->fillRect(140, 41, 22, 5, 0xFFFF);

    // Nube 3 (pequena, centro-izquierda)
    _canvas->fillRect(80, 60, 26, 5, 0xFFFF);
    _canvas->fillRect(87, 56, 14, 4, 0xFFFF);
}

void GotchiRenderer::_drawHabitat(GotchiType type) {
    switch (type) {

    case GotchiType::ORGANIC:
        _drawBackground();
        break;

    case GotchiType::CRYSTAL: {
        // Ice cave — dark navy background, icy ground, crystal spikes
        _canvas->fillRect(0, 18, 240, 90, 0x0010);
        // Stalactites from ceiling
        const int stx[] = {40, 80, 130, 175, 210};
        const int stw[] = {12, 7,  16,  9,   11};
        const int sth[] = {22, 14, 18, 20,   15};
        for (int i = 0; i < 5; i++) {
            _canvas->fillTriangle(stx[i], 18, stx[i]+stw[i], 18,
                                  stx[i]+stw[i]/2, 18+sth[i], 0x4DF7);
        }
        // Crystal floor columns
        const int cpx[] = {15, 65, 140, 190};
        const int cpw[] = {10, 6, 12, 8};
        for (int i = 0; i < 4; i++) {
            _canvas->fillRect(cpx[i], 85, cpw[i], 13, 0x8EFF);
            _canvas->drawRect(cpx[i], 85, cpw[i], 13, 0xCEFF);
        }
        // Ground: icy platform
        _canvas->fillRect(0, 97, 240, 11, 0x2D7F);
        _canvas->fillRect(0, 98, 240, 3,  0x8EFF);  // bright ice surface
        break;
    }

    case GotchiType::ENERGY: {
        // Plasma field — dark background, glowing grid, energy glow
        _canvas->fillRect(0, 18, 240, 90, 0x080C);
        // Grid lines
        for (int y = 25; y < 96; y += 8)
            _canvas->drawFastHLine(0, y, 240, 0x2104);
        for (int x = 0; x < 240; x += 20)
            _canvas->drawFastVLine(x, 18, 78, 0x2104);
        // Energy nodes at grid intersections
        const int enx[] = {40, 100, 160, 220, 20, 80, 140, 200};
        const int eny[] = {33, 41, 33, 57, 57, 65, 49, 41};
        for (int i = 0; i < 8; i++)
            _canvas->fillRect(enx[i]-1, eny[i]-1, 3, 3, 0xFFE0);
        // Glow ground
        _canvas->fillRect(0, 95, 240, 2,  0xFFE0);
        _canvas->fillRect(0, 97, 240, 11, 0x2104);
        break;
    }

    case GotchiType::CYBER: {
        // Cyberpunk room — dark walls, neon accents, lit floor grid
        _canvas->fillRect(0, 18, 240, 90, 0x0821);
        // Dark panels on walls
        _canvas->fillRect(4,  25, 50, 55, 0x0842);
        _canvas->fillRect(186, 25, 50, 55, 0x0842);
        _canvas->drawRect(4,  25, 50, 55, 0x03EF);
        _canvas->drawRect(186, 25, 50, 55, 0x03EF);
        // Neon strips on walls
        _canvas->drawFastHLine(0, 24, 240, 0xF800);
        _canvas->drawFastHLine(0, 93, 240, 0x03EF);
        // Floor with cyan grid
        _canvas->fillRect(0, 94, 240, 14, 0x1082);
        for (int x = 0; x < 240; x += 24)
            _canvas->drawFastVLine(x, 94, 14, 0x03EF);
        _canvas->drawFastHLine(0, 101, 240, 0x03EF);
        break;
    }

    case GotchiType::ELEMENTAL: {
        // Volcano — red-orange sky gradient, rocky ground, lava crack
        _canvas->fillRect(0, 18, 240, 28, 0x8000);   // dark red top sky
        _canvas->fillRect(0, 46, 240, 28, 0xC820);   // mid orange
        _canvas->fillRect(0, 74, 240, 22, 0xFC40);   // bright orange horizon
        // Rock formations
        _canvas->fillRect(0,  85, 28, 13, 0x2104);
        _canvas->fillRect(212, 82, 28, 16, 0x2104);
        _canvas->fillTriangle(0, 85, 28, 85, 14, 70, 0x2104);
        _canvas->fillTriangle(212, 82, 240, 82, 226, 66, 0x2104);
        // Rocky ground
        _canvas->fillRect(0, 97, 240, 11, 0x3186);
        // Lava crack glow
        _canvas->drawFastHLine(0, 96, 240, 0xFD20);
        _canvas->drawFastHLine(0, 97, 240, 0xFC00);
        break;
    }

    case GotchiType::SOUL: {
        // Cosmic void — deep space, stars, floating platform
        _canvas->fillRect(0, 18, 240, 90, 0x0801);
        // Stars (fixed positions)
        const int sx[] = {12, 34, 58, 82, 110, 140, 168, 198, 220,
                           25, 70, 95, 130, 160, 210, 45, 155, 185};
        const int sy[] = {22, 35, 28, 44,  24,  38,  28,  22,  42,
                           55, 50, 32,  60,  48,  35,  68,  30,  58};
        for (int i = 0; i < 18; i++) {
            uint16_t starCol = (i % 3 == 0) ? 0xFFFF : (i % 3 == 1) ? 0xCCCC : 0xA01F;
            _canvas->fillRect(sx[i], sy[i], (i % 4 == 0) ? 2 : 1, (i % 4 == 0) ? 2 : 1, starCol);
        }
        // Floating platform
        _canvas->fillRect(60, 93, 120, 7,  0x5820);
        _canvas->drawRect(60, 93, 120, 7,  0xA01F);
        _canvas->drawFastHLine(60, 94, 120, 0xD01F);  // top shine
        // Void floor
        _canvas->fillRect(0, 100, 240, 8, 0x0000);
        break;
    }

    default:
        _drawBackground();
        break;
    }
}

void GotchiRenderer::_drawEmote(Mood mood, int x, int y) {
    // Bubble sized to hold 8×8 sprite at scale 2 (→16×16) with 3px padding each side
    constexpr int BW = 22, BH = 20;
    int bx = x - BW / 2;
    int by = y - BH - 4;

    uint16_t bubbleColor = 0xFFFF;
    _canvas->fillRoundRect(bx, by, BW, BH, 3, bubbleColor);
    _canvas->drawRoundRect(bx, by, BW, BH, 3, 0xC618);
    _canvas->fillTriangle(x - 2, by + BH, x + 2, by + BH, x, by + BH + 4, bubbleColor);

    SpritePalette pal;
    pal.transparent = 0x0001;  // sentinel — never matches real colors
    pal.primary     = 0x2104;  // dark grey — main lines
    pal.secondary   = 0x001F;  // blue — tears / sweat drops
    pal.dark        = 0x8410;  // mid grey — shadow
    pal.accent      = 0xFFE0;  // yellow — sparkle / anger marks
    pal.color5      = 0x0000;

    int sx = bx + (BW - EMOTE_W * EMOTE_SCALE) / 2;
    int sy = by + (BH - EMOTE_H * EMOTE_SCALE) / 2;
    _drawSprite(gotchiEmoteSprite(mood), EMOTE_W, EMOTE_H, sx, sy, EMOTE_SCALE, pal);
}

void GotchiRenderer::_drawStatsBar() {
    _canvas->fillRect(0, 0, 240, 18, 0x0000);
    _canvas->setTextColor(TFT_WHITE);
    _canvas->setTextSize(1);

    GotchiType type = _pet->gotchiType();
    uint16_t typeColor = gotchiTypeColor(type);

    // Left accent: 3px stripe + 8×8 type icon
    _canvas->fillRect(0, 0, 3, 18, typeColor);
    SpritePalette iconPal;
    iconPal.transparent = 0x0001;
    iconPal.primary     = typeColor;
    iconPal.secondary   = 0xFFFF;
    iconPal.dark        = 0x0000;
    iconPal.accent      = 0xFFFF;
    iconPal.color5      = 0x0000;
    _drawSprite(gotchiTypeIcon(type), TYPE_ICON_W, TYPE_ICON_H, 3, 5, 1, iconPal);

    // Bottom accent line
    _canvas->drawFastHLine(0, 17, 240, typeColor);

    PetStats s = _pet->stats();

    auto barColor = [](uint8_t v) -> uint16_t {
        if (v > 60) return 0x07E0;   // green
        if (v > 30) return 0xFFE0;   // yellow
        return 0xF800;               // red
    };
    auto drawGauge = [&](int x, int y, const uint8_t* icon, uint8_t val) {
        uint16_t bc = barColor(val);
        SpritePalette p;
        p.transparent = 0x0001; p.primary = bc; p.secondary = 0xFFFF;
        p.dark = 0x0000; p.accent = 0xFFFF; p.color5 = 0x0000;
        _drawSprite(icon, HUD_ICON_W, HUD_ICON_H, x, y + 1, 1, p);
        int bx = x + HUD_ICON_W + 1;
        _canvas->drawRect(bx, y, 30, 8, 0x4208);
        int fill = (28 * val) / 100;
        if (fill > 0) _canvas->fillRect(bx + 1, y + 1, fill, 6, bc);
    };

    drawGauge(13, 5, SPR_HUD_HP,     s.health);
    drawGauge(51, 5, SPR_HUD_HUNGER, s.hunger);
    drawGauge(89, 5, SPR_HUD_ENERGY, s.energy);

    auto dt = M5.Rtc.getDateTime();
    char timeBuf[6];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", dt.time.hours, dt.time.minutes);
    _canvas->setTextColor(TFT_GREEN);
    _canvas->drawString(timeBuf, 148, 4);

    const char* stageStr[] = {"EGG","BABY","YNG","ADLT"};
    if (_pet->stage() == LifeStage::ADULT) {
        const char* formStr[] = {"HLT","NRM","NEG"};
        char info[16];
        snprintf(info, sizeof(info), "%s G%d %s/%s",
                 gotchiTypeShortName(type),
                 _pet->currentID().generation,
                 stageStr[(uint8_t)_pet->stage()],
                 formStr[(uint8_t)_pet->adultForm()]);
        uint16_t formColor = (_pet->adultForm() == AdultForm::HEALTHY)   ? (uint16_t)0x07E0 :
                             (_pet->adultForm() == AdultForm::NEGLECTED) ? (uint16_t)0xF800 : (uint16_t)0xAAAA;
        _canvas->setTextColor(formColor);
        _canvas->drawString(info, 162, 4);
    } else {
        char info[12];
        snprintf(info, sizeof(info), "%s G%d %s",
                 gotchiTypeShortName(type),
                 _pet->currentID().generation,
                 stageStr[(uint8_t)_pet->stage()]);
        _canvas->setTextColor(typeColor);
        _canvas->drawString(info, 174, 4);
    }
}

void GotchiRenderer::_drawActionBar(uint8_t selected, bool visible) {
    if (!visible) return;

    _canvas->fillRect(0, 108, 240, 27, 0x1082);

    for (int i = 0; i < 5; i++) {
        int cx = 24 + i * 48;
        int cy = 121;
        bool active = (i == selected);

        uint16_t bg = active ? 0x07E0 : 0x2104;
        _canvas->fillRoundRect(cx - 20, cy - 10, 40, 20, 3, bg);
        if (active) _canvas->drawRoundRect(cx - 21, cy - 11, 42, 22, 3, TFT_WHITE);

        SpritePalette pal;
        pal.transparent = 0x0001;
        pal.primary     = active ? (uint16_t)0x0000 : (uint16_t)0x8888;
        pal.secondary   = active ? (uint16_t)0x001F : (uint16_t)0x4444;
        pal.dark        = active ? (uint16_t)0x2104 : (uint16_t)0x4208;
        pal.accent      = 0xFFFF;
        pal.color5      = 0x0000;

        int sx = cx - (ACTION_ICON_W * ACTION_ICON_SCALE) / 2;
        int sy = cy - (ACTION_ICON_H * ACTION_ICON_SCALE) / 2;
        _drawSprite(gotchiActionIcon(i), ACTION_ICON_W, ACTION_ICON_H, sx, sy, ACTION_ICON_SCALE, pal);
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

void GotchiRenderer::_drawIndicators() {
    struct Indicator { const uint8_t* data; uint16_t color; bool show; };
    Indicator inds[] = {
        { SPR_IND_SICK,   0xF800, _pet->isSick() },
        { SPR_IND_HUNGRY, 0xFD20, _pet->stats().hunger < 20 && !_pet->isSleeping() },
        { SPR_IND_DIRTY,  0xC618, _pet->dirtyness() > 70 },
    };

    SpritePalette pal;
    pal.transparent = 0x0001;
    pal.secondary   = 0xFFFF;
    pal.dark        = 0x0000;
    pal.accent      = 0xFFFF;
    pal.color5      = 0x0000;

    int iy = 22;
    for (auto& ind : inds) {
        if (!ind.show) continue;
        pal.primary = ind.color;
        _drawSprite(ind.data, IND_ICON_W, IND_ICON_H, 228, iy, 1, pal);
        iy += 10;
    }
}

SpriteFrame GotchiRenderer::_selectSprite(LifeStage stage, GotchiType type, uint8_t frame) {
    if (stage == LifeStage::EGG) {
        if (_animTag == AnimTag::HATCH) {
            const uint8_t* hatchFrames[] = {
                SPR_EGG_HATCH_F0, SPR_EGG_HATCH_F1,
                SPR_EGG_HATCH_F2, SPR_EGG_HATCH_F3
            };
            return SpriteFrame{hatchFrames[frame % EGG_HATCH_FRAMES], EGG_W, EGG_H, 3};
        }
        return gotchiTypeEggFrame(type, frame);
    }
    if (stage == LifeStage::BABY)  return gotchiTypeBabyFrame(type, frame);
    if (stage == LifeStage::YOUNG) return gotchiTypeYoungFrame(type, frame);
    return gotchiTypeAdultFrame(type, frame);
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
        _canvas->setTextColor(_miniGameExtra ? (uint16_t)0xFEA8 : (uint16_t)0xFFFF);
        _canvas->drawCenterString(_miniGameExtra ? "CARA" : "CRUZ", 120, 100);
        _canvas->setTextSize(1);
        _canvas->setTextColor(0x4208);
        _canvas->drawCenterString("A: otra vez  B: salir", 120, 122);
    } else {
        _canvas->setTextColor(0x4208);
        _canvas->drawCenterString("A: lanzar  B: salir", 120, 115);
    }
}

void GotchiRenderer::_drawMagic8Ball() {
    static const uint8_t* const BALL_FRAMES[4] = {
        SPR_8BALL_F0, SPR_8BALL_F1, SPR_8BALL_F2, SPR_8BALL_F3
    };
    static const char* const RESULTS[13] = {
        "",
        "100% real",
        "Casi seguro",
        "Good vibes",
        "Si",
        "Dale una vuelta",
        "Ahora mismo no",
        "Sigue buscando",
        "Mejor no saberlo",
        "No",
        "Ni de broma",
        "Mala vibra",
        "Imposible",
    };

    // Dark background
    _canvas->fillScreen(0x0841);

    // Header
    _canvas->fillRect(0, 0, 240, 18, 0x0000);
    _canvas->setTextColor(0xFFFF);
    _canvas->setTextSize(1);
    _canvas->drawCenterString("BOLA 8", 120, 4);
    _canvas->setTextColor(0x4208);
    _canvas->drawString("B:salir", 192, 4);

    // Ball sprite (scale 4 = 64x64), centered
    SpritePalette pal;
    pal.transparent = 0x0841;  // matches background so transparent blends in
    pal.primary     = 0x0000;  // black outline/lines
    pal.secondary   = 0x2104;  // dark grey shadow
    pal.dark        = 0x52AA;  // mid grey body
    pal.accent      = 0xAD55;  // light grey highlight
    pal.color5      = 0xFFFF;  // white window

    constexpr int SCALE = 4;
    constexpr int SW    = BALL8_W * SCALE;
    constexpr int SH    = BALL8_H * SCALE;
    int ballX = (240 - SW) / 2;
    int ballY = 26;
    _drawSprite(BALL_FRAMES[_miniGameFrame], BALL8_W, BALL8_H, ballX, ballY, SCALE, pal);

    // Bottom area
    _canvas->setTextSize(1);
    uint8_t state    = _miniGameState;  // BallState as uint8_t: 0=IDLE,1=SHAKING,2=EASING,3=RESULT
    uint8_t resultId = _miniGameExtra;

    if (state == 3 && resultId >= 1 && resultId <= 12) {  // RESULT
        uint16_t textColor;
        if      (resultId <= 4)  textColor = 0x07E0;  // green  (positive)
        else if (resultId <= 8)  textColor = 0xFFE0;  // yellow (neutral)
        else                     textColor = 0xF800;  // red    (negative)

        _canvas->fillRect(0, 108, 240, 27, 0x0000);
        _canvas->setTextSize(2);
        _canvas->setTextColor(textColor);
        _canvas->drawCenterString(RESULTS[resultId], 120, 111);
        _canvas->setTextSize(1);
        _canvas->setTextColor(0x4208);
        _canvas->drawCenterString("A: otra vez  B: salir", 120, 128);
    } else if (state == 0) {  // IDLE
        _canvas->fillRect(0, 108, 240, 27, 0x0000);
        _canvas->setTextColor(0x4208);
        _canvas->drawCenterString("Agita para consultar", 120, 120);
    }
}

void GotchiRenderer::_drawFrame() {
    if (!_display || !_display->acquire(100)) return;

    _canvas = &_display->canvas();
    _canvas->fillScreen(TFT_BLACK);

    if (_miniGameId == 1) {
        _lastFrameMs = millis();
        _drawFlipCoin();
        _canvas->pushSprite(0, 0);
        _display->release();
        return;
    }

    if (_miniGameId == 2) {
        _lastFrameMs = millis();
        _drawMagic8Ball();
        _canvas->pushSprite(0, 0);
        _display->release();
        return;
    }

    if (_pet->isDead()) {
        _drawDeathScreen();
    } else {
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
        GotchiType type  = _pet->gotchiType();

        _drawCreatureAtmosphere(delta);
        SpritePalette pal = _buildPalette(vis, _pet->stage(), type);

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
        SpriteFrame frame = _selectSprite(_pet->stage(), type, frameIdx);

        // Z-depth: scale sprite based on _zDepth (0=FAR, 1=NORMAL, 2=PEEK)
        _updateZ(delta);
        float screenH = 27.0f + (_zDepth / 2.0f) * 108.0f;
        frame.scale = (uint8_t)max(1, (int)roundf(screenH / (float)frame.h));

        // X: use wandering position; Y: centered vertically (Z controls apparent size)
        int spriteX = (int)_posX - (frame.w * frame.scale) / 2;
        spriteX = max(PLAY_X0, min(PLAY_X1 - frame.w * frame.scale, spriteX));
        int spriteY = 67 - (frame.h * frame.scale) / 2;  // centered; overflows at PEEK

        _drawSprite(frame.data, frame.w, frame.h, spriteX, spriteY, frame.scale, pal);
        _lastSpriteTopY = spriteY;

        if (_pet->mood() != Mood::NEUTRAL && _pet->stage() != LifeStage::EGG) {
            _drawEmote(_pet->mood(), (int)_posX, spriteY - 2);
        }

        _drawSpeechBubble(delta);

        if (_pet->isSleeping() && _pet->stage() != LifeStage::EGG) {
            _drawSleepZs((int)_posX + 10, spriteY);
        }

        if (_pet->isAgony()) {
            _drawAgonyOverlay();
        }

        if (_moodPeekMs > 0) {
            _moodPeekInMs += delta;
            _drawMoodPeek();
            _moodPeekMs = (_moodPeekMs > delta) ? _moodPeekMs - delta : 0;
            if (_moodPeekMs == 0) _moodPeekInMs = 0;
        } else {
            _moodPeekInMs = 0;
        }

        _applyTransitionOverlay(delta);
    }

    _canvas->pushSprite(0, 0);
    _display->release();

}

GotchiRenderer::SpritePalette GotchiRenderer::_buildPalette(const GotchiVisual& vis, LifeStage stage, GotchiType type) {
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
    } else if (stage == LifeStage::ADULT && _pet) {
        switch (_pet->adultForm()) {
        case AdultForm::HEALTHY:
            sat_primary = 240; val_primary = 255;
            sat_secondary = 210; val_secondary = 255;
            pal.accent = 0xFFE0;  // golden sparkle accent
            break;
        case AdultForm::NEGLECTED:
            sat_primary = 80;  val_primary = 140;
            sat_secondary = 60; val_secondary = 120;
            pal.accent = 0x4208;  // dim grey accent
            break;
        default:  // NORMAL
            sat_primary = 220; val_primary = 240;
            sat_secondary = 180; val_secondary = 255;
            break;
        }
    }

    uint8_t hue_primary   = vis.hue_primary;
    uint8_t hue_secondary = vis.hue_secondary;

    if (type == GotchiType::ELEMENTAL) {
        hue_primary = 5 + (millis() / 10000) % 6;
        hue_secondary = 28 + (millis() / 15000) % 4;
    } else if (type == GotchiType::ENERGY || type == GotchiType::SOUL) {
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

void GotchiRenderer::_drawDeathScreen() {
    static const char* CAUSE_NAMES[] = { "hambre", "enfermedad", "suciedad", "maltrato", "abandono" };

    _canvas->fillScreen(0x0000);
    _canvas->fillRect(0, 0, 240, 18, 0x0000);

    // Skull sprite centered in play area
    GotchiVisual vis = decodeVisual(_pet->currentID().visual_seed);
    SpritePalette pal = _buildPalette(vis, LifeStage::EGG, GotchiType::ORGANIC);
    pal.primary   = 0xC618;  // grey skull
    pal.secondary = 0xFFFF;
    constexpr int SKULL_SCALE = 3;
    constexpr int SKULL_W = 16, SKULL_H = 16;
    int sx = 120 - (SKULL_W * SKULL_SCALE) / 2;
    int sy = 28;
    _drawSprite(SPR_DEATH, SKULL_W, SKULL_H, sx, sy, SKULL_SCALE, pal);

    _canvas->setTextSize(1);

    // Generation and days
    char genBuf[24];
    uint8_t cause = _pet->lastDeathCause();
    snprintf(genBuf, sizeof(genBuf), "Gen %d  |  %d dias",
             _pet->currentID().generation,
             _pet->lastDaysLived());
    _canvas->setTextColor(0x8410);
    _canvas->drawCenterString(genBuf, 120, 80);

    // Cause of death
    char causeBuf[20];
    const char* causeName = (cause < 5) ? CAUSE_NAMES[cause] : "desconocido";
    snprintf(causeBuf, sizeof(causeBuf), "causa: %s", causeName);
    _canvas->setTextColor(0xF800);
    _canvas->drawCenterString(causeBuf, 120, 90);

    // Prompt
    _canvas->fillRect(0, 108, 240, 27, 0x0861);
    _canvas->setTextColor(TFT_WHITE);
    _canvas->drawCenterString("[ Btn A ]  nuevo huevo", 120, 118);
}

void GotchiRenderer::_drawAgonyOverlay() {
    // Pulse red border at ~1 Hz to signal imminent death
    if ((millis() / 500) % 2 == 0) {
        uint16_t red = 0xF800;
        _canvas->drawRect(0,  18, 240, 90, red);
        _canvas->drawRect(1,  19, 238, 88, red);
    }
}

void GotchiRenderer::_drawHatchPrompt() {
    _canvas->fillRect(0, 108, 240, 27, 0x0861);
    _canvas->setTextColor(TFT_WHITE);
    _canvas->setTextSize(1);
    _canvas->drawCenterString("[ Btn A ]  eclosionar", 120, 118);
}

void GotchiRenderer::_drawCreatureAtmosphere(uint32_t /*deltaMs*/) {
    CreatureType ct = _pet->creature();
    uint32_t t = millis();

    switch (ct) {

    case CreatureType::BYTEE: {
        // Deep space purple — #1a0a2e → RGB565 0x1845
        _canvas->fillScreen(0x1845);
        // Fixed star field with per-star twinkle
        static const int16_t STARS[][2] = {
            {12,8},{40,22},{68,14},{95,35},{130,8},{160,28},{190,12},{220,40},
            {30,50},{80,65},{140,45},{200,60},{55,78},{110,90},{170,72},
            {15,100},{90,110},{210,95}
        };
        for (int i = 0; i < 18; i++) {
            uint32_t period = 500 + (uint32_t)i * 137;
            bool lit = ((t % period) < (period * 2 / 3));
            if (lit) {
                uint16_t col = (i % 5 == 0) ? (uint16_t)0xFEC0 : (uint16_t)0xCE59;
                _canvas->drawPixel(STARS[i][0], STARS[i][1], col);
            }
        }
        // 3 slow gold sparkles drifting upward
        for (int i = 0; i < 3; i++) {
            int gy = 130 - (int)((t / 100 + i * 430) % 135);
            int gx = 20 + i * 90 + (int)(sinf(t * 0.001f + i * 2.1f) * 18);
            bool vis2 = ((t / 300 + i * 7) % 5) < 3;
            if (vis2) _canvas->fillRect(gx, gy, 2, 2, 0xFEC0);
        }
        break;
    }

    case CreatureType::CTHULHU: {
        // Deep ocean green — #0a1a0f → RGB565 0x08C1
        _canvas->fillScreen(0x08C1);
        // Slow mist bands
        for (int i = 0; i < 3; i++) {
            int my = 25 + i * 38 + (int)(sinf(t * 0.0003f + i * 1.5f) * 9);
            _canvas->fillRect(0, my, 240, 3, 0x0182);
        }
        // 3 bubbles rising at staggered rates
        for (int i = 0; i < 3; i++) {
            uint32_t phase = (t / 70 + (uint32_t)i * 450) % 140;
            int bx = 35 + i * 80 + (int)(sinf(t * 0.0008f + i) * 14);
            int by = 134 - (int)phase;
            _canvas->drawCircle(bx, by, 2, 0x07C6);
            _canvas->drawPixel(bx - 1, by - 2, 0x0564);
        }
        break;
    }

    case CreatureType::JACK: {
        // Stone grey — #1a1a1a → RGB565 0x18C3
        _canvas->fillScreen(0x18C3);
        // Very rare raindrop: one drop every ~4 seconds
        uint32_t cycle = t / 80;
        if (cycle % 50 < 4) {
            uint32_t seed = cycle / 50;
            int rx = (int)((seed * 73u + 17u) % 200) + 20;
            int ry = (int)((cycle % 50) * 3);
            _canvas->drawFastVLine(rx, ry, 5, 0x39C7);
        }
        break;
    }

    case CreatureType::LUMI: {
        // Twilight lavender gradient top→bottom
        for (int y = 0; y < 135; y += 4) {
            uint8_t r = 0x1a + (uint8_t)(y * 16 / 135);
            uint8_t g = 0x0a;
            uint8_t b = 0x2e + (uint8_t)(y * 16 / 135);
            uint16_t col = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
            _canvas->fillRect(0, y, 240, 4, col);
        }
        // Star twinkles
        static const int16_t LSTARS[][2] = {
            {15,10},{50,8},{90,20},{130,5},{175,15},{210,8},
            {35,35},{75,28},{120,38},{180,30},{220,45},
            {10,55},{60,50},{100,62},{155,48}
        };
        for (int i = 0; i < 15; i++) {
            uint32_t period = 700 + (uint32_t)i * 210;
            bool lit = ((t % period) < (period / 3));
            if (lit) {
                uint16_t col = (i % 3 == 0) ? (uint16_t)0xFFFF :
                               (i % 3 == 1) ? (uint16_t)0xE71C : (uint16_t)0xDEFB;
                _canvas->drawPixel(LSTARS[i][0], LSTARS[i][1], col);
                if (i % 5 == 0) {
                    _canvas->drawPixel(LSTARS[i][0] + 1, LSTARS[i][1], col);
                    _canvas->drawPixel(LSTARS[i][0], LSTARS[i][1] + 1, col);
                }
            }
        }
        // 2 flower petals drifting down
        for (int i = 0; i < 2; i++) {
            uint32_t phase = (t / 60 + (uint32_t)i * 1100) % 200;
            if (phase < 135) {
                int px = 40 + i * 130 + (int)(sinf(t * 0.0009f + i * 2.0f) * 18);
                int py = (int)phase;
                _canvas->fillRect(px, py, 3, 2, 0xF81F);
            }
        }
        break;
    }

    default:
        _drawBackground();
        break;
    }
}

void GotchiRenderer::_updateZ(uint32_t deltaMs) {
    // Per-creature Z personality
    CreatureType ct = _pet->creature();
    float zDefault = 1.0f, zMin = 0.4f, zMax = 1.7f, zSpeed = 0.003f;
    switch (ct) {
    case CreatureType::BYTEE:   zDefault=1.0f; zMin=0.4f; zMax=1.7f; zSpeed=0.003f; break;
    case CreatureType::CTHULHU: zDefault=0.9f; zMin=0.2f; zMax=2.0f; zSpeed=0.002f; break;
    case CreatureType::JACK:    zDefault=1.0f; zMin=0.8f; zMax=1.1f; zSpeed=0.0005f; break;
    case CreatureType::LUMI:    zDefault=1.1f; zMin=0.5f; zMax=1.8f; zSpeed=0.008f; break;
    default: break;
    }

    // Mood → Z target
    float z = zDefault;
    switch (_pet->mood()) {
    case Mood::SLEEPING:  z = zMin; break;
    case Mood::SCARED:    z = max(zMin, 0.3f); break;
    case Mood::STARTLED:  z = max(zMin, 0.3f); break;
    case Mood::SAD:       z = max(zMin, 0.5f); break;
    case Mood::PENSIVE:   z = max(zMin, 0.7f); break;
    case Mood::ANNOYED:   z = max(zMin, zDefault - 0.2f); break;
    case Mood::HAPPY:     z = min(zMax, zDefault + 0.2f); break;
    case Mood::LAUGHING:  z = min(zMax, 1.3f); break;
    case Mood::ANGRY:     z = min(zMax, 1.4f); break;
    case Mood::EXCITED:   z = min(zMax, 1.5f); break;
    default:              z = zDefault; break;
    }
    if (z < zMin) z = zMin;
    if (z > zMax) z = zMax;
    _zTarget = z;

    // Wake animation: Z pulse toward CLOSE on startup / waking from sleep
    if (_wakeMs > 0) {
        _wakeMs = (_wakeMs > deltaMs) ? _wakeMs - deltaMs : 0;
        float progress = (float)_wakeMs / WAKE_MS;  // 1.0→0.0
        if (progress > 0.5f) {
            float t = (progress - 0.5f) * 2.0f;  // 1.0→0.0
            _zTarget = zDefault + (zMax - zDefault) * (1.0f - t);
        }
        // Second half: normal mood logic settles Z back to default
    }

    // Smooth lerp toward target
    float diff = _zTarget - _zDepth;
    float step = zSpeed * (float)deltaMs;
    if (fabsf(diff) <= step) _zDepth = _zTarget;
    else                      _zDepth += (diff > 0.0f) ? step : -step;
}

void GotchiRenderer::beginTransition(uint16_t color) {
    _transitionColor = color;
    _transitionMs    = TRANSITION_MS;
}

void GotchiRenderer::triggerWake() {
    _wakeMs = WAKE_MS;
}

void GotchiRenderer::_applyTransitionOverlay(uint32_t deltaMs) {
    if (_transitionMs == 0) return;

    uint32_t elapsed  = TRANSITION_MS - _transitionMs;
    uint32_t halfSolid = (TRANSITION_MS - TRANSITION_SOLID_MS) / 2;  // ramp width

    uint8_t level;  // 0=none 1=25% 2=75% 3=solid
    if (elapsed < halfSolid) {
        if      (elapsed < halfSolid / 3)     level = 1;
        else if (elapsed < halfSolid * 2 / 3) level = 2;
        else                                   level = 3;
    } else if (_transitionMs > halfSolid) {
        level = 3;
    } else {
        if      (_transitionMs > halfSolid * 2 / 3) level = 3;
        else if (_transitionMs > halfSolid / 3)      level = 2;
        else                                         level = 1;
    }

    switch (level) {
    case 1:
        for (int y = 0; y < 135; y += 4)
            _canvas->drawFastHLine(0, y, 240, _transitionColor);
        break;
    case 2:
        for (int y = 0; y < 135; y += 2)
            _canvas->drawFastHLine(0, y, 240, _transitionColor);
        break;
    case 3:
        _canvas->fillScreen(_transitionColor);
        break;
    default: break;
    }

    _transitionMs = (_transitionMs > deltaMs) ? _transitionMs - deltaMs : 0;
}

// ── Speech bubble phrase banks ────────────────────────────────────────────────
static const char* const BYTEE_REACT[]   = { "!", "Oh!", "Interesting...", "Calculating...", "..." };
static const char* const BYTEE_ATTN[]    = { "Hey, are you there?", "I found something!", "Look at this!" };
static const char* const CTHULHU_REACT[] = { "eep!", "the sounds...", "*tentacle noises*", "hugs?" };
static const char* const CTHULHU_ATTN[]  = { "free hugs available", "...I see you", "ancient loneliness" };
static const char* const JACK_REACT[]    = { ".", "hmm" };
static const char* const JACK_ATTN[]     = { "..." };
static const char* const LUMI_REACT[]    = { "!!!", "oh oh oh!", "notice me!", "*sparkles*" };
static const char* const LUMI_ATTN[]     = { "HELLO??", "pay attention to meeee", "I'm right here!!!" };

void GotchiRenderer::showSpeech(const char* text, uint16_t durationMs) {
    strncpy(_speechText, text, sizeof(_speechText) - 1);
    _speechText[sizeof(_speechText) - 1] = '\0';
    int len = strlen(_speechText);
    _isMarquee   = (len * 6 > 88);
    _marqueeOff  = 0.0f;
    _speechRemMs = durationMs;
    _attentionIdleMs = 0;
}

void GotchiRenderer::triggerReaction() {
    if (!_pet) return;
    const char* const* bank;
    uint8_t n;
    switch (_pet->creature()) {
    case CreatureType::CTHULHU: bank = CTHULHU_REACT; n = 4; break;
    case CreatureType::JACK:    bank = JACK_REACT;    n = 2; break;
    case CreatureType::LUMI:    bank = LUMI_REACT;    n = 4; break;
    default:                    bank = BYTEE_REACT;   n = 5; break;
    }
    showSpeech(bank[millis() % n], 2500);
}

void GotchiRenderer::triggerAttention() {
    if (!_pet) return;
    const char* const* bank;
    uint8_t n;
    switch (_pet->creature()) {
    case CreatureType::CTHULHU: bank = CTHULHU_ATTN; n = 3; break;
    case CreatureType::JACK:    bank = JACK_ATTN;    n = 1; break;
    case CreatureType::LUMI:    bank = LUMI_ATTN;    n = 3; break;
    default:                    bank = BYTEE_ATTN;   n = 3; break;
    }
    const char* phrase = bank[millis() % n];
    bool willMarquee = ((int)strlen(phrase) * 6 > 88);
    showSpeech(phrase, willMarquee ? 7000 : 4000);
}

void GotchiRenderer::_drawSpeechBubble(uint32_t deltaMs) {
    if (!_pet || _pet->stage() == LifeStage::EGG) return;

    // Auto-attention idle accumulator
    if (_speechRemMs == 0 && !_pet->isSleeping()) {
        _attentionIdleMs += deltaMs;
        uint32_t thresh;
        switch (_pet->creature()) {
        case CreatureType::JACK:    thresh = 720000; break;
        case CreatureType::CTHULHU: thresh =  60000; break;
        case CreatureType::LUMI:    thresh =  15000; break;
        default:                    thresh =  30000; break;
        }
        if (_attentionIdleMs >= thresh) triggerAttention();
    }

    if (_speechRemMs == 0) return;
    _speechRemMs = (_speechRemMs > deltaMs) ? _speechRemMs - deltaMs : 0;

    // Bubble geometry
    int textLen  = strlen(_speechText);
    int textPixW = textLen * 6;
    int bw = _isMarquee ? 96 : max(textPixW + 10, 28);
    int bh = 16;
    int botY = max(bh + 8, min(_lastSpriteTopY - 2, 110));
    int topY = botY - bh;
    int bx   = max(2, min((int)_posX - bw / 2, 238 - bw));

    // Per-creature colors
    CreatureType ct = _pet->creature();
    uint16_t fillC, bordC, textC;
    switch (ct) {
    case CreatureType::CTHULHU: fillC=0x0841; bordC=0x07E0; textC=0xB7F5; break;
    case CreatureType::JACK:    fillC=0x2104; bordC=0x7BEF; textC=0xC618; break;
    case CreatureType::LUMI:    fillC=0x100C; bordC=0xF81F; textC=0xFFFF; break;
    default: /* Bytee */        fillC=0x0808; bordC=0xFFE0; textC=0xFFFF; break;
    }

    // Draw bubble background + border (per creature style)
    switch (ct) {
    case CreatureType::JACK:
        _canvas->fillRect(bx, topY, bw, bh, fillC);
        _canvas->drawRect(bx, topY, bw, bh, bordC);
        break;
    case CreatureType::LUMI: {
        _canvas->fillRoundRect(bx, topY, bw, bh, 4, fillC);
        _canvas->drawRoundRect(bx, topY, bw, bh, 4, bordC);
        uint8_t t = (uint8_t)(millis() / 250);
        _canvas->drawPixel(bx + bw - 5, topY + 2, (t % 2) ? bordC : 0xFFFF);
        _canvas->drawPixel(bx + bw - 3, topY + 4, (t % 2) ? 0xFFFF : bordC);
        break;
    }
    case CreatureType::CTHULHU: {
        _canvas->fillRoundRect(bx, topY, bw, bh, 3, fillC);
        _canvas->drawRoundRect(bx, topY, bw, bh, 3, bordC);
        uint8_t t = (uint8_t)(millis() / 400);
        _canvas->drawPixel(bx - 1, topY + 4 + (t % 4), bordC);
        _canvas->drawPixel(bx + bw, topY + 6 + (t % 3), bordC);
        break;
    }
    default: // Bytee: rect + gold corner dots
        _canvas->fillRect(bx, topY, bw, bh, fillC);
        _canvas->drawRect(bx, topY, bw, bh, bordC);
        _canvas->drawPixel(bx - 1,  topY - 1,  bordC);
        _canvas->drawPixel(bx + bw, topY - 1,  bordC);
        _canvas->drawPixel(bx - 1,  topY + bh, bordC);
        _canvas->drawPixel(bx + bw, topY + bh, bordC);
        break;
    }

    // Tail pointing from bubble bottom toward creature center
    int tailX = max(bx + 4, min((int)_posX - 2, bx + bw - 8));
    switch (ct) {
    case CreatureType::JACK:
        _canvas->drawFastVLine(tailX, botY, 5, bordC);
        _canvas->drawFastHLine(tailX, botY + 5, 4, bordC);
        break;
    case CreatureType::LUMI:
        _canvas->drawFastVLine(tailX + 1, botY, 4, bordC);
        _canvas->drawPixel(tailX + 2, botY + 3, bordC);
        break;
    case CreatureType::CTHULHU:
        _canvas->drawPixel(tailX,     botY,     bordC);
        _canvas->drawPixel(tailX + 1, botY + 1, bordC);
        _canvas->drawPixel(tailX,     botY + 2, bordC);
        _canvas->drawPixel(tailX + 1, botY + 3, bordC);
        break;
    default: // Bytee: crystal diamond point
        _canvas->drawPixel(tailX + 1, botY,     bordC);
        _canvas->drawPixel(tailX,     botY + 1, bordC);
        _canvas->drawPixel(tailX + 2, botY + 1, bordC);
        _canvas->drawPixel(tailX + 1, botY + 2, bordC);
        break;
    }

    // Text
    _canvas->setTextFont(1);
    _canvas->setTextSize(1);
    _canvas->setTextColor(textC, fillC);
    int textAreaX = bx + 4;
    int textAreaW = bw - 8;
    int textY     = topY + (bh - 8) / 2;

    if (!_isMarquee) {
        _canvas->setCursor(textAreaX, textY);
        _canvas->print(_speechText);
    } else {
        _marqueeOff -= 0.04f * (float)deltaMs;
        if (_marqueeOff < -(float)textPixW) _marqueeOff = (float)textAreaW;
        int curX = textAreaX + (int)_marqueeOff;
        for (const char* p = _speechText; *p; ++p) {
            if (curX >= textAreaX && curX + 6 <= textAreaX + textAreaW) {
                _canvas->setCursor(curX, textY);
                _canvas->write(*p);
            }
            curX += 6;
        }
    }
}

void GotchiRenderer::_drawMoodPeek() {
    static const char* MOOD_NAMES[] = {
        "Neutral",    "Contento",  "Mal",      "Pensativo",
        "Triste",     "Dormido",   "Emocionado","Risas",
        "Mareado",    "Molesto",   "Enfadado",  "Sobresaltado", "Asustado"
    };

    // Slide-in/out animation
    static constexpr int     PANEL_H   = 35;
    static constexpr int     PANEL_Y0  = 100;
    static constexpr uint32_t SLIDE_MS = 150;
    int slideY = 0;
    if (_moodPeekInMs < SLIDE_MS) {
        slideY = (int)(PANEL_H * (1.0f - (float)_moodPeekInMs / SLIDE_MS));
    } else if (_moodPeekMs < SLIDE_MS) {
        slideY = (int)(PANEL_H * (1.0f - (float)_moodPeekMs / SLIDE_MS));
    }
    int py = PANEL_Y0 + slideY;

    _canvas->fillRect(0, py, 240, PANEL_H, 0x0000);
    _canvas->drawFastHLine(0, py, 240, 0x4208);

    CreatureType ct = _pet->creature();
    const char* ctName = (ct == CreatureType::BYTEE)   ? "Bytee"   :
                         (ct == CreatureType::CTHULHU) ? "Cthulhu" :
                         (ct == CreatureType::JACK)    ? "Jack"    : "Lumi";

    uint8_t moodIdx = (uint8_t)_pet->mood();
    const char* moodName = (moodIdx < 13) ? MOOD_NAMES[moodIdx] : "...";

    _canvas->setTextFont(1);
    _canvas->setTextColor(0x8410, 0x0000);
    _canvas->setCursor(6, py + 4);
    _canvas->print(ctName);

    _canvas->setTextColor(0xFFFF, 0x0000);
    _canvas->setTextSize(2);
    _canvas->setCursor(6, py + 15);
    _canvas->print(moodName);
    _canvas->setTextSize(1);

    // Emote sprite on the right
    SpritePalette pal;
    pal.transparent = 0x0001;
    pal.primary   = 0x2104;
    pal.secondary = 0x001F;
    pal.dark      = 0x8410;
    pal.accent    = 0xFFE0;
    pal.color5    = 0x0000;
    _drawSprite(gotchiEmoteSprite(_pet->mood()), EMOTE_W, EMOTE_H,
                200, py + 6, EMOTE_SCALE, pal);
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
            case 5: color = pal.color5; break;
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
