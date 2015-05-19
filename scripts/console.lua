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
  name                     = 'console',
  EXTENSION_TIME           = 1200.0,
  RETRACTION_TIME          = 1200.0,
  MARGIN                   = 8,
  HORIZONTAL_SCROLL_AMOUNT = 6,
  VERTICAL_SCROLL_AMOUNT   = 12,
  FG_COLOR                 = {1.0, 1.0, 1.0, 1.00},
  BG_COLOR                 = {0.0, 0.0, 0.0, 0.85},
  SHORTCUT_MARKER          = '/',
})

function Console:new(c)
  c = c or {}

  c.extension_time = c.extension_time or Console.EXTENSION_TIME
  c.retraction_time = c.retraction_time or Console.RETRACTION_TIME
  c.max_height = c.max_height or d2k.Video.get_screen_height() / 2
  c.shortcut_marker = c.shortcut_marker or Console.SHORTCUT_MARKER

  c.max_width = d2k.Video.get_screen_width()
  c.width = c.max_width
  c.height = 0
  c.z_index = 2
  c.scroll_rate = 0.0
  c.last_scroll_ms = 0

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
    word_wrap = TextWidget.WRAP_WORD,
    use_markup = true,
    strip_ending_newline = true
  })

  c.shortcut_regex = GLib.Regex.new(
    '([^"]\\S*|".+?"|\'.+?\'|)\\s*',
    GLib.RegexCompileFlags.OPTIMIZE,
    GLib.RegexMatchFlags.NOTEMPTY
  )

  setmetatable(c, self)
  self.__index = self

  c:set_name('console')

  return c
end

function Console:is_active()
  return self.height > 0 and self.scroll_rate >= 0.0
end

function Console:get_name()
  return 'Console'
end

function Console:reset()
  self.output:reset()
  self.input:reset()
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
end

function Console:draw()
  if self.height <= 0 then
    return
  end

  self.input:draw()
  self.output:draw()
end

function Console:parse_shortcut_command(short_command)
    local wrote_function_name = false
    local wrote_first_argument = false
    local command = ''

    if short_command:sub(1, 1) == self.shortcut_marker then
        short_command = short_command:sub(2)
    end

    local tokens = self.shortcut_regex:split(short_command, 0)

    for i, token in ipairs(tokens) do
        if #token ~= 0 then
            if not wrote_function_name then
                command = command .. token .. '('
                wrote_function_name = true
            elseif not wrote_first_argument then
                command = command .. token
                wrote_first_argument = true
            else
                command = command .. ', ' .. token
            end

        end
    end

    command = command .. ')'

    return command
end

function Console:handle_input(input)
  local command = input

  if input:sub(1, 1) == self.shortcut_marker then
    command = 'd2k.Shortcuts.' .. self:parse_shortcut_command(input)
  end

  local func, err = load(command, 'Console input', 't')

  if not func then
    self:mecho(string.format(
      '<span color="red">Console:handle_input: Error: %s</span>', err
    ))
    return
  end

  local worked, err = pcall(func)

  if not worked then
    self:mecho(string.format(
      '<span color="red">Console:handle_input: Error: %s</span>', err
    ))
  end
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
    elseif event:is_key_press(d2k.Key.PAGEUP) then
      self.output:scroll_up(Console.HORIZONTAL_SCROLL_AMOUNT * 10)
      return true
    elseif event:is_key_press(d2k.Key.DOWN) then
      self.output:scroll_down(Console.HORIZONTAL_SCROLL_AMOUNT)
      return true
    elseif event:is_key_press(d2k.Key.PAGEDOWN) then
      self.output:scroll_down(Console.HORIZONTAL_SCROLL_AMOUNT * 10)
      return true
    end
  end

  if event:is_key_press(d2k.Key.UP) then
    self.input:show_previous_command()
    return true
  elseif event:is_key_press(d2k.Key.DOWN) then
    self.input:show_next_command()
    return true
  elseif event:is_key_press(d2k.Key.LEFT) then
    self.input:move_cursor_left()
    return true
  elseif event:is_key_press(d2k.Key.RIGHT) then
    self.input:move_cursor_right()
    return true
  elseif event:is_key_press(d2k.Key.DELETE) then
    self.input:delete_next_character()
    return true
  elseif event:is_key_press(d2k.Key.BACKSPACE) then
    self.input:delete_previous_character()
    return true
  elseif event:is_key_press(d2k.Key.HOME) then
    self.input:move_cursor_to_start()
    return true
  elseif event:is_key_press(d2k.Key.END) then
    self.input:move_cursor_to_end()
    return true
  elseif event:is_key_press(d2k.Key.RETURN) or
         event:is_key_press(d2k.Key.KP_ENTER) then
    self:handle_input(self.input:get_text())
    self.input:save_text_as_command()
    self.input:clear()
    return true
  elseif event:is_key() and event:is_press() then
    local char = event:get_char()

    if char then
      self.input:insert_character(event:get_char())
    end

    return true
  end

  return false
end

function Console:scroll_down()
  self.scroll_rate = self.max_height / self.extension_time
  self.last_scroll_ms = d2k.System.get_ticks()
end

function Console:scroll_up()
  self.scroll_rate = -(self.max_height / self.retraction_time)
  self.last_scroll_ms = d2k.System.get_ticks()
end

function Console:toggle_scroll()
  if self.height == self.max_height then
    self:scroll_up()
  elseif self.height == 0 then
    self:scroll_down()
  elseif self.scroll_rate < 0 then
    self:scroll_up()
  elseif self.scroll_rate > 0 then
    self:scroll_down()
  end
end

function Console:summon()
  self.height = self.max_height
  self.scroll_rate = 0.0
  self.last_scroll_ms = d2k.System.get_ticks()
end

function Console:banish()
  self.height = 0
  self.scroll_rate = 0.0
  self.last_scroll_ms = d2k.System.get_ticks()
end

function Console:set_fullscreen()
  self.height = d2k.Video.get_screen_height()
  self.scroll_rate = 0.0
  self.last_scroll_ms = d2k.System.get_ticks()
end

function Console:write(text)
  -- CG: [TODO] Bail if repredicting
  if d2k.Video.is_enabled() then
    self.output:write(text)
  else
    d2k.System.print(text)
  end
end

function Console:mwrite(markup)
  -- CG: [TODO] Bail if repredicting
  if d2k.Video.is_enabled() then
    self.output:mwrite(markup)
  else
    d2k.System.print(markup) -- CG: [TODO] Strip markup if video is disabled
  end
end

function Console:echo(text)
  -- CG: [TODO] Bail if repredicting
  if d2k.Video.is_enabled() then
    self.output:echo(text)
  else
    print(text)
  end
end

function Console:mecho(markup)
  -- CG: [TODO] Bail if repredicting
  if d2k.Video.is_enabled() then
    self.output:mecho(markup)
  else
    print(markup) -- CG: [TODO] Strip markup if video is disabled
  end
end

return {Console = Console}

-- vi: et ts=2 sw=2

