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

Overlay = {}

local lgi = require('lgi')
local Cairo = lgi.cairo

function Overlay:new(o)
  o = o or {
    render_context = nil,
    render_surface = nil
  }
  setmetatable(o, self)
  self.__index = self

  o:build()

  return o
end

function Overlay:initialized()
  if self.render_context and self.render_surface then
    return true
  end

  return false
end

function Overlay:build()
  print('[!!] Building overlay')
  assert(not self:initialized(), 'overlay already built')

  d2k.build_overlay_pixels()

  self.render_surface = Cairo.ImageSurface.create_for_data(
    d2k.get_overlay_pixels(),
    Cairo.Format.RGB24, -- CG: FIXME: This is duplicated in i_video.c
    d2k.get_screen_width(),
    d2k.get_screen_height(),
    d2k.get_screen_stride()
  )

  local status = self.render_surface.status
  if status == Cairo.Status.SUCCESS then
    local cairo_error = Cairo.Status.to_string(status)
    error(string.format('Error creating overlay surface (%s)', cairo_error))
  end

  self.render_context = Cairo.Context.create(self.render_surface)

  local status = self.render_context.status
  if status == Cairo.Status.SUCCESS then
    local cairo_error = Cairo.Status.to_string(status)
    error(string.format('Error creating overlay surface (%s)', cairo_error))
  end

  if d2k.using_opengl() then
    d2k.build_overlay_texture()
  end

  d2k.clear_overlay_needs_resetting()
end

function Overlay:destroy()
  print('[!!] Destroying overlay')
  assert(self:initialized(), 'overlay already destroyed')

  self.render_context = nil
  self.render_surface = nil

  d2k.destroy_overlay_pixels()
  if d2k.using_opengl() then
    d2k.destroy_overlay_texture()
  end
end

function Overlay:reset()
  if self:initialized() then
    self:destroy()
  end

  self:build()
end

function Overlay:lock()
  if d2k.overlay_needs_resetting() then
    print('[!!] overlay needs resetting')
    self:reset()
  end

  d2k.lock_screen()
end

function Overlay:unlock()
  self.render_surface:flush()
  d2k.unlock_screen()
end

function Overlay:clear()
  self.render_context:set_operator(Cairo.OPERATOR_CLEAR)
  self.render_context:paint()
end

return {Overlay = Overlay}

-- vi: et ts=2 sw=2

