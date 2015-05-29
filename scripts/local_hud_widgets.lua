-------------------------------------------------------------------------------
--                                                                           --
-- D2K: A Doom Source Port for the 21st Century                              --
--                                                                           --
-- This file is released into the public domain.                             --
--                                                                           --
-------------------------------------------------------------------------------

local TextWidget = require('text_widget')

local messages_widget = TextWidget.TextWidget:new({
  name = 'messages',
  z_index = 1,
  top_margin = 0,
  bottom_margin = 0,
  left_margin = 8,
  right_margin = 8,
  width = -1,
  bg_color = {0.0, 0.0, 0.0, 0.0},
  vertical_alignment = TextWidget.ALIGN_BOTTOM,
  outline_color = {0.0, 0.0, 0.0, 1.0},
  outline_text = true,
  outline_width = 1,
  line_height = 4,
  use_markup = true,
  strip_ending_newline = true,
  retractable = true,
  retraction_time = 666,
  retraction_timeout = 2000
})

messages_widget:set_external_text_source(
  d2k.Game.get_consoleplayer_messages,
  d2k.Game.get_consoleplayer_messages_updated,
  d2k.Game.clear_consoleplayer_messages_updated
)

d2k.hud:add_widget(messages_widget)

-- vi: et ts=2 sw=2
