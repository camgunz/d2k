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

ContainerWidget = class('ContainerWidget', InputInterface.InputInterface)
ContainerWidget:include(InputInterfaceContainer.InputInterfaceContainer)

function ContainerWidget:initialize(cw)
    cw = cw or {}

    cw.name = cw.name or 'container widget'

    InputInterface.InputInterface.initialize(self, cw)

    self.interfaces = {}
end

function ContainerWidget:sort_interfaces()
    table.sort(self.interfaces, function(i1, i2)
        return i1:get_z_index() < i2:get_z_index()
    end)
end

function ContainerWidget:activate(interface)
    InputInterface.InputInterface.activate(self)
    if interface then
        InputInterfaceContainer.InputInterfaceContainer.activate(
            self, interface
        )
    end
end

function ContainerWidget:is_active()
    return InputInterface.InputInterface.is_active(self)
end

function ContainerWidget:reset()
    InputInterfaceContainer.InputInterfaceContainer.reset(self)
end

function ContainerWidget:tick()
    InputInterfaceContainer.InputInterfaceContainer.tick(self)
end

function ContainerWidget:invalidate_render()
    InputInterfaceContainer.InputInterfaceContainer.invalidate_render(self)
end

function ContainerWidget:position_view()
end

function ContainerWidget:render()
    InputInterfaceContainer.InputInterfaceContainer.render(self)
end

function ContainerWidget:needs_rendering()
    return (
        InputInterfaceContainer.InputInterfaceContainer.needs_rendering(self)
    )
end

function ContainerWidget:handle_event(event)
    return InputInterfaceContainer.InputInterfaceContainer.handle_event(self, event)
end

function ContainerWidget:scale(value)
    InputInterfaceContainer.InputInterfaceContainer.scale(self, value)
end

function ContainerWidget:handle_overlay_reset()
    InputInterfaceContainer.InputInterfaceContainer.handle_overlay_reset(self)
end

return {ContainerWidget = ContainerWidget}

-- vi: et ts=4 sw=4

