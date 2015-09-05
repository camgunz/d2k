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
local InputInterfaceContainer = require('input_interface_container')

GameInterface = class('GameInterface', InputInterface.InputInterface)
GameInterface:include(InputInterfaceContainer.InputInterfaceContainer)

function GameInterface:initialize(gi)
  gi = gi or {}

  gi.name = gi.name or 'Game Interface'

  InputInterface.InputInterface.initialize(self, gi)

  self.interfaces = {}
end

function GameInterface:in_level()
  return d2k.Game.in_level()
end

function GameInterface:tick()
  d2k.Game.tick()
  InputInterfaceContainer.InputInterfaceContainer.tick(self)
end

function GameInterface:draw()
  d2k.Game.draw()
  InputInterfaceContainer.InputInterfaceContainer.draw(self)
end

function GameInterface:handle_event(event)
  local handled = false
  local active_before = self:is_active()

  handled = InputInterfaceContainer.InputInterfaceContainer.handle_event(
    self, event
  )

  if handled then
    return true
  end

  if not d2k.Game.in_level() then
    return false
  end

  handled = d2k.Game.handle_event(event)

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

return {GameInterface = GameInterface}

-- vi: et ts=2 sw=2

