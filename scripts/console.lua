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
local TextWidget = require('text_widget')
local InputWidget = require('input_widget')

local Console = Widget.Widget:new({
  EXTENSION_TIME           = 1200.0,
  RETRACTION_TIME          = 1200.0,
  MARGIN                   = 8,
  HORIZONTAL_SCROLL_AMOUNT = 6,
  VERTICAL_SCROLL_AMOUNT   = 12,
  FG_COLOR                 = {1.0, 1.0, 1.0, 1.0},
  BG_COLOR                 = {  0,   0,   0, 0.85},
})

function Console:new(c)
  c = c or {}

  c.extension_time = c.extension_time or Console.EXTENSION_TIME
  c.retraction_time = c.retraction_time or Console.RETRACTION_TIME
  c.max_height = c.max_height or d2k.Video.get_screen_height() / 2

  c.max_width = d2k.Video.get_screen_width()
  c.width = c.max_width
  c.height = c.max_height
  c.scroll_rate = 0.0
  c.last_scroll_ms = 0

  c.input = InputWidget.InputWidget:new({
    x = 0,
    y = 0,
    width = c.max_width,
    height = 0,
    top_margin = c.input_margin or Console.MARGIN,
    bottom_margin = c.bottom_margin or Console.MARGIN,
    left_margin = c.left_margin or Console.MARGIN,
    right_margin = c.right_margin or Console.MARGIN,
    horizontal_alignment = TextWidget.ALIGN_LEFT,
    vertical_alignment = TextWidget.ALIGN_CENTER,
    fg_color = c.fg_color or Console.FG_COLOR,
    bg_color = c.bg_color or Console.BG_COLOR,
  })

  c.output = TextWidget.TextWidget:new({
    x = 0,
    y = 0,
    width = c.max_width,
    height = c.max_height,
    top_margin = c.top_margin or 0,
    bottom_margin = c.bottom_margin or Console.MARGIN,
    left_margin = c.left_margin or Console.MARGIN,
    right_margin = c.right_margin or Console.MARGIN,
    horizontal_alignment = TextWidget.ALIGN_LEFT,
    vertical_alignment = TextWidget.ALIGN_BOTTOM,
    fg_color = c.fg_color or Console.FG_COLOR,
    bg_color = c.bg_color or Console.BG_COLOR,
    word_wrap = TextWidget.WRAP_WORD
  })

  setmetatable(c, self)
  self.__index = self

  c:set_name('Console Output')

  return c
end

function Console:get_name()
  return 'Console'
end

function Console:reset()
  self.output:reset()
end

function Console:tick()
  if self.scroll_rate ~= 0 then
    local current_ms = d2k.System.get_ticks()

    if self.last_scroll_ms == 0 then
      self.last_scroll_ms = current_ms
    end

    local ms_elapsed = current_ms - self.last_scroll_ms

    self.height = self.height + (self.scroll_rate * ms_elapsed)

    if self.height < 0 then
      self.height = 0
      self.scroll_rate = 0
    elseif self.height > self.max_height then
      self.height = self.max_height
      self.scroll_rate = 0
    end
  end

  self.input:tick()
  self.input:set_y(self.height - self.input:get_height())

  self.output:tick()
  self.output:set_height(self.input:get_y())
  self.output:set_text(string.format('input_y: %d', self.input:get_y()))
end

function Console:draw()
  if self.height <= 0 then
    return
  end

  self.input:draw()
  self.output:draw()
end

function Console:handle_event(event)
  if event:is_key_press(d2k.Key.BACKQUOTE) then
    self:toggle_scroll()
  end

  if self.scroll_rate < 0 or self.height == 0 then
    return
  end

  if d2k.KeyStates.shift_is_down() then
    if event:is_key_press(d2k.Key.UP) then
      self.output:scroll_up(Console.HORIZONTAL_SCROLL_AMOUNT)
      return true
    elseif event:is_key_press(d2k.Key.PAGE_UP) then
      self.output:scroll_up(Console.HORIZONTAL_SCROLL_AMOUNT * 10)
      return true
    elseif event:is_key_press(d2k.Key.DOWN) then
      self.output:scroll_down(Console.HORIZONTAL_SCROLL_AMOUNT)
      return true
    elseif event:is_key_press(d2k.Key.PAGE_DOWN) then
      self.output:scroll_down(Console.HORIZONTAL_SCROLL_AMOUNT * 10)
      return true
    end
  elseif event:is_key_press(d2k.Key.UP) then
    self.input:show_previous_command()
  elseif event:is_key_press(d2k.Key.DOWN) then
    self.input:show_next_command()
  elseif event:is_key_press(d2k.Key.LEFT) then
    self.input:move_cursor_left()
  elseif event:is_key_press(d2k.Key.RIGHT) then
    self.input:move_cursor_right()
  elseif event:is_key() and event:is_press() then
    self.input:insert_text(event:get_char())
  end

  return false
end

function Console:toggle_scroll()
  if self.height == self.max_height then
    self.scroll_rate = -(self.max_height / self.retraction_time)
  elseif self.height == 0 then
    self.scroll_rate = self.max_height / self.extension_time
  elseif self.scroll_rate > 0 then
    self.scroll_rate = -(self.max_height / self.retraction_time)
  elseif self.scroll_rate < 0 then
    self.scroll_rate = self.max_height / self.extension_time
  end

  self.last_scroll_ms = d2k.System.get_ticks()
end


return {Console = Console}

-- vi: et ts=2 sw=2

