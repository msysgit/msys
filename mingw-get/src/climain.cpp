/*
 * climain.cpp
 *
 * $Id: climain.cpp,v 1.1 2009-11-16 21:54:30 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Implementation of the main program function, which is invoked by
 * the command line start-up stub when arguments are supplied; this
 * causes the application to continue running as a CLI process.
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
#include <string.h>

#include "dmh.h"

#include "pkgbase.h"
#include "pkgtask.h"

EXTERN_C int climain( int argc, char **argv )
{
  /* Set up the diagnostic message handler, using the console's
   * `stderr' stream for notifications...
   */
  dmh_init( DMH_SUBSYSTEM_TTY, *argv++ );

  /* TODO: insert code here, to interpret any OPTIONS specified
   * on the command line.
   */

  /* Interpret the `action keyword', specifying the action to be
   * performed on this invocation...
   */
  int action = action_code( *argv );
  if( action < 0 )
    /*
     * The specified action keyword was invalid;
     * force an abort through a DMH_FATAL notification...
     */
    dmh_notify( DMH_FATAL, "%s: unknown action keyword\n", *argv );

  /* If we get to here, then the specified action identifies a
   * valid operation; load the package database, according to the
   * local `profile' configuration, and invoke the operation.
   */
  const char *dfile;
  pkgXmlDocument dbase( dfile = xmlfile( "profile" ) );
  if( dbase.IsOk() )
  {
    /* We successfully loaded the basic settings...
     * The configuration file name was pushed on to the heap,
     * by xmlfile(); we don't need that any more, (because it
     * is reproduced within the database image itself), so
     * free the heap copy, to avoid memory leaks.
     */
    free( (void *)(dfile) );

#if 0
    /* Merge all package lists, as specified in the "repository"
     * section of the "profile", into the XML database tree...
     */
    if( dbase.BindRepositories() == NULL )
      /*
       * ...bailing out, on an invalid profile specification...
       */
      dmh_notify( DMH_FATAL, "%s: invalid application profile\n", dbase.Value() );

    /* Now schedule the specified action for each additionally
     * specified command line argument, (each of which is assumed
     * to represent a package name...
     */
    while( --argc )
      dbase.Schedule( (unsigned long)(action), *++argv );

    /* ...and finally, execute all scheduled actions...
     */
    dbase.ExecuteActions();
#endif

    /* If we get this far, then all actions completed successfully;
     * we are done...
     */
    return EXIT_SUCCESS;
  }

  /* If we get to here, then the package database load failed;
   * once more, we force an abort through a DMH_FATAL notification...
   *
   * Note: although dmh_notify does not return, in the DMH_FATAL case,
   * GCC cannot know this, so we pretend that it gives us a return value,
   * to avoid a possible warning about reaching the end of a non-void
   * function without a return value assignment...
   */
  return dmh_notify( DMH_FATAL, "%s: cannot load configuration\n", dfile );
}

/* $RCSfile: climain.cpp,v $: end of file */
