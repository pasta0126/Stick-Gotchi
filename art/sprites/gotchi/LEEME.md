# Sprites de Gotchi

## Paleta — importar antes de dibujar

1. Abrir Aseprite
2. Edit > Palettes > Load Palette → `art/_palette/gotchi_sprite.gpl`
3. Image > Color Mode → **Indexed**
4. La paleta queda con exactamente 5 colores fijos:

| Índice | Color en Aseprite | Rol en runtime |
|--------|-------------------|----------------|
| 0 | Magenta `#FF00FF` | Transparente |
| 1 | Azul `#0000FF` | Color primario del gotchi (lo pone el DNA) |
| 2 | Cyan `#00FFFF` | Color secundario / highlight |
| 3 | Gris `#404040` | Sombra / outline / detalles oscuros |
| 4 | Amarillo `#FFFF00` | Acento / detalle especial |

**No dibujar con ningún otro color.** El sistema reemplaza estos 5 colores en
tiempo real según la semilla del GotchiDNA — el artista define la forma, el
código pone los colores reales.

---

## Archivos a crear (uno por etapa por rama)

Cada archivo = un canvas fijo. Las animaciones van como **tags**.

| Archivo | Canvas | Escala × | Píxeles en pantalla |
|---------|--------|----------|---------------------|
| `egg.aseprite` | 12 × 14 px | 3× | 36 × 42 |
| `blob_baby.aseprite` | 16 × 16 px | 3× | 48 × 48 |
| `blob_young.aseprite` | 20 × 22 px | 2× | 40 × 44 |
| `blob_adult.aseprite` | 24 × 26 px | 2× | 48 × 52 |
| `plant_baby.aseprite` | 16 × 16 px | 3× | 48 × 48 |
| `plant_young.aseprite` | 20 × 22 px | 2× | 40 × 44 |
| `plant_adult.aseprite` | 24 × 26 px | 2× | 48 × 52 |
| `libre_baby.aseprite` | 16 × 16 px | 3× | 48 × 48 |
| `libre_young.aseprite` | 20 × 22 px | 2× | 40 × 44 |
| `libre_adult.aseprite` | 24 × 26 px | 2× | 48 × 52 |

---

## Tags de animación — nombres exactos

Crear estas tags en Aseprite (Frame Tags panel). Los nombres deben ser exactamente estos:

### egg.aseprite
| Tag | Frames | Descripción |
|-----|--------|-------------|
| `idle` | 2 | Huevo quieto, pequeña vibración |
| `hatch` | 4 | Secuencia de eclosión |

### blob_baby / plant_baby / libre_baby
| Tag | Frames | Descripción |
|-----|--------|-------------|
| `idle` | 2 | Respiración suave |
| `eat` | 3 | Animación de comer |
| `play` | 4 | Salto / movimiento alegre |
| `sleep` | 2 | Dormido (ojos cerrados, sube y baja) |
| `die` | 4 | Agonía → desmayo |

### blob_young / plant_young / libre_young  (mismas tags)
### blob_adult / plant_adult / libre_adult  (mismas tags)

---

## Personalidad visual de cada rama

### BLOB
Forma: redondeada, gelatinosa. Tiene pequeñas antenas o protuberancias.
Arms (limbs) stub-like. Cuerpo principal tipo blob/slime.

### PLANT
Forma: tiene elementos vegetales — hojas pequeñas, brotes, textura orgánica.
No es un cactus ni una flor exacta, es un ser vivo con partes vegetales integradas.

### LIBRE (fuego fatuo / hada)
Forma: más etérea, simétrica, con "alas" o extensiones de luz en los laterales.
Menos peso visual, más flotante. Las extensiones laterales se animan.

---

## Export

File > Export Sprite Sheet
- Sheet type: Horizontal Strip
- ✅ Export JSON (formato Hash)
- Destino: `art/export/gotchi/<nombre_archivo>.png`
