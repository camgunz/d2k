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


#ifndef N_HTTP_H__
#define N_HTTP_H__

typedef enum {
  HTTP_METHOD_NONE,
  HTTP_METHOD_GET,
  HTTP_METHOD_POST,
  HTTP_METHOD_PUT,
  HTTP_METHOD_DELETE
} http_method_e;

struct http_req_s;
typedef struct http_req_s http_req_t;

typedef void (http_handler_t)(http_req_t *req);

char*       N_HTTPEscape(const char *s);
void        N_HTTPClearReqs(void);
http_req_t* N_HTTPReqNew(http_method_e method, char *url);
long        N_HTTPReqErrno(http_req_t *req);
const char* N_HTTPReqStrerror(http_req_t *req);
void        N_HTTPReqSetCallback(http_req_t *req, http_handler_t *callback);
void        N_HTTPReqSetCallbackData(http_req_t *req, void *data);
void        N_HTTPReqSetAuth(http_req_t *req, const char *auth);
void        N_HTTPReqAddHeader(http_req_t *req, const char *header);
buf_t*      N_HTTPReqGetOutgoingData(http_req_t *req);
void        N_HTTPReqHandleOutgoingDataChanged(http_req_t *req);
void        N_HTTPReqSend(http_req_t *req);
buf_t*      N_HTTPReqGetReceivedData(http_req_t *req);
long        N_HTTPReqGetStatus(http_req_t *req);
void*       N_HTTPReqGetCallbackData(http_req_t *req);

#endif

/* vi: set et ts=2 sw=2: */
