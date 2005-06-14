/* Copyright (C) 2005
 *   Written by Keith Marshall (keithmarshall@users.sourceforge.net)
 *
 * Provides:
 *   int win32run_command_sequence( char *command );
 *
 *     This function may be used in place of a system() function call,
 *     on Win32 platforms; it uses a UNIXy shell, if one is available,
 *     to run "command", rather than "cmd.exe", as used by system().
 *     If no UNIXy shell is available, the call fails, returning -1.
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
#include "winexec.h"

#include <stdio.h>
#include <stdlib.h>

/* If we ever try to compile this module on a non_WIN32 system,
 * we will most likely find that _P_WAIT is not defined, (because
 * we do not have process.h).
 */

#ifndef _P_WAIT
# define _P_WAIT 0
#endif

#define NO_SHELL -1

/* win32run_command_sequence(): used in place of a system() call,
 * to invoke the appropriate sequence of manpage formatting commands.
 */

int win32run_command_sequence( const char *commands )
{
  static char *shell = NULL;

  /* We need a UNIXy shell, to execute the command pipeline.
   * If we didn't previously identify one which we can deploy,
   * then first check the environment for a candidate.
   */

  if( shell == NULL )
    shell = getenv( "SHELL" );

  if( shell )
    return spawnlp( _P_WAIT, shell, shell, "-c", commands, NULL );

  return NO_SHELL;
}

/* $Source: /srv/git/msys/cvs-mirror/man/src/win32/wincmds.c,v $: end of file */
