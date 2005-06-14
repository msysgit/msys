/* Copyright (C) 2004, 2005
     Written by: Keith Marshall (keith.d.marshall@ntlworld.com)

This file implements a generic wrapper for any of the 'execl...' or
'spawnl...' functions provided in the MSVC runtime library; it corrects
a defect, shared by all of these functions, whereby the passed arguments
are incorrectly interpreted in the child process.  Compile it for each
individual function required, using:

        gcc -c -DFUNCTION=<name> -o <name>w.o execlw.c

where <name> is the actual name of the 'execl...' or 'spawnl...' function
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
 * and the associated interface to the MSVC function wrapped */

#define PASTE(A,B)         A##B
#define WRAPPER(FUNCTION)  PASTE(FUNCTION,_wrapper)
#define WRAPPED(FUNCTION)  PASTE(WRAPPED_,FUNCTION)

#define WRAPPED_execl      execv_wrapper
#define WRAPPED_execle     execve_wrapper
#define WRAPPED_execlp     execvp_wrapper
#define WRAPPED_execlpe    execvpe_wrapper

#define WRAPPED_spawnl     spawnv_wrapper
#define WRAPPED_spawnle    spawnve_wrapper
#define WRAPPED_spawnlp    spawnvp_wrapper
#define WRAPPED_spawnlpe   spawnvpe_wrapper

/* make a wrapper for 'execl' by default
 * (override this by specifying '-DFUNCTION=name' on command line
 *  with 'execl', 'execle', 'execlp', 'execlpe', 'spawnl', 'spawnle',
 *  'spawnlp' and 'spawnlpe' valid as 'name') */

#ifndef FUNCTION
#define FUNCTION  execl
#endif

/* specify the argument list declarations
 * for each of the wrapper functions */

#define ARGDECL(FUNCTION)  PASTE(ARGDECL_,FUNCTION)

/* first for the 'exec' family */

#define ARGDECL_execl      const char* filepath, const char* args, ...
#define ARGDECL_execle     const char* filepath, const char* args, ...
#define ARGDECL_execlp     const char* filename, const char* args, ...
#define ARGDECL_execlpe    const char* filename, const char* args, ...

/* and then their equivalents in the 'spawn' family */

#define ARGDECL_spawnl     int mode, ARGDECL_execl
#define ARGDECL_spawnle    int mode, ARGDECL_execle
#define ARGDECL_spawnlp    int mode, ARGDECL_execlp
#define ARGDECL_spawnlpe   int mode, ARGDECL_execlpe

/* specify the corresponding sets of arguments
 * to be passed to the wrapped function */

#define USEARGS(FUNCTION)  PASTE(USEARGS_,FUNCTION)
#define ARGV               (const char* const*) &args

/* again, initially for the 'exec' family */

#define USEARGS_execl      filepath, ARGV
#define USEARGS_execle     filepath, ARGV, execl_wrapper_getenvp( ARGV )
#define USEARGS_execlp     filename, ARGV
#define USEARGS_execlpe    filename, ARGV, execl_wrapper_getenvp( ARGV )

/* and their equivalents in the 'spawn' family */

#define USEARGS_spawnl     mode, USEARGS_execl
#define USEARGS_spawnle    mode, USEARGS_execle
#define USEARGS_spawnlp    mode, USEARGS_execlp
#define USEARGS_spawnlpe   mode, USEARGS_execlpe

extern argv_t execl_wrapper_getenvp( argv_t argv );

int __cdecl WRAPPER(FUNCTION)( ARGDECL(FUNCTION) )
{
  /* invoke the wrapped MSVC 'exec' or 'spawn' system service
   * enclosing the passed arguments in double quotes, as required,
   * so that the (broken) default parsing in the MSVC runtime doesn't
   * split them at whitespace */

    return( WRAPPED(FUNCTION)( USEARGS(FUNCTION) ));
}

/* execlw.c: end of file */
