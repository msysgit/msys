/*
 * catclose.c
 *
 * $Id: catclose.c,v 1.2 2007-07-16 19:59:42 keithmarshall Exp $
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
#include <errno.h>

static
void *mc_close( struct mc_tab *cdt, va_list argv )
{
  /* This call-back function provides the actual `catclose' implementation.
   * There is one entry in the `argv' vector, representing the descriptor
   * for the catalogue which is to be closed.
   */
  int catd = va_arg( argv, int );
  if( (catd < 0) || (catd >= cdt->curr_size) )
  {
    /* The catalogue descriptor is simply an integer index into an array
     * of pointers to the actual message catalogue data structures.  There
     * are exactly cdt->curr_size elements in this descriptor array, with
     * index values ranging from 0 .. cdt->curr_size - 1; if here, then
     * the value of catd falls outside this range, we decline to accept
     * it, bailing out with errno = EBADF, as specified by POSIX.
     */
    errno = EBADF;
    return (void *)(-1);
  }
  /* If we get past this validity check, then we simply hand off the task
   * of closing the message catalogue, and freeing its resource allocation,
   * to the _mc_free_ function.
   */
  return (void *)_mc_free_( cdt->tab + catd );
}

int catclose( nl_catd catd )
{
  /* All the hard work is done by the `mc_close' call-back from `_mctab_'.
   */
  return (int)_mctab_( mc_close, catd );
}

/* $RCSfile: catclose.c,v $Revision: 1.2 $: end of file */
