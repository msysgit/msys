/*
 * mcload.c
 *
 * $Id: mcload.c,v 1.1 2007-04-06 22:34:51 keithmarshall Exp $
 *
 * Copyright (C) 2006, Keith Marshall
 *
 * This file implements the `mc_load' function, which is used by `gencat'
 * to load the messages from an existing message catalogue, reconstructing
 * the original internal dictionary structure from which it was compiled.
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

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include <gencat.h>
#include <msgcat.h>
#include <mctab.h>

#include <platform.h>

#define mkptr(TYPE,PTR,OFFSET) (TYPE)((char *)(PTR) + (OFFSET))

struct msgdict *mc_load( const char *name )
{
  int fd;
  struct stat catinfo;
  struct msgdict *retptr = NULL;

  errno = 0;
  if( ((fd = mc_validate( name )) >= 0)
  &&  ((fstat( fd, &catinfo ) == 0))
  &&  ((catinfo.st_size > MINGW32_CATGETS_ST_SIZE_MIN))
  &&  ((retptr = mc_malloc( sizeof( struct msgdict ))) != NULL)
  &&  ((retptr->base = mc_malloc( catinfo.st_size )) != NULL)
  &&  ((read( fd, retptr->base, catinfo.st_size ) == catinfo.st_size))  )
  {
    struct msgdict *this = retptr;
    struct key *setptr = ((MSGCAT *)(this->base))->mc.index;
    int numsets = mkptr( struct key *, this->base, setptr->offset ) - setptr;
    struct key *msgptr = setptr + numsets;

    /* All initial message dictionary entries have `lineno' field set to zero,
     * indicating their source to be the named message catalogue being updated.
     */

    retptr->lineno = 0;

    /* Step through all message sets listed in the index...
     */

    while( numsets-- )
    {
      int msgcount;

      this->set = setptr++->setnum;
      msgcount = mkptr( struct key *, this->base, setptr->offset ) - msgptr;
      while( msgcount-- )
      {
	this->msg = msgptr->msgnum;
	this->loc = msgptr++->offset;
	if( numsets | msgcount )
	{
	  if( (this->link = mc_malloc( sizeof( struct msgdict ))) == NULL )
	  {
	    for( this = retptr; this != NULL; this = retptr )
	    {
	      retptr = this->link;
	      if( (retptr == NULL) && (this->base != NULL) )
		free( this->base );
	      free( this );
	    }
	    return NULL;
	  }
	  *(this->link) = *this;
	  this->len = msgptr->offset - this->loc;
	  this = this->link;
	}
	else
	{
	  this->len = catinfo.st_size - this->loc;
	  this->link = NULL;
	}
      }
    }
    close( fd );
  }

  else if( retptr != NULL )
  {
    if( retptr->base != NULL )
      free( retptr->base );
    free( retptr );
    retptr = NULL;
  }

  return retptr;
}

/* $RCSfile: mcload.c,v $Revision: 1.1 $: end of file */
