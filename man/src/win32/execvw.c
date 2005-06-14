/* Copyright (C) 2004, 2005
     Written by Keith Marshall (keith.d.marshall@ntlworld.com)

This file implements a generic wrapper for any of the 'execv...' or
'spawnv...' functions provided in the MSVC runtime library; it corrects
a defect, shared by all of these functions, whereby the passed arguments
are incorrectly interpreted in the child process.  Compile it for each
individual function required, using:

        gcc -c -DFUNCTION=<name> -o <name>w.o execvw.c

where <name> is the actual name of the 'execv...' or 'spawnv...' function
to be referenced;  (each wrapper function must be compiled individually).

This is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

This software is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY of any kind; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Please refer to
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this software; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin St - Fifth Floor, Boston, MA 02110-1301,
USA. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#define EXEC_FUNCTION_WRAPPERS   1
#define SPAWN_FUNCTION_WRAPPERS  1

#include "winexec.h"

/* specify the wrapper function we are constructing
 * and the associated MSVC function wrapped */

#define PASTE(A,B)         A##B
#define WRAPPER(FUNCTION)  PASTE(FUNCTION,_wrapper)
#define WRAPPED(FUNCTION)  PASTE(_,FUNCTION)

/* make a wrapper for 'execv' by default
 * (override this by specifying '-DFUNCTION=name' on command line
 *  with 'execv', 'execve', 'execvp', 'execvpe', 'spawnv', 'spawnve',
 *  'spawnvp' and 'spawnvpe' valid as 'name') */

#ifndef FUNCTION
#define FUNCTION  execv
#endif

/* specify the argument list declarations
 * for each of the wrapper functions */

#define ARGDECL(FUNCTION)  PASTE(ARGDECL_,FUNCTION)

/* first for the 'exec' family */

#define ARGDECL_execv      const char* filepath, argv_t argv
#define ARGDECL_execve     const char* filepath, argv_t argv, argv_t envp
#define ARGDECL_execvp     const char* filename, argv_t argv
#define ARGDECL_execvpe    const char* filename, argv_t argv, argv_t envp

/* and then their equivalents in the 'spawn' family */

#define ARGDECL_spawnv     int mode, ARGDECL_execv
#define ARGDECL_spawnve    int mode, ARGDECL_execve
#define ARGDECL_spawnvp    int mode, ARGDECL_execvp
#define ARGDECL_spawnvpe   int mode, ARGDECL_execvpe

/* specify the corresponding sets of arguments
 * to be passed to the wrapped function */

#define USEARGS(FUNCTION)  PASTE(USEARGS_,FUNCTION)

/* again, initially for the 'exec' family */

#define USEARGS_execv      filepath, (argv_t) quoted_argv
#define USEARGS_execve     filepath, (argv_t) quoted_argv, envp
#define USEARGS_execvp     filename, (argv_t) quoted_argv
#define USEARGS_execvpe    filename, (argv_t) quoted_argv, envp

/* and their equivalents in the 'spawn' family */

#define USEARGS_spawnv     mode, USEARGS_execv
#define USEARGS_spawnve    mode, USEARGS_execve
#define USEARGS_spawnvp    mode, USEARGS_execvp
#define USEARGS_spawnvpe   mode, USEARGS_execvpe

extern char** get_quoted_argv( argv_t argv );
extern void purge_quoted_argv( char** argv );

int __cdecl WRAPPER(FUNCTION)( ARGDECL(FUNCTION) )
{
  /* invoke the wrapped MSVC 'exec' or 'spawn' system service
   * enclosing the passed arguments in double quotes, as required,
   * so that the (broken) default parsing in the MSVC runtime doesn't
   * split them at whitespace */

  char** quoted_argv;   /* used to build a quoted local copy of 'argv' */
  int status = -1;      /* initialise return code, in case we fail */

  if( (quoted_argv = get_quoted_argv( argv )) != NULL )
  {
    /* invoke the wrapped MSVC service function
     * passing our now appropriately quoted copy of 'argv' */

    status = WRAPPED(FUNCTION)( USEARGS(FUNCTION) );

    /* clean up our memory allocations
     * for the quoted copy of 'argv', which is no longer required */

    purge_quoted_argv( quoted_argv );
  }

  /* finally ...
   * return the status code returned by 'spawnvp',
   * or a failure code if we fell through */

  return( status );
}

/* execvw.c: end of file */
