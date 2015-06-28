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
local GLib = lgi.GLib
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
  tw.max_width = tw.max_width or tw.width or 0
  tw.max_height = tw.max_height or tw.height or 0
  tw.fg_color = tw.fg_color or {1.0, 1.0, 1.0, 1.0}
  tw.bg_color = tw.bg_color or {0.0, 0.0, 0.0, 0.0}
  tw.outline_color = tw.outline_color or {0.0, 0.0, 0.0, 0.0}
  tw.outline_text = tw.outline_text or false
  tw.outline_width = tw.outline_width or 0
  tw.line_height = tw.line_height or 0
  tw.scrollable = tw.scrollable or false
  tw.font_description_text = tw.font_description_text or
                             d2k.hud.font_description_text
  tw.use_markup = tw.use_markup or false
  tw.strip_ending_newline = tw.strip_ending_newline or false

  tw.text_context = nil
  tw.current_render_context = nil
  tw.layout = nil

  tw.horizontal_offset = 0.0
  tw.vertical_offset = 0.0

  tw.get_external_text = nil
  tw.external_text_updated = nil
  tw.clear_external_text_updated = nil

  setmetatable(tw, self)
  self.__index = self

  tw:build_layout()

  if tw.word_wrap then
    tw:set_word_wrap(tw.word_wrap)
  else
    tw:set_word_wrap(TextWidget.WRAP_NONE)
  end

  if tw.horizontal_alignment then
    tw:set_horizontal_alignment(tw.horizontal_alignment)
  else
    tw:set_horizontal_alignment(TextWidget.ALIGN_LEFT)
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

function TextWidget:get_top_margin()
  return self.top_margin
end

function TextWidget:set_top_margin(top_margin)
  self.top_margin = top_margin
end

function TextWidget:get_bottom_margin()
  return self.bottom_margin
end

function TextWidget:set_bottom_margin(bottom_margin)
  self.bottom_margin = bottom_margin
end

function TextWidget:get_left_margin()
  return self.left_margin
end

function TextWidget:set_left_margin(left_margin)
  self.left_margin = left_margin
end

function TextWidget:get_right_margin()
  return self.right_margin
end

function TextWidget:set_right_margin(right_margin)
  self.right_margin = right_margin
end

function TextWidget:get_text()
  return self.text
end

function TextWidget:set_text(text)
  if self.get_external_text then
    error(string.format("%s: Can't set text when displaying external text",
      self.name
    ))
  end
  self.text = text
  self.needs_updating = true
end

function TextWidget:get_max_width()
  return self.max_width
end

function TextWidget:set_max_width(max_width)
  self.max_width = max_width
end

function TextWidget:get_max_height()
  return self.max_height
end

function TextWidget:set_max_height(max_height)
  self.max_height = max_height
end

function TextWidget:get_fg_color()
  return self.fg_color
end

function TextWidget:set_fg_color(fg_color)
  self.fg_color = fg_color
end

function TextWidget:get_bg_color()
  return self.bg_color
end

function TextWidget:set_bg_color(bg_color)
  self.bg_color = bg_color
end

function TextWidget:get_outline_color()
  return self.outline_color
end

function TextWidget:set_outline_color(outline_color)
  self.outline_color = outline_color
end

function TextWidget:get_outline_text()
  return self.outline_text
end

function TextWidget:set_outline_text(outline_text)
  self.outline_text = outline_text
end

function TextWidget:get_outline_width()
  return self.outline_width
end

function TextWidget:set_outline_width(outline_width)
  self.outline_width = outline_width
end

function TextWidget:get_line_height()
  return self.line_height
end

function TextWidget:set_line_height(line_height)
  self.line_height = line_height
end

function TextWidget:get_scrollable()
  return self.scrollable
end

function TextWidget:set_scrollable(scrollable)
  self.scrollable = scrollable
end

function TextWidget:get_font_description_text()
  return self.font_description_text
end

function TextWidget:set_font_description_text(font_description_text)
  self.font_description_text = font_description_text
end

function TextWidget:get_use_markup()
  return self.use_markup
end

function TextWidget:set_use_markup(use_markup)
  self.use_markup = use_markup
end

function TextWidget:get_strip_ending_newline()
  return self.strip_ending_newline
end

function TextWidget:set_strip_ending_newline(strip_ending_newline)
  self.strip_ending_newline = strip_ending_newline
end

function TextWidget:get_text_context()
  return self.text_context
end

function TextWidget:set_text_context(text_context)
  self.text_context = text_context
end

function TextWidget:get_current_render_context()
  return self.current_render_context
end

function TextWidget:set_current_render_context(current_render_context)
  self.current_render_context = current_render_context
end

function TextWidget:get_layout()
  return self.layout
end

function TextWidget:set_layout(layout)
  self.layout = layout
end

function TextWidget:get_horizontal_offset()
  return self.horizontal_offset
end

function TextWidget:set_horizontal_offset(horizontal_offset)
  self.horizontal_offset = horizontal_offset
  self:check_offsets()
end

function TextWidget:get_vertical_offset()
  return self.vertical_offset
end

function TextWidget:set_vertical_offset(vertical_offset)
  self.vertical_offset = vertical_offset
  self:check_offsets()
end

function TextWidget:update_layout_if_needed()
  if not self.needs_updating then
    return
  end

  local text = self:get_text()

  if self:get_strip_ending_newline() then
    while true do
      local text_length = #text

      if text:sub(text_length, text_length) ~= '\n' then
        break
      end

      text = text:sub(1, text_length - 1)
    end
  end

  if self:get_use_markup() then
    self:get_layout():set_markup(text, -1)
  else
    self:get_layout():set_text(text, -1)
  end

  PangoCairo.update_context(
    d2k.overlay.render_context, d2k.overlay.text_context
  )
  PangoCairo.update_layout(d2k.overlay.render_context, self:get_layout())
  self:check_offsets()

  if self.get_external_text then
    local layout_width, layout_height = self:get_layout():get_pixel_size()

    self:set_height(layout_height)
  end

  self.needs_updating = false
end

function get_line_y(layout_rect, line_rect)
  return layout_rect.height + line_rect.y
end

function TextWidget:print_lines()
  local iter = self:get_layout():get_iter()

  repeat
    local line = iter:get_line_readonly()
    local ink_rect, line_rect = iter:get_line_extents()

    line_rect.x      = line_rect.x / Pango.SCALE
    line_rect.y      = line_rect.y / Pango.SCALE
    line_rect.width  = line_rect.width / Pango.SCALE
    line_rect.height = line_rect.height / Pango.SCALE

    print(string.format('{%s, %s, %s, %s}',
      line_rect.x,
      line_rect.y,
      line_rect.width,
      line_rect.height
    ))
  until not iter:next_line()
end

function TextWidget:tick()
  local current_ms = d2k.System.get_ticks()

  if self.get_external_text and self.external_text_updated() then
    self.text = self.get_external_text()
    self.needs_updating = true
    self.clear_external_text_updated()
  end
end

function TextWidget:draw()
  local cr = d2k.overlay.render_context
  local line_count = 0
  local fg_color = self:get_fg_color()
  local bg_color = self:get_bg_color()
  local outline_color = self:get_outline_color()
  local current_render_context = self:get_current_render_context()

  if not current_render_context or current_render_context ~= cr then
    self:build_layout()
  end

  self:update_layout_if_needed()

  local lw, lh = self:get_layout():get_pixel_size()

  line_count = self:get_layout():get_line_count()

  cr:save()

  cr:set_operator(Cairo.Operator.OVER)

  cr:reset_clip()
  cr:new_path()
  cr:rectangle(self:get_x(), self:get_y(), self:get_width(), self:get_height())
  cr:clip()

  cr:set_source_rgba(bg_color[1], bg_color[2], bg_color[3], bg_color[4])
  cr:paint()

  if line_count <= 0 then
    return
  end

  cr:set_source_rgba(fg_color[1], fg_color[2], fg_color[3], fg_color[4])

  local lx = self:get_x() + self:get_left_margin()
  local ly = self:get_y() + self:get_top_margin()
  local text_width = self:get_width() - (
    self:get_left_margin() + self:get_right_margin()
  )
  local text_height = self:get_height() - (
    self:get_top_margin() + self:get_bottom_margin()
  )
  local layout_width, layout_height = self:get_layout():get_pixel_size()
  local layout_ink_extents, layout_logical_extents =
    self:get_layout():get_pixel_extents()

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

  local iter = self:get_layout():get_iter()
  local line_number = 1
  local min_line = 0
  local max_line = 0
  local start_y_offset = 0
  local rendered_at_least_one_line = false
  local line_height = self:get_line_height()

  if line_height > 0 then
    if line_count <= line_height then
      min_line = 1
      max_line = line_count
    elseif self.vertical_alignment == TextWidget.ALIGN_TOP then
      min_line = 1
      max_line = line_height
    elseif self.vertical_alignment == TextWidget.ALIGN_CENTER then
      local half_lines = line_count / 2
      local half_line_height = line_height / 2

      min_line = math.floor(half_lines - half_line_height)
      max_line = math.floor(half_lines + half_line_height)

      while ((max_line - min_line) + 1) < line_height do
        max_line = max_line + 1
      end
    else
      min_line = (line_count - line_height) + 1
      max_line = line_count
    end
  end

  -- if self:get_name() == 'messages' then
  --   print(string.format('Drawing messages at %s/%s', lx, ly))
  -- end

  repeat
    local line_baseline_pixels = iter:get_baseline() / Pango.SCALE
    local line_end_y = line_baseline_pixels + ly
    local should_render_line = false
    local should_quit_after_rendering = false

    if line_height > 0 then
      if line_number >= min_line and line_number <= max_line then
        should_render_line = true
      end
    elseif line_end_y > 0 then
      should_render_line = true
    end

    if should_render_line then
      local line = iter:get_line_readonly()
      local line_ink_extents, line_logical_extents = iter:get_line_extents()
      local line_logical_x = line_logical_extents.x / Pango.SCALE
      local line_logical_y = line_logical_extents.y / Pango.SCALE
      local line_logical_height = line_logical_extents.height / Pango.SCALE
      local line_start_x = line_logical_x + lx
      local line_start_y = line_logical_y + ly

      if line_height > 0 then
        if line_number == max_line then
          should_quit_after_rendering = true
        end
      elseif line_start_y >= text_height then
        should_quit_after_rendering = true
      end

      if line_height > 0 and
         (not rendered_at_least_one_line) and
         line_logical_y > 0 then
        start_y_offset = -line_logical_y
      end

      rendered_at_least_one_line = true

      cr:move_to(line_start_x, line_baseline_pixels + ly + start_y_offset)

      if self:get_outline_text() then
        cr:save()

        PangoCairo.layout_line_path(cr, line)

        cr:save()
        cr:set_line_width(self:get_outline_width())
        cr:set_source_rgba(
          outline_color[1],
          outline_color[2],
          outline_color[3],
          outline_color[4]
        )
        cr:stroke_preserve()
        cr:restore()

        cr:set_source_rgba(fg_color[1], fg_color[2], fg_color[3], fg_color[4])
        cr:fill_preserve()

        cr:restore()
      else
        PangoCairo.show_layout_line(cr, line)
      end

      if should_quit_after_rendering then
        break
      end
    elseif self:get_name() == 'messages' then
      local line = iter:get_line_readonly()

      print(string.format('Not rendering line [%s]',
        self.text:sub(line.start_index + 1, line.length + 1)
      ))
    end

    line_number = line_number + 1
  until not iter:next_line()

  cr:restore()
end

function TextWidget:build_layout()
  self:set_current_render_context(d2k.overlay.render_context)
  self:set_layout(Pango.Layout.new(d2k.overlay.text_context))

  local layout = self:get_layout()

  self:get_layout():set_font_description(Pango.FontDescription.from_string(
    self:get_font_description_text()
  ))

  local layout_ink_extents, layout_logical_extents = layout:get_pixel_extents()
end

function TextWidget:set_height_by_lines(line_count)
  self:set_line_height(line_count)
end

function TextWidget:set_external_text_source(get_text,
                                             text_updated,
                                             clear_text_updated)
  self.get_external_text = get_text
  self.external_text_updated = text_updated
  self.clear_external_text_updated = clear_text_updated
end

function TextWidget:get_text()
  if self.get_external_text then
    return self.get_external_text()
  else
    return self.text
  end
end

function TextWidget:clear()
  if self.get_external_text then
    error(string.format("%s: Can't clear text when displaying external text",
      self.name
    ))
  end
  self:set_text('')
end

function TextWidget:get_ellipsize()
  return self.ellipsize
end

function TextWidget:set_ellipsize(ellipsize)
  local layout = self:get_layout()

  if ellipsize == TextWidget.ELLIPSIZE_NONE then
    self.ellipsize = TextWidget.ELLIPSIZE_NONE
    layout:set_ellipsize(Pango.EllipsizeMode.NONE)
  elseif ellipsize == TextWidget.ELLIPSIZE_START then
    self.ellipsize = TextWidget.ELLIPSIZE_START
    layout:set_ellipsize(Pango.EllipsizeMode.START)
  elseif ellipsize == TextWidget.ELLIPSIZE_MIDDLE then
    self.ellipsize = TextWidget.ELLIPSIZE_MIDDLE
    layout:set_ellipsize(Pango.EllipsizeMode.MIDDLE)
  elseif ellipsize == TextWidget.ELLIPSIZE_END then
    self.ellipsize = TextWidget.ELLIPSIZE_END
    layout:set_ellipsize(Pango.EllipsizeMode.END)
  else
    s = 'TextWidget:set_ellipsize: Invalid ellipsization value %s'
    error(s:format(ellipsize))
  end

  self.needs_updating = true
end

function TextWidget:is_ellipsize()
  return self:get_layout():is_ellipsized()
end

function TextWidget:get_word_wrap()
  return self.word_wrap
end

function TextWidget:set_word_wrap(word_wrap)
  local layout = self:get_layout()

  if word_wrap == TextWidget.WRAP_NONE then
    self.word_wrap = TextWidget.WRAP_NONE
    layout:set_width(-1)
  elseif word_wrap == TextWidget.WRAP_WORD then
    self.word_wrap = TextWidget.WRAP_WORD
    layout:set_width(self:get_width() * Pango.SCALE)
    layout:set_wrap(Pango.WrapMode.WORD)
  elseif word_wrap == TextWidget.WRAP_CHAR then
    self.word_wrap = TextWidget.WRAP_CHAR
    layout:set_width(self:get_width() * Pango.SCALE)
    layout:set_wrap(Pango.WrapMode.CHAR)
  elseif word_wrap == TextWidget.WRAP_WORD_CHAR then
    self.word_wrap = TextWidget.WRAP_WORD_CHAR
    layout:set_width(self:get_width() * Pango.SCALE)
    layout:set_wrap(Pango.WrapMode.WORD_CHAR)
  else
    s = 'TextWidget:set_word_wrap: Invalid word wrap value %s'
    error(s:format(word_wrap))
  end

  self.needs_updating = true
end

function TextWidget:is_wrapped()
  return self:get_layout():is_wrapped()
end

function TextWidget:get_horizontal_alignment()
  return self.horizontal_alignment
end

function TextWidget:set_horizontal_alignment(horizontal_alignment)
  local layout = self:get_layout()

  if horizontal_alignment == TextWidget.ALIGN_LEFT then
    self.horizontal_alignment = TextWidget.ALIGN_LEFT
    layout:set_alignment(Pango.Alignment.LEFT)
  elseif horizontal_alignment == TextWidget.ALIGN_CENTER then
    self.horizontal_alignment = TextWidget.ALIGN_CENTER
    layout:set_alignment(Pango.Alignment.CENTER)
  elseif horizontal_alignment == TextWidget.ALIGN_RIGHT then
    self.horizontal_alignment = TextWidget.ALIGN_RIGHT
    layout:set_alignment(Pango.Alignment.RIGHT)
  elseif horizontal_alignment == TextWidget.ALIGN_JUSTIFY then
    self.horizontal_alignment = TextWidget.ALIGN_JUSTIFY
    layout:set_alignment(Pango.Alignment.JUSTIFY)
  else
    s = 'TextWidget:set_horizontal_alignment: Invalid horizontal alignment %s'
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
    s = 'TextWidget:set_vertical_alignment: Invalid vertical alignment %s'
    error(s:format(vertical_alignment))
  end

  self.needs_updating = true
end

function TextWidget:check_offsets()
  local text_width = self:get_width() - (
    self:get_left_margin() + self:get_right_margin()
  )
  local text_height = self:get_height() - (
    self:get_top_margin() + self:get_bottom_margin()
  )
  local layout_width, layout_height = self:get_layout():get_pixel_size()
  local min_x = 0
  local max_x = layout_width - text_width
  local x_delta = max_x - min_x
  local min_y = 0
  -- local max_y = layout_height - text_height
  local max_y = text_height - self:get_top_margin()
  local y_delta = max_y - min_y

  if self.horizontal_alignment == TextWidget.ALIGN_CENTER then
    min_x = -((layout_width - text_width) / 2) - self:get_left_margin()
    max_x = ((layout_width - text_width) / 2)
  elseif self.horizontal_alignment == TextWidget.ALIGN_RIGHT then
    min_x = -(layout_width - text_width) - self:get_left_margin()
    max_x = 0
  end

  if self.vertical_alignment == TextWidget.ALIGN_CENTER then
    min_y = -((layout_height - text_height) / 2) - self:get_top_margin()
    max_y = ((layout_height - text_height) / 2)
  elseif self.vertical_alignment == TextWidget.ALIGN_BOTTOM then
    min_y = -(layout_height - text_height) - self:get_top_margin()
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

  -- if self:get_name() == 'messages' then
  --   print(string.format('check_offsets: min_y/max_y/vertical_offset/lh/th: %s, %s, %s, %s, %s',
  --     min_y,
  --     max_y,
  --     self:get_vertical_offset(),
  --     layout_height,
  --     text_height
  --   ))
  -- end

  if self:get_horizontal_offset() < min_x then
    self.horizontal_offset = min_x
  end

  if self:get_horizontal_offset() > max_x then
    self.horizontal_offset = max_x
  end

  if self:get_vertical_offset() < min_y then
    self.vertical_offset = min_y
  end

  if self:get_vertical_offset() > max_y then
    self.vertical_offset = max_y
  end

  if self:get_name() == 'messages' then
    print(string.format('check_offsets 2: min_y/max_y/vertical_offset: %s, %s, %s',
      min_y,
      max_y,
      self:get_vertical_offset()
    ))
  end
end

function TextWidget:scroll_left(pixels)
  self:set_horizontal_offset(self:get_horizontal_offset() - pixels)
  self:check_offsets()
end

function TextWidget:scroll_right(pixels)
  self:set_horizontal_offset(self:get_horizontal_offset() + pixels)
  self:check_offsets()
end

function TextWidget:scroll_up(pixels)
  self:set_vertical_offset(self:get_vertical_offset() - pixels)
end

function TextWidget:scroll_down(pixels)
  self:set_vertical_offset(self:get_vertical_offset() + pixels)
end

function TextWidget:write(text)
  if self.get_external_text then
    error(string.format("%s: Can't write text when displaying external text",
      self.name
    ))
  end

  if d2k.Video.is_enabled() then
    self:set_text(self:get_text() .. GLib.markup_escape_text(text, -1))
  else
    d2k.System.print(text)
  end
end

function TextWidget:mwrite(markup)
  if self.get_external_text then
    error(string.format("%s: Can't write markup when displaying external text",
      self.name
    ))
  end

  if self:get_use_markup() then
    self:set_text(self:get_text() .. markup)
  else
    self:write(markup)
  end
end

function TextWidget:echo(text)
  if self.get_external_text then
    error(string.format("%s: Can't echo text when displaying external text",
      self.name
    ))
  end

  self:set_text(self:get_text() .. GLib.markup_escape_text(text, -1) .. '\n')
end

function TextWidget:mecho(markup)
  if self.get_external_text then
    error(string.format("%s: Can't echo markup when displaying external text",
      self.name
    ))
  end

  if self:get_use_markup() then
    self:set_text(self:get_text() .. markup .. '\n')
  else
    self:echo(markup)
  end
end

return {
  TextWidget         = TextWidget,
  ALIGN_LEFT         = TextWidget.ALIGN_LEFT,
  ALIGN_CENTER       = TextWidget.ALIGN_CENTER,
  ALIGN_RIGHT        = TextWidget.ALIGN_RIGHT,
  ALIGN_JUSTIFY      = TextWidget.ALIGN_JUSTIFY,
  ALIGN_TOP          = TextWidget.ALIGN_TOP,
  ALIGN_BOTTOM       = TextWidget.ALIGN_BOTTOM,
  WRAP_NONE          = TextWidget.WRAP_NONE,
  WRAP_WORD          = TextWidget.WRAP_WORD,
  WRAP_CHAR          = TextWidget.WRAP_CHAR,
  WRAP_WORD_CHAR     = TextWidget.WRAP_WORD_CHAR,
  ELLIPSIZE_NONE     = TextWidget.ELLIPSIZE_NONE,
  ELLIPSIZE_START    = TextWidget.ELLIPSIZE_START,
  ELLIPSIZE_MIDDLE   = TextWidget.ELLIPSIZE_MIDDLE,
  ELLIPSIZE_END      = TextWidget.ELLIPSIZE_END,
}

-- vi: et ts=2 sw=2

