/*
 * mctab.c
 *
 * $Id: mctab.c,v 1.1 2007-04-06 22:34:52 keithmarshall Exp $
 *
 * Copyright (C) 2006, Keith Marshall
 *
 * This file defines the global message catalogue descriptor table accessor
 * hook, required to support the MinGW implementation of the POSIX compatible
 * `catgets' internationalisation function.
 *
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last modification: 07-Dec-2006
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

#include <stdio.h>
#include <stdarg.h>
#include <mctab.h>

/*
 * The catalogue descriptor table is initialised with no entry slots;
 * new slots will be allocated on demand, to satisfy `catopen' requests.
 * Thus, the initial slot count is set to zero, and the allocation pointer
 * is set to `NULL'.
 *
 * When `catopen' requires that the descriptor table be expanded, the slot
 * count is incremented, and a `realloc' request is issued, to allocate the
 * required memory.  The increment size is initialised to allocate one slot
 * per request; while this may seem to be an inefficient memory allocation
 * strategy, it should be remembered that most applications will require
 * only one message catalogue, so the strategy is reasonable.  Should an
 * application require more slots, a mechanism is provided to allow for
 * adjustment of the increment, when making the `catopen' call.
 *
 */

static
struct mc_tab mc_tab = { 0, NL_CATD_BLKSIZ_MIN, NULL };

/*
 * Access to the catalogue descriptor is provided through the `_mctab_'
 * function; the table is manipulated indirectly by the calling function,
 * which must specify a call-back function to operate on the descriptor
 * table via the pointer provided by `_mctab_'.
 *
 * Call-back functions are expected to return a generic pointer type; this
 * returned directly to the calling function, which may then cast it to
 * any required data type.
 *
 */

void *_mctab_( void *function( struct mc_tab *, va_list ), ... )
{
  va_list argv;
  va_start( argv, function );
  return function( &mc_tab, argv );
  va_end( argv );
}

/* $RCSfile: mctab.c,v $Revision: 1.1 $: end of file */
