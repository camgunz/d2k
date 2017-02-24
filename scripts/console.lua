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
local UIObject = require('ui_object')
local InputInterface = require('input_interface')
local TextWidget = require('text_widget')
local InputWidget = require('input_widget')
local ContainerWidget = require('container_widget')

local Console = class('Console', ContainerWidget.ContainerWidget)

local DEFAULT_EXTENSION_TIME        = 1200.0
local DEFAULT_RETRACTION_TIME       = 1200.0
local DEFAULT_INPUT_TOP_PADDING     = 0.025
local DEFAULT_INPUT_BOTTOM_PADDING  = 0.025
local DEFAULT_INPUT_LEFT_PADDING    = 0.0125
local DEFAULT_INPUT_RIGHT_PADDING   = 0.0125
local DEFAULT_OUTPUT_TOP_PADDING    = 0
local DEFAULT_OUTPUT_BOTTOM_PADDING = 0.025
local DEFAULT_OUTPUT_LEFT_PADDING   = 0.0125
local DEFAULT_OUTPUT_RIGHT_PADDING  = 0.0125
local DEFAULT_Z_INDEX               = 2
local DEFAULT_SCROLL_AMOUNT         = 6
local DEFAULT_FG_COLOR              = {0.90, 0.90, 0.90, 1.00}
local DEFAULT_BG_COLOR              = {0.00, 0.00, 0.00, 0.85}
local DEFAULT_WIDTH                 = 1
local DEFAULT_HEIGHT                = .5

function Console:initialize(c)
    c = c or {}

    c.name = c.name or 'Console'
    c.fullscreen = c.fullscreen or true
    c.fg_color = DEFAULT_FG_COLOR
    c.bg_color = DEFAULT_BG_COLOR

    self.interfaces = {}

    ContainerWidget.ContainerWidget.initialize(self, c)

    self.extension_time = c.extension_time or DEFAULT_EXTENSION_TIME
    self.retraction_time = c.retraction_time or DEFAULT_RETRACTION_TIME
    self.scroll_amount = c.scroll_amount or DEFAULT_SCROLL_AMOUNT
    self.max_width = c.max_width or DEFAULT_WIDTH
    self.max_height = c.max_height or DEFAULT_HEIGHT
    self.z_index = c.z_index or DEFAULT_Z_INDEX

    self.width = self.max_width
    self.height = 0
    self.scroll_rate = 0.0
    self.last_scroll_ms = 0
    self.font = c.font or Font.get_default_console_font()

    self.input = InputWidget.InputWidget({
        name = 'console input',
        x = 0,
        y = 0,
        width = self.width,
        height = 0,
        z_index = self.z_index,
        top_padding = c.input_top_padding or DEFAULT_INPUT_TOP_PADDING,
        bottom_padding = c.input_bottom_padding or
                         DEFAULT_INPUT_BOTTOM_PADDING,
        left_padding = c.input_left_padding or DEFAULT_INPUT_LEFT_PADDING,
        right_padding = c.input_right_padding or DEFAULT_INPUT_RIGHT_PADDING,
        horizontal_alignment = TextWidget.ALIGN_LEFT,
        vertical_alignment = TextWidget.ALIGN_CENTER,
        fg_color = self.fg_color or DEFAULT_FG_COLOR,
        bg_color = self.bg_color or DEFAULT_BG_COLOR,
        use_parent_font = true,
        input_handler = d2k.CommandInterface.handle_input,
    })

    self.input.console = self
    self.input:set_parent(self)
    self.input.is_active = function(self)
        return self.console:get_pixel_height() > 0 and
               self.console:get_scroll_rate() >= 0.0
    end

    self.output = TextWidget.TextWidget({
        name = 'console output',
        x = 0,
        y = 0,
        width = self.width,
        height = self.max_height,
        z_index = self.z_index,
        top_padding = c.output_top_padding or DEFAULT_OUTPUT_TOP_PADDING,
        bottom_padding = c.output_bottom_padding or
                         DEFAULT_OUTPUT_BOTTOM_PADDING,
        left_padding = c.output_left_padding or DEFAULT_OUTPUT_LEFT_PADDING,
        right_padding = c.output_right_padding or DEFAULT_OUTPUT_RIGHT_PADDING,
        horizontal_alignment = TextWidget.ALIGN_LEFT,
        vertical_alignment = TextWidget.ALIGN_BOTTOM,
        fg_color = self.fg_color or DEFAULT_FG_COLOR,
        bg_color = self.bg_color or DEFAULT_BG_COLOR,
        use_parent_font = true,
        word_wrap = TextWidget.WRAP_WORD,
        use_markup = true,
        strip_ending_newline = true,
    })

    self.output.console = self
    self.output:set_parent(self)

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

function Console:get_scroll_amount()
    return self.scroll_amount
end

function Console:set_scroll_amount(scroll_amount)
    self.scroll_amount = scroll_amount
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
    self.interfaces[1] = self.input
end

function Console:get_output()
    return self.output
end

function Console:set_output(output)
    self.output = output
    self.interfaces[2] = self.output
end

function Console:is_active()
    return self:get_pixel_height() > 0 and self:get_scroll_rate() >= 0.0
end

function Console:get_name()
    return self.name
end

function Console:tick()
    if self:get_scroll_rate() ~= 0 then
        local current_ms = d2k.System.get_ticks()

        if self:get_last_scroll_ms() == 0 then
            self:set_last_scroll_ms(current_ms)
        end

        local ms_elapsed = current_ms - self:get_last_scroll_ms()
        local new_height = math.max(
            0, self:get_pixel_height() + (self:get_scroll_rate() * ms_elapsed)
        )

        if self:get_pixel_height() ~= new_height then
            self:set_pixel_height(new_height)
        end

        if self:get_pixel_height() < 0 then
            self:set_height(0)
            self:set_scroll_rate(0)
        elseif self:get_pixel_height() >
               self:get_max_pixel_height() then
            self:set_pixel_height(self:get_max_pixel_height())
            self:set_scroll_rate(0)
        end
    end

    local bottom = self:get_pixel_height()
    local input_height = self:get_input():get_pixel_height()
    local output_height = self:get_output():get_pixel_height()
    local input_y = self:get_input():get_y()
    local new_input_y = bottom - input_height

    ContainerWidget.ContainerWidget.tick(self)

    if input_y ~= new_input_y then
        self:get_input():set_y(new_input_y)
        input_y = new_input_y
    end

    local new_output_height = math.max(input_y, 0)

    if output_height ~= new_output_height then
        self:get_output():set_pixel_height(new_output_height)
    end
end

function Console:render()
    if self:get_pixel_height() <= 0 then
        return
    end

    ContainerWidget.ContainerWidget.render(self)
end

function Console:handle_event(event)
    if event:is_key_press(d2k.Key.BACKQUOTE) then
        self:toggle_scroll()
        return true
    end

    if self:get_scroll_rate() < 0 or self:get_pixel_height() == 0 then
        return false
    end

    if d2k.KeyStates.shift_is_down() then
        if event:is_key_press(d2k.Key.UP) then
            self:get_output():scroll_up(self:get_scroll_amount())
            return true
        elseif event:is_key_press(d2k.Key.PAGEUP) then
            self:get_output():scroll_up(self:get_scroll_amount() * 10)
            return true
        elseif event:is_key_press(d2k.Key.DOWN) then
            self:get_output():scroll_down(self:get_scroll_amount())
            return true
        elseif event:is_key_press(d2k.Key.PAGEDOWN) then
            self:get_output():scroll_down(self:get_scroll_amount() * 10)
            return true
        end
    end

    return self:get_input():handle_event(event)
end

function Console:scroll_down()
    self:set_scroll_rate(
        self:get_max_pixel_height() / self:get_extension_time()
    )
    self:set_last_scroll_ms(d2k.System.get_ticks())
end

function Console:scroll_up()
    self:set_scroll_rate(
        -(self:get_max_pixel_height() / self:get_retraction_time())
    )
    self:set_last_scroll_ms(d2k.System.get_ticks())
end

function Console:toggle_scroll()
    if self:get_pixel_height() == self:get_max_pixel_height() then
        self:scroll_up()
    elseif self:get_pixel_height() == 0 then
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
    self:set_height(d2k.overlay:get_pixel_height())
    self:set_scroll_rate(0.0)
    self:set_last_scroll_ms(d2k.System.get_ticks())
end

return {Console = Console}

-- vi: et ts=4 sw=4

