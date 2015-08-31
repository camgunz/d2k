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

  get_front_interface = function(self)
    if #self.interfaces > 0 then
      return self.interfaces[1]
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

  get_interfaces = function(self)
    return self.interfaces
  end,

  activate = function(self, interface)
    local parent = nil
    
    if self.get_parent then
      parent = self:get_parent()
    end

    for i, ifc in ipairs(self.interfaces) do
      if ifc == interface then
        table.remove(self.interfaces, i)
        table.insert(self.interfaces, 1, interface)
      end
    end

    if parent then
      parent:activate(self)
    end
  end,

  deactivate = function(self, interface)
    local parent = nil
    
    if self.get_parent then
      parent = self:get_parent()
    end

    for i, ifc in ipairs(self.interfaces) do
      if ifc == interface then
        table.remove(self.interfaces, i)
        table.insert(self.interfaces, interface)
      end
    end

    if parent then
      parent:deactivate(self)
    end
  end,

  reset = function(self)
    for i, interface in ipairs(self.interfaces) do
      interface:reset()
    end
  end,

  tick = function(self)
    for i, interface in ipairs(self.interfaces) do
      local worked, err = pcall(function()
        interface:tick()
      end)

      if not worked then
        print(err)
      end
    end
  end,

  draw = function(self)
    for i, interface in ipairs(self.interfaces) do
      interface:draw()
    end
  end,

  handle_event = function(self, event)
    for i, interface in ipairs(self.interfaces) do
      if interface:handle_event(event) then
        return
      end
      if interface:is_fullscreen() then
        return
      end
    end
  end,

}

return {InputInterfaceContainer = InputInterfaceContainer}

-- vi: et ts=2 sw=2

