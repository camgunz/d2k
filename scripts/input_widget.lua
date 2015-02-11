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

InputWidget.PROMPT_THICKNESS = 2
InputWidget.CURSOR_THICKNESS = 2

function InputWidget:new(iw)
  iw = iw or {}

  iw.cursor_color = iw.cursor_color or {0.8, 0.8, 0.8, 1.0}
  iw.prompt_color = iw.prompt_color or {1.0, 1.0, 1.0, 1.0}

  iw.cursor = 0
  iw.cursor_active = 0
  iw.cursor_trailing = 0

  setmetatable(iw, self)
  self.__index = self

  iw:set_name('Input Widget')
  iw:build_layout()

  return iw
end

function InputWidget:get_height()
  local input_text = self:get_text()
  local set_dummy_text = false

  if not input_text then
    self.input:set_text('DOOM')
    set_dummy_text = true
  end

  self:update_layout_if_needed()

  local layout_width, layout_height = self.layout:get_pixel_size()

  if set_dummy_text then
    self.input:set_text('')
  end

  return self.top_margin + layout_height + self.bottom_margin
end

function InputWidget:tick()
  local layout_width, layout_height = self.layout:get_pixel_size()
  local new_height = self.top_margin + layout_height + self.bottom_margin

  if self.height ~= new_height then
    self:set_height(new_height)
    self.layout:set_width(
      self.width - (self.left_margin + self.right_margin + (new_height / 2.0))
    )
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
  local cursor_line_number, cursor_line_x = self.layout:index_to_line_x(
    self.cursor, self.cursor_trailing
  )
  local cursor_position = self.layout:index_to_pos(self.cursor)
  local strong_pos, weak_pos = self.layout:get_cursor_pos(self.cursor)

  cursor_line_x = cursor_line_x / Pango.SCALE

  print(string.format(
    'cursor, trailing: %d, %s', self.cursor, self.cursor_trailing
  ))
  print(string.format('index_to_line_x: line #, line X: %d, %s',
    cursor_line_number, cursor_line_x
  ))
  print(string.format('strong position: %dx%d+%d+%d',
    strong_pos.x / Pango.SCALE,
    strong_pos.y / Pango.SCALE,
    strong_pos.width / Pango.SCALE,
    strong_pos.height / Pango.SCALE
  ))
  print(string.format('weak position: %dx%d+%d+%d',
    weak_pos.x / Pango.SCALE,
    weak_pos.y / Pango.SCALE,
    weak_pos.width / Pango.SCALE,
    weak_pos.height / Pango.SCALE
  ))

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

    if line_number == cursor_line_number then
      cr:save()

      cr:set_source_rgba(
        self.cursor_color[1],
        self.cursor_color[2],
        self.cursor_color[3],
        self.cursor_color[4]
      )

      cr:move_to(line_start_x + cursor_line_x, line_start_y)
      cr:line_to(
        line_start_x + cursor_line_x, line_start_y + line_logical_height
      )
      cr:set_line_width(InputWidget.CURSOR_THICKNESS)
      cr:stroke()

      cr:restore()
    end

    if line_start_y >= text_height then
      break
    end

    line_number = line_number + 1
  until not iter:next_line()

  cr:restore()
end

function InputWidget:show_previous_command()
end

function InputWidget:show_next_command()
end

function InputWidget:move_cursor_left()
  if self.cursor == 0 then
    print('cursor is 0')
    return
  end

  print(string.format('move_cursor_left: %d, %d',
    self.cursor, self.cursor_trailing
  ))

  local text_length = #self.text
  local cursor = self.cursor
  local trailing = self.cursor_trailing

  if cursor >= text_length then
    cursor = text_length - 1
    trailing = 0
  end

  local new_index, new_trailing = self.layout:move_cursor_visually(
    true, cursor, trailing, -1
  )

  print(string.format('move_cursor_left: %d, %d', new_index, new_trailing))

  self.cursor = new_index + new_trailing
  self.cursor_trailing = new_trailing
end

function InputWidget:move_cursor_right()
  local text_length = #self.text

  if self.cursor == text_length then
    return
  end

  local new_index, new_trailing = self.layout:move_cursor_visually(
    true, self.cursor, self.cursor_trailing, 1
  )

  if new_index == GLib.MAXINT32 then
    print('Cursor moved off the end of the layout')
    new_index = text_length
  elseif new_index < 0 then
    print('Cursor moved off the beginning of the layout')
    new_index = 0
  end

  print(string.format('move_cursor_left: %d, %d', new_index, new_trailing))

  self.cursor = new_index + new_trailing
  self.cursor_trailing = new_trailing
end

function InputWidget:remove_previous_character()
  local old_pos = self.cursor
  local text_length = #self.text

  if self.cursor == 0 then
    return
  end

  if self.cursor == (text_length - 1) then
    self.text = self.text:sub(1, text_length - 1)
  else
    self.text = self.text:sub(1, self.cursor - 1) ..
                self.text:sub(self.cursor + 1)
  end

  self.needs_updating = true
end

function InputWidget:remove_next_character()
  local old_pos = self.cursor
  local text_length = #self.text

  if self.cursor == (text_length - 1) then
    return
  end

  if self.cursor == 0 then
    self.text = self.text:sub(2)
  else
    self.text = self.text:sub(1, self.cursor - 2) ..
                self.text:sub(self.cursor)
  end

  self.needs_updating = true
end

function InputWidget:insert_character(char)
  local old_pos = self.cursor
  local start_text = ''
  local text_length = #self.text
  
  if self.cursor == 0 then
    self.text = char .. self.text
  elseif self.cursor == (text_length - 1) then
    self.text = self.text .. char
  else
    self.text = self.text:sub(1, self.cursor) ..
                char ..
                self.text:sub(self.cursor + 1)
  end

  self.layout:set_text(self.text, -1)

  self.needs_updating = true
  self:update_layout_if_needed()

  local new_index, new_trailing = self.layout:move_cursor_visually(
    true, self.cursor, self.cursor_trailing, 1
  )

  if new_index == GLib.MAXINT32 then
    print('Cursor moved off the end of the layout')
    new_index = #self.text
  elseif new_index < 0 then
    print('Cursor moved off the beginning of the layout')
    new_index = 0
  end

  self.cursor = new_index + new_trailing
  self.cursor_trailing = new_trailing

  print(string.format('Inserting [%s] at [%d] (%s), (%s), new cursor: %d (%d)',
    char,
    old_pos,
    self.text,
    self.layout:get_text(),
    self.cursor,
    self.cursor_trailing
  ))

end

return {InputWidget = InputWidget}

-- vi: et ts=2 sw=2

