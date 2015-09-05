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
local lgi = require('lgi')
local Cairo = lgi.cairo

local InputInterface = require('input_interface')
local InputInterfaceContainer = require('input_interface_container')

HUD = class('HUD', InputInterface.InputInterface)
HUD:include(InputInterfaceContainer.InputInterfaceContainer)

function HUD:initialize(h)
  h = h or {}

  h.name = h.name or 'HUD'

  InputInterface.InputInterface.initialize(self, h)

  self.interfaces = {}

  self.widgets = self.interfaces
  self.widgets_by_z_index = {}

  self.font_description_text = h.font_description_text or
                                 'Noto Sans,Arial Unicode MS,Unifont 11'
end

function HUD:handle_overlay_built(overlay)
  self:reset()

  for i, w in pairs(self.widgets) do
    w:handle_overlay_built()
  end
end

function HUD:handle_overlay_destroyed(overlay)
  for i, w in pairs(self.widgets) do
    w:handle_overlay_destroyed()
  end
end

function HUD:add_interface(interface)
  InputInterfaceContainer.InputInterfaceContainer.add_interface(
    self, interface
  )
  self:sort_widgets()
end

function HUD:remove_interface(interface)
  InputInterfaceContainer.InputInterfaceContainer.remove_interface(
    self, interface
  )
  self:sort_widgets()
end

function HUD:add_widget(widget)
  self:add_interface(widget)
end

function HUD:remove_widget(widget)
  self:remove_interface(widget)
end

function HUD:sort_widgets()
  self.widgets_by_z_index = {}

  for i = 1, #self.widgets do
    self.widgets_by_z_index[i] = self.widgets[i]
  end

  table.sort(self.widgets_by_z_index, function(w1, w2)
    if w1:get_z_index() < w2:get_z_index() then
      return -1
    elseif w2:get_z_index() < w1:get_z_index() then
      return 1
    end

    return 0
  end)
end

function HUD:draw()
  for i, w in ipairs(self.widgets_by_z_index) do
    w:draw()
  end
end

function HUD:handle_event(event)
  for i, w in ipairs(self.widgets) do
    if w:handle_event(event) then
      return true
    end
  end

  return false
end

return {HUD = HUD}

-- vi: et ts=2 sw=2

