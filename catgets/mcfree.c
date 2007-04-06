/*
 * mcfree.c
 *
 * $Id: mcfree.c,v 1.1 2007-04-06 22:34:52 keithmarshall Exp $
 *
 * Copyright (C) 2006, Keith Marshall
 *
 * This file implements the `_mc_free_' function, which is used to close
 * a message catalogue, which was opened by the `catopen' function, and to
 * release the resources which had been allocated to it.
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

#include <stdlib.h>
#include <unistd.h>
#include <nl_types.h>

#include <msgcat.h>
#include <mctab.h>

int _mc_free_( struct mc_descriptor *mc )
{
  /* Close the file associated with the message catalogue descriptor,
   * saving the return status, for use as our own return code.
   */
  int retval = close( mc->fd );

  /* Free the memory used for the message catalogue data buffer.
   */
  if( mc->data != NULL )
  {
    free( mc->data );
    mc->data = NULL;
  }

  /* Mark the message catalogue descriptor as unused, and we're done.
   */
  mc->fd = -1;
  return retval;
}

/* $RCSfile: mcfree.c,v $Revision: 1.1 $: end of file */
