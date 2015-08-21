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
local InputInterface = require('input_interface')

class.Widget(InputInterface.InputInterface)

Widget.SNAP_NONE = 0
Widget.SNAP_RIGHT = 1
Widget.SNAP_BOTTOM = 2

function Widget.sort_by_z_index(w1, w2)
  if w1:get_z_index() < w2:get_z_index() then
    return true
  end

  return false
end

function Widget:__init(w)
  w = w or {}

  w.name = w.name or 'Widget'

  self.InputInterface:__init(w)

  self.x = w.x or 0
  self.y = w.y or 0
  self.snap = w.snap or Widget.SNAP_NONE
  self.width = w.width or 0
  self.height = w.height or 0
  self.max_width = w.max_width or 0
  self.max_height = w.max_height or 0
  self.z_index = w.z_index or 0
  self.use_proportional_dimensions = w.use_proportional_dimensions or true
end

function Widget:get_x()
  if self:get_snap() == Widget.SNAP_RIGHT then
    return d2k.Video.get_screen_width() - self:get_width_in_pixels()
  end

  if self.x < 0 then
    return d2k.Video.get_screen_width() + self.x
  end

  return self.x
end

function Widget:set_x(x)
  if self:get_snap() == Widget.SNAP_RIGHT then
    error(string.format(
      '%s: Cannot set X coordinate when snapped to the right\n',
      self:get_name()
    ))
  end

  self.x = x
end

function Widget:get_y()
  if self:get_snap() == Widget.SNAP_BOTTOM then
    return d2k.Video.get_screen_height() - self:get_height_in_pixels()
  end

  if self.y < 0 then
    return d2k.Video.get_screen_height() + self.y
  end

  return self.y
end

function Widget:set_y(y)
  if self:get_snap() == Widget.SNAP_BOTTOM then
    error(string.format(
      '%s: Cannot set Y coordinate when snapped to the bottom\n',
      self:get_name()
    ))
  end

  self.y = y
end

function Widget:get_snap()
  return self.snap
end

function Widget:set_snap(snap)
  self.snap = snap
end

function Widget:get_width()
  if self.width < 0 or self.width > 1 then
    error(string.format('%s: Invalid width %s\n', self:get_name(), self.width))
  end

  return self.width
end

function Widget:set_width(width)
  if width == self.width then
    return
  end

  if not self:get_use_proportional_dimensions() then
    self.width = width
    self:set_needs_updating(true)
    return
  end

  if width < 0 or width > 1 then
    error(string.format('%s: Invalid width %s\n', self:get_name(), width))
  end

  self.width = width
  self:set_needs_updating(true)
end

function Widget:get_width_in_pixels()
  if self:get_use_proportional_dimensions() then
    return math.floor(self.width * d2k.overlay:get_width())
  end

  return self.width
end

function Widget:set_width_in_pixels(width)
  if self:get_use_proportional_dimensions() then
    self.width = width / d2k.overlay:get_width()
  else
    self.width = width
  end

  self:set_needs_updating(true)
end

function Widget:get_height()
  if self.height < 0 or self.height > 1 then
    error(string.format('%s: Invalid height %s\n',
      self:get_name(), self.height
    ))
  end

  return self.height
end

function Widget:set_height(height)
  if height == self.height then
    return
  end

  if not self:get_use_proportional_dimensions() then
    self.height = height
    self:set_needs_updating(true)
    return
  end

  if height < 0 or height > 1 then
    error(string.format('%s: Invalid height %s\n',
      self:get_name(), height
    ))
  end

  self.height = height
  self:set_needs_updating(true)
end

function Widget:get_height_in_pixels()
  if self:get_use_proportional_dimensions() then
    return math.floor(self.height * d2k.overlay:get_height())
  end

  return self.height
end

function Widget:set_height_in_pixels(height)
  if self:get_use_proportional_dimensions() then
    self.height = height / d2k.overlay:get_height()
  else
    self.height = height
  end

  self:set_needs_updating(true)
end

function Widget:get_max_width()
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

function Widget:set_max_width(max_width)
  if max_width == self.max_width then
    return
  end

  if not self:get_use_proportional_dimensions() then
    self.max_width = max_width
    self:set_needs_updating(true)
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

  self:set_needs_updating(true)
end

function Widget:get_max_width_in_pixels()
  if self:get_use_proportional_dimensions() then
    return self.max_width * d2k.overlay:get_width()
  end

  return self.max_width
end

function Widget:set_max_width_in_pixels(max_width)
  if self:get_use_proportional_dimensions() then
    self.max_width = max_width / d2k.overlay:get_width()
  else
    self.max_width = max_width
  end

  self:set_needs_updating(true)
end

function Widget:get_max_height()
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

function Widget:set_max_height(max_height)
  if max_height == self.max_height then
    return
  end

  if not self:get_use_proportional_dimensions() then
    self.max_height = max_height
    self:set_needs_updating(true)
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

  self:set_needs_updating(true)
end

function Widget:get_max_height_in_pixels()
  if self:get_use_proportional_dimensions() then
    return self.max_height * d2k.overlay:get_height()
  end

  return self.max_height
end

function Widget:set_max_height_in_pixels(max_height)
  if self:get_use_proportional_dimensions() then
    self.max_height = max_height / d2k.overlay:get_height()
  else
    self.max_height = max_height
  end

  self:set_needs_updating(true)
end

function Widget:get_z_index()
  return self.z_index
end

function Widget:set_z_index(z_index)
  self.z_index = z_index
  if self.parent then
    self.parent:sort_widgets()
  end
end

function Widget:get_use_proportional_dimensions()
  return self.use_proportional_dimensions
end

function Widget:set_use_proportional_dimensions(use_proportional_dimensions)
  self.use_proportional_dimensions = proportional_dimensions
end

function Widget:remove_parent()
  local current_parent = self:get_parent()

  InputInterface.InputInterface.remove_parent(self)

  if current_parent then
    current_parent:sort_widgets()
  end
end

function Widget:set_parent(parent)
  InputInterface.InputInterface.set_parent(self, parent)

  if self:get_parent() then
    self:get_parent():sort_widgets()
  end
end

function Widget:handle_overlay_built(overlay)
  self:reset()
end

function Widget:handle_overlay_destroyed(overlay)
end

function Widget:handle_event(event)
end

return {Widget = Widget}

-- vi: et ts=2 sw=2

