# Iconos de UI

## Action Bar — 5 iconos

Archivo: `icons_actions.aseprite`
Canvas: **16 × 16 px** por icono
Color mode: **RGB** (full color, no paleta indexada)
Frames: 5 frames, uno por icono

| Frame | Tag | Icono | Acción |
|-------|-----|-------|--------|
| 0 | `feed` | 🍖 hueso / comida | Alimentar |
| 1 | `play` | 🎮 pelota / estrella | Jugar |
| 2 | `medicine` | 💊 pastilla / cruz | Medicina |
| 3 | `light` | 💡 lámpara / sol | Toggle luz |
| 4 | `clean` | 🛁 burbuja / escoba | Limpiar |

No usar emoji literales — dibujar versiones pixel art de estos conceptos.
Estilo: outline claro, reconocible a 16×16. Fondo transparente (magenta `#FF00FF`
o usar alpha si el archivo es RGB).

---

## Colores sugeridos para iconos

Sin restricción de paleta, pero buscar coherencia:
- Outlines: negro o gris oscuro
- Rellenos: un color principal por icono (naranja para comida, verde para jugar, etc.)
- Fondo: transparente

El icono seleccionado en la action bar se resalta con un borde blanco en código —
no necesitas dibujar el estado seleccionado.

---

## Export

File > Export Sprite Sheet → Horizontal Strip → ✅ JSON
Destino: `art/export/ui/icons_actions.png`
