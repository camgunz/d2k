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

HUDWidget = {}

--
-- CG: TODO: "Fullscreen" HUD widget:
--             - HUDWidget:handles_all_events()
--             - HUDWidget:activate()
--               - in hud, keep list of fs widgets in order of activation
--             - HUDWidget:deactivate()
--               - in hud, remove fs widget from list
--             - HUDWidget:is_active()
--

function HUDWidget:new(w)
  w = w or {}

  setmetatable(w, self)
  self.__index = self

  w:set_name('HUD Widget')
  w:deactivate()

  return w
end

function HUDWidget:get_name()
  return self.name
end

function HUDWidget:set_name(name)
  self.name = name
end

function HUDWidget:reset()
end

function HUDWidget:tick()
end

function HUDWidget:draw()
end

function HUDWidget:activate()
  self.active = true
end

function HUDWidget:deactivate()
  self.active = false
end

function HUDWidget:is_active()
  return self.active
end

function HUDWidget:on_add(hud)
  self:activate()
end

function HUDWidget:on_remove(hud)
  self:deactivate()
end

function HUDWidget:handle_event(event)
end

return {HUDWidget = HUDWidget}

-- vi: et ts=2 sw=2

