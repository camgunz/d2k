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
-- with D2K.  If not, see <http:--www.gnu.org/licenses/>.                    --
--                                                                           --
-------------------------------------------------------------------------------

-- [CG] [FIXME] These cheats are all strongly tied to Doom, assuming the same
--              keys, weapons, etc.  So I think there should probably be a
--              namespace for this kind of thing.

Cheat = d2k.cheat_engine.add_cheat

function get_give_power_func(pow)
    return function()
        local consoleplayer = d2k.Game.get_consoleplayer()
        consoleplayer.add_permanent_power(pow)
    end
end

function get_toggle_key_func(key)
    return function()
        local consoleplayer = d2k.Game.get_consoleplayer()

        if not consoleplayer then
            return
        end

        if consoleplayer:has_key(key) then
            consoleplayer:remove_key(key)
        else
            consoleplayer:give_key(key)
        end
    end
end

Cheat {
    code = 'idmus',
    description = 'Change music',
    when = CheatEngine.ALWAYS,
    func = function(tens, ones)
        if type(tens) ~= 'number' or type(ones) ~= 'number' then
            return
        end

        local musnum = tens * 10 + ones

        d2k.Messaging.echo(d2k.Deh.s_STSTR_MUS)
        d2k.Music.set_track(musnum)
    end
}

Cheat {
    code = 'idchoppers',
    description = 'Chainsaw',
    when = CheatEngine.NEVER,
    func = function ()
        local consoleplayer = d2k.Game.get_consoleplayer()

        if not consoleplayer then
            return
        end

        consoleplayer:add_power(d2k.Game.Powers.Invulnerability)
        consoleplayer:add_weapon(d2k.Game.Weapons.Chainsaw)
        -- Ty 03/27/98 - externalized
        d2k.Messaging.echo(d2k.Deh.s_STSTR_CHOPPERS)
    end
}

Cheat {
    code = 'iddqd',
    description = 'God mode',
    when = CheatEngine.NEVER,
    func = function()
        local consoleplayer = d2k.Game.get_consoleplayer()

        if not consoleplayer then
            return
        end

        if consoleplayer:is_cheating(d2k.Game.Cheats.GodMode) then
            consoleplayer.remove_cheat(d2k.Game.Cheats.GodMode)
            -- Ty 03/27/98 - externalized
            d2k.Messaging.echo(d2k.Deh.s_STSTR_DQDOFF);
            return
        end

        local actor = consoleplayer:get_actor()

        if actor then
            actor:set_health(d2k.Deh.GodHealth)
        end

        consoleplayer:set_health(d2k.Deh.GodHealth)
        -- Ty 03/27/98 - externalized
        d2k.Messaging.echo(d2k.Deh.s_STSTR_DQDON);
    end
}

Cheat {
    code = 'idkfa',
    description = 'Ammo & Keys',
    when = CheatEngine.NEVER,
    func = function()
        local consoleplayer = d2k.Game.get_consoleplayer()

        if not consoleplayer then
            return
        end

        if not consoleplayer:has_backpack() then
            consoleplayer:double_ammo()
            console:set_has_backpack(true)
        end

        -- Ty 03/09/98 - deh
        consoleplayer:set_armor(d2k.Deh.idfa_armor)
        -- Ty 03/09/98 - deh
        consoleplayer:set_armor_type(d2k.Deh.idfa_armor_class)
    end
}

Cheat {
    code = 'idfa',
    description = 'Ammo',
    when = CheatEngine.NEVER,
}

Cheat {
    code = 'idspispopd',
    description = 'No Clipping 1',
    when = CheatEngine.NEVER,
}

Cheat {
    code = 'idclip',
    description = 'No Clipping 2',
    when = CheatEngine.NEVER,
}

-- CPhipps - new health and armour cheat codes
Cheat {
    code = 'idbeholdh',
    description = 'Invincibility',
    when = CheatEngine.NEVER,
    func = function()
        local consoleplayer = d2k.Game.get_consoleplayer()

        if not consoleplayer then
            return
        end

        if consoleplayer:is_cheating(d2k.Game.Cheats.GodMode) then
            return
        end

        local actor = consoleplayer:get_actor()

        if actor then
            actor:set_health(d2k.Deh.mega_health)
        end

        consoleplayer:set_health(d2k.Deh.mega_health)
        -- Ty 03/27/98 - externalized
        d2k.Messaging.echo(s_STSTR_BEHOLDX)
    end
}

-- CPhipps - new health and armour cheat codes
Cheat {
    code = 'idbeholdm',
    description = 'Invincibility',
    when = CheatEngine.NEVER,
    func = function()
        local consoleplayer = d2k.Game.get_consoleplayer()

        if not consoleplayer then
            return
        end

        -- Ty 03/09/98 - deh
        consoleplayer:set_armor(d2k.Deh.idfa_armor)
        -- Ty 03/09/98 - deh
        consoleplayer:set_armor_type(d2k.Deh.idfa_armor_class)
        -- Ty 03/27/98 - externalized
        d2k.Messaging.echo(s_STSTR_BEHOLDX)
    end
}

Cheat {
    code = 'idbeholdv',
    description = 'Invincibility',
    when = CheatEngine.NEVER,
    func = get_give_power_func(dk2.Game.Powers.Invulnerability)
}

Cheat {
    code = 'idbeholds',
    description = 'Berserk',
    when = CheatEngine.NEVER,
    func = get_give_power_func(dk2.Game.Powers.Strength)
}

Cheat {
    code = 'idbeholdi',
    description = 'Invisibility',
    when = CheatEngine.NEVER,
    func = get_give_power_func(dk2.Game.Powers.Invisibility)
}

Cheat {
    code = 'idbeholdr',
    description = 'Radiation Suit',
    when = CheatEngine.NEVER,
    func = get_give_power_func(d2k.Game.Powers.IronFeet)
}

Cheat {
    code = 'idbeholda',
    description = 'Auto-map',
    when = CheatEngine.NOT_DM,
    func = get_give_power_func(dk2.Game.Powers.AllMap)
}

Cheat {
    code = 'idbeholdl',
    description = 'Lite-Amp Goggles',
    when = CheatEngine.NOT_DM,
    func = get_give_power_func(dk2.Game.Powers.Infrared)
}

Cheat {
    code = 'idbehold',
    description = 'BEHOLD menu',
    when = CheatEngine.NOT_DM,
}

Cheat {
    code = 'idclev',
    description = 'Level Warp',
    when = CheatEngine.NEVER | CheatEngine.NOT_MENU,
}

Cheat {
    code = 'idmypos',
    description = 'Player Position',
    when = CheatEngine.NOT_DM,
}

Cheat {
    code = 'idrate',
    description = 'Frame rate',
    when = CheatEngine.ALWAYS,
    func = function()
        local fps_widget = d2k.widgets['fps']

        if fps_widget.enabled() then
            fps_widget.disable()
        else
            d2k.widgets['fps'].enable()
        end
    end
}

-- phares
Cheat {
    code = 'tntcomp'
    when = CheatEngine.NEVER,
}

-- jff 2/01/98 kill all monsters
Cheat {
    code = 'tntem'
    when = CheatEngine.NEVER,
}

-- killough 2/07/98: moved from am_map.c
Cheat {
    code = 'iddt',
    description = 'Map cheat',
    when = CheatEngine.NOT_DM,
}

-- killough 2/07/98: HOM autodetector
Cheat {
    code = 'tnthom'
    when = CheatEngine.ALWAYS,
}

-- killough 2/16/98: generalized key cheats
Cheat {
    code = 'tntkey'
    when = CheatEngine.NEVER,
    func = function()
        d2k.Messaging.echo('Red, Yellow, Blue')
    end
}

Cheat {
    code = 'tntkeyr'
    when = CheatEngine.NEVER,
    func = function()
        d2k.Messaging.echo('Card, Skull')
    end
}

Cheat {
    code = 'tntkeyy'
    when = CheatEngine.NEVER,
}

Cheat {
    code = 'tntkeyb'
    when = CheatEngine.NEVER,
}

Cheat {
    code = 'tntkeyrc'
    when = CheatEngine.NEVER,
    func = get_give_key_func(dk2.Game.Keys.RedKeyCard)
}

Cheat {
    code = 'tntkeyyc'
    when = CheatEngine.NEVER,
    func = get_give_key_func(dk2.Game.Keys.YellowKeyCard)
}

Cheat {
    code = 'tntkeybc'
    when = CheatEngine.NEVER,
    func = get_give_key_func(dk2.Game.Keys.BlueKeyCard)
}

Cheat {
    code = 'tntkeyrs'
    when = CheatEngine.NEVER,
    func = get_give_key_func(dk2.Game.Keys.RedSkullKey)
}

Cheat {
    code = 'tntkeyys'
    when = CheatEngine.NEVER,
    func = get_give_key_func(dk2.Game.Keys.YellowSkullKey)
}

Cheat {
    code = 'tntkeybs'
    when = CheatEngine.NEVER,
    func = get_give_key_func(dk2.Game.Keys.BlueSkullKey)
}

-- killough 2/16/98: end generalized keys

-- Ty 04/11/98 - Added TNTKA
Cheat {
    code = 'tntka'
    when = CheatEngine.NEVER,
}

-- killough 2/16/98: generalized weapon cheats

Cheat {
    code = 'tntweap'
    when = CheatEngine.NEVER,
}

Cheat {
    code = 'tntweap'
    when = CheatEngine.NEVER,
}

Cheat {
    code = 'tntammo'
    when = CheatEngine.NEVER,
}

Cheat {
    code = 'tntammo'
    when = CheatEngine.NEVER,
}

-- killough 2/16/98: end generalized weapons

-- invoke translucency         -- phares
Cheat {
    code = 'tnttran'
    when = CheatEngine.ALWAYS,
}

-- killough 2/21/98: smart monster toggle
Cheat {
    code = 'tntsmart'
    when = CheatEngine.NEVER,
}

-- killough 2/21/98: pitched sound toggle
Cheat {
    code = 'tntpitch'
    when = CheatEngine.ALWAYS,
}

-- killough 2/21/98: reduce RSI injury by adding simpler alias sequences:
-- killough 2/21/98: same as tnttran
Cheat {
    code = 'tntran'
    when = CheatEngine.ALWAYS,
}

-- killough 2/21/98: same as tntammo
Cheat {
    code = 'tntamo'
    when = CheatEngine.NEVER,
}

-- killough 2/21/98: same as tntammo
Cheat {
    code = 'tntamo'
    when = CheatEngine.NEVER,
}

-- killough 3/6/98: -fast toggle
Cheat {
    code = 'tntfast'
    when = CheatEngine.NEVER,
}

-- phares 3/10/98: toggle variable friction effects
Cheat {
    code = 'tntice'
    when = CheatEngine.NEVER,
}

-- phares 3/10/98: toggle pushers
Cheat {
    code = 'tntpush'
    when = CheatEngine.NEVER,
}

-- [RH] Monsters don't target
Cheat {
    code = 'notarget'
    when = CheatEngine.NEVER,
}

-- fly mode is active
Cheat {
    code = 'fly'
    when = CheatEngine.NEVER,
}

