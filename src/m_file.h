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


#ifndef M_FILE_H__
#define M_FILE_H__

#if 0
typedef bool(*file_iterator)(const char *path);

#ifdef _WIN32
typedef bool(*file_walker)(const char *path);
#else
typedef  int(*file_walker)(const char *path, const struct stat *stat_result,
                           int flags, struct FTW *walker);
#endif
#endif

const char* M_GetFileError(void);
char*       M_LocalizePath(const char *path);
char*       M_UnLocalizePath(const char *local_path);
bool        M_PathExists(const char *path);
char*       M_GetCurrentFolder(void);
bool        M_SetCurrentFolder(const char *path);
char*       M_Dirname(const char *path);
char*       M_Basename(const char *path);
bool        M_DirnameIsFolder(const char *path);
bool        M_PathJoinBuf(buf_t *buf, const char *one, const char *two);
char*       M_PathJoin(const char *one, const char *two);
bool        M_IsFolder(const char *path);
bool        M_IsFile(const char *path);
bool        M_IsFileInFolder(const char *folder, const char *file);
bool        M_IsRootFolder(const char *path);
bool        M_IsAbsolutePath(const char *path);
char*       M_StripAbsolutePath(const char *path);
bool        M_RenamePath(const char *oldpath, const char *newpath);

bool        M_CreateFolder(const char *path, int mode);
bool        M_CreateFile(const char *path, int mode);
bool        M_DeletePath(const char *path);
bool        M_DeleteFolder(const char *path);
bool        M_DeleteFile(const char *path);
bool        M_DeleteFileInFolder(const char *folder, const char *file);
#if 0
bool        M_IterateFiles(const char *path, file_iterator iterator);
bool        M_WalkFiles(const char *path, file_walker walker);
bool        M_DeleteFolderAndContents(const char *path);
bool        M_ReadFromFile(void *ptr, size_t size, size_t count, FILE *f);
bool        M_WriteToFile(const void *ptr, size_t size, size_t count, FILE *f);
#endif
int         M_Open(const char *path, int flags, int mode);
bool        M_Close(int fd);
bool        M_Seek(int fd, off_t offset, int origin);
bool        M_Read(int fd, void *vbuf, size_t sz);
uint32_t    M_FDLength(int fd);
FILE*       M_OpenFile(const char *path, const char *mode);
bool        M_ReadFile(const char *path, char **data, size_t *size);
bool        M_ReadFileBuf(buf_t *buf, const char *path);
bool        M_WriteFile(const char* name, const char* source, size_t size);
long        M_GetFilePosition(FILE *f);
bool        M_SeekFile(FILE *f, long int offset, int origin);
uint32_t    M_FileLength(FILE *f);
bool        M_FlushFile(FILE *f);
bool        M_CloseFile(FILE *f);
// killough
void        M_ExtractFileBase(const char *path, char *dest);
// killough 1/18/98
char*       M_AddDefaultExtension(const char *path, const char *ext);
char*       M_SetFileExtension(const char *path, const char *ext);

#endif

/* vi: set et ts=2 sw=2: */

