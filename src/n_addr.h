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


#ifndef N_ADDR_H__
#define N_ADDR_H__

size_t      N_IPToString(uint32_t address, char *buffer);
const char* N_IPToConstString(uint32_t address);
bool        N_IPToInt(const char *address_string, uint32_t *address_int);
size_t      N_GetHostFromAddressString(const char *address, char **host);
bool        N_GetPortFromAddressString(const char *address, uint16_t *port);
size_t      N_ParseAddressString(const char *address, char **host,
                                                      uint16_t *port);

#endif

/* vi: set et ts=2 sw=2: */
