/* Copyright (C) 2005
 *   Written by Keith Marshall (keithmarshall@users.sourceforge.net)
 *
 * Provides:
 *   char* win32path_transform( const char* pathname );
 *
 *     This function replaces all occurrences of the "\" character,
 *     in the specified Win32 "pathname", with the "/" character.
 *
 *   int win32path_is_absolute( const char* pathname );
 *
 *     This function determines if the specified Win32 "pathname"
 *     represents an absolute location in the Win32 file system.
 *     (Note that "pathname" is transformed in-situ, according to
 *      the algorithm employed by win32path_transform).
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY of any kind; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Please refer
 * to the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin St - Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "compat.h"

/* win32path_transform(): fix up Win32 path names, so that they
 * conform to the POSIX convention of using "/" as the directory separator,
 * (but leave "D:" as a drive designator, if it is present).
 */

char *win32path_transform (const char *pathname)
{
  char *p = (char *)pathname;

  if ( p )
    do if (*p == '\\') *p = '/';
      while (*p++);
  return (char *) pathname;
}

/* win32path_is_absolute(): check if a specified Win32 path,
 * which may, or may not have been transformed by win32path_transform(),
 * represents an absolute reference to a file system location.
 */

int win32path_is_absolute (const char *pathname)
{
  pathname = win32path_transform (pathname);
  return (*pathname == '/') || ((pathname[1] == ':') && (pathname[2] == '/'));
}

/* $Source: /srv/git/msys/cvs-mirror/man/src/win32/winposix.c,v $: end of file */
