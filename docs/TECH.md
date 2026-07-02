# Arquitectura Técnica — Stick-Gotchi v3 (Carousel Launcher)

> Versión 3.0 | 2026-07-02

## Principios de diseño

- **Sin heap dinámico para apps**: instancias estáticas en `main.cpp`, inyectadas por puntero.
- **CarouselHome es la raíz**: no hay overlay de menú separado. `AppManager` siempre tiene una app activa; al arrancar, esa app es `CarouselHome`.
- **Btn C fuera de `ButtonManager`**: es control de energía (reboot/power off), gestionado directo en `main.cpp` sobre `M5.BtnPWR`. `ButtonManager` solo conoce A y B.
- **Sin dependencias externas más allá de M5Unified**: NimBLE-Arduino y M5Stack-Avatar se retiraron por no usarse (BLE era protocolo de moods del gotchi, ya eliminado).

---

## Estructura de módulos

```
src/
├── core/                       (sin cambios estructurales)
│   ├── AppBase.h                — setHomeCallback() en vez de setMenuCallback()
│   ├── AppManager.cpp/.h
│   ├── ButtonManager.cpp/.h     — solo A/B; C se maneja en main.cpp
│   ├── DisplayManager.cpp/.h
│   └── InputEvent.h             — comentarios actualizados con la nueva semántica
│
├── render/
│   └── SpriteBlit.h             — SpritePalette + drawPaletteSprite(), compartido
│
├── home/
│   ├── CarouselHome.cpp/.h      — launcher raíz: carrusel cíclico de tiles
│   └── TileIcons.h              — iconos vectoriales placeholder por tile
│
├── apps/
│   ├── coinflip/
│   │   ├── CoinFlipApp.cpp/.h   — AppBase wrapper + dibujo
│   │   └── FlipCoinGame.cpp/.h  — máquina de estados del juego (sin cambios de lógica)
│   ├── magic8ball/
│   │   ├── Magic8BallApp.cpp/.h — AppBase wrapper + polling IMU + dibujo
│   │   └── Magic8BallGame.cpp/.h
│   └── imudemo/                 (sin cambios)
│       └── ImuDemoApp.cpp/.h    — Accelerometer / Gyroscope / Orientation
│
├── generated/
│   ├── sprites_coin.h           (compartido, sin cambios)
│   └── sprites_8magicball.h     (compartido, sin cambios)
│
└── main.cpp                     — reescrito: registra tiles, arranca en CarouselHome
```

---

## CarouselHome

```cpp
struct CarouselTile {
    const char* name;
    std::function<void(M5Canvas&, int, int, int, uint32_t)> iconFn;
    std::function<AppBase*()> launch;
};

class CarouselHome : public AppBase {
    void addTile(const CarouselTile&);
    bool onInput(const InputEvent& e) override;
    // B SHORT  → avanza _index (módulo n)
    // A SHORT  → _apps->launchApp(_tiles[_index].launch())
    // B LONG   → no-op (no hay nivel superior a Home)
};
```

`_index` es un miembro de la instancia estática — sobrevive a `launchApp()` hacia otra app y de vuelta (porque `CarouselHome` nunca se destruye, solo se suspende/relanza), pero no se persiste en NVS: tras un reinicio siempre empieza en la tile 0.

Dibuja 3 tiles por frame (offsets -1, 0, +1 respecto a `_index`, con módulo circular), la central con borde resaltado y label; solo redibuja cuando hay un cambio (`_dirty`), no en cada tick.

---

## Botón de energía (Btn C / `M5.BtnPWR`)

`main.cpp` implementa su propia detección de pulsación corta/larga sobre `M5.BtnPWR` (el mismo patrón que `ButtonManager` mantiene para A/B, pero standalone):

```cpp
// corto  → ESP.restart()
// largo (700ms) → M5.Power.powerOff()
```

Nunca se enruta a `AppManager` ni a ninguna app — es puramente control de energía.

---

## Apps de minijuego

`CoinFlipApp` y `Magic8BallApp` envuelven la lógica de juego existente (`FlipCoinGame`, `Magic8BallGame`, sin cambios) en un `AppBase` independiente. Cada una:

- Dibuja directamente sobre `DisplayManager::canvas()` (sin task de FreeRTOS propio — el frame rate de ~30fps se logra acumulando `deltaMs` en `update()`, igual que `ImuDemoApp`).
- Usa `drawPaletteSprite()` de `render/SpriteBlit.h` para blitear los sprites indexados (`sprites_coin.h` / `sprites_8magicball.h`).
- Llama a `_homeCallback()` en Btn B largo para volver a `CarouselHome`.

---

## Invariantes a mantener

- `DisplayManager::acquire/release` siempre en par. Nunca dibujar sin el mutex.
- `ButtonManager` solo procesa A y B. C vive fuera, en `main.cpp`, y solo controla energía.
- `CarouselHome` es la única app sin `_homeCallback` (no hay nivel por encima de Home).
- Cualquier app nueva se añade como una tile más del mismo carrusel — no se crean submenús ni jerarquías adicionales.
