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
local Widget = require('widget')

local TextWidget = Widget.Widget:new()

TextWidget.ALIGN_LEFT    = 0
TextWidget.ALIGN_CENTER  = 1
TextWidget.ALIGN_RIGHT   = 2
TextWidget.ALIGN_JUSTIFY = 3

TextWidget.ALIGN_TOP     = 0
TextWidget.ALIGN_BOTTOM  = 2

TextWidget.WRAP_NONE      = 0
TextWidget.WRAP_WORD      = 1
TextWidget.WRAP_CHAR      = 2
TextWidget.WRAP_WORD_CHAR = 3

TextWidget.ELLIPSIZE_NONE   = 0
TextWidget.ELLIPSIZE_START  = 1
TextWidget.ELLIPSIZE_MIDDLE = 2
TextWidget.ELLIPSIZE_END    = 3

function TextWidget:new(tw)
  tw = tw or {}
  
  tw.top_margin = tw.top_margin or 0
  tw.bottom_margin = tw.bottom_margin or 0
  tw.left_margin = tw.left_margin or 0
  tw.right_margin = tw.right_margin or 0
  tw.text = tw.text or ''
  tw.max_width = tw.max_width or 0
  tw.max_height = tw.max_height or 0
  tw.fg_color = tw.fg_color or {1.0, 1.0, 1.0, 1.0}
  tw.bg_color = tw.bg_color or {0.0, 0.0, 0.0, 0.0}
  tw.scrollable = tw.scrollable or false
  tw.font_description_text = tw.font_description_text or 
                             d2k.hud.font_description_text
  tw.use_markup = tw.use_markup or false

  tw.text_context = nil
  tw.current_render_context = nil
  tw.layout = nil
  tw.horizontal_offset = 0.0
  tw.vertical_offset = 0.0

  setmetatable(tw, self)
  self.__index = self

  tw:set_name('Text Widget')
  tw:build_layout()

  if tw.word_wrap then
    tw:set_word_wrap(tw.word_wrap)
  else
    tw:set_word_wrap(TextWidget.WRAP_NONE)
  end

  if tw.horizontal_alignment then
    tw:set_horizontal_alignment(tw.horizontal_alignment)
  else
    tw:set_horizontal_alignment(TextWidget.ALIGN_RIGHT)
  end

  if tw.vertical_alignment then
    tw:set_vertical_alignment(tw.vertical_alignment)
  else
    tw:set_vertical_alignment(TextWidget.ALIGN_TOP)
  end

  if tw.ellipsize then
    tw:set_ellipsize(tw.ellipsize)
  else
    tw:set_ellipsize(TextWidget.ELLIPSIZE_NONE)
  end

  return tw
end

function TextWidget:update_layout_if_needed()
  if self.needs_updating then
    if self.use_markup then
      self.layout:set_markup(self.text, -1)
    else
      self.layout:set_text(self.text, -1)
    end
    PangoCairo.update_context(
      d2k.overlay.render_context, d2k.overlay.text_context
    )
    PangoCairo.update_layout(d2k.overlay.render_context, self.layout)
    self:check_offsets()
  end
end

function TextWidget:draw()
  local cr = d2k.overlay.render_context
  local line_count = 0

  if not self.current_render_context or self.current_render_context ~= cr then
    self:build_layout()
  end

  self:update_layout_if_needed()

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

  local lx = self.x + self.left_margin
  local ly = self.y + self.top_margin
  local text_width = self.width - (self.left_margin + self.right_margin)
  local text_height = self.height - (self.top_margin + self.bottom_margin)
  local layout_width, layout_height = self.layout:get_pixel_size()
  local layout_ink_extents, layout_logical_extents =
    self.layout:get_pixel_extents()

  if self.vertical_alignment == TextWidget.ALIGN_CENTER then
    ly = ly + (text_height / 2) - (layout_height / 2)
  elseif self.vertical_alignment == TextWidget.ALIGN_BOTTOM then
    ly = ly + text_height - layout_height
  end

  if self.horizontal_alignment == TextWidget.ALIGN_CENTER then
    lx = (text_width / 2) - (layout_width / 2)
  elseif self.horizontal_alignment == TextWidget.ALIGN_RIGHT then
    lx = lx + text_width - layout_width
  end

  if layout_width > text_width then
    lx = lx - self.horizontal_offset
  end

  if layout_height > text_height then
    ly = ly - self.vertical_offset
  end

  local iter = self.layout:get_iter()
  local line_number = 1
  repeat
    local line = iter:get_line_readonly()
    local line_ink_extents, line_logical_extents = iter:get_line_extents()
    local line_logical_x = line_logical_extents.x / Pango.SCALE
    local line_logical_y = line_logical_extents.y / Pango.SCALE
    local line_baseline_pixels = iter:get_baseline() / Pango.SCALE
    local line_start_x = line_logical_x + lx
    local line_start_y = line_logical_y + ly
    local line_end_y = line_baseline_pixels + ly

    if line_end_y > 0 then
      cr:move_to(line_start_x, line_baseline_pixels + ly)
      PangoCairo.show_layout_line(cr, line)
    end

    if line_start_y >= text_height then
      break
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

function TextWidget:clear()
  self:set_text('')
end

function TextWidget:get_ellipsize()
  return self.ellipsize
end

function TextWidget:set_ellipsize(ellipsize)
  if ellipsize == TextWidget.ELLIPSIZE_NONE then
    self.ellipsize = TextWidget.ELLIPSIZE_NONE
    self.layout:set_ellipsize(Pango.EllipsizeMode.NONE)
  elseif ellipsize == TextWidget.ELLIPSIZE_START then
    self.ellipsize = TextWidget.ELLIPSIZE_START
    self.layout:set_ellipsize(Pango.EllipsizeMode.START)
  elseif ellipsize == TextWidget.ELLIPSIZE_MIDDLE then
    self.ellipsize = TextWidget.ELLIPSIZE_MIDDLE
    self.layout:set_ellipsize(Pango.EllipsizeMode.MIDDLE)
  elseif ellipsize == TextWidget.ELLIPSIZE_END then
    self.ellipsize = TextWidget.ELLIPSIZE_END
    self.layout:set_ellipsize(Pango.EllipsizeMode.END)
  else
    s = 'TextWidget:set_ellipsize: Invalid ellipsization value %d'
    error(s:format(ellipsize))
  end

  self.needs_updating = true
end

function TextWidget:is_ellipsize()
  return self.layout:is_ellipsized()
end

function TextWidget:get_word_wrap()
  return self.word_wrap
end

function TextWidget:set_word_wrap(word_wrap)
  if word_wrap == TextWidget.WRAP_NONE then
    self.word_wrap = TextWidget.WRAP_NONE
    self.layout:set_width(-1)
  elseif word_wrap == TextWidget.WRAP_WORD then
    self.word_wrap = TextWidget.WRAP_WORD
    self.layout:set_width(self.width * Pango.SCALE)
    self.layout:set_wrap(Pango.WrapMode.WORD)
  elseif word_wrap == TextWidget.WRAP_CHAR then
    self.word_wrap = TextWidget.WRAP_CHAR
    self.layout:set_width(self.width * Pango.SCALE)
    self.layout:set_wrap(Pango.WrapMode.CHAR)
  elseif word_wrap == TextWidget.WRAP_WORD_CHAR then
    self.word_wrap = TextWidget.WRAP_WORD_CHAR
    self.layout:set_width(self.width * Pango.SCALE)
    self.layout:set_wrap(Pango.WrapMode.WORD_CHAR)
  else
    s = 'TextWidget:set_word_wrap: Invalid word wrap value %d'
    error(s:format(word_wrap))
  end

  self.needs_updating = true
end

function TextWidget:is_wrapped()
  return self.layout:is_wrapped()
end

function TextWidget:get_horizontal_alignment()
  return self.horizontal_alignment
end

function TextWidget:set_horizontal_alignment(horizontal_alignment)
  if horizontal_alignment == TextWidget.ALIGN_LEFT then
    self.horizontal_alignment = TextWidget.ALIGN_LEFT
    self.layout:set_alignment(Pango.Alignment.LEFT)
  elseif horizontal_alignment == TextWidget.ALIGN_CENTER then
    self.horizontal_alignment = TextWidget.ALIGN_CENTER
    self.layout:set_alignment(Pango.Alignment.CENTER)
  elseif horizontal_alignment == TextWidget.ALIGN_RIGHT then
    self.horizontal_alignment = TextWidget.ALIGN_RIGHT
    self.layout:set_alignment(Pango.Alignment.RIGHT)
  elseif horizontal_alignment == TextWidget.ALIGN_JUSTIFY then
    self.horizontal_alignment = TextWidget.ALIGN_JUSTIFY
    self.layout:set_alignment(Pango.Alignment.JUSTIFY)
  else
    s = 'TextWidget:set_horizontal_alignment: Invalid horizontal alignment %d'
    error(s:format(horizontal_alignment))
  end

  self.needs_updating = true
end

function TextWidget:get_vertical_alignment()
  return self.vertical_alignment
end

function TextWidget:set_vertical_alignment(vertical_alignment)
  if vertical_alignment == TextWidget.ALIGN_TOP then
    self.vertical_alignment = TextWidget.ALIGN_TOP
  elseif vertical_alignment == TextWidget.ALIGN_CENTER then
    self.vertical_alignment = TextWidget.ALIGN_CENTER
  elseif vertical_alignment == TextWidget.ALIGN_BOTTOM then
    self.vertical_alignment = TextWidget.ALIGN_BOTTOM
  else
    s = 'TextWidget:set_vertical_alignment: Invalid vertical alignment %d'
    error(s:format(vertical_alignment))
  end

  self.needs_updating = true
end

function TextWidget:check_offsets()
  local text_width = self.width - (self.left_margin + self.right_margin)
  local text_height = self.height - (self.top_margin + self.bottom_margin)
  local layout_width, layout_height = self.layout:get_pixel_size()
  local min_x = 0
  local max_x = layout_width - text_width
  local x_delta = max_x - min_x
  local min_y = 0
  local max_y = layout_height - text_height
  local y_delta = max_y - min_y

  if self.horizontal_alignment == TextWidget.ALIGN_CENTER then
    min_x = -((layout_width - text_width) / 2) - self.left_margin
    max_x = ((layout_width - text_width) / 2)
  elseif self.horizontal_alignment == TextWidget.ALIGN_RIGHT then
    min_x = -(layout_width - text_width) - self.left_margin
    max_x = 0
  end

  if self.vertical_alignment == TextWidget.ALIGN_CENTER then
    min_y = -((layout_height - text_height) / 2) - self.top_margin
    max_y = ((layout_height - text_height) / 2)
  elseif self.vertical_alignment == TextWidget.ALIGN_BOTTOM then
    min_y = -(layout_height - text_height) - self.top_margin
    max_y = 0
  end

  if layout_width <= text_width then
    min_x = 0
    max_x = 0
  end

  if layout_height <= text_height then
    min_y = 0
    max_y = 0
  end

  if self.horizontal_offset < min_x then
    self.horizontal_offset = min_x
  end

  if self.horizontal_offset > max_x then
    self.horizontal_offset = max_x
  end

  if self.vertical_offset < min_y then
    self.vertical_offset = min_y
  end

  if self.vertical_offset > max_y then
    self.vertical_offset = max_y
  end

end

function TextWidget:get_horizontal_offset()
  return self.horizontal_offset
end

function TextWidget:set_horizontal_offset(horizontal_offset)
  self.horizontal_offset = horizontal_offset
  self:check_offsets()
end

function TextWidget:scroll_left(pixels)
  self.horizontal_offset = self.horizontal_offset - pixels
  self:check_offsets()
end

function TextWidget:scroll_right(pixels)
  self.horizontal_offset = self.horizontal_offset + pixels
  self:check_offsets()
end

function TextWidget:get_vertical_offset()
  return self.vertical_offset
end

function TextWidget:set_vertical_offset(vertical_offset)
  self.vertical_offset = vertical_offset
  self:check_offsets()
end

function TextWidget:scroll_up(pixels)
  self.vertical_offset = self.vertical_offset - pixels
  self:check_offsets()
end

function TextWidget:scroll_down(pixels)
  self.vertical_offset = self.vertical_offset + pixels
  self:check_offsets()
end

return {
  TextWidget       = TextWidget,
  ALIGN_LEFT       = TextWidget.ALIGN_LEFT,
  ALIGN_CENTER     = TextWidget.ALIGN_CENTER,
  ALIGN_RIGHT      = TextWidget.ALIGN_RIGHT,
  ALIGN_JUSTIFY    = TextWidget.ALIGN_JUSTIFY,
  ALIGN_TOP        = TextWidget.ALIGN_TOP,
  ALIGN_BOTTOM     = TextWidget.ALIGN_BOTTOM,
  WRAP_NONE        = TextWidget.WRAP_NONE,
  WRAP_WORD        = TextWidget.WRAP_WORD,
  WRAP_CHAR        = TextWidget.WRAP_CHAR,
  WRAP_WORD_CHAR   = TextWidget.WRAP_WORD_CHAR,
  ELLIPSIZE_NONE   = TextWidget.ELLIPSIZE_NONE,
  ELLIPSIZE_START  = TextWidget.ELLIPSIZE_START,
  ELLIPSIZE_MIDDLE = TextWidget.ELLIPSIZE_MIDDLE,
  ELLIPSIZE_END    = TextWidget.ELLIPSIZE_END,
}

-- vi: et ts=2 sw=2

