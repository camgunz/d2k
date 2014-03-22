# To Do

  - Modify save loading:
    - `savegamebuffer` & `savegamesize` should be rolled into a `buf_t`
    - Modify savegame stuff so that it uses `buf_t` functions instead of `*p++`
      stuff.
    - All the `P_Archive*` functions ought to take a `buf_t *` instead of using
      a global.
  - Consider using a MessagePack map for all the player change stuff.
    - Extensible
    - Cuts down on the total # of messages
  - Fill out the handler functions
    - Probably involves creating some new methods/enums/etc.
  - Finally revamp `d_client.c` and `d_server.c`.
  - Add command-line arguments for new netcode.
    - `-serve <host>:<port>`
      - Also enables headless mode.
    - `-connect <host>:<port>`
    - host & port will live for now until HTTP/JSON comes online; let's get a
      prototype out first.
  - Add unlagged
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

