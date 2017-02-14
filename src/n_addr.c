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


#include "z_zone.h"

#include <enet/enet.h>

#include "doomdef.h"
#include "n_main.h"

#define MAX_ADDRESS_LENGTH 500

#define to_uchar(x)         ((unsigned char)((x) & 0xFF))
#define to_ushort(x)        ((unsigned short)((x) & 0xFFFF))
#define string_to_ushort(x) (to_ushort(strtol(x, NULL, 10)))

size_t N_IPToString(uint32_t address, char *buffer) {
  return snprintf(buffer, 16, "%u.%u.%u.%u",
    to_uchar(address >> 24),
    to_uchar(address >> 16),
    to_uchar(address >>  8),
    to_uchar(address      )
  );
}

const char* N_IPToConstString(uint32_t address) {
  static char buf[16];

  N_IPToString(address, &buf[0]);

  return &buf[0];
}

bool N_IPToInt(const char *address_string, uint32_t *address_int) {
  int ip = 0;
  int arg_count;
  unsigned char octets[4];

  arg_count = sscanf(address_string, "%hhu.%hhu.%hhu.%hhu",
    &octets[0],
    &octets[1],
    &octets[2],
    &octets[3]
  );

  if (arg_count != 4) {
    D_Msg(MSG_ERROR, "Malformed IP address %s.\n", address_string);
    return false;
  }

  for (int i = 0; i < 4; i++) {
    ip += octets[i] << (8 * (3 - i));
  }

  return ip;
}

size_t N_GetHostFromAddressString(const char *address, char **host) {
  char *sep = NULL;
  size_t host_length = 0;
  size_t address_length = strlen(address);

  if (address_length > MAX_ADDRESS_LENGTH) {
    return 0;
  }

  sep = strchr(address, ':');

  if (!sep) {
    host_length = address_length;

    if (!*host) {
      *host = strdup(address);
    }
    else {
      strncpy(*host, address, address_length + 1);
    }
  }
  else {
    host_length = sep - address;

    if (!*host) {
      *host = calloc(host_length + 1, sizeof(char));
    }
    else {
      (*host)[host_length] = '\0';
    }

    strncpy(*host, address, host_length);
  }

  return host_length;
}

bool N_GetPortFromAddressString(const char *address, uint16_t *port) {
  char *p = NULL;

  *port = 0;

  if (strlen(address) > MAX_ADDRESS_LENGTH) {
    return false;
  }

  if ((p = strchr(address, ':')) && strlen(p++)) {
    *port = string_to_ushort(p);
    return true;
  }

  return false;
}

size_t N_ParseAddressString(const char *address, char **host, uint16_t *port) {
  unsigned char octets[4];
  uint16_t      tmp_port;
  int           parsed_tokens;
  size_t        address_length;
  size_t        bytes_written;

  parsed_tokens = sscanf(address, "%hhu.%hhu.%hhu.%hhu:%hu",
    &octets[0],
    &octets[1],
    &octets[2],
    &octets[3],
    &tmp_port
  );

  if (parsed_tokens != 5) {
    D_Msg(MSG_WARN, "Invalid IP address %s\n", address);
    return 0;
  }

  address_length = snprintf(NULL, 0, "%hhu.%hhu.%hhu.%hhu",
    octets[0],
    octets[1],
    octets[2],
    octets[3]
  );

  if (!*host) {
    *host = calloc(address_length + 1, sizeof(char));

    if (!*host)
      I_Error("Calloc failed");
  }

  bytes_written = snprintf(*host, address_length + 1, "%hhu.%hhu.%hhu.%hhu",
    octets[0],
    octets[1],
    octets[2],
    octets[3]
  );

  if (bytes_written != address_length) {
    D_Msg(MSG_ERROR, "Error copying host: %s\n", strerror(errno));
    return 0;
  }

  *port = tmp_port;

  return bytes_written;
}

size_t N_OldParseAddressString(const char *address, char **host,
                                                    uint16_t *port) {
  char *sep = NULL;
  size_t host_length = 0;
  size_t address_length = strlen(address);
  bool should_free = false;
  unsigned char a;
  unsigned char b;
  unsigned char c;
  unsigned char d;
  
  if (address_length > MAX_ADDRESS_LENGTH)
    return 0;

  if (address_length == 0)
    return 0;

  sep = strchr(address, ':');

  if (!sep) {
    if (!*host) {
      *host = strdup(address);
      should_free = true;
    }
    else {
      memset(*host, 0, (address_length + 1) * sizeof(char));
      strncpy(*host, address, address_length + 1);
    }
  }
  else {
    host_length = sep - address;

    if (host_length > 0) {
      if (!*host) {
        *host = calloc(host_length + 1, sizeof(char));
        should_free = true;
      }
      else {
        memset(host, 0, (host_length + 1) * sizeof(char));
      }
      strncpy(*host, address, host_length);
    }
  }

  if (sscanf(address, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d) != 4) {
    if (should_free) {
      free(*host);
      *host = NULL;
    }

    return 0;
  }

  if (sep && strlen(++sep))
    *port = string_to_ushort(sep);

  return host_length;
}

/* vi: set et ts=2 sw=2: */
