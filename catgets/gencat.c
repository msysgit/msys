/*
 * gencat.c
 *
 * $Id: gencat.c,v 1.1 2007-04-06 22:34:51 keithmarshall Exp $
 *
 * Copyright (C) 2006, 2007, Keith Marshall
 *
 * This file implements the `main' function for the `gencat' program.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last modification: 05-Mar-2007
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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>

char *progname, *invoked;

#include <gencat.h>
#include <gcmsgs.h>
#include <msgcat.h>
#include <mctab.h>

#include <platform.h>
#include <debug.h>

int gencat_errno = 0;
nl_catd gencat_messages;

int main( int argc, char **argv )
{
  char *msgcat;
  char *outfile = NULL;

  int mc, numsets, setnum, msgcount;
  off_t offset, msg_index_offset;

  char tmpcat[] = "mcXXXXXX";
  struct ident tag = { mk_msgcat_tag( MINGW32_CATGETS )};
  MSGCAT cat_index; struct key *set_index, *msg_index;

  struct msgdict *cat, *curr;

# ifdef _O_BINARY
  /*
   * This is specific to the Win32 platform;
   * we must ensure that all file I/O operations are performed
   * in BINARY translation mode.
   *
   */
  _fmode = _O_BINARY;
  _setmode( _fileno( stdin ), _O_BINARY );
  _setmode( _fileno( stdout ), _O_BINARY );
# endif

  progname = basename( invoked = *argv );
  gencat_messages = catopen( "gencat", NL_CAT_LOCALE );

  cat_index.id = tag;

  if( --argc > 1 )
  {
    /* Initialise the message list, to incorporate any messages which
     * are already contained within the specified message catalogue.
     */

    if( (cat = mc_load( msgcat = *++argv )) == NULL )
      switch( errno )
      {
	case ENOENT:
	  /*
	   * Not an error; the target catalogue simply doesn't yet exist.
	   */
	  dfprintf(( stderr, "ignoring non-existent catalogue %s; errno = %d, ENOENT = %d\n", msgcat, errno, ENOENT ));
	  break;

	default:
	  fprintf( errmsg( MSG_CATLOAD_FAILED ), progname, msgcat );
	  return EXIT_FAILURE;
      }

    /* Merge new or updated message definitions from input files.
     */

    while( --argc )
      cat = mc_merge( cat, mc_source( *++argv ));

    msgcount = numsets = setnum = 0;
    for( curr = cat; curr != NULL; curr = curr->link )
    {
      ++msgcount;
      if( curr->set > setnum )
      {
	++numsets;
	setnum = curr->set;
      }
    }
    dfprintf(( stderr, "%u messages in %u sets\n", msgcount, numsets ));
    offset = ( numsets + msgcount ) * sizeof( struct key );
    if( (set_index = mc_malloc( offset )) == NULL )
    {
      fprintf( errmsg( MSG_OUT_OF_MEMORY ), progname );
      return( EXIT_FAILURE );
    }
    msg_index = set_index + numsets;

    /* Create a temporary output file,
     * which will eventually become the generated message catalogue,
     * or will be copied to stdout, as appropriate...
     */
    if( (mc = mkstemp( tmpcat )) < 0 )
    {
      /* ...diagnosing, and bailing out if we can't. */

      fprintf( errmsg( MSG_OUTPUT_NOFILE ), progname, tmpcat );
      return EXIT_FAILURE;
    }

    write( mc, &cat_index, sizeof( MSGCAT ) );
    offset = lseek( mc, offset, SEEK_CUR );
    setnum = 0;
    msg_index_offset = sizeof( MSGCAT ) + numsets * sizeof( struct key );
    for( curr = cat; curr != NULL; curr = curr->link )
    {
      if( curr->set > setnum )
      {
	set_index->setnum = setnum = curr->set;
	(set_index++)->offset = msg_index_offset;
      }
      msg_index_offset += sizeof( struct key );
      msg_index->msgnum = curr->msg;
      (msg_index++)->offset = offset;

      offset += write( mc, curr->base + curr->loc, curr->len );
    }

    set_index = msg_index - numsets - msgcount;
    lseek( mc, sizeof( MSGCAT ), SEEK_SET );
    write( mc, set_index, (numsets + msgcount) * sizeof( struct key ) );

    if( gencat_errno == 0 )
    {
      if( outfile == NULL )
	outfile = msgcat;

      if( strcmp( outfile, "-" ) == 0 )
      {
	int count;
	char buf[ BUFSIZ ];
	lseek( mc, (off_t)(0), SEEK_SET );
	while( (count = read( mc, buf, BUFSIZ )) > 0 )
	  write( STDOUT_FILENO, buf, count );
	close( mc );
	remove( tmpcat );
      }
      else
      {
	close( mc );
	rename( tmpcat, outfile );
	chmod( outfile, 0644 );
      }
    }
    else
    {
      close( mc );
      remove( tmpcat );
    }
  }

  else
  {
    /* User specified insufficient command line arguments.
     * Diagnose, and bail out.
     */

    fprintf( errmsg( MSG_MISSING_ARGS ), progname );
    fprintf( errmsg( MSG_GENCAT_USAGE ), progname );
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/* $RCSfile: gencat.c,v $Revision: 1.1 $: end of file */
