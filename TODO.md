# To Do

  1. Add old messages to `n_proto.c`
    * Add pack/unpack functions.
    * Create an `old_netcode_dispatch` function and a `new_netcode_dispatch`
      function for the two `switch` statements; solves the problem with
      overloaded network message values.
  2. Fill out the handler functions
    * Probably involves creating some new methods/enums/etc.
  3. Finally revamp `d_client.c` and `d_server.c`.
  4. Add command-line arguments for new netcode.
    * `-serve <host>:<port>`
      * Also enables headless mode.
    * `-connect <host>:<port>`

