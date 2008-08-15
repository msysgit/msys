/** \file archive_base.c
 *
 * Common functions used by the archive unpacking routines. Mainly copied from
 * zlib's untgz.c.
 */

/* Created: JohnE, 2008-07-11
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#include "archive_base.h"

#include <stdio.h>
#include <errno.h>
#include <direct.h>

#include <windows.h>
#ifndef F_OK
#  define F_OK  0
#endif
#define mkdir(dirname,mode)   _mkdir(dirname)
#ifdef _MSC_VER
#  define access(path,mode)   _access(path,mode)
#  define chmod(path,mode)    _chmod(path,mode)
#  define strdup(str)         _strdup(str)
#endif


// Contains the current error string
static char cur_error[2048] = {'\0'};

/* Get the current error string */
const char* archive_geterror()
{
	return cur_error;
}

/* Set the current error string */
void archive_seterror(const char* err_fmt, ...)
{
  va_list args;
  va_start(args, err_fmt);
  vsnprintf(cur_error, 2047, err_fmt, args);
  va_end(args);
}


/* recursive mkdir
 * abort on ENOENT; ignore other errors like "directory already exists"
 * return 1 if OK
 *        0 on error */
int makedir (const char* base, const char *newdir)
{
  char buffer[1024];
  int blen = strlen(strncpy(buffer, base, 1022));
  buffer[blen] = '/';
  int len = blen + strlen(strncpy(buffer + blen + 1, newdir, 1021 - blen));
  char *p;

  if (len <= 0)
    return 0;
  if (buffer[len-1] == '/')
    buffer[len-1] = '\0';
  if (mkdir(buffer, 0755) == 0)
    return 1;

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      if ((mkdir(buffer, 0755) == -1))
        {
          if (errno == ENOENT)
            {
              archive_seterror("Couldn't create directory %s", buffer);
              return 0;
            }
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  return 1;
}


/* set file time */
int setfiletime (const char *fname,time_t ftime)
{
#ifdef WIN32
  static int isWinNT = -1;
  SYSTEMTIME st;
  FILETIME locft, modft;
  struct tm *loctm;
  HANDLE hFile;
  int result;

  loctm = localtime(&ftime);
  if (loctm == NULL)
    return -1;

  st.wYear         = (WORD)loctm->tm_year + 1900;
  st.wMonth        = (WORD)loctm->tm_mon + 1;
  st.wDayOfWeek    = (WORD)loctm->tm_wday;
  st.wDay          = (WORD)loctm->tm_mday;
  st.wHour         = (WORD)loctm->tm_hour;
  st.wMinute       = (WORD)loctm->tm_min;
  st.wSecond       = (WORD)loctm->tm_sec;
  st.wMilliseconds = 0;
  if (!SystemTimeToFileTime(&st, &locft) ||
      !LocalFileTimeToFileTime(&locft, &modft))
    return -1;

  if (isWinNT < 0)
    isWinNT = (GetVersion() < 0x80000000) ? 1 : 0;
  hFile = CreateFile(fname, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                     (isWinNT ? FILE_FLAG_BACKUP_SEMANTICS : 0),
                     NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    return -1;
  result = SetFileTime(hFile, NULL, NULL, &modft) ? 0 : -1;
  CloseHandle(hFile);
  return result;
#else
  struct utimbuf settime;

  settime.actime = settime.modtime = ftime;
  return utime(fname,&settime);
#endif
}


/* push file attributes */
int push_attr(struct attr_item **list, const char *fname, int mode, time_t time)
{
  struct attr_item *item;

  item = (struct attr_item *)malloc(sizeof(struct attr_item));
  if (item == NULL)
    {
      archive_seterror("Out of memory");
      return -1;
    }
  item->fname = strdup(fname);
  item->mode  = mode;
  item->time  = time;
  item->next  = *list;
  *list       = item;
  return 0;
}


/* restore file attributes */
void restore_attr(struct attr_item **list)
{
  struct attr_item *item, *prev;

  for (item = *list; item != NULL; )
    {
      setfiletime(item->fname,item->time);
      chmod(item->fname,item->mode);
      free(item->fname);
      prev = item;
      item = item->next;
      free(prev);
    }
  *list = NULL;
}

