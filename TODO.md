# To Do

1. Junk old busted netcode, replace with the new hotness

1. Get rid of spurious NetUpdate calls everywhere; they don't help anymore, but
   they do waste bandwidth

1. Fix choppiness between frames; rendering is probably only done at 35Hz if
   interpolation is disabled, which won't work for OpenGL.

1. Have server actually send out updates

1. Figure out how to have the server run the game with zero players
  - Probably just have the server be player 0 for now

1. Compile & Test prototype

---

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

