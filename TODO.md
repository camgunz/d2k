# To Do

1. Fix bugs:
  - The client will still occasionally not clear commands out
  - `P_Printf` doesn't work in server mode
  - When console scrollback fills up, FPS sinks
  - Add a server message sound (just use radio/tink?)
  - When switching to a different interface (menu, HUD, console, etc.), all key
    presses should be cleared and spurious key releases ignored
  - Add a message indicating that the server is full; currently it looks like a
    crash
  - Slow down the super fast doomguy face
  - Maybe something like `enet_flush_whatever` will kill the minimum ping

1. Need a "not enough spawnpoints" strategy

1. Test resolution switching
  - Probably have to override `:reset` in widgets to update a bunch of stuff

1. Trace messaging uses after message channels are closed

-- Proto Complete Here --

1. Improve the configuration file and configuration variable system
  - Ties into scripting and console
  - JSON (Jansson)

1. Add maplist
  - Add `map` command

1. Add authorization framework

1. Add RCON

1. Build master server
  - Create a server description specification in JSON

1. Add WAD downloading
  - cURL
  - the client should do this between frames in case it needs to download a
    huge file (or a file from a slow server); libcurl ought to make this
    pretty easy

1. Add drawing options to draw in the old PrBoom+/Doom style
  - Need to refactor `screens`

1. Move menu into scripting
  - This is because the menu is drawn onto the game screen, then the console is
    drawn onto the overlay

1. Remove 4 player restriction
  - All playernums become `unsigned int`
  - `players` becomes a GPtrArray
  - `playeringame` becomes `bool D_PlayerInGame(unsigned int playernum)`
    - Which just iterates
  - `MAXPLAYERS` becomes `VANILLA_MAXPLAYERS` for compat
  - Anything defined using `MAXPLAYERS` will be refactored
  - Player names are hardcoded for DeHackEd; the way this should work is:
    - Keep a private array of the default names ("Green", "Indigo", etc.)
    - After initialization, check if the names have been modified, by a
      DeHackEd patch or anything
    - If so, set each player's name accordingly
    - Of course, this only works for the 1st 4 players; after that, fuck it
      - Maybe assign more colors

1. Add spectators

1. Add 3D physics

1. Add ZDoom physics
  - Including ZDoom SSG spread

1. Add SNDCURVE lump support

1. Improve PWO
  - Probably this is just gonna be a script
    - How to not send an arbitrary script to the server though...?
    - Does it make sense to have scripting contexts per-client serverside?

-- Suitable For DEATHMATCH Here --

1. Add announcer

1. Add slopes

1. Add teams

1. Add Hexen map support

1. Add scripted game modes

-- Suitable For Competition Here --

## Smaller Issues

1. Remove server-as-player hack
  - Use a camera in non-headless mode (add non-headless mode???)

1. Add error codes to `n_proto`

1. Messages widget should (optionally) display markup

1. Servers shouldn't `quit`, they should `shutdown`; also prevents accidentally
   running `/quit` in the console instead of `:quit` and closing the server....

1. Fix switching to vidingl
  - Currently nothing clears or renders when you switch to vidingl from OpenGL

1. Add auto-scroll to TextWidget

1. Update/Mirror dependencies somewhere
  - It would be really cool to use the pacman/pkgbuild system and mirror source
    also

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

## Future

1. Remove software renderer
  - See about implementing Doom lighting and 8-bit color using shaders

1. Setup testing framework
  - Demos
  - Multiplayer command injection

## Features

1. UDMF

1. ZIP/PK3 resource files

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

