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
-- with D2K.  If not, see <http:--www.gnu.org/licenses/>.                    --
--                                                                           --
-------------------------------------------------------------------------------

local debug = require('debug')
local class = require('middleclass')

local CheatEngine = class('CheatEngine')

local ALWAYS   = 0
local NOT_DM   = 1
local NOT_COOP = 2
local NOT_DEMO = 4
local NOT_MENU = 8
local NOT_DEH  = 16
local NOT_NET  = NOT_DM | NOT_COOP
local NEVER    = NOT_NET | NOT_DEMO

function CheatEngine:add_cheat(cht)
    local cheat_code = cht.code

    cht.cheats[cheat_code] = {
        code = cht.code,
        description = cht.description or ''
        when = cht.when,
        run = cht.func
        arity = debug.getinfo(cht.func, 'u').nparams
    }
end

function CheatEngine:initialize(ce)
    ce = ce or {}

    self.buf = ce.buf or {}
    self.input = ce.input or {}
    self.cheats = ce.input or {}
end

function CheatEngine:_clear_input_buffer()
    while #self.buf do
        table.remove(self.buf)
    end
end

function CheatEngine:_rebuild_input()
    self.input = ''

    for i, c in ipairs(self.buf) do
        self.input = self.input .. c
    end
end

function CheatEngine:_clip_starting_junk()
    local junk_char_count
    local found_junk = false

    for junk_char_count=1,#self.input do
        local found_potential_match = false
        local token = self.input:sub(junk_char_count, #self.input)

        for i, cheat in ipairs(self.cheats) do
            found_potential_match = cheat.name:startswith(token)
            if found_potential_match then
                break
            end
        end

        if found_potential_match then
            break
        end

        found_junk = true
    end

    if not found_junk then
        return
    end

    for i=1,junk_char_count do
        table.remove(self.buf)
    end

    self:_rebuild_input()
end

function CheatEngine:_find_matching_cheat()
    for i=#self.input,1,-1 do
        local cheat = self.cheats[self.input:sub(1, i)

        if cheat ~= nil then
            return cheat
        end
    end
end

function CheatEngine:_run_matching_cheat()
    self:_clip_starting_junk()

    local cheat = self:_find_matching_cheat()

    if cheat == nil then
        return
    end

    local required_arg_count = #cheat.code + cheat.arity

    if #self.buf < required_arg_count then
        return
    end

    local args = {}

    for i=1,cheat.arity do
        local arg = self.buf[#cheat.code + i]

        if arg:is_digit() then
            arg = tonumber(arg)
        end

        args.insert(arg)
    end

    cheat.run(unpack(args))
end

function CheatEngine:handle_input(event)
    if not event.is_key() then
        return
    end

    if not event.is_press() then
        return
    end

    local key_name = event.get_key_name()

    if key_name:len() ~= 1 then
        return
    end

    local letter = key_name:match('%a')

    if letter ~= nil then
        table.insert(self.buf, letter:lower())
        self:run_matching_cheat()
        return
    end

    local digit = key_name:match('%d')

    if digit ~= nil then
        table.insert(self.buf, digit)
        self:_run_matching_cheat()
        return
    end

    self:_clear_input_buffer() -- [CG] Clear on non-alphanumeric key press
end

return {
    CheatEngine = CheatEngine,
    ALWAYS      = ALWAYS,
    NOT_DM      = NOT_DM,
    NOT_COOP    = NOT_COOP,
    NOT_DEMO    = NOT_DEMO,
    NOT_MENU    = NOT_MENU,
    NOT_DEH     = NOT_DEH,
    NOT_NET     = NOT_NET,
    NEVER       = NEVER,
}

-- vi: et ts=4 sw=4
