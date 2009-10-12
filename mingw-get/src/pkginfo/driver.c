/*
 * driver.c
 *
 * $Id: driver.c,v 1.1 2009-10-12 21:35:29 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Simple driver program, for the lexical package name analyser, as
 * implemented in the "flex" file "pkginfo.l".  When compiled as:
 *
 *   lex -t pkginfo.l > pkginfo.c
 *   gcc -o pkginfo driver.c pkginfo.c
 *
 * it creates a simple command line tool for analysis and validation
 * of package archive names, in accordance with agreed MinGW Project
 * package naming conventions.
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
#include <stdio.h>

#include "pkginfo.h"

static __inline__
char *spec( char *tag )
{
  /* A local helper function...
   * Returns the content of `tag', if defined,
   * otherwise "<unspecified>".
   */
  static char *unspecified = "<unspecified>";

  if( tag == NULL )
    return unspecified;
  return tag;
}

int main( int argc, char **argv )
{
  /* A trivial driver program,
   * to illustrate the behaviour of the "pkginfo" scanner.
   */
  pkginfo_t tags = {
    /*
     * Labels to print,
     * identifying individual elements of a package tarname.
     */
    "Package Name:",
    "Package Version:",
    "Package Build:",
    "Subsystem Name:",
    "Subsystem Version:",
    "Subsystem Build:",
    "Release Status:",
    "Release Reference:",
    "Component Type:",
    "Component Version:",
    "Archive Format:",
    "Compression Type"
  }, signature;

  /* Treating each command line argument as an individual
   * package tarball name...
   */
  while( --argc )
  {
    int start;
    void *refdata;

    /* ...analyse it...
     */
    if( (refdata = get_pkginfo( *++argv, signature )) != NULL )
    {
      /* ...and, on success, print its decomposition summary.
       */
      for( start = PACKAGE_NAME; start < PACKAGE_TAG_COUNT; start++ )
	printf( "%-19s%s\n", tags[start], spec( signature[start] ) );

      /* To avoid memory leaks...
       * free the dynamic memory allocated by the scanner.
       */
      free( refdata );
    }
  }

  /* Trivially, always return success.
   */
  return 0;
}

/* $RCSfile: driver.c,v $: end of file */
