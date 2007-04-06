/*
 * catgets.c
 *
 * $Id: catgets.c,v 1.1 2007-04-06 22:34:52 keithmarshall Exp $
 *
 * Copyright (C) 2006, Keith Marshall
 *
 * This file implements a POSIX compatible `catgets' function for MinGW.
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

#include <stdlib.h>
#include <nl_types.h>

#include <msgcat.h>
#include <mctab.h>

/* `mkptr' macro facilitates conversion of the file pointer offsets,
 * used to identify the position of data fields in a message catalogue,
 * to actual data pointers of the appropriate type.
 */
#define mkptr( TYPE, BASE, OFFSET )  ((TYPE) (((char *) (BASE)) + (OFFSET)))

/* `MC_SET_INDEX' macros define pointers to the extents of the `set' index,
 * which must be passed to the `index_lookup' function, for retrieval of the
 * offset of the index entries relating to a specified `message set', within
 * the `message' index table.
 */
#define MC_SET_INDEX_1ST(MC) (MC)->mc.index
#define MC_SET_INDEX_END(MC) mkptr( struct key *, (MC), (MC)->mc.index->offset )
#define MC_SET_INDEX_ALL(MC) MC_SET_INDEX_1ST(MC), MC_SET_INDEX_END(MC) - 1

/* `MC_MSG_INDEX' macros perform a similar function, in this case defining
 * the extents within the `message' index table, for all messages contained
 * in a specified `set', as indicated by an `index_lookup' on `MC_SET_INDEX';
 * these `MC_MSG_INDEX' extents are passed to `index_lookup', to retrieve
 * the location of a specified message within the catalogue.
 */
#define MC_MSG_INDEX_1ST(MC,SP) mkptr( struct key *, (MC), (SP)[0].offset )
#define MC_MSG_INDEX_END(MC,SP) mkptr( struct key *, (MC), (SP)[1].offset ) - 1
#define MC_MSG_INDEX_SET(MC,SP) MC_MSG_INDEX_1ST(MC,SP), MC_MSG_INDEX_END(MC,SP)

static
struct key *index_lookup( unsigned refval, struct key *base, struct key *end )
{
  /* Retrieve a pointer to the index entry, within either the set index,
   * or the message index of the message catalogue, for which the set number,
   * or the message number as appropriate, matches refval.
   */

  if( (base == NULL) || (end == NULL) || (end < base) )
    /*
     * The index extents specified are not valid,
     * so we simply bail out.
     */
    return NULL;

  /* Perform a binary search within the entire range of the specified index,
   * reducing the span of the range by fifty percent in each inspection cycle,
   * until we either find the required entry, or the range limits cross over.
   */

  while( end >= base )
  { /*
     * Identify the index subscript to locate the middle entry,
     * within the currently selected subrange of the index.
     */

    unsigned long lookup = (end - base) >> 1;
    
    if( base[ lookup ].keyval == refval )
      /*
       * We found the entry we were looking for,
       * so simply return the current lookup pointer.
       */
      return base + lookup;
    
    else if( base[ lookup ].keyval < refval )
      /*
       * If the current lookup range includes the entry we are seeking,
       * then it must lie within the upper fifty percent subrange bounds,
       * so discard all entries below and including the current lookup,
       * and perform the next lookup cycle on the remainder.
       */
      base += lookup + 1;

    else
      /* Conversely, if we get to here,
       * we need to continue our search in the lower half of the range,
       * so discard all entries above and including the current lookup,
       * before continuing to the next lookup cycle.
       */
      end = base + lookup - 1;
  }

  /* If we fall through the lookup loop,
   * then the entry we wanted isn't listed in the index,
   * so we must return the `not found' result.
   */

  return NULL;
}

char *catgets( nl_catd cat, int setnum, int msgnum, __const char *fallback )
{
  /* Retrieve a pointer to the message designated by `setnum' and `msgnum',
   * within the message catalogue specified by `cat', returning it if found,
   * otherwise simply return the specified pointer to the `fallback' text.
   */

  MSGCAT *mc;
  struct key *refptr;

  /* First verify that the specified message catalogue has been opened,
   * and if so, perform a lookup for the specified `setnum:msgnum' reference,
   * returning a pointer to its text definition, if it is present.
   */

  if( ((mc = _mc_select_( cat )) != NULL)
  &&  ((refptr = index_lookup( setnum, MC_SET_INDEX_ALL( mc ))) != NULL)
  &&  ((refptr = index_lookup( msgnum, MC_MSG_INDEX_SET( mc, refptr ))) != NULL) )
    return mkptr( char *, mc, refptr->offset );

  /* If we get to here, then the message catalogue lookup failed,
   * so we return the `fallback' text.
   */

  return (char *)(fallback);
}

/* $RCSfile: catgets.c,v $Revision: 1.1 $: end of file */
