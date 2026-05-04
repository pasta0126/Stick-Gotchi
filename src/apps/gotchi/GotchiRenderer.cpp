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

        _drawHabitat(type);
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

        if (_pet->stage() != LifeStage::EGG) {
            _drawIndicators();
        }

        if (_pet->isAgony()) {
            _drawAgonyOverlay();
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
