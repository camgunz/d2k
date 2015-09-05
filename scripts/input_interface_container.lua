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
    for i, interface in ipairs(self.interfaces) do
      if interface:is_active() then
        return interface
      end
    end
  end,

  add_interface = function(self, new_interface)
    for i, interface in pairs(self.interfaces) do
      if new_interface == interface then
        return
      end
    end

    table.insert(self.interfaces, new_interface)
  end,

  remove_interface = function(self, interface)
    for i, ifc in pairs(self.interfaces) do
      if ifc == interface then
        table.remove(self.interfaces, i)
        return
      end
    end
  end,

  get_interface = function(self, interface_name)
    for i, interface in pairs(self.interfaces) do
      if interface:get_name() == interface_name then
        return interface
      end
    end
  end,

  activate = function(self, interface)
    for i, iface in ipairs(self.interfaces) do
      if iface == interface and not iface:is_active() then
        table.remove(self.interfaces, i)
        table.insert(self.interfaces, 1, iface)
      end
    end

    self.active = true
  end,

  is_active = function(self)
    for i, iface in pairs(self.interfaces) do
      if iface:is_active() then
        return true
      end
    end

    return false
  end,

  reset = function(self)
    for i, interface in ipairs(self.interfaces) do
      interface:reset()
    end
  end,

  tick = function(self)
    for i, interface in ipairs(self.interfaces) do
      interface:tick()
    end
  end,

  draw = function(self)
    for i, interface in ipairs(self.interfaces) do
      interface:draw()
    end
  end,

  handle_event = function(self, event)
    for i, interface in ipairs(self.interfaces) do
      if interface:is_active() then
        if interface:handle_event(event) then
          return
        end
      end
    end

    for i, interface in ipairs(self.interfaces) do
      if not interface:is_active() then
        if interface:handle_event(event) then
          return
        end
      end
    end
  end,
}

return {InputInterfaceContainer = InputInterfaceContainer}

-- vi: et ts=2 sw=2

