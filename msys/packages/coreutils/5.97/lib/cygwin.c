/* cygwin.c - helper functions unique to Cygwin

   Copyright (C) 2005, 2006 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Written by Eric Blake.  */

#include "cygwin.h"
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

/* Return -1 if PATH not found, 0 if PATH spelled correctly, and 1 if PATH
   had ".exe" automatically appended by cygwin.  Don't change errno.  */

int
cygwin_spelling (char const *path)
{
  char path_exact[PATH_MAX + 4];
  int saved_errno = errno;
  int result = 0; /* Start with assumption that PATH is okay.  */
  int len = strlen (path);

  if (! path || ! *path || len >= PATH_MAX)
    /* PATH will cause EINVAL or ENAMETOOLONG, treat it as non-existing.  */
    return -1;
  if (path[len - 1] == '.' || path[len-1] == '/')
    /* Don't change spelling if there is a trailing `.' or `/'.  */
    return 0;
  if (readlink (path, NULL, 0) < 0)
    { /* PATH is not a symlink.  */
      if (errno == EINVAL)
	{ /* PATH exists.  Appending trailing `.' exposes whether it is
	     PATH or PATH.exe for normal disk files, but also check appending
	     trailing `.exe' to be sure on virtual/managed directories.  */
	  strcat (strcpy (path_exact, path), ".");
	  if (access (path_exact, F_OK) < 0)
	    { /* PATH. does not exist.  */
	      strcat (path_exact, "exe");
	      if (access (path_exact, F_OK) == 0)
		/* But PATH.exe does, so append .exe.  */
		result = 1;
	    }
	}
      else
	/* PATH does not exist.  */
	result = -1;
    }
  else
    { /* PATH is a symlink.  Appending trailing `.lnk' exposes whether
	 it is PATH or PATH.exe.  */
      strcat (strcpy (path_exact, path), ".lnk");
      if (readlink (path_exact, NULL, 0) < 0)
	result = 1;
    }

  errno = saved_errno;
  return result;
}
