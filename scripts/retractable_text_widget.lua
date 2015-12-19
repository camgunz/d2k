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
local GLib = lgi.GLib
local Pango = lgi.Pango
local PangoCairo = lgi.PangoCairo
local TextWidget = require('text_widget')

RetractableTextWidget = class('RetractableTextWidget', TextWidget.TextWidget)

local RETRACT_NONE = 0
local RETRACT_UP   = 1
local RETRACT_DOWN = 2

local RETRACTION_TIME    = 2000
local RETRACTION_TIMEOUT = 2000

function RetractableTextWidget:initialize(rtw)
  rtw = rtw or {}

  self.retractable = rtw.retractable or RETRACT_NONE
  self.retraction_time = rtw.retraction_time or RETRACTION_TIME
  self.retraction_timeout = rtw.retraction_timeout or RETRACTION_TIMEOUT

  self.retracting = false
  self.retraction_target = 0
  self.retraction_start_time = 0
  self.last_retracted_line_number = 0
  self.strip_ending_newline = true
  self.visible_lines = {}

  TextWidget.TextWidget.initialize(self, rtw)
end

function RetractableTextWidget:set_vertical_alignment(vertical_alignment)
  if not (vertical_alignment == TextWidget.ALIGN_TOP or
          vertical_alignment == TextWidget.ALIGN_BOTTOM) then
    error(
      'Invalid vertical alignment; retractable text widgets must align to ' ..
      'either top or bottom'
    )
  end

  TextWidget.TextWidget.set_vertical_alignment(self, vertical_alignment)
end

function RetractableTextWidget:get_retractable()
  if self.retractable == RETRACT_NONE then
    return false
  end

  return self.retractable
end

function RetractableTextWidget:set_retractable(retractable)
  if retractable == false then
    self.retractable = RETRACT_NONE
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

function RetractableTextWidget:get_retraction_start_time()
  return self.retraction_start_time
end

function RetractableTextWidget:set_retraction_start_time(retraction_start_time)
  self.retraction_start_time = retraction_start_time
end

function RetractableTextWidget:update_layout_line_count()
  if #self:get_text() == 0 then
    self.layout_line_count = 0
  else
    self.layout_line_count = self:get_layout():get_line_count()
  end
end

function RetractableTextWidget:get_last_retracted_line_number()
  return self.last_retracted_line_number
end

function RetractableTextWidget:set_last_retracted_line_number(
  last_retracted_line_number
)
  self.last_retracted_line_number = last_retracted_line_number
end

function RetractableTextWidget:get_visible_lines()
  return self.visible_lines
end

function RetractableTextWidget:set_visible_lines(visible_lines)
  self.visible_lines = visible_lines
end

function RetractableTextWidget:refresh_visible_lines()
  local visible_lines = {}
  local last_retracted_line_number = self:get_last_retracted_line_number()
  local line_number = last_retracted_line_number + 1
  local iter = self:get_layout():get_iter()

  for i=1,last_retracted_line_number do
    if not iter:next_line() then
      return
    end
  end

  while #visible_lines < self:get_line_height() do
    local ink_rect, logical_rect = iter:get_line_extents()

    table.insert(visible_lines, {
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

  self.visible_lines = visible_lines
end

function RetractableTextWidget:update_visible_lines()
  local visible_lines = self:get_visible_lines()
  local line_height = self:get_line_height()
  local last_retracted_line_number = self:get_last_retracted_line_number()
  local line_number = last_retracted_line_number + 1
  local line_count = self:get_layout_line_count()
  local iter = self:get_layout():get_iter()
  local removed_visible_line = false

  for i=1,last_retracted_line_number do
    if not iter:next_line() then
      self:set_retracting(false)
      self:set_retraction_start_time(0)
      self:set_height_in_pixels(0)
      return
    end
  end

  while true do
    if #visible_lines > self:get_line_height() then
      local line = visible_lines[1]

      self:set_last_retracted_line_number(line.number)
      table.remove(visible_lines, 1)
      removed_visible_line = true
    end

    local ink_rect, logical_rect = iter:get_line_extents()

    table.insert(visible_lines, {
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

  self:set_visible_lines(visible_lines)

  if removed_visible_line then
    if self:get_retracting() then
      self:set_retracting(false)
      self:set_retraction_start_time(0)
    else
      local visible_line_count = #visible_lines

      self:set_height_in_pixels(
        self:get_top_margin() +
        visible_lines[visible_line_count].y +
        visible_lines[visible_line_count].height -
        visible_lines[1].y +
        self:get_bottom_margin()
      )
    end
  end

  print(string.format('Set vl to %s', #visible_lines))
end


function RetractableTextWidget:reset()
  local layout = self:get_layout()
  local layout_width, layout_height = layout:get_pixel_size()
  local line_count = self:get_layout_line_count()

  self:set_retracting(false)
  self:set_retraction_start_time(0)
  self:set_last_retracted_line_number(0)
end

function RetractableTextWidget:retract(current_ms)
  local current_ms = current_ms or d2k.System.get_ticks()
  local retraction_start_time = self:get_retraction_start_time()
  local retraction_ms_elapsed = current_ms - retraction_start_time
  local ms_retracted = retraction_ms_elapsed - self:get_retraction_timeout()

  if ms_retracted <= 0 then
    return true
  end

  local retraction_time = self:get_retraction_time()
  local height_fraction = ms_retracted / retraction_time
  local visible_lines = self:get_visible_lines()
  local top_line = visible_lines[1]
  local next_line = visible_lines[2]
  local target = 0

  if next_line == nil then
    target = top_line.height
  else
    target = next_line.y - top_line.y
  end

  local retraction_distance = target
  local y_delta = (ms_retracted / retraction_time) * retraction_distance

  if y_delta >= retraction_distance then
    return false
  end

  self:set_height_in_pixels(self:get_height_in_pixels() - y_delta)
  -- self:set_vertical_offset(top + y_delta)
  self:handle_display_change()
  return true
end

function RetractableTextWidget:tick()
  local current_ms = d2k.System.get_ticks()

  TextWidget.TextWidget.tick(self)

  if not self:get_retractable() then
    return
  end

  if #self:get_text() == 0 then
    return
  end

  local visible_lines = self:get_visible_lines()

  local visible_line_count = 0
  local start_line = 0
  local end_line = 0

  if not visible_lines then
    self:set_retracting(false)
    self:set_height_in_pixels(0)
    self:set_retraction_start_time(0)
  end

  local visible_line_count = #visible_lines

  if visible_line_count == 0 then
    self:set_retracting(false)
    self:set_height_in_pixels(0)
    self:set_retraction_start_time(0)
    return
  end

  local start_line = self.visible_lines[1]
  local end_line = self.visible_lines[visible_line_count]

  if self:get_retracting() then
    if not self:retract(current_ms) then
      visible_line_count = visible_line_count - 1
      end_line = self.visible_lines[visible_line_count]
      if end_line then
        self:set_height_in_pixels(
          self:get_top_margin() +
          end_line.y +
          end_line.height -
          start_line.y +
          self:get_bottom_margin()
        )
      else
        self:set_height_in_pixels(
          self:get_top_margin() +
          start_line.height +
          self:get_bottom_margin()
        )
      end
      self:set_retracting(false)
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
    self:set_retracting(true)
  end
end

function RetractableTextWidget:layout_text()
  TextWidget.TextWidget.layout_text(self)

  self:update_visible_lines()
end

function RetractableTextWidget:render()
  local cr = d2k.overlay.render_context
  local fg_color = self:get_fg_color()
  local bg_color = self:get_bg_color()
  local outline_color = self:get_outline_color()
  local lw = self:get_layout_pixel_width()
  local lh = self:get_layout_pixel_height()
  local line_count = self:get_layout_line_count()

  cr:save()

  cr:set_operator(Cairo.Operator.OVER)

  --[[
  cr:reset_clip()
  cr:new_path()
  cr:rectangle(
    self:get_x(),
    self:get_y(),
    self:get_width_in_pixels(),
    self:get_height_in_pixels()
  )
  cr:clip()
  --]]

  cr:set_source_rgba(bg_color[1], bg_color[2], bg_color[3], bg_color[4])
  cr:paint()

  if line_count <= 0 then
    cr:restore()
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
  local layout_ink_extents = self:get_layout_ink_extents()
  local layout_logical_extents = self:get_layout_logical_extents()
  local retractable = self:get_retractable()

  if self.vertical_alignment == TextWidget.ALIGN_BOTTOM then
    ly = ly + text_height - lh
  end

  if self.horizontal_alignment == TextWidget.ALIGN_CENTER then
    lx = (text_width / 2) - (layout_width / 2)
  elseif self.horizontal_alignment == TextWidget.ALIGN_RIGHT then
    lx = lx + text_width - lw
  end

  if retractable or layout_height > text_height then
    ly = ly - self:get_vertical_offset()
  end

  self:refresh_visible_lines()

  local visible_lines = self:get_visible_lines()
  local start_y_offset = 0
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

function RetractableTextWidget:handle_content_change()
  TextWidget.TextWidget.handle_content_change(self)
  self:update_visible_lines()
end

function RetractableTextWidget:check_offsets()
end

return {
  RetractableTextWidget = RetractableTextWidget,
  RETRACT_NONE          = RETRACT_NONE,
  RETRACT_UP            = RETRACT_UP,
  RETRACT_DOWN          = RETRACT_DOWN,
  RETRACTION_TIME       = RETRACTION_TIME,
  RETRACTION_TIMEOUT    = RETRACTION_TIMEOUT
}

-- vi: et ts=2 sw=2

