-- egg.aseprite: 12x14px, Indexed (5 colores), 6 frames
-- Rediseno: vial cybertech con cadena de ADN
-- Tags: idle (1-2), hatch (3-6)

local OUTPUT = "D:\\projects\\Stick-Gotchi\\art\\sprites\\gotchi\\egg.aseprite"

local T = 0   -- transparente
local P = 1   -- primario   (azul  -> DNA reemplaza en runtime)
local H = 2   -- highlight  (cyan  -> cristal/brillo)
local O = 3   -- outline    (gris  -> contorno)
local A = 4   -- acento     (amarillo -> ADN / energia)

-- 6 frames: [frame][row 1..14][col 1..12]
local FRAMES = {

  -- Frame 1: idle - vial completo
  -- Cap (tapa) ancha, tubo con ADN en zigzag, base redondeada
  {
    {T,T,T,O,O,O,O,O,T,T,T,T},  -- cap top
    {T,T,O,P,H,P,P,H,P,O,T,T},  -- cap body (H = paneles tech)
    {T,T,O,O,O,O,O,O,O,O,T,T},  -- cap base
    {T,T,T,O,H,P,P,P,O,T,T,T},  -- vial top
    {T,T,T,O,H,A,P,A,O,T,T,T},  -- ADN: A en cols 5 y 7
    {T,T,T,O,H,P,A,P,O,T,T,T},  -- puente ADN col 6
    {T,T,T,O,H,A,P,A,O,T,T,T},  -- ADN
    {T,T,T,O,H,P,A,P,O,T,T,T},  -- puente
    {T,T,T,O,H,A,P,A,O,T,T,T},  -- ADN
    {T,T,T,O,H,P,A,P,O,T,T,T},  -- puente
    {T,T,T,O,H,A,P,A,O,T,T,T},  -- ADN final
    {T,T,T,O,H,P,P,P,O,T,T,T},  -- vial bottom
    {T,T,T,T,O,P,P,O,T,T,T,T},  -- redondeado
    {T,T,T,T,T,O,O,T,T,T,T,T},  -- punta
  },

  -- Frame 2: idle bob - desplazado 1px abajo
  {
    {T,T,T,T,T,T,T,T,T,T,T,T},  -- vacio (shift)
    {T,T,T,O,O,O,O,O,T,T,T,T},  -- cap top
    {T,T,O,P,H,P,P,H,P,O,T,T},  -- cap body
    {T,T,O,O,O,O,O,O,O,O,T,T},  -- cap base
    {T,T,T,O,H,P,P,P,O,T,T,T},  -- vial top
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,A,P,O,T,T,T},
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,A,P,O,T,T,T},
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,A,P,O,T,T,T},
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,P,P,O,T,T,T},  -- bottom (punta cortada por shift)
    {T,T,T,T,O,P,P,O,T,T,T,T},
  },

  -- Frame 3: hatch F0 - grieta en la tapa
  {
    {T,T,T,O,O,A,O,O,T,T,T,T},  -- grieta en centro del cap top
    {T,T,O,P,H,A,P,H,P,O,T,T},  -- grieta atraviesa cap body
    {T,T,O,O,O,A,O,O,O,O,T,T},  -- grieta en cap base
    {T,T,T,O,H,P,P,P,O,T,T,T},  -- vial intacto
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,A,P,O,T,T,T},
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,A,P,O,T,T,T},
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,A,P,O,T,T,T},
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,P,P,O,T,T,T},
    {T,T,T,T,O,P,P,O,T,T,T,T},
    {T,T,T,T,T,O,O,T,T,T,T,T},
  },

  -- Frame 4: hatch F1 - tapa se parte, energia escapando
  {
    {T,T,O,T,T,A,T,T,O,T,T,T},  -- tapa abierta (hueco en centro)
    {T,T,O,A,H,T,H,A,O,T,T,T},  -- mitades del cap separandose
    {T,T,T,O,A,H,H,A,O,T,T,T},  -- energia/glow saliendo
    {T,T,T,O,H,A,A,H,O,T,T,T},  -- grieta entra al vial
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,A,P,O,T,T,T},
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,A,P,O,T,T,T},
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,A,P,O,T,T,T},
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,P,P,O,T,T,T},
    {T,T,T,T,O,P,P,O,T,T,T,T},
    {T,T,T,T,T,O,O,T,T,T,T,T},
  },

  -- Frame 5: hatch F2 - vial se rompe, ADN escapando
  {
    {T,A,T,T,T,T,T,T,A,T,T,T},  -- fragmentos de tapa volando
    {T,T,T,O,T,A,A,T,O,T,T,T},  -- restos del cap
    {T,T,O,T,A,H,H,A,T,O,T,T},  -- energia saliendo
    {T,T,A,O,T,A,T,O,A,T,T,T},  -- vial superior roto
    {T,T,T,A,H,A,P,A,H,A,T,T},  -- ADN escapando a los lados
    {A,T,T,T,A,P,H,P,A,T,T,A},  -- hebras de ADN disparandose
    {T,T,T,O,H,A,P,A,O,T,T,T},  -- mitad inferior del vial aun intacta
    {T,T,T,O,H,P,A,P,O,T,T,T},
    {T,T,T,O,H,A,P,A,O,T,T,T},
    {T,T,T,O,H,P,P,P,O,T,T,T},
    {T,T,T,T,O,P,P,O,T,T,T,T},
    {T,T,T,T,T,O,O,T,T,T,T,T},
    {T,T,T,T,T,T,T,T,T,T,T,T},
    {T,T,T,T,T,T,T,T,T,T,T,T},
  },

  -- Frame 6: hatch F3 - explosion radial de energia (vial desaparece)
  {
    {T,T,T,T,A,T,T,A,T,T,T,T},  -- chispas lejanas
    {T,T,A,T,T,H,H,T,T,A,T,T},  -- anillo exterior
    {T,A,T,T,P,H,H,P,T,T,A,T},  -- ADN orbital
    {T,T,T,P,H,A,A,H,P,T,T,T},  -- nucleo brillando
    {T,T,A,P,A,H,H,A,P,A,T,T},  -- maximo brillo
    {T,T,H,A,H,P,P,H,A,H,T,T},  -- centro
    {T,T,A,P,A,H,H,A,P,A,T,T},  -- simetrico
    {T,T,T,P,H,A,A,H,P,T,T,T},
    {T,A,T,T,P,H,H,P,T,T,A,T},
    {T,T,A,T,T,H,H,T,T,A,T,T},
    {T,T,T,T,A,T,T,A,T,T,T,T},  -- chispas lejanas
    {T,T,T,T,T,T,T,T,T,T,T,T},
    {T,T,T,T,T,T,T,T,T,T,T,T},
    {T,T,T,T,T,T,T,T,T,T,T,T},
  },
}

-- Crear sprite
local spr = Sprite(12, 14, ColorMode.INDEXED)

local pal = Palette(5)
pal:setColor(0, Color{r=255, g=0,   b=255, a=255})
pal:setColor(1, Color{r=0,   g=0,   b=255, a=255})
pal:setColor(2, Color{r=0,   g=255, b=255, a=255})
pal:setColor(3, Color{r=64,  g=64,  b=64,  a=255})
pal:setColor(4, Color{r=255, g=255, b=0,   a=255})
spr:setPalette(pal)
spr.transparentColor = 0

local layer = spr.layers[1]
layer.name = "sprites"

for i = 2, 6 do
  spr:newFrame()
end

for fi = 1, 6 do
  local cel = layer:cel(fi)
  if not cel then
    cel = spr:newCel(layer, spr.frames[fi])
  end
  local img = cel.image
  local data = FRAMES[fi]
  for y = 0, 13 do
    for x = 0, 11 do
      img:putPixel(x, y, data[y+1][x+1])
    end
  end
end

local tagIdle  = spr:newTag(1, 2)
tagIdle.name   = "idle"

local tagHatch = spr:newTag(3, 6)
tagHatch.name  = "hatch"

spr:saveAs(OUTPUT)
print("OK: " .. OUTPUT)
