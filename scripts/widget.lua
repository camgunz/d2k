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

  setmetatable(w, self)
  self.__index = self

  w:set_name('HUD Widget')
  w:deactivate()

  return w
end

function Widget:get_name()
  return self.name
end

function Widget:set_name(name)
  self.name = name
end

function Widget:reset()
end

function Widget:tick()
end

function Widget:draw()
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
  self:activate()
end

function Widget:on_remove(hud)
  self:deactivate()
end

function Widget:handle_event(event)
end

return {Widget = Widget}

-- vi: et ts=2 sw=2

