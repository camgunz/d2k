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

d2k.cvars = {}

d2k.config.validate = function()
    local ConfigSchema = require('config_schema')
    local schema = ConfigSchema.ConfigSchema

    for section_name, section in pairs(schema) do
        local section_is_table = type(section) == 'table'
        local section_is_array = false
        
        if section_is_table and section[1] ~= nil then
            section_is_array = true
        end

        if section_is_table and not section_is_array then
            for value_name, value in ipairs(section) do
                local value_type = type(value)
                local value_is_table = value_type == 'table'
                local value_is_array = value_is_table and value[1] ~= nil
                local config_value = d2k.cvars[section_name][value_name]

                if config_value == nil then
                    if value_is_table and not value_is_array then
                        d2k.cvars[section_name][value_name] = value.default
                    elseif value_is_array then
                        -- [CG] TODO: Handle each array value as potentially a
                        --            table instead of a raw value
                        d2k.cvars[section_name][value_name] = {}
                        for i=1,#value do
                            d2k.cvars[section_name][value_name][i] = value[i]
                        end
                    else
                        d2k.cvars[section_name][value_name] = value
                    end
                else
                    local config_value_type = type(config_value)
                    local config_value_is_table = config_value_type == 'table'
                    local config_value_is_array = config_value[i] ~= nil

                    if config_value_type ~= schema_value_type then
                        error(string.format(
                            'Config value [%s.%s] must be a %s (got %s)',
                            section_name,
                            value_name,
                            value_type,
                            config_value_type
                        ))
                    elseif value_is_array and not config_value_is_array then
                        error(string.format(
                            'Config value [%s.%s] must be a table, not an ' ..
                            'array',
                            section_name,
                            value_name
                        ))
                    end

                    if value.min ~= nil and value.max ~= nil and
                       (config_value < value.min or
                        config_value > value.max) then
                        error(string.format(
                            'Config value [%s.%s] must be between %s and %s',
                            section_name,
                            value_name,
                            value.min,
                            value.max
                        ))
                    elseif value.min ~= nil and config_value < value.min then
                        error(string.format(
                            'Config value [%s.%s] must be >= %s',
                            section_name,
                            value_name,
                            value.min
                        ))
                    elseif value.max ~= nil and config_value > value.max then
                        error(string.format(
                            'Config value [%s.%s] must be <= %s',
                            section_name,
                            value_name,
                            value.max
                        ))
                    end
                end
            end
        end
    end
end

d2k.config.get_default = function()
    local ConfigSchema = require('config_schema')
    local schema = ConfigSchema.ConfigSchema
    local config = {}

    for section_name, section in pairs(schema) do
        local section_is_table = type(section) == 'table'
        local section_is_array = section_is_table and section[1] ~= nil

        if section_is_table and not section_is_array then
            config[section_name] = {}

            for value_name, value in pairs(section) do
                local value_is_table = type(value) == 'table'
                local value_is_array = value_is_table and value[1] ~= nil

                if value_is_table then
                    if value_is_array then
                        for i=1,#value do
                            config[section_name][value_name][i] = value[i]
                        end
                    else
                        config[section_name][value_name] = value.default
                    end
                else
                    config[section_name][value_name] = value
                end
            end
        elseif section_is_array then
            config[section_name] = {}

            for i=1,#section do
                config[section_name][i] = section[i]
            end
        else
            config[section_name] = section_value
        end
    end

    return serpent.block(config, { comment = false })
end

d2k.config.serialize = function()
    return serpent.block(d2k.cvars, { comment = false })
end

d2k.config.deserialize = function(config_data)
    d2k.cvars = serpent.load(config_data)
end

-- vi: et ts=4 sw=4

