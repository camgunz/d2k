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

require('lgob.cairo')
require('lgob.pango')
require('lgob.pangocairo')

HUD = {}

function HUD:init()
  print('HUD: Initializing')

  self.render_surface = cairo.ImageSurface.create(
    cairo.FORMAT_ARGB32,
    d2k.get_screen_width(),
    d2k.get_screen_height()
  )
  self.cr = cairo.Context.create(self.render_surface)
  self.active = false
end

function HUD:add_widget(widget)
  if self.widgets == nil then
    self.widgets = {}
  end

  table.insert(self.widgets, widget)
  widget.hud = self
end

function HUD:remove_widget(widget)
  for i = #self.widgets, 1, -1 do
    local w = self.widgets[i]

    if w == widget then
      table.remove(self.widgets, i)
    end
  end

  widget.hud = nil
end

function HUD:start()
  print('HUD: Starting')
  if self.active then
    self:stop()
  end

  self.active = true

  for i, w in pairs(self.widgets) do
    w:reset()
  end

  d2k.reset_overlay()
end

function HUD:stop()
  self.active = false
end

function HUD:tick()
  for i, w in pairs(self.widgets) do
    w:tick()
  end
end

function HUD:clear()
  self.cr.operator = 'CLEAR'
  self.cr:paint()
end


function HUD:update()
  updated = false

  for i, w in pairs(self.widgets) do
    if w:was_updated() then
      if not updated then
        self:clear()
        self.cr.operator = 'SOURCE'
        updated = true
      end
      w:draw()
    end
  end

  if updated then
    if d2k.using_opengl() then
      self.cr.operator = 'OVER'
    end
    self.render_surface:flush()
  end
end

function HUD:render()
  self:update()

  -- self:clear()

  self.cr.operator = 'SOURCE'

  d2k.blit_overlay(
    self.render_surface:get_data(),
    self.render_surface:get_width(),
    self.render_surface:get_height()
  )

end

return {HUD = HUD}

-- vi: et ts=2 sw=2

