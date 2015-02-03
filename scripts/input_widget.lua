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
local Cairo = lgi.cairo
local Pango = lgi.Pango
local PangoCairo = lgi.PangoCairo
local TextWidget = require('text_widget')

local InputWidget = TextWidget.TextWidget:new()

InputWidget.PROMPT_THICKNESS = 2
InputWidget.CURSOR_THICKNESS = 2

function InputWidget:new(iw)
  iw = iw or {}

  iw.cursor_color = iw.cursor_color or {0.8, 0.8, 0.8, 1.0}

  iw.cursor = 0
  iw.cursor_active = 0

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
  local strong, weak = self.layout:get_cursor_pos(self.cursor)

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
    self.fg_color[1],
    self.fg_color[2],
    self.fg_color[3],
    self.fg_color[4]
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

  local strong_cursor, weak_cursor = self.layout:get_cursor_pos(self.cursor)
  local curs_x = strong_cursor.x / Pango.SCALE
  local curs_y = strong_cursor.y / Pango.SCALE
  local curs_width = strong_cursor.width / Pango.SCALE
  local curs_height = strong_cursor.height / Pango.SCALE
  local prompt_width = self.height / 2.0

  cr:set_source_rgba(
    self.cursor_color[1],
    self.cursor_color[2],
    self.cursor_color[3],
    self.cursor_color[4]
  )

  cr:move_to(
    self.x + self.left_margin + curs_x + prompt_width,
    self.y + self.top_margin + curs_y
  )
  cr:line_to(
    self.x + self.left_margin + curs_x + prompt_width,
    self.y + self.top_margin + curs_y + self.height - self.bottom_margin
  )
  cr:set_line_width(InputWidget.CURSOR_THICKNESS)
  cr:stroke()

  cr:restore()
end

function InputWidget:show_previous_command()
end

function InputWidget:show_next_command()
end

function InputWidget:move_cursor_left()
end

function InputWidget:move_cursor_right()
end

function InputWidget:insert_text(text)
  print(string.format('Inserting [%s] at [%d]', text, self.cursor))
end

return {InputWidget = InputWidget}

-- vi: et ts=2 sw=2

