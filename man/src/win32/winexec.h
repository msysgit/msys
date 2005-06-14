/* execwrap.h
 *
 * Wrapper functions to work around defective argument passing
 * in MSVCRT _spawn... and _exec... functions.
 * 
 * Copyright (C) 2004, 2005
 *  Written by Keith Marshall <keith.d.marshall@ntlworld.com>
 *
 *  Last update: 14-Jun-2005
 *
 *  This is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE; see the GNU
 *  General Public License for further details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with groff; see the file COPYING.  If not, write to the Free
 *  Software Foundation, 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

#ifndef __STRICT_ANSI__

#ifndef EXECWRAP_H
#define EXECWRAP_H 1

#ifndef __cdecl
#define __cdecl __attribute__((__cdecl__))
#endif

#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

/* Argument parsing in ALL msvcrt.dll implementations of '_exec...()'
 * and '_spawn...()' functions is BROKEN!!!  (argument grouping specified
 * in the caller is NOT preserved in the execed or spawned process, if
 * any single argument contains white space).
 *
 * The following wrapper functions correct this deficiency;
 * (in user code, calls to the native functions are transparently
 *  substituted by calls to the corresponding wrapper).
 */

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef EXEC_FUNCTION_WRAPPERS
#define EXEC_FUNCTION_WRAPPERS 1
/* These defines transparently substitute the wrappers
 * for the native MSVCRT function calls. User code MUST NEVER
 * define 'EXEC_FUNCTION_WRAPPERS'!
 */
# define _execl   execl_wrapper
# define _execle  execle_wrapper
# define _execlp  execlp_wrapper
# define _execlpe execlpe_wrapper
# define _execv   execv_wrapper
# define _execve  execve_wrapper
# define _execvp  execvp_wrapper
# define _execvpe execvpe_wrapper

# ifndef _NO_OLDNAMES
/* It is probably safest to substitute calls
 * using the old function names, as well.
 */
#  define execl   execl_wrapper
#  define execle  execle_wrapper
#  define execlp  execlp_wrapper
#  define execlpe execlpe_wrapper
#  define execv   execv_wrapper
#  define execve  execve_wrapper
#  define execvp  execvp_wrapper
#  define execvpe execvpe_wrapper
# endif
#endif

typedef const char* const* argv_t;

int __cdecl execl_wrapper	( const char*, const char*, ... );
int __cdecl execle_wrapper	( const char*, const char*, ... );
int __cdecl execlp_wrapper	( const char*, const char*, ... );
int __cdecl execlpe_wrapper	( const char*, const char*, ... );
int __cdecl execv_wrapper	( const char*, argv_t );
int __cdecl execve_wrapper	( const char*, argv_t, argv_t );
int __cdecl execvp_wrapper	( const char*, argv_t );
int __cdecl execvpe_wrapper	( const char*, argv_t, argv_t );

#ifndef SPAWN_FUNCTION_WRAPPERS
#define SPAWN_FUNCTION_WRAPPERS 1
/* These defines transparently substitute the wrappers
 * for the native MSVCRT function calls. User code MUST NEVER
 * define 'SPAWN_FUNCTION_WRAPPERS'!
 */
# define _spawnl   spawnl_wrapper
# define _spawnle  spawnle_wrapper
# define _spawnlp  spawnlp_wrapper
# define _spawnlpe spawnlpe_wrapper
# define _spawnv   spawnv_wrapper
# define _spawnve  spawnve_wrapper
# define _spawnvp  spawnvp_wrapper
# define _spawnvpe spawnvpe_wrapper

# ifndef _NO_OLDNAMES
/* It is probably safest to substitute calls
 * using the old function names, as well.
 */
#  define spawnl   spawnl_wrapper
#  define spawnle  spawnle_wrapper
#  define spawnlp  spawnlp_wrapper
#  define spawnlpe spawnlpe_wrapper
#  define spawnv   spawnv_wrapper
#  define spawnve  spawnve_wrapper
#  define spawnvp  spawnvp_wrapper
#  define spawnvpe spawnvpe_wrapper
# endif
#endif

int __cdecl spawnl_wrapper	( int, const char*, const char*, ... );
int __cdecl spawnle_wrapper	( int, const char*, const char*, ... );
int __cdecl spawnlp_wrapper	( int, const char*, const char*, ... );
int __cdecl spawnlpe_wrapper	( int, const char*, const char*, ... );
int __cdecl spawnv_wrapper	( int, const char*, argv_t );
int __cdecl spawnve_wrapper	( int, const char*, argv_t, argv_t );
int __cdecl spawnvp_wrapper	( int, const char*, argv_t );
int __cdecl spawnvpe_wrapper	( int, const char*, argv_t, argv_t );

#ifdef	__cplusplus
}
#endif

#endif	/* EXECWRAP_H not defined */

#endif	/* not __STRICT_ANSI__ */

