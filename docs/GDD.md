# Game Design Document — Stick-Gotchi

> Versión 3.0 | 2026-07-02 — Rework: Carousel Launcher

## Visión

Un gadget de escritorio minimalista para el M5Stick C Plus2. No hay mascota, no hay ciclo de vida: el dispositivo es un **launcher de mini-apps** navegado con un carrusel de tiles. Enciende, se ve un carrusel, seleccionas una tile con dos botones, usas la app, vuelves al carrusel. Sin gestión, sin stats, sin cuidado — solo herramientas y juguetes rápidos.

Esto reemplaza tanto el Tamagotchi original (v1) como el pivot de "compañero reactivo" (v2, mayo 2026) — ambos descartados por completo.

---

## Layout físico de botones

```
        ┌──────────────┐   ← Btn B (arriba)
        │              │
        │   PANTALLA   │◄──── Btn A (lateral, arriba)
        │  135 x 240   │
        │              │◄──── Btn C / Power (lateral, abajo)
        └──────────────┘
```

## Mapeo de botones

| Botón | Gesto | Acción |
|---|---|---|
| Btn B | Corto | Avanzar / navegar en el carrusel |
| Btn A | Corto | Seleccionar / confirmar (input positivo) — lanza la tile centrada |
| Btn B | Largo | Cancelar / volver al carrusel (home) |
| Btn A | Largo | Sin función — reservado a futuro |
| Btn C (power) | Corto | Reiniciar el dispositivo |
| Btn C (power) | Largo (700ms) | Apagar el dispositivo |

Btn C **nunca** tiene función de UI/menú — solo control de energía.

---

## Pantalla Home — Carousel

Carrusel horizontal circular de tiles (icono + texto). Se muestran 3 tiles a la vez: la tile central siempre está seleccionada/resaltada, las laterales son un adelanto de lo que hay a cada lado. Al pulsar B, el carrusel se desplaza una posición; al llegar al final vuelve a empezar (cíclico en ambas direcciones).

Solo hay **un nivel de menú**: seleccionar una tile lanza su subprograma directamente, sin submenús. La posición en el carrusel se mantiene mientras el dispositivo está encendido (no se resetea al volver de una app), pero no se persiste entre reinicios.

### Tiles actuales

| Tile | Descripción |
|---|---|
| Coin Flip | Lanza una moneda animada (cara/cruz) |
| Magic 8-Ball | Agita el dispositivo (IMU) para consultar la bola 8 |
| Accelerometer | Demo de acelerómetro — bola en una caja |
| Gyroscope | Demo de giroscopio — radar de 3 ejes |
| Orientation | Demo de orientación — horizonte artificial |

Cada mini-app usa Btn A para su interacción propia (lanzar moneda, reiniciar consulta, etc.) y Btn B largo para volver al carrusel.

---

## Roadmap

| Fase | Contenido |
|---|---|
| Rework v3 | Carousel Launcher, botones redefinidos, Coin Flip + Magic 8-Ball + demos IMU migrados |
| Siguiente | Iconos de tile en pixel art (Aseprite) en vez de iconos vectoriales placeholder |
| Futuro | Nuevas mini-apps se añaden como tiles adicionales del mismo carrusel |
