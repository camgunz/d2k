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

local Shortcut = require('shortcuts').add

Shortcut {
    name = 'exit',
    help = 'exit\n  Alias for quit',
    func = d2k.System.exit
}

Shortcut {
    name = 'quit',
    help = 'quit\n  Quit D2K',
    func = d2k.System.quit
}

Shortcut {
    name = 'write',
    help = 'write [message]\n  Write a message to the console, escape markup',
    func = d2k.Messaging.write
}

Shortcut {
    name = 'mwrite',
    help = 'mwrite [message]\n' ..
           '  Write a message to the console, process markup',
    func = d2k.Messaging.mwrite
}


Shortcut {
    name = 'echo',
    help = 'echo [message]\n' ..
           '  Write a message and a newline to the console, escape markup',
    func = d2k.Messaging.echo
}

Shortcut {
    name = 'mecho',
    help = 'mecho [message]\n' ..
           '  Write a message and a newline to the console, process markup',
    func = d2k.Messaging.mecho
}

Shortcut {
    name = 'is_singleplayer',
    help = 'is_singleplayer\n' ..
           '  Returns true if the game is in singleplayer mode',
    func = d2k.Net.is_singleplayer
}

Shortcut {
    name = 'is_multiplayer',
    help = 'is_multiplayer\n' ..
           '  Returns true if the game is in multiplayer mode',
    func = d2k.Net.is_multiplayer
}

Shortcut {
    name = 'is_client',
    help = 'is_client\n  Returns true if D2K is running as a client',
    func = d2k.Net.is_client
}

Shortcut {
    name = 'is_server',
    help = 'is_server\n  Returns true if D2K is running as a server',
    func = d2k.Net.is_server
}

Shortcut {
    name = 'connect',
    help = 'connect [address:port]\n  Connects to a remote server',
    func = d2k.Net.connect
}

Shortcut {
    name = 'disconnect',
    help = 'disconnect\n  Disconnects from the server',
    func = d2k.Net.disconnect
}

Shortcut {
    name = 'reconnect',
    help = 'reconnect\n  Reconnects to the previous server',
    func = d2k.Net.reconnect
}

Shortcut {
    name = 'say',
    help = 'say [message]\n  Broadcasts a message',
    func = d2k.Client.say
}

Shortcut {
    name = 'say_to',
    help = 'say_to [playernum] [message]\n  Sends a message to a player',
    func = d2k.Client.say_to
}

Shortcut {
    name = 'say_to_server',
    help = 'say_to_server [message]\n  Sends a message to the server',
    func = d2k.Client.say_to_server
}

Shortcut {
    name = 'say_to_team',
    help = 'say_to_team [message]\n  Broadcast a message to your current team',
    func = d2k.Client.say_to_team
}

Shortcut {
    name = 'set_name',
    help = 'set_name [name]\n  Set your name',
    func = d2k.Client.set_name
}

Shortcut {
    name = 'set_team',
    help = 'set_team [team]\n  Set your team',
    func = d2k.Client.set_name
}

Shortcut {
    name = 'set_bobbing',
    help = 'set_bobbing [value]\n  Set your player bobbing value',
    func = d2k.Client.set_bobbing
}

func, err = loadfile(d2k.script_folder .. '/local_console_shortcuts.lua', 't')

if not func then
    d2k.Messaging.mecho(string.format('<span color="red">%s</span>', err))
else
    local worked, err = pcall(func)
    if not worked then
        d2k.Messaging.mecho(string.format('<span color="red">%s</span>', err))
    end
end

-- vi: et ts=4 sw=4

