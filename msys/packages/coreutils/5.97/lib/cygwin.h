/* cygwin.h - helper functions unique to Cygwin

   Copyright (C) 2005 Free Software Foundation, Inc.

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

#ifndef CYGWIN_H
# define CYGWIN_H 1

int cygwin_spelling (char const *);

/* Append ".exe" to char *__NAME; __name will be evaluated more than once, and
   relies on cygwin's PATH_MAX (256) being less than the stack size.  */
#define CYGWIN_APPEND_EXE(__name) \
  __name = strcat (strcpy (malloc (strlen (__name) + 5), __name), ".exe")
#if (__CYGWIN__ || __MSYS__) && 1024 < PATH_MAX
# error The cygwin distribution has increased PATH_MAX; revisit this file.
#endif

#endif /* CYGWIN_H */
