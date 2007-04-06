/*
 * mcmerge.c
 *
 * $Id: mcmerge.c,v 1.1 2007-04-06 22:34:52 keithmarshall Exp $
 *
 * Copyright (C) 2006, Keith Marshall
 *
 * This file implements the `mc_merge' function, which is used by
 * `gencat' to merge the compiled message dictionary derived from
 * any single source file into its current internal dictionary.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last modification: 30-Dec-2006
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
#include <debug.h>

#include <stdio.h>
#include <stdlib.h>

#include <gencat.h>
#include <gcmsgs.h>

struct msgdict *mc_merge( struct msgdict *cat, struct msgdict *input )
{
  struct msgdict *mark = cat;

# ifdef DEBUG
  if( input )
  {
    struct msgdict *curr;
    if( cat == NULL )
    {
      fprintf( stderr, "no existing messages in catalogue; loading %s\n", input->src );
    }
    else
    {
      fprintf( stderr, "merge messages from %s\n", input->src );
      for( curr = cat; curr != NULL; curr = curr->link )
	fprintf( stderr, "%s:%u: set %u message %u\n", curr->src, curr->lineno, curr->set, curr->msg );
    }
    for( curr = input; curr != NULL; curr = curr->link )
      fprintf( stderr, "%s:%u: set %u message %u\n", curr->src, curr->lineno, curr->set, curr->msg );
  }
# endif

  while( input )
  {
    struct msgdict *next = input->link;
    
    dfprintf(( stderr, "Process entry from %s line %u\n", input->src, input->lineno ));
    if( input->base == NULL )
    {
      /* This is a `delete' operation...
       * FIXME: we don't have support for this yet!
       */
      fprintf( errmsg( MSG_DEL_UNSUPPORTED ), progname, input->src, input->lineno );
      free( input );
    }

    else
    {
      /* This is either an `insert' or a `replace' operation...
       * locate the insertion point, within the current message list.
       */

      struct msgdict *curr = mark;
      while( curr && (curr->key < input->key) )
      {
	mark = curr;
	curr = curr->link;
      }

      if( curr == NULL )
      {
	/* This is appending to the end of the message list... */

	if( mark )
	{
	  /* ...extending an existing message list. */

	  input->link = mark->link;
	  mark->link = input;
	  dfprintf(( stderr, "Append set %u message %u after set %u message %u\n",
	               input->set, input->msg, mark->set, mark->msg
	          ));
	}

	else
	{
	  /* There is no current message list; start one now! */

	  cat = mark = input;
	  mark->link = NULL;
	  dfprintf(( stderr, "Initialise message list at set %u message %u\n", mark->set, mark->msg ));
	}
      }

      else if( curr->key == input->key )
      {
	/* This is replacement of an existing message. */

	if( curr->lineno > 0 )
	{
	  /* This a collision...
	   * diagnose, and ignore this redefinition.
	   */

	  fprintf( errmsg( MSG_REDEFINED ), GENCAT_MSG_INPUT, curr->msg, curr->set );
	  fprintf( errmsg( MSG_PREVIOUS_HERE ), GENCAT_MSG_SRC( curr ));
	}

	else
	{
	  dfprintf(( stderr, "Replace set %u message %u\n", curr->set, curr->msg ));

	  if( curr == cat )
	  {
	    /* This is the first message in the current list,
	     * so we simply replace it.
	     */

	    cat = input;
	  }

	  else
	  {
	    /* There are preceding messages in the list,
	     * so we must relink the predecessor to this replacement.
	     */

	    mark->link = input;
	  }

	  input->link = curr->link;
	  free( curr );
	  mark = input;
	}
      }

      else
      {
	/* This is insertion of a new message within the current list,
	 * (maybe preceding all messages which are already in the list).
	 */

	dfprintf(( stderr, "Insert set %u message %u ", input->set, input->msg ));
	if( mark == curr )
	{
	  /* This entry must precede any which is already in the list,
	   * so make it the new initial message.
	   */

	  dfprintf(( stderr, "at start of list " ));
	  cat = mark = input;
	}

	else
	{
	  /* There is already an existing message which must precede this one,
	   * so link this new one to follow the existing predecessor.
	   */

	  dfprintf(( stderr, "after set %u message %u and ", mark->set, mark->msg ));
	  mark->link = input;
	}
	dfprintf(( stderr, "before set %u message %u\n", curr->set, curr->msg ));
	input->link = curr;
      }
    }
    input = next;
  }
# ifdef DEBUG
  if( cat )
  {
    struct msgdict *curr;
    for( curr = cat; curr != NULL; curr = curr->link )
      fprintf( stderr, "%s:%u: set %u message %u\n", curr->src, curr->lineno, curr->set, curr->msg );
  }
# endif
  return cat;
}

/* $RCSfile: mcmerge.c,v $Revision: 1.1 $: end of file */
