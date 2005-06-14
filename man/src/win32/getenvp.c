/* Copyright (C) 2004, 2005
     Written by Keith Marshall (keith.d.marshall@ntlworld.com)

This file implements the 'execl_wrapper_getenvp()' function, required
by the 'execlw.c' implementation of wrappers for the MSVC runtime library
'execle()', 'execlpe()', 'spawnle()', and 'spawnlpe()' functions.

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

const char* const*
execl_wrapper_getenvp( const char* const* argv )
{
  /* step over the 'argv' arguments in an 'execle',
   * 'execlpe', 'spawnle', or 'spawnlpe' wrapper function call,
   * to locate the following 'envp' argument.
   */

  if( argv )
    while( *argv++ )
      ;
  return( argv );
}

/* getenvp.c: end of file */
