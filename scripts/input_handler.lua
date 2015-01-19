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

InputHandler = {}

function InputHandler:new(ih)
  ih = ih or {
    current_event = d2k.InputEvent:new(),
  }

  setmetatable(ih, self)
  self.__index = self

  return ih
end

function InputHandler:handle_event()
  local ev = self.current_event

  if d2k.System.handle_event(ev) then
    return
  end

  if not d2k.Video.is_enabled() then
    if d2k.Game.handle_event(ev) then
      return
    end
  end

  if ev:is_key_press(d2k.KeyBinds.get_screenshot()) then
    d2k.Misc.take_screenshot()
  end

  if d2k.Menu.is_active() then
    if d2k.Menu.handle_event(ev) then
      return
    end
  end

  if d2k.hud:handle_event(ev) then
    return
  end

  if d2k.Menu.handle_event(ev) then
    return
  end

  if d2k.Main.handle_event(ev) then
    return
  end

  if d2k.StatusBar.handle_event(ev) then
    return
  end

  if d2k.AutoMap.handle_event(ev) then
    return
  end

  d2k.Game.handle_event(ev)
end

function InputHandler:handle_events()
  while true do
    self.current_event:reset()
    event_received, event_populated = d2k.populate_event(self.current_event)

    if not event_received then
      break
    end

    if event_populated then
      self:handle_event()
    end
  end
end

return {InputHandler = InputHandler}

-- vi: et ts=2 sw=2

