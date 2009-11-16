/*
 * mkpath.c
 *
 * $Id: mkpath.c,v 1.1 2009-11-16 21:54:30 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Helper functions for constructing path names, creating directory
 * hierarchies, and preparing to write new files within any specified
 * file system hierarchy.
 *
 *
 * This is free software.  Permission is granted to copy, modify and
 * redistribute this software, under the provisions of the GNU General
 * Public License, Version 3, (or, at your option, any later version),
 * as published by the Free Software Foundation; see the file COPYING
 * for licensing details.
 *
 * Note, in particular, that this software is provided "as is", in the
 * hope that it may prove useful, but WITHOUT WARRANTY OF ANY KIND; not
 * even an implied WARRANTY OF MERCHANTABILITY, nor of FITNESS FOR ANY
 * PARTICULAR PURPOSE.  Under no circumstances will the author, or the
 * MinGW Project, accept liability for any damages, however caused,
 * arising from the use of this software.
 *
 */
#include "mkpath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
 /*
  * MS-Windows nuisances...
  * mkdir() function doesn't accept a `mode' argument; ignore it.
  */
# define mkdir( PATH, MODE )  _mkdir( PATH )
 /*
  * MS-Windows _O_BINARY vs. _O_TEXT discrimination can't be explicitly
  * resolved in a simple `creat()' call; instead, we will use `_open()',
  * with the following explicit attribute set...
  */
# define _O_NEWFILE  _O_RDWR | _O_CREAT | _O_TRUNC | _O_BINARY
# define creat(P,M)  _open( P, _O_NEWFILE, _map_posix_mode(M) )

 /* Furthermore, MS-Windows protection modes are naive, in comparison
  * to the POSIX modes specified within tar archive headers; firstly,
  * there is no concept of `execute' permissions; secondly, there is
  * no concept of a `write-only' file, (thus _S_IREAD permission is
  * implicitly granted for all files); thirdly, `write' permission is
  * granted only by a single _S_IWRITE flag, so at best, we must set
  * this if any of POSIX's S_IWUSR, S_IWGRP or S_IWOTH flags are set
  * in the tar header; finally, MS-Windows has no counterpart for any
  * of POSIX's `suid', `sgid' or `sticky' bits.
  */
# define _S_IWANY  0222  /* eqv. POSIX's S_IWUSR | S_IWGRP | S_IWOTH */
# define _map_posix_mode(M)  _S_IREAD | (((M) & _S_IWANY) ? _S_IWRITE : 0)

#endif

int mkpath( char *buf, const char *fmt, const char *file, const char *modifier )
{
  /* A helper function, for constructing package URL strings.
   * Return value is the length, in bytes, of the constructed string,
   * which is returned in "buf"; call with "buf = NULL" to determine
   * the size of buffer required, without storing the string.
   *
   * Constructed URL is copied from "fmt", with...
   *
   *   %%     replaced by a single literal "%" character;
   *   %[/]F  replaced by the string passed as "file";
   *   %[/]M  replaced by the string passed as "modifier";
   *   %[/]R  replaced by the "APPROOT" environment string.
   *
   * Any other character present in "fmt" is copied literally.  In
   * the case of "%F", "%M" and "%R", inclusion of the optional "/"
   * flag causes a single "/" character to be inserted before the
   * substitute string, provided this is not NULL; ("\\" may be
   * used, but is not recommended, in place of the "/" flag).
   */
  char c;
  int len = 0;

  /* Scan "fmt"...
   */
  do { if( (c = *fmt++) == '%' )
       {
	 /* Interpret substitution tags...
	  */
	 char flag = *fmt;
	 const char *subst = NULL;

	 /* ...checking for presence of a "/" flag...
	  */
	 if( ((flag == '/') || (flag == '\\')) && fmt[1] )
	   /*
	    * ...and, when found, with a possibly valid format spec,
	    * advance to parse that spec...
	    */
	   ++fmt;

	 else
	   /* ...establish absence of the flag.
	    */
	   flag = '\0';

	 switch( c = *fmt++ )
	 {
	   case 'F':
	     /*
	      * Schedule substitution of text specified as "file".
	      */
	     subst = file;
	     break;

	   case 'M':
	     /*
	      * Schedule substitution of text specified as "modifier",
	      */
	     subst = modifier;
	     break;

	   case 'R':
	     /*
	      * Schedule substitution from the "APPROOT" environment string,
	      */
	     subst = getenv( "APPROOT" );
	     break;

	   case '%':
	     /*
	      * Interpreting "%%", but may have been "%/%", which is invalid...
	      */
	     if( flag == '\0' ) 
	     {
	       /*
	        * It was just "%%", so store a literal "%" character.
	        */
	       if( buf != NULL )
		 *buf++ = '%';
	       ++len;
	       break;
	     }

	     /* If we get to here, it was the invalid "%/%" form; backtrack,
	      * and fall through to emit literal "%/", then resume parsing,
	      * treating the second "%" as the possible starting character
	      * of a new format specification.
	      */
	     c = flag;
	     --fmt;

	   default:
	     if( buf != NULL )
	     {
	       /* Store the literal "%" character,
		* followed by the unrecognised tag character.
		*/
	       *buf++ = '%';
	       *buf++ = c;
	     }
	     len += 2;
	 }

	 if( subst != NULL )
	 {
	   /* Perform scheduled substitution of "file", "modifier"
	    * or the APPROOT environment string...
	    */
	   if( flag )
	   {
	     ++len;
	     if( buf != NULL )
	       *buf++ = flag;
	   }
	   while( *subst )
	   {
	     /* ...counting and copying character by character.
	      */
	     ++len;
	     if( buf != NULL )
	       *buf++ = *subst;
	     ++subst;
	   }
	 }
       }

       else
       {
	 /*
	  * Copy one literal character from "fmt"...
	  */
	 if( buf != NULL )
	   /*
	    * ...storing as necessary...
	    */
	   *buf++ = c;

	 /* ...and counting it anyway.
	  */
	 ++len;
       }
     } while( c );

  /* Always return the total number of characters which were, or would
   * have been transferred to "buf".
   */
  return len;
}

static
void create_parent_directory_hierarchy( const char *pathname, int mode )
{
  /* Recursive helper function to create a directory branch, including
   * all missing parent directories, (analogous to using "mkdir -p").
   *
   * FIXME: We allow for either "/" or "\" as the directory separator;
   * do we also need to accommodate possible use of multibyte-character
   * encodings?  (Hopefully, archives should rely exclusively on the
   * POSIX Portable Character set, i.e. 7-bit ASCII, so maybe not).
   */
  char *parse, *parent, *mark = NULL, *stop = NULL;

  /* We work with a copy of the supplied "pathname", so we can guarantee
   * we have a modifiable string, and can accept a "const" input string.
   */
  if( (parse = parent = strdup( pathname )) != NULL )
  {
    /* Having obtained a valid copy of "pathname", we parse it...
     */
    while( *parse )
    {
      /* Set the "stop" mark at the first in the last sequence of
       * one or more directory separators detected, (if any)...
       */
      stop = mark;
      /*
       * ...then step over any following characters which are
       * neither the string terminator, nor further separators.
       */
      while( *parse && (*parse != '/') && (*parse != '\\') )
	++parse;

      /* If we haven't yet found the string terminator, then we
       * must have found a new sequence of one or more separators;
       * mark it as a new candidate "stop" mark location.
       */
      if( *parse )
	mark = parse;

      /* Now, step over all contiguous separators in the current
       * sequence, before restarting the outer loop, to parse the
       * next directory or file name, (if any).  Note that we defer
       * updating the "stop" mark until the start of this new cycle;
       * this ensures that we correctly ignore any separators which
       * trail at the end of "pathname", with no following "name"
       * component.
       */
      while( (*parse == '/') || (*parse == '\\') )
	++parse;
    }
    if( stop != NULL )
    {
      /* We found a valid point, at which to split the current leaf
       * of "pathname" from its parent branch hierarchy; split it and
       * recurse through the "mkdir" function, to create the parent
       * directory hierarchy, as required.
       */
      *stop = '\0';
      mkdir_recursive( parent, mode );
    }

    /* We are now done with our temporary copy of "pathname"; reclaim
     * the memory which was allocated to store it.
     */
    free( parent );
  }
}

int mkdir_recursive( const char *pathname, int mode )
{
  /* Public entry point for the recursive "mkdir" function.
   *
   * First, we attempt a simple "mkdir"; if this succeeds, the
   * parent directory branch is already in place, and we have
   * nothing more to do.
   */
  if( mkdir( pathname, mode ) == 0 )
    return 0;

  /* Otherwise...
   */
  switch( errno )
  {
    case ENOENT:
      /*
       * This indicates a gap in the parent branch hierarchy;
       * call the preceding helper, to fill the gap...
       */
      create_parent_directory_hierarchy( pathname, mode );
      /*
       * ...before making a further attempt to add the leaf.
       */
      return mkdir( pathname, mode );

    case EEXIST:
      {
	/* Here, the initial "mkdir" failed because a file
	 * system entity called "pathname" already exists; if
	 * this is already a directory, all is well; (there is
	 * no need to create it again)...
	 */
	struct stat target;
	if( (stat( pathname, &target ) == 0) && S_ISDIR( target.st_mode ) )
	  return 0;
      }
  }
  /* ...otherwise we simply fall through and fail...
   */
  return -1;
}

int set_output_stream( const char *pathname, int mode )
{
  /* Attach the extractor's output data stream to a specified file,
   * creating the parent directory branch hierarchy as required, and
   * return a file descriptor on the stream.
   */
  int fd;

  /* First, simply attempt to create the destination file...
   */
  if( ((fd = creat( pathname, mode )) < 0) && (errno == ENOENT) )
  {
    /* ...but, on failure due to a gap in the directory structure
     * call the preceding helper to create the necessary hierarchy...
     */
    create_parent_directory_hierarchy( pathname, 0755 );
    /*
     * ...before making a further attempt to create the file.
     */
    return creat( pathname, mode );
  }

  /* Here, we will have the invalid file descriptor from the initial
   * failed attempt to create the file; we return it to indicate the
   * ultimate failure to create this file.
   */
  return fd;
}

/* $RCSfile: mkpath.c,v $: end of file */
