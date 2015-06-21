#ifndef UDS_H__
#define UDS_H__

#define CLIENT_SOCKET_NAME "client.sock"
#define SERVER_SOCKET_NAME "server.sock"

struct uds_s;
struct uds_peer_s;

typedef void (*uds_handle_data)(struct uds_s *uds, struct uds_peer_s *peer);
typedef void (*uds_handle_exception)(struct uds_s *uds);

/* CG: TODO: Add error handler */

typedef struct uds_s {
  gchar               *socket_path;
  GSocket             *socket;
  GIOCondition         condition;
  GPtrArray           *peers;
  GHashTable          *peer_directory;
  GString             *input;
  GString             *exception;
  uds_handle_data      handle_data;
  uds_handle_exception handle_exception;
  gboolean             waiting_for_connection;
  gboolean             has_pending_data;
  gboolean             service_manually;
} uds_t;

typedef struct uds_peer_s {
  uds_t          *uds;
  GSocketAddress *address;
  GString        *output;
  gboolean        disconnected;
} uds_peer_t;

gchar*      socket_address_to_string(GSocketAddress *addr);
void        uds_init(uds_t *uds, const gchar *socket_path,
                                 uds_handle_data handle_data,
                                 uds_handle_exception handle_exception,
                                 gboolean service_manually);
void        uds_free(uds_t *uds);
void        uds_connect(uds_t *uds, const gchar *socket_path);
void        uds_service(uds_t *uds);
uds_peer_t* uds_get_peer(uds_t *uds, const gchar *peer_address);
GIOChannel* uds_get_iochannel(uds_t *uds);
void        uds_broadcast(uds_t *uds, const gchar *data);
gboolean    uds_sendto(uds_t *uds, const gchar *peer_address,
                                   const gchar *data);
void        uds_peer_sendto(uds_peer_t *peer, const gchar *data);

#endif

/* vi: set et ts=2 sw=2: */

