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

/* [CG] [FIXME] Supports only HTTP (not HTTPS, or anything else) for now */

#include "z_zone.h"

#include <curl/curl.h>

#include "i_system.h"
#include "n_main.h"
#include "n_http.h"

#define DEBUG_HTTP 0

#define CLIENT_USER_AGENT PACKAGE_NAME "-client/v" PACKAGE_VERSION

#define SERVER_USER_AGENT PACKAGE_NAME "-server/v" PACKAGE_VERSION

struct http_req_s {
  size_t             index;
  http_method_e      method;
  char              *url;
  struct curl_slist *headers;
  CURL              *curl_handle;
  buf_t              outgoing_data;
  buf_t              incoming_data;
  long               status_code;
  CURLcode           curl_errno;
  http_handler_t    *callback;
  void              *callback_data;
};

static GArray *http_reqs = NULL;
static CURLM  *multi_handle = NULL;

static size_t send_http_data(void *p, size_t size, size_t nmemb, void *s) {
  http_req_t *req = (http_req_t *)s;
  size_t byte_count = M_BufferGetDataRemaining(&req->outgoing_data);

  if (byte_count > (size * nmemb)) {
    byte_count = (size * nmemb);
  }

  memcpy(p, M_BufferGetDataAtCursor(&req->outgoing_data), byte_count);

  (void)M_BufferSeekForward(&req->outgoing_data, byte_count);

  return byte_count;
}

static size_t receive_http_data(void *p, size_t size, size_t nmemb, void *d) {
  http_req_t *req = (http_req_t *)d;
  size_t byte_count = size * nmemb;

#if DEBUG_HTTP
  printf("Receiving HTTP data: %zu/%zu.\n", size, nmemb);
#endif

  M_BufferEnsureCapacity(&req->incoming_data, byte_count);
  M_BufferWrite(&req->incoming_data, p, byte_count);

  return byte_count;
}

static void http_req_clear(http_req_t *req) {
  req->index = 0;
  req->method = HTTP_METHOD_NONE;

  if (req->url) {
    req->url = NULL;
  }

  if (req->headers) {
    curl_slist_free_all(req->headers);
    req->headers = NULL;
  }

  if (req->curl_handle) {
    curl_easy_cleanup(req->curl_handle);
    req->curl_handle = NULL;
  }

  M_BufferClear(&req->outgoing_data);
  M_BufferClear(&req->incoming_data);

  req->status_code = 0;
  req->curl_errno = CURLE_OK;
  req->callback = NULL;
  req->callback_data = NULL;
}

static void http_reqs_clear_func(gpointer data) {
  http_req_clear((http_req_t *)data);
}

char* N_HTTPEscape(const char *s) {
  return curl_easy_escape(NULL, s, 0);
}

void N_HTTPClearReqs(void) {
  g_array_remove_range(http_reqs, 0, http_reqs->len);
}

http_req_t* N_HTTPReqNew(http_method_e method, char *url) {
  http_req_t *req = NULL;
  CURL *curl_handle = curl_easy_init();

  if (!curl_handle) {
    D_MsgLocalError("Unknown error creating cURL handle\n");
    return NULL;
  }

  g_array_set_size(http_reqs, http_reqs->len + 1);
  req = &g_array_index(http_reqs, http_req_t, http_reqs->len - 1);

  req->index = http_reqs->len - 1;
  req->method = method;
  req->url = strdup(url);
  req->headers = NULL;
  req->curl_handle = curl_handle;
  M_BufferInit(&req->incoming_data);
  M_BufferInit(&req->outgoing_data);
  req->status_code = 0;
  req->curl_errno = 0;
  req->callback = NULL;
  req->callback_data = NULL;

#if DEBUG_HTTP
  curl_easy_setopt(req->curl_handle, CURLOPT_VERBOSE, 1L);
#endif

  curl_easy_setopt(req->curl_handle, CURLOPT_URL, req->url);

  if (SERVER) {
    curl_easy_setopt(req->curl_handle, CURLOPT_USERAGENT, SERVER_USER_AGENT);
  }
  else {
    curl_easy_setopt(req->curl_handle, CURLOPT_USERAGENT, CLIENT_USER_AGENT);
  }

  curl_easy_setopt(req->curl_handle, CURLOPT_WRITEFUNCTION, receive_http_data);
  curl_easy_setopt(req->curl_handle, CURLOPT_WRITEDATA, (void *)req);
  curl_easy_setopt(req->curl_handle, CURLOPT_READFUNCTION, send_http_data);
  curl_easy_setopt(req->curl_handle, CURLOPT_READDATA, (void *)req);
  curl_easy_setopt(req->curl_handle, CURLOPT_PRIVATE, (void *)req);

  switch (method) {
    case HTTP_METHOD_POST:
      curl_easy_setopt(req->curl_handle, CURLOPT_POST, 1L);
      break;
    case HTTP_METHOD_PUT:
      curl_easy_setopt(req->curl_handle, CURLOPT_UPLOAD, 1L);
      break;
    case HTTP_METHOD_DELETE:
      curl_easy_setopt(req->curl_handle, CURLOPT_CUSTOMREQUEST, "DELETE");
      break;
    case HTTP_METHOD_GET:
       break;
    default:
      I_Error("Unsupported HTTP method\n");
  }

  return req;
}

bool N_HTTPReqHasError(http_req_t *req) {
  return req->curl_errno != 0;
}

long N_HTTPReqErrno(http_req_t *req) {
  return req->curl_errno;
}

const char* N_HTTPReqStrerror(http_req_t *req) {
  return curl_easy_strerror(req->curl_errno);
}

void N_HTTPReqSetCallback(http_req_t *req, http_handler_t *callback) {
  req->callback = callback;
}

void N_HTTPReqSetCallbackData(http_req_t *req, void *data) {
  req->callback_data = data;
}

void N_InitHTTP(void) {
  CURLcode curl_errno;

#ifdef _WIN32
    curl_errno = curl_global_init(CURL_GLOBAL_WIN32);
#else
    curl_errno = curl_global_init(CURL_GLOBAL_NOTHING);
#endif

  if (curl_errno != 0) {
    I_Error("N_InitHTTP: %s\n", curl_easy_strerror(curl_errno));
  }

  if (!(multi_handle = curl_multi_init())) {
    I_Error("N_InitHTTP: Could not initialize cURL.\n");
  }

  http_reqs = g_array_new(false, false, sizeof(http_req_t));
  g_array_set_clear_func(http_reqs, http_reqs_clear_func);
}

void N_HTTPServiceRequests(void) {
  int handle_count;
  int req_info_count;
  long curl_timeout;
  CURLMsg *req_info;
  CURLMcode mres;
  int maxfd = -1;
  fd_set fdread;
  fd_set fdwrite;
  fd_set fdexcep;

  FD_ZERO(&fdread);
  FD_ZERO(&fdwrite);
  FD_ZERO(&fdexcep);

  mres = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

  if (mres != CURLM_OK) {
    D_MsgLocalWarn(
      "service_http_requests: Error running curl_multi_fdset:\n\t%d %s.\n",
      mres, curl_easy_strerror((CURLcode)mres)
    );

    return;
  }

  mres = curl_multi_timeout(multi_handle, &curl_timeout);

  if (mres != CURLM_OK) {
    D_MsgLocalWarn(
      "service_http_requests: Error running curl_multi_timeout:\n\t%d %s.\n",
      mres, curl_easy_strerror((CURLcode)mres)
    );

    return;
  }

  /*
   * [CG] CURLM_CALL_MULTI_PERFORM means "call curl_multi_perform() again, so
   *      loop here (shouldn't really be that long) while cURL wants us to.
   */
  do {
    mres = curl_multi_perform(multi_handle, &handle_count);
  } while (mres == CURLM_CALL_MULTI_PERFORM);

  if (mres != CURLM_OK) {
    D_MsgLocalWarn(
      "service_http_requests: Error running curl_multi_perform:\n\t%d %s.\n",
      mres, curl_easy_strerror((CURLcode)mres)
    );

    return;
  }

  while ((req_info = curl_multi_info_read(multi_handle, &req_info_count))) {
    CURLcode res;
    http_req_t *req = NULL;
    size_t req_index = 0;

    /*
     * [CG] CURLMSG_DONE is the only value, so it's not like we're ignoring
     *      things here.  I guess it's open for expansion.
     */
    if (req_info->msg != CURLMSG_DONE) {
      continue;
    }
    
    curl_multi_remove_handle(multi_handle, req->curl_handle);

    res = curl_easy_getinfo(req_info->easy_handle, CURLINFO_PRIVATE, &req);

    if (res != CURLE_OK) {
      D_MsgLocalWarn(
        "service_http_requests: Error looking up request in request "
        "info:\n\t%d %s,\n\n",
        res, curl_easy_strerror(res)
      );

      continue;
    }

    req_index = req->index;

    curl_easy_getinfo(
      req_info->easy_handle, CURLINFO_RESPONSE_CODE, &req->status_code
    );

    for (size_t i = 0; i < 10; i++) {
      if (req->status_code != 0) {
        break;
      }

      I_Sleep(1);

      curl_easy_getinfo(
        req_info->easy_handle, CURLINFO_RESPONSE_CODE, &req->status_code
      );
    }

    if (req->status_code == 0) {
      D_MsgLocalWarn(
        "service_http_request: timed out waiting for response from [%s].\n",
        req->url
      );
    }

    if (req->callback) {
      req->callback(req);
    }

    g_array_remove_index_fast(http_reqs, req->index);
    req = &g_array_index(http_reqs, http_req_t, req_index);
    req->index = req_index;
  }
}

void N_HTTPReqSetAuth(http_req_t *req, const char *auth) {
  curl_easy_setopt(req->curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
  curl_easy_setopt(req->curl_handle, CURLOPT_USERPWD, auth);
}

void N_HTTPReqAddHeader(http_req_t *req, const char *header) {
  req->headers = curl_slist_append(req->headers, header);
}

buf_t* N_HTTPReqGetOutgoingData(http_req_t *req) {
  return &req->outgoing_data;
}

void N_HTTPReqHandleOutgoingDataChanged(http_req_t *req) {
  switch (req->method) {
    case HTTP_METHOD_PUT:
      curl_easy_setopt(
        req->curl_handle,
        CURLOPT_INFILESIZE_LARGE,
        (curl_off_t)M_BufferGetDataRemaining(&req->outgoing_data)
      );
      break;
    case HTTP_METHOD_POST:
      curl_easy_setopt(
        req->curl_handle,
        CURLOPT_POSTFIELDSIZE_LARGE,
        (curl_off_t)M_BufferGetDataRemaining(&req->outgoing_data)
      );
      break;
    default:
      I_Error(
        "N_HTTPReqSetOutgoingData: Cannot set outgoing data if method is "
        "neither PUT nor POST\n"
      );
      break;
  }
}

void N_HTTPReqSend(http_req_t *req) {
  if (req->headers) {
    curl_easy_setopt(req->curl_handle, CURLOPT_HTTPHEADER, req->headers);
  }

  curl_multi_add_handle(multi_handle, req->curl_handle);
}

buf_t* N_HTTPReqGetReceivedData(http_req_t *req) {
  return &req->incoming_data;
}

long N_HTTPReqGetStatus(http_req_t *req) {
  return req->status_code;
}

void* N_HTTPReqGetCallbackData(http_req_t *req) {
  return req->callback_data;
}

/* vi: set et ts=2 sw=2: */
