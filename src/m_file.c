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

#include "lprintf.h"
#include "m_buf.h"
#include "m_file.h"

static int fs_error_code = 0;

/* cph - disk icon not implemented */
static inline void I_BeginRead(void) {}
static inline void I_EndRead(void) {}

static void set_error_code(void) {
  fs_error_code = errno;
}

#ifndef _WIN32
static int remover(const char *path, const struct stat *stat_result,
                   int flags, struct FTW *walker) {
  if (flags == FTW_DNR || flags == FTW_NS)
    errno = EACCES;

  if (flags == FTW_D || flags == FTW_DP) {
    if (rmdir((char *)path) == -1)
      return -1;
  }
  else if (flags == FTW_F || flags == FTW_SL) {
    if (remove((char *)path) == -1)
      return -1;
  }

  return 0;
}
#endif

const char* M_GetFileSystemErrorMessage(void) {
#if !defined(_WIN32) || defined(__MINGW32__)
  return (const char *)strerror(fs_error_code);
#else
  static char error_message[512];

  strerror_s(error_message, 512, fs_error_code);

  return (const char *)error_message;
#endif
}

//
// M_NormalizeSlashes
//
// Remove trailing slashes, translate backslashes to slashes
// The string to normalize is passed and returned in str
//
// killough 11/98: rewritten
//
void M_NormalizeSlashes(char *str) {
  char *p;
  char use_slash      = '/'; // The slash type to use for normalization.
  char replace_slash = '\\'; // The kind of slash to replace.
  bool is_unc = false;

#ifdef _WIN32
  // This is an UNC path; it should use backslashes.
  // NB: We check for both in the event one was changed earlier by mistake.
  if (strlen(str) > 2 &&
      ((str[0] == '\\' || str[0] == '/') && str[0] == str[1])) {
    use_slash = '\\';
    replace_slash = '/';
    is_unc = true;
  }
#endif
   
  // Convert all replace_slashes to use_slashes
  for (p = str; *p; p++) {
    if (*p == replace_slash) {
      *p = use_slash;
    }
  }

  // Remove trailing slashes
  while (p > str && *--p == use_slash)
    *p = 0;

  // Collapse multiple slashes
  for (p = str + (is_unc ? 2 : 0); (*str++ = *p);) {
    if (*p++ == use_slash) {
      while (*p == use_slash) {
        p++;
      }
    }
  }
}

bool M_PathExists(const char *path) {
  struct stat stat_result;

  if (stat(path, &stat_result) != -1)
    return true;

  return false;
}

bool M_DirnameIsFolder(const char *path) {
  bool is_folder = false;
  char *dn = M_Dirname(path);

  if (M_IsFolder(dn))
    is_folder = true;

  free(dn);

  return is_folder;
}

void M_PathJoinBuf(buf_t *buf, const char *d, const char *f) {
  bool needs_slash = false;
  size_t d_length = strlen(d);
  size_t f_length = strlen(f);
  size_t path_len = d_length + f_length + 1;

  if ((d[d_length]) != '/') {
    needs_slash = true;
    path_len++;
  }

  M_BufferClear(buf);
  M_BufferEnsureCapacity(buf, path_len);

  M_BufferWriteChars(buf, d, d_length);
  if (needs_slash)
    M_BufferWriteChar(buf, '/');
  M_BufferWriteChars(buf, f, f_length);
}

char* M_PathJoin(const char *one, const char *two) {
  char *path = NULL;
  bool needs_slash = false;
  size_t one_length = strlen(one);
  size_t two_length = strlen(two);
  size_t path_len = one_length + two_length + 1;

  if ((one[one_length]) != '/') {
    needs_slash = true;
    path_len++;
  }

  path = calloc(path_len, sizeof(char));

  if (!path)
    I_Error("M_PathJoin: Error allocating path.\n");

  strcat(path, one);
  if (needs_slash)
    strcat(path, "/");
  strcat(path, two);

  return path;
}

bool M_IsFile(const char *path) {
  struct stat stat_result;

  if (stat(path, &stat_result) == -1)
    return false;

  if (stat_result.st_mode & S_IFREG)
    return true;

  return false;
}

bool M_IsFileInFolder(const char *folder, const char *file) {
  bool ret;
  char *full_path = M_PathJoin(folder, file);

  ret = M_IsFile(full_path);
  free(full_path);

  return ret;
}

bool M_IsFolder(const char *path) {
  struct stat stat_result;

  if (stat(path, &stat_result) == -1)
    return false;

  if (stat_result.st_mode & S_IFDIR)
    return true;

  return false;
}

bool M_IsRootFolder(const char *path) {
  bool out = false;

  if ((*path) == '/')
    out = true;

#ifdef _WIN32
  char *winpath = strdup(path);
  size_t path_size = strlen(winpath);

  if (path_size < 2)
    return false;

  M_NormalizeSlashes(winpath);

  if ((path_size == 2) && (isalpha(winpath[0])) && (winpath[1] == ':'))
    out = true;
  else if ((path_size == 3) && (strncmp(winpath + 1, ":/", 2) == 0))
    out = true;
  else if ((path_size == 2) && (strncmp(winpath, "//", 2) == 0))
    out = true;
  else if ((path_size == 4) && (strncmp(winpath, "//?/", 4) == 0))
    out = true;

#endif
  return out;
}

bool M_IsAbsolutePath(const char *path) {
  bool out = false;

  if ((*path) == '/')
    out = true;

#ifdef _WIN32
  char *winpath = strdup(path);
  size_t path_size = strlen(winpath);

  if (path_size < 3)
    return false;

  M_NormalizeSlashes(winpath);

  if (strncmp(winpath + 1, ":/", 2) == 0) // [CG] Normal C: type path.
    return true;
  else if (strncmp(winpath, "//", 2) == 0) // [CG] UNC path.
    return true;
  else if (strncmp(winpath, "//?/", 4) == 0) // [CG] Long UNC path.
    return true;
#endif
  return false;
}

const char* M_StripAbsolutePath(const char *path) {
  const char *relative_path = path;

  if (!M_IsAbsolutePath(path))
    return path;

#ifdef _WIN32
  char *winpath = strdup(path);
  size_t path_size = strlen(winpath);

  if (path_size < 3)
    return path;

  M_NormalizeSlashes(winpath);

  if (strncmp(winpath + 1, ":/", 2) == 0) // [CG] Normal C: type path.
    relative_path = winpath + 3;
  else if (strncmp(winpath, "//?/", 4) == 0) // [CG] Long UNC path.
    relative_path = winpath + 4;
#endif

  while ((*relative_path) == '/')
    relative_path++;

  return relative_path;
}

// [CG] Creates a folder.  On *NIX systems the folder is given 0700
//      permissions.
bool M_CreateFolder(const char *path) {
#ifdef _WIN32
  if (_mkdir(path) == -1) {
#else
  if (mkdir(path, S_IRWXU) == -1) {
#endif
    set_error_code();
    return false;
  }

  return true;
}

// [CG] Creates a file.  On *NIX systems the folder is given 0600 permissions.
//      If the file already exists, error codes are set.
bool M_CreateFile(const char *path) {
#ifdef _WIN32
  int fd = _open(path, _O_RDWR | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
#else
  int fd = open(path, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
#endif

  if (fd == -1) {
    set_error_code();
    return false;
  }
#if !defined(_WIN32) || defined(__MINGW32__)
  close(fd);
#else
  _close(fd);
#endif
  return true;
}

bool M_DeletePath(const char *path) {
  if (!M_PathExists(path))
    return false;

  if (M_IsFile(path))
    return M_DeleteFile(path);
  else if (M_IsFolder(path))
    return M_DeleteFolder(path);
  else
    return false;
}

bool M_DeleteFile(const char *path) {
  if (remove(path) != 0) {
    set_error_code();
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

bool M_DeleteFolder(const char *path) {
#ifdef _WIN32
  int result;
  DWORD attr = GetFileAttributes(path);

  if (attr & FILE_ATTRIBUTE_READONLY) {
    attr &= ~FILE_ATTRIBUTE_READONLY;
    if ((result = SetFileAttributes(path, attr)) == 0)
      return false;
  }

  if (_rmdir(path) == -1) {
#else
  if (rmdir(path) == -1) {
#endif
    set_error_code();
    return false;
  }

  return true;
}

bool M_IterateFiles(const char *path, file_iterator iterator) {
  size_t path_len;
  buf_t entry_buf;
  buf_t path_buf;

  if (!M_IsFolder(path))
    return false;

  path_len = strlen(path);
  M_BufferInit(&entry_buf);
  M_BufferInitWithCapacity(&path_buf, path_len + 1);
  M_BufferWriteString(&path_buf, path, path_len);
  M_NormalizeSlashes(M_BufferGetData(&path_buf));

#ifdef _WIN32
  WIN32_FIND_DATA fdata;
  HANDLE folder_handle;
  buf_t star_buf;

  M_BufferInit(&star_buf);

  // [CG] Check that the folder path has the minimum reasonable length.
  if (path_len < 4)
    I_Error("M_IterateFiles: Invalid path: %s.\n", path);

  // [CG] Check that the folder path ends in a backslash, if not add it.  Then
  //      add an asterisk (apparently Windows needs this).
  M_BufferCopy(&star_buf, &path_buf);
  M_BufferWriteChar(&star_buf, '/');
  M_BufferWriteChar(&star_buf, '*');

  folder_handle = FindFirstFile(M_BufferGetData(&star_buf), &fdata);

  while (FindNextFile(folder_handle, &fdata)) {
    size_t entry_length = strlen(fdata.cFileName);

    // [CG] Skip the "current folder" and "previous folder" entries.
    if ((entry_length == 1) && (!strcmp(fdata.cFileName, ".")))
      continue;
    if ((entry_length == 2) && (!strcmp(fdata.cFileName, "..")))
      continue;

    M_PathJoinBuf(&entry_buf, M_BufferGetData(&path_buf), fdata.cFileName);
    M_NormalizeSlashes(M_BufferGetData(&entry_buf));

    if (!iterator(M_BufferGetData(&entry_buf))) {
      set_error_code();
      FindClose(folder_handle);
      return false;
    }
  }

  FindClose(folder_handle);

  // [CG] FindNextFile returns false if there is an error, but running out of
  //      contents is considered an error so we need to check for that code to
  //      determine if we successfully walked all the contents.
  if (GetLastError() != ERROR_NO_MORE_FILES) {
    set_error_code();
    return false;
  }
#else
  DIR *d;
  struct dirent *e;

  if (!(d = opendir(path))) {
    set_error_code();
    return false;
  }

  while(d) {
    if (!(e = readdir(d))) {
      closedir(d);
      set_error_code();
      return false;
    }

    M_PathJoinBuf(&entry_buf, M_BufferGetData(&path_buf), e->d_name);
    M_NormalizeSlashes(M_BufferGetData(&entry_buf));

    if (!iterator(M_BufferGetData(&entry_buf))) {
      set_error_code();
      closedir(d);
      return false;
    }
  }
#endif
  return true;
}

bool M_WalkFiles(const char *path, file_walker walker) {
  if (!M_IsFolder(path))
    return false;

#ifdef _WIN32
  size_t path_len = strlen(path);
  WIN32_FIND_DATA fdata;
  HANDLE folder_handle;
  buf_t star_buf;
  buf_t entry_buf;
  buf_t path_buf;

  M_BufferInit(&star_buf);
  M_BufferInit(&entry_buf);
  M_BufferInitWithCapacity(&path_buf, path_len + 1);
  M_BufferWriteString(&path_buf, path);
  M_NormalizeSlashes(M_BufferGetData(&path_buf));

  // [CG] Check that the folder path has the minimum reasonable length.
  if (path_len < 4)
    I_Error("M_WalkFiles: Invalid path: %s.\n", path);

  // [CG] Check that the folder path ends in a backslash, if not add it.  Then
  //      add an asterisk (apparently Windows needs this).
  M_BufferCopy(&star_buf, &path_buf);
  M_BufferWriteChar(&star_buf, '/');
  M_BufferWriteChar(&star_buf, '*');

  folder_handle = FindFirstFile(M_BufferGetData(&star_buf), &fdata);

  while(FindNextFile(folder_handle, &fdata)) {
    size_t entry_length = strlen(fdata.cFileName);

    // [CG] Skip the "current folder" and "previous folder" entries.
    if ((entry_length == 1) && (!strcmp(fdata.cFileName, ".")))
      continue;
    if ((entry_length == 2) && (!strcmp(fdata.cFileName, "..")))
      continue;

    M_PathJoinBuf(&entry_buf, M_BufferGetData(&path_buf), fdata.cFileName);
    M_NormalizeSlashes(M_BufferGetData(&entry_buf));

    if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if (!M_WalkFiles(M_BufferGetData(&entry_buf), walker))
        return false;
    }
    else if (!walker(M_BufferGetData(&entry_buf))) {
      FindClose(folder_handle);
      set_error_code();
      return false;
    }
  }

  FindClose(folder_handle);

  // [CG] FindNextFile returns false if there is an error, but running out of
  //      contents is considered an error so we need to check for that code to
  //      determine if we successfully walked all the contents.
  if (GetLastError() == ERROR_NO_MORE_FILES) {
    if (!walker(path))
      return false;
  }
  else {
    set_error_code();
    return false;
  }
#else
  if (nftw(path, walker, 64, FTW_CHDIR | FTW_DEPTH | FTW_PHYS) == -1) {
    set_error_code();
    return false;
  }
#endif
  return true;
}

bool M_DeleteFolderAndContents(const char *path) {
#ifdef _WIN32
  return M_WalkFiles(path, M_DeletePath);
#else
  return M_WalkFiles(path, remover);
#endif
}

char* M_GetCurrentFolder(void) {
  char *output = NULL;

#ifdef _WIN32
  char *temp = NULL;

  if (!(temp = _getcwd(temp, 1))) {
    set_error_code();
    return false;
  }
  output = strdup(temp);
  free(temp);
#else
  size_t size;

  size = (size_t)pathconf(".", _PC_PATH_MAX);
  output = malloc(size * sizeof(char));
  if (!(output = getcwd(output, size))) {
    set_error_code();
    return false;
  }
#endif

  return output;
}

bool M_SetCurrentFolder(const char *path) {
#if !defined(_WIN32) || defined(__MINGW32__)
  if (chdir(path) == -1)
#else
  if (_chdir(path) == -1)
#endif
    return false;

  return true;
}

char* M_Dirname(const char *path) {
  char *dn = NULL;

#ifdef _WIN32
  errno_t error = 0;
  const char drive[_MAX_DRIVE + 1];
  const char dir[_MAX_DIR + 1];

  error = _splitpath_s(
      path, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0
  );

  if (error)
    I_Error("M_Dirname: Error getting dirname of %s.\n", path);

  dn = calloc(strlen(drive) + strlen(dir) + 1, sizeof(char));

  if (dn == NULL)
    I_Error("M_Dirname: Error allocating dirname\n");

  strcat(dn, drive);
  strcat(dn, dir);
#else
  char *mod_path = strdup(path);

  if (mod_path == NULL)
    I_Error("M_Dirname: Error duplicating path\n");

  dn = strdup(dirname(mod_path));

  if (dn == NULL)
    I_Error("M_Dirname: Error duplicating dirname\n");

  free(mod_path);
#endif

  return dn;
}

const char* M_Basename(const char *path) {
  const char *bn = NULL;

#ifdef _WIN32
  errno_t error = 0;
  char filename[_MAX_FNAME + 1];
  char extension[_MAX_EXT + 1];

  if (filename == NULL)
    I_Error("M_Basename: Error allocating filename.\n");

  if (extension == NULL)
    I_Error("M_Basename: Error allocating extension.\n");

  error = _splitpath_s(
    path, NULL, 0, NULL, 0, filename, _MAX_FNAME, extension, _MAX_EXT
  );

  if (error)
    I_Error("M_Basename: Error getting basename of %s.\n", path);

  bn = calloc(strlen(filename) + strlen(extension) + 2, sizeof(char));

  if (bn == NULL)
    I_Error("M_Basename: Error allocating basename.\n");

  strcat(bn, filename);
  strcat(bn, ".");
  strcat(bn, extension);
#else
  char *mod_path = strdup(path);

  if (mod_path == NULL)
    I_Error("M_Basename: Error duplicating path\n");

  /*
   * [CG] Uses POSIX version because, even though _GNU_SOURCE is defined by
   *      CMakeLists.txt, libgen.h is included by z_zone.h
   */
  bn = strdup(basename(mod_path));

  if (bn == NULL)
    I_Error("M_Basename: Error duplicating basename\n");

  free(mod_path);
#endif

  return bn;
}

bool M_RenamePath(const char *oldpath, const char *newpath) {
  if (rename(oldpath, newpath)) {
    set_error_code();
    return false;
  }
  return true;
}

FILE* M_OpenFile(const char *path, const char *mode) {
  FILE *f = fopen(path, mode);

  if (!f)
    set_error_code();

  return f;
}

bool M_ReadFromFile(void *ptr, size_t size, size_t count, FILE *f) {
  size_t result;

  I_BeginRead();
  result = fread(ptr, size, count, f);
  I_EndRead();

  if (result != count) {
    if (!feof(f)) {
      set_error_code();
      return false;
    }
  }

  return true;
}

bool M_WriteToFile(const void *ptr, size_t size, size_t count, FILE *f) {
  size_t bytes_written;

  I_BeginRead();
  bytes_written = fwrite(ptr, size, count, f);
  I_EndRead();

  if (bytes_written != (size * count)) {
    set_error_code();
    return false;
  }

  return true;
}

long M_GetFilePosition(FILE *f) {
  long result = ftell(f);

  if (result == -1)
    set_error_code();

  return result;
}

bool M_SeekFile(FILE *f, long int offset, int origin) {
  if (fseek(f, offset, origin) != 0) {
    set_error_code();
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
    set_error_code();
    return false;
  }

  return true;
}

bool M_CloseFile(FILE *f) {
  if (fclose(f) != 0) {
    set_error_code();
    return false;
  }

  return true;
}

/*
 * M_ReadFile
 *
 * killough 9/98: rewritten to use stdio and to flash disk icon
 */
int M_ReadFile(const char *name, byte **buffer) {
  FILE *fp;
  bool success;
  size_t length;

  if (!(fp = M_OpenFile(name, "rb")))
    return false;

  length = M_FileLength(fp);

  *buffer = calloc(length, sizeof(byte));

  success = M_ReadFromFile(*buffer, sizeof(byte), length, fp);
  M_CloseFile(fp);

  if (success)
    return length;

  /*
   * cph 2002/08/10 - this used to return 0 on error, but that's ambiguous,
   * because we could have a legit 0-length file. So make it -1.
   */
  return -1;
}

/*
 * M_WriteFile
 *
 * killough 9/98: rewritten to use stdio and to flash disk icon
 */
bool M_WriteFile(const char *name, const void *source, size_t length) {
  FILE *fp;
  bool success;

  if (!(fp = M_OpenFile(name, "rb")))
    return false;

  success = M_WriteToFile(source, sizeof(byte), length, fp);
  M_CloseFile(fp);

  return success;
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

//
// 1/18/98 killough: adds a default extension to a path
// Note: Backslashes are treated specially, for MS-DOS.
//

char* M_AddDefaultExtension(char *path, const char *ext) {
  char *p = path;

  while (*p++);

  while (p-- > path && *p != '/' && *p != '\\') {
    if (*p == '.')
      return path;
  }

  if (*ext != '.')
    strcat(path, ".");

  return strcat(path, ext);
}

void M_ReportFileSystemError(void) {
  set_error_code();
}

/* vi: set et ts=2 sw=2: */

