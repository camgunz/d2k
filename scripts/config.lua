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

    for i, x in ipairs(t) do
        is_array = true
        break
    end

    return is_array
end

function get_type_and_value(var)
    local var_type = type(var)

    if var_type == 'boolean' then
        return BOOLEAN, var
    elseif var_type == 'number' then
        local int = tonumber(var, 10)

        if int ~= nil then
            return INTEGER, int
        else
            return FLOAT, var
        end
    elseif var_type == 'string' then
        return STRING, var
    elseif var_type == 'table' then
        if is_array(var) then
            return ARRAY, var
        else
            return TABLE, var
        end
    else
        cprint(debug.traceback())
        error(string.format('Invalid type "%s"', var_type))
    end
end

function CVar:initialize(args)
    if not args then
        error('No arguments passed to cvar:initialize')
    end

    self._name = args.name
    self._help = args.help or 'Sorry, no help text yet'
    self._default = args.default

    cprint('cvar:')
    for name, value in pairs(args) do
        cprint(string.format('  %s: %s', name, value))
    end
    cprint('')

    self._type, self._value = get_type_and_value(args.default)

    self._options = args['options'] or nil
    self._min = args['min'] or nil
    self._max = args['max'] or nil

    if self._options ~= nil and self._min ~= nil then
        error(string.format(
            'Cannot specify both "options" and "min" for CVar %s',
            self._name
        ))
    end

    if self._options ~= nil and self._max ~= nil then
        error(string.format(
            'Cannot specify both "options" and "max" for CVar %s',
            self._name
        ))
    end
    if self._options ~= nil and self._type ~= INTEGER and
                                self._type ~= FLOAT and
                                self._type ~= STRING then
        error(string.format([[
                CVar %s does not support "options", only integers, floats, and
                strings do
            ]],
            self._name
        ))
    end

    if self._min ~= nil and not (self._type == INTEGER or
                                 self._type == FLOAT) then
        error(string.format(
            'CVar %s does not support "min", only integers and floats do',
            self._name
        ))
    end

    if self._max ~= nil and not (self._type == INTEGER or
                                 self._type == FLOAT) then
        error(string.format(
            'CVar %s does not support "max", only integers and floats do',
            self._name
        ))
    end
end

function CVar:get_name()
    return self._name
end

function CVar:get_type()
    return self._type
end

function CVar:get_default()
    return self._default
end

function CVar:get_options()
    return self._options
end

function CVar:get_min()
    return self._min
end

function CVar:get_max()
    return self._max
end

function CVar:get_value()
    return self._value
end

function CVar:set_value(var)
    local var_type = type(var)
    local options = self:get_options()

    if options ~= nil and not tablex.find(options, var) then
        error(string.format('Cannot assign invalid option %s to %s',
            var,
            self:get_name()
        ))
    end

    if self:get_min() ~= nil and var < self:get_min() then
        error(string.format(
            'Cannot assign %s to %s, value is below the minimum (%s)',
            var,
            self:get_name(),
            self:get_min()
        ))
    end

    if self:get_max() ~= nil and var < self:get_max() then
        error(string.format(
            'Cannot assign %s to %s, value is above the maximum (%s)',
            var,
            self:get_name(),
            self:get_min()
        ))
    end

    if var_type == 'table' and is_array(var) and self:get_type() == ARRAY then
        self._value = var
    elseif var_type == 'boolean' and self:get_type() == BOOLEAN then
        self._value = var
    elseif var_type == 'integer' and self:get_type() == INTEGER then
        self._value = var
    elseif var_type == 'float' and self:get_type() == FLOAT then
        self._value = var
    elseif var_type == 'string' and self:get_type() == STRING then
        self._value = var
    else
        error(string.format('Cannot assign value of type %s to %s',
            var_type,
            self:get_name()
        ))
    end

    d2k.config_file.save()
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
    self._cvars = {}
end

function Config:get_cvars()
    return self._cvars
end

function Config:get_section(path)
    local parts = path.split('.')
    local namespaces = table.unpack(parts, i, #parts - 1)
    local name = parts[#parts]
    local section = self:get_cvars()

    for i, namespace in ipairs(namespaces) do
        if section[namespace] == nil then
            error(string.format('No CVar "%s"', path))
        else
            section = section[namespace]
        end
    end

    return section
end

function Config:get_or_make_section_and_name(path)
    local parts = path:split('.')
    local namespaces = {}
    local name = parts[#parts]
    local section = self:get_cvars()

    for i, part in ipairs(parts) do
        if i ~= #parts then
            table.insert(namespaces, part)
        end
    end

    for i, namespace in ipairs(namespaces) do
        if section[namespace] == nil then
            section[namespace] = {}
        else
            section = section[namespace]
        end
    end

    return section, name
end

function Config:get_section_and_name(path)
    local parts = path:split('.')
    local namespaces = {}
    local name = parts[#parts]
    local section = self:get_cvars()

    for i, part in ipairs(parts) do
        if i ~= #parts then
            table.insert(namespaces, part)
        end
    end

    for i, namespace in ipairs(namespaces) do
        if section[namespace] == nil then
            error(string.format('No CVar "%s"', path))
        else
            section = section[namespace]
        end
    end

    return section, name
end

function Config:get_cvar(path)
    local section, name = self:get_section_and_name(path)

    return section[name]
end

function Config:set_cvar(path, cvar)
    local section, name = self:get_section_and_name(path)

    section[name] = cvar
end

function Config:get_values()
    local values = {}

    function walk(section, values)
        for name, value in pairs(self:get_cvars()) do
            if type(value) == 'table' then
                if value['isInstanceof'] and value:isInstanceOf('CVar') then
                    values[name] = value:get_value()
                else
                    values[name] = {}
                    if is_array(value) then
                        for i, element in ipairs(value) do
                            table.insert(values[name], element)
                        end
                    else
                        walk(section[name], values[name])
                    end
                end
            end
        end
    end

    walk(self:get_cvars(), values)

    return values
end

function Config:update(values)
    local cvars = self:get_cvars()

    for name, value in pairs(values) do
        local value_type = type(value)

        if value_type == 'table' and not is_array(value) then
            validate(cvars[name], config[name])
        else
            local succeeded, err = pcall(function()
                config['name']:set_value(value)
            end)

            if not succeeded then
                mprint(string.format('<span color="red">%s</span>', err))
            end
        end
    end
end

function Config:serialize()
    return serpent.block(self:get_values(), { comment = false })
end

function cvar(args)
    print(string.format('Adding cvar %s (%s)', args.name, args.default))

    local section, name = d2k.config:get_or_make_section_and_name(args.name)

    args.name = name

    section[name] = CVar(args)
end

return {
    CVar = CVar,
    Config = Config,
    BOOLEAN = BOOLEAN,
    INTEGER = INTEGER,
    FLOAT = FLOAT,
    STRING = STRING,
    ARRAY = ARRAY,
    cvar = cvar,
}

-- vi: et ts=4 sw=4

