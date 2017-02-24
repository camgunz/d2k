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

local class = require('middleclass')
local lgi = require('lgi')
local Cairo = lgi.cairo
local TextWidget = require('text_widget')
local InputWidget = require('input_widget')
local RetractableTextWidget = require('retractable_text_widget')
local TableWidget = require('table_widget')
local UIObject = require('ui_object')
local Fonts = require('fonts')
local ContainerWidget = require('container_widget')

d2k.widgets = {}

local messages_widget = RetractableTextWidget.RetractableTextWidget({
    name = 'messages',
    z_index = 1,
    top_padding = 0.0125,
    bottom_padding = 0,
    left_padding = 0.0125,
    right_padding = 0.0125,
    width = 1,
    bg_color = {0.0, 0.0, 0.0, 0.0},
    vertical_alignment = TextWidget.ALIGN_BOTTOM,
    outline_color = {0.0, 0.0, 0.0, 1.0},
    outline_text = true,
    outline_width = 1,
    height_in_lines = 4,
    use_markup = true,
    font = Fonts.get_default_messages_font(),
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

messages_widget:set_parent(d2k.hud)
d2k.widgets['messages'] = messages_widget

local chat_widget = InputWidget.InputWidget({
    name = 'chat',
    z_index = 1,
    parent_reference_point = UIObject.REFERENCE_POINT_SOUTHWEST,
    local_reference_point = UIObject.REFERENCE_POINT_SOUTHWEST,
    top_padding = 0.0125,
    bottom_padding = 0.0125,
    left_padding = 0.0125,
    right_padding = 0.0125,
    width = 1,
    line_height = 1,
    use_markup = false,
    strip_ending_newline = true,
    horizontal_alignment = TextWidget.ALIGN_LEFT,
    vertical_alignment = TextWidget.ALIGN_CENTER,
    fg_color = {1.0, 1.0, 1.0, 1.0},
    bg_color = {0.0, 0.0, 0.0, 0.85},
    input_handler = function(input) d2k.Client.say(input) end,
    deactivate_on_input = true
})

function chat_widget:render()
    if self:is_active() then
        InputWidget.InputWidget.render(self)
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
d2k.widgets['chat'] = chat_widget

local fps_widget = TextWidget.TextWidget({
    name = 'fps',
    z_index = 1,
    top_padding = 0.0125,
    bottom_padding = 0.0125,
    left_padding = 0.025,
    right_padding = 0.025,
    x = 0,
    y = 100,
    width = .175,
    height = .1,
    use_markup = false,
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
    end
end

fps_widget:set_last_time(d2k.System.get_ticks())
fps_widget:set_frame_count(0)
fps_widget:set_text('0 FPS')

d2k.widgets['fps'] = fps_widget

local netstats_widget = TextWidget.TextWidget({
    name = 'netstats',
    z_index = 1,
    top_padding = 0.0125,
    bottom_padding = 0.0125,
    left_padding = 0.025,
    right_padding = 0.025,
    parent_reference_point = UIObject.REFERENCE_POINT_EAST,
    local_reference_point = UIObject.REFERENCE_POINT_EAST,
    y = -50,
    width = .44,
    height = .26,
    use_markup = false,
    horizontal_alignment = TextWidget.ALIGN_LEFT,
    vertical_alignment = TextWidget.ALIGN_TOP,
    fg_color = {1.0, 1.0, 1.0, 1.00},
    bg_color = {0.0, 0.0, 0.0, 0.65},
})

function netstats_widget:get_last_time()
    return self.last_time
end

function netstats_widget:set_last_time(last_time)
    self.last_time = last_time
end

function netstats_widget:tick()
    local current_time = d2k.System.get_ticks()
    local time_elapsed = current_time - self:get_last_time()

    if time_elapsed >= 1000 then
        local netstats = d2k.Client.get_netstats()

        -- [CG] FIXME The functionality to clear netstats has been removed, so
        --            this widget needs to remember values between TICs if it
        --            wants to show recent information instead of average
        --            information.

        if netstats ~= nil then
            local gametic = d2k.Game.get_gametic()
            local tic_delta = gametic - netstats.server_tic

            netstats.upload = netstats.upload / 1000
            netstats.download = netstats.download / 1000
            netstats.packet_loss = netstats.packet_loss / 100
            netstats.packet_loss_jitter = netstats.packet_loss_jitter / 100

            self:set_text(string.format(
                'U/D:             %d KB/s | %d KB/s\n' ..
                'Ping:            %d ms\n' ..
                'Jitter:          %d\n' ..
                'Commands:        %d / %d\n' ..
                'PL:              %0.2f%% | %0.2f%%\n' ..
                'Throttle:        %d\n' ..
                'Server/Game TIC: %d / %d (%d)\n',
                netstats.upload,
                netstats.download,
                netstats.ping_average,
                netstats.jitter_average,
                netstats.unsynchronized_commands,
                netstats.total_commands,
                netstats.packet_loss,
                netstats.packet_loss_jitter,
                netstats.throttle,
                netstats.server_tic,
                gametic,
                tic_delta
            ))
        end

        self:set_last_time(current_time)
    end
end

netstats_widget:set_last_time(d2k.System.get_ticks())

netstats_widget:set_pixel_width(282)
netstats_widget:set_pixel_height(125)
d2k.widgets['netstats'] = netstats_widget

local scoreboard_widget = ContainerWidget.ContainerWidget({
    name = 'scoreboard',
    x = 0,
    y = 0,
    z_index = 1,
    parent_reference_point = UIObject.REFERENCE_POINT_CENTER,
    local_reference_point = UIObject.REFERENCE_POINT_CENTER,
    width = .7,
    height = .7,
    use_markup = true,
    fg_color = {1.0, 1.0, 1.0, 1.00},
    bg_color = {0.0, 0.0, 0.0, 0.65}
})

-- function scoreboard_widget:render()
--     local cr = d2k.overlay.render_context
--     local bg_color = self:get_bg_color()
-- 
--     cr:save()
--     cr:set_operator(Cairo.Operator.OVER)
--     cr:set_source_rgba(bg_color[1], bg_color[2], bg_color[3], bg_color[4])
--     cr:paint()
--     cr:restore()
-- end

function scoreboard_widget:render()
    if not self:is_active() then
        return
    end

    local cr = d2k.overlay.render_context
    local bg_color = self:get_bg_color()

    cr:save()
    cr:set_operator(Cairo.Operator.OVER)
    cr:set_source_rgba(bg_color[1], bg_color[2], bg_color[3], bg_color[4])
    cr:paint()

    cr:restore()

    ContainerWidget.ContainerWidget.render(self)
end

function scoreboard_widget:is_active()
    return ContainerWidget.ContainerWidget.is_active(self) or (
        d2k.Game.is_multiplayer() and d2k.Game.in_intermission()
    )
end

function scoreboard_widget:handle_event(event)
    if not self:is_active() then
        if event:is_key_press(d2k.Key.BACKSLASH) then
            self:activate()
            return true
        end
    elseif event:is_key_release(d2k.Key.BACKSLASH) then
        self:invalidate_render()
        self:deactivate()
        return true
    end

    return false
end

scoreboard_widget:set_parent(d2k.hud)

local scoreboard_title_widget = TextWidget.TextWidget({
    name = 'scoreboard_title',
    z_index = scoreboard_widget.z_index,
    y = 20,
    top_padding = 0.0375,
    bottom_padding = 0.0375,
    left_padding = 0.0375,
    right_padding = 0.0375,
    parent_reference_point = UIObject.REFERENCE_POINT_NORTH,
    local_reference_point = UIObject.REFERENCE_POINT_NORTH,
    width = 1,
    height = .18,
    use_markup = true,
    use_parent_font = true,
    horizontal_alignment = TextWidget.ALIGN_CENTER,
    vertical_alignment = TextWidget.ALIGN_TOP,
    fg_color = {1.0, 1.0, 1.0, 1.00},
    bg_color = {0.0, 0.0, 0.0, 0.00}
})

scoreboard_title_widget:set_parent(scoreboard_widget)

function scoreboard_title_widget:tick()
    local game_mode = d2k.Game.get_game_mode()

    if game_mode ~= self.game_mode then
        self.game_mode = game_mode
        if self.game_mode == 'deathmatch' then
            local frag_limit = d2k.Game.get_frag_limit() or 'None'
            local time_limit = d2k.Game.get_time_limit() or 0

            if time_limit ~= 0 then
                local minutes = math.floor(time_limit / 60)
                local seconds = math.floor(time_limit % 60)

                time_limit = string.format('%d:%02d', minutes, seconds)
            else
                time_limit = 'None'
            end

            self:set_text(string.format('%s\nFrag Limit: %s\tTime Limit: %s',
                string.upper(self.game_mode), frag_limit, time_limit
            ))
        elseif self.game_mode == 'cooperative' then
            self:set_text(string.format('%s', string.upper(self.game_mode)))
        elseif self.game_mode == 'solonet' then
            self:set_text(string.format('%s', string.upper(self.game_mode)))
        elseif self.game_mode == 'singleplayer' then
            self:set_text(string.format('%s', string.upper(self.game_mode)))
        end
        self:handle_content_change()
    end
    TextWidget.TextWidget.tick(self)
end

local scoreboard_player_table_widget = TableWidget.TableWidget({
    name = 'scoreboard_player_table',
    z_index = scoreboard_widget.z_index,
    top_padding = 0.0375,
    bottom_padding = 0.0375,
    left_padding = 0.0375,
    right_padding = 0.0375,
    parent_reference_point = UIObject.REFERENCE_POINT_NORTH,
    local_reference_point = UIObject.REFERENCE_POINT_NORTH,
    width = .9,
    height = .7,
    horizontal_alignment = TextWidget.ALIGN_CENTER,
    vertical_alignment = TextWidget.ALIGN_TOP,
    fg_color = {1.0, 1.0, 1.0, 1.00},
    bg_color = {0.0, 0.0, 0.0, 0.00}
})

scoreboard_player_table_widget:add_row()    -- Add header
scoreboard_player_table_widget:add_column() -- Player name
scoreboard_player_table_widget:add_column() -- Frags
scoreboard_player_table_widget:add_column() -- Deaths
scoreboard_player_table_widget:add_column() -- Ping
scoreboard_player_table_widget:add_column() -- Time (minutes)

scoreboard_player_table_widget:get_cell(1, 2):set_text('Frags')
scoreboard_player_table_widget:get_cell(1, 3):set_text('Deaths')
scoreboard_player_table_widget:get_cell(1, 4):set_text('Ping')
scoreboard_player_table_widget:get_cell(1, 5):set_text('Time')

function scoreboard_player_table_widget:tick()
    local current_player_count = (#self.rows) - 1
    local players = d2k.Game.get_players()
    local added_rows = false

    table.sort(players, function(p1, p2)
        if p1.frags > p2.frags then
            return true
        else
            return false
        end
    end)

    while #self.rows < (#players + 1) do
        self:add_row()
        added_rows = true
    end

    while #self.rows > (#players + 1) do
        self:remove_row(#self.rows)
    end

    for i, player in ipairs(players) do
        local row = i + 1
        local name = player.name
        local frags = tostring(player.frags)
        local deaths = tostring(player.deaths)
        local ping = tostring(player.ping)
        local time = string.format('%02d:%02d',
            math.floor(player.time / 60), math.floor(player.time % 60)
        )

        local name_cell = self:get_cell(row, 1)
        local frags_cell = self:get_cell(row, 2)
        local deaths_cell = self:get_cell(row, 3)
        local ping_cell = self:get_cell(row, 4)
        local time_cell = self:get_cell(row, 5)

        if name_cell:get_text() ~= name then
            name_cell:set_text(name)
        end

        if frags_cell:get_text() ~= frags then
            frags_cell:set_text(frags)
        end

        if deaths_cell:get_text() ~= deaths then
            deaths_cell:set_text(deaths)
        end

        if ping_cell:get_text() ~= ping then
            ping_cell:set_text(ping)
        end

        if time_cell:get_text() ~= time then
            time_cell:set_text(time)
        end
    end

    if added_rows then
        self:set_cell_width(1.0 / 5.0)
        self:set_cell_height(1.0 / 9.0)
        self:set_column_horizontal_alignment(2, TextWidget.ALIGN_CENTER)
        self:set_column_horizontal_alignment(3, TextWidget.ALIGN_CENTER)
        self:set_column_horizontal_alignment(4, TextWidget.ALIGN_CENTER)
        self:set_column_horizontal_alignment(5, TextWidget.ALIGN_CENTER)
    end
end

scoreboard_player_table_widget:set_cell_width(1.0 / 5.0)
scoreboard_player_table_widget:set_cell_height(1.0 / 9.0)

scoreboard_player_table_widget:set_parent(scoreboard_widget)
scoreboard_player_table_widget:set_y(
  scoreboard_title_widget:get_pixel_height() + 5
)
d2k.widgets['scoreboard'] = scoreboard_widget

-- vi: et ts=2 sw=2

