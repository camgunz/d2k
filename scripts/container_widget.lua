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

ContainerWidget = class('ContainerWidget', InputInterface.InputInterface)
ContainerWidget:include(InputInterfaceContainer.InputInterfaceContainer)

function ContainerWidget:initialize(cw)
  cw = cw or {}

  cw.name = cw.name or 'ContainerWidget'

  InputInterface.InputInterface.initialize(self, cw)

  self.interfaces = {}
end

function ContainerWidget:sort_interfaces()
  table.sort(self.interfaces, function(i1, i2)
    return i1:get_z_index() < i2:get_z_index()
  end)
end

function ContainerWidget:activate()
  self.active = true
end

function ContainerWidget:deactivate()
  self.active = false
end

function ContainerWidget:is_active()
  return self.active
end

function ContainerWidget:begin_render()
  d2k.overlay.render_context:push_group_with_content(
    Cairo.Content.COLOR_ALPHA
  )
end

function ContainerWidget:position_view()
end

function ContainerWidget:clip_view()
  local cr = d2k.overlay.render_context

  cr:rectangle(
    self:get_x(),
    self:get_y(),
    self:get_pixel_width(),
    self:get_pixel_height()
  )
end

function ContainerWidget:render()
  local cr = d2k.overlay.render_context
  local fg_color = self:get_fg_color()
  local bg_color = self:get_bg_color()

  cr:save()

  cr:set_operator(Cairo.Operator.OVER)

  cr:set_source_rgba(bg_color[1], bg_color[2], bg_color[3], bg_color[4])
  cr:paint()

  cr:restore()

  InputInterfaceContainer.InputInterfaceContainer.render(self)
end

function ContainerWidget:get_render()
  if self:needs_rendering() then
    self:begin_render()
    self:render()
    self:end_render()
  end

  return self.cached_render
end

function ContainerWidget:end_render()
  self.position_changed = false
  self.dimensions_changed = false
  self.display_changed = false
  self.cached_render = d2k.overlay.render_context:pop_group()
end

return {ContainerWidget = ContainerWidget}

-- vi: et ts=2 sw=2

