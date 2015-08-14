-------------------------------------------------------------------------------
--                                                                           --
-- D2K: A Doom Source Port for the 21st Century                              --
--                                                                           --
-- This file is released into the public domain.                             --
--                                                                           --
-------------------------------------------------------------------------------

local InputWidget = require('input_widget')
local TextWidget = require('text_widget')
local RetractableTextWidget = require('retractable_text_widget')

local messages_widget = RetractableTextWidget.RetractableTextWidget:new({
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

local chat_widget = InputWidget.InputWidget:new({
  name = 'chat',
  z_index = 1,
  top_margin = 4,
  bottom_margin = 4,
  left_margin = 8,
  right_margin = 8,
  width = 1,
  line_height = 1,
  use_markup = true,
  font_description_text = 'ascsys,Arial Unicode MS, Unifont 11',
  strip_ending_newline = true,
  horizontal_alignment = TextWidget.ALIGN_LEFT,
  vertical_alignment = TextWidget.ALIGN_CENTER,
  fg_color = {1.0, 1.0, 1.0, 1.0},
  bg_color = {0.0, 0.0, 0.0, 0.85},
})

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

d2k.hud:add_widget(messages_widget)
d2k.hud:add_widget(chat_widget)

-- vi: et ts=2 sw=2
