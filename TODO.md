# To Do

  - Fill out the handler functions
    - Probably involves creating some new methods/enums/etc.
    - Player messages should have a recipient array; this is both extensible
      and avoids sending duplicate message content in the case of multiple
      recipients
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

