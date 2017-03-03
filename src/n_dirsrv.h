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

#ifndef N_DIRSRV_H__
#define N_DIRSRV_H__

struct dir_srv_s;
typedef struct dir_srv_s dir_srv_t;

struct dir_srv_grp_s;
typedef struct dir_srv_grp_s dir_srv_grp_t;

struct dir_srv_srv_s;
typedef struct dir_srv_srv_s dir_srv_srv_t;

struct dir_srv_srv_info_s;
typedef struct dir_srv_srv_info_s dir_srv_srv_info_t;

void N_DirSrvsInit(void);
void N_DirSrvsService(void);

void N_DirSrvAdd(const char *address, unsigned short port, bool https);
dir_srv_t* N_DirSrvGet(const char *address, unsigned short port);
void N_DirSrvGetGroups(dir_srv_t *ds, GPtrArray *dir_srv_grps);
void N_DirSrvGroupGetServers(dir_srv_grp_t *dsg, GPtrArray *dir_srv_srvs);

void N_GetDirSrvs(GPtrArray *dir_srvs);

/*
 * [CG] Probably this will be replaced by specific N_DirSrvServerGet* functions
 *      that essentially look things up in the received JSON
 */
dir_srv_srv_info_t* N_DirSrvServerGetInfo(dir_srv_srv_t *dss);

void SV_DirSrvAdd(const char *address, unsigned short port,
                                       bool https,
                                       const char *username,
                                       const char *password_hash,
                                       const char *server_name,
                                       const char *server_group);
void SV_DirSrvSetAuth(dir_srv_t *ds, const char *username,
                                    const char *password_hash);
void SV_DirSrvSetGroup(dir_srv_t *ds, const char *group);
void SV_DirSrvsSetServerName(const char *name);
/*
 * [CG] The JSON information from the directory server has only two purposes:
 *
 *   - Provides connection information for the client
 *     - address:port
 *     - required WADs
 *   - Provides enough information for the client to implement its own
 *     directory server client in scripting
 *     - Probably just return a copy of the JSON string to Lua in this case
 */

#endif

/* vi: set et ts=2 sw=2: */
