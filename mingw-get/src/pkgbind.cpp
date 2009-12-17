/*
 * pkgbind.cpp
 *
 * $Id: pkgbind.cpp,v 1.2 2009-12-17 17:35:12 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Implementation of repository binding for the pkgXmlDocument class.
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dmh.h"
#include "pkgbase.h"

pkgXmlNode *pkgXmlDocument::BindRepositories( bool force_update )
{
  /* Identify the repositories specified in the application profile,
   * and merge their associated package distribution lists into the
   * active XML database, which is bound to the profile.
   */
  pkgXmlNode *dbase = GetRoot();

  /* Before blindly proceeding, perform a sanity check...
   * Verify that this XML database defines an application profile,
   * and that the associated application is "mingw-get"...
   */
  if( (strcmp( dbase->GetName(), "profile" ) == 0)
  &&  (strcmp( dbase->GetPropVal( "application", "?" ), "mingw-get") == 0) )
  {
    /* Sanity check passed...
     * Walk the XML data tree, selecting "repository" specifications...
     */
    pkgXmlNode *repository = dbase->FindFirstAssociate( "repository" );
    while( repository != NULL )
    {
      /* For each "repository" specified, identify its "catalogues"...
       *
       * FIXME: this requires the "package-lists" to be individually
       * specified within the locally defined "repository" elements;
       * it should allow for deduction of these, from a specifically
       * named "repository-index" file identified via the repository
       * URI template, and hosted by the download server itself.
       */
      pkgXmlNode *catalogue = repository->FindFirstAssociate( "package-list" );
      while( catalogue != NULL )
      {
	/* ...and for each named "catalogue"...
	 */
	const char *dfile, *dname = catalogue->GetPropVal( "catalogue", NULL );
	if( (dname != NULL) && ((dfile = xmlfile( dname )) != NULL) )
	{
	  /* Check for a locally cached copy of the "package-list" file...
	   */
	  if( force_update || (access( dfile, F_OK ) != 0) )
	    /*
	     * When performing an "update", or if no local copy is available...
	     * Force a "sync", to fetch a copy from the public host.
	     */
	    SyncRepository( dname, repository );

	  /* We SHOULD now have a locally cached copy of the package-list;
	   * attempt to merge it into the active profile database...
	   */
	  pkgXmlDocument merge( dfile );
	  if( merge.IsOk() )
	  {
	    /* We successfully loaded the XML catalogue; refer to its
	     * root element...
	     */
	    dmh_printf( "Bind repository: %s\n", merge.Value() );
	    pkgXmlNode *pkglist;
	    if( (pkglist = merge.GetRoot()) != NULL )
	    {
	      /* ...read it, selecting each of the "package-collection"
	       * records contained within it...
	       */
	      pkglist = pkglist->FindFirstAssociate( "package-collection" );
	      while( pkglist != NULL )
	      {
		/* ...and append a copy of each to the active profile...
		 */
		dbase->LinkEndChild( pkglist->Clone() );

		/* Move on to the next "package-collection" (if any)
		 * within the current catalogue...
		 */
		pkglist = pkglist->FindNextAssociate( "package-collection" );
	      }
	    }
	  }
	  else
	    dmh_notify( DMH_WARNING, "Bind repository: FAILED: %s\n", dfile );

	  /* However we handled it, the XML file's path name in "dfile" was
	   * allocated on the heap; we lose its reference on termination of
	   * this loop, so we must free it to avoid a memory leak.
	   */
	  free( (void *)(dfile) );
	}

	/* A repository may comprise an arbitrary collection of software
	 * catalogues; move on, to process the next catalogue (if any) in
	 * the current repository collection.
	 */
	catalogue = catalogue->FindNextAssociate( "package-list" );
      }

      /* Similarly, a complete distribution may draw from an arbitrary set
       * of distinct repositories; move on, to process the next repository
       * specified (if any).
       */
      repository = repository->FindNextAssociate( "repository" );
    }

    /* On successful completion, return a pointer to the root node
     * of the active XML profile.
     */
    return dbase;
  }

  /* Fall through on total failure to interpret the profile, returning
   * NULL to indicate failure.
   */
  return NULL;
}

/* $RCSfile: pkgbind.cpp,v $: end of file */
