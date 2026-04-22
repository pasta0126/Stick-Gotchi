# Fondos de escena

## Dimensiones

```
240 × 90 px   (play zone exacta de la pantalla)
```

El stats bar (arriba) y la action bar (abajo) se dibujan sobre el fondo en código.
El fondo cubre solo la zona central donde vive el gotchi.

## Paleta

Full color — sin restricciones de paleta. Estos sprites no usan el sistema de 5 índices.
Usar RGB normal en Aseprite (Image > Color Mode → RGB).

Sin embargo: mantener colores limitados y coherentes. Estilo pixel art, no ilustración
detallada. El gotchi tiene que leerse bien sobre el fondo — evitar colores que compitan
con los colores primarios (azul/cyan/amarillo que usamos como placeholder).

---

## Archivos a crear

### bg_day.aseprite — 240 × 90 px
Escena interior de día. Idea: habitación simple estilo solarpunk/retro.
- Piso distinguible del techo/pared
- Elemento decorativo izquierda y/o derecha (planta, ventana, lámpara)
- Centro despejado — ahí vive el gotchi
- Colores cálidos-neutros, luz diurna

### bg_night.aseprite — 240 × 90 px
Misma habitación, iluminación nocturna.
- Más oscuro, azules/morados
- Si hay ventana: estrellas o luna
- El gotchi duerme aquí — ambiente tranquilo

---

## Notas de diseño

- El gotchi se renderiza encima del fondo, centrado en la play zone
- No poner elementos que "interfieran" con la zona central (aprox x: 80–160)
- Piso sugerido en y ≈ 70–80 de los 90px (el gotchi "para" ahí)
- Estilo: pocos colores, pixel art limpio, no foto-realista

---

## Export

File > Export (no sprite sheet) → PNG plano
Destino: `art/export/backgrounds/bg_day.png` / `bg_night.png`
No se necesita JSON para los fondos.
