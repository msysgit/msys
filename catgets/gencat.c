/*
 * gencat.c
 *
 * $Id: gencat.c,v 1.7 2008-01-12 18:01:42 keithmarshall Exp $
 *
 * Copyright (C) 2006, 2007, 2008, Keith Marshall
 *
 * This file implements the `main' function for the `gencat' program.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last modification: 12-Jan-2008
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
#define PROGRAM_IDENTITY	progname, PACKAGE_VERSION

#define AUTHOR_IDENTITY		"Keith Marshall"
#define AUTHOR_ATTRIBUTION	AUTHOR_IDENTITY" for the MinGW Project"

#define COPYRIGHT_YEARS		"2006, 2007, 2008"
#define COPYRIGHT_HOLDER	COPYRIGHT_YEARS, AUTHOR_IDENTITY
#define COPYRIGHT_NOTICE	MSG_COPYRIGHT_NOTICE, COPYRIGHT_HOLDER

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
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

/* Option control flags, for printing of program identification banners,
 * as required for `help', `version' and copyright notifications.
 */
#define print_top_banner	0x08
#define print_nl_duplicate	0x04

/* Standard arguments, to include in a program identification banner.
 */
#define top_banner_text PROGRAM_IDENTITY, AUTHOR_ATTRIBUTION

static
void banner_printf( int opts, int set, int msg, const char *fmt, ... )
{
  /* A helper function, for printing banner notifications.
   */
  va_list args;
  const char *nl_fmt = catgets( gencat_messages, set, msg, NULL );

  if( opts & print_top_banner )
    printf( catgets( gencat_messages, MSG_PROGRAM_BANNER ), top_banner_text );

  va_start( args, fmt );
  if( (nl_fmt == NULL) || ((opts & print_nl_duplicate) != 0) )
    /*
     * A national language translation has been provided, but the
     * English version must also be displayed, or there is no available
     * translation, so we just print the English text anyway.
     */
    vprintf( fmt, args );

  if( nl_fmt != NULL )
    /*
     * When we found a national language translation, we print it.
     */
    vprintf( nl_fmt, args );
  va_end( args );
}

int main( int argc, char **argv )
{
  struct option options[] =
  {
    /* The options understood by this version of `gencat';
     * all are long form, to be evaluated by getopt_long_only().
     */
    { "help",    no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'v' },
    {  NULL,     0,           NULL,  0  }
  };

  char *msgcat;
  char *outfile = NULL;

  int opt, mc, numsets, setnum, msgcount;
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

  if( (*argv = strdup( progname )) != NULL )
  {
    /* Set up the form of the program name to display in diagnostics;
     * exclude the path component, and discard any `.exe' suffix.
     */
    int ext = strlen( *argv ) - 4;
    if( (ext > 0) && (strcasecmp( *argv + ext, ".exe" ) == 0) )
      *(*argv + ext) = '\0';
  }
  else
    /* Something went wrong...
     * retain the basename from the original program path name,
     * as the effective program name.
     */
    *argv = progname;

  /* Evaluate any options, specified on the command line.
   */
  while( (opt = getopt_long_only( argc, argv, "vh", options, NULL )) != -1 )
    switch( opt )
    {
      case 'h':
	/*
	 * This is a request to display a help message.
	 */
	banner_printf( print_top_banner, MSG_GENCAT_SYNOPSIS, *argv );
	exit( EXIT_SUCCESS );

      case 'v':
	/*
	 * And this is for display of version and copyright info.
	 */
	banner_printf( print_top_banner + print_nl_duplicate, COPYRIGHT_NOTICE );
	exit( EXIT_SUCCESS );

      case '?':
	/*
	 * This means the user specified an unrecognised option...
	 * continue parsing, to catch other possible errors, but flag it,
	 * so we can bail out before processing any message catalogue.
	 */
	gencat_errno = opt;
    }

  progname = *argv;
  cat_index.id = tag;

  if( ((argc -= optind) > 1) && (gencat_errno == 0) )
  {
    /* Establish the message catalogue name, recognising `/dev/stdout'
     * as an alias for `-', representing the standard output stream.
     */
    if( strcasecmp( (msgcat = *(argv += optind)), "/dev/stdout" ) == 0 )
      msgcat = "-";

    /* Initialise the message list, to incorporate any messages which
     * are already contained within the specified message catalogue.
     */
    if( (cat = mc_load( msgcat )) == NULL )
      switch( errno )
      {
	case ENOENT:
	  /*
	   * Not an error; the target catalogue simply doesn't yet exist.
	   */
	  dfprintf(( stderr, "ignoring non-existent catalogue %s; errno = %d, ENOENT = %d\n", msgcat, errno, ENOENT ));
	  break;

	default:
	  fprintf( errmsg( MSG_BAD_CATALOGUE ), progname, msgcat );
	  return EXIT_FAILURE;
      }

    /* Merge new or updated message definitions from input files.
     */
    while( --argc )
      cat = mc_merge( cat, mc_source( *++argv ));

    /* Walk the resultant in-memory linked message list, counting...
     */
    msgcount = numsets = setnum = 0;
    for( curr = cat; curr != NULL; curr = curr->link )
    {
      /* the number of individual messages defined...
       */
      ++msgcount;
      if( curr->set > setnum )
      {
	/* and the number of distinct message sets,
	 * to which they are allocated.
	 */
	++numsets;
	setnum = curr->set;
      }
    }
    dfprintf(( stderr, "%u messages in %u sets\n", msgcount, numsets ));

    /* Compute the required image size for the message catalogue index,
     * and allocate memory in which to construct it...
     */
    offset = ( numsets + msgcount ) * sizeof( struct key );
    if( (set_index = mc_malloc( offset )) == NULL )
    {
      /* bailing out, if insufficient memory.
       */
      fprintf( errmsg( MSG_OUT_OF_MEMORY ), progname );
      return( EXIT_FAILURE );
    }
    /* Locate the start of the message index entries,
     * within the composite set and message index image.
     */
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

    /* Seek forward in the temporary output file,
     * to reserve space into which we will later write the message
     * catalogue header record, and the message index.
     */
    offset = lseek( mc, sizeof( MSGCAT ) + offset, SEEK_SET );

    /* Compute the offset, ON DISK, for the start of the message
     * index, within the composite set and message index image, in
     * terms of its physical byte count from start of file.
     */
    msg_index_offset = sizeof( MSGCAT ) + numsets * sizeof( struct key );

    /* Forcing a set number transition into the first message set...
     */
    setnum = 0;
    /*
     * Walk the in-memory linked message list again...
     */
    for( curr = cat; curr != NULL; curr = curr->link )
    {
      /* adding a new set index entry, on each set number transition...
       */
      if( curr->set > setnum )
      {
	/* incorporating the applicable set number,
	 * and the ON-DISK byte offset for the index entry
	 * associated with its first included message.
	 */
	set_index->setnum = setnum = curr->set;
	(set_index++)->offset = msg_index_offset;
      }
      /* Adjust the cumulative computed value of the ON-DISK
       * message index offset, to account for each message traversed...
       */
      msg_index_offset += sizeof( struct key );
      /*
       * while incorporating the appropriate message number,
       * and message data offset, into the in-memory image of
       * the message index.
       */
      msg_index->msgnum = curr->msg;
      (msg_index++)->offset = offset;
      /*
       * and write out the message data, to the catalogue file,
       * updating `offset' to track where the data for the NEXT message,
       * if any, is to be written.
       */
      offset += write( mc, curr->base + curr->loc, curr->len );
    }

    /* Store the effective data size of the catalogue into the header.
     */
    cat_index.mc.extent = offset;

    /* Rewind, and write out the message catalogue header record,
     * at the start of the file space reserved previously.
     */
    lseek( mc, 0L, SEEK_SET );
    write( mc, &cat_index, sizeof( MSGCAT ) );

    /* Locate the start of the in-memory image of the index, once more,
     * and copy it into the remaining reserved space in the output file.
     */
    set_index = msg_index - numsets - msgcount;
    write( mc, set_index, (numsets + msgcount) * sizeof( struct key ) );

    /* Check message catalogue generation status...
     */
    if( gencat_errno == 0 )
    {
      /* Completed without error...
       */
      if( outfile == NULL )
	/*
	 * Always true, at present...
	 * (this is here to accommodate a possible future implementation
	 *  of an `--output-file=NAME' option, to facilitate including an
	 *  existing message catalogue into a new, and differently named,
	 *  derivative message catalogue).
	 */
	outfile = msgcat;

      if( strcmp( outfile, "-" ) == 0 )
      {
	/* This is emitting the message catalogue content to the standard
	 * output stream; rewind the temporary catalogue file, read back,
	 * and then rewrite its content to the stdout stream.
	 */
	int count;
	char buf[ BUFSIZ ];
	lseek( mc, (off_t)(0), SEEK_SET );
	while( (count = read( mc, buf, BUFSIZ )) > 0 )
	  write( STDOUT_FILENO, buf, count );

	/* When done, close and delete the temporary file.
	 */
	close( mc );
	remove( tmpcat );
      }
      else
      {
	/* This is saving the message catalogue as a permanent disk file,
	 * overwriting any previously existing version; this is achieved by
	 * simply renaming the temporary file, and setting its attributes
	 * appropriately.
	 */
	close( mc );
	rename( tmpcat, outfile );
	chmod( outfile, 0644 );
      }
    }
    else
    { /* An error occurred...
       * The temporary file is likely to be corrupt,
       * so simply discard it, and report failure.
       */
      close( mc );
      remove( tmpcat );
      return EXIT_FAILURE;
    }
  }

  else
  { /* An error was detected, while parsing the command line...
     */
    if( argc < 2 )
      /*
       * User specified insufficient command line arguments.
       */
      fprintf( errmsg( MSG_MISSING_ARGS ), progname );

    /* In any case, (it may have been an unrecognised option,
     * which was previously diagnosed), display the usage summary,
     * and bail out.
     */
    fprintf( errmsg( MSG_GENCAT_USAGE ), progname );
    return EXIT_FAILURE;
  }
  /* On successful completion, report it.
   */
  return EXIT_SUCCESS;
}

/* $RCSfile: gencat.c,v $Revision: 1.7 $: end of file */
