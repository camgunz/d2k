#ifndef UDS_H__
#define UDS_H__

#define CLIENT_SOCKET_NAME "client.sock"
#define SERVER_SOCKET_NAME "server.sock"
#define UDS_CLIENT_ANNOUNCE "HAIL SATAN"

struct uds_s;
struct uds_peer_s;

typedef void (*uds_handle_new_data)(struct uds_s *uds, struct uds_peer_s *peer);

typedef struct uds_s {
  gchar               *socket_path;
  GSocket             *socket;
  GHashTable          *peers;
  GString             *input;
  GString             *exception;
  uds_handle_new_data  handle_new_data;
  gboolean             waiting_for_connection;
} uds_t;

typedef struct uds_peer_s {
  GSocketAddress *address;
  GString        *output;
  gboolean        disconnected;
} uds_peer_t;

gchar*      socket_address_to_string(GSocketAddress *addr);
void        uds_init(uds_t *uds, const gchar *socket_path,
                                 uds_handle_new_data handle_new_data);
void        uds_free(uds_t *uds);
void        uds_peer_free(uds_peer_t *peer);
void        uds_connect(uds_t *uds, const gchar *socket_path);
void        uds_service(uds_t *uds);
uds_peer_t* uds_get_peer(uds_t *uds, const gchar *peer_address);
void        uds_sendto(uds_t *uds, uds_peer_t *peer, const gchar *data);
void        uds_broadcast(uds_t *uds, const gchar *data);

#endif

/* vi: set et ts=2 sw=2: */

