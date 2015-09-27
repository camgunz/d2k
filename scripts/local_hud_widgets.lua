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

local TextWidget = require('text_widget')
local InputWidget = require('input_widget')
local RetractableTextWidget = require('retractable_text_widget')
local InputInterface = require('input_interface')

local messages_widget = RetractableTextWidget.RetractableTextWidget({
  name = 'messages',
  z_index = 1,
  top_margin = 4,
  bottom_margin = 0,
  left_margin = 8,
  right_margin = 8,
  width = 1,
  bg_color = {0.0, 0.0, 0.0, 0.0},
  vertical_alignment = TextWidget.ALIGN_BOTTOM,
  outline_color = {0.0, 0.0, 0.0, 1.0},
  outline_text = true,
  outline_width = 1,
  line_height = 4,
  use_markup = true,
  font_description_text = 'Zeroes Two 12',
  strip_ending_newline = true,
  retractable = RetractableTextWidget.RETRACT_UP,
  retraction_time = 500,
  retraction_timeout = 2000,
})

messages_widget:set_external_text_source(
  d2k.Game.get_consoleplayer_messages,
  d2k.Game.get_consoleplayer_messages_updated,
  d2k.Game.clear_consoleplayer_messages_updated
)

local chat_widget = InputWidget.InputWidget({
  name = 'chat',
  z_index = 1,
  snap = InputInterface.SNAP_BOTTOM,
  top_margin = 4,
  bottom_margin = 4,
  left_margin = 8,
  right_margin = 8,
  width = 1,
  line_height = 1,
  use_markup = false,
  font_description_text = 'Noto Sans,Arial Unicode MS,Unifont 11',
  strip_ending_newline = true,
  horizontal_alignment = TextWidget.ALIGN_LEFT,
  vertical_alignment = TextWidget.ALIGN_CENTER,
  fg_color = {1.0, 1.0, 1.0, 1.0},
  bg_color = {0.0, 0.0, 0.0, 0.85},
  input_handler = function(input) d2k.Client.say(input) end,
  deactivate_on_input = true
})

messages_widget:set_parent(d2k.hud)

function chat_widget:tick()
  InputWidget.InputWidget.tick(self)
end

function chat_widget:draw()
  if self:is_active() then
    InputWidget.InputWidget.draw(self)
  end
end

function chat_widget:handle_event(event)
  if not self:is_active() then
    if event:is_key_press(d2k.Key.t) then
      self:activate()
      return true
    end

    return false
  end

  if event:is_key_press(d2k.Key.ESCAPE) then
    self:clear()
    self:deactivate()
    return true
  end

  return InputWidget.InputWidget.handle_event(self, event)
end

chat_widget:set_parent(d2k.hud)

local fps_widget = TextWidget.TextWidget({
  name = 'fps',
  z_index = 1,
  top_margin = 4,
  bottom_margin = 4,
  left_margin = 8,
  right_margin = 8,
  x = 0,
  y = 0,
  width = .2,
  height = .1,
  use_markup = false,
  font_description_text = 'Noto Sans,Arial Unicode MS,Unifont 11',
  horizontal_alignment = TextWidget.ALIGN_RIGHT,
  vertical_alignment = TextWidget.ALIGN_CENTER,
  fg_color = {1.0, 1.0, 1.0, 1.00},
  bg_color = {0.0, 0.0, 0.0, 0.65},
})

function fps_widget:get_last_time()
  return self.last_time
end

function fps_widget:set_last_time(last_time)
  self.last_time = last_time
end

function fps_widget:get_frame_count()
  return self.frame_count
end

function fps_widget:set_frame_count(frame_count)
  self.frame_count = frame_count
end

function fps_widget:increment_frame_count()
  self.frame_count = self.frame_count + 1
end

function fps_widget:tick()
  local current_time = d2k.System.get_ticks()
  local time_elapsed = current_time - self:get_last_time()

  self:increment_frame_count()

  if time_elapsed >= 1000 then
    self:set_text(string.format(
      '%03.2f FPS', (self:get_frame_count() / time_elapsed) * 1000
    ))
    self:set_frame_count(0)
    self:set_last_time(current_time)

    self.needs_updating = true
    print(self:get_text())
  end
end

fps_widget:set_last_time(d2k.System.get_ticks())
fps_widget:set_frame_count(0)
fps_widget:set_text('0 FPS')

fps_widget:set_parent(d2k.hud)

-- vi: et ts=2 sw=2

