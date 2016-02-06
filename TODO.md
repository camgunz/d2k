# To Do

## Proto

1. Implement relative widget positions
  - `w:set_relative_positioning(true)`
  - `w:center_horizontally()`
  - `w:center_vertically()`

1. Add netstats

1. Add scoreboard
  - keybinding

1. Add maplist
  - Add `map` command

1. Test resolution switching
  - Probably have to override `:reset` in widgets to update a bunch of stuff

1. Do some basic net testing

## After Proto

1. Update/Mirror dependencies somewhere
  - It would be really cool to use the pacman/pkgbuild system and mirror source
    also

## ZDDL

1. Add error codes to `n_proto`

1. Messages widget should (optionally) display markup

1. Menu draws under console
  - This is because the menu is drawn onto the game screen, then the console is
    drawn onto the overlay

1. Servers shouldn't `quit`, they should `shutdown`; also prevents accidentally
   running `/quit` in the console instead of `:quit` and closing the server....

1. Fix switching to vidingl
  - Currently nothing clears or renders when you switch to vidingl from OpenGL

1. Remove 4 player restriction
  - All playernums become `unsigned int`
  - `players` becomes a GHashTable
  - `playeringame` becomes `bool D_PlayerInGame(unsigned int playernum)`
    - Which is a wrapper around `g_hash_table_contains`
  - `MAXPLAYERS` becomes `VANILLA_MAXPLAYERS` for compat
  - Anything defined using `MAXPLAYERS` will be refactored
  - Servers aren't players
    - Use a camera in non-headless mode (add non-headless mode???)
    - Sending a message to the server can use a bool `to_server` instead of the
      `-1` recipient (which is a hack)
  - Player names are hardcoded for DeHackEd; the way this should work is:
    - Keep a private array of the default names ("Green", "Indigo", etc.)
    - After initialization, check if the names have been modified, by a
      DeHackEd patch or anything
    - If so, set each player's name accordingly
    - Of course, this only works for the 1st 4 players; after that, fuck it
      - Maybe assign more colors

1. Add spectators

1. Improve the configuration file and configuration variable system
  - Ties into scripting and console

1. Add auto-scroll to TextWidget

1. Manually delta compress commands (serverside and clientside)
  - bitmap:
    - `index` changed      = 1
    - `tic` changed        = 2
    - `server_tic` changed = 4
    - `forward` changed    = 8
    - `side` changed       = 16
    - `angle` changed      = 32
    - `buttons` changed    = 64
  - then deltas:
    - All diffs are `char` except for `angle`, which is a `short`
    - Effectively limits lag to 255 commands, which is way more than enough

  Hypothetically:
    { 4096, 4012, 4008, 50, 50, 180, 1 } // 17 bytes
    { 4097, 4013, 4008, 50, 50, 180, 1 } // 17 bytes
                                         // Total: 34

  Becomes:
    { 4096, 4012, 4008, 50, 50, 180, 1 } // 17 bytes
    { 3, 1, 1}                           //  3 bytes
                                         // Total: 20, saved: 14

1. Add unlagged
  - Save attacking player position
  - Save current game state
  - Restore game state that player was viewing during the attack
    - This is contained in the command
  - Restore attacking player position (if possible)
  - Run hit detection and damage calculation
    - For every impacted actor:
      - Save momx/momy/momz values
      - Save damagecount
  - Restore saved state
  - For every impacted actor:
    - Add new momx/momy/momz values to the current momx/momy/momz
    - Restore damagecount (MAX(old, new))
  - Might require adding a new field to `netticcmd_t`: `world_tic`
    - Denotes the world the client was looking at when the command was made
    - If the client's enabled extrapolation, unlagged can just use `tic`
    - Otherwise, the client has to stamp the command with the last world it
      loaded from the server, which only the client can know reliably
    - `world_tic` is probably the cleanest way to handle this

1. Implement actor vectors
  - Send movement start TIC
  - x/y/z represent the start position at time of last velocity change
  - More amenable to delta compression
  - Easier to nudge projectiles

1. Add projectile nudging
  - Projectiles are currently predicted, thus they usually appear ahead of
    where they normally would
  - Projectile nudging would spawn the projectile normally (i.e., right in
    front of the firing player) with a higher-than-normal velocity, and then
    adjust the velocity downwards based on the client's lag and a preselected
    function (curve)

1. Add 3D physics

1. Add ZDoom physics
  - Including ZDoom SSG spread

1. Add SNDCURVE lump support

1. Improve PWO
  - Probably this is just gonna be a script
    - How to not send an arbitrary script to the server though...?
    - Does it make sense to have scripting contexts per-client serverside?

1. Add HTTP and JSON (cURL and Jansson)
  - Have client download missing WADs
    - the client should do this between frames in case it needs to download a
      huge file (or a file from a slow server); libcurl ought to make this
      pretty easy
  - Create a server description specification in JSON

1. Add authorization framework

1. Add RCON

1. Add maplist support

1. Little bugs:
  - Add a server message sound (just use radio/tink?)
  - When switching to a different interface (menu, HUD, console, etc.), all key
    presses should be cleared and spurious key releases ignored
  - Add a message indicating that the server is full; currently it looks like a
    crash
  - Slow down the super fast doomguy face

## CTF

1. Add slopes

1. Flesh out teams

1. Add Hexen map support

1. Add scripted game modes

## Version 0.9

1. Remove all vestiges of command sync, it just doesn't make sense anymore

1. Remove software renderer
  - See about implementing Doom lighting and 8-bit color using shaders

1. Setup testing framework
  - Demos
  - Multiplayer command injection

## Version 1.0

1. Settle on types for:
  - Player indices
  - TICs

1. Cleanup #includes

1. Update renderer

1. UDMF

1. Cameras

1. ZIP/PK3 resource files

## Features

1. Bots

1. ACS Scripting

1. DECORATE

1. EDF

1. ExtraData

1. MAPINFO

1. 3D floors

1. 3D MixTex

1. Portals

1. Polyobjects

1. Ambient Sounds

1. Support for other ID Tech 1 games (Heretic, Hexen, etc.)

<!-- vi: set et ts=4 sw=4 tw=79: -->

