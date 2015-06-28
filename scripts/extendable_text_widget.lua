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

local ExtendableTextWidget = Widget.Widget:new()

ExtendableTextWidget.ALIGN_LEFT    = 0
ExtendableTextWidget.ALIGN_CENTER  = 1
ExtendableTextWidget.ALIGN_RIGHT   = 2
ExtendableTextWidget.ALIGN_JUSTIFY = 3

ExtendableTextWidget.ALIGN_TOP     = 0
ExtendableTextWidget.ALIGN_BOTTOM  = 2

ExtendableTextWidget.WRAP_NONE      = 0
ExtendableTextWidget.WRAP_WORD      = 1
ExtendableTextWidget.WRAP_CHAR      = 2
ExtendableTextWidget.WRAP_WORD_CHAR = 3

ExtendableTextWidget.ELLIPSIZE_NONE   = 0
ExtendableTextWidget.ELLIPSIZE_START  = 1
ExtendableTextWidget.ELLIPSIZE_MIDDLE = 2
ExtendableTextWidget.ELLIPSIZE_END    = 3

ExtendableTextWidget.RETRACT_NONE  = 0
ExtendableTextWidget.RETRACT_UP    = 1
ExtendableTextWidget.RETRACT_DOWN  = 2
ExtendableTextWidget.RETRACT_LEFT  = 3
ExtendableTextWidget.RETRACT_RIGHT = 4

ExtendableTextWidget.RETRACTION_TIME = 2000
ExtendableTextWidget.RETRACTION_TIMEOUT = 2000

function ExtendableTextWidget:new(tw)
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
  tw.retractable = tw.retractable or ExtendableTextWidget.RETRACT_NONE
  tw.retraction_time = tw.retraction_time or ExtendableTextWidget.RETRACTION_TIME
  tw.retraction_timeout = tw.retraction_timeout or ExtendableTextWidget.RETRACTION_TIMEOUT

  tw.text_context = nil
  tw.current_render_context = nil
  tw.layout = nil

  tw.horizontal_offset = 0.0
  tw.vertical_offset = 0.0

  tw.last_retraction_time = 0
  tw.currently_retracting = false

  tw.get_external_text = nil
  tw.external_text_updated = nil
  tw.clear_external_text_updated = nil

  setmetatable(tw, self)
  self.__index = self

  tw:build_layout()

  if tw.word_wrap then
    tw:set_word_wrap(tw.word_wrap)
  else
    tw:set_word_wrap(ExtendableTextWidget.WRAP_NONE)
  end

  if tw.horizontal_alignment then
    tw:set_horizontal_alignment(tw.horizontal_alignment)
  else
    tw:set_horizontal_alignment(ExtendableTextWidget.ALIGN_LEFT)
  end

  if tw.vertical_alignment then
    tw:set_vertical_alignment(tw.vertical_alignment)
  else
    tw:set_vertical_alignment(ExtendableTextWidget.ALIGN_TOP)
  end

  if tw.ellipsize then
    tw:set_ellipsize(tw.ellipsize)
  else
    tw:set_ellipsize(ExtendableTextWidget.ELLIPSIZE_NONE)
  end

  return tw
end

function ExtendableTextWidget:get_top_margin()
  return self.top_margin
end

function ExtendableTextWidget:set_top_margin(top_margin)
  self.top_margin = top_margin
end

function ExtendableTextWidget:get_bottom_margin()
  return self.bottom_margin
end

function ExtendableTextWidget:set_bottom_margin(bottom_margin)
  self.bottom_margin = bottom_margin
end

function ExtendableTextWidget:get_left_margin()
  return self.left_margin
end

function ExtendableTextWidget:set_left_margin(left_margin)
  self.left_margin = left_margin
end

function ExtendableTextWidget:get_right_margin()
  return self.right_margin
end

function ExtendableTextWidget:set_right_margin(right_margin)
  self.right_margin = right_margin
end

function ExtendableTextWidget:get_text()
  return self.text
end

function ExtendableTextWidget:reset_retraction()
  if not self:get_retractable() then
    return
  end

  self:set_last_retraction_time(d2k.System.get_ticks())
  self:set_currently_retracting(false)
end

function ExtendableTextWidget:set_text(text)
  self.text = text

  if self:currently_retracting() then
    local line = self:get_first_retractable_line()
    local retraction_direction = self:get_retracting()

    if retraction_direction == ExtendableTextWidget.RETRACT_UP then
      self:set_vertical_offset(line.y)
    elseif retraction_direction == ExtendableTextWidget.RETRACT_DOWN then
      self:set_vertical_offset(line.y - line.height)
    end
  end

  self:reset_retraction()
end

function ExtendableTextWidget:get_max_width()
  return self.max_width
end

function ExtendableTextWidget:set_max_width(max_width)
  self.max_width = max_width
end

function ExtendableTextWidget:get_max_height()
  return self.max_height
end

function ExtendableTextWidget:set_max_height(max_height)
  self.max_height = max_height
end

function ExtendableTextWidget:get_fg_color()
  return self.fg_color
end

function ExtendableTextWidget:set_fg_color(fg_color)
  self.fg_color = fg_color
end

function ExtendableTextWidget:get_bg_color()
  return self.bg_color
end

function ExtendableTextWidget:set_bg_color(bg_color)
  self.bg_color = bg_color
end

function ExtendableTextWidget:get_outline_color()
  return self.outline_color
end

function ExtendableTextWidget:set_outline_color(outline_color)
  self.outline_color = outline_color
end

function ExtendableTextWidget:get_outline_text()
  return self.outline_text
end

function ExtendableTextWidget:set_outline_text(outline_text)
  self.outline_text = outline_text
end

function ExtendableTextWidget:get_outline_width()
  return self.outline_width
end

function ExtendableTextWidget:set_outline_width(outline_width)
  self.outline_width = outline_width
end

function ExtendableTextWidget:get_line_height()
  return self.line_height
end

function ExtendableTextWidget:set_line_height(line_height)
  self.line_height = line_height
end

function ExtendableTextWidget:get_scrollable()
  return self.scrollable
end

function ExtendableTextWidget:set_scrollable(scrollable)
  self.scrollable = scrollable
end

function ExtendableTextWidget:get_font_description_text()
  return self.font_description_text
end

function ExtendableTextWidget:set_font_description_text(font_description_text)
  self.font_description_text = font_description_text
end

function ExtendableTextWidget:get_use_markup()
  return self.use_markup
end

function ExtendableTextWidget:set_use_markup(use_markup)
  self.use_markup = use_markup
end

function ExtendableTextWidget:get_strip_ending_newline()
  return self.strip_ending_newline
end

function ExtendableTextWidget:set_strip_ending_newline(strip_ending_newline)
  self.strip_ending_newline = strip_ending_newline
end

function ExtendableTextWidget:get_retractable()
  if self.retractable == ExtendableTextWidget.RETRACT_NONE then
    return false
  end

  return self.retractable
end

function ExtendableTextWidget:set_retractable(retractable)
  if retractable == false then
    self.retractable = ExtendableTextWidget.RETRACT_NONE
  else
    self.retractable = retractable
  end
end

function ExtendableTextWidget:get_retraction_time()
  return self.retraction_time
end

function ExtendableTextWidget:set_retraction_time(retraction_time)
  self.retraction_time = retraction_time
end

function ExtendableTextWidget:get_retraction_timeout()
  return self.retraction_timeout
end

function ExtendableTextWidget:set_retraction_timeout(retraction_timeout)
  self.retraction_timeout = retraction_timeout
end

function ExtendableTextWidget:get_text_context()
  return self.text_context
end

function ExtendableTextWidget:set_text_context(text_context)
  self.text_context = text_context
end

function ExtendableTextWidget:get_current_render_context()
  return self.current_render_context
end

function ExtendableTextWidget:set_current_render_context(current_render_context)
  self.current_render_context = current_render_context
end

function ExtendableTextWidget:get_layout()
  return self.layout
end

function ExtendableTextWidget:set_layout(layout)
  self.layout = layout
end

function ExtendableTextWidget:get_horizontal_offset()
  return self.horizontal_offset
end

function ExtendableTextWidget:set_horizontal_offset(horizontal_offset)
  self.horizontal_offset = horizontal_offset
  self:check_offsets()
end

function ExtendableTextWidget:get_vertical_offset()
  return self.vertical_offset
end

function ExtendableTextWidget:set_vertical_offset(vertical_offset)
  self.vertical_offset = vertical_offset
  self:check_offsets()
end

function ExtendableTextWidget:get_last_retraction_time()
  return self.last_retraction_time
end

function ExtendableTextWidget:set_last_retraction_time(last_retraction_time)
  self.last_retraction_time = last_retraction_time
end

function ExtendableTextWidget:get_currently_retracting()
  return self.currently_retracting
end

function ExtendableTextWidget:set_currently_retracting(currently_retracting)
  self.currently_retracting = currently_retracting
end

function ExtendableTextWidget:update_layout_if_needed()
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

function ExtendableTextWidget:get_first_retractable_line()
  local layout = self:get_layout()
  local horizontal_alignment = self:get_horizontal_alignment()
  local vertical_alignment = self:get_vertical_alignment()
  local line_x = self:get_x()
  local line_y = self:get_y()

  if #self.text == 0 then
    return nil
  end

  if horizontal_alignment == ExtendableTextWidget.ALIGN_LEFT then
    line_x = line_x + self:get_left_margin()
  elseif horizontal_alignment == ExtendableTextWidget.ALIGN_CENTER then
    line_x = line_x + (
      (self:get_width() + self:get_left_margin() - self:get_right_margin()) / 2
    )
  elseif horizontal_alignment == ExtendableTextWidget.ALIGN_RIGHT then
    line_x = line_x + (self:get_width() - self:get_right_margin())
  end

  if vertical_alignment == ExtendableTextWidget.ALIGN_UP then
    line_y = line_y + self:get_top_margin()
  elseif vertical_alignment == ExtendableTextWidget.ALIGN_CENTER then
    line_y = line_y + (
      (self:get_height() + self:get_top_margin() - self:get_bottom_margin()) / 2
    )
  elseif vertical_alignment == ExtendableTextWidget.ALIGN_BOTTOM then
    line_y = line_y + (self:get_height() - self:get_bottom_margin())
  end

  if retractable == ExtendableTextWidget.RETRACT_UP then
    line_y = line_y + self:get_vertical_offset()
  elseif retractable == ExtendableTextWidget.RETRACT_DOWN then
    line_y = line_y - self:get_vertical_offset()
  elseif retractable == ExtendableTextWidget.RETRACT_LEFT then
    line_x = line_x + self:get_horizontal_offset()
  elseif retractable == ExtendableTextWidget.RETRACT_RIGHT then
    line_x = line_x - self:get_horizontal_offset()
  end

  local iter = self:get_layout():get_iter()

  repeat
    local line = iter:get_line_readonly()
    local ink_rect, line_rect = iter:get_line_extents()

    line_rect.x      = line_rect.x / Pango.SCALE
    line_rect.y      = line_rect.y / Pango.SCALE
    line_rect.width  = line_rect.width / Pango.SCALE
    line_rect.height = line_rect.height / Pango.SCALE

    if line_x >= line_rect.x and line_x <= (line_rect.x + line_rect.width) and
       line_y >= line_rect.y and line_y <= (line_rect.y + line_rect.height) and
       line_rect.y + line_rect.height > self:get_vertical_offset() then
      -- print(string.format('HO, VO, TM, BM: %s, %s, %s, %s',
      --   self:get_horizontal_offset(),
      --   self:get_vertical_offset(),
      --   self:get_top_margin(),
      --   self:get_bottom_margin()
      -- ))
      -- print(string.format('First retractable line {%s, %s, %s, %s} (%sx%s)',
      --   line_rect.x,
      --   line_rect.y,
      --   line_rect.width,
      --   line_rect.height,
      --   line_x,
      --   line_y
      -- ))

      return line_rect
    end

  until not iter:next_line()
end

function ExtendableTextWidget:print_lines()
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

function ExtendableTextWidget:get_retracting()
  if not self:get_retractable() then
    return false
  end

  if #self.text == 0 then
    return false
  end

  local line = self:get_first_retractable_line()

  if line == nil then
    return false
  end

  local last_retraction_time = self:get_last_retraction_time()

  if last_retraction_time == 0 then
    return false
  end

  local retraction_ms_elapsed = current_ms - self:get_last_retraction_time()
  local retraction_timeout = self:get_retraction_timeout()

  if retraction_ms_elapsed < retraction_timeout then
    return false
  end

  return true
end

function ExtendableTextWidget:tick()
  local current_ms = d2k.System.get_ticks()
  local retractable = self:get_retractable()

  if self.get_external_text and self.external_text_updated() then
    self.text = self.get_external_text()
    self.needs_updating = true
    self.clear_external_text_updated()
  end

  if not retractable then
    return
  end

  if true then
    return
  end

  if #self.text == 0 then
    return
  end

  local line = self:get_first_retractable_line()

  if line == nil then
    self:set_last_retraction_time(current_ms)
    return
  end

  local last_retraction_time = self:get_last_retraction_time()

  if last_retraction_time == 0 then
    self:set_last_retraction_time(current_ms)
    return
  end

  local retraction_ms_elapsed = current_ms - self:get_last_retraction_time()
  local retraction_timeout = self:get_retraction_timeout()

  if retraction_ms_elapsed < retraction_timeout then
    return
  end

  local ms_retracted = retraction_ms_elapsed - retraction_timeout
  local height_fraction = ms_retracted / self:get_retraction_time()
  local y_delta = (ms_retracted / self:get_retraction_time()) * line.height

  self:set_currently_retracting(true)

  y_delta = math.min(y_delta, line.height + .1)

  self:set_vertical_offset(line.y + y_delta)

  if y_delta >= line.height then
    self:reset_retraction()
  end
end

function ExtendableTextWidget:draw()
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
  local retractable = self:get_retractable()
  local horizontally_retractable = retractable == (
    ExtendableTextWidget.RETRACT_LEFT or ExtendableTextWidget.RETRACT_RIGHT
  )
  local vertically_retractable = retractable == (
    ExtendableTextWidget.RETRACT_UP or ExtendableTextWidget.RETRACT_DOWN
  )

  if self.vertical_alignment == ExtendableTextWidget.ALIGN_CENTER then
    ly = ly + (text_height / 2) - (layout_height / 2)
  elseif self.vertical_alignment == ExtendableTextWidget.ALIGN_BOTTOM then
    ly = ly + text_height - layout_height
  end

  if self.horizontal_alignment == ExtendableTextWidget.ALIGN_CENTER then
    lx = (text_width / 2) - (layout_width / 2)
  elseif self.horizontal_alignment == ExtendableTextWidget.ALIGN_RIGHT then
    lx = lx + text_width - layout_width
  end

  if horizontally_retractable or layout_width > text_width then
    lx = lx - self:get_horizontal_offset()
  end

  if vertically_retractable or layout_height > text_height then
    ly = ly - self:get_vertical_offset()
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
    elseif self.vertical_alignment == ExtendableTextWidget.ALIGN_TOP then
      min_line = 1
      max_line = line_height
    elseif self.vertical_alignment == ExtendableTextWidget.ALIGN_CENTER then
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
    end

    line_number = line_number + 1
  until not iter:next_line()

  cr:restore()
end

function ExtendableTextWidget:build_layout()
  self:set_current_render_context(d2k.overlay.render_context)
  self:set_layout(Pango.Layout.new(d2k.overlay.text_context))

  local layout = self:get_layout()

  self:get_layout():set_font_description(Pango.FontDescription.from_string(
    self:get_font_description_text()
  ))

  local layout_ink_extents, layout_logical_extents = layout:get_pixel_extents()
end

function ExtendableTextWidget:set_height_by_lines(line_count)
  self:set_line_height(line_count)
end

function ExtendableTextWidget:set_external_text_source(get_text,
                                             text_updated,
                                             clear_text_updated)
  self.get_external_text = get_text
  self.external_text_updated = text_updated
  self.clear_external_text_updated = clear_text_updated
end

function ExtendableTextWidget:get_text()
  if self.get_external_text then
    return self.get_external_text()
  else
    return self.text
  end
end

function ExtendableTextWidget:set_text(text)
  if self.get_external_text then
    error(string.format("%s: Can't set text when displaying external text",
      self.name
    ))
  end
  self.text = text
  self.needs_updating = true
end

function ExtendableTextWidget:clear()
  if self.get_external_text then
    error(string.format("%s: Can't clear text when displaying external text",
      self.name
    ))
  end
  self:set_text('')
end

function ExtendableTextWidget:get_ellipsize()
  return self.ellipsize
end

function ExtendableTextWidget:set_ellipsize(ellipsize)
  local layout = self:get_layout()

  if ellipsize == ExtendableTextWidget.ELLIPSIZE_NONE then
    self.ellipsize = ExtendableTextWidget.ELLIPSIZE_NONE
    layout:set_ellipsize(Pango.EllipsizeMode.NONE)
  elseif ellipsize == ExtendableTextWidget.ELLIPSIZE_START then
    self.ellipsize = ExtendableTextWidget.ELLIPSIZE_START
    layout:set_ellipsize(Pango.EllipsizeMode.START)
  elseif ellipsize == ExtendableTextWidget.ELLIPSIZE_MIDDLE then
    self.ellipsize = ExtendableTextWidget.ELLIPSIZE_MIDDLE
    layout:set_ellipsize(Pango.EllipsizeMode.MIDDLE)
  elseif ellipsize == ExtendableTextWidget.ELLIPSIZE_END then
    self.ellipsize = ExtendableTextWidget.ELLIPSIZE_END
    layout:set_ellipsize(Pango.EllipsizeMode.END)
  else
    s = 'ExtendableTextWidget:set_ellipsize: Invalid ellipsization value %s'
    error(s:format(ellipsize))
  end

  self.needs_updating = true
end

function ExtendableTextWidget:is_ellipsize()
  return self:get_layout():is_ellipsized()
end

function ExtendableTextWidget:get_word_wrap()
  return self.word_wrap
end

function ExtendableTextWidget:set_word_wrap(word_wrap)
  local layout = self:get_layout()

  if word_wrap == ExtendableTextWidget.WRAP_NONE then
    self.word_wrap = ExtendableTextWidget.WRAP_NONE
    layout:set_width(-1)
  elseif word_wrap == ExtendableTextWidget.WRAP_WORD then
    self.word_wrap = ExtendableTextWidget.WRAP_WORD
    layout:set_width(self:get_width() * Pango.SCALE)
    layout:set_wrap(Pango.WrapMode.WORD)
  elseif word_wrap == ExtendableTextWidget.WRAP_CHAR then
    self.word_wrap = ExtendableTextWidget.WRAP_CHAR
    layout:set_width(self:get_width() * Pango.SCALE)
    layout:set_wrap(Pango.WrapMode.CHAR)
  elseif word_wrap == ExtendableTextWidget.WRAP_WORD_CHAR then
    self.word_wrap = ExtendableTextWidget.WRAP_WORD_CHAR
    layout:set_width(self:get_width() * Pango.SCALE)
    layout:set_wrap(Pango.WrapMode.WORD_CHAR)
  else
    s = 'ExtendableTextWidget:set_word_wrap: Invalid word wrap value %s'
    error(s:format(word_wrap))
  end

  self.needs_updating = true
end

function ExtendableTextWidget:is_wrapped()
  return self:get_layout():is_wrapped()
end

function ExtendableTextWidget:get_horizontal_alignment()
  return self.horizontal_alignment
end

function ExtendableTextWidget:set_horizontal_alignment(horizontal_alignment)
  local layout = self:get_layout()

  if horizontal_alignment == ExtendableTextWidget.ALIGN_LEFT then
    self.horizontal_alignment = ExtendableTextWidget.ALIGN_LEFT
    layout:set_alignment(Pango.Alignment.LEFT)
  elseif horizontal_alignment == ExtendableTextWidget.ALIGN_CENTER then
    self.horizontal_alignment = ExtendableTextWidget.ALIGN_CENTER
    layout:set_alignment(Pango.Alignment.CENTER)
  elseif horizontal_alignment == ExtendableTextWidget.ALIGN_RIGHT then
    self.horizontal_alignment = ExtendableTextWidget.ALIGN_RIGHT
    layout:set_alignment(Pango.Alignment.RIGHT)
  elseif horizontal_alignment == ExtendableTextWidget.ALIGN_JUSTIFY then
    self.horizontal_alignment = ExtendableTextWidget.ALIGN_JUSTIFY
    layout:set_alignment(Pango.Alignment.JUSTIFY)
  else
    s = 'ExtendableTextWidget:set_horizontal_alignment: Invalid horizontal alignment %s'
    error(s:format(horizontal_alignment))
  end

  self.needs_updating = true
end

function ExtendableTextWidget:get_vertical_alignment()
  return self.vertical_alignment
end

function ExtendableTextWidget:set_vertical_alignment(vertical_alignment)
  if vertical_alignment == ExtendableTextWidget.ALIGN_TOP then
    self.vertical_alignment = ExtendableTextWidget.ALIGN_TOP
  elseif vertical_alignment == ExtendableTextWidget.ALIGN_CENTER then
    self.vertical_alignment = ExtendableTextWidget.ALIGN_CENTER
  elseif vertical_alignment == ExtendableTextWidget.ALIGN_BOTTOM then
    self.vertical_alignment = ExtendableTextWidget.ALIGN_BOTTOM
  else
    s = 'ExtendableTextWidget:set_vertical_alignment: Invalid vertical alignment %s'
    error(s:format(vertical_alignment))
  end

  self.needs_updating = true
end

function ExtendableTextWidget:check_offsets()
  if self:get_retractable() then
    return
  end

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

  if self.horizontal_alignment == ExtendableTextWidget.ALIGN_CENTER then
    min_x = -((layout_width - text_width) / 2) - self:get_left_margin()
    max_x = ((layout_width - text_width) / 2)
  elseif self.horizontal_alignment == ExtendableTextWidget.ALIGN_RIGHT then
    min_x = -(layout_width - text_width) - self:get_left_margin()
    max_x = 0
  end

  if self.vertical_alignment == ExtendableTextWidget.ALIGN_CENTER then
    min_y = -((layout_height - text_height) / 2) - self:get_top_margin()
    max_y = ((layout_height - text_height) / 2)
  elseif self.vertical_alignment == ExtendableTextWidget.ALIGN_BOTTOM then
    min_y = -(layout_height - text_height) - self:get_top_margin()
    max_y = 0
  end

  if not self:get_retractable() then
    if layout_width <= text_width then
      min_x = 0
      max_x = 0
    end

    if layout_height <= text_height then
      min_y = 0
      max_y = 0
    end
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

function ExtendableTextWidget:scroll_left(pixels)
  self:set_horizontal_offset(self:get_horizontal_offset() - pixels)
  self:check_offsets()
end

function ExtendableTextWidget:scroll_right(pixels)
  self:set_horizontal_offset(self:get_horizontal_offset() + pixels)
  self:check_offsets()
end

function ExtendableTextWidget:scroll_up(pixels)
  self:set_vertical_offset(self:get_vertical_offset() - pixels)
end

function ExtendableTextWidget:scroll_down(pixels)
  self:set_vertical_offset(self:get_vertical_offset() + pixels)
end

function ExtendableTextWidget:write(text)
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

function ExtendableTextWidget:mwrite(markup)
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

function ExtendableTextWidget:echo(text)
  if self.get_external_text then
    error(string.format("%s: Can't echo text when displaying external text",
      self.name
    ))
  end

  self:set_text(self:get_text() .. GLib.markup_escape_text(text, -1) .. '\n')
end

function ExtendableTextWidget:mecho(markup)
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
  ExtendableTextWidget         = ExtendableTextWidget,
  ALIGN_LEFT         = ExtendableTextWidget.ALIGN_LEFT,
  ALIGN_CENTER       = ExtendableTextWidget.ALIGN_CENTER,
  ALIGN_RIGHT        = ExtendableTextWidget.ALIGN_RIGHT,
  ALIGN_JUSTIFY      = ExtendableTextWidget.ALIGN_JUSTIFY,
  ALIGN_TOP          = ExtendableTextWidget.ALIGN_TOP,
  ALIGN_BOTTOM       = ExtendableTextWidget.ALIGN_BOTTOM,
  WRAP_NONE          = ExtendableTextWidget.WRAP_NONE,
  WRAP_WORD          = ExtendableTextWidget.WRAP_WORD,
  WRAP_CHAR          = ExtendableTextWidget.WRAP_CHAR,
  WRAP_WORD_CHAR     = ExtendableTextWidget.WRAP_WORD_CHAR,
  ELLIPSIZE_NONE     = ExtendableTextWidget.ELLIPSIZE_NONE,
  ELLIPSIZE_START    = ExtendableTextWidget.ELLIPSIZE_START,
  ELLIPSIZE_MIDDLE   = ExtendableTextWidget.ELLIPSIZE_MIDDLE,
  ELLIPSIZE_END      = ExtendableTextWidget.ELLIPSIZE_END,
  RETRACT_NONE       = ExtendableTextWidget.RETRACT_NONE,
  RETRACT_UP         = ExtendableTextWidget.RETRACT_UP,
  RETRACT_DOWN       = ExtendableTextWidget.RETRACT_DOWN,
  RETRACT_LEFT       = ExtendableTextWidget.RETRACT_LEFT,
  RETRACT_RIGHT      = ExtendableTextWidget.RETRACT_RIGHT,
  RETRACTION_TIME    = ExtendableTextWidget.RETRACTION_TIME,
  RETRACTION_TIMEOUT = ExtendableTextWidget.RETRACTION_TIMEOUT
}

-- vi: et ts=2 sw=2

