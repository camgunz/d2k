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

local classlib = require('classlib')

class.InputInterfaceContainer()

function InputInterfaceContainer:__init()
  self.interfaces = {}
  self.active_interfaces = {}
end

function InputInterfaceContainer:get_front_interface()
  if #self.active_interfaces > 0 then
    return self.active_interfaces[1]
  end
end

function InputInterfaceContainer:add_interface(interface)
  for i, ifc in pairs(self.interfaces) do
    if ifc == new_interface then
      return
    end
  end

  table.insert(self.interfaces, interface)
end

function InputInterfaceContainer:remove_interface(interface)
  for i, ifc in pairs(self.interfaces) do
    if ifc == interface then
      table.remove(self.interfaces, i)
      return
    end
  end
end

function InputInterfaceContainer:get_interface(interface_name)
  for i, interface in pairs(self.interfaces) do
    if interface:get_name() == interface_name then
      return interface
    end
  end
end

function InputInterfaceContainer:activate_interface(interface)
  for i, active_interface in pairs(self.active_interfaces) do
    if interface == active_interface then
      return
    end
  end

  for i, ifc in pairs(self.interfaces) do
    if ifc == interface then
      table.insert(self.active_interfaces, 0, ifc)
      return
    end
  end
end

function InputInterfaceContainer:deactivate_interface(interface)
  for i, active_interface in pairs(self.active_interfaces) do
    if interface == active_interface then
      table.remove(self.active_interfaces, i)
      return
    end
  end
end

function InputInterfaceContainer:get_interfaces()
  return self.interfaces
end

function InputInterfaceContainer:get_active_interfaces()
  return self.active_interfaces
end

return {InputInterfaceContainer = InputInterfaceContainer}

-- vi: et ts=2 sw=2

