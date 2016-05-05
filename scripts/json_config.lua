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

local serpent = require('serpent')

d2k.Config.validate = function()
    local ConfigSchema = require('config_schema')
    local schema = ConfigSchema.ConfigSchema

    print('Validating config (to do...)')
end

d2k.Config.get_default = function()
    local ConfigSchema = require('config_schema')
    local schema = ConfigSchema.ConfigSchema

    for section_name, section in pairs(schema) do
        local section_type = type(section)
        local is_table = section_type == 'table'
        local is_array = section[1] ~= nil

        if is_table and not is_array then
            for value_name, value in pairs(section) do
                if type(value) == 'table' then
                    section[value_name] = value.default
                end
            end
        end
    end

    d2k.Config.write(serpent.block(schema, { comment = false }))
end

-- vi: et ts=4 sw=4

