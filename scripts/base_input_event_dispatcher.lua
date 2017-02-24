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

BaseInputEventDispatcher = class('BaseInputEventDispatcher')

function BaseInputEventDispatcher:initialize(ied)
    ied = ied or {}

    self.input_interfaces = ied['input_interfaces'] or {}
    self.current_event = d2k.InputEvent:new()
end

function BaseInputEventDispatcher:dispatch_events()
    while true do
        self.current_event:reset()
        event_received, event_populated = d2k.populate_event(
            self.current_event
        )

        if not event_received then
            break
        end

        if event_populated then
            local result, err = pcall(self.dispatch_event, self)

            if err then
                print(string.format(
                    'InputHandler:dispatch_events: error: %s', err
                ))
            end
      end
    end
end

function BaseInputEventDispatcher:dispatch_event()
    self.input_interfaces:handle_event(self.current_event)
end

return {BaseInputEventDispatcher = BaseInputEventDispatcher}

-- vi: et ts=4 sw=4

