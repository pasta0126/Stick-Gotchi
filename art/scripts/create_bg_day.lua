-- bg_day.aseprite: 240x135px, RGB, 1 frame
-- Fondo de dia: cielo degradado + nubes + sol + suelo

local OUTPUT = "D:\\projects\\Stick-Gotchi\\art\\sprites\\backgrounds\\bg_day.aseprite"

local function px(r, g, b)
  return app.pixelColor.rgba(r, g, b, 255)
end

local function fillRect(img, x1, y1, x2, y2, color)
  x1 = math.max(0, x1)
  y1 = math.max(0, y1)
  x2 = math.min(img.width  - 1, x2)
  y2 = math.min(img.height - 1, y2)
  for y = y1, y2 do
    for x = x1, x2 do
      img:putPixel(x, y, color)
    end
  end
end

-- Crear sprite RGB
local spr = Sprite(240, 135, ColorMode.RGB)
local layer = spr.layers[1]
layer.name = "background"

local cel = layer:cel(spr.frames[1])
local img = cel.image

-- Cielo: degradado azul (top oscuro -> bottom claro)
for y = 0, 104 do
  local t  = y / 104
  local r  = math.floor(72  + t * 40)   -- 72  -> 112
  local g  = math.floor(140 + t * 45)   -- 140 -> 185
  local b  = math.floor(210 - t * 20)   -- 210 -> 190
  local c  = px(r, g, b)
  for x = 0, 239 do
    img:putPixel(x, y, c)
  end
end

-- Franja de horizonte (hierba lejana, verde suave)
fillRect(img,   0, 105, 239, 114, px(112, 185, 85))
fillRect(img,   0, 115, 239, 122, px(92,  160, 68))

-- Suelo (verde oscuro)
fillRect(img,   0, 123, 239, 128, px(72,  138, 52))
fillRect(img,   0, 129, 239, 134, px(55,  112, 40))

-- Sol (esquina superior derecha)
local sunY = px(255, 215, 50)
local sunH = px(255, 245, 160)
fillRect(img, 204,  6, 230, 32, sunY)   -- cuerpo
fillRect(img, 209,  9, 225, 29, sunH)   -- brillo interior
-- rayos stub
fillRect(img, 211,  2, 222,  5, sunY)   -- arriba
fillRect(img, 231, 12, 235, 26, sunY)   -- derecha
fillRect(img, 211, 33, 222, 36, sunY)   -- abajo
fillRect(img, 199, 12, 203, 26, sunY)   -- izquierda

-- Nube 1 (izquierda, y=22)
local cld = px(240, 248, 255)
fillRect(img,  18, 26,  72, 34, cld)
fillRect(img,  26, 19,  54, 26, cld)
fillRect(img,  38, 15,  52, 19, cld)

-- Nube 2 (centro, y=16)
fillRect(img, 120, 20, 178, 28, cld)
fillRect(img, 130, 13, 162, 20, cld)
fillRect(img, 143, 10, 158, 14, cld)

-- Nube 3 (pequena, y=38)
fillRect(img,  85, 38, 118, 44, cld)
fillRect(img,  92, 33, 108, 38, cld)

spr:saveAs(OUTPUT)
print("OK: " .. OUTPUT)
