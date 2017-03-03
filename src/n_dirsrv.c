/*****************************************************************************/
/* D2K: A Doom Source Port for the 21st Century                              */
/*                                                                           */
/* Copyright (C) 2014: See COPYRIGHT file                                    */
/*                                                                           */
/* This file is part of D2K.                                                 */
/*                                                                           */
/* D2K is free software: you can redistribute it and/or modify it under the  */
/* terms of the GNU General Public License as published by the Free Software */
/* Foundation, either version 2 of the License, or (at your option) any      */
/* later version.                                                            */
/*                                                                           */
/* D2K is distributed in the hope that it will be useful, but WITHOUT ANY    */
/* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS */
/* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more    */
/* details.                                                                  */
/*                                                                           */
/* You should have received a copy of the GNU General Public License along   */
/* with D2K.  If not, see <http://www.gnu.org/licenses/>.                    */
/*                                                                           */
/*****************************************************************************/

/*
 * [CG] [TODO] Add support for protocols other than HTTP(S) and formats other
 *             than JSON
 */

#include "z_zone.h"

#include <jansson.h>

#include "doomdef.h"
#include "i_system.h"
#include "n_http.h"
#include "n_main.h"
#include "n_dirsrv.h"
#include "sv_main.h"

extern char gamemapname[9];

#define DEBUG_DIRECTORY_SERVER 0
#define DIRECTORY_SERVER_TIMEOUT_LIMIT 5

typedef enum {
  DS_STATE_DISABLED,
  DS_STATE_ENABLED,
  DS_STATE_LISTING,
  DS_STATE_LISTED,
  DS_STATE_DELISTING,
  DS_STATE_DELISTED,
  DS_STATE_UPDATING,
  DS_STATE_UPDATED
} dir_srv_state_e;

struct dir_srv_s {
  dir_srv_state_e state;
  size_t          index;
  const char     *address;
  unsigned int    port;
  bool            https;
  const char     *server_group;
  const char     *server_name;
  char           *url;
  char           *server_url;
  char           *auth;
  time_t          last_update;
  size_t          timeouts;
};

struct dir_srv_grp_s {
  const char *name;
};

struct dir_srv_srv_s {
  const char *name;
};

struct dir_srv_srv_info_s {
  size_t maxplayers;
};

static GArray *directory_servers = NULL;

static void dir_srv_update_urls(dir_srv_t *ds) {
  if (ds->url) {
    free(ds->url);
    ds->url = NULL;
  }

  if (ds->https) {
    if ((ds->port == 0) || (ds->port == 443)) {
      ds->url = g_strdup_printf("https://%s", ds->address);
    }
    else {
      ds->url = g_strdup_printf("https://%s:%u", ds->address, ds->port);
    }
  }
  else if ((ds->port == 0) || (ds->port == 80)) {
    ds->url = g_strdup_printf("http://%s", ds->address);
  }
  else {
    ds->url = g_strdup_printf("http://%s:%u", ds->address, ds->port);
  }

  if (SERVER) {
    char *escaped_server_group = N_HTTPEscape(ds->server_group);
    char *escaped_server_name = N_HTTPEscape(ds->server_name);

    if (ds->server_url) {
      free(ds->server_url);
      ds->server_url = NULL;
    }

    ds->server_url = g_strdup_printf("%s/servers/%s/%s",
      ds->url,
      escaped_server_group,
      escaped_server_name
    );

    free(escaped_server_group);
    free(escaped_server_name);
  }
}

static void dir_srv_clear(dir_srv_t *ds) {
  ds->state = DS_STATE_DISABLED;
  ds->index = 0;
  ds->address = NULL;
  ds->port = 0;
  ds->https = false;
  ds->server_group = NULL;
  ds->server_name = NULL;
  if (ds->url) {
    free(ds->url);
    ds->url = NULL;
  }
  if (ds->server_url) {
    free(ds->server_url);
    ds->server_url = NULL;
  }
  if (ds->auth) {
    free(ds->auth);
    ds->auth = NULL;
  }
  ds->last_update = 0;
  ds->timeouts = 0;
}

static void directory_servers_clear_func(gpointer data) {
  dir_srv_clear((dir_srv_t *)data);
}

static void check_request_status(http_req_t *req, bool *disabled,
                                                  bool *updated) {
  dir_srv_t *ds = N_HTTPReqGetCallbackData(req);
  long status_code = N_HTTPReqGetStatus(req);

  switch (status_code) {
    case 0:
    case 408:
      /*
       * [CG] Maybe put a limit on timeouts instead of removing on the 1st
       *      one.
       */
      D_Msg(MSG_ERROR, "Directory server [%s] timed out\n", ds->url);
      ds->timeouts++;
      if (ds->timeouts >= DIRECTORY_SERVER_TIMEOUT_LIMIT) {
        D_Msg(MSG_ERROR,
          "Directory server [%s] reached timeout limit, delisting\n", ds->url
        );
        ds->state = DS_STATE_DISABLED;
        *disabled = true;
      }
      else {
        *disabled = false;
      }

      *updated = false;
      break;
    case 200:
    case 201:
    case 204:
      ds->last_update = time(NULL);
      ds->timeouts = 0;
      *disabled = false;
      *updated = true;
      break;
    case 304:
      if (SERVER) {
        D_Msg(MSG_ERROR, "Got unexpected 304 (Not Modified) from [%s]\n",
          ds->url
        );
      }
      ds->last_update = time(NULL);
      ds->timeouts = 0;
      *disabled = false;
      *updated = false;
      break;
    case 401:
      D_Msg(MSG_ERROR, "Authentication to [%s] failed, delisting\n", ds->url);
      ds->timeouts = 0;
      ds->state = DS_STATE_DISABLED;
      *disabled = true;
      *updated = false;
      break;
    default:
      D_Msg(MSG_ERROR, "Unexpected HTTP status code '%ld' from [%s]\n",
        status_code,
        ds->url
      );
      /* [CG] [TODO] Delist after a certain threshold here */
      ds->timeouts = 0;
      *disabled = false;
      *updated = false;
      break;
  }
}

static void dir_srv_set_updated(http_req_t *req) {
  bool disabled;
  bool updated;

  check_request_status(req, &disabled, &updated);

  if (disabled || !updated) {
    return;
  }

  dir_srv_t *ds = (dir_srv_t *)N_HTTPReqGetCallbackData(req);

  ds->state = DS_STATE_UPDATED;
}

static void dir_srv_set_listed(http_req_t *req) {
  bool disabled;
  bool updated;

  check_request_status(req, &disabled, &updated);

  if (disabled || !updated) {
    return;
  }

  dir_srv_t *ds = (dir_srv_t *)N_HTTPReqGetCallbackData(req);

  ds->state = DS_STATE_LISTED;
}

static void dir_srv_update_listings(http_req_t *req) {
  bool disabled;
  bool updated;

  check_request_status(req, &disabled, &updated);

  if (disabled) {
    return;
  }

  dir_srv_t *ds = N_HTTPReqGetCallbackData(req);

  if (updated) {
    buf_t *listings_buf = N_HTTPReqGetReceivedData(req);

    (void)listings_buf;

    /* [CG] [TODO] Update local listings from JSON in listings_buf */
  }

  ds->state = DS_STATE_UPDATED;
}

static void dir_srv_set_delisted(http_req_t *req) {
  bool disabled;
  bool updated;

  check_request_status(req, &disabled, &updated);

  dir_srv_t *ds = N_HTTPReqGetCallbackData(req);

  if (disabled || !updated) {
    D_Msg(MSG_INFO, "Error delisting from [%s].\n", ds->url);
  }
  else {
    D_Msg(MSG_INFO, "Delisted from [%s].\n", ds->url);
  }

  ds->state = DS_STATE_DELISTED;
}

static void dir_srv_list(dir_srv_t *ds) {
  char *jstr = NULL;
  json_t *json = json_object();
  json_t *server = json_object();

  json_object_set_new(server, "group", json_string(ds->server_group));
  json_object_set_new(server, "name", json_string(ds->server_group));
  json_object_set_new(server, "host", json_string(SV_GetHost()));
  json_object_set_new(server, "port", json_integer(SV_GetPort()));
  json_object_set_new(json, "server", server);

  jstr = json_dumps(json, JSON_COMPACT);

  json_decref(json);

  if (!jstr) {
    D_Msg(MSG_ERROR, "Error encoding JSON when advertising to [%s]\n",
      ds->url
    );
    return;
  }

  http_req_t *req = N_HTTPReqNew(HTTP_METHOD_POST, ds->server_url);
  buf_t *outgoing_data = N_HTTPReqGetOutgoingData(req);

  M_BufferWriteString(outgoing_data, jstr, strlen(jstr));
  N_HTTPReqHandleOutgoingDataChanged(req);
  N_HTTPReqSetCallback(req, dir_srv_set_listed);
  N_HTTPReqSetCallbackData(req, (void *)ds);

  ds->state = DS_STATE_LISTING;
  N_HTTPReqSend(req);
}

static void dir_srv_delist(dir_srv_t *ds) {
  http_req_t *req = N_HTTPReqNew(HTTP_METHOD_DELETE, ds->server_url);
  N_HTTPReqSetCallback(req, dir_srv_set_delisted);
  N_HTTPReqSetCallbackData(req, (void *)ds);

  ds->state = DS_STATE_DELISTING;
  N_HTTPReqSend(req);
}

static void dir_srv_delist_all(void) {
  size_t listed_server_count = 0;

  D_Msg(MSG_INFO, "Delisting from all directory servers\n");

  N_HTTPClearReqs();

  for (size_t i = 0; i < directory_servers->len; i++) {
    dir_srv_t *ds = &g_array_index(directory_servers, dir_srv_t, i);

    if (ds->state == DS_STATE_DISABLED) {
      continue;
    }

    dir_srv_delist(ds);
  }

  do {
    I_Sleep(1);
    listed_server_count = 0;

    for (size_t i = 0; i < directory_servers->len; i++) {
      dir_srv_t *ds = &g_array_index(directory_servers, dir_srv_t, i);

      if (ds->state == DS_STATE_DISABLED) {
        continue;
      }

      if (ds->state != DS_STATE_DELISTED) {
        listed_server_count++;
      }
    }
  } while (listed_server_count);
}

static void dir_srv_send_update(dir_srv_t *ds) {
  const char *jstr = "{\"greeting\": \"Hey there!\"";
  http_req_t *req = N_HTTPReqNew(HTTP_METHOD_PUT, ds->server_url);
  buf_t *outgoing_data = N_HTTPReqGetOutgoingData(req);

  M_BufferWriteString(outgoing_data, jstr, strlen(jstr));
  N_HTTPReqHandleOutgoingDataChanged(req);
  N_HTTPReqSetCallback(req, dir_srv_set_updated);
  N_HTTPReqSetCallbackData(req, (void *)ds);

  ds->state = DS_STATE_UPDATING;
  N_HTTPReqSend(req);
}

static void dir_srv_request_latest(dir_srv_t *ds) {
  http_req_t *req = N_HTTPReqNew(HTTP_METHOD_PUT, ds->server_url);

  N_HTTPReqSetCallback(req, dir_srv_update_listings);
  N_HTTPReqSetCallbackData(req, (void *)ds);

  ds->state = DS_STATE_UPDATING;
  N_HTTPReqSend(req);
}

void N_DirSrvsInit(void) {
  directory_servers = g_array_new(false, false, sizeof(dir_srv_t));
  g_array_set_clear_func(directory_servers, directory_servers_clear_func);

  if (SERVER) {
    atexit(dir_srv_delist_all);
  }
  else {
    atexit(N_HTTPClearReqs);
  }
}

void N_DirSrvsService(void) {
  time_t now = time(NULL);

  for (size_t i = 0; i < directory_servers->len; i++) {
    dir_srv_t *ds = &g_array_index(directory_servers, dir_srv_t, i);
    double last_update = difftime(now, ds->last_update);

    if (SERVER) {
      switch (ds->state) {
        case DS_STATE_DISABLED:
          break;
        case DS_STATE_ENABLED:
          dir_srv_list(ds);
          break;
        case DS_STATE_UPDATED:
          if (last_update >= (2.0 + ((double)ds->index))) {
            dir_srv_send_update(ds);
          }
          break;
        default:
          break;
      }
    }
    else if (CLIENT) {
      switch (ds->state) {
        case DS_STATE_DISABLED:
          break;
        case DS_STATE_ENABLED:
        case DS_STATE_UPDATED:
          if (last_update >= (2.0 + ((double)ds->index))) {
            dir_srv_request_latest(ds);
          }
          break;
        default:
          break;
      }
    }
  }
}

void N_DirSrvAdd(const char *address, unsigned short port, bool https) {
  dir_srv_t *ds = N_DirSrvGet(address, port);

  if (ds) {
    D_Msg(MSG_WARN, "Duplicate directory server [%s:%u]\n", address, port);
  }
  else {
    g_array_set_size(directory_servers, directory_servers->len + 1);
    ds = &g_array_index(
      directory_servers, dir_srv_t, directory_servers->len - 1
    );
  }

  dir_srv_clear(ds);

  ds->state = DS_STATE_ENABLED;
  ds->index = directory_servers->len - 1;
  ds->address = address;
  ds->port = port;
  ds->https = https;

  dir_srv_update_urls(ds);
}

dir_srv_t* N_DirSrvGet(const char *address, unsigned short port) {
  for (size_t i = 0; i < directory_servers->len; i++) {
    dir_srv_t *ds = &g_array_index(directory_servers, dir_srv_t, i);

    if ((strcmp(ds->address, address) == 0) && (ds->port == port)) {
      return ds;
    }
  }

  return NULL;
}

void N_DirSrvGetGroups(dir_srv_t *ds, GPtrArray *dir_srv_grps) {
}

void N_DirSrvGroupGetServers(dir_srv_grp_t *dsg, GPtrArray *dir_srv_srvs) {
}

void N_GetDirSrvs(GPtrArray *dir_srvs) {
}

dir_srv_srv_info_t* N_DirSrvServerGetInfo(dir_srv_srv_t *dss) {
  return NULL;
}

void SV_DirSrvAdd(const char *address, unsigned short port,
                                       bool https,
                                       const char *username,
                                       const char *password_hash,
                                       const char *server_name,
                                       const char *server_group) {
  dir_srv_t *ds = N_DirSrvGet(address, port);

  if (ds) {
    D_Msg(MSG_WARN, "Duplicate directory server [%s:%u]\n", address, port);
  }
  else {
    g_array_set_size(directory_servers, directory_servers->len + 1);
    ds = &g_array_index(
      directory_servers, dir_srv_t, directory_servers->len - 1
    );
  }

  dir_srv_clear(ds);

  ds->state = DS_STATE_ENABLED;
  ds->index = directory_servers->len - 1;
  ds->address = address;
  ds->port = port;
  ds->https = https;
  ds->server_group = server_group;
  ds->server_name = server_name;
  ds->last_update = 0;

  SV_DirSrvSetAuth(ds, username, password_hash);
  dir_srv_update_urls(ds);
}

void SV_DirSrvSetAuth(dir_srv_t *ds, const char *username,
                                     const char *password_hash) {
  if (ds->auth) {
    free(ds->auth);
    ds->auth = NULL;
  }

  ds->auth = g_strdup_printf("%s:%s", username, password_hash);
}

void N_DirSrvSetGroup(dir_srv_t *ds, const char *server_group) {
  ds->server_group = server_group;
  dir_srv_update_urls(ds);
}

void SV_DirSrvsSetServerName(const char *name) {
  for (size_t i = 0; i < directory_servers->len; i++) {
    dir_srv_t *ds = &g_array_index(directory_servers, dir_srv_t, i);

    ds->server_name = name;
    dir_srv_update_urls(ds);
  }
}

#if 0
char* SV_GetStateJSON(void) {
  int i, j, connected_clients;

  Json::Value server_json;
  Json::FastWriter writer;

  connected_clients = 0;
  for (i = 1; i < MAX_CLIENTS; i++) {
    if (playeringame[i]) {
      connected_clients++;
    }
  }

  server_json["connected_clients"] = connected_clients;
  server_json["map"] = gamemapname;

  if (CS_TEAMS_ENABLED) {
    for (i = team_color_none; i < team_color_max; i++) {
      server_json["teams"][i]["color"] = team_color_names[i];
      server_json["teams"][i]["score"] = team_scores[i];
      for (j = 1; j < MAXPLAYERS; j++) {
        client_t *client = &clients[j];
        player_t *player = &players[j];

        if (!playeringame[j] || client->team != i) {
          continue;
        }

        server_json["teams"][i]["players"][j - 1]["name"] =
          player->name;
        server_json["teams"][i]["players"][j - 1]["lag"] =
          client->stats.transit_lag;
        server_json["teams"][i]["players"][j - 1]["packet_loss"] =
          client->stats.packet_loss;
        server_json["teams"][i]["players"][j - 1]["score"] =
          client->stats.score;
        server_json["teams"][i]["players"][j - 1]["time"] =
          (gametic - client->stats.join_tic) / TICRATE;
        server_json["teams"][i]["players"][j - 1]["playing"] =
          !client->spectating;
      }
    }
  }
  else {
    for (i = 1; i < MAXPLAYERS; i++) {
      client_t *client = &clients[i];
      player_t *player = &players[i];

      if (!playeringame[i]) {
        continue;
      }

      server_json["players"][i - 1]["name"] = player->name;
      server_json["players"][i - 1]["lag"] = client->stats.transit_lag;
      server_json["players"][i - 1]["packet_loss"] =
        client->stats.packet_loss;
      server_json["players"][i - 1]["score"] = client->stats.score;
      server_json["players"][i - 1]["time"] =
        (gametic - client->stats.join_tic) / TICRATE;
      server_json["players"][i - 1]["playing"] = !client->spectating;
    }
  }

  return estrdup(writer.write(server_json).c_str());
}

for (i = 0; i < sv_master_server_count; i++) {
  master = master_servers + i;
  request = SV_GetMasterRequest(master, HTTP_METHOD_PUT);
  json = cs_server_config;
  json["server"]["group"] = master->group;
  json["server"]["name"] = master->name;
  json_string = writer.write(json);
  SV_SetMasterRequestData(request, json_string.c_str());
  N_SendMasterRequest(request);

  // printf("Attempting to advertise to %s.\n", request->url);
  // printf("Sending JSON:\n%s\n", json_string.c_str());
  // printf("Received data:\n%s\n", request->received_data);

  if (request->curl_errno != CURLE_OK) {
    I_Error(
      "Curl error during during advertising:\n%ld %s",
      (long int)request->curl_errno,
      curl_easy_strerror(request->curl_errno)
    );
  }

  if (request->status_code == 201) {
    printf("Advertising on master [%s].\n", master->address);
    advertised = true;
  }
  else if (request->status_code == 301) {
    I_Error(
      "Server '%s' is already advertised on the master [%s].\n",
      master->name,
      master->address
    );
  }
  else if (request->status_code == 401) {
    I_Error(
      "Authenticating on master [%s] failed.\n", master->address
    );
  }
  else {
    I_Error(
      "Received unexpected HTTP status code '%ld' when advertising on "
      "master '%s'.\n",
      (long int)request->status_code,
      master->address
    );
  }
  SV_FreeMasterRequest(request);
}
#endif

/* vi: set et ts=2 sw=2: */
