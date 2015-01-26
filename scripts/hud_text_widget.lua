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
local HUDWidget = require('hud_widget')

local TextWidget = HUDWidget.HUDWidget:new()

function TextWidget:new(tw)
  tw = tw or {}
  
  tw.x = tw.x or 0
  tw.y = tw.y or 0
  tw.width = tw.width or 0
  tw.height = tw.height or 0
  tw.top_margin = tw.top_margin or 0
  tw.bottom_margin = tw.bottom_margin or 0
  tw.left_margin = tw.left_margin or 0
  tw.right_margin = tw.right_margin or 0
  tw.text = tw.text or ''
  tw.max_width = tw.max_width or d2k.Video.get_screen_width()
  tw.max_height = tw.max_height or d2k.Video.get_screen_height()
  tw.fg_color = tw.fg_color or {1.0, 1.0, 1.0, 1.0}
  tw.bg_color = tw.bg_color or {0.0, 0.0, 0.0, 0.0}
  tw.scrollable = tw.scrollable or false
  tw.align_bottom = tw.align_bottom or false
  tw.font_description_text = tw.font_description_text or 
                             d2k.hud.font_description_text
  tw.word_wrap = tw.word_wrap or false
  tw.scroll_amount = tw.scroll_amount or false

  tw.text_context = nil
  tw.current_render_context = nil
  tw.layout = nil
  tw.layout_width = 0
  tw.layout_height = 0
  tw.needs_updating = false
  tw.offset = 0.0

  setmetatable(tw, self)
  self.__index = self

  tw:set_name('Text Widget')
  tw:build_layout()

  return tw
end

function TextWidget:reset()
end

function TextWidget:tick()
end

function TextWidget:draw()
  local cr = d2k.overlay.render_context
  local line_count = 0

  if not self.current_render_context or self.current_render_context ~= cr then
    self:build_layout()
  end

  if self.word_wrap then
    self.layout:set_width(self.width * Pango.SCALE)
  else
    self.layout:set_width(-1)
  end

  -- self.layout:set_height(self.height * Pango.SCALE)

  if self.needs_updating then
    self.layout:set_markup(self.text, -1)
    PangoCairo.update_context(cr, d2k.overlay.text_context)
    PangoCairo.update_layout(cr, self.layout)
  end

  line_count = self.layout:get_line_count()

  cr:save()

  cr:set_operator(Cairo.Operator.OVER)

  cr:reset_clip()
  cr:new_path()
  cr:rectangle(self.x, self.y, self.width, self.height)
  cr:clip()

  cr:set_source_rgba(
    self.bg_color[1],
    self.bg_color[2],
    self.bg_color[3],
    self.bg_color[4]
  )
  cr:paint()

  if line_count <= 0 then
    return
  end

  cr:reset_clip()
  cr:new_path()
  cr:rectangle(
    self.x + self.left_margin,
    self.y + self.top_margin,
    self.width - (self.left_margin + self.right_margin),
    self.height - (self.top_margin + self.bottom_margin)
  )
  cr:clip()

  cr:set_source_rgba(
    self.fg_color[1],
    self.fg_color[2],
    self.fg_color[3],
    self.fg_color[4]
  )

  local x = self.x + self.left_margin
  local y = self.y + self.top_margin
  local text_height = self.height - (self.top_margin + self.bottom_margin)
  local layout_width, layout_height = self.layout:get_pixel_size()
  local layout_ink_extents, layout_logical_extents = self.layout:get_pixel_extents()
  local layout_offset = (layout_height - text_height) - self.offset

  local iter = self.layout:get_iter()
  local line_number = 0
  local y_cutoff = layout_logical_extents.height - text_height

  repeat
    local line = iter:get_line_readonly()
    local line_ink_extents, line_logical_extents = iter:get_line_extents()
    local baseline = iter:get_baseline()
    local baseline_pixels = baseline / Pango.SCALE
    local line_x = x + (line_logical_extents.x / Pango.SCALE)
    local line_y = y + (line_logical_extents.y / Pango.SCALE) - layout_offset

    if baseline_pixels > y_cutoff or line_y < text_height then
      cr:move_to(
        x + (line_logical_extents.x / Pango.SCALE),
        y + (line_logical_extents.y / Pango.SCALE) - layout_offset
      )
      PangoCairo.show_layout_line(cr, line)

      if (baseline_pixels - layout_offset) > (self.height - self.bottom_margin) then
        break
      end
    end

    line_number = line_number + 1
  until not iter:next_line()

  cr:restore()
end

function TextWidget:build_layout()
  self.current_render_context = d2k.overlay.render_context
  self.layout = Pango.Layout.new(d2k.overlay.text_context)
  self.layout:set_font_description(Pango.FontDescription.from_string(
    self.font_description_text
  ))
end

function TextWidget:get_text()
  return self.text
end

function TextWidget:set_text(text)
  self.text = text
  self.needs_updating = true
end

function TextWidget:set_wrap_mode(mode)
  self.layout:set_wrap(mode)
end

function TextWidget:set_width(width)
  self.width = width
  self.needs_updating = true
end

function TextWidget:set_height(height)
  self.height = height
  self.needs_updating = true
end

function TextWidget:increase_offset()
  self.offset = math.min(self.height, self.offset + self.scroll_amount)
end

function TextWidget:decrease_offset()
  self.offset = math.max(0, self.offset - self.scroll_amount)
end

return {TextWidget = TextWidget}

-- vi: et ts=2 sw=2

