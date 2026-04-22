# Pipeline de arte — Stick-Gotchi

## Pantalla
```
240 × 135 px  (landscape)

y 0–17    → stats bar (18px)
y 22–111  → play zone (90px) — donde vive el gotchi
y 110–134 → action bar (25px)
```

## Flujo de trabajo

```
Aseprite (.aseprite)
  → File > Export Sprite Sheet → PNG + JSON
  → guardar en art/export/<categoría>/
  → python tools/spr2c.py        (yo corro esto)
  → src/generated/<nombre>.h     (C array listo para compilar)
```

## Estructura de carpetas

```
art/
├── _palette/          ← paletas .gpl para importar en Aseprite
├── sprites/
│   ├── gotchi/        ← archivos .aseprite de los gotchis
│   ├── emotes/        ← expresiones faciales
│   ├── backgrounds/   ← fondos de escena
│   └── ui/            ← iconos de la barra de acciones
└── export/            ← PNGs + JSONs exportados desde Aseprite
    ├── gotchi/
    ├── emotes/
    ├── backgrounds/
    └── ui/
```

## Regla de export (gotchi + emotes + ui)

En Aseprite: **File > Export Sprite Sheet**
- Sheet type: `Horizontal Strip`
- Constraints: ninguna
- ✅ Export JSON  →  formato `Hash`
- Guardar: `art/export/<categoría>/<nombre_archivo>.png`
  el JSON se guarda automáticamente al lado con el mismo nombre

El archivo PNG y su JSON siempre juntos, mismo nombre, misma carpeta.

## Regla de export (backgrounds)

Backgrounds son PNG planos, full color. No necesitan JSON.
**File > Export** (no sprite sheet) → `art/export/backgrounds/<nombre>.png`
