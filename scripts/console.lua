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
local InputInterface = require('input_interface')
local TextWidget = require('text_widget')
local InputWidget = require('input_widget')

Console = class('Console', InputInterface.InputInterface)

local EXTENSION_TIME           = 1200.0
local RETRACTION_TIME          = 1200.0
local MARGIN                   = 8
local HORIZONTAL_SCROLL_AMOUNT = 6
local VERTICAL_SCROLL_AMOUNT   = 12
local FG_COLOR                 = {1.0, 1.0, 1.0, 1.00}
local BG_COLOR                 = {0.0, 0.0, 0.0, 0.85}

function Console:initialize(c)
  c = c or {}

  c.name = c.name or 'Console'

  InputInterface.InputInterface.initialize(self, c)

  self.extension_time = c.extension_time or EXTENSION_TIME
  self.retraction_time = c.retraction_time or RETRACTION_TIME
  self.max_width = c.max_width or 1
  self.max_height = c.max_height or .5
  self.z_index = c.z_index or 2

  self.width = self.max_width
  self.height = 0
  self.scroll_rate = 0.0
  self.last_scroll_ms = 0

  self.font_description_text = c.font_description_text or
                               d2k.hud.font_description_text

  self.input = InputWidget.InputWidget({
    name = 'console input',
    x = 0,
    y = 0,
    width = self.width,
    height = 0,
    z_index = self.z_index,
    top_margin = self.input_margin or MARGIN,
    bottom_margin = self.bottom_margin or MARGIN,
    left_margin = self.left_margin or MARGIN,
    right_margin = self.right_margin or MARGIN,
    horizontal_alignment = TextWidget.ALIGN_LEFT,
    vertical_alignment = TextWidget.ALIGN_CENTER,
    fg_color = self.fg_color or FG_COLOR,
    bg_color = self.bg_color or BG_COLOR,
    font_description_text = self.font_description_text,
    input_handler = d2k.CommandInterface.handle_input,
  })

  self.output = TextWidget.TextWidget({
    name = 'console output',
    x = 0,
    y = 0,
    width = self.width,
    height = self.max_height,
    z_index = self.z_index,
    top_margin = self.top_margin or 0,
    bottom_margin = self.bottom_margin or MARGIN,
    left_margin = self.left_margin or MARGIN,
    right_margin = self.right_margin or MARGIN,
    horizontal_alignment = TextWidget.ALIGN_LEFT,
    vertical_alignment = TextWidget.ALIGN_BOTTOM,
    fg_color = self.fg_color or FG_COLOR,
    bg_color = self.bg_color or BG_COLOR,
    font_description_text = self.font_description_text,
    word_wrap = TextWidget.WRAP_WORD,
    use_markup = true,
    strip_ending_newline = true,
  })

  self.output:set_external_text_source(
    d2k.Messaging.get_console_messages,
    d2k.Messaging.get_console_messages_updated,
    d2k.Messaging.clear_console_messages_updated
  )
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

--[[
function Console:is_active()
  return self:get_height_in_pixels() > 0 and self:get_scroll_rate() >= 0.0
end
--]]

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
end

function Console:handle_event(event)
  if event:is_key_press(d2k.Key.BACKQUOTE) then
    self:toggle_scroll()
    return true
  end

  if self:get_scroll_rate() < 0 or self:get_height_in_pixels() == 0 then
    return false
  end

  if d2k.KeyStates.shift_is_down() then
    if event:is_key_press(d2k.Key.UP) then
      self:get_output():scroll_up(HORIZONTAL_SCROLL_AMOUNT)
      return true
    elseif event:is_key_press(d2k.Key.PAGEUP) then
      self:get_output():scroll_up(HORIZONTAL_SCROLL_AMOUNT * 10)
      return true
    elseif event:is_key_press(d2k.Key.DOWN) then
      self:get_output():scroll_down(HORIZONTAL_SCROLL_AMOUNT)
      return true
    elseif event:is_key_press(d2k.Key.PAGEDOWN) then
      self:get_output():scroll_down(HORIZONTAL_SCROLL_AMOUNT * 10)
      return true
    end
  end

  return self:get_input():handle_event(event)
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

return {
  Console                  = Console,
  EXTENSION_TIME           = EXTENSION_TIME,
  RETRACTION_TIME          = RETRACTION_TIME,
  MARGIN                   = MARGIN,
  HORIZONTAL_SCROLL_AMOUNT = HORIZONTAL_SCROLL_AMOUNT,
  VERTICAL_SCROLL_AMOUNT   = VERTICAL_SCROLL_AMOUNT,
  FG_COLOR                 = FG_COLOR,
  BG_COLOR                 = BG_COLOR
}

-- vi: et ts=2 sw=2

