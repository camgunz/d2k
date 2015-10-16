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

local class = require('middleclass')
local InputInterface = require('input_interface')

Menu = class('Menu', InputInterface.InputInterface)

function Menu:initialize(m)
  m = m or {}

  m.name = m.name or 'Menu'
  m.fullscreen = m.fullscreen or true

  InputInterface.InputInterface.initialize(self, m)
end

function Menu:activate()
  InputInterface.InputInterface.activate(self)
  d2k.Menu.activate()
end

function Menu:deactivate()
  InputInterface.InputInterface.deactivate(self)
  d2k.Menu.deactivate()
end

function Menu:reset()
  InputInterface.InputInterface.reset(self)
  self:deactivate()
end

function Menu:tick()
  d2k.Menu.tick()
end

function Menu:render()
  d2k.Menu.render()
end

function Menu:handle_event(event)
  local active_before = self:is_active()
  local handled = d2k.Menu.handle_event(event)

  if handled then
    local active_after = self:is_active()

    if active_before == false and active_after == true then
      self:activate()
    elseif active_before == true and active_after == false then
      self:deactivate()
    end
  end

  return handled
end

return {Menu = Menu}

-- vi: et ts=2 sw=2

