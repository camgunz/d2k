-------------------------------------------------------------------------------
--                                                                           --
-- D2K: A Doom Source Port for the 21st Century                              --
--                                                                           --
-- Copyright (C) 2014: See COPYRIGHT file                                    --
--                                                                           --
-- This file is part of D2K.                                                 --
--                                                                           --
-- D2K is free sofiware: you can redistribute it and/or modify it under the  --
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

local lgi = require('lgi')
local GLib = lgi.GLib
local Cairo = lgi.cairo
local Pango = lgi.Pango
local PangoCairo = lgi.PangoCairo
local utf8 = require('utf8')
local TextWidget = require('text_widget')

local InputWidget = TextWidget.TextWidget:new({
  use_markup = false
})

InputWidget.PROMPT_THICKNESS = 1.1
InputWidget.CURSOR_THICKNESS = 1.1

function InputWidget:new(iw)
  iw = iw or {}

  iw.cursor_color = iw.cursor_color or {0.8, 0.8, 0.8, 1.0}
  iw.prompt_color = iw.prompt_color or {1.0, 1.0, 1.0, 1.0}
  iw.cursor_blink_speed = iw.cursor_blink_speed or 500

  iw.cursor = 0
  iw.cursor_active = 0
  iw.cursor_trailing = 0
  iw.cursor_timer = d2k.System.get_ticks()
  iw.history = {}
  iw.history_index = 0

  setmetatable(iw, self)
  self.__index = self

  iw:build_layout()

  return iw
end

function InputWidget:set_text(text)
  TextWidget.TextWidget.set_text(self, text)

  if self.cursor >= #self:get_text() - 1 then
    self.cursor = 0
    self.cursor_trailing = 0
  end
end

function InputWidget:get_height()
  local input_text = self:get_text()
  local set_dummy_text = false

  if #input_text == 0 then
    self.layout:set_text('DOOM')
    set_dummy_text = true
  end

  self:update_layout_if_needed()

  local layout_width, layout_height = self.layout:get_pixel_size()

  if set_dummy_text then
    self.layout:set_text('')
  end

  return self.top_margin + layout_height + self.bottom_margin
end

function InputWidget:tick()
  local layout_width, layout_height = self.layout:get_pixel_size()
  local new_height = self.top_margin + layout_height + self.bottom_margin
  local ticks = d2k.System.get_ticks()

  if self.height ~= new_height then
    self:set_height(new_height)
  end

  if ticks > self.cursor_timer + self.cursor_blink_speed then
    self.cursor_active = not self.cursor_active
    self.cursor_timer = ticks
  end
end

function InputWidget:draw()
  local cr = d2k.overlay.render_context
  local layout_width, layout_height = self.layout:get_pixel_size()
  local lh_fracunit = 4.0
  local lh_frac = layout_height / lh_fracunit
  local lh_half = layout_height / 2.0
  local prompt_width = self.height / 2.0
  local height = self:get_height()

  cr:save()

  cr:set_operator(Cairo.Operator.OVER)

  cr:reset_clip()
  cr:new_path()
  cr:rectangle(self.x, self.y, self.width, self:get_height())
  cr:clip()

  cr:set_source_rgba(
    self.bg_color[1],
    self.bg_color[2],
    self.bg_color[3],
    self.bg_color[4]
  )
  cr:paint()

  cr:reset_clip()
  cr:new_path()
  cr:rectangle(
    self.x + self.left_margin,
    self.y + self.top_margin,
    self.width - (self.left_margin + self.right_margin),
    height - (self.top_margin + self.bottom_margin)
  )
  cr:clip()

  cr:set_source_rgba(
    self.prompt_color[1],
    self.prompt_color[2],
    self.prompt_color[3],
    self.prompt_color[4]
  )

  cr:move_to(self.x + self.left_margin, self.y + self.top_margin + lh_frac)
  cr:line_to(
    self.x + self.left_margin + lh_half, self.y + self.top_margin + lh_half
  )
  cr:line_to(
    self.x + self.left_margin,
    self.y + self.top_margin + (layout_height - lh_frac)
  )

  cr:set_line_width(InputWidget.PROMPT_THICKNESS)
  cr:stroke();

  cr:set_source_rgba(
    self.fg_color[1],
    self.fg_color[2],
    self.fg_color[3],
    self.fg_color[4]
  )

  local lx = self.x + self.left_margin + prompt_width
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

  if self.cursor_active then
    local cursor_pos = self.layout:index_to_pos(self.cursor)
    cursor_pos.x = cursor_pos.x / Pango.SCALE
    cursor_pos.y = cursor_pos.y / Pango.SCALE
    cursor_pos.width = cursor_pos.width / Pango.SCALE
    cursor_pos.height = cursor_pos.height / Pango.SCALE

    if self.cursor_trailing then
      cursor_pos.x = cursor_pos.x + (cursor_pos.width * self.cursor_trailing)
    end

    cr:set_source_rgba(
      self.cursor_color[1],
      self.cursor_color[2],
      self.cursor_color[3],
      self.cursor_color[4]
    )

    cr:move_to(lx + cursor_pos.x, ly + cursor_pos.y)
    cr:line_to(lx + cursor_pos.x, ly + cursor_pos.y + cursor_pos.height)
    cr:set_line_width(InputWidget.CURSOR_THICKNESS)
    cr:stroke()
  end

  cr:restore()
end

function InputWidget:save_text_as_command()
  table.insert(self.history, self:get_text())
  self.history_index = 0
end

function InputWidget:show_previous_command()
  local command_count = #self.history

  if self.history_index < command_count then
    self.history_index = self.history_index + 1
    self:set_text(self.history[command_count - (self.history_index - 1)])
  end
end

function InputWidget:show_next_command()
  local command_count = #self.history

  if self.history_index == 0 then
    self:set_text('')
  else
    self.history_index = self.history_index - 1
    self:set_text(self.history[command_count - (self.history_index - 1)])
  end
end

function InputWidget:print_cursor_stats(s)
  return
  --[[
  local cursor_position = self.layout:index_to_pos(self.cursor)
  local cursor_line_number, cursor_line_x = self.layout:index_to_line_x(
    self.cursor, self.cursor_trailing
  )

  print(string.format('%s: cursor, trailing, text length, line #, line x, pos: %d, %d, %d, %d, %d, %dx%d+%d+%d',
    s,
    self.cursor,
    self.cursor_trailing,
    #self:get_text(),
    cursor_line_number,
    cursor_line_x / Pango.SCALE,
    cursor_position.x / Pango.SCALE,
    cursor_position.y / Pango.SCALE,
    cursor_position.width / Pango.SCALE,
    cursor_position.height / Pango.SCALE
  ))
  --]]
end

function InputWidget:move_cursor_left()
  self:print_cursor_stats('move_cursor_left (before)')

  if self.cursor == 0 and self.cursor_trailing == 0 then
    return
  end

  self.cursor, self.cursor_trailing = self.layout:move_cursor_visually(
    true, self.cursor, self.cursor_trailing, -1
  )

  if self.cursor == GLib.MAXINT32 then
    error('Cursor moved off the end of the layout')
  elseif self.cursor < 0 then
    error('Cursor moved off the beginning of the layout')
  end

  self:activate_cursor()

  self:print_cursor_stats('move_cursor_left (after)')
end

function InputWidget:move_cursor_right()
  self:print_cursor_stats('move_cursor_right (before)')

  local text_length = #self:get_text()

  if (self.cursor + self.cursor_trailing) == text_length then
    return
  end

  self.cursor, self.cursor_trailing = self.layout:move_cursor_visually(
    true, self.cursor, self.cursor_trailing, 1
  )

  if self.cursor == GLib.MAXINT32 then
    error('Cursor moved off the end of the layout')
  elseif self.cursor < 0 then
    error('Cursor moved off the beginning of the layout')
  end

  self:activate_cursor()

  self:print_cursor_stats('move_cursor_right (after)')
end

function InputWidget:move_cursor_to_start()
  self.cursor = 0
  self.cursor_trailing = 0

  self:activate_cursor()
end

function InputWidget:move_cursor_to_end()
  local text_length = #self:get_text()

  if text_length > 0 then
    self.cursor = text_length - 1
    self.cursor_trailing = 1
  end

  self:activate_cursor()
end

function InputWidget:set_external_text_source(get_text,
                                              text_updated,
                                              clear_text_updated)
  error(string.format('%s: Cannot set external text source on input widgets',
    self.name
  ))
end

function InputWidget:delete_previous_character()
  local index = self.cursor + self.cursor_trailing
  local text = self:get_text()
  local text_length = #text

  if self.cursor == 0 then
    return
  end

  self:move_cursor_left()

  if index == text_length then
    self:set_text(text:sub(1, text_length - 1))
  else
    self:set_text(text:sub(1, index - 1) .. text:sub(index + 1))
  end

  self:update_layout_if_needed()
  self:activate_cursor()
end

function InputWidget:delete_next_character()
  local index = self.cursor + self.cursor_trailing
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

  self.needs_updating = true
  self:activate_cursor()
end

function InputWidget:insert_character(char)
  self:print_cursor_stats('insert_character (before)')

  local old_pos = self.cursor
  local start_text = ''
  local text = self:get_text()
  local text_length = #text
  local index = self.cursor + self.cursor_trailing

  if index == 0 then
    self:set_text(char .. text)
  elseif index == text_length then
    self:set_text(text .. char)
  else
    self:set_text(text:sub(1, index) .. char .. text:sub(index + 1))
  end

  self.layout:set_text(self:get_text(), -1)

  self.needs_updating = true
  self:update_layout_if_needed()

  self:print_cursor_stats('insert_character (after)')

  self:move_cursor_right()

  self:activate_cursor()
end

function InputWidget:activate_cursor()
  self.cursor_active = true
  self.cursor_timer = d2k.System.get_ticks()
end

return {InputWidget = InputWidget}

-- vi: et ts=2 sw=2

