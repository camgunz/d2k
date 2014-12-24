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

HUD = {}

function HUD:main()
  print('Main!')
end

function HUD:init()
  print('Initializing HUD')

  self.render_context = xf.get_render_context()
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
  if self.active then
    self.stop()
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
  for i, w in pairs(self.widgets) do
    w:tick()
  end
end

function HUD:draw()
  for i, w in pairs(self.widgets) do
    w:draw()
  end
end

return {HUD = HUD}

-- vi: et ts=2 sw=2

