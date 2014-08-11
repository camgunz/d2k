# To Do

## Prototype

1. Fix problems
  * Monsters freaking out randomly
    * Dump the game options array to check
  * Fix intermission (single-player is busted too)
    * Handle player input during intermission
  * Test quitting and rejoining
  * Test sounds with more than 1 player

1. Fix messaging
  * Walking over stuff prints 5000 messages to the console
  * Add more scripting commands
    * `say`
    * `say_team`
    * `alias`
    * `idrate` (make a widget for this?)

1. Make playing a game possible in general
  * Probably need stuff like fraglimit or whatever

## ZDDL

1. Add latency mitigation
  - projectile nudging
  - skip correction
  - unlagged
    - Save attacking player position
    - Save current game state
    - Restore game state that player was viewing during the attack
      - This is contained in the command
    - Restore attacking player position (if possible)
    - Run hit detection and damage calculation
      - For every impacted actor:
        - Save momx/momy/momz values
    - Restore saved state
    - For every impacted actor:
      - Add new momx/momy/momz values to the current momx/momy/momz

1. Add spectators

1. Add a scoreboard

1. Better configuration file and configuration variable system
  - Ties into scripting and console

1. Improve console
  - Command history
    - Do this yourself
  - Tab-completion
  - Clipboard (cut/copy/paste)
  - Selection
    - Mouse

1. Add HTTP and JSON (cURL and Jansson)
  - Have client download missing WADs
    - the client should do this between frames in case it needs to download a
      huge file (or a file from a slow server); libcurl ought to make this
      pretty easy
  - Create a server description specification in JSON

1. 3D physics

1. SNDCURVE

1. ZDoom physics
  - ZDoom SSG spread

1. PWO

1. Slopes

## CTF

1. Hexen map support

1. Scripted game modes

## Version 1.0

1. Make Windows compilation possible
  * Need to build a libXDiff DLL in mingw64-builds

1. Remove all vestiges of command sync, it just doesn't make sense anymore

1. Remove software renderer
  * See about implementing Doom lighting and 8-bit color using shaders

1. Setup testing framework
  - Demos
  - Multiplayer command injection

## Miscellaneous

1. Settle on types for:
    - Player indices
    - TICs

1. Cleanup #includes

1. Update renderer

1. Update `players`:
  - `players` will become an `obuf_t`
  - `playeringame` becomes `dboolean playeringame(unsigned short playernum)`
  - `MAXPLAYERS` becomes `VANILLA_MAXPLAYERS` for compat
  - Anything defined using `MAXPLAYERS` will be refactored
  - Servers aren't players
    - Use a camera in non-headless mode
    - All playernums should be unsigned shorts
    - Sending a message to the server can use a bool `to_server` instead of the
      `-1` recipient (which is a hack)
  - Player names are hardcoded for DeHackEd; the way this should work is:
    - Keep a private array of the default names ("Green", "Indigo", etc.)
    - After initialization, check if the names have been modified, by a
      DeHackEd patch or anything
    - If so, set each player's name accordingly
    - Of course, this only works for the 1st 4 players; after that, fuck it

1. Refactor the HUD to use cairo

1. Reimplement the HUD entirely in scripting

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

1. Cameras

1. UDMF

1. ZIP/PK3 resource files

<!-- vi: set et ts=4 sw=4 tw=79: -->

