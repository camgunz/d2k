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
local BaseInputEventDispatcher = require('base_input_event_dispatcher')

InputEventDispatcher = class('InputEventDispatcher',
    BaseInputEventDispatcher.BaseInputEventDispatcher
)

function InputEventDispatcher:dispatch_event()
  if d2k.Input.handle_event(self.current_event) then
    return
  end

  if self.current_event:is_key_press(d2k.KeyBinds.get_screenshot()) then
      d2k.Misc.take_screenshot()
  end

  BaseInputEventDispatcher.BaseInputEventDispatcher.dispatch_event(self)

  --[[
  if d2k.Menu.is_active() then
    if d2k.Menu.handle_event(self.current_event) then
      return
    end
  end

  if d2k.hud:handle_event(self.current_event) then
    return
  end

  if d2k.Menu.handle_event(self.current_event) then
    return
  end

  if d2k.Main.handle_event(self.current_event) then
    return
  end

  if d2k.StatusBar.handle_event(self.current_event) then
    return
  end

  if d2k.AutoMap.handle_event(self.current_event) then
    return
  end

  d2k.Game.handle_event(self.current_event)
  --]]
end

return {InputEventDispatcher = InputEventDispatcher}

-- vi: et ts=4 sw=4

