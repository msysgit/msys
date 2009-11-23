/*
 * pkgfind.cpp
 *
 * $Id: pkgfind.cpp,v 1.1 2009-11-23 20:44:25 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Implementation of search routines for locating specified records
 * within the XML package-collection database.
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
#include <string.h>

#include "pkgbase.h"

static inline
bool pkgHasMatchingName( pkgXmlNode *pkg, const char *wanted )
{
  /* Helper to locate a package specification by package name;
   * returns "true" if the XML node under consideration defines
   * a "package", having the "wanted" name; else "false".
   */
  return pkg->IsElementOfType( "package" )
    /*
     * subject to the canonical name of the package matching
     * the "wanted" name, or any assigned package name alias...
     */
    &&( (strcmp( wanted, pkg->GetPropVal( "name", "" )) == 0)
        || (has_keyword( pkg->GetPropVal( "alias", NULL ), wanted ) != 0)
      );
}

pkgXmlNode *
pkgXmlDocument::FindPackageByName( const char *name, const char *subsystem )
{
  pkgXmlNode *dir = GetRoot()->GetChildren();
  /*
   * Working from the root of the package directory tree...
   * search all "package-collection" XML nodes, to locate a package
   * by "name"; return a pointer to the XML node which contains the
   * specification for the package, or NULL if no such package.
   */
  while( dir != NULL )
  {
    /* Select only "package-collection" elements...
     */
    if( dir->IsElementOfType( "package-collection" )
    &&  match_if_explicit( subsystem, dir->GetPropVal( "subsystem", NULL )) )
    {
      /* ...inspect the content of each...
       */
      pkgXmlNode *pkg = dir->GetChildren();
      while( pkg != NULL )
      {
	/* ...returning immediately, if we find a "package"
	 * element with the required "name" property...
	 */
	if( pkgHasMatchingName( pkg, name ) )
	  return pkg;

	/* ...otherwise, continue searching among any further
	 * entries in the current "package-collection"...
	 */
	pkg = pkg->GetNext();
      }
    }
    /* ...and ultimately, in any further "package-collection" elements
     * which may be present.
     */
    dir = dir->GetNext();
  }

  /* If we get to here, we didn't find the required "package";
   * return NULL, to indicate failure.
   */
  return NULL;
}

static
pkgXmlNode* pkgFindNextAssociate( pkgXmlNode* pkg, const char* tagname )
{
  /* Core implementation for both pkgXmlNode::FindFirstAssociate
   * and pkgXmlNode::FindNextAssociate methods.  This helper starts
   * at the node specified by "pkg", examining it, and if necessary,
   * each of its siblings in turn, until one of an element type
   * matching "tagname" is found.
   */
  while( pkg != NULL )
  {
    /* We still have this "pkg" node, not yet examined...
     */
    if( pkg->IsElementOfType( tagname ) )
      /*
       * ...it matches our search criterion; return it...
       */
      return pkg;

    /* The current "pkg" node didn't match our criterion;
     * move on, to examine its next sibling, if any...
     */
    pkg = pkg->GetNext();
  }

  /* We ran out of siblings to examine, without finding any
   * to match our criterion; return nothing...
   */
  return NULL;
}

pkgXmlNode*
pkgXmlNode::FindFirstAssociate( const char* tagname )
{
  /* For the node on which this method is invoked,
   * return the first, if any, of its immediate children,
   * which is an element of the type specified by "tagname"...
   */
  return this ? pkgFindNextAssociate( GetChildren(), tagname ) : NULL;
}

pkgXmlNode*
pkgXmlNode::FindNextAssociate( const char* tagname )
{
  /* Invoked on any node returned by "FindFirstAssociate",
   * or on any node already returned by "FindNextAssociate",
   * return the next sibling node, if any, which is an element
   * of the type specified by "tagname"...
   */
  return this ? pkgFindNextAssociate( GetNext(), tagname ) : NULL;
}

/* $RCSfile: pkgfind.cpp,v $: end of file */
