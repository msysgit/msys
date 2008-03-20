/* Copyright (C) 2008
 *   Written by Keith Marshall (keithmarshall@users.sourceforge.net)
 *
 * Provides:
 *   void win32_iso639_default( const char *varname );
 *
 *     This function may be used to establish the ISO-639 language
 *     code corresponding to the user's default locale, placing the
 *     returned value into the environment as an assignment for the
 *     variable specified by `varname'; the function performs no
 *     action, if `varname' has previously been assigned.
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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void win32_iso639_default( const char *varname )
{
  /* Assign the ISO-639 language identifier code, representing the
   * user's default locale, to the environment variable specified by
   * `varname', provided that variable has no prior assignment.
   */
  if( getenv( varname ) == NULL )
  {
    /* The specified environment variable hasn't been defined...
     * Allocate a buffer, sufficiently large to assemble an assignment
     * for it, with the appropriate ISO-639 language code in place.
     */
    size_t bufreq = strlen( varname ) + 10;
    char buf[ bufreq ], *value = buf + bufreq - 9;

    /* Use the Win32 API to retrieve the ISO-639 language code for the
     * user's default locale...
     */
    if( GetLocaleInfo( GetUserDefaultLCID(), LOCALE_SISO639LANGNAME, value, 9 )
    /*
     * and assemble it into the required format for assignment to the
     * specified environment variable...
     */
    &&  sprintf( buf, "%s=%s", varname, value )  )
      /*
       * if successfully assembled, add it to the environment.
       */
      putenv( buf );
  }
}

/* $RCSfile: winlang.c,v $: end of file */
