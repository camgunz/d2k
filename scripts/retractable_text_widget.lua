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

local RetractableTextWidget = Widget.Widget:new()

RetractableTextWidget.ALIGN_LEFT    = 0
RetractableTextWidget.ALIGN_CENTER  = 1
RetractableTextWidget.ALIGN_RIGHT   = 2
RetractableTextWidget.ALIGN_JUSTIFY = 3

RetractableTextWidget.ALIGN_TOP     = 0
RetractableTextWidget.ALIGN_BOTTOM  = 2

RetractableTextWidget.WRAP_NONE      = 0
RetractableTextWidget.WRAP_WORD      = 1
RetractableTextWidget.WRAP_CHAR      = 2
RetractableTextWidget.WRAP_WORD_CHAR = 3

RetractableTextWidget.ELLIPSIZE_NONE   = 0
RetractableTextWidget.ELLIPSIZE_START  = 1
RetractableTextWidget.ELLIPSIZE_MIDDLE = 2
RetractableTextWidget.ELLIPSIZE_END    = 3

RetractableTextWidget.RETRACT_NONE  = 0
RetractableTextWidget.RETRACT_UP    = 1
RetractableTextWidget.RETRACT_DOWN  = 2
RetractableTextWidget.RETRACT_LEFT  = 3
RetractableTextWidget.RETRACT_RIGHT = 4

RetractableTextWidget.RETRACTION_TIME = 2000
RetractableTextWidget.RETRACTION_TIMEOUT = 2000

--
-- [TODO]
--
-- - Throw an error during initialization if vertical_align isn't ALIGN_TOP or
--   ALIGN_BOTTOM.
-- - Rename to RetractableTextWidget
-- - Only override methods that are actually different from TextWidget
-- - Pre-set fields properly, and only either new fields or fields whose
--   default value differs from TextWidget's default values
-- - On resize, we need to set the last retraction target to the last line
--

function RetractableTextWidget:new(rtw)
  rtw = rtw or {}

  rtw.top_margin = rtw.top_margin or 0
  rtw.bottom_margin = rtw.bottom_margin or 0
  rtw.left_margin = rtw.left_margin or 0
  rtw.right_margin = rtw.right_margin or 0
  rtw.text = rtw.text or ''
  rtw.max_width = rtw.max_width or rtw.width or 0
  rtw.max_height = rtw.max_height or rtw.height or 0
  rtw.fg_color = rtw.fg_color or {1.0, 1.0, 1.0, 1.0}
  rtw.bg_color = rtw.bg_color or {0.0, 0.0, 0.0, 0.0}
  rtw.outline_color = rtw.outline_color or {0.0, 0.0, 0.0, 0.0}
  rtw.outline_text = rtw.outline_text or false
  rtw.outline_width = rtw.outline_width or 0
  rtw.line_height = rtw.line_height or 0
  rtw.scrollable = rtw.scrollable or false
  rtw.font_description_text = rtw.font_description_text or
                              d2k.hud.font_description_text
  rtw.use_markup = rtw.use_markup or false
  rtw.strip_ending_newline = rtw.strip_ending_newline or false
  rtw.retractable = rtw.retractable or RetractableTextWidget.RETRACT_NONE
  rtw.retraction_time = rtw.retraction_time or
                        RetractableTextWidget.RETRACTION_TIME
  rtw.retraction_timeout = rtw.retraction_timeout or
                           RetractableTextWidget.RETRACTION_TIMEOUT

  rtw.retracting = false
  rtw.retracting_line_number = -1
  rtw.last_retracted_line_number = 0
  rtw.retraction_target = 0
  rtw.retraction_start_time = 0

  rtw.text_context = nil
  rtw.current_render_context = nil
  rtw.layout = nil

  rtw.horizontal_offset = 0.0
  rtw.vertical_offset = 0.0

  rtw.get_external_text = nil
  rtw.external_text_updated = nil
  rtw.clear_external_text_updated = nil

  setmetatable(rtw, self)
  self.__index = self

  rtw:build_layout()

  if rtw.word_wrap then
    rtw:set_word_wrap(rtw.word_wrap)
  else
    rtw:set_word_wrap(RetractableTextWidget.WRAP_NONE)
  end

  if rtw.horizontal_alignment then
    rtw:set_horizontal_alignment(rtw.horizontal_alignment)
  else
    rtw:set_horizontal_alignment(RetractableTextWidget.ALIGN_LEFT)
  end

  if rtw.vertical_alignment then
    rtw:set_vertical_alignment(rtw.vertical_alignment)
  else
    rtw:set_vertical_alignment(RetractableTextWidget.ALIGN_TOP)
  end

  if rtw.ellipsize then
    rtw:set_ellipsize(rtw.ellipsize)
  else
    rtw:set_ellipsize(RetractableTextWidget.ELLIPSIZE_NONE)
  end

  return rtw
end

function RetractableTextWidget:get_top_margin()
  return self.top_margin
end

function RetractableTextWidget:set_top_margin(top_margin)
  self.top_margin = top_margin
end

function RetractableTextWidget:get_bottom_margin()
  return self.bottom_margin
end

function RetractableTextWidget:set_bottom_margin(bottom_margin)
  self.bottom_margin = bottom_margin
end

function RetractableTextWidget:get_left_margin()
  return self.left_margin
end

function RetractableTextWidget:set_left_margin(left_margin)
  self.left_margin = left_margin
end

function RetractableTextWidget:get_right_margin()
  return self.right_margin
end

function RetractableTextWidget:set_right_margin(right_margin)
  self.right_margin = right_margin
end

function RetractableTextWidget:get_text()
  return self.text
end

function RetractableTextWidget:set_text(text)
  self.text = text
end

function RetractableTextWidget:get_max_width()
  return self.max_width
end

function RetractableTextWidget:set_max_width(max_width)
  self.max_width = max_width
end

function RetractableTextWidget:get_max_height()
  return self.max_height
end

function RetractableTextWidget:set_max_height(max_height)
  self.max_height = max_height
end

function RetractableTextWidget:get_fg_color()
  return self.fg_color
end

function RetractableTextWidget:set_fg_color(fg_color)
  self.fg_color = fg_color
end

function RetractableTextWidget:get_bg_color()
  return self.bg_color
end

function RetractableTextWidget:set_bg_color(bg_color)
  self.bg_color = bg_color
end

function RetractableTextWidget:get_outline_color()
  return self.outline_color
end

function RetractableTextWidget:set_outline_color(outline_color)
  self.outline_color = outline_color
end

function RetractableTextWidget:get_outline_text()
  return self.outline_text
end

function RetractableTextWidget:set_outline_text(outline_text)
  self.outline_text = outline_text
end

function RetractableTextWidget:get_outline_width()
  return self.outline_width
end

function RetractableTextWidget:set_outline_width(outline_width)
  self.outline_width = outline_width
end

function RetractableTextWidget:get_line_height()
  return self.line_height
end

function RetractableTextWidget:set_line_height(line_height)
  self.line_height = line_height
end

function RetractableTextWidget:get_scrollable()
  return self.scrollable
end

function RetractableTextWidget:set_scrollable(scrollable)
  self.scrollable = scrollable
end

function RetractableTextWidget:get_font_description_text()
  return self.font_description_text
end

function RetractableTextWidget:set_font_description_text(font_description_text)
  self.font_description_text = font_description_text
end

function RetractableTextWidget:get_use_markup()
  return self.use_markup
end

function RetractableTextWidget:set_use_markup(use_markup)
  self.use_markup = use_markup
end

function RetractableTextWidget:get_strip_ending_newline()
  return self.strip_ending_newline
end

function RetractableTextWidget:set_strip_ending_newline(strip_ending_newline)
  self.strip_ending_newline = strip_ending_newline
end

function RetractableTextWidget:get_retractable()
  if self.retractable == RetractableTextWidget.RETRACT_NONE then
    return false
  end

  return self.retractable
end

function RetractableTextWidget:set_retractable(retractable)
  if retractable == false then
    self.retractable = RetractableTextWidget.RETRACT_NONE
  else
    self.retractable = retractable
  end
end

function RetractableTextWidget:get_retraction_time()
  return self.retraction_time
end

function RetractableTextWidget:set_retraction_time(retraction_time)
  self.retraction_time = retraction_time
end

function RetractableTextWidget:get_retraction_timeout()
  return self.retraction_timeout
end

function RetractableTextWidget:set_retraction_timeout(retraction_timeout)
  self.retraction_timeout = retraction_timeout
end

function RetractableTextWidget:get_retracting()
  return self.retracting
end

function RetractableTextWidget:set_retracting(retracting)
  self.retracting = retracting
end

function RetractableTextWidget:get_retracting_line_number()
  return self.retracting_line_number
end

function RetractableTextWidget:set_retracting_line_number(
  retracting_line_number
)
  self.retracting_line_number = retracting_line_number
end

function RetractableTextWidget:get_last_retracted_line_number()
  return self.last_retracted_line_number
end

function RetractableTextWidget:set_last_retracted_line_number(
  last_retracted_line_number
)
  self.last_retracted_line_number = last_retracted_line_number
end

function RetractableTextWidget:get_retraction_target()
  return self.retraction_target
end

function RetractableTextWidget:set_retraction_target(retraction_target)
  self.retraction_target = retraction_target
end

function RetractableTextWidget:get_retraction_start_time()
  return self.retraction_start_time
end

function RetractableTextWidget:set_retraction_start_time(retraction_start_time)
  self.retraction_start_time = retraction_start_time
end

function RetractableTextWidget:get_text_context()
  return self.text_context
end

function RetractableTextWidget:set_text_context(text_context)
  self.text_context = text_context
end

function RetractableTextWidget:get_current_render_context()
  return self.current_render_context
end

function RetractableTextWidget:set_current_render_context(
  current_render_context
)
  self.current_render_context = current_render_context
end

function RetractableTextWidget:get_layout()
  return self.layout
end

function RetractableTextWidget:set_layout(layout)
  self.layout = layout
end

function RetractableTextWidget:get_horizontal_offset()
  return self.horizontal_offset
end

function RetractableTextWidget:set_horizontal_offset(horizontal_offset)
  self.horizontal_offset = horizontal_offset
  self:check_offsets()
end

function RetractableTextWidget:get_vertical_offset()
  return self.vertical_offset
end

function RetractableTextWidget:set_vertical_offset(vertical_offset)
  self.vertical_offset = vertical_offset
  self:check_offsets()
end

function RetractableTextWidget:get_layout_line_count()
  if #self:get_text() == 0 then
    return 0
  end

  return self:get_layout():get_line_count()
end

function RetractableTextWidget:get_min_max_line_numbers(line_count)
  local line_height = self:get_line_height()
  local last_retracted_line_number = self:get_last_retracted_line_number()
  local line_count = line_count or self:get_layout_line_count()
  local min_line = 1
  local max_line = line_count
  
  if line_height == 0 or line_count <= line_height then
    if last_retracted_line_number == 0 then
      return min_line, max_line
    end

    min_line = last_retracted_line_number + 1

    if min_line > line_count then
      return 0, 0
    end

    return min_line, max_line
  end

  min_line = line_count - (line_height - 1)

  if min_line <= last_retracted_line_number then
    min_line = last_retracted_line_number + 1
  end

  max_line = math.min(min_line + (line_height - 1), line_count)

  return min_line, max_line
end

function RetractableTextWidget:update_layout_if_needed()
  if not self.needs_updating then
    return
  end

  local text = self:get_text()
  local layout = self:get_layout()

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
  PangoCairo.update_layout(d2k.overlay.render_context, layout)
  self:check_offsets()

  if self.get_external_text then
    local line_count = self:get_layout_line_count()
    local start_line, end_line = self:get_min_max_line_numbers(line_count)
    local lines = self:get_visible_line_dimensions(start_line, end_line)
    local visible_line_count = #lines

    if visible_line_count == 0 then
      self:set_height_in_pixels(0)
      return
    end

    self:set_height_in_pixels(
      self:get_top_margin() +
      lines[1].y +
      lines[visible_line_count].y +
      lines[visible_line_count].height +
      self:get_bottom_margin()
    )
  end

  self.needs_updating = false
end

function RetractableTextWidget:get_visible_lines(start_line, end_line)
  local line_height = self:get_line_height()
  local layout = self:get_layout()
  local line_number = 1
  local lines = {}

  if start_line == 0 or end_line == 0 then
    return lines
  end

  local line_count = self:get_layout_line_count()
  local iter = layout:get_iter()

  if start_line == nil or end_line == nil then
    start_line, end_line = self:get_min_max_line_numbers(line_count)
  end

  while line_number < start_line do
    iter:next_line()
    line_number = line_number + 1
  end

  while line_number <= end_line do
    local ink_rect, logical_rect = iter:get_line_extents()

    table.insert(lines, {
      number     = line_number,
      x          = logical_rect.x      / Pango.SCALE,
      y          = logical_rect.y      / Pango.SCALE,
      width      = logical_rect.width  / Pango.SCALE,
      height     = logical_rect.height / Pango.SCALE,
      baseline   = iter:get_baseline() / Pango.SCALE,
      pango_line = iter:get_line_readonly()
    })

    if not iter:next_line() then
      break
    end

    line_number = line_number + 1
  end

  return lines
end

function RetractableTextWidget:get_visible_line_dimensions(
  start_line, end_line
)
  local line_height = self:get_line_height()
  local layout = self:get_layout()
  local line_number = 1
  local lines = {}

  if start_line == 0 or end_line == 0 then
    return lines
  end

  local line_count = self:get_layout_line_count()
  local iter = layout:get_iter()

  if start_line == nil or end_line == nil then
    start_line, end_line = self:get_min_max_line_numbers(line_count)
  end

  while line_number < start_line do
    iter:next_line()
    line_number = line_number + 1
  end

  while line_number <= end_line do
    local ink_rect, logical_rect = iter:get_line_extents()

    table.insert(lines, {
      number = line_number,
      x      = logical_rect.x      / Pango.SCALE,
      y      = logical_rect.y      / Pango.SCALE,
      width  = logical_rect.width  / Pango.SCALE,
      height = logical_rect.height / Pango.SCALE
    })

    if not iter:next_line() then
      break
    end

    line_number = line_number + 1
  end

  return lines
end

function RetractableTextWidget:reset()
  local layout = self:get_layout()
  local layout_width, layout_height = layout:get_pixel_size()
  local line_count = self:get_layout_line_count()

  self:set_retracting(false)
  if self:get_retracting_line_number() == -1 then
    self:set_retracting_line_number(-1)
    self:set_last_retracted_line_number(0)
    self:set_vertical_offset(0)
  else
    self:set_retracting_line_number(0)
    self:set_last_retracted_line_number(line_count)
    self:set_vertical_offset(layout_height)
  end
  self:set_retraction_target(0)
  self:set_retraction_start_time(0)
end

function RetractableTextWidget:tick()
  local current_ms = d2k.System.get_ticks()

  if self.get_external_text and self.external_text_updated() then
    self.text = self.get_external_text()
    self.needs_updating = true
    self.clear_external_text_updated()
  end

  if not self:get_retractable() then
    return
  end

  if #self.text == 0 then
    return
  end

  local start_line, end_line = self:get_min_max_line_numbers()

  if start_line == 0 or end_line == 0 then
    return
  end

  local visible_lines = self:get_visible_line_dimensions(start_line, end_line)
  local visible_line_count = #visible_lines

  if visible_line_count == 0 then
    return
  end

  local layout_width, layout_height = self:get_layout():get_pixel_size()

  self:set_height_in_pixels(
    (layout_height - visible_lines[1].y) +
    self:get_top_margin() +
    self:get_bottom_margin()
  )

  if self:get_retracting() then
    local retracting_line_number = self:get_retracting_line_number()

    if retracting_line_number < start_line or
       retracting_line_number > end_line or
       not self:retract(current_ms, visible_lines[1].y) then
      self:set_vertical_offset(self:get_retraction_target())
      self:set_retracting(false)
      self:set_last_retracted_line_number(retracting_line_number)
      self:set_retracting_line_number(0)
      self:set_retraction_target(0)
      self:set_retraction_start_time(0)
    end

    return
  end

  local retraction_start_time = self:get_retraction_start_time()

  if retraction_start_time == 0 then
    self:set_retraction_start_time(current_ms + self:get_retraction_timeout())
    return
  end

  if current_ms > self:get_retraction_time() then
    self:set_retracting_line_number(visible_lines[1].number)
    if visible_line_count > 1 then
      self:set_retraction_target(visible_lines[2].y)
    else
      self:set_retraction_target(layout_height)
    end
    self:set_retracting(true)
  end
end

function RetractableTextWidget:retract(current_ms, top)
  local current_ms = current_ms or d2k.System.get_ticks()
  local retraction_start_time = self:get_retraction_start_time()
  local retraction_ms_elapsed = current_ms - retraction_start_time
  local ms_retracted = retraction_ms_elapsed - self:get_retraction_timeout()

  if ms_retracted <= 0 then
    return true
  end

  local retraction_time = self:get_retraction_time()
  local height_fraction = ms_retracted / retraction_time
  local target = self:get_retraction_target()
  local retraction_distance = target - top
  local y_delta = (ms_retracted / retraction_time) * retraction_distance

  if y_delta < retraction_distance then
    self:set_vertical_offset(top + y_delta)
    return true
  end

  return false
end

function RetractableTextWidget:draw()
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

  line_count = self:get_layout_line_count()

  cr:save()

  cr:set_operator(Cairo.Operator.OVER)

  cr:reset_clip()
  cr:new_path()
  cr:rectangle(
    self:get_x(),
    self:get_y(),
    self:get_width_in_pixels(),
    self:get_height_in_pixels()
  )
  cr:clip()

  cr:set_source_rgba(bg_color[1], bg_color[2], bg_color[3], bg_color[4])
  cr:paint()

  if line_count <= 0 then
    return
  end

  cr:set_source_rgba(fg_color[1], fg_color[2], fg_color[3], fg_color[4])

  local lx = self:get_x() + self:get_left_margin()
  local ly = self:get_y() + self:get_top_margin()
  local text_width = self:get_width_in_pixels() - (
    self:get_left_margin() + self:get_right_margin()
  )
  local text_height = self:get_height_in_pixels() - (
    self:get_top_margin() + self:get_bottom_margin()
  )
  local layout_width, layout_height = self:get_layout():get_pixel_size()
  local layout_ink_extents, layout_logical_extents =
    self:get_layout():get_pixel_extents()
  local retractable = self:get_retractable()

  if self.vertical_alignment == RetractableTextWidget.ALIGN_BOTTOM then
    ly = ly + text_height - layout_height
  end

  if self.horizontal_alignment == RetractableTextWidget.ALIGN_CENTER then
    lx = (text_width / 2) - (layout_width / 2)
  elseif self.horizontal_alignment == RetractableTextWidget.ALIGN_RIGHT then
    lx = lx + text_width - layout_width
  end

  if retractable or layout_height > text_height then
    ly = ly - self:get_vertical_offset()
  end

  local min_line, max_line = self:get_min_max_line_numbers(line_count)
  local visible_lines = self:get_visible_lines(min_line, max_line)
  local start_y_offset = 0
  local rendered_at_least_one_line = false
  local line_height = self:get_line_height()

  for i, line in ipairs(visible_lines) do
    local line_start_x = line.x + lx
    local line_start_y = line.y + ly

    if line_height > 0 and i == 1 and line.y > 0 then
      start_y_offset = -line.y
    end

    cr:move_to(line_start_x, line.baseline + ly + start_y_offset)

    if self:get_outline_text() then
      cr:save()

      PangoCairo.layout_line_path(cr, line.pango_line)

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
      PangoCairo.show_layout_line(cr, line.pango_line)
    end
  end

  cr:restore()
end

function RetractableTextWidget:build_layout()
  self:set_current_render_context(d2k.overlay.render_context)
  self:set_layout(Pango.Layout.new(d2k.overlay.text_context))

  local layout = self:get_layout()

  self:get_layout():set_font_description(Pango.FontDescription.from_string(
    self:get_font_description_text()
  ))

  local layout_ink_extents, layout_logical_extents = layout:get_pixel_extents()
end

function RetractableTextWidget:set_height_by_lines(line_count)
  self:set_line_height(line_count)
end

function RetractableTextWidget:set_external_text_source(get_text,
                                             text_updated,
                                             clear_text_updated)
  self.get_external_text = get_text
  self.external_text_updated = text_updated
  self.clear_external_text_updated = clear_text_updated
end

function RetractableTextWidget:get_text()
  if self.get_external_text then
    return self.get_external_text()
  else
    return self.text
  end
end

function RetractableTextWidget:set_text(text)
  if self.get_external_text then
    error(string.format("%s: Can't set text when displaying external text",
      self.name
    ))
  end
  self.text = text
  self.needs_updating = true
end

function RetractableTextWidget:clear()
  if self.get_external_text then
    error(string.format("%s: Can't clear text when displaying external text",
      self.name
    ))
  end
  self:set_text('')
end

function RetractableTextWidget:get_ellipsize()
  return self.ellipsize
end

function RetractableTextWidget:set_ellipsize(ellipsize)
  local layout = self:get_layout()

  if ellipsize == RetractableTextWidget.ELLIPSIZE_NONE then
    self.ellipsize = RetractableTextWidget.ELLIPSIZE_NONE
    layout:set_ellipsize(Pango.EllipsizeMode.NONE)
  elseif ellipsize == RetractableTextWidget.ELLIPSIZE_START then
    self.ellipsize = RetractableTextWidget.ELLIPSIZE_START
    layout:set_ellipsize(Pango.EllipsizeMode.START)
  elseif ellipsize == RetractableTextWidget.ELLIPSIZE_MIDDLE then
    self.ellipsize = RetractableTextWidget.ELLIPSIZE_MIDDLE
    layout:set_ellipsize(Pango.EllipsizeMode.MIDDLE)
  elseif ellipsize == RetractableTextWidget.ELLIPSIZE_END then
    self.ellipsize = RetractableTextWidget.ELLIPSIZE_END
    layout:set_ellipsize(Pango.EllipsizeMode.END)
  else
    s = 'RetractableTextWidget:set_ellipsize: Invalid ellipsization value %s'
    error(s:format(ellipsize))
  end

  self.needs_updating = true
end

function RetractableTextWidget:is_ellipsize()
  return self:get_layout():is_ellipsized()
end

function RetractableTextWidget:get_word_wrap()
  return self.word_wrap
end

function RetractableTextWidget:set_word_wrap(word_wrap)
  local layout = self:get_layout()

  if word_wrap == RetractableTextWidget.WRAP_NONE then
    self.word_wrap = RetractableTextWidget.WRAP_NONE
    layout:set_width(-1)
  elseif word_wrap == RetractableTextWidget.WRAP_WORD then
    self.word_wrap = RetractableTextWidget.WRAP_WORD
    layout:set_width(self:get_width_in_pixels() * Pango.SCALE)
    layout:set_wrap(Pango.WrapMode.WORD)
  elseif word_wrap == RetractableTextWidget.WRAP_CHAR then
    self.word_wrap = RetractableTextWidget.WRAP_CHAR
    layout:set_width(self:get_width_in_pixels() * Pango.SCALE)
    layout:set_wrap(Pango.WrapMode.CHAR)
  elseif word_wrap == RetractableTextWidget.WRAP_WORD_CHAR then
    self.word_wrap = RetractableTextWidget.WRAP_WORD_CHAR
    layout:set_width(self:get_width_in_pixels() * Pango.SCALE)
    layout:set_wrap(Pango.WrapMode.WORD_CHAR)
  else
    s = 'RetractableTextWidget:set_word_wrap: Invalid word wrap value %s'
    error(s:format(word_wrap))
  end

  self.needs_updating = true
end

function RetractableTextWidget:is_wrapped()
  return self:get_layout():is_wrapped()
end

function RetractableTextWidget:get_horizontal_alignment()
  return self.horizontal_alignment
end

function RetractableTextWidget:set_horizontal_alignment(horizontal_alignment)
  local layout = self:get_layout()

  if horizontal_alignment == RetractableTextWidget.ALIGN_LEFT then
    self.horizontal_alignment = RetractableTextWidget.ALIGN_LEFT
    layout:set_alignment(Pango.Alignment.LEFT)
  elseif horizontal_alignment == RetractableTextWidget.ALIGN_CENTER then
    self.horizontal_alignment = RetractableTextWidget.ALIGN_CENTER
    layout:set_alignment(Pango.Alignment.CENTER)
  elseif horizontal_alignment == RetractableTextWidget.ALIGN_RIGHT then
    self.horizontal_alignment = RetractableTextWidget.ALIGN_RIGHT
    layout:set_alignment(Pango.Alignment.RIGHT)
  elseif horizontal_alignment == RetractableTextWidget.ALIGN_JUSTIFY then
    self.horizontal_alignment = RetractableTextWidget.ALIGN_JUSTIFY
    layout:set_alignment(Pango.Alignment.JUSTIFY)
  else
    s = 'RetractableTextWidget:set_horizontal_alignment: ' ..
        'Invalid horizontal alignment %s'
    error(s:format(horizontal_alignment))
  end

  self.needs_updating = true
end

function RetractableTextWidget:get_vertical_alignment()
  return self.vertical_alignment
end

function RetractableTextWidget:set_vertical_alignment(vertical_alignment)
  if vertical_alignment == RetractableTextWidget.ALIGN_TOP then
    self.vertical_alignment = RetractableTextWidget.ALIGN_TOP
  elseif vertical_alignment == RetractableTextWidget.ALIGN_CENTER then
    self.vertical_alignment = RetractableTextWidget.ALIGN_CENTER
  elseif vertical_alignment == RetractableTextWidget.ALIGN_BOTTOM then
    self.vertical_alignment = RetractableTextWidget.ALIGN_BOTTOM
  else
    s = 'RetractableTextWidget:set_vertical_alignment: ' ..
        'Invalid vertical alignment %s'
    error(s:format(vertical_alignment))
  end

  self.needs_updating = true
end

function RetractableTextWidget:check_offsets()
  if self:get_retractable() then
    return
  end

  local text_width = self:get_width_in_pixels() - (
    self:get_left_margin() + self:get_right_margin()
  )
  local text_height = self:get_height_in_pixels() - (
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

  if self.horizontal_alignment == RetractableTextWidget.ALIGN_CENTER then
    min_x = -((layout_width - text_width) / 2) - self:get_left_margin()
    max_x = ((layout_width - text_width) / 2)
  elseif self.horizontal_alignment == RetractableTextWidget.ALIGN_RIGHT then
    min_x = -(layout_width - text_width) - self:get_left_margin()
    max_x = 0
  end

  if self.vertical_alignment == RetractableTextWidget.ALIGN_CENTER then
    min_y = -((layout_height - text_height) / 2) - self:get_top_margin()
    max_y = ((layout_height - text_height) / 2)
  elseif self.vertical_alignment == RetractableTextWidget.ALIGN_BOTTOM then
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
end

function RetractableTextWidget:scroll_left(pixels)
  self:set_horizontal_offset(self:get_horizontal_offset() - pixels)
  self:check_offsets()
end

function RetractableTextWidget:scroll_right(pixels)
  self:set_horizontal_offset(self:get_horizontal_offset() + pixels)
  self:check_offsets()
end

function RetractableTextWidget:scroll_up(pixels)
  self:set_vertical_offset(self:get_vertical_offset() - pixels)
end

function RetractableTextWidget:scroll_down(pixels)
  self:set_vertical_offset(self:get_vertical_offset() + pixels)
end

function RetractableTextWidget:write(text)
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

function RetractableTextWidget:mwrite(markup)
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

function RetractableTextWidget:echo(text)
  if self.get_external_text then
    error(string.format("%s: Can't echo text when displaying external text",
      self.name
    ))
  end

  self:set_text(self:get_text() .. GLib.markup_escape_text(text, -1) .. '\n')
end

function RetractableTextWidget:mecho(markup)
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
  RetractableTextWidget = RetractableTextWidget,
  ALIGN_LEFT            = RetractableTextWidget.ALIGN_LEFT,
  ALIGN_CENTER          = RetractableTextWidget.ALIGN_CENTER,
  ALIGN_RIGHT           = RetractableTextWidget.ALIGN_RIGHT,
  ALIGN_JUSTIFY         = RetractableTextWidget.ALIGN_JUSTIFY,
  ALIGN_TOP             = RetractableTextWidget.ALIGN_TOP,
  ALIGN_BOTTOM          = RetractableTextWidget.ALIGN_BOTTOM,
  WRAP_NONE             = RetractableTextWidget.WRAP_NONE,
  WRAP_WORD             = RetractableTextWidget.WRAP_WORD,
  WRAP_CHAR             = RetractableTextWidget.WRAP_CHAR,
  WRAP_WORD_CHAR        = RetractableTextWidget.WRAP_WORD_CHAR,
  ELLIPSIZE_NONE        = RetractableTextWidget.ELLIPSIZE_NONE,
  ELLIPSIZE_START       = RetractableTextWidget.ELLIPSIZE_START,
  ELLIPSIZE_MIDDLE      = RetractableTextWidget.ELLIPSIZE_MIDDLE,
  ELLIPSIZE_END         = RetractableTextWidget.ELLIPSIZE_END,
  RETRACT_NONE          = RetractableTextWidget.RETRACT_NONE,
  RETRACT_UP            = RetractableTextWidget.RETRACT_UP,
  RETRACT_DOWN          = RetractableTextWidget.RETRACT_DOWN,
  RETRACT_LEFT          = RetractableTextWidget.RETRACT_LEFT,
  RETRACT_RIGHT         = RetractableTextWidget.RETRACT_RIGHT,
  RETRACTION_TIME       = RetractableTextWidget.RETRACTION_TIME,
  RETRACTION_TIMEOUT    = RetractableTextWidget.RETRACTION_TIMEOUT
}

-- vi: et ts=2 sw=2

