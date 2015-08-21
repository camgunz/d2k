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

class.InputInterface()

function InputInterface:__init(ii)
  ii = ii or {}

  self.name = ii.name or ''

  self.active = false
  self.parent = nil
  self.needs_updating = false
end

function InputInterface:get_name()
  return self.name
end

function InputInterface:set_name(name)
  self.name = name
end

function InputInterface:activate()
  local parent = self:get_parent()

  self.active = true

  if parent then
    parent:activate_interface(self)
  end
end

function InputInterface:deactivate()
  local parent = self:get_parent()

  self.active = false

  if parent then
    parent:deactivate_interface(self)
  end
end

function InputInterface:toggle()
  if self:is_active() then
    self:deactivate()
  else
    self:activate()
  end
end

function InputInterface:is_active()
  return self.active
end

function InputInterface:is_enabled()
  return self.parent ~= nil
end

function InputInterface:get_parent()
  return self.parent
end

function InputInterface:remove_parent()
  local current_parent = self:get_parent()

  if current_parent then
    self:get_parent():remove_interface(self)
    self.parent = nil
  end
end


function InputInterface:set_parent(parent)
  self:remove_parent()

  if parent then
    parent:add_interface(self)
  end
end

function InputInterface:get_needs_updating()
  return self.needs_updating
end

function InputInterface:set_needs_updating(needs_updating)
  self.needs_updating = needs_updating
end

function InputInterface:is_upfront()
  return self:get_parent() and self:get_parent():get_front_interface() == self
end

function InputInterface:reset()
end

function InputInterface:tick()
end

function InputInterface:draw()
end

function InputInterface:handle_event(event)
end

return {InputInterface = InputInterface}

-- vi: et ts=2 sw=2

