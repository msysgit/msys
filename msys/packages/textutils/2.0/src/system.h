/* system-dependent definitions for fileutils, textutils, and sh-utils packages.
   Copyright (C) 89, 1991-1998, 1999 Free Software Foundation, Inc.

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

/* Include sys/types.h before this file.  */

#include <sys/stat.h>

#ifdef STAT_MACROS_BROKEN
# undef S_ISBLK
# undef S_ISCHR
# undef S_ISDIR
# undef S_ISFIFO
# undef S_ISLNK
# undef S_ISMPB
# undef S_ISMPC
# undef S_ISNWK
# undef S_ISREG
# undef S_ISSOCK
#endif /* STAT_MACROS_BROKEN.  */

#ifndef S_IFMT
# define S_IFMT 0170000
#endif
#if !defined(S_ISBLK) && defined(S_IFBLK)
# define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#endif
#if !defined(S_ISCHR) && defined(S_IFCHR)
# define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#endif
#if !defined(S_ISDIR) && defined(S_IFDIR)
# define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISREG) && defined(S_IFREG)
# define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISFIFO) && defined(S_IFIFO)
# define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#endif
#if !defined(S_ISLNK) && defined(S_IFLNK)
# define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif
#if !defined(S_ISSOCK) && defined(S_IFSOCK)
# define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#endif
#if !defined(S_ISMPB) && defined(S_IFMPB) /* V7 */
# define S_ISMPB(m) (((m) & S_IFMT) == S_IFMPB)
# define S_ISMPC(m) (((m) & S_IFMT) == S_IFMPC)
#endif
#if !defined(S_ISNWK) && defined(S_IFNWK) /* HP/UX */
# define S_ISNWK(m) (((m) & S_IFMT) == S_IFNWK)
#endif

#ifndef S_IEXEC
# define S_IEXEC S_IXUSR
#endif

#ifndef S_IXUSR
# define S_IXUSR S_IEXEC
#endif
#ifndef S_IXGRP
# define S_IXGRP (S_IEXEC >> 3)
#endif
#ifndef S_IXOTH
# define S_IXOTH (S_IEXEC >> 6)
#endif
#ifndef S_IXUGO
# define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

#if !defined(HAVE_MKFIFO)
# define mkfifo(path, mode) (mknod ((path), (mode) | S_IFIFO, 0))
#endif

#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

/* <unistd.h> should be included before any preprocessor test
   of _POSIX_VERSION.  */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifndef STDIN_FILENO
# define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
# define STDERR_FILENO 2
#endif


#if HAVE_LIMITS_H
/* limits.h must come before pathmax.h because limits.h on some systems
   undefs PATH_MAX, whereas pathmax.h sets PATH_MAX.  */
# include <limits.h>
#endif

/* Don't use bcopy!  Use memmove if source and destination may overlap,
   memcpy otherwise.  */

#ifdef HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#else
# include <strings.h>
#endif

#include <errno.h>
#ifndef errno
extern int errno;
#endif

#if HAVE_STDLIB_H
# define getopt system_getopt
# include <stdlib.h>
# undef getopt
#endif

/* The following test is to work around the gross typo in
   systems like Sony NEWS-OS Release 4.0C, whereby EXIT_FAILURE
   is defined to 0, not 1.  */
#if !EXIT_FAILURE
# undef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#else
# include <sys/file.h>
#endif

#if !defined (SEEK_SET)
# define SEEK_SET 0
# define SEEK_CUR 1
# define SEEK_END 2
#endif
#ifndef F_OK
# define F_OK 0
# define X_OK 1
# define W_OK 2
# define R_OK 4
#endif

/* For systems that distinguish between text and binary I/O.
   O_BINARY is usually declared in fcntl.h  */
#if !defined O_BINARY && defined _O_BINARY
  /* For MSC-compatible compilers.  */
# define O_BINARY _O_BINARY
# define O_TEXT _O_TEXT
#endif
#if O_BINARY
# ifndef __DJGPP__
#  define setmode _setmode
#  define fileno(_fp) _fileno (_fp)
# endif /* not DJGPP */
# define SET_BINARY(_f) do {if (!isatty(_f)) setmode (_f, O_BINARY);} while (0)
# define SET_BINARY2(_f1, _f2)		\
  do {					\
    if (!isatty (_f1))			\
      {					\
        setmode (_f1, O_BINARY);	\
	if (!isatty (_f2))		\
	  setmode (_f2, O_BINARY);	\
      }					\
  } while(0)
#else
# define SET_BINARY(f) (void)0
# define SET_BINARY2(f1,f2) (void)0
# define O_BINARY 0
# define O_TEXT 0
#endif /* O_BINARY */

#ifdef HAVE_DIRENT_H
# include <dirent.h>
# define NLENGTH(direct) (strlen((direct)->d_name))
#else /* not HAVE_DIRENT_H */
# define dirent direct
# define NLENGTH(direct) ((direct)->d_namlen)
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif /* HAVE_SYS_NDIR_H */
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif /* HAVE_SYS_DIR_H */
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif /* HAVE_NDIR_H */
#endif /* HAVE_DIRENT_H */

#ifdef CLOSEDIR_VOID
/* Fake a return value. */
# define CLOSEDIR(d) (closedir (d), 0)
#else
# define CLOSEDIR(d) closedir (d)
#endif

/* Get or fake the disk device blocksize.
   Usually defined by sys/param.h (if at all).  */
#if !defined(DEV_BSIZE) && defined(BSIZE)
# define DEV_BSIZE BSIZE
#endif
#if !defined(DEV_BSIZE) && defined(BBSIZE) /* SGI */
# define DEV_BSIZE BBSIZE
#endif
#ifndef DEV_BSIZE
# define DEV_BSIZE 4096
#endif

/* Extract or fake data from a `struct stat'.
   ST_BLKSIZE: Optimal I/O blocksize for the file, in bytes. */
#ifndef HAVE_ST_BLKSIZE
# define ST_BLKSIZE(statbuf) DEV_BSIZE
#else /* HAVE_ST_BLKSIZE */
/* Some systems, like Sequents, return st_blksize of 0 on pipes. */
# define ST_BLKSIZE(statbuf) ((statbuf).st_blksize > 0 \
			       ? (statbuf).st_blksize : DEV_BSIZE)
#endif /* HAVE_ST_BLKSIZE */

#include "sys2.h"
