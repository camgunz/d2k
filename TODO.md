# To Do

## General

It turns out ENet supports a max 4095 connections.  This makes sense because on
UNIX platforms it uses either `select` or `poll`, and those don't really scale
past that.

I don't like this artificial limitation though, even though it's tempting to
just pick a number below that -- say 2000 -- and say, "we support 2000 players
max".  For one, I think the limits should be dictated by hardware, not
arbitrary limits.  But it also confuses a lot of things.  There should be
multiple counts:
- maximum connections supported (these may not be full clients) 
- maximum clients supported (these may only be spectators, not players)
- maximum players supported

Right now we just use `MAXPLAYERS` for all of these, and that's confusing.

The main issue is working around ENet's artificial limitation.  I actually
think replacing ENet with regular sockets and libuv is fine.  libuv would also
clean up some of the directory server code, which would be welcome.

In the interim, I think `MAXPLAYERS` needs to be removed.  It ultimately needs
to be removed anyway, but the concepts behind `players` and `playeringame` and
`consoleplayer`/`displayplayer` need to be refactored since the list of
`player_t` instances will be dynamic now.

So a TODO list:

- Get netcode working again
  - With a static `players` and with `MAXPLAYERS` set to 4.
  - Also using bitmaps
- Increase `MAXPLAYERS` to 16384
  - The goal here is to find overruns and assumptions
- Refactor `players`, `playeringame`, `consoleplayer`, `displayplayer`
  - `player_t` gets an... `index` field?
  - `void P_PlayersInit(void)`
  - `bool P_PlayersIter(player_t *start)`
  - `player_t* P_PlayersGetNew(void)`
  - `void P_PlayerRemove(player_t *player)`
  - `void P_PlayerIsConsoleplayer(player_t *player)`
  - `void P_PlayerIsDisplayplayer(player_t *player)`
  - `playeringame` disappears completely
- Add config options in the server section for:
  - `max_connections`
  - `max_clients`
  - `max_players`
- Put a soft cap of 2000 on those config options until ENet is removed.
- Resume refactoring the network message packers to use 

## Smaller Issues

1. Add error codes to `n_proto`

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

