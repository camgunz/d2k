# To Do

## ZDDL

1. Bugs
  - Fix sound
    - I think this is due to the received/run split, clients aren't getting the
      right commands from other clients

1. Add latency mitigation
  - player movement smoothing
    - Working on serverside player command limit
    - Make player position extrapolation optional (makes unlagged less
      accurate)
  - unlagged
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
  - projectile nudging
    - right now, projectiles are predicted, and they therefore usually appear
      ahead of where they would normally spawn
    - projectile nudging would spawn the projectile normally (i.e., right in
      front of the firing player), and adjust the projectile's velocity based
      on the client's current lag and a preselected function (curve)

1. Remove 4 player restriction

1. Add spectators

1. Improve the configuration file and configuration variable system
  - Ties into scripting and console

1. Add 3D physics

1. Add SNDCURVE lump support

1. Add ZDoom physics
  - Including ZDoom SSG spread

1. Improve PWO

1. Improve console
  - Serverside console input
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

1. New widgets
  - Framerate
  - Network stats
  - Add a scoreboard

1. Add a server message sound (just use radio/tink?)

1. When switching to a different interface (menu, HUD, console, etc.), all
   key presses should be cleared and spurious key releases ignored

1. Add a message indicating that the server is full; currently it looks like a
   crash

1. Slow down the super fast doomguy face

## CTF

1. Add slopes

1. Flesh out teams

1. Add Hexen map support

1. Add scripted game modes

1. Add more scripting commands
  - `say_team`
  - `team`

## Version 0.9

1. Make Windows compilation possible
  - Need to build a libXDiff DLL in mingw64-builds

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

1. Update `players`:
  - All playernums should be unsigned shorts
  - `players` will become a `GArray`
  - `playeringame` becomes `bool D_PlayerInGame(unsigned short playernum)`
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

1. Refactor the HUD to use cairo

1. Reimplement the HUD entirely in scripting

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

