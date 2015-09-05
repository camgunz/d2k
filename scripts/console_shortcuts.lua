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

d2k.Shortcuts = {}
d2k.Shortcuts.exit = d2k.System.exit
d2k.Shortcuts.quit = d2k.System.quit
d2k.Shortcuts.write = d2k.Messaging.write
d2k.Shortcuts.mwrite = d2k.Messaging.mwrite
d2k.Shortcuts.echo = d2k.Messaging.echo
d2k.Shortcuts.mecho = d2k.Messaging.mecho
d2k.Shortcuts.is_singleplayer = d2k.Net.is_singleplayer
d2k.Shortcuts.is_multiplayer = d2k.Net.is_multiplayer
d2k.Shortcuts.is_client = d2k.Net.is_client
d2k.Shortcuts.is_server = d2k.Net.is_server
d2k.Shortcuts.connect = d2k.Net.connect
d2k.Shortcuts.disconnect = d2k.Net.disconnect
d2k.Shortcuts.reconnect = d2k.Net.reconnect
d2k.Shortcuts.say = d2k.Client.say
d2k.Shortcuts.say_to = d2k.Client.say_to
d2k.Shortcuts.say_to_server = d2k.Client.say_to_server
d2k.Shortcuts.say_to_team = d2k.Client.say_to_team
d2k.Shortcuts.say_to_current_team = d2k.Client.say_to_current_team
d2k.Shortcuts.set_name = d2k.Client.set_name
d2k.Shortcuts.set_team = d2k.Client.set_team
d2k.Shortcuts.set_bobbing = d2k.Client.set_bobbing

func, err = loadfile(d2k.script_folder .. '/local_console_shortcuts.lua', 't')

if not func then
  d2k.Messaging.mecho(string.format('<span color="red">%s</span>', err))
else
  local worked, err = pcall(func)
  if not worked then
    d2k.Messaging.mecho(string.format('<span color="red">%s</span>', err))
  end
end

-- vi: et ts=2 sw=2

