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
local HUDWidget = require('hud_widget')
local TextWidget = require('hud_text_widget')

Console = HUDWidget.HUDWidget:new({
  SCROLL_DOWN_TIME = 1200.0,
  SCROLL_UP_TIME   = 1200.0,
  MARGIN           = 8
})

function Console:new(c)
  c = c or {}

  c.scroll_down_time = c.scroll_down_time or Console.SCROLL_DOWN_TIME
  c.scroll_up_time = c.scroll_up_time or Console.SCROLL_UP_TIME
  c.max_height = c.max_height or d2k.Video.get_screen_height() / 2

  c.max_width = d2k.Video.get_screen_width()
  c.width = c.max_width
  c.height = c.max_height
  c.scroll_rate = 0.0
  c.last_scroll_ms = 0

  c.output = TextWidget.TextWidget:new({
    x = 0,
    y = 0,
    top_margin = c.top_margin or Console.MARGIN,
    bottom_margin = c.bottom_margin or Console.MARGIN,
    left_margin = c.left_margin or Console.MARGIN,
    right_margin = c.right_margin or Console.MARGIN,
    width = c.max_width,
    height = c.max_height,
    fg_color = c.fg_color or {0.1, 0.8, 0.6, 1.0},
    bg_color = c.bg_color or {  0,   0,   0, 0.6}
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

    print(string.format('Scroll rate, height: %f, %d',
      self.scroll_rate, self.height
    ))
  end

  self.output.height = self.height

  self.output:set_text(string.format("[UD]Ladna: console height: %d",
    self.height
  ))
  self.output:tick()
end

function Console:draw()
  if self.height <= 0 then
    return
  end

  self.output:draw()
end

function Console:handle_event(event)
  if event:is_key_press(d2k.Key.BACKQUOTE) then
    self:toggle_scroll()
  end
end

function Console:toggle_scroll()
  if self.height == self.max_height then
    self.scroll_rate = -(self.max_height / self.scroll_down_time)
  elseif self.height == 0 then
    self.scroll_rate = self.max_height / self.scroll_down_time
  elseif self.scroll_rate < 0 then
    self.scroll_rate = self.max_height / self.scroll_down_time
  elseif self.scroll_rate > 0 then
    self.scroll_rate = -(self.max_height / self.scroll_down_time)
  end

  self.last_scroll_ms = d2k.System.get_ticks()
end


return {Console = Console}

-- vi: et ts=2 sw=2

