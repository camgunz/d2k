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
d2k.Shortcuts.write = function(text) d2k.console:write(text) end
d2k.Shortcuts.mwrite = function(markup) d2k.console:mwrite(markup) end
d2k.Shortcuts.echo = function(text) d2k.console:echo(text) end
d2k.Shortcuts.mecho = function(markup) d2k.console:mecho(markup) end

func, err = loadfile(d2k.script_folder .. '/local_console_shortcuts.lua', 't')

if not func then
  d2k.console:mecho(string.format('<span color="red">%s</span>', err))
end

-- vi: et ts=2 sw=2

