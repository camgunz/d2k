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

local Widget = {}

function Widget:new(w)
  w = w or {}

  w.x = w.x or 0
  w.y = w.y or 0
  w.width = w.width or 0
  w.height = w.height or 0
  w.max_width = w.max_width or 0
  w.max_height = w.max_height or 0
  w.z_index = w.z_index or 0
  w.use_proportional_dimensions = w.use_proportional_dimensions or true

  w.hud = nil
  w.enabled = false
  w.active = false
  w.needs_updating = false

  setmetatable(w, self)
  self.__index = self

  w:set_name(w.name or 'Widget')
  w:deactivate()

  return w
end

function Widget:get_name()
  return self.name
end

function Widget:set_name(name)
  self.name = name
end

function Widget:get_x()
  return self.x
end

function Widget:set_x(x)
  self.x = x
end

function Widget:get_y()
  return self.y
end

function Widget:set_y(y)
  self.y = y
end

function Widget:get_width()
  if not self:get_use_proportional_dimensions() then
    return self.width
  end

  if self.width > 1 or self.width < 0 then
    error(string.format('%s: Invalid width %d\n', self:get_name(), self.width))
  end

  if self.width == 1 then
    return d2k.overlay:get_width()
  end

  return d2k.overlay:get_width() * self.width
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
    error(string.format('%s: Invalid width %d\n', self:get_name(), width))
  end

  if width == 1 then
    self.width = dk2.overlay:get_width()
  elseif width > 1 then
    self.width = width / d2k.overlay:get_width()
  else
    self.width = width
  end

  self:set_needs_updating(true)
end

function Widget:get_width_in_pixels()
  if self:get_use_proportional_dimensions() then
    return self.width * d2k.overlay:get_width()
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
  if not self:get_use_proportional_dimensions() then
    return self.height
  end

  if self.height < 0 or self.height > 1 then
    error(string.format('%s: Invalid height %f\n',
      self:get_name(), self.height
    ))
  end

  return d2k.overlay:get_height() * self.height
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

  if self.height < 0 or self.height > 1 then
    error(string.format('%s: Invalid height %d\n',
      self:get_name(), self.height
    ))
  end

  if height == 1 then
    self.height = d2k.overlay:get_height()
  elseif height > 1 then
    self.height = height / d2k.overlay:get_height()
  else
    self.height = height
  end

  self:set_needs_updating(true)
end

function Widget:get_height_in_pixels()
  if self:get_use_proportional_dimensions() then
    return self.height * d2k.overlay:get_height()
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
    error(string.format('%s: Invalid max width %d\n',
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
    error(string.format('%s: Invalid max_width %d\n',
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
    error(string.format('%s: Invalid max height %d\n',
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
    error(string.format('%s: Invalid max_height %d\n',
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
  d2k.hud:sort_widgets()
end

function Widget:get_use_proportional_dimensions()
  return self.use_proportional_dimensions
end

function Widget:set_use_proportional_dimensions(use_proportional_dimensions)
  self.use_proportional_dimensions = proportional_dimensions
end

function Widget:get_hud()
  return self.hud
end

function Widget:set_hud(hud)
  self.hud = hud
end

function Widget:enable()
  self.enabled = true
end

function Widget:disable()
  self.enabled = false
end

function Widget:is_enabled()
  return self.enabled
end

function Widget:activate()
  self.active = true
end

function Widget:deactivate()
  self.active = false
end

function Widget:is_active()
  return self.active
end

function Widget:get_needs_updating()
  return self.needs_updating
end

function Widget:set_needs_updating(needs_updating)
  self.needs_updating = needs_updating
end

function Widget:reset()
end

function Widget:tick()
end

function Widget:draw()
end

function Widget:handle_add(hud)
  self:enable()
  hud:sort_widgets()
end

function Widget:handle_remove(hud)
  self:disable()
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

