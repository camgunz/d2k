# To Do

1. Add a message indicating that the server is full; currently it looks like a
   crash

1. Fix bugs:
  - Cheats don't work (probably scripting input handler)
  - Sometimes the server doesn't call `G_WorldDone` when `gameaction` is set to
    `ga_worlddone`; something sets it back to `ga_nothing` first.
    - It looks this might also set `GS_FINALE`
    - And also it prevents the server from sending `NM_FULLSTATE` to clients
  - Console and messages widget should render during intermission
    - I think widgets need a "render during intermission" flag
  - Windows crashes pretty early on
  - `P_Printf` doesn't work in server mode
  - Messaging needs to print to command-line console if graphics haven't yet
    been initialized
    - Or, initialize graphics much sooner
  - When console scrollback fills up, FPS sinks
  - Add a server message sound (just use radio/tink?)
  - Currently nothing clears or renders when you switch to vidingl from OpenGL
    - Also I think OpenGL mode doesn't refresh properly (console sticks)

1. Test/Fix resolution switching
  - Probably have to override `:reset` in widgets to update a bunch of stuff

:::: Proto Release Here ::::

1. Add spectators
  - This is essentially reworking the logic between `playeringame`, `players`,
    and walkcamera/spycam
  - Move all the `SV_*` calls to `netpeer_t` instead of `playernum`; eventually
    these will apply to peers that are just spectators

1. Remove 4 player restriction

:::: Alpha Release Here ::::

---

1. Move configuration into scripting
  - Refactor screens
    - From what I can tell:
      - screens[0] is the foreground for HUD, status bar, finale, and menu
      - screens[1] is kept blank to erase screens[0]
        ...although it looks like everything in r_draw.c uses it...
      - screens[2] and screens[3] are used for the screen wipe (src and dst
        respectively)
      - screens[4] is the STBAR background
    - `V_CopyRect`
    - `V_FillFlat`
    - `V_FillPatch`
    - `V_DrawBackground`
    - `V_DrawMemPatch`
    - `V_DrawNumPatch`
    - `V_DrawLine`
      - Uses screen[0] always, via PUTDOT -> `V_PlotPixel(0, ...)`
    - `V_FillRect`
    - `V_PlotPixel`

1. Add maplist
  - Add `map` command

1. Add RCON
  - Need a way to intercept console I/O

1. Build master server
  - Create a server description specification in JSON

1. Add WAD downloading
  - cURL
  - the client should do this between frames in case it needs to download a
    huge file (or a file from a slow server); libcurl ought to make this
    pretty easy

:::: Beta Release Here ::::

1. Add 3D physics

1. Add ZDoom physics
  - Including ZDoom SSG spread

1. Add SNDCURVE lump support

1. Improve PWO
  - Probably this is just gonna be a script
    - How to not send an arbitrary script to the server though...?
    - Does it make sense to have scripting contexts per-client serverside?
    - How to configure a script, since it can't be in `d2k_config.lua`?
    - There should maybe be a `local_config.lua` file that runs after config
      processing takes place, to modify and augment the loaded config

1. C/S Demos
  - serverside
  - clientside

1. Add announcer

:::: Version 1 - DEATHMATCH Capable :::

1. Add slopes

1. Add teams

1. Add Hexen map support

1. Add scripted game modes

:: Suitable For Competition Here ::

## Smaller Issues

1. Remove server-as-player hack
  - Use a camera in non-headless mode (add non-headless mode???)

1. Add error codes to `n_proto`

1. Messages widget should (optionally) display markup

1. Servers shouldn't `quit`, they should `shutdown`; also prevents accidentally
   running `/quit` in the console instead of `:quit` and closing the server....

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

## Future/Possible

1. Remove software renderer
  - See about implementing Doom lighting and 8-bit color using shaders

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

