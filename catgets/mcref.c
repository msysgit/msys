/*
 * mcref.c
 *
 * $Id: mcref.c,v 1.1 2007-04-06 22:34:52 keithmarshall Exp $
 *
 * Copyright (C) 2006, Keith Marshall
 *
 * This file implements the `_mc_select_' function, which is used to map
 * a message catalogue descriptor, of type `nl_catd', to an actual pointer
 * to the message data, as established by calling the `catopen' function,
 * and recorded in the internal message catalogue descriptor table.
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

#include <stdarg.h>
#include <stdlib.h>
#include <nl_types.h>

#include <msgcat.h>
#include <mctab.h>

static
void *do_mc_select( struct mc_tab *cdt, va_list argv )
{
  /* This call-back function provides the actual implementation.
   * It receives a pointer to the catalogue descriptor table, and the
   * caller's catalogue descriptor, from the `_mc_select_' function call,
   * redirected via `_mctab_' to get the `cdt' pointer; it verifies that
   * the `cdt' has been initialised, and that the specified descriptor
   * refers to a valid `cdt' entry, then returns the message data
   * pointer stored in that `cdt' entry, or NULL on failure.
   *
   * Note that, although this call-back function returns a pointer of
   * type `MSGCAT *', it is implemented with a generic `void *' return
   * type declaration, for compatibility with `_mctab_'.
   */
  
  nl_catd catd = va_arg( argv, nl_catd );
  return( cdt->tab && (catd >= 0) && (catd < cdt->curr_size) )
    ? cdt->tab[ catd ].data
    : NULL;
}

MSGCAT *_mc_select_( nl_catd catd )
{
  /* The public interface is a trivial call-back request to the preceding
   * function, through the `_mctab_' redirector.
   */

  return (MSGCAT *)_mctab_( do_mc_select, catd );
}

/* $RCSfile: mcref.c,v $Revision: 1.1 $: end of file */
