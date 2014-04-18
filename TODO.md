# To Do

## Singleplayer Client:
  - Hey look, 28.5ms has elapsed!
    - Better build a command!
    - Better run a tic!

## Command-Sync Server:
  - Hey look, I got a sync message!
    - Better relay it to my clients and sync them up!

## Command-Sync Client:
  - Hey look, 28.5 ms has elapsed!
    - Better build a command!
      - Better sync up the server!
    - Hey look, I've got commands for all other players for the next gametic!
      - Better run a tic!
      - (repeat)
  - Hey look, I got a sync message!
    - Better sync up the server!

## Delta-Sync Server:
  - Hey look, 28.5 ms has elapsed!
    - Better build a (blank) command!
    - Better run a tic!
      - Hey this player has X commands!
        - Better run X commands for this player!
      - Better sync up my clients!
  - Hey look, I got a sync message!
    - Better add new commands!
    - Better remove old states from my state buffer!
    - Better sync up my clients!

## Delta-Sync Client:
  - Hey look, 28.5 ms has elapsed!
    - Better build a command!
      - Better sync up the server!
    - Better run a tic!
      - Hey this player has X commands!
        - Better run X commands for this player!
  - Hey look, I got a sync message!
    - Better remove old commands from my player's command buffer!
    - Better sync up the server!

    /*
     * CG: This deserves explanation.  When a DELTACLIENT loads a new state,
     *     it will overwrite its local commands, causing the most recent
     *     commands (that either have not been sent or will not reach the
     *     server due to packet loss) to be lost forever.  Therefore, when
     *     connected, a DELTACLIENT will not load commands for its local player.
     *     Hacky, I know.  The alternatives are moving player command buffers to
     *     the network message itself (but that skips delta compression), saving
     *     and restoring the local commands (unnecessary copies) or keeping the
     *     local player's commands and creating an abstration layer around them.
     */
    if (DELTACLIENT && N_GetPeer(0) != NULL)

So the question is whether the separate local command buffer is worse than a
special case in `P_UnArchivePlayers`.  Do the local command buffer; I like the
`P_GetPlayerCommands(unsigned short playernum)` abstraction, and I'm unsure of
the effects of putting a special case in `P_UnArchivePlayers`.

When DELTACLIENT gets sync from DELTASERVER, it also gets the consoleplayer's
command buffer filled with the commands DELTASERVER has from it, as well as the
gametic of the current state.  So do the following after loading state:

    cbuf_t *player_commands = &players[consoleplayer].commands;
    cbuf_t *local_commands = P_GetPlayerCommands(consoleplayer);
    netpeer_t *server = N_GetPeer(0);

    if (server == NULL)
      return;

    if (M_CBufGetObjectCount(player_commands) == 0) {
      server->command_tic = gametic;
    }
    else {
      CBUF_FOR_EACH(player_commands, entry) {
        netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

        server->command_tic = MAX(server->command_tic, ncmd->tic);
      }
    }

    CBUF_FOR_EACH(local_commands, entry) {
      netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

      if (ncmd->tic < server->command_tic) {
        M_CBufRemove(local_commands, entry.index);
        entry.index--;
      }
      else {
        M_CBufAppend(player_commands, ncmd);
      }
    }

1's Cons: Copies a bunch of data every 28.5ms
2's Cons: Sends a bunch of data in the sync message, bloating it
3's Cons: Hacky

When deltaclients send commands, they're stamped with the gametic they were
made for.  When deltaservers receive the commands, they therefore know what
kind of lag is occurring.  Deltaservers don't need to accept commands for
previous gametics, nor do they need to run commands built for future gametics.
They must, however, run all commands inbetween to avoid "buffering" client
commands and causing artificial lag.

Delta sync updates contain the current gametic, the tic of the command most
recently received by the server (`command_tic`), and the buffered commands for
all players.  When a client receives a sync update, it deletes from its
player's command buffer all commands whose tic is &lt;= `command_tic`, and then
loads the state delta.

1. Players are running (and therefore flushing their command buffers) commands
   before sending them to the server, causing nothing to be sent.  To fix this:
   - In `P_PlayerThink`: if player is consoleplayer, seek to the command for
     the current gametic, otherwise...

---

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

