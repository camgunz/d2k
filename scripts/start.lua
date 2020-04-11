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

local Fonts = require('fonts')

function cprint(s)
    io.write(string.format('%s\n', s))
end

function cprintf(fmt, ...)
    cprint(string.format(fmt, ...))
end

function echo(s)
    d2k.Messaging.echo(tostring(s))
end

function mecho(s)
    d2k.Messaging.mecho(tostring(s))
end

function echof(fmt, ...)
    d2k.Messaging.echo(string.format(fmt, ...))
end

function mechof(fmt, ...)
    d2k.Messaging.mecho(string.format(fmt, ...))
end

print = echo
mprint = mecho
printf = echof
mprintf = mechof

function write(s)
    d2k.Messaging.write(s)
end

function mwrite(s)
    d2k.Messaging.mwrite(s)
end

function writef(fmt, ...)
    d2k.Messaging.write(string.format(fmt, ...))
end

function mwritef(fmt, ...)
    d2k.Messaging.mwrite(string.format(fmt, ...))
end

function load_config()
    print('X_Init: Loading configuration.')

    local Config = require('config')

    d2k.config = Config.Config()
    require('cvars')
end

function load_input_event_handlers()
    print('X_Init: Loading input event handlers.')

    local InputInterfaces = require('input_interfaces')
    local InputEventDispatcher = require('input_event_dispatcher')

    d2k.interfaces = InputInterfaces.InputInterfaces({name = 'D2K Interfaces'})
    d2k.input_event_dispatcher = InputEventDispatcher.InputEventDispatcher({
        input_interfaces = d2k.interfaces
    })
end

function load_overlay()
    print('X_Init: Creating overlay.')

    local Overlay = require('overlay')

    d2k.overlay = Overlay.Overlay()
end

function load_menu()
    print('X_Init: Creating menu.')

    local Menu = require('menu')

    d2k.menu = Menu.Menu()
    d2k.interfaces:add_interface(d2k.menu)
end

function load_console()
    print('X_Init: Creating console')

    local Console = require('console')

    d2k.console = Console.Console({font = Fonts.get_default_console_font()})
    d2k.interfaces:add_interface(d2k.console)
    d2k.overlay:add_reset_listener(d2k.console)
end

function load_game_interface()
    print('X_Init: Creating game interface')

    local GameInterface = require('game_interface')

    d2k.game_interface = GameInterface.GameInterface()
    d2k.interfaces:add_interface(d2k.game_interface)
end

function load_hud()
    print('X_Init: Creating HUD')

    local HUD = require('hud')

    d2k.hud = HUD.HUD()
    d2k.interfaces:add_interface(d2k.hud)
    d2k.overlay:add_reset_listener(d2k.hud)
end

function load_hud_widgets()
    print('X_Init: Loading HUD widgets')

    func, err = loadfile(d2k.script_folder .. '/local_hud_widgets.lua', 't')

    if func then
        local succeeded, err = pcall(func)
        if not succeeded then
            mprint(string.format('<span color="red">%s</span>', err))
            print(err)
        end
    else
        mprint(string.format('<span color="red">%s</span>', err))
        print(err)
    end
end

function load_console_shortcuts()
    print('X_Init: Loading console shortcuts')
    require('console_shortcuts')
end

function main()
    load_config()
    load_input_event_handlers()

    if d2k.Video.is_enabled() then
        load_overlay()
        load_menu()
        load_console()
        load_game_interface()
        load_hud()
        load_hud_widgets()
    else
        d2k.overlay = nil
        d2k.hud = nil
    end

    load_console_shortcuts()
end

main()

-- vi: et ts=4 sw=4

