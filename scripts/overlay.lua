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

local lgi = require('lgi')
local Cairo = lgi.cairo
local Pango = lgi.Pango
local PangoCairo = lgi.PangoCairo

local Overlay = {}

function Overlay:new(o)
  o = o or {}

  o.build_listeners = {}
  o.destroy_listeners = {}

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
  assert(not self:initialized(), 'overlay already built')

  self.width = d2k.Video.get_screen_width()
  self.height = d2k.Video.get_screen_height()

  d2k.Video.build_overlay_pixels()

  self.render_surface = Cairo.ImageSurface.create_for_data(
    d2k.Video.get_overlay_pixels(),
    Cairo.Format.RGB24, -- CG: FIXME: This is duplicated in i_video.c
    self.width,
    self.height,
    d2k.Video.get_screen_stride()
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

  if d2k.Video.using_opengl() then
    d2k.Video.build_overlay_texture()
  end

  d2k.Video.clear_overlay_needs_resetting()

  local font_options = Cairo.FontOptions.create()
  local font_map = PangoCairo.FontMap.get_default()

  self.text_context = font_map:create_context()
  -- CG: FIXME: DPI should be part of a config file
  self.text_context:set_resolution(96.0)
  font_options:set_hint_style(Cairo.HINT_STYLE_FULL)
  font_options:set_hint_metrics(Cairo.HINT_METRICS_ON)
  font_options:set_antialias(Cairo.ANTIALIAS_SUBPIXEL)
  self.text_context:set_font_options(font_options)
  self.render_context:update_context(self.text_context)

  for i, l in pairs(self.build_listeners) do
    l.handle_overlay_built(self)
  end
end

function Overlay:destroy()
  assert(self:initialized(), 'overlay already destroyed')

  self.render_context = nil
  self.render_surface = nil

  d2k.Video.destroy_overlay_pixels()
  if d2k.Video.using_opengl() then
    d2k.Video.destroy_overlay_texture()
  end

  for i, l in pairs(self.destroy_listeners) do
    l.handle_overlay_destroyed(self)
  end
end

function Overlay:reset()
  if self:initialized() then
    self:destroy()
  end

  self:build()
end

function Overlay:lock()
  if d2k.Video.overlay_needs_resetting() then
    self:reset()
  end

  d2k.Video.lock_screen()
end

function Overlay:unlock()
  self.render_surface:flush()
  d2k.Video.unlock_screen()
end

function Overlay:clear()
  self.render_context:set_operator(Cairo.OPERATOR_CLEAR)
  self.render_context:paint()
end

function Overlay:get_width()
  return self.width
end

function Overlay:get_height()
  return self.height
end

function Overlay:add_build_listener(listener)
  table.insert(self.build_listeners, listener)
end

function Overlay:add_destroy_listener(listener)
  table.insert(self.destroy_listeners, listener)
end

return {Overlay = Overlay}

-- vi: et ts=2 sw=2

