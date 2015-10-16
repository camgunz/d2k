-------------------------------------------------------------------------------
--                                                                           --
-- D2K: A Doom Source Port for the 21st Century                              --
--                                                                           --
-- Copyright (C) 2014: See COPYRIGHT file                                    --
--                                                                           --
-- This file is part of D2K.                                                 --
--                                                                           --
-- D2K is free software: you can redistribute it and/or modify it under the  --
-- terms of the GNU General Public License as published by the Free Sofiware --
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
local GLib = lgi.GLib
local Cairo = lgi.cairo
local Pango = lgi.Pango
local PangoCairo = lgi.PangoCairo
local TextWidget = require('text_widget')

InputWidget = class('InputWidget', TextWidget.TextWidget)

InputWidget.use_markup = false

local PROMPT_THICKNESS = 1.1
local CURSOR_THICKNESS = 1.1

function InputWidget:initialize(iw)
  iw = iw or {}

  TextWidget.TextWidget.initialize(self, iw)

  self.cursor_color = iw.cursor_color or {0.8, 0.8, 0.8, 1.0}
  self.prompt_color = iw.prompt_color or {1.0, 1.0, 1.0, 1.0}
  self.cursor_blink_speed = iw.cursor_blink_speed or 500

  self.cursor = 0
  self.cursor_active = 0
  self.cursor_trailing = 0
  self.cursor_timer = d2k.System.get_ticks()
  self.history = {}
  self.history_index = 0
  self.scroll_offset = 0
  self.input_handler = iw.input_handler or function(input) end
  self.deactivate_on_input = iw.deactivate_on_input or false
end

function InputWidget:get_cursor_color()
  return self.cursor_color
end

function InputWidget:set_cursor_color(cursor_color)
  self.cursor_color = cursor_color
  self:handle_display_change()
end

function InputWidget:get_prompt_color()
  return self.prompt_color
end

function InputWidget:set_prompt_color(prompt_color)
  self.prompt_color = prompt_color
  self:handle_display_change()
end

function InputWidget:get_cursor_blink_speed()
  return self.cursor_blink_speed
end

function InputWidget:set_cursor_blink_speed(cursor_blink_speed)
  self.cursor_blink_speed = cursor_blink_speed
end

function InputWidget:get_cursor()
  return self.cursor
end

function InputWidget:set_cursor(cursor)
  self.cursor = cursor
  self:handle_display_change()
end

function InputWidget:get_cursor_active()
  return self.cursor_active
end

function InputWidget:set_cursor_active(cursor_active)
  self.cursor_active = cursor_active
  self:handle_display_change()
end

function InputWidget:toggle_cursor_active()
  self:set_cursor_active(not self:get_cursor_active())
  self:handle_display_change()
end

function InputWidget:get_cursor_trailing()
  return self.cursor_trailing
end

function InputWidget:set_cursor_trailing(cursor_trailing)
  self.cursor_trailing = cursor_trailing
end

function InputWidget:get_cursor_timer()
  return self.cursor_timer
end

function InputWidget:set_cursor_timer(cursor_timer)
  self.cursor_timer = cursor_timer
end

function InputWidget:get_history()
  return self.history
end

function InputWidget:set_history(history)
  self.history = history
end

function InputWidget:get_history_index()
  return self.history_index
end

function InputWidget:set_history_index(history_index)
  self.history_index = history_index
end

function InputWidget:get_scroll_offset()
  return self.scroll_offset
end

function InputWidget:set_scroll_offset(scroll_offset)
  self.scroll_offset = scroll_offset
end

function InputWidget:get_deactivate_on_input()
  return self.deactivate_on_input
end

function InputWidget:set_deactivate_on_input(deactivate_on_input)
  self.deactivate_on_input = deactivate_on_input
end

function InputWidget:set_text(text)
  TextWidget.TextWidget.set_text(self, text)

  local text_length = #self:get_text()

  if text_length == 0 then
    self:set_cursor(0)
    self:set_cursor_trailing(0)
  elseif self:get_cursor() > text_length - 1 then
    self:set_cursor(text_length - 1)
    self:set_cursor_trailing(1)
  end
end

function InputWidget:tick()
  if not self:is_active() then
    return
  end

  local line_height = self:get_font_height()
  local layout_width = self:get_layout_pixel_width()
  local layout_height = self:get_layout_pixel_height()
  local text_height = math.max(line_height, layout_height)
  local height = self:get_top_margin() + text_height + self:get_bottom_margin()
  local ticks = d2k.System.get_ticks()

  if self:get_height_in_pixels() ~= height then
    self:set_height_in_pixels(height)
  end

  if ticks > self:get_cursor_timer() + self:get_cursor_blink_speed() then
    self:toggle_cursor_active()
    self:set_cursor_timer(ticks)
  end
end

function InputWidget:render()
  local cr = d2k.overlay.render_context
  local line_height = self:get_font_height()
  local layout_width = self:get_layout_pixel_width()
  local layout_height = self:get_layout_pixel_height()
  local text_height = math.max(line_height, layout_height)
  local height = self:get_top_margin() + text_height + self:get_bottom_margin()
  local h_quart = text_height / 4.0
  local h_half = h_quart * 2.0
  local fg_color = self:get_fg_color()
  local bg_color = self:get_bg_color()
  local cursor_color = self:get_cursor_color()
  local prompt_color = self:get_prompt_color()
  local prompt_width = text_height

  cr:save()

  cr:set_operator(Cairo.Operator.OVER)

  cr:reset_clip()
  cr:new_path()
  cr:rectangle(self:get_x(), self:get_y(), self:get_width_in_pixels(), height)
  cr:clip()

  cr:set_source_rgba(bg_color[1], bg_color[2], bg_color[3], bg_color[4])
  cr:paint()

  cr:reset_clip()
  cr:new_path()
  cr:rectangle(
    self:get_x() + self:get_left_margin(),
    self:get_y() + self:get_top_margin(),
    self:get_width_in_pixels() - (
      self:get_left_margin() + self:get_right_margin()
    ),
    height - (self:get_top_margin() + self:get_bottom_margin())
  )
  cr:clip()

  cr:set_source_rgba(
    prompt_color[1],
    prompt_color[2],
    prompt_color[3],
    prompt_color[4]
  )

  cr:move_to(
    self:get_x() + self:get_left_margin(),
    self:get_y() + self:get_top_margin() + h_quart
  )
  cr:line_to(
    self:get_x() + self:get_left_margin() + h_half,
    self:get_y() + self:get_top_margin() + h_half
  )
  cr:line_to(
    self:get_x() + self:get_left_margin(),
    self:get_y() + self:get_top_margin() + h_half + h_quart
  )

  cr:set_line_width(PROMPT_THICKNESS)
  cr:stroke();

  local lx = self:get_x() + self:get_left_margin() + prompt_width
  local ly = self:get_y() + self:get_top_margin()
  local text_width = self:get_width_in_pixels() - (
    self:get_left_margin() + self:get_right_margin()
  )

  if self:get_vertical_alignment() == TextWidget.ALIGN_CENTER then
    ly = ly + (text_height / 2) - h_half
  elseif self:get_vertical_alignment() == TextWidget.ALIGN_BOTTOM then
    ly = ly + text_height - height
  end

  if self:get_horizontal_alignment() == TextWidget.ALIGN_CENTER then
    lx = (text_width / 2) - h_half
  elseif self:get_horizontal_alignment() == TextWidget.ALIGN_RIGHT then
    lx = lx + text_width - layout_width
  end

  if layout_width > text_width then
    lx = lx - self:get_horizontal_offset()
  end

  if layout_height > text_height then
    ly = ly - self:get_vertical_offset()
  end

  local cursor_pos = self:get_layout():index_to_pos(self:get_cursor())
  cursor_pos.x = cursor_pos.x / Pango.SCALE
  cursor_pos.y = cursor_pos.y / Pango.SCALE
  cursor_pos.width = cursor_pos.width / Pango.SCALE
  cursor_pos.height = cursor_pos.height / Pango.SCALE

  if self:get_cursor_trailing() then
    cursor_pos.x =
      cursor_pos.x + (cursor_pos.width * self:get_cursor_trailing())
  end

  if cursor_pos.x > (text_width - prompt_width) + self:get_scroll_offset() then
    self:set_scroll_offset(
      cursor_pos.x - (text_width - prompt_width)
    )
  elseif cursor_pos.x < self:get_scroll_offset() then
    self:set_scroll_offset(cursor_pos.x)
  end

  lx = lx - self:get_scroll_offset()

  cr:reset_clip()
  cr:new_path()
  cr:rectangle(
    self:get_x() + self:get_left_margin() + prompt_width,
    self:get_y(),
    text_width - prompt_width,
    height
  )
  cr:clip()

  if self:get_cursor_active() then
    cr:set_source_rgba(
      cursor_color[1], cursor_color[2], cursor_color[3], cursor_color[4]
    )

    cursor_pos.width  = math.max(cursor_pos.width, 4)
    cursor_pos.height = math.max(cursor_pos.height, text_height)
    cursor_pos.x      = math.max(cursor_pos.x, cursor_pos.width / 2)

    cr:move_to(lx + cursor_pos.x, ly + cursor_pos.y)
    cr:line_to(lx + cursor_pos.x, ly + cursor_pos.y + cursor_pos.height)
    cr:set_line_width(CURSOR_THICKNESS)
    cr:stroke()
  end

  cr:set_source_rgba(fg_color[1], fg_color[2], fg_color[3], fg_color[4])

  local iter = self:get_layout():get_iter()
  local line_number = 0
  repeat
    local line = iter:get_line_readonly()
    local line_ink_extents, line_logical_extents = iter:get_line_extents()
    local line_logical_x = line_logical_extents.x / Pango.SCALE
    local line_logical_y = line_logical_extents.y / Pango.SCALE
    local line_logical_height = line_logical_extents.height / Pango.SCALE
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

function InputWidget:save_text_into_history()
  table.insert(self:get_history(), self:get_text())
  self:set_history_index(0)
end

function InputWidget:show_previous_history_entry()
  local history = self:get_history()
  local command_count = #history
  local history_index = self:get_history_index()
  local new_history_index = history_index + 1

  if history_index == command_count then
    return
  end

  self:set_history_index(new_history_index)
  self:set_text(history[command_count - (new_history_index - 1)])
end

function InputWidget:show_next_history_entry()
  local history = self:get_history()
  local command_count = #history
  local history_index = self:get_history_index()
  local new_history_index = history_index - 1

  if history_index == 0 then
    return
  end

  self:set_history_index(new_history_index)

  if new_history_index == 0 then
    self:set_text('')
  else
    self:set_text(history[command_count - (new_history_index - 1)])
  end
end

function InputWidget:print_cursor_stats(s)
  --[[
  local cursor_position = self:get_layout():index_to_pos(self:get_cursor())
  local cursor_line_number, cursor_line_x = self:get_layout():index_to_line_x(
    self:get_cursor(), self:get_cursor_trailing()
  )

  print(string.format(
    '%s: cursor, trailing, text length, line #, line x, pos: ' ..
    '%d, %d, %d, %d, %d, %dx%d+%d+%d',
    s,
    self:get_cursor(),
    self:get_cursor_trailing(),
    #self:get_text(),
    cursor_line_number,
    cursor_line_x / Pango.SCALE,
    cursor_position.x / Pango.SCALE,
    cursor_position.y / Pango.SCALE,
    cursor_position.width / Pango.SCALE,
    cursor_position.height / Pango.SCALE
  ))
  --]]

  return
end

function InputWidget:move_cursor_visually(strong, direction)
  local cursor, cursor_trailing = self:get_layout():move_cursor_visually(
    strong, self:get_cursor(), self:get_cursor_trailing(), direction
  )

  self:set_cursor(cursor)
  self:set_cursor_trailing(cursor_trailing)
end

function InputWidget:move_cursor_left()
  self:print_cursor_stats('move_cursor_left (before)')

  if self:get_cursor() == 0 and self:get_cursor_trailing() == 0 then
    return
  end

  self:move_cursor_visually(true, -1)

  if self:get_cursor() == GLib.MAXINT32 then
    error('Cursor moved off the end of the layout')
  elseif self:get_cursor() < 0 then
    error('Cursor moved off the beginning of the layout')
  end

  self:activate_cursor()

  self:print_cursor_stats('move_cursor_left (after)')
end

function InputWidget:move_cursor_right()
  self:print_cursor_stats('move_cursor_right (before)')

  local text_length = #self:get_text()

  if (self:get_cursor() + self:get_cursor_trailing()) == text_length then
    return
  end

  self:move_cursor_visually(true, 1)

  if self:get_cursor() == GLib.MAXINT32 then
    error('Cursor moved off the end of the layout')
  elseif self:get_cursor() < 0 then
    error('Cursor moved off the beginning of the layout')
  end

  self:activate_cursor()

  self:print_cursor_stats('move_cursor_right (after)')
end

function InputWidget:move_cursor_to_start()
  self:set_cursor(0)
  self:set_cursor_trailing(0)

  self:activate_cursor()
end

function InputWidget:move_cursor_to_end()
  local text_length = #self:get_text()

  if text_length > 0 then
    self:set_cursor(text_length - 1)
    self:set_cursor_trailing(1)
  end

  self:activate_cursor()
end

function InputWidget:set_external_text_source(get_text,
                                              text_updated,
                                              clear_text_updated)
  error(string.format('%s: Cannot set external text source on input widgets',
    self:get_name()
  ))
end

function InputWidget:delete_previous_character()
  local index = self:get_cursor() + self:get_cursor_trailing()
  local text = self:get_text()
  local text_length = #text

  if index == 0 then
    return
  end

  self:move_cursor_left()

  if index == text_length then
    self:set_text(text:sub(1, text_length - 1))
  else
    self:set_text(text:sub(1, index - 1) .. text:sub(index + 1))
  end

  self:activate_cursor()
end

function InputWidget:delete_next_character()
  local index = self:get_cursor() + self:get_cursor_trailing()
  local text = self:get_text()
  local text_length = #text

  if index == text_length then
    return
  end

  if index == 0 then
    self:set_text(text:sub(index + 2))
  else
    self:set_text(text:sub(1, index) .. text:sub(index + 2))
  end

  self:activate_cursor()

  self:handle_content_change()
end

function InputWidget:insert_character(char)
  local old_pos = self:get_cursor()
  local start_text = ''
  local text = self:get_text()
  local text_length = #text
  local index = self:get_cursor() + self:get_cursor_trailing()

  if index == 0 then
    self:set_text(char .. text)
  elseif index == text_length then
    self:set_text(text .. char)
  else
    self:set_text(text:sub(1, index) .. char .. text:sub(index + 1))
  end

  self:get_layout():set_text(self:get_text(), -1)

  self:move_cursor_right()
  self:activate_cursor()

  self:handle_content_change()
end

function InputWidget:activate_cursor()
  self:set_cursor_active(true)
  self:set_cursor_timer(d2k.System.get_ticks())
end

function InputWidget:handle_event(event)
  if event:is_key_press(d2k.Key.UP) then
    self:show_previous_history_entry()
    return true
  elseif event:is_key_press(d2k.Key.DOWN) then
    self:show_next_history_entry()
    return true
  elseif event:is_key_press(d2k.Key.LEFT) then
    self:move_cursor_left()
    return true
  elseif event:is_key_press(d2k.Key.RIGHT) then
    self:move_cursor_right()
    return true
  elseif event:is_key_press(d2k.Key.DELETE) then
    self:delete_next_character()
    return true
  elseif event:is_key_press(d2k.Key.BACKSPACE) then
    self:delete_previous_character()
    return true
  elseif event:is_key_press(d2k.Key.HOME) then
    self:move_cursor_to_start()
    return true
  elseif event:is_key_press(d2k.Key.END) then
    self:move_cursor_to_end()
    return true
  elseif event:is_key_press(d2k.Key.RETURN) or
         event:is_key_press(d2k.Key.KP_ENTER) then
    self.input_handler(self:get_text())
    self:save_text_into_history()
    self:clear()
    if self:get_deactivate_on_input() then
      self:deactivate()
    end
    return true
  elseif event:is_key() and event:is_press() then
    local char = event:get_char()

    if char then
      self:insert_character(event:get_char())
    end

    return true
  end

  return false
end

return {
  InputWidget      = InputWidget,
  PROMPT_THICKNESS = PROMPT_THICKNESS,
  CURSOR_THICKNESS = CURSOR_THICKNESS
}

-- vi: et ts=2 sw=2

