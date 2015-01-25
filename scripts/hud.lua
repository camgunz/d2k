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

HUD = {}

function HUD:new(h)
  h = h or {
    widgets = {},
    active = false,
    font_description_text = 'Monkirta Pursuit NC 10'
  }

  setmetatable(h, self)
  self.__index = self

  return h
end

function HUD:add_widget(widget)
  table.insert(self.widgets, widget)

  widget.hud = self
  widget:on_add(self)
end

function HUD:remove_widget(widget)
  for i = #self.widgets, 1, -1 do
    local w = self.widgets[i]

    if w == widget then
      table.remove(self.widgets, i)
    end
  end

  widget.hud = nil
  widget:on_remove(self)
end

function HUD:start()
  if self.active then
    self:stop()
  end

  self.active = true

  for i, w in pairs(self.widgets) do
    w:reset()
  end
end

function HUD:stop()
  self.active = false
end

function HUD:tick()
  if not self.active then
    return
  end

  for i, w in pairs(self.widgets) do
    w:tick()
  end
end

function HUD:draw()
  if not self.active then
    return
  end

  d2k.overlay:lock()

  if d2k.Video.using_opengl() then
    d2k.overlay:clear()
  end

  d2k.overlay.render_context:set_operator(Cairo.Operator.OVER)

  for i, w in pairs(self.widgets) do
    w:draw()
  end

  d2k.overlay:unlock()
end

function HUD:handle_event(event)
  print('Hello from hud.handle_event')
  for i, w in pairs(self.widgets) do
    if w:is_active() and w:handle_event(event) then
      return true
    end
  end

  return false
end

return {HUD = HUD}

-- vi: et ts=2 sw=2

