/* dirname.c -- return all but the last element in a path
   Copyright 1990, 1998, 2000, 2001 Free Software Foundation, Inc.

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
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <dirname.h>
#include <xalloc.h>

#ifndef FILESYSTEM_PREFIX_LEN
# define FILESYSTEM_PREFIX_LEN(Filename) 0
#endif

#ifndef ISSLASH
# define ISSLASH(C) ((C) == '/')
#endif

char *base_name PARAMS ((char const *name));

/* Return the length of `dirname (PATH)', or zero if PATH is
   in the working directory.  Works properly even if
   there are trailing slashes (by effectively ignoring them).  */
size_t
dirlen (char const *path)
{
  size_t prefix_length = FILESYSTEM_PREFIX_LEN (path);
  size_t length;

  /* Strip the basename and any redundant slashes before it.  */
  for (length = base_name (path) - path;  prefix_length < length;  length--)
    if (! ISSLASH (path[length - 1]))
      return length;

  /* But don't strip the only slash from "/".  */
  return prefix_length + ISSLASH (path[prefix_length]);
}

/* Return the leading directories part of PATH,
   allocated with malloc.  If out of memory, return 0.
   Works properly even if there are trailing slashes
   (by effectively ignoring them).  */

char *
dir_name (char const *path)
{
  size_t length = dirlen (path);
  int append_dot = (length == FILESYSTEM_PREFIX_LEN (newpath));
  char *newpath = xmalloc (length + append_dot + 1);
  memcpy (newpath, path, length);
  if (append_dot)
    newpath[length++] = '.';
  newpath[length] = 0;
  return newpath;
}

#ifdef TEST_DIRNAME
/*

Run the test like this (expect no output):
  gcc -DHAVE_CONFIG_H -DTEST_DIRNAME -I.. -O -Wall basename.c dirname.c
  sed -n '/^BEGIN-DATA$/,/^END-DATA$/p' dirname.c|grep -v DATA|./a.out

BEGIN-DATA
foo//// .
bar/foo//// bar
foo/ .
/ /
. .
a .
END-DATA

*/

# define MAX_BUFF_LEN 1024
# include <stdio.h>
# if STDC_HEADERS || HAVE_STRING_H
#  include <string.h>
# else
#  include <strings.h>
# endif

int
main ()
{
  char buff[MAX_BUFF_LEN + 1];

  buff[MAX_BUFF_LEN] = 0;
  while (fgets (buff, MAX_BUFF_LEN, stdin) && buff[0])
    {
      char path[MAX_BUFF_LEN];
      char expected_result[MAX_BUFF_LEN];
      char const *result;
      sscanf (buff, "%s %s", path, expected_result);
      result = dir_name (path);
      if (strcmp (result, expected_result))
	printf ("%s: got %s, expected %s\n", path, result, expected_result);
    }
  return 0;
}
#endif