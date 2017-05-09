# D2K Network Sync

The essence of any C/S netcode is synchronization between the server and its
clients.  This document describes the architecture and implementation of D2K's
netcode synchronization.

## Commands

Commands have two indices:

- Index of the command
- TIC for which the command was generated

ex: a client that joins the game at TIC 487 will generate commands starting at
0, so `{0, 487}`, `{1, 488}`, and so on.  Note that it's possible for clients
to lag, so the next command may be `{2, 490}`, skipping `489`.

These are tracked in `netticcmd_t`, as the fields `index` and `tic`
respectively.

## Tracking synchronization

Synchronization means a couple things:

- The game state each client has received
- The commands the server has received for each client
- The commands each client has received for every other client

These are tracked in `netsync_t`, as the fields `tic`, `command_index`, and
`command_indices` respectively.

Note that the client and server interpret these fields differently:

- `netsync_t.tic`
  - **clientside**: Latest game state TIC received from the server
  - **serverside**: Latest game state TIC acknowledged by the client
- `netsync_t.command_index`
  - **clientside**: Latest command index acknowledged by the server
  - **serverside**: Latest command index received from the client
- `netsync_t.command_indices[peer]`
  - **clientside**: Latest command index for the given peer received from the
    server
  - **serverside**: Latest command index acknowledged by the client for the
    given peer

You can see a pattern.  The sender keeps track of its remote peer when sending
updates, and the receiver keeps track of the latest version received.

## API

The API for sync is (hopefully) straightforward:

- `N_PeerGetSyncCommandIndex`
  - **clientside**: Returns the latest command index acknowledged by the server
  - **serverside**: Returns the latest command index received by the client
- `N_PeerSetSyncCommandIndex`
  - **clientside**: Sets the latest command index acknowledged by the server
  - **serverside**: Sets the latest command index received by the client
- `N_PeerUpdateSyncCommandIndex`
  - **clientside**: Updates the latest command index acknowledged by the server
    if it's newer
  - **serverside**: Updates the latest command index received by the client if
    it's newer
- `N_PeerGetSyncCommandIndexForPlayer`
  - **clientside**: Returns the latest command index for the given peer
    received from the server
  - **serverside**: Returns the latest command index acknowledged by the client
    for the given peer
- `N_PeerSetSyncCommandIndexForPlayer`
  - **clientside**: Sets the latest command index for the given peer received
    from the server
  - **serverside**: Sets the latest command index acknowledged by the client
    for the given peer
- `N_PeerUpdateSyncCommandIndexForPlayer`
  - **clientside**: Updates the latest command index for the given peer
    received from the server if it's newer
  - **serverside**: Updates the latest command index acknowledged by the client
    for the given peer if it's newer

## Maintaining synchronization

The goal of synchronization is to reconcile differences between client and
server worlds caused by latency.  The latency between clients and servers is
not constant, so it's possible that while the client generates commands `{0,
487}`, `{1, 488}`, `{2, 489}`, the server may receive all these commands for
the first time at TIC 490 (actually it's more than possible, it's likely).
This means that while the client predicted the outcomes of its commands for
TICs 487, 488, and 489, the earliest the server can run them is TIC 490, and
that may change the outcome.  For example, the generating client may have been
shot and therefore moved on TIC 488, so their prediction of their position will
be incorrect.

However, client commands will stack up if the server only runs a single command
command each TIC, causing artificial lag.  Therefore the server has a simple
algorithm to run multiple commands per TIC depending upon how many commands it
received, and so on.

The implications for sync are that the TIC has to be rewritten by the server,
so when it received `{0, 487}`, `{1, 488}`, and `{2, 489}`, it rewrites them as
`{0, 490}`, `{1, 490}`, and `{2, 491}`.  The server then has to tell the client
it rewrote the commands.

This is necessary to re-synchronize clientside prediction.  Clients need to
know when their commands were run in order to correctly rebuild the world from
the last game state point.  This is implemented as a simple search-and-replace,
because the indices are not rewritten.

<!-- vi: set et ts=2 sw=2 tw=79: -->
