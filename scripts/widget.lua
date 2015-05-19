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

local Widget = {}

function Widget:new(w)
  w = w or {}

  w.x = w.x or 0
  w.y = w.y or 0
  w.width = w.width or 0
  w.height = w.height or 0
  w.z_index = 0

  w.enabled = false
  w.active = false
  w.needs_updating = false

  setmetatable(w, self)
  self.__index = self

  w:set_name(w.name or 'Widget')
  w:deactivate()

  return w
end

function Widget:get_name()
  return self.name
end

function Widget:set_name(name)
  self.name = name
end

function Widget:get_x()
  return self.x
end

function Widget:set_x(x)
  self.x = x
end

function Widget:get_y()
  return self.y
end

function Widget:set_y(y)
  self.y = y
end

function Widget:get_width()
  return self.width
end

function Widget:set_width(width)
  if width == self.width then
    return
  end

  self.width = width
  self.needs_updating = true
  self:check_offsets()
end

function Widget:get_height()
  return self.height
end

function Widget:set_height(height)
  if height == self.height then
    return
  end

  self.height = height
  self.needs_updating = true
  self:check_offsets()
end

function Widget:get_z_index()
  return self.z_index
end

function Widget:set_z_index(z_index)
  self.z_index = z_index
  d2k.hud:sort_widgets()
end

function Widget:reset()
end

function Widget:tick()
end

function Widget:draw()
end

function Widget:enable()
  self.enabled = true
end

function Widget:disable()
  self.enabled = false
end

function Widget:is_enabled()
  return self.enabled
end

function Widget:activate()
  self.active = true
end

function Widget:deactivate()
  self.active = false
end

function Widget:is_active()
  return self.active
end

function Widget:on_add(hud)
  self:enable()
  d2k.hud:sort_widgets()
end

function Widget:on_remove(hud)
  self:disable()
end

function Widget:on_size_change()
end

function Widget:handle_event(event)
end

return {Widget = Widget}

-- vi: et ts=2 sw=2

