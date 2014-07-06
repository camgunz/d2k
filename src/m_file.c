// Emacs style mode select -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Charles Gunyon
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//--------------------------------------------------------------------------
//
// DESCRIPTION:
//    Platform-independent filesystem operations.
//
//-----------------------------------------------------------------------------

#include "z_zone.h"

#include <glib.h>
#include <glib/gstdio.h>

#include "lprintf.h"
#include "m_buf.h"
#include "m_file.h"

#define M_FILE_ERROR m_file_error_quark()

static GQuark m_file_error_quark(void) {
  return g_quark_from_static_string("m-file-error-quark");
}

static GError *file_error = NULL;

/* cph - disk icon not implemented */
static inline void I_BeginRead(void) {}
static inline void I_EndRead(void) {}

static void clear_file_error(void) {
  if (file_error)
    g_clear_error(&file_error);
}

static void set_file_error(GQuark domain, gint code, gchar *message) {
  if (file_error)
    g_clear_error(&file_error);

  g_propagate_error(&file_error, g_error_new_literal(domain, code, message));
}

static void set_file_error_from_errno(void) {
  set_file_error(
    M_FILE_ERROR, g_file_error_from_errno(errno), strerror(errno)
  );
}

#if 0
static gchar* localize_path(const char *path) {
  gsize sz;
  size_t path_len = strlen(path);

  if (!path)
    return NULL;

#ifdef _WIN32
  // return g_win32_locale_filename_from_utf8(path);
  gchar *lp = g_convert(
    path, path_len, "wchar_t", "UTF-8", NULL, &sz, &file_error
  );
  
  wprintf(L"localize_path: path_len, bytes_written, lp: %zu, %zu, %s\n",
    path_len, sz, lp
  );

  return lp;
#else
  return g_filename_from_utf8(path, -1, NULL, &sz, &file_error);
#endif
}

static gchar* unlocalize_path(const char *local_path) {
  gsize sz;

  return g_filename_to_utf8(local_path, -1, NULL, &sz, &file_error);
}

static bool unlocalize_path_buf(buf_t *buf, const char *local_path) {
  gsize sz;
  gchar *ulp = g_locale_to_utf8(local_path, -1, NULL, &sz, &file_error);

  if (ulp == NULL)
    return false;

  M_BufferWrite(buf, ulp, sz);
  M_BufferWriteUChar(buf, 0);

  g_free(ulp);

  return true;
}
#endif

const char* M_GetFileError(void) {
  return file_error->message;
}

char* M_LocalizePath(const char *path) {
  gsize sz;
  gchar *lp;
  char *out;

  if (!path)
    return NULL;

  clear_file_error();

#ifdef _WIN32
  size_t path_len = strlen(path);

  lp = g_convert(path, path_len, "wchar_t", "UTF-8", NULL, &sz, &file_error);
  out = calloc(sz + sizeof(wchar_t), 1);
#else
  lp = g_filename_from_utf8(path, -1, NULL, &sz, &file_error);
  out = calloc(sz + 1, sizeof(char));
#endif
  
  if (!out)
    I_Error("M_LocalizePath: calloc failed");

  memcpy(out, lp, sz);

  g_free(lp);

  return out;
}

char* M_UnLocalizePath(const char *local_path) {
  gsize sz;
  gchar *ulp;
  char *out;

  if (!local_path)
    return NULL;

  clear_file_error();

#ifdef _WIN32
  ulp = g_convert(local_path, -1, "UTF-8", "wchar_t", NULL, &sz, &file_error);
#else
  ulp = g_filename_to_utf8(local_path, -1, NULL, &sz, &file_error);
#endif
  
  out = calloc(sz + 1, sizeof(char));

  if (!out)
    I_Error("M_UnLocalizePath: calloc failed");

  memcpy(out, ulp, sz);

  g_free(ulp);

  return out;
}

bool M_PathExists(const char *path) {
  return g_file_test(path, G_FILE_TEST_EXISTS);
}

char* M_GetCurrentFolder(void) {
  gchar *current_folder = g_get_current_dir();
  char *out = strdup(current_folder);

  g_free(current_folder);

  return out;
}

bool M_SetCurrentFolder(const char *path) {
  int res = g_chdir(path);

  if (res == -1) {
    set_file_error_from_errno();
    return false;
  }

  return true;
}

char* M_Dirname(const char *path) {
  char *gdir = g_path_get_dirname(path);
  char *out = strdup(gdir);

  g_free(gdir);

  return out;
}

char* M_Basename(const char *path) {
  char *gbase = g_path_get_basename(path);
  char *out = strdup(gbase);

  g_free(gbase);

  return out;
}

bool M_DirnameIsFolder(const char *path) {
  char *dn = M_Dirname(path);
  bool is_folder = M_IsFolder(dn);

  free(dn);

  return is_folder;
}

bool M_PathJoinBuf(buf_t *buf, const char *one, const char *two) {
  char *joined_path = g_build_filename(one, two, NULL);

  M_BufferWriteString(buf, joined_path, strlen(joined_path));

  g_free(joined_path);

  return true;
}

char* M_PathJoin(const char *one, const char *two) {
  char *joined_path = g_build_filename(one, two, NULL);
  char *out = strdup(joined_path);

  g_free(joined_path);

  return out;
}

bool M_IsFolder(const char *path) {
  return g_file_test(path, G_FILE_TEST_IS_DIR);
}

bool M_IsFile(const char *path) {
  return g_file_test(path, G_FILE_TEST_IS_REGULAR);
}

bool M_IsFileInFolder(const char *folder, const char *file) {
  char *full_path = M_PathJoin(folder, file);
  bool ret = M_IsFile(full_path);

  free(full_path);

  return ret;
}

bool M_IsRootFolder(const char *path) {
  const gchar *no_root = g_path_skip_root(path);

  return (no_root != NULL && *no_root == '0');
}

bool M_IsAbsolutePath(const char *path) {
  return g_path_is_absolute(path);
}

char* M_StripAbsolutePath(const char *path) {
  const gchar *no_root = g_path_skip_root(path);

  if (no_root != NULL && *no_root == '0')
    return NULL;

  return strdup(no_root);
}

bool M_RenamePath(const char *oldpath, const char *newpath) {
  int res = g_rename(oldpath, newpath);

  if (res == -1) {
    set_file_error_from_errno();
    return false;
  }

  return true;
}

bool M_CreateFolder(const char *path, int mode) {
  int res = g_mkdir(path, mode);

  if (res == -1) {
    set_file_error_from_errno();
    return false;
  }

  return true;
}

bool M_CreateFile(const char *path, int mode) {
  int fd = g_creat(path, S_IRUSR | S_IWUSR);

  if (fd == -1) {
    set_file_error_from_errno();
    return false;
  }

  return M_Close(fd);
}

bool M_DeletePath(const char *path) {
  if (!M_PathExists(path))
    return false;

  if (M_IsFile(path))
    return M_DeleteFile(path);

  if (M_IsFolder(path))
    return M_DeleteFolder(path);

  return false;
}

bool M_DeleteFolder(const char *path) {
  int res = g_rmdir(path);

  if (res == -1) {
    set_file_error_from_errno();
    return false;
  }

  return true;
}

bool M_DeleteFile(const char *path) {
  int res = g_unlink(path);

  if (res == -1) {
    set_file_error_from_errno();
    return false;
  }

  return true;
}

bool M_DeleteFileInFolder(const char *folder, const char *file) {
  bool ret;
  char *full_path = M_PathJoin(folder, file);

  ret = M_DeleteFile(full_path);
  free(full_path);

  return ret;
}

#if 0
bool M_IterateFiles(const char *path, file_iterator iterator) {
  DIR *d;
  struct dirent *e;
  size_t path_len;
  buf_t entry_buf;
  buf_t path_buf;

  if (!M_IsFolder(path))
    return false;

  path_len = strlen(path);
  M_BufferInit(&entry_buf);
  M_BufferInitWithCapacity(&path_buf, path_len + 1);
  M_BufferWriteString(&path_buf, path, path_len);

  if (!(d = opendir(path))) {
    set_file_error_from_errno();
    return false;
  }

  while(d) {
    if (!(e = readdir(d))) {
      closedir(d);
      set_file_error_from_errno();
      return false;
    }

    M_PathJoinBuf(&entry_buf, M_BufferGetData(&path_buf), e->d_name);

    if (!iterator(M_BufferGetData(&entry_buf))) {
      set_file_error_from_errno();
      closedir(d);
      return false;
    }
  }
  return true;
}

bool M_WalkFiles(const char *path, file_walker walker) {
  if (!M_IsFolder(path))
    return false;

  if (nftw(path, walker, 64, FTW_CHDIR | FTW_DEPTH | FTW_PHYS) == -1) {
    set_file_error_from_errno();
    return false;
  }

  return true;
}

bool M_DeleteFolderAndContents(const char *path) {
  return M_WalkFiles(path, remover);
}
#endif

int M_Open(const char *path, int flags, int mode) {
  int fd = g_open(path, flags, mode);

  if (fd == -1)
    set_file_error_from_errno();

  return fd;
}

bool M_Close(int fd) {
  clear_file_error();

  return g_close(fd, &file_error);
}

bool M_Seek(int fd, off_t offset, int origin) {
  if (lseek(fd, offset, origin) == ((off_t)-1)) {
    set_file_error_from_errno();
    return false;
  }

  return true;
}

bool M_Read(int fd, void *vbuf, size_t sz) {
  void *buf = vbuf;

  while (sz) {
    ssize_t bytes_read = read(fd, buf, sz);

    if (bytes_read <= 0) {
      set_file_error_from_errno();
      return false;
    }

    sz -= bytes_read;
    buf += bytes_read;
  }

  return true;
}

uint32_t M_FDLength(int fd) {
  off_t curpos, len;

  curpos = lseek(fd, 0, SEEK_CUR);
  lseek(fd, 0, SEEK_END);
  len = lseek(fd, 0, SEEK_CUR);
  lseek(fd, curpos, SEEK_SET);

  return (uint32_t)len;
}

FILE* M_OpenFile(const char *path, const char *mode) {
  FILE *f = g_fopen(path, mode);

  if (!f)
    set_file_error_from_errno();

  return f;
}

bool M_ReadFile(const char *path, char **data, size_t *size) {
  clear_file_error();

  I_BeginRead();
  gboolean res = g_file_get_contents(path, data, size, &file_error);
  I_EndRead();

  return res;
}

bool M_ReadFileBuf(buf_t *buf, const char *path) {
  char *data = NULL;
  gsize size;

  clear_file_error();

  I_BeginRead();
  gboolean res = g_file_get_contents(path, &data, &size, &file_error);
  I_EndRead();

  if (!res)
    return false;

  M_BufferWrite(buf, data, size);

  return true;
}

bool M_WriteFile(const char *path, const char *contents, size_t size) {
  clear_file_error();

  I_BeginRead();
  gboolean res = g_file_set_contents(path, contents, size, &file_error);
  I_EndRead();

  return res;
}

long M_GetFilePosition(FILE *f) {
  long result = ftell(f);

  if (result == -1)
    set_file_error_from_errno();

  return result;
}

bool M_SeekFile(FILE *f, long int offset, int origin) {
  if (fseek(f, offset, origin) != 0) {
    set_file_error_from_errno();
    return false;
  }

  return true;
}

/*
 * M_FileLength
 *
 * Gets the length of a file given its handle.
 * haleyjd 03/09/06: made global
 * haleyjd 01/04/10: use fseek/ftell
 */
uint32_t M_FileLength(FILE *f) {
  long curpos, len;

  curpos = M_GetFilePosition(f);
  M_SeekFile(f, 0, SEEK_END);
  len = M_GetFilePosition(f);
  M_SeekFile(f, curpos, SEEK_SET);

  return (uint32_t)len;
}

bool M_FlushFile(FILE *f) {
  if (fflush(f) != 0) {
    set_file_error_from_errno();
    return false;
  }

  return true;
}

bool M_CloseFile(FILE *f) {
  if (fclose(f) != 0) {
    set_file_error_from_errno();
    return false;
  }

  return true;
}

void M_ExtractFileBase(const char *path, char *dest) {
  const char *src = path + strlen(path) - 1;
  int length;

  // back up until a \ or the start
  while (src != path && src[-1] != ':' // killough 3/22/98: allow c:filename
         && *(src-1) != '\\'
         && *(src-1) != '/') {
    src--;
  }

  // copy up to eight characters
  memset(dest, 0, 8);
  length = 0;

  while ((*src) && (*src != '.') && (++length < 9)) {
    *dest++ = toupper(*src);
    src++;
  }
  /*
   * cph - length check removed, just truncate at 8 chars.
   * If there are 8 or more chars, we'll copy 8, and no zero termination
   */
}

/*
 * 1/18/98 killough: adds a default extension to a path
 * Note: Backslashes are treated specially, for MS-DOS.
 */
char* M_AddDefaultExtension(const char *path, const char *ext) {
  char *dirname;
  char *basename;
  char *path_ext;
  char *out;
  buf_t buf;

  basename = M_Basename(path);

  if (basename == NULL) {
    I_Error("M_AddDefaultExtension: Error getting basename of %s (%s)",
      path, M_GetFileError()
    );
  }

  dirname = M_Dirname(path);

  if (dirname == NULL) {
    I_Error("M_AddDefaultExtension: Error getting dirname of %s (%s)",
      path, M_GetFileError()
    );
  }

  path_ext = strrchr(basename, '.');

  if (path_ext) {
    free(dirname);
    free(basename);
    return strdup(path);
  }

  M_BufferInit(&buf);

  if (!M_PathJoinBuf(&buf, dirname, basename)) {
    I_Error("M_AddDefaultExtension: Error joining %s and %s (%s)",
      dirname, basename, M_GetFileError()
    );
  }

  free(dirname);
  free(basename);

  M_BufferSeekBackward(&buf, 1);
  M_BufferWriteUChar(&buf, '.');
  M_BufferWriteString(&buf, ext, strlen(ext));

  out = strdup(M_BufferGetData(&buf));

  M_BufferFree(&buf);

  return out;
}

char* M_SetFileExtension(const char *path, const char *ext) {
  char *dirname;
  char *basename;
  char *path_ext;
  char *out;
  buf_t buf;

  basename = M_Basename(path);

  if (basename == NULL) {
    I_Error("M_SetFileExtension: Error getting basename of %s (%s)",
      path, M_GetFileError()
    );
  }

  dirname = M_Dirname(path);

  if (dirname == NULL) {
    I_Error("M_SetFileExtension: Error getting dirname of %s (%s)",
      path, M_GetFileError()
    );
  }

  path_ext = strrchr(basename, '.');
  if (path_ext)
    *path_ext = 0;

  M_BufferInit(&buf);

  if (!M_PathJoinBuf(&buf, dirname, basename)) {
    I_Error("M_SetFileExtension: Error joining %s and %s (%s)",
      dirname, basename, M_GetFileError()
    );
  }

  free(dirname);
  free(basename);

  M_BufferSeekBackward(&buf, 1);
  M_BufferWriteUChar(&buf, '.');
  M_BufferWriteString(&buf, ext, strlen(ext));

  out = strdup(M_BufferGetData(&buf));

  M_BufferFree(&buf);

  return out;
}

/* vi: set et ts=2 sw=2: */

