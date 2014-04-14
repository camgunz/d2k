# To Do

## Notes

In command sync, servers send an array of commands for each player, so:

    [
      0: cmd1, cmd2,
      1: cmd1
      2: cmd1, cmd2, cmd3, cmd4
      3: cmd1, cmd2, cmd3
    ]

In delta sync, the command arrays are stored in each `player_t`, which leads to
the same result.

This accomplishes two things:

- Each player receives the other players' commands, and can therefore run a
  tic; in this example `cmd1` is present for all players, so the receiving
  player can run 1 tic.
- The receiving player knows which of its commands have made it to the server
  and can pop them off the local command buffer

`maketic` and `gametic` are always sync'd, meaning that a `netticcmd_t`'s `tic`
member always refers to the tic in which it is intended to be run.

There will never be holes in ncmd buffers, because clients only flush them when
the server has acknowledged receipt, either through the above method or through delta
sync.

Clients must trim their local player command buffers whenever sync is received;
either via a command array or a delta.

Servers have to keep track of which commands each client has acknowledged
receipt of.

## To Do (For Real)

1. Have a look at command & state buffer trimming; the chance these are correct
   is very small.

1. Start testing prototype

---

1. Fix command-sync netcode

1. If there are errors while reading a network message, message reading
   completely bails, leaving the message partially read.  I'm not totally sure
   what to do about that though.  I'm dissatisfied with attaching a size to the
   message, but I think that's the only solution.  Unfortunately, this
   necessitates a pretty thorough reworking of the netcode.

1. Add unlagged
  - Save attacking player position
  - Save current game state
  - Restore game state that player was viewing during the attack
    - This is contained in the command
  - Set attacking player position (if possible)
  - Run hit detection & damage calculation
    - For every impacted actor:
      - Save momx/momy/momz values
  - Restore saved state
  - For every impacted actor:
    - Add new momx/momy/momz values to the current momx/momy/momz

1. Setup testing framework

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

1. Add HTTP & JSON
  - Have client download missing WADs
    - the client should do this between frames in case it needs to download a
      huge file (or a file from a slow server); libcurl ought to make this
      pretty easy

1. Header cleanup (omfg)

1. Make tics unsigned (there's no reason for a tic value to ever be negative)

1. Update renderer

1. Player names are hardcoded for DeHackEd; the way this should work is:
  - Keep a private array of the default names ("Green", "Indigo", etc.)
  - After initialization, check if the names have been modified, by a DeHackEd
    patch or anything
  - If so, set each player's name accordingly
  - Of course, this only works for the 1st 4 players; after that, fuck it

<!-- vi: set et ts=4 sw=4 tw=79: -->

