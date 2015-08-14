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
local TextWidget = require('text_widget')
local InputWidget = require('input_widget')

local Console = Widget.Widget:new({
  EXTENSION_TIME           = 1200.0,
  RETRACTION_TIME          = 1200.0,
  MARGIN                   = 8,
  HORIZONTAL_SCROLL_AMOUNT = 6,
  VERTICAL_SCROLL_AMOUNT   = 12,
  FG_COLOR                 = {1.0, 1.0, 1.0, 1.00},
  BG_COLOR                 = {0.0, 0.0, 0.0, 0.85},
})

function Console:new(c)
  c = c or {}

  c.extension_time = c.extension_time or Console.EXTENSION_TIME
  c.retraction_time = c.retraction_time or Console.RETRACTION_TIME
  c.max_width = c.max_width or 1
  c.max_height = c.max_height or .5
  c.z_index = c.z_index or 2

  c.width = c.max_width
  c.height = 0
  c.scroll_rate = 0.0
  c.last_scroll_ms = 0

  c.font_description_text = c.font_description_text or
                            d2k.hud.font_description_text

  c.input = InputWidget.InputWidget:new({
    name = 'console input',
    x = 0,
    y = 0,
    width = c.max_width,
    height = 0,
    z_index = c.z_index,
    top_margin = c.input_margin or Console.MARGIN,
    bottom_margin = c.bottom_margin or Console.MARGIN,
    left_margin = c.left_margin or Console.MARGIN,
    right_margin = c.right_margin or Console.MARGIN,
    horizontal_alignment = TextWidget.ALIGN_LEFT,
    vertical_alignment = TextWidget.ALIGN_CENTER,
    fg_color = c.fg_color or Console.FG_COLOR,
    bg_color = c.bg_color or Console.BG_COLOR,
    font_description_text = c.font_description_text
  })

  c.output = TextWidget.TextWidget:new({
    name = 'console output',
    x = 0,
    y = 0,
    width = c.max_width,
    height = c.max_height,
    z_index = c.z_index,
    top_margin = c.top_margin or 0,
    bottom_margin = c.bottom_margin or Console.MARGIN,
    left_margin = c.left_margin or Console.MARGIN,
    right_margin = c.right_margin or Console.MARGIN,
    horizontal_alignment = TextWidget.ALIGN_LEFT,
    vertical_alignment = TextWidget.ALIGN_BOTTOM,
    fg_color = c.fg_color or Console.FG_COLOR,
    bg_color = c.bg_color or Console.BG_COLOR,
    font_description_text = c.font_description_text,
    word_wrap = TextWidget.WRAP_WORD,
    use_markup = true,
    strip_ending_newline = true
  })

  c.output:set_external_text_source(
    d2k.Messaging.get_console_messages,
    d2k.Messaging.get_console_messages_updated,
    d2k.Messaging.clear_console_messages_updated
  )

  setmetatable(c, self)
  self.__index = self

  c:set_name('console')

  return c
end

function Console:get_extension_time()
  return self.extension_time
end

function Console:set_extension_time(extension_time)
  self.extension_time = extension_time
end

function Console:get_retraction_time()
  return self.retraction_time
end

function Console:set_retraction_time(retraction_time)
  self.retraction_time = retraction_time
end

function Console:get_scroll_rate()
  return self.scroll_rate
end

function Console:set_scroll_rate(scroll_rate)
  self.scroll_rate = scroll_rate
end

function Console:get_last_scroll_ms()
  return self.last_scroll_ms
end

function Console:set_last_scroll_ms(last_scroll_ms)
  self.last_scroll_ms = last_scroll_ms
end

function Console:get_input()
  return self.input
end

function Console:set_input(input)
  self.input = input
end

function Console:get_output()
  return self.output
end

function Console:set_output(output)
  self.output = output
end

function Console:is_active()
  return self:get_height_in_pixels() > 0 and self:get_scroll_rate() >= 0.0
end

function Console:get_name()
  return self.name
end

function Console:reset()
  self:get_output():reset()
  self:get_input():reset()
end

function Console:tick()
  if self:get_scroll_rate() ~= 0 then
    local current_ms = d2k.System.get_ticks()

    if self:get_last_scroll_ms() == 0 then
      self:set_last_scroll_ms(current_ms)
    end

    local ms_elapsed = current_ms - self:get_last_scroll_ms()

    self:set_height_in_pixels(math.max(
      0, self:get_height_in_pixels() + (self:get_scroll_rate() * ms_elapsed)
    ))

    if self:get_height_in_pixels() < 0 then
      self:set_height(0)
      self:set_scroll_rate(0)
    elseif self:get_height_in_pixels() > self:get_max_height() then
      self:set_height_in_pixels(self:get_max_height_in_pixels())
      self:set_scroll_rate(0)
    end
  end

  self:get_input():tick()
  self:get_input():set_y(
    self:get_height_in_pixels() - self:get_input():get_height_in_pixels()
  )

  self:get_output():tick()
  self:get_output():set_height_in_pixels(math.max(self:get_input():get_y(), 0))
end

function Console:draw()
  if self:get_height_in_pixels() <= 0 then
    return
  end

  self:get_input():draw()
  self:get_output():draw()
end

function Console:handle_input(input)
  d2k.CommandInterface.handle_input(input);
end

function Console:handle_event(event)
  if event:is_key_press(d2k.Key.BACKQUOTE) then
    self:toggle_scroll()
    return
  end

  if self:get_scroll_rate() < 0 or self:get_height_in_pixels() == 0 then
    return
  end

  if d2k.KeyStates.shift_is_down() then
    if event:is_key_press(d2k.Key.UP) then
      self:get_output():scroll_up(Console.HORIZONTAL_SCROLL_AMOUNT)
      return true
    elseif event:is_key_press(d2k.Key.PAGEUP) then
      self:get_output():scroll_up(Console.HORIZONTAL_SCROLL_AMOUNT * 10)
      return true
    elseif event:is_key_press(d2k.Key.DOWN) then
      self:get_output():scroll_down(Console.HORIZONTAL_SCROLL_AMOUNT)
      return true
    elseif event:is_key_press(d2k.Key.PAGEDOWN) then
      self:get_output():scroll_down(Console.HORIZONTAL_SCROLL_AMOUNT * 10)
      return true
    end
  end

  if event:is_key_press(d2k.Key.UP) then
    self:get_input():show_previous_command()
    return true
  elseif event:is_key_press(d2k.Key.DOWN) then
    self:get_input():show_next_command()
    return true
  elseif event:is_key_press(d2k.Key.LEFT) then
    self:get_input():move_cursor_left()
    return true
  elseif event:is_key_press(d2k.Key.RIGHT) then
    self:get_input():move_cursor_right()
    return true
  elseif event:is_key_press(d2k.Key.DELETE) then
    self:get_input():delete_next_character()
    return true
  elseif event:is_key_press(d2k.Key.BACKSPACE) then
    self:get_input():delete_previous_character()
    return true
  elseif event:is_key_press(d2k.Key.HOME) then
    self:get_input():move_cursor_to_start()
    return true
  elseif event:is_key_press(d2k.Key.END) then
    self:get_input():move_cursor_to_end()
    return true
  elseif event:is_key_press(d2k.Key.RETURN) or
         event:is_key_press(d2k.Key.KP_ENTER) then
    self:handle_input(self:get_input():get_text())
    self:get_input():save_text_as_command()
    self:get_input():clear()
    return true
  elseif event:is_key() and event:is_press() then
    local char = event:get_char()

    if char then
      self:get_input():insert_character(event:get_char())
    end

    return true
  end

  return false
end

function Console:scroll_down()
  self:set_scroll_rate(self:get_max_height() / self:get_extension_time())
  self:set_last_scroll_ms(d2k.System.get_ticks())
end

function Console:scroll_up()
  self:set_scroll_rate(-(self:get_max_height() / self:get_retraction_time()))
  self:set_last_scroll_ms(d2k.System.get_ticks())
end

function Console:toggle_scroll()
  if self:get_height_in_pixels() == self:get_max_height() then
    self:scroll_up()
  elseif self:get_height_in_pixels() == 0 then
    self:scroll_down()
  elseif self:get_scroll_rate() < 0 then
    self:scroll_up()
  elseif self:get_scroll_rate() > 0 then
    self:scroll_down()
  end
end

function Console:summon()
  self:set_height(self:get_max_height())
  self:set_scroll_rate(0.0)
  self:set_last_scroll_ms(d2k.System.get_ticks())
end

function Console:banish()
  self:set_height(0)
  self:set_scroll_rate(0.0)
  self:set_last_scroll_ms(d2k.System.get_ticks())
end

function Console:set_fullscreen()
  self:set_height(d2k.overlay:get_height_in_pixels())
  self:set_scroll_rate(0.0)
  self:set_last_scroll_ms(d2k.System.get_ticks())
end

return {Console = Console}

-- vi: et ts=2 sw=2

