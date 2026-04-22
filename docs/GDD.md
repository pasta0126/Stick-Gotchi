# Game Design Document — Stick-Gotchi

> Versión 1.0 | 2026-04-22

## Visión

Un compañero digital autocontenido que vive, crece y muere en el dispositivo. Sin dependencias externas. La pantalla del M5Stick C Plus2 es su mundo completo. La filosofía es la del Tamagotchi original de Bandai: cuidado, consecuencias, vínculo emocional.

---

## Ciclo de Vida

### Etapas evolutivas

```
Huevo → Bebé → Joven → Adulto
 1h      1-3d    3-7d    7d+
```

Cada etapa tiene sprites distintos pero mantiene la identidad visual de la semilla (misma paleta, mismos ojos). La rama evolutiva Joven → Adulto depende del nivel de cuidado acumulado:

| Cuidado promedio | Forma adulta |
|---|---|
| > 70% stats | Forma saludable (brillante, definida) |
| 40–70% stats | Forma normal |
| < 40% stats | Forma descuidada (más oscura, ojos distintos) |

### Tiempos

| Etapa | Duración | Desencadenante |
|---|---|---|
| Huevo | 1 hora real | Eclosiona solo (o antes si lo cuidas: hablarle, temperatura) |
| Bebé | 1–3 días | Crece al alcanzar niveles de cuidado mínimos |
| Joven | 3–7 días | Crece al cumplir requisitos de cuidado |
| Adulto | 7+ días | Vive hasta que la Salud llegue a 0 |

Con huevo heredado (bonus): cada etapa puede acortarse hasta un 20%.

---

## Sistema de Stats

Cinco valores continuos de 0 a 100:

| Stat | Ícono | Baja cuando | Sube cuando |
|---|---|---|---|
| Hambre | 🍖 | Tiempo (tasa variable por etapa) | Alimentas |
| Energía | ⚡ | Actividad, horas despierto | Duerme |
| Humor | 😊 | Hambre baja, ruido, sacudida fuerte | Juegas, silencio, cariño |
| Salud | ♥ | Descuido prolongado (cualquier stat < 20 por > 30 min) | Tiempo + cuidado activo |
| Edad | 📅 | — | Tiempo real (días vividos) |

### Tasas de degradación base (ajustables)

| Stat | Bebé | Joven | Adulto |
|---|---|---|---|
| Hambre | -1/10min | -1/8min | -1/6min |
| Energía | -1/15min | -1/12min | -1/10min |
| Humor | -1/20min | -1/15min | -1/12min |

Salud no degrada directamente — es consecuencia de descuido acumulado.

---

## Acciones del Usuario

### Mapeado de botones

| Botón | Gesto | Acción |
|---|---|---|
| Btn B | Corto | Ciclar ícono activo en barra de acciones |
| Btn A | Corto | Ejecutar acción seleccionada |
| Btn A | Largo | Mostrar/ocultar barra de acciones |
| Btn C | Corto | Abrir menú del sistema |

### Acciones disponibles

| Ícono | Acción | Efecto |
|---|---|---|
| 🍖 | Alimentar | +Hambre, pequeño +Humor |
| 🎮 | Jugar | +Humor, -Energía |
| 💊 | Medicina | +Salud (solo si está enfermo) |
| 💡 | Luz | Toggle luz encendida/apagada (afecta si duerme) |
| 🛁 | Limpiar | Sube Humor si estaba bajo por "suciedad" (futuro) |

---

## Interacción Ambiental

### Micrófono — Ruido ambiental

Muestreo continuo en background. Umbrales sobre nivel RMS:

| Nivel | Rango | Efecto en gotchi |
|---|---|---|
| Silencio | < 40 dB | Tranquilo, duerme mejor |
| Normal | 40–70 dB | Neutro |
| Ruidoso | > 70 dB | -Humor gradual, interrumpe sueño |
| Estruendo | > 85 dB sostenido (>2s) | Mood SCARED temporal, -Humor notable |

### IMU — Movimiento / Sacudida

Detectado por variación de aceleración (Δg sobre baseline):

| Intensidad | Δg | Efecto |
|---|---|---|
| Suave (< 0.5g) | Cosquilleo | Lo despierta feliz si dormía, lo entretiene si está activo |
| Media (0.5–1.5g) | Mareo | Mood DIZZY temporal (3–5s) |
| Fuerte (> 1.5g) | Susto | Mood SCARED, -Humor, -Salud si repetida |
| Muy fuerte (> 2.5g) | Daño | -Salud directa |

---

## Sistema de Tiempo y Sueño

### Tiempo real (RTC)

El gotchi vive en tiempo real. La hora del día afecta su comportamiento:

| Franja | Horas | Estado |
|---|---|---|
| Noche | 22:00–07:00 | Tiende a dormir, no muere por descuido en stats |
| Día | 07:00–22:00 | Activo, stats degradan normalmente |

### Mecánica de sueño

- A las 22:00 → el gotchi bosteza y se duerme solo.
- Si Energía < 20% durante el día → se duerme sin importar la hora.
- **Ruido durante sueño** (> 70 dB) → lo desvela, -Humor al despertar.
- **Sacudida suave de noche** → lo despierta con buen humor.
- **Sacudida fuerte de noche** → lo despierta asustado, muy mal humor.
- Mientras duerme: Energía sube, Hambre sigue bajando lentamente.

### Protección nocturna

Durante las horas de sueño (22:00–07:00), la Salud no degrada aunque los stats estén bajos. El gotchi no puede morir de noche — pero al despertar sí puede estar en estado crítico.

---

## Muerte y Herencia

### Muerte

- Salud llega a 0 y permanece así durante **15 minutos** → muerte.
- Período de agonía visible: animación diferente, stats en rojo, el gotchi se ve mal.
- Muerte definitiva: pantalla de despedida + transición al huevo.

### Sistema de herencia — Lineaje

Cada gotchi tiene un **GotchiID** de 64 bits:

```
[ generación 8b ][ parent_id 24b ][ seed_visual 16b ][ timestamp_nacimiento 16b ]
```

Al morir, el gotchi "deja un huevo". El nuevo GotchiID se calcula:

```
nueva_seed_visual = mutar(seed_visual_padre, ±rango_pequeño)
nueva_generación = generación_padre + 1
parent_id = id_padre
```

### Bonuses heredados (según vida del padre)

| Condición del padre | Bonus en hijo |
|---|---|
| Vivió > 14 días | Eclosión 20% más rápida |
| Humor promedio > 75 | +10 Humor base |
| Salud promedio > 75 | +10 Salud base |
| Murió con > 60 en todos los stats | Forma adulta ligeramente más bella |

El historial de los últimos 5 antepasados se guarda en NVS (ID + días vividos + causa de muerte).

---

## Pantalla de Estadísticas (Stats Screen)

App separada accesible desde el menú. Muestra:

### Pestaña 1 — Estado actual

```
┌─────────────────────────────────────────────┐
│  NOMBRE          GOTCHI-7F3A    GEN 3        │
│  ─────────────────────────────────────────  │
│  Edad     ████████████░░░░  12 días          │
│  Hambre   ████████░░░░░░░░  67%              │
│  Energía  ██████████████░░  89%              │
│  Humor    ██████░░░░░░░░░░  45% ← bajo!      │
│  Salud    ████████████████  98%              │
│  ─────────────────────────────────────────  │
│  Etapa: ADULTO (forma saludable)             │
│  Estado: DESPIERTO / PENSATIVO               │
└─────────────────────────────────────────────┘
```

### Pestaña 2 — Linaje

```
┌─────────────────────────────────────────────┐
│  ÁRBOL GENEALÓGICO                          │
│                                             │
│  [Tú]   7F3A  Gen3  12 días  (vivo)         │
│  [P]    3B21  Gen2  23 días  (vejez)        │
│  [A]    1A09  Gen1   8 días  (descuido)     │
│  [B]    0001  Gen0   3 días  (primer gotchi)│
└─────────────────────────────────────────────┘
```

### Pestaña 3 — Histórico

```
┌─────────────────────────────────────────────┐
│  SESIÓN ACTUAL                              │
│  Tiempo vivo:    12d 4h 33m                 │
│  Humor promedio: 71%                        │
│  Muertes por hambre: 0                      │
│  Veces alimentado:  47                      │
│  Veces jugado:      23                      │
│  Sacudidas fuertes:  2                      │
└─────────────────────────────────────────────┘
```

Navegación entre pestañas: Btn B. Salir: Btn A largo.

---

## Identidad Visual — Sistema de Semillas

### GotchiSeed (16 bits)

```
[ hue_primario 5b ][ hue_secundario 5b ][ tipo_ojos 2b ][ marcas 2b ][ silueta 2b ]
```

| Campo | Opciones |
|---|---|
| hue_primario | 32 tonos de hue (paleta HSV, S y V fijos por etapa) |
| hue_secundario | 32 tonos (complementario o análogo) |
| tipo_ojos | 0=redondo, 1=diamante, 2=corazón, 3=estrella |
| marcas | 0=ninguna, 1=puntos, 2=rayas, 3=corazones |
| silueta | 0=redondo, 1=cuadrado, 2=triangular, 3=irregular |

La semilla no cambia durante la vida del gotchi. Al heredar, se muta ±1–2 bits en hue y marcas — evolución visual gradual a través de generaciones.

### Sprites por etapa

| Etapa | Tamaño base | Escala render | Píxeles en pantalla |
|---|---|---|---|
| Huevo | 12×14 px | 3× | 36×42 px |
| Bebé | 16×16 px | 3× | 48×48 px |
| Joven | 20×22 px | 2× | 40×44 px |
| Adulto | 24×26 px | 2× | 48×52 px |

Animaciones: idle (2 frames), comer (3 frames), jugar (4 frames), dormir (2 frames), morir (4 frames).

---

## Layout de Pantalla Principal

```
┌─────────────────────────────────────────────┐  240px
│ ♥98  🍖67  ⚡89  😊45        22:34  GEN3  │  ← 18px stats bar
│ ─────────────────────────────────────────── │
│                                             │
│                 [ GOTCHI ]                  │  ← 90px zona gotchi
│                                             │
│  [ 🍖 ]  [ 🎮 ]  [ 💊 ]  [ 💡 ]  [ 🛁 ]  │  ← 27px action bar
└─────────────────────────────────────────────┘
                                             135px
```

La barra de acciones se oculta si no hay interacción en 10s. Ícono activo resaltado con borde blanco.

---

## Moods

Sistema existente conservado y extendido:

| Mood | Valor | Cuándo |
|---|---|---|
| NEUTRAL | 0 | Estado base |
| HAPPY | 1 | Stats altos, acaba de comer/jugar |
| SICK | 2 | Salud < 30 |
| PENSIVE | 3 | Humor 30–50, sin interacción reciente |
| SAD | 4 | Humor < 30 |
| SLEEPING | 5 | Dormido |
| EXCITED | 6 | Acaba de jugar / sacudida suave |
| LAUGHING | 7 | Interacción positiva intensa |
| DIZZY | 8 | Sacudida media |
| ANNOYED | 9 | Ruido persistente > 70 dB |
| ANGRY | 10 | Hambre < 10 + descuido prolongado |
| STARTLED | 11 | Ruido súbito > 85 dB |
| SCARED | 12 | Sacudida fuerte / oscuridad súbita |

---

## Apps en el menú

| App | Descripción |
|---|---|
| Gotchi | App principal — mascota viva |
| Stats | Pantalla de estadísticas y linaje |
| IMU Demo | Demo de acelerómetro (conservada) |

---

## Roadmap

| Fase | Contenido |
|---|---|
| MVP | Stats, sueño, muerte, herencia básica, UI con iconos |
| V1 | Sistema visual con semilla, sprites por etapa, animaciones |
| V2 | Pantalla de stats completa, linaje, histórico |
| V3 | WiFi/BLE gotchi-a-gotchi (futuro) |
