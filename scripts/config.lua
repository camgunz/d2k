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

json = require('dkjson')

d2k.Config.validate = function()
    local ConfigSchema = require('config_schema')

    print('Validating config (to do...)')
end

d2k.Config.generate_default_config = function()
    local ConfigSchema = require('config_schema')
    local config = json.decode(json.encode(ConfigSchema.ConfigSchema))

    for section_name, section in pairs(config) do
        local is_array = false

        if section[1] then
            is_array = true
        end

        if not is_array then
            for value_name, value in pairs(section) do
                if type(value) == 'table' then
                    section[value_name] = value.default
                end
            end
        end
    end

    d2k.Config.write(json.encode(config, { indent = true }))
end

-- vi: et ts=4 sw=4

