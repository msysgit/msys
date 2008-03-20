#ifndef COMPAT_H
/*
 * File: compat.h
 * Centralised platform dependency handling for man's C compilation units.
 *
 * Copyright (C) 2005, 2008,
 *   Keith D. Marshall <keithmarshall@users.sourceforge.net>
 *
 * This file is part of the man package.
 *
 * man is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2, or (at your option) any later version.
 *
 * man is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with man; see the file COPYING.  If not, write to the Free Software
 * Foundation, 51 Franklin St - Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#define COMPAT_H

/* Always include config.h, if we have it. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Always prefer strchr() over index(), and strrchr() over rindex(),
 * if we have them.
 */
#ifdef HAVE_STRCHR
# define index  strchr
#endif
#ifdef HAVE_STRRCHR
# define rindex strrchr
#endif

#ifdef __MINGW32__
/*
 * This could probably be better handled by autoconf,
 * or by rationalisation of the code for glob.c itself,
 * but, as a quick and dirty hack for now...
 *
 * On MinGW based platforms, glob.c will not currently compile,
 * unless we define...
 */
# define _POSIX_SOURCE
# define __GNU_LIBRARY__
#endif
#ifdef __CYGWIN__
/*
 * And similarly, for Cygwin...
 * (except that here, we MUST NOT define _POSIX_SOURCE)...
 */
# define __GNU_LIBRARY__

/* Cygwin also requires us to honour the file naming conventions
 * of the Win32 file system.
 */
#define HAVE_WIN32_FILE_SYSTEM  1
#endif

#ifdef _WIN32
/*
 * Handle various nuisances/peculiarities of Microsoft's
 * 32-bit Windows platform.
 *
 * Win32 Path Name Handling
 * ------------------------
 *
 * Here, we must also honour the Win32 file system conventions.
 */
#define HAVE_WIN32_FILE_SYSTEM  1

/* Silently correct the misdemeanours of Win32, and its
 * "dyed in the wool" users who refuse to believe that it is ok
 * to use "/" instead of "\" in path names; however, we must
 * still use ";" as the path separator.
 */
#define PATH_SEPARATOR_CHAR    ';'
#define POSIX_STYLE_PATH(p)    win32_path_transform((p))
#define MAP_POSIX_PATH_NAME(p) win32_map_posix_path_name((p))

extern char *win32_path_transform(const char *);
extern const char *win32_map_posix_path_name(const char *);

/*
 * Because we are forcing the use of "/" as a directory separator,
 * we don't need to check for leading "\" as an absolute path indicator,
 * but we *do* still need to allow for a drive designator.
 */
#define IS_ABSOLUTE_PATH(p) win32_path_is_absolute((p))

extern int win32_path_is_absolute(const char *);

/*
 * Win32 User Identification
 * -------------------------
 *
 * Win32 user model is not consistent with POSIX,
 * so, when we care, just pretend we are running as root...
 */
#define getuid()   0
#define geteuid()  0
#define getgid()   0
#define getegid()  0

/*
 * Running a Command Pipeline on Win32
 * -----------------------------------
 *
 * On POSIXy systems, we do this by simply passing an appropriately
 * formatted command line to the system() function.  On Win32, this
 * implies the use of cmd.exe to run the pipeline, which is a POSIX
 * format command line, and will, therefore, fail; instead we use a
 * replacement function, win32_run_command_sequence(), which spawns
 * a UNIXy shell, to execute the command sequence.
 */
#define RUN_COMMAND_SEQUENCE(cmds)  win32_run_command_sequence((cmds))

extern int win32_run_command_sequence(const char *);

/*
 * Establishing the ISO-639 language code, for selection of national
 * language man pages, and message catalogues.
 */
#define set_iso639_default(VAR)  win32_iso639_default((VAR))

extern void win32_iso639_default(const char *);


#else /* !_WIN32 */
/*
 * POSIX Style Path Name Handling
 * ------------------------------
 *
 * On an already POSIXy system,
 * we don't need any Win32 normalisation of the DIR separator,
 * and we keep to the standard ":" as the path separator.
 */
#define PATH_SEPARATOR_CHAR	':'
#define POSIX_STYLE_PATH(p)	(p)
#define MAP_POSIX_PATH_NAME(p)	(p)

/*
 * POSIX mandates that absolute path names begin with a "/".
 */
#define IS_ABSOLUTE_PATH(p)  (*(p) == '/')

/*
 * Running a POSIX Command Pipeline
 * --------------------------------
 *
 * This is achieved by using a simple system() function call.
 */
#define RUN_COMMAND_SEQUENCE(cmds)  system((cmds))
#endif /* !_WIN32 */

#ifndef HAVE_WIN32_FILE_SYSTEM
/*
 * The code base will use this as a flag, to indicate when
 * Win32 file system semantics prevail; if we didn't set this
 * above, to indicate that such a file system is present, then
 * we must ensure that it is a valid symbol, resolving as a
 * `false' status flag.
 */
#define HAVE_WIN32_FILE_SYSTEM  0
#endif

#ifndef set_iso639_default
/*
 * On most platforms, we don't need any special provision to
 * establish the ISO-639 language code associated with the user's
 * locale; it is implicit within the native locale implementation.
 */
#define set_iso639_default(VAR)  /* do nothing */
#endif

#endif /* COMPAT_H */
