/* msys_symlink.c
 * Copyright (C) 2002, Earnie Boyd
 * This file is a part of MSYS.
 * ***************************************************************************/

#include "msys_symlink.h"
#if !DO_CPP_NEW
#include <stdlib.h>
#endif

extern "C"
int
msys_symlink (const char * topath, const char * frompath)
{

  char wtopath[MAX_PATH] = "\0";
  char wfrompath[MAX_PATH] = "\0";
  static char dir_created[MAX_PATH];
  char *w_topath = wtopath;
  char *w_frompath = wfrompath;
#if DO_CPP_NEW
  struct stat *sb_frompath = new struct stat;
  struct stat *sb_topath = new struct stat;
  struct _WIN32_FIND_DATAA *dHfile = new struct _WIN32_FIND_DATAA;
#else
  struct stat *sb_frompath = (struct stat *) malloc (sizeof (struct stat));
  struct stat *sb_topath = (struct stat *) malloc (sizeof (struct stat));
  struct _WIN32_FIND_DATAA *dHfile = (struct _WIN32_FIND_DATAA *) malloc (sizeof (struct _WIN32_FIND_DATAA));
#endif
  int existing_destination;
  int destination_isdir;
  int src_isdir;
  HANDLE dH;
  BOOL findfiles;
  BOOL frompath_needs_slash;
  BOOL topath_needs_slash;
  char fromfile[MAX_PATH];
  char tofile[MAX_PATH];
  /*
   * FIXME: Use relavent switches.
   * FIXME: remove this comment.
  x.dereference = DEREF_NEVER;
  x.preserve_owner_and_group = 1;
  x.preserve_chmod_bits = 1;
  x.preserve_timestamps = 1;
  x.require_preserve = 1;
  x.recursive = 1;
  x.copy_as_regular = 0;
  x.umask_kill = ~ (mode_t) 0;
  x.xstat = lstat;
  */

  debug_printf("msys_symlink (%s, %s)", frompath, topath);

  frompath_needs_slash = FALSE;
  topath_needs_slash = FALSE;

  if (stat (frompath, sb_frompath))
    {
      debug_printf("Failed stat");
      return 1;
    }

  if (stat (topath, sb_topath))
    {
      if (errno == ENOENT)
	{
	  existing_destination = 0;
	  debug_printf("no existing destination");
	}
      else
	{
	  debug_printf("error: %d", errno);
	  return 1;
	}
    }
  else
    {
      existing_destination = 1;
      debug_printf("existing destination");
    }

  src_isdir = S_ISDIR(sb_frompath->st_mode);
  destination_isdir = S_ISDIR(sb_topath->st_mode) | src_isdir;

  {
    char *tptr;
    if (*(tptr = strchr (frompath, '\0') -1) != '/')
	frompath_needs_slash = TRUE;
    else
	*tptr = '\0';
    if (*(tptr = strchr (topath, '\0') -1) != '/')
	topath_needs_slash - TRUE;
    else
	*tptr = '\0';
  }

  w_frompath = msys_p2w(frompath);
  debug_printf("w_frompath: %s", w_frompath);
  w_topath = msys_p2w(topath);
  debug_printf("w_topath: %s", w_topath);

  if (destination_isdir)
    {
      if (!existing_destination)
	{
	  if (!CreateDirectoryEx(w_frompath, w_topath, NULL))
	    {
	      debug_printf("CreateDirectoryEx(%s, %s, 0) failed", w_frompath, w_topath);
	      return 1;
	    }
	  strcpy(dir_created, "./\0");
	  strcat(dir_created, w_topath);
	}
      else
	{
	  set_errno(EEXIST);
	  return 1;
	}
	{
	  char findpath[MAX_PATH+2];
	  strcpy(findpath, w_frompath);
	  strcat(findpath, "/*");
	  dH = FindFirstFile(findpath, dHfile);
	  debug_printf("dHfile(1): %s", dHfile->cFileName);
	  findfiles = FindNextFile (dH, dHfile);
	  debug_printf("dHfile(2): %s", dHfile->cFileName);
	  findfiles = FindNextFile (dH, dHfile);
	  debug_printf("dHfile(3): %s", dHfile->cFileName);
	}
	while (findfiles)
	  {
	    strcpy(fromfile, w_frompath);
	    if (frompath_needs_slash)
		strcat(fromfile, "/");
	    strcat(fromfile, dHfile->cFileName);
	    debug_printf("%s <-> %s", fromfile, dir_created);
	    if (strcmp (fromfile, dir_created))
	      {
		strcpy(tofile, w_topath);
		//if (topath_needs_slash)
		    strcat(tofile, "/");
		strcat(tofile, dHfile->cFileName);
		if (msys_symlink (tofile, fromfile))
		    return 1;
	      }
	    findfiles = FindNextFile (dH, dHfile);
	    debug_printf("dHfile(4): %s", dHfile->cFileName);
	  }
	if (GetLastError() != ERROR_NO_MORE_FILES)
	    return 1;
    }
  else
    {
      if (!CopyFile (w_frompath, w_topath, FALSE))
	  return 1;
    }

  return 0;
}
