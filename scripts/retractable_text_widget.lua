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
  self.visible_line_indices = {}

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
  local visible_line_indices = self:get_visible_line_indices()
  local visible_lines = {}
  local iter = self:get_layout():get_iter()

  print(string.format('get_visible_lines - indices: %s', #visible_line_indices))

  if #visible_line_indices < 1 then
    return visible_lines
  end

  for i=1,#visible_line_indices do
    local ink_rect, logical_rect = iter:get_line_extents()

    print(string.format('get_visible_lines- adding line %s', i))
    table.insert(visible_lines, {
      number     = visible_line_indices[i],
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
  end

  local s = string.format('get_visible_lines - visible_lines: %s',
    visible_lines[1].number
  )

  for i=2,#visible_lines do
    s = string.format('%s, %s', s, visible_lines[i].number)
  end

  print(s)
  print(string.format('get_visible_lines- last retracted line: %s',
    last_retracted_line_number
  ))
  return visible_lines
end

function RetractableTextWidget:get_visible_line_indices()
  return self.visible_line_indices
end

function RetractableTextWidget:set_visible_line_indices(visible_line_indices)
  self.visible_line_indices = visible_line_indices
end

function RetractableTextWidget:update_visible_line_indices()
  local line_height = self:get_line_height()
  local line_count = self:get_layout_line_count()
  local visible_line_indices = self:get_visible_line_indices()
  local last_retracted_line_number = self:get_last_retracted_line_number()

  print(string.format(
    'update_visible_line_indices - visible_line_indices: %s, %s, %s, %s',
    line_height,
    line_count,
    #visible_line_indices,
    last_retracted_line_number
  ))

  -- If there is no text, there are no visible lines.

  if #self.text <= 0 then
    print(string.format('update_visible_line_indices - no text'))
    self:set_visible_line_indices({})
    return
  end

  -- The first visible line is either the Nth from the end (where N is
  -- line_height) or the line after the last retracted line index, whichever is
  -- newer.

  local first_visible_line_index = ((line_count - line_height) + 1)

  -- This also handles the case where line_count < line_height, making
  -- first_visible_line_index negative.  last_retracted_line_number will be 0
  -- to start, and as a result this check ensures first_visible_line_index will
  -- always be at least 1.

  if first_visible_line_index < (last_retracted_line_number + 1) then
    first_visible_line_index = last_retracted_line_number + 1
  end

  print(string.format(
    'update_visible_line_indices - first_visible_line_index: %s',
    first_visible_line_index
  ))

  local new_visible_line_indices = {}

  -- Build the new visible line indices.

  for i=first_visible_line_index,line_count do
    print(string.format(
      'update_visible_line_indices - adding visible line index %s', i
    ))
    table.insert(new_visible_line_indices, i)
  end

  local s = 'update_visible_line_indices - %s'

  if #visible_line_indices <= 0 then
    -- No current visible line indices:
    -- - Set visible line indices to new visible line indices.
    -- - Set height to full height.

    print(string.format(
      'update_visible_line_indices - No previously visible lines'
    ))
    self:set_visible_line_indices(new_visible_line_indices)
    self:set_full_height()
    return
  end

  local gap = first_visible_line_index -
              visible_line_indices[#visible_line_indices]

  if gap > 1 then
    -- Got an entirely new set of visible lines:
    -- - Stop retracting.
    -- - Set height to full height.

    print(string.format(
      'update_visible_line_indices - Entirely new set of visible lines'
    ))

    self:set_retracting(false)
    self:set_retraction_start_time(0)
    self:set_last_retracted_line_index(first_visible_line_index - 1)

    self:set_visible_line_indices(new_visible_line_indices)
    self:set_full_height()
    return
  end

  -- Concatenate the list of visible line indices, removing duplicates.

  local last_old_visible_line_index =
    visible_line_indices[#visible_line_indices]

  while #new_visible_line_indices > 0 do
    if new_visible_line_indices[1] > last_old_visible_line_index then
      break
    end

    print(string.format(
      'update_visible_line_indices - Removing duplicate index %s',
      new_visible_line_indices[1]
    ))

    table.remove(new_visible_line_indices, 1)
  end

  -- If there were no new visible line indices, we're done

  if #new_visible_line_indices <= 0 then
    print(string.format(
      'update_visible_line_indices - No new visible lines'
    ))
    return
  end

  -- If the new set of visible line indices is longer than line_height, then
  -- remove old lines until it isn't.

  local last_removed_line_index = 0

  while #visible_line_indices + #new_visible_line_indices > line_height do
    last_removed_line_index = visible_line_indices[1]
    print(string.format(
      'update_visible_line_indices - Removing old line %s',
      visible_line_indices[1]
    ))
    table.remove(visible_line_indices, 1)
  end

  -- Build the new list of visible line indices

  if #visible_line_indices then
    for i=1,#new_visible_line_indices do
      table.insert(visible_line_indices, new_visible_line_indices[i])
    end
  else
    visible_line_indices = new_visible_line_indices
  end

  self:set_visible_line_indices(visible_line_indices)

  -- Visible line overflow:
  -- - Stop retracting
  -- - Set the new visible line indices to the newest indices that fit
  -- - Set height to full height

  if last_removed_line_index > 0 then
    print(string.format(
      'update_visible_line_indices - Handling visible line overflow'
    ))
    self:set_retracting(false)
    self:set_retraction_start_time(0)
    self:set_last_retracted_line_index(last_removed_line_index)
    self:set_full_height()
    return
  end

  -- Retracting:
  -- - Add the height of each new line to this widget's height
  -- - Update height
  -- - Update visible lines

  local height = self:get_height_in_pixels()
  local first_new_visible_line_index = new_visible_line_indices[1]
  local iter = self:get_layout():get_iter()

  -- Iterate until we reach the 1st visible line

  for i=2,first_new_visible_line_index do
    if not iter:next_line() then
      return
    end
    print(string.format(
      'update_visible_line_indices - Iterated to line %s', i
    ))
  end

  -- Get the geometry of the 1st visible line

  local ink_rect, logical_rect = iter:get_line_extents()
  local start_y = logical_rect.y / Pango.SCALE

  -- Iterate until we reach the last visible line

  for i=2,#new_visible_line_indices do
    if iter:at_last_line() then
      print(string.format(
        'update_visible_line_indices - Reached last line prematurely'
      ))
      break
    end

    iter:next_line()
  end

  local ink_rect, logical_rect = iter:get_line_extents()
  local line_y = logical_rect.y / Pango.SCALE
  local line_height = logical_rect.height / Pango.SCALE

  print(string.format(
    'update_visible_line_indices - New height: %s',
    height + (line_y - start_y) + line_height
  ))

  self:set_height_in_pixels(height + (line_y - start_y) + line_height)
end

function RetractableTextWidget:set_full_height()
  local visible_lines = self:get_visible_lines()

  self:set_height_in_pixels(
    self:get_top_margin() +
    visible_lines[#visible_lines].y +
    visible_lines[#visible_lines].height -
    visible_lines[1].y +
    self:get_bottom_margin()
  )
end

function RetractableTextWidget:update_height()
  local height = 0
  local visible_lines = self:get_visible_lines()

  print(string.format('update_height - Updating height'))

  if self:get_retracting() then
    height = self:get_height_in_pixels()

    for i=2,#visible_lines do
      height = height + visible_lines[i].height
    end
  elseif #visible_lines > 0 then
    height = (
      self:get_top_margin() +
      visible_lines[#visible_lines].y +
      visible_lines[#visible_lines].height -
      visible_lines[1].y +
      self:get_bottom_margin()
    )
  else
    height = 0
  end

  self:set_height_in_pixels(height)
  print(string.format('%s visible lines total', #visible_lines))
  print(string.format('Height: %s', height))
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

  local visible_line_indices = self:get_visible_line_indices()

  if #visible_line_indices < 1 then
    self:set_retracting(false)
    self:set_height_in_pixels(0)
    self:set_retraction_start_time(0)
    return
  end

  if self:get_retracting() then
    if not self:retract(current_ms) then
      self:set_retracting(false)
      self:set_retraction_start_time(0)
      self:set_last_retracted_line_number(visible_line_indices[1])

      table.remove(visible_line_indices, 1)

      self:update_height()
    end
  else
    if self:get_retraction_start_time() == 0 then
      self:set_retraction_start_time(current_ms + self:get_retraction_timeout())
    elseif current_ms > self:get_retraction_time() then
      self:set_retracting(true)
    end
  end
end

function RetractableTextWidget:layout_text()
  TextWidget.TextWidget.layout_text(self)

  self:update_visible_line_indices()
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

  local visible_lines = self:get_visible_lines()

  if #visible_lines <= 0 then
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

  local start_y_offset = 0
  local line_height = self:get_line_height()

  print(string.format('Rendering %s lines', #visible_lines))

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

  self:layout_text()
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

