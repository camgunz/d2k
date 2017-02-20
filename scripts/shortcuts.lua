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

local FullShortcuts = {}

function add(sc)
    FullShortcuts[sc.name] = sc
    d2k.Shortcuts[sc.name] = sc.func
end

function get_help(shortcut_name)
    local shortcut = FullShortcuts[shortcut_name]

    if not shortcut then
        return 'No help for ' .. shortcut_name
    elseif not shortcut['help'] then
        return 'No help for ' .. shortcut_name
    else
        return shortcut.help
    end
end


return {
    add = add,
    get_help = get_help,
}

-- vi: et ts=4 sw=4

