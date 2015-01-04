-------------------------------------------------------------------------------
--                                                                           --
-- D2K: A Doom Source Port for the 21st Century                              --
--                                                                           --
-- Copyright (C) 2014: See COPYRIGHT file                                    --
--                                                                           --
-- This file is part of D2K.                                                 --
--                                                                           --
-- D2K is free software: you can redistribute it and/or modify it under the  --
-- terms of the GNU General Public License as published by the Free Software --
-- Foundation, either version 2 of the License, or (at your option) any      --
-- later version.                                                            --
--                                                                           --
-- D2K is distributed in the hope that it will be useful, but WITHOUT ANY    --
-- WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS --
-- FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more    --
-- details.                                                                  --
--                                                                           --
-- You should have received a copy of the GNU General Public License along   --
-- with D2K.  If not, see <http://www.gnu.org/licenses/>.                    --
--                                                                           --
-------------------------------------------------------------------------------

local lgi = require('lgi')
local Cairo = lgi.cairo
local Pango = lgi.Pango
local PangoCairo = lgi.PangoCairo
local hud_widget = require('hud_widget')

Console = hud_widget.HUDWidget:new()

function Console:get_name()
  return 'console'
end

function Console:reset()
end

function Console:tick()
end

function Console:old_draw()
  local xc = 80
  local yc = 60
  local radius = 30
  local angle1 = math.rad(45)
  local angle2 = math.rad(180)

  d2k.overlay.context:set_source_rgba(0, 0, 0, 0.6)
  d2k.overlay.context:set_line_width(10)
  d2k.overlay.context:arc(xc, yc, radius, angle1, angle2)
  d2k.overlay.context:stroke()
  d2k.overlay.context:set_source_rgba(1, 0.2, 0.2, 0.6)
  d2k.overlay.context:set_line_width(6)
  d2k.overlay.context:arc(xc, yc, 10, 0, math.rad(360))
  d2k.overlay.context:fill()
  d2k.overlay.context:arc(xc, yc, radius, angle1, angle1)
  d2k.overlay.context:line_to(xc, yc)
  d2k.overlay.context:arc(xc, yc, radius, angle2, angle2)
  d2k.overlay.context:line_to(xc, yc)
  d2k.overlay.context:stroke()
end

function Console:draw()
  local pango_context = PangoCairo.create_context(d2k.overlay.context)
  local pango_layout = Pango.Layout.new(pango_context)

  -- local font_description = Pango.FontDescription.from_string('Sans Bold 27')

  -- layout:set_font_description(font_description)
  pango_layout:set_text("Hey There", -1)
  PangoCairo.update_context(d2k.overlay.context, pango_context)
  PangoCairo.update_layout(d2k.overlay.context, pango_layout)

  PangoCairo.show_layout(d2k.overlay.context, pango_layout)
end

function Console:new_draw()
  local word_count = 9
  local font_description = Pango.FontDescription.from_string('Sans Bold 27')
  local width = d2k.get_screen_width()
  local height = d2k.get_screen_height()
  local radius = (width < height and width or height) / 2
  local layout = PangoCairo.create_layout(d2k.overlay.context)

  layout:set_text("Text", -1)
  layout:set_font_description(font_description)
  d2k.overlay.context:translate(radius, radius)

  -- Draw the layout word_count times in a circle
  for i = 0, word_count do
    local angle = (360 * i) / word_count
    -- d2k.overlay.context:save()

    -- Color gradient
    local red = (1 + math.cos((angle - 60) * math.pi / 180)) / 2
    d2k.overlay.context:set_source_rgb(red, 0, 1 - red)
    d2k.overlay.context:rotate(angle * math.pi / 180)

    -- Inform Pango to re-layout the text with the new transformation
    PangoCairo.update_layout(d2k.overlay.context, layout)
    local width, height = layout:get_size()
    d2k.overlay.context:move_to(-(width / Pango.SCALE) / 2, -radius)
    PangoCairo.show_layout(d2k.overlay.context, layout)

    -- d2k.overlay.context:restore()
  end
end

return {Console = Console}

-- vi: et ts=2 sw=2

