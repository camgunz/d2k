# To Do

- Remove ENet
  - Replace with libuv and regular sockets

- Define the distinction between peers, spectators, and players
  - The main thing is that it's possible for there to be peer connected to a
    server that other peers don't ever know about, or there can be peers
    connected to a server that don't have a player.  I ran into this when
    refactoring the chat message stuff; that code currently presumes that if
    there's a peer there's a player, which is wrong if the peer is spectating.
    This has other implications, like assuming that if connected you have a
    player in the game, you have a consoleplayer, and so on.  A lot of work to
    do here probably.

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

