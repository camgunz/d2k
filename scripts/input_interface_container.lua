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

-- Requires:
--   self.interfaces = {}

InputInterfaceContainer = {

    get_active_interface = function(self)
        if not self.interfaces then
            return
        end

        for i, interface in ipairs(self.interfaces) do
            if interface:is_active() then
                return interface
            end
        end
    end,

    add_interface = function(self, interface)
        local name = 'unknown container'
        local get_name = self.get_name

        if get_name then
            name = get_name(self)
        end

        if self.interfaces then
            for i, iface in ipairs(self.interfaces) do
                if iface == interface then
                    return
                end
            end
        end

        table.insert(self.interfaces, interface)

        self:sort_interfaces()
    end,

    remove_interface = function(self, interface)
        if not self.interfaces then
            return
        end

        for i, iface in ipairs(self.interfaces) do
            if iface == interface then
                table.remove(self.interfaces, i)
                return
            end
        end

        self:sort_interfaces()
    end,

    get_interface = function(self, interface_name)
        if not self.interfaces then
            return
        end

        for i, interface in ipairs(self.interfaces) do
            if interface:get_name() == interface_name then
                return interface
            end
        end
    end,

    activate = function(self, interface)
        if not self.interfaces then
            return
        end

        for i, iface in pairs(self.interfaces) do
            if iface == interface and not iface:is_active() then
                table.remove(self.interfaces, i)
                table.insert(self.interfaces, 1, iface)
            end
        end
    end,

    is_active = function(self)
        if not self.interfaces then
            return false
        end

        for i, iface in ipairs(self.interfaces) do
            if iface:is_active() then
                return true
            end
        end

        return false
    end,

    reset = function(self)
        if not self.interfaces then
            return
        end

        for i, interface in ipairs(self.interfaces) do
            interface:reset()
        end
    end,

    tick = function(self)
        if not self.interfaces then
            return
        end

        for i, interface in ipairs(self.interfaces) do
            interface:tick()
        end
    end,

    invalidate_render = function(self)
        if not self.interfaces then
            return
        end

        if self.interfaces then
            for i, interface in ipairs(self.interfaces) do
                interface:invalidate_render()
            end
        end
    end,

    needs_rendering = function(self)
        for i, interface in ipairs(self.interfaces) do
            if interface:needs_rendering() then
                return true
            end
        end

        return false
    end,

    render = function(self)
        if not self.interfaces then
            return
        end

        local cr = d2k.overlay.render_context

        for i, interface in ipairs(self.interfaces) do
            cr:save()
            cr:set_source(interface:get_render())
            interface:clip_view()
            interface:position_view()
            cr:clip()
            cr:paint()
            cr:restore()
        end
    end,

    handle_event = function(self, event)
        if not self.interfaces then
            return false
        end

        for i, interface in ipairs(self.interfaces) do
            if interface:is_active() then
                if interface:handle_event(event) then
                    return true
                end
            end
        end

        for i, interface in ipairs(self.interfaces) do
            if not interface:is_active() then
                if interface:handle_event(event) then
                    return true
                end
            end
        end

        return false
    end,

    scale = function(self, value)
        if self.interfaces then
            for i, interface in ipairs(self.interfaces) do
                interface:scale(value)
            end
        end
    end,

    handle_overlay_reset = function(self)
        if self.interfaces then
            for i, interface in ipairs(self.interfaces) do
                interface:handle_overlay_reset()
            end
        end
    end,

    sort_interfaces = function(self)
    end,

}

return {InputInterfaceContainer = InputInterfaceContainer}

-- vi: et ts=4 sw=4

