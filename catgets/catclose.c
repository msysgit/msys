/*
 * catclose.c
 *
 * $Id: catclose.c,v 1.1 2007-04-06 22:34:52 keithmarshall Exp $
 *
 * Copyright (C) 2006, Keith Marshall
 *
 * This file implements the `catclose' function, required to support
 * POSIX compatible national language message catalogues in MinGW.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last modification: 09-Dec-2006
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

#include <stdarg.h>
#include <mctab.h>

static
void *mc_close( struct mc_tab *cdt, va_list argv )
{
  /* This call-back function provides the actual `catclose' implementation.
   */
  return (void *)_mc_free_( cdt->tab + va_arg( argv, int ) );
}

int catclose( nl_catd catd )
{
  /* All the hard work is done by the `mc_close' call-back from `_mctab_'.
   */
  return (int)_mctab_( mc_close, catd );
}

/* $RCSfile: catclose.c,v $Revision: 1.1 $: end of file */
