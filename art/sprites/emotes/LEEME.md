# Sprites de Emotes

Las expresiones faciales que flotan sobre el gotchi cuando cambia de mood.

## Paleta — importar antes de dibujar

Edit > Palettes > Load Palette → `art/_palette/emote.gpl`
Image > Color Mode → **Indexed**

| Índice | Color | Rol |
|--------|-------|-----|
| 0 | Magenta `#FF00FF` | Transparente |
| 1 | Blanco `#FFFFFF` | Área principal / ojos blancos |
| 2 | Negro `#000000` | Pupils, líneas, contorno |
| 3 | Rojo `#FF4040` | Mejillas, rubor, énfasis |

Paleta fija — los emotes tienen sus propios colores, no usan el DNA del gotchi.

---

## Archivo

Un solo archivo: `emotes.aseprite`
Canvas: **16 × 12 px**
Cada frame = un emote diferente. Usar tags para nombrarlos.

## Tags — nombres exactos y orden

| Tag | Frame | Expresión |
|-----|-------|-----------|
| `neutral` | 0 | Cara plana, ojos medios |
| `happy` | 1 | Ojos semicerrados felices, curva de sonrisa |
| `sick` | 2 | Ojos x, tono verdoso si puedes con el rojo |
| `pensive` | 3 | Un ojo entrecerrado, mira al costado |
| `sad` | 4 | Ojos hacia abajo, boca caída |
| `sleeping` | 5 | Ojos cerrados (líneas), ZZZ |
| `excited` | 6 | Ojos abiertos grandes, boca abierta |
| `laughing` | 7 | Ojos cerrados de risa, boca abierta con dientes |
| `dizzy` | 8 | Ojos espiral o X girados |
| `annoyed` | 9 | Ceja fruncida, boca tensa |
| `angry` | 10 | Cejas muy juntas hacia abajo, boca apretada |
| `startled` | 11 | Ojos muy abiertos, cejas arriba |
| `scared` | 12 | Ojos llorosos, boca temblorosa |

**Total: 13 frames**

---

## Export

File > Export Sprite Sheet → Horizontal Strip → ✅ JSON
Destino: `art/export/emotes/emotes.png`
