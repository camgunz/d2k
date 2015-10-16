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

InputInterface = class('InputInterface')

local SNAP_NONE = 0
local SNAP_RIGHT = 1
local SNAP_BOTTOM = 2

function InputInterface:initialize(ii)
  ii = ii or {}

  self.parent         = nil
  self.active         = false

  self.name                        = ii.name or 'Input Interface'
  self.x                           = ii.x or 0
  self.y                           = ii.y or 0
  self.width                       = ii.width or 0
  self.height                      = ii.height or 0
  self.max_width                   = ii.max_width or 0
  self.max_height                  = ii.max_height or 0
  self.z_index                     = ii.z_index or 0
  self.snap                        = ii.snap or SNAP_NONE
  self.use_proportional_dimensions = ii.use_proportional_dimensions or true
  self.fullscreen                  = ii.fullscreen or false
  self.position_changed            = false
  self.dimensions_changed          = false
  self.cached_render               = nil
end

function InputInterface:get_name()
  return self.name
end

function InputInterface:set_name(name)
  self.name = name
end

function InputInterface:get_x()
  if self:get_snap() == SNAP_RIGHT then
    return d2k.Video.get_screen_width() - self:get_width_in_pixels()
  end

  --[[
  if self.x < 0 then
    return d2k.Video.get_screen_width() + self.x
  end
  --]]

  return self.x
end

function InputInterface:set_x(x)
  if self:get_snap() == SNAP_RIGHT then
    error(string.format(
      '%s: Cannot set X coordinate when snapped to the right\n',
      self:get_name()
    ))
  end

  self.x = x

  self:handle_position_change()
end

function InputInterface:get_y()
  if self:get_snap() == SNAP_BOTTOM then
    return d2k.Video.get_screen_height() - self:get_height_in_pixels()
  end

  --[[
  if self.y < 0 then
    return d2k.Video.get_screen_height() + self.y
  end
  --]]

  return self.y
end

function InputInterface:set_y(y)
  if self:get_snap() == SNAP_BOTTOM then
    error(string.format(
      '%s: Cannot set Y coordinate when snapped to the bottom\n',
      self:get_name()
    ))
  end

  self.y = y

  self:handle_position_change()
end

function InputInterface:get_width()
  if self.width < 0 or self.width > 1 then
    error(string.format('%s: Invalid width %s\n', self:get_name(), self.width))
  end

  return self.width
end

function InputInterface:set_width(width)
  if width == self.width then
    return
  end

  if not self:get_use_proportional_dimensions() then
    self.width = width
    self:handle_dimension_change()
    return
  end

  if width < 0 or width > 1 then
    error(string.format('%s: Invalid width %s\n', self:get_name(), width))
  end

  self.width = width
  self:handle_dimension_change()
end

function InputInterface:get_width_in_pixels()
  if self:get_use_proportional_dimensions() then
    return math.floor(self.width * d2k.overlay:get_width())
  end

  return self.width
end

function InputInterface:set_width_in_pixels(width)
  if self:get_use_proportional_dimensions() then
    self.width = width / d2k.overlay:get_width()
  else
    self.width = width
  end

  self:handle_dimension_change()
end

function InputInterface:get_height()
  if self.height < 0 or self.height > 1 then
    error(string.format('%s: Invalid height %s\n',
      self:get_name(), self.height
    ))
  end

  return self.height
end

function InputInterface:set_height(height)
  if height == self.height then
    return
  end

  if not self:get_use_proportional_dimensions() then
    self.height = height
    self:handle_dimension_change()
    return
  end

  if height < 0 or height > 1 then
    error(string.format('%s: Invalid height %s\n',
      self:get_name(), height
    ))
  end

  self.height = height

  self:handle_dimension_change()
end

function InputInterface:get_height_in_pixels()
  if self:get_use_proportional_dimensions() then
    return math.floor(self.height * d2k.overlay:get_height())
  end

  return self.height
end

function InputInterface:set_height_in_pixels(height)
  if self:get_use_proportional_dimensions() then
    self.height = height / d2k.overlay:get_height()
  else
    self.height = height
  end

  self:handle_dimension_change()
end

function InputInterface:get_max_width()
  if not self:get_use_proportional_dimensions() then
    return self.max_width
  end

  if self.max_width < 0 or self.max_width > 1 then
    error(string.format('%s: Invalid max width %s\n',
      self:get_name(), self.max_width
    ))
  end

  if self.max_width == 0 then
    return d2k.overlay:get_width()
  end

  return d2k.overlay:get_width() * self.max_width
end

function InputInterface:set_max_width(max_width)
  if max_width == self.max_width then
    return
  end

  if not self:get_use_proportional_dimensions() then
    self.max_width = max_width
    self:handle_dimension_change()
  end

  if max_width < 0 or max_width > 1 then
    error(string.format('%s: Invalid max_width %s\n',
      self:get_name(), max_width
    ))
  end

  if max_width == 1 then
    self.max_width = dk2.overlay:get_width()
  elseif max_width > 1 then
    self.max_width = max_width / d2k.overlay:get_width()
  else
    self.max_width = max_width
  end

  self:handle_dimension_change()
end

function InputInterface:get_max_width_in_pixels()
  if self:get_use_proportional_dimensions() then
    return self.max_width * d2k.overlay:get_width()
  end

  return self.max_width
end

function InputInterface:set_max_width_in_pixels(max_width)
  if self:get_use_proportional_dimensions() then
    self.max_width = max_width / d2k.overlay:get_width()
  else
    self.max_width = max_width
  end

  self:handle_dimension_change()
end

function InputInterface:get_max_height()
  if not self:get_use_proportional_dimensions() then
    return self.max_height
  end

  if self.max_height < 0 or self.max_height > 1 then
    error(string.format('%s: Invalid max height %s\n',
      self:get_name(), self.max_height
    ))
  end

  if self.max_height == 0 then
    return d2k.overlay:get_height()
  end

  return d2k.overlay:get_height() * self.max_height
end

function InputInterface:set_max_height(max_height)
  if max_height == self.max_height then
    return
  end

  if not self:get_use_proportional_dimensions() then
    self.max_height = max_height
    self:handle_dimension_change()
  end

  if max_height < 0 or max_height > 1 then
    error(string.format('%s: Invalid max_height %s\n',
      self:get_name(), max_height
    ))
  end

  if max_height == 1 then
    self.max_height = dk2.overlay:get_height()
  elseif max_height > 1 then
    self.max_height = max_height / d2k.overlay:get_height()
  else
    self.max_height = max_height
  end

  self:handle_dimension_change()
end

function InputInterface:get_max_height_in_pixels()
  if self:get_use_proportional_dimensions() then
    return self.max_height * d2k.overlay:get_height()
  end

  return self.max_height
end

function InputInterface:set_max_height_in_pixels(max_height)
  if self:get_use_proportional_dimensions() then
    self.max_height = max_height / d2k.overlay:get_height()
  else
    self.max_height = max_height
  end

  self:handle_dimension_change()
end

function InputInterface:get_z_index()
  return self.z_index
end

function InputInterface:set_z_index(z_index)
  local parent = self:get_parent()

  self.z_index = z_index

  if parent then
    parent:sort_interfaces()
  end

  self:handle_position_change()
end

function InputInterface:get_snap()
  return self.snap
end

function InputInterface:set_snap(snap)
  self.snap = snap

  self:handle_position_change()
end

function InputInterface:get_use_proportional_dimensions()
  return self.use_proportional_dimensions
end

function InputInterface:set_use_proportional_dimensions(pd)
  self.use_proportional_dimensions = pd

  self:handle_dimension_change()
end

function InputInterface:is_fullscreen()
  return self.fullscreen
end

function InputInterface:set_fullscreen(fullscreen)
  self.fullscreen = fullscreen

  self:handle_position_change()
  self:handle_dimension_change()
end

function InputInterface:get_cached_render()
  return self.cached_render
end

function InputInterface:set_cached_render(render)
  self.cached_render = render
end

function InputInterface:activate()
  local parent = self:get_parent()

  self.active = true

  if parent then
    parent:activate(self)
  end
end

function InputInterface:deactivate()
  self.active = false
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

  self.parent = parent
end

function InputInterface:reset()
end

function InputInterface:tick()
end

function InputInterface:invalidate_render()
  self.cached_render = nil
end

function InputInterface:needs_rendering()
  if self.cached_render == nil then
    return true
  end

  return false
end

function InputInterface:begin_render()
  d2k.overlay.render_context:push_group_with_content(
    Cairo.Content.COLOR_ALPHA
  )
end

function InputInterface:render()
end

function InputInterface:get_render()
  if self:needs_rendering() then
    self:begin_render()
    self:render()
    self:end_render()
  end

  return self.cached_render
end

function InputInterface:end_render()
  self.cached_render = d2k.overlay.render_context:pop_group()
end

function InputInterface:handle_event(event)
end

function InputInterface:handle_overlay_built(overlay)
  self:reset()
end

function InputInterface:handle_overlay_destroyed(overlay)
end

function InputInterface:handle_position_change()
  self.position_changed = true
  self:invalidate_render()
end

function InputInterface:handle_dimension_change()
  self.dimensions_changed = true
  self:invalidate_render()
end

function InputInterface:handle_display_change()
  self.display_changed = true
  self:invalidate_render()
end

return {
  InputInterface = InputInterface,
  SNAP_NONE      = SNAP_NONE,
  SNAP_RIGHT     = SNAP_RIGHT,
  SNAP_BOTTOM    = SNAP_BOTTOM
}


-- vi: et ts=2 sw=2

