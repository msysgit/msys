/*
 * mciconv.c
 *
 * $Id: mciconv.c,v 1.1 2007-04-06 22:34:51 keithmarshall Exp $
 *
 * Copyright (C) 2006, Keith Marshall
 *
 * This file implements the `iconv' wrapper functions used by `gencat',
 * to emulate the `mbtowc' and `wctomb' transformations required to support
 * character codesets other than those of locales installed on the host.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last modification: 28-Dec-2006
 *
 *
 * This is free software.  It is provided AS IS, in the hope that it may
 * be useful, but WITHOUT WARRANTY OF ANY KIND, not even an IMPLIED WARRANTY
 * of MERCHANTABILITY, nor of FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 * Permission is granted to redistribute this software, either "as is" or
 * in modified form, under the terms of the GNU General Public License, as
 * published by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to the
 * Free Software Foundation, 51 Franklin St - Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <iconv.h>
#include <errno.h>

#include <gencat.h>
#include <gcmsgs.h>
#include <debug.h>

char *map_codeset( iconv_t *map, char *codeset, char *internal )
{
  if( ((map[0] = iconv_open( internal, codeset )) == (iconv_t)(-1))
  ||  ((map[1] = iconv_open( codeset, internal )) == (iconv_t)(-1))  )
  {
    if( map[0] != (iconv_t)(-1) )
    {
      iconv_close( map[0] );
      map[0] = (iconv_t)(-1);
    }
    fprintf( errmsg( MSG_UNKNOWN_CODESET ), progname, codeset );
    return NULL;
  }
  return codeset;
}

size_t
iconv_wrap( int mode, iconv_t map, char *in, size_t len, char *out, size_t max )
{
  size_t rtn = (mode ? max : len);

# ifndef ICONV_CONST
# define ICONV_CONST  /* no qualifier required for iconv arg-2 */
# endif
# define ICONV_CAST   ICONV_CONST char **

  if( (iconv( map, (ICONV_CAST)(&in), &len, &out, &max ) == (size_t)(-1))
  &&  ( errno != E2BIG )   )
  {
    perror( "iconv" );
    return (size_t)(0);
  }
  return (size_t)(rtn - (mode ? max : len));
}

/* $RCSfile: mciconv.c,v $Revision: 1.1 $: end of file */
