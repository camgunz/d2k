# To Do

Get prototype working

- Wiping really lags the client; is it possible to detect a wipe and not build
  commands or service the network for that time?
  - In fact, "resync" functionality might be useful in other areas as well

---

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

1. Type problems:
  - Playernums are unsigned shorts; fix this everywhere
  - TICs are unsigned ints; fix this everywhere

1. Don't have the command-sync server run any game code; it should strictly
   relay commands between clients.

1. If there are errors while reading a network message, message reading
   completely bails, leaving the message partially read.  I think the best
   solution here is a "table of contents" message prepended to the packet,
   which is just an array of byte indices.  Whenever a packing routine writes
   a message type marker, it first writes to the TOC the current cursor
   position of the buffer it's about to use.  When packet sending time arrives,
   `N_ServiceNetworkTimeout` creates a new `nm_toc` message, which is a msgpack
   array of byte indices, and writes it to the packet before the other buffer.

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

