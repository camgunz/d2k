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

#include "p_checksum.h"
#include "p_user.h"
#include "g_game.h"
#include "md5.h"

/* forward decls */
static void p_checksum_cleanup(void);
void checksum_gamestate(int tic);

/* vars */
static void p_checksum_nop(int tic){} /* do nothing */
void (*P_Checksum)(int) = p_checksum_nop;

/*
 * P_RecordChecksum
 * sets up the file and function pointers to write out checksum data
 */
static FILE *outfile = NULL;
static struct MD5Context md5global;

void P_RecordChecksum(const char *file) {
    size_t fnsize;

    fnsize = strlen(file);

    /* special case: write to stdout */
    if(0 == strncmp("-",file,MIN(1,fnsize)))
        outfile = stdout;
    else {
        outfile = fopen(file,"wb");
        if(NULL == outfile) {
            I_Error("cannot open %s for writing checksum:\n%s\n",
                    file, strerror(errno));
        }
        atexit(p_checksum_cleanup);
    }

    MD5Init(&md5global);

    P_Checksum = checksum_gamestate;
}

void P_ChecksumFinal(void) {
    int i;
    unsigned char digest[16];

    if (!outfile)
      return;

    MD5Final(digest, &md5global);
    fprintf(outfile, "final: ");
    for (i=0; i<16; i++)
        fprintf(outfile,"%x", digest[i]);
    fprintf(outfile, "\n");
    MD5Init(&md5global);
}

static void p_checksum_cleanup(void) {
    if (outfile && (outfile != stdout))
        fclose(outfile);
}

/*
 * runs on each tic when recording checksums
 */
void checksum_gamestate(int tic) {
    int i;
    struct MD5Context md5ctx;
    unsigned char digest[16];
    char buffer[2048];

    fprintf(outfile,"%6d, ", tic);

    /* based on "ArchivePlayers" */
    MD5Init(&md5ctx);
    for (i=0 ; i<MAXPLAYERS ; i++) {
        if (!playeringame[i]) continue;

        snprintf(buffer, sizeof(buffer), "%d", players[i].health);
        buffer[sizeof(buffer)-1] = 0;

        MD5Update(&md5ctx, (unsigned char const *)&buffer, strlen(buffer));
    }
    MD5Final(digest, &md5ctx);
    for (i=0; i<16; i++) {
        MD5Update(&md5global, (unsigned char const *)&digest[i], sizeof(digest[i]));
        fprintf(outfile,"%x", digest[i]);
    }

    fprintf(outfile,"\n");
}

/* vi: set et ts=2 sw=2: */

