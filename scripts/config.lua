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
local serpent = require('serpent')

local CVARS = {}

local CVar = class('CVar')
local Config = class('Config')

local BOOLEAN = 1
local INTEGER = 2
local FLOAT = 3
local STRING = 4
local ARRAY = 5
local TABLE = 6

function is_array(t)
    local is_array = false

    for i, x in ipairs(var) do
        is_array = true
        break
    end

    return is_array
end

function get_type_and_value(var)
    if var_type == 'boolean' then
        return (BOOLEAN, var)
    elseif var_type == 'number' then
        local int = tonumber(var, 10)

        if int ~= nil then
            return (INTEGER, int)
        else
            return (FLOAT, var)
        end
    elseif var_type == 'string' then
        return (STRING, var)
    elseif var_type == 'table' then
        if is_array(var) then
            return (ARRAY, var)
        else
            return (TABLE, var)
        end

    else
        error(string.format('Invalid type "%s"', var_type))
    end
end

function CVar:initialize(args)
    if not args then
        error('No arguments passed to cvar:initialize')
    end

    self._name = args.name
    self._help = args.help or 'No help text yet'

    self._type = nil
    self._default = nil
    self._value = nil
    self._options = {}
    self._min = nil
    self._max = nil

    local var_type, var_value = get_value_and_type(args.default)

    if var_type == 'table' then
        -- This can be:
        -- - { values... }
        -- - { default = default }
        -- - { default = default, options = { values... } }
        -- - { default = default, min = min }
        -- - { default = default, max = max }
        -- - { default = default, min = min, max = max }

        if var['default'] == nil then
            error(string.format('CVar %s missing "default" key', self._name))
        end

        local default = var['default']

        default_type, default_value = get_value_and_type(default)

        if default_type == TABLE then
            error(string.format(
                'Default value for CVar %s cannot be a table',
                self._name
            ))
        end

        self._type = default_type
        self._default = default_value
        self._value = default

        if var['options'] ~= nil then
            if var['min'] ~= nil then
                error(string.format(
                    'Cannot specify both "options" and "min" for CVar %s',
                    self._name
                ))
            end

            if var['max'] ~= nil then
                error(string.format(
                    'Cannot specify both "options" and "max" for CVar %s',
                    self._name
                ))
            end

            if self._type ~= INTEGER and
               self._type ~= FLOAT and
               self._type ~= STRING then
                error(string.format(
                    'CVar %s does not support "options", only integers, '
                    'floats and strings do',
                    self._name
                ))
            end

            self._options = var['options']
        end

        if var['min'] ~= nil and not (self._type == INTEGER or
                                      self._type == FLOAT) then
            error(string.format(
                'CVar %s does not support "min", only integers and floats do',
                self._name
            ))
        end

        if var['max'] ~= nil and not (self._type == INTEGER or
                                      self._type == FLOAT) then
            error(string.format(
                'CVar %s does not support "max", only integers and floats do',
                self._name
            ))
        end

        self._min = var['min']
        self._max = var['max']
    end
end

function CVar:get_name()
    return self._name
end

function CVar:get_type()
    return self._type
end

function CVar:get_value()
    return self._value
end

function CVar:get_default()
    return self._default
end

function CVar:get_options()
    return self._options
end

function CVar:get_min()
    self._min = nil
    self._max = nil

function CVar:set_value(var)
    var_type = type(var)

    if #self._options and not tablex.find(self._options, var) then
        error(string.format('Cannot assign invalid option %s to %s',
            var,
            self._name
        ))
    end

    if self._min ~= nil and var < self._min then
        error(string.format('Cannot assign %s to %s, value is too low',
            var,
            self._name
        ))
    end

    if self._min ~= nil and var < self._max then
        error(string.format('Cannot assign %s to %s, value is too large',
            var,
            self._name
        ))
    end

    if var_type == 'table' and is_array(var) and self._type == ARRAY then
        self._value = var
    elseif var_type == 'boolean' and self._type == BOOLEAN then
        self._value = var
    elseif var_type == 'integer' and self._type == INTEGER then
        self._value = var
    elseif var_type == 'float' and self._type == FLOAT then
        self._value = var
    elseif var_type == 'string' and self._type == STRING then
        self._value = var
    else
        error(string.format('Cannot assign value of type %s to %s',
            var_type,
            self._name
        ))
    end
end

function cvar(args)
    local section_path = args.name
    local help = args.help

    local parts = section_path.split('.')
    local namespaces = table.unpack(parts, i, #parts - 1)
    local name = parts[#parts]
    local section = CVARS

    for i, namespace in ipairs(namespaces) do
        if section[namespace] == nil then
            section[namespace] = {}
        else
            section = section[namespace]
        end
    end

    namespace[name] = CVar({
        name = name,
        help = help,
        default = args.default,
        options = args.options,
        min = args.min,
        max = arg.max,
    })
end

return {
    CVar = CVar
    BOOLEAN = BOOLEAN,
    INTEGER = INTEGER,
    FLOAT = FLOAT,
    STRING = STRING,
    ARRAY = ARRAY
    CVAR = CVAR,
}

function validate(schema)
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

function build_from_defaults(schema)
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

--[[
So the steps here are:

- D2K calls d2k.config.init() (or whatever), which builds the main config
  object in memory at d2k.config
- D2K loads the configuration file data, passing it to
  d2k.config:update_from_file_data
- d2k.config:update_from_file_data:
  - deserializes the config, storing it in a local variable
  - validates the config, walking through it and its tree of CVARs
  - if a config value is invalid, the cvar's value is not set and a warning is
    printed to the console
--]]

function Config:initialize()
    for name, section in pairs(CVARS)
end

d2k.config.validate = function()
    local ConfigSchema = require('config_schema')
    local schema = ConfigSchema.ConfigSchema
    validate(schema)
end

d2k.config.build_from_defaults = function()
    local ConfigSchema = require('config_schema')
    local schema = ConfigSchema.ConfigSchema

    return get_default(schema)
end

d2k.config.serialize = function()
    return serpent.block(d2k.cvars, { comment = false })
end

d2k.config.deserialize = function(config_data)
    d2k.cvars = serpent.load(config_data)
end

-- vi: et ts=4 sw=4

