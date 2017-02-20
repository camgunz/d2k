-------------------------------------------------------------------------------
--                                                                           --
-- Place any custom console shortcuts in this file                           --
--                                                                           --
-------------------------------------------------------------------------------

local Shortcuts = require('shortcuts')
local Shortcut = Shortcuts.add

Shortcut {
    name = 'enable_widget',
    help = 'enable_widget [widget_name]\n  Enable a widget',
    func = function(widget_name)
        d2k.hud:add_interface(d2k.widgets[widget_name])
    end
}

Shortcut {
    name = 'disable_widget',
    help = 'disable_widget [widget_name]\n  Disable a widget',
    func = function(widget_name)
        d2k.hud:remove_interface(d2k.widgets[widget_name])
    end
}

Shortcut {
    name = 'help',
    help = 'help <command>\n  List commands or print help for a command',
    func = function(command_name)
        if not command_name then
            for name,func in pl.tablex.sort(d2k.Shortcuts) do
                print(name)
            end
        else
            print(Shortcuts.get_help(command_name))
        end
    end
}

print('Loaded local console shortcuts')

-- vi: et ts=4 sw=4

