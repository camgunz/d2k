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

Console = hud_widget.HUDWidget:new({
  SCROLL_DOWN_TIME = 150.0,
  SCROLL_UP_TIME   = 150.0,
  MARGIN           = 8
})

function Console:new(c)
  c = c or {}
  setmetatable(c, self)
  self.__index = self

  c.layout_context = nil
  c.layout = nil
  c.scroll_rate = 0.0
  c.height = 0.0
  c.max_width = d2k.Video.get_screen_width()
  c.max_height = d2k.Video.get_screen_height()
  c:build_layout()

  return c
end

function Console:get_name()
  return 'console'
end

function Console:reset()
end

function Console:tick()
end

function Console:build_layout()
  self.render_context = d2k.overlay.render_context
  self.layout_context = PangoCairo.create_context(d2k.overlay.render_context)
  self.layout = Pango.Layout.new(self.layout_context)
  self.layout:set_font_description(Pango.FontDescription.from_string(
    'Monkirta Pursuit NC 10'
  ))
end

function Console:draw()
  if not self.render_context then
    self:build_layout()
  elseif d2k.overlay.render_context ~= self.render_context then
    self:build_layout()
  end

  self.layout:set_text("Hey There", -1)
  PangoCairo.update_context(d2k.overlay.render_context, self.layout_context)
  PangoCairo.update_layout(d2k.overlay.render_context, self.layout)
  PangoCairo.show_layout(d2k.overlay.render_context, self.layout)
end

return {Console = Console}

-- vi: et ts=2 sw=2

