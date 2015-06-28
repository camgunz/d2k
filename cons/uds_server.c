#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "uds.h"

#define MESSAGE_INTERVAL 1000

#define TR \
"You're it\n"                                                  \
"No, you're it\n"                                              \
"Hey, you're really it\n"                                      \
"You're it\n"                                                  \
"No I mean it, you're it\n"                                    \
"\n"                                                           \
"Say it\n"                                                     \
"Don't spray it\n"                                             \
"Spirit desire (face me)\n"                                    \
"Spirit desire (don't displace me)\n"                          \
"Spirit desire\n"                                              \
"We will fall\n"                                               \
"\n"                                                           \
"Miss me\n"                                                    \
"Don't dismiss me\n"                                           \
"\n"                                                           \
"Spirit desire\n"                                              \
"Spirit desire [x3]\n"                                         \
"We will fall\n"                                               \
"Spirit desire\n"                                              \
"We will fall\n"                                               \
"Spirit desire [x3]\n"                                         \
"We will fall\n"                                               \
"Spirit desire\n"                                              \
"We will fall\n"                                               \
"\n"                                                           \
"\n"                                                           \
"Everybody's talking 'bout the stormy weather\n"               \
"And what's a man do to but work out whether it's true?\n"     \
"Looking for a man with a focus and a temper\n"                \
"Who can open up a map and see between one and two\n"          \
"\n"                                                           \
"Time to get it\n"                                             \
"Before you let it\n"                                          \
"Get to you\n"                                                 \
"\n"                                                           \
"Here he comes now\n"                                          \
"Stick to your guns\n"                                         \
"And let him through\n"                                        \
"\n"                                                           \
"Everybody's coming from the winter vacation\n"                \
"Taking in the sun in a exaltation to you\n"                   \
"You come running in on platform shoes\n"                      \
"With Marshall stacks\n"                                       \
"To at least just give us a clue\n"                            \
"Ah, here it comes\n"                                          \
"I know it's someone I knew\n"                                 \
"\n"                                                           \
"Teenage riot in a public station\n"                           \
"Gonna fight and tear it up in a hypernation for you\n"        \
"\n"                                                           \
"Now I see it\n"                                               \
"I think I'll leave it out of the way\n"                       \
"Now I come near you\n"                                        \
"And it's not clear why you fade away\n"                       \
"\n"                                                           \
"Looking for a ride to your secret location\n"                 \
"Where the kids are setting up a free-speed nation, for you\n" \
"Got a foghorn and a drum and a hammer that's rockin'\n"       \
"And a cord and a pedal and a lock, that'll do me for now\n"   \
"\n"                                                           \
"It better work out\n"                                         \
"I hope it works out my way\n"                                 \
"Cause it's getting kind of quiet in my city's head\n"         \
"Takes a teen age riot to get me out of bed right now\n"       \
"\n"                                                           \
"You better look it\n"                                         \
"We're gonna shake it\n"                                       \
"Up to him\n"                                                  \
"\n"                                                           \
"He acts the hero\n"                                           \
"We paint a zero\n"                                            \
"On his hand\n"                                                \
"\n"                                                           \
"We know it's down\n"                                          \
"We know it's bound too loose\n"                               \
"Everybody's sound is round it\n"                              \
"Everybody wants to be proud to choose\n"                      \
"So who's to take the blame for the stormy weather\n"          \
"You're never gonna stop all the teenage leather and booze\n"  \
"\n"                                                           \
"It's time to go round\n"                                      \
"A one man showdown\n"                                         \
"Teach us how to fail\n"                                       \
"\n"                                                           \
"We're off the streets now\n"                                  \
"And back on the road\n"                                       \
"On the riot trail\n"                                          \

#define GA \
"Four score and seven years ago our fathers brought forth on this "          \
"continent, a new nation, conceived in Liberty, and dedicated to the "       \
"proposition that all men are created equal. "                               \
"\n\n"                                                                       \
"Now we are engaged in a great civil war, testing whether that nation, or "  \
"any nation so conceived and so dedicated, can long endure. We are met on "  \
"a great battle-field of that war. We have come to dedicate a portion of "   \
"that field, as a final resting place for those who here gave their lives "  \
"that that nation might live. It is altogether fitting and proper that we "  \
"should do this."                                                            \
"\n"                                                                         \
"But, in a larger sense, we can not dedicate -- we can not consecrate -- "   \
"we can not hallow -- this ground. The brave men, living and dead, who "     \
"struggled here, have consecrated it, far above our poor power to add or "   \
"detract. The world will little note, nor long remember what we say here, "  \
"but it can never forget what they did here. It is for us the living, "      \
"rather, to be dedicated here to the unfinished work which they who fought " \
"here have thus far so nobly advanced. It is rather for us to be here "      \
"dedicated to the great task remaining before us -- that from these "        \
"honored dead we take increased devotion to that cause for which they gave " \
"the last full measure of devotion -- that we here highly resolve that "     \
"these dead shall not have died in vain -- that this nation, under God, "    \
"shall have a new birth of freedom -- and that government of the people, "   \
"by the people, for the people, shall not perish from the earth."

static uds_t uds;

static void handle_data(uds_t *uds, uds_peer_t *peer) {
  // g_print("Got data [%s]\n", uds->input->str);
  uds_peer_sendto(peer, TR);
}

static void handle_exception(uds_t *uds) {
  g_printerr("Got exception [%s]\n", uds->exception->str);
}

static gboolean task_broadcast_data(gpointer user_data) {
  static guint counter = 1;
  static GString *s = NULL;
  
  uds_t *uds = (uds_t *)user_data;

  if (!s)
    s = g_string_new("");

  g_string_printf(s, "Server message %u", counter);
  g_print("%s\n", s->str);
  uds_broadcast(uds, s->str);

  counter++;

  return TRUE;
}

static void cleanup(void) {
  uds_free(&uds);
}

int main(int argc, char **argv) {
  GMainContext *mc;
  GMainLoop    *loop;

  memset(&uds, 0, sizeof(uds_t));

  uds_init(
    &uds,
    SERVER_SOCKET_NAME,
    handle_data,
    handle_exception,
    FALSE
  );

  atexit(cleanup);

  mc = g_main_context_default();
  loop = g_main_loop_new(mc, FALSE);
  // g_timeout_add(MESSAGE_INTERVAL, task_broadcast_data, &uds);

  g_main_loop_run(loop);

  return EXIT_SUCCESS;
}

/* vi: set et ts=2 sw=2: */

