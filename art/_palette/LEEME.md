# Paletas — cómo importar en Aseprite

## Archivos

| Archivo | Usar para |
|---------|-----------|
| `gotchi_sprite.gpl` | Sprites de gotchi (egg, baby, young, adult — las 3 ramas) |
| `emote.gpl` | Sprites de emotes (expresiones faciales) |

## Cómo importar

1. Abrir Aseprite con el archivo nuevo creado
2. Menú: **Edit > Palettes > Load Palette...**
3. Seleccionar el archivo `.gpl` correspondiente
4. Luego: **Image > Color Mode > Indexed**
5. En el diálogo que aparece: seleccionar "Use current palette"

La paleta queda fija con exactamente los colores del archivo.

## Por qué estos colores exactos

Los colores en Aseprite son solo placeholders — el renderer en C++ los reemplaza
en tiempo real por los colores reales del GotchiDNA. El script `tools/spr2c.py`
detecta a qué índice pertenece cada pixel por posición en la paleta, no por el
valor RGB. Por eso los colores de referencia pueden ser cualquier cosa visible,
pero deben ser consistentes.

## Verificación rápida

Abrir cualquier sprite terminado → Image > Color Mode → Indexed.
Si aparece "This will reduce the palette to N colors" con N > 5 (gotchi) o N > 4
(emote), hay pixels pintados con colores fuera de paleta — revisar y corregir
antes de exportar.
