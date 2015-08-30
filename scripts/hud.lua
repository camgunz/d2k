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
local Widget = require('widget')

HUD = class('HUD', InputInterface.InputInterface)
HUD:include(InputInterfaceContainer.InputInterfaceContainer)

function HUD:initialize(h)
  h = h or {}

  h.name = h.name or 'HUD'
  InputInterface.InputInterface.initialize(self, h)

  self.widgets = {}
  self.widgets_by_z_index = {}

  self.interfaces = self.widgets
  self.active_interfaces = self.widgets_by_z_index

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

function HUD:add_interface(interface)
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
  table.sort(self.widgets, Widget.sort_by_z_index)
  table.sort(self.widgets_by_z_index, Widget.sort_by_z_index)
end

function HUD:reset()
  for i, w in pairs(self.widgets) do
    w:reset()
  end
end

function HUD:tick()
  for i, w in pairs(self.widgets) do
    local worked, err = pcall(function()
      w:tick()
    end)

    if not worked then
      print(err)
    end
  end
end

function HUD:draw()
  d2k.overlay:lock()

  if d2k.Video.using_opengl() then
    d2k.overlay:clear()
  end

  d2k.overlay.render_context:set_operator(Cairo.Operator.OVER)

  for i, w in pairs(self.widgets_by_z_index) do
    w:draw()
  end

  d2k.overlay:unlock()
end

function HUD:handle_event(event)
  for i, w in ipairs(self.widgets_by_z_index) do
    if w:handle_event(event) then
      return true
    end
  end

  return false
end

return {HUD = HUD}

-- vi: et ts=2 sw=2

