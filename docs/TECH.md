# Arquitectura Técnica — Stick-Gotchi v2

> Versión 1.0 | 2026-04-22

## Principios de diseño

- **Sin heap dinámico**: todo con instancias estáticas o buffers fijos.
- **FreeRTOS mínimo**: solo el task de render en Core 0. La lógica del gotchi corre en Core 1 (loop principal).
- **NVS como única persistencia**: sin filesystem, sin SD.
- **Sin dependencias externas nuevas**: M5Unified + NimBLE-Arduino, ya presentes.

---

## Módulos nuevos / modificados

```
src/
├── core/                      (sin cambios estructurales)
│   ├── AppBase.h
│   ├── AppManager.cpp/.h
│   ├── ButtonManager.cpp/.h
│   ├── DisplayManager.cpp/.h
│   └── InputEvent.h
│
├── apps/
│   ├── gotchi/
│   │   ├── GotchiApp.cpp/.h          (modificado — UI con iconos)
│   │   ├── GotchiPet.cpp/.h          (REESCRITO — stats, muerte, herencia)
│   │   ├── GotchiRenderer.cpp/.h     (REESCRITO — pixel art, semilla)
│   │   ├── GotchiLineage.cpp/.h      (NUEVO — árbol genealógico)
│   │   └── GotchiSprites.h           (NUEVO — datos de sprites por etapa)
│   │
│   ├── stats/
│   │   ├── StatsApp.cpp/.h           (NUEVO — pantalla de estadísticas)
│   │   └── StatsRenderer.cpp/.h      (NUEVO — render de barras, linaje)
│   │
│   └── imudemo/                      (sin cambios)
│       ├── ImuDemoApp.cpp/.h
│
├── gotchi/
│   ├── GotchiDNA.cpp/.h              (NUEVO — sistema de semilla/ID)
│   ├── GotchiSleep.cpp/.h            (NUEVO — lógica de sueño/día-noche)
│   ├── GotchiAudio.cpp/.h            (NUEVO — sampling de micrófono)
│   └── GotchiShake.cpp/.h            (NUEVO — detección de sacudidas IMU)
│
└── main.cpp                          (modificado — añadir StatsApp)
```

---

## GotchiDNA — Sistema de Identidad

### GotchiID (64 bits)

```cpp
struct GotchiID {
    uint8_t  generation;      // 0–255
    uint8_t  parent_id[3];    // 24 bits del ID del padre (truncado)
    uint16_t visual_seed;     // semilla visual determinista
    uint16_t birth_epoch;     // timestamp truncado (días desde epoch fijo)
};
```

### GotchiSeed — decodificación

```cpp
struct GotchiVisual {
    uint8_t hue_primary;    // 0–31  → hue = hue_primary * 11.6°
    uint8_t hue_secondary;  // 0–31
    uint8_t eye_type;       // 0–3
    uint8_t mark_type;      // 0–3
    uint8_t body_shape;     // 0–3
};

GotchiVisual decodeVisual(uint16_t seed) {
    return {
        .hue_primary   = (seed >> 11) & 0x1F,
        .hue_secondary = (seed >> 6)  & 0x1F,
        .eye_type      = (seed >> 4)  & 0x03,
        .mark_type     = (seed >> 2)  & 0x03,
        .body_shape    = seed         & 0x03,
    };
}
```

### Mutación al heredar

```cpp
uint16_t mutateSeed(uint16_t parent_seed, uint32_t rng) {
    // Mutar hue_primary ±1, hue_secondary ±1, marks aleatorio 10% probabilidad
    uint8_t hp = (parent_seed >> 11) & 0x1F;
    uint8_t hs = (parent_seed >> 6)  & 0x1F;
    hp = (hp + (rng & 1 ? 1 : -1)) & 0x1F;
    hs = (hs + ((rng >> 1) & 1 ? 1 : -1)) & 0x1F;
    uint8_t marks = ((rng >> 8) % 10 == 0)
        ? ((parent_seed >> 2) & 0x03 + 1) % 4
        : (parent_seed >> 2) & 0x03;
    return (hp << 11) | (hs << 6) | ((parent_seed >> 4) & 0x03) << 4
         | (marks << 2) | (parent_seed & 0x03);
}
```

---

## GotchiPet — Modelo de datos

```cpp
enum class LifeStage : uint8_t {
    EGG = 0, BABY, YOUNG, ADULT
};

enum class AdultForm : uint8_t {
    HEALTHY = 0, NORMAL, NEGLECTED
};

struct GotchiStats {
    float hunger;    // 0–100
    float energy;    // 0–100
    float mood;      // 0–100
    float health;    // 0–100
    uint16_t age_days;
    uint32_t alive_seconds;

    // Acumuladores para herencia
    float avg_mood;
    float avg_health;
    uint32_t feed_count;
    uint32_t play_count;
    uint32_t hard_shake_count;
};

struct GotchiState {
    GotchiID    id;
    GotchiStats stats;
    LifeStage   stage;
    AdultForm   form;
    GotchiMood  mood_enum;
    bool        is_sleeping;
    bool        is_alive;
    uint32_t    low_health_timer_ms;  // para muerte por descuido
};
```

### Tasas de degradación (por segundo)

```cpp
struct DecayRates {
    float hunger;   // pts/seg
    float energy;
    float mood;
};

constexpr DecayRates kDecay[4] = {
    //  hunger     energy    mood
    { 1.0/600,  1.0/900,  1.0/1200 },  // EGG (mínimo)
    { 1.0/600,  1.0/900,  1.0/1200 },  // BABY
    { 1.0/480,  1.0/720,  1.0/900  },  // YOUNG
    { 1.0/360,  1.0/600,  1.0/720  },  // ADULT
};
```

---

## GotchiLineage — Persistencia NVS

### Esquema NVS

Namespace: `gotchi`

| Key | Tipo | Contenido |
|---|---|---|
| `state` | blob | GotchiState completo |
| `anc_0` | blob | GotchiAncestor del padre |
| `anc_1` | blob | GotchiAncestor del abuelo |
| `anc_2` | blob | GotchiAncestor del bisabuelo |
| `anc_3` | blob | 4° generación |
| `anc_4` | blob | 5° generación |
| `heritage` | blob | GotchiHeritage (bonuses acumulados) |

```cpp
struct GotchiAncestor {
    GotchiID id;
    uint16_t days_lived;
    uint8_t  cause_of_death;   // 0=hambre,1=salud,2=otro
    uint8_t  avg_mood_pct;
    uint8_t  avg_health_pct;
};

struct GotchiHeritage {
    float    bonus_mood;      // 0–10 pts adicionales base
    float    bonus_health;    // 0–10 pts adicionales base
    float    hatch_speedup;   // 0.0–0.2 (0–20% más rápido)
    bool     beauty_bonus;    // forma adulta mejorada
};
```

---

## GotchiAudio — Micrófono

Task en Core 0, baja prioridad, muestrea cada 100ms.

```cpp
struct AudioState {
    float    rms_db;          // nivel actual en dB
    uint32_t loud_start_ms;   // cuándo empezó el ruido alto
    bool     is_noisy;        // > 70 dB activo
    bool     sustained_loud;  // > 85 dB por > 2s
};
```

El nivel dB se pasa como evento al GotchiPet a través de una cola FreeRTOS (QueueHandle_t, capacidad 4). Sin polling directo desde el modelo.

---

## GotchiShake — IMU

Leído en el loop de update de GotchiApp. Sin task extra — el IMU es rápido.

```cpp
enum class ShakeLevel : uint8_t {
    NONE = 0, SOFT, MEDIUM, HARD, VIOLENT
};

ShakeLevel detectShake(float ax, float ay, float az) {
    float delta = fabsf(ax) + fabsf(ay) + fabsf(az) - 1.0f; // quitar gravedad
    if (delta < 0.3f) return ShakeLevel::NONE;
    if (delta < 0.5f) return ShakeLevel::SOFT;
    if (delta < 1.5f) return ShakeLevel::MEDIUM;
    if (delta < 2.5f) return ShakeLevel::HARD;
    return ShakeLevel::VIOLENT;
}
```

---

## GotchiRenderer — Pixel Art

### Pipeline de render

```
GotchiState + GotchiVisual
        ↓
  seleccionar sprite base (etapa + forma adulta)
        ↓
  aplicar paleta (hue_primary → color body, hue_secondary → color belly)
        ↓
  superponer ojos (tipo_ojos, frame animación)
        ↓
  superponer marcas corporales
        ↓
  dibujar emote/mood (overlay, ya implementado)
        ↓
  M5Canvas::pushImage() escalado 2× o 3×
```

### Formato de sprite

```cpp
// Sprite 16×16, índices de paleta (0=transparente, 1=primario, 2=secundario, 3=oscuro, 4=blanco)
using SpriteData = uint8_t[16][16];

// Paleta calculada desde seed
struct SpritePalette {
    uint16_t transparent;  // 0x0000 (color key)
    uint16_t primary;      // RGB565 desde hue_primary
    uint16_t secondary;    // RGB565 desde hue_secondary
    uint16_t dark;         // primary desaturado/oscurecido
    uint16_t white;        // 0xFFFF
};
```

### Conversión HSV → RGB565

```cpp
uint16_t hsvToRgb565(uint8_t hue32, uint8_t sat, uint8_t val) {
    float h = hue32 * 360.0f / 32.0f;
    // HSV → RGB → RGB565
    // ... conversión estándar
}
```

---

## StatsApp — Pantalla de estadísticas

App independiente. No tiene task propio — dibuja on-demand con DisplayManager.

```cpp
class StatsApp : public AppBase {
    uint8_t current_tab = 0;   // 0=estado, 1=linaje, 2=histórico

    void drawTabStatus();
    void drawTabLineage();
    void drawTabHistory();

    bool onInput(InputEvent e) override {
        if (e.button == Button::B && e.type == Press::SHORT)
            current_tab = (current_tab + 1) % 3;
        if (e.button == Button::A && e.type == Press::LONG)
            appManager->switchTo(nullptr);  // volver
        return true;
    }
};
```

---

## UI Layout — GotchiApp

```
Y=0   ┌─────────────────────────────────────────────────┐
      │ ♥98  🍖67  ⚡89  😊45             22:34  G3   │  18px
Y=18  ├─────────────────────────────────────────────────┤
      │                                                 │
      │               [ GOTCHI SPRITE ]                 │  90px
      │                                                 │
Y=108 ├─────────────────────────────────────────────────┤
      │  [ 🍖 ]  [ 🎮 ]  [ 💊 ]  [ 💡 ]  [ 🛁 ]      │  27px
Y=135 └─────────────────────────────────────────────────┘
      0                                               240px
```

La barra de stats usa fuente pequeña (6×8 o 7×7). Los iconos de acción son sprites 16×16 con fondo negro, borde blanco en el activo.

---

## Fases de implementación

### Fase MVP (primera iteración con Haiku)

1. **GotchiDNA**: struct GotchiID, generación de seed, mutación al heredar.
2. **GotchiPet reescrito**: 5 stats, tasas de degradación, lógica de muerte (timer 15min), detección de agonía.
3. **Sueño básico**: horario 22:00–07:00, sleep por energía baja, protección nocturna.
4. **GotchiShake**: integrado en GotchiApp.update(), eventos al GotchiPet.
5. **UI con iconos**: barra de acciones, navegación Btn B / confirmar Btn A.
6. **NVS persistencia**: guardar/cargar GotchiState + 5 ancestros.
7. **Herencia al morir**: calcular heritage, mutar seed, crear nuevo huevo.

### Fase V1

8. **GotchiAudio**: task de micrófono, eventos de ruido al GotchiPet.
9. **GotchiSprites**: sprites pixel art reales por etapa.
10. **GotchiRenderer reescrito**: pipeline completo con paleta desde seed.
11. **Animaciones**: idle, comer, jugar, dormir, morir.

### Fase V2

12. **StatsApp**: las 3 pestañas completas.
13. **StatsRenderer**: barras de stats, árbol genealógico, histórico.
14. **Ramas evolutivas**: detección de forma adulta según cuidado acumulado.

---

## Consideraciones de memoria

| Recurso | Uso estimado |
|---|---|
| GotchiState en RAM | ~80 bytes |
| 5 ancestros en RAM | ~100 bytes |
| Sprites (4 etapas × 2 frames) | ~2KB (índices uint8) |
| Buffer M5Canvas (240×135×2) | ~65KB (ya asignado) |
| NVS (estado + ancestros) | < 1KB |

El ESP32 tiene 520KB SRAM — uso muy por debajo del límite.

---

## Invariantes a mantener

- `DisplayManager::acquire/release` siempre en par. Nunca dibujar sin el mutex.
- `GotchiRenderer` task solo en Core 0. Toda la lógica de GotchiPet en Core 1.
- NVS se escribe solo en eventos discretos (muerte, cada 5 minutos de actividad) — nunca en cada tick.
- La semilla visual (`visual_seed`) es inmutable durante la vida del gotchi. Solo cambia en herencia.
