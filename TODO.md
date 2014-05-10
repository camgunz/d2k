# To Do

- Get prototype working
  - Commands need an index that isn't gametic; they still need gametic for
    unlagged, but gametic == index is an invalid assumption

---

1. Add unlagged
  - Save attacking player position
  - Save current game state
  - Restore game state that player was viewing during the attack
    - This is contained in the command
  - Restore attacking player position (if possible)
  - Run hit detection & damage calculation
    - For every impacted actor:
      - Save momx/momy/momz values
  - Restore saved state
  - For every impacted actor:
    - Add new momx/momy/momz values to the current momx/momy/momz

2. Setup testing framework

3. Type problems:
  - Playernums are unsigned shorts; fix this everywhere
  - TICs are unsigned ints; fix this everywhere

4. Build the command-sync server
  - Shouldn't run any game code; just relay commands between clients

5. Update `players`:
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

6. Add HTTP & JSON
  - Have client download missing WADs
    - the client should do this between frames in case it needs to download a
      huge file (or a file from a slow server); libcurl ought to make this
      pretty easy
  - Convert configuration file(s) to JSON

7. Header cleanup (omfg)

8. Add enough scripting to add a console

8. Update renderer

<!-- vi: set et ts=4 sw=4 tw=79: -->

