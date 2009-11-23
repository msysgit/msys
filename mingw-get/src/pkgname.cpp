/*
 * pkgname.cpp
 *
 * $Id: pkgname.cpp,v 1.1 2009-11-23 20:44:25 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Implementation for the non-inherited components of the pkgXmlNode
 * class, as declared in file "pkgdesc.h"; fundamentally, these are
 * the accessors for package "tarname" properties, as specified in
 * XML nodes identified as "release" elements.
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

static
const char *pkgArchiveName( pkgXmlNode *rel, const char *tag, unsigned opt )
{
  /* Local helper to establish actual release file names...
   * applicable only to XML "release" elements.
   */
  if( ! rel->IsElementOfType( "release" ) )
  {
    dmh_control( DMH_BEGIN_DIGEST );
    dmh_notify( DMH_ERROR, "internal package specification error\n" );
    dmh_notify( DMH_ERROR, "can't get 'tarname' for non-release element\n" );
    dmh_notify( DMH_ERROR, "please report this to the package maintainer\n" );
    dmh_control( DMH_END_DIGEST );
    return NULL;
  }

  /* Given a package release specification...
   * determine the archive name for the tarball to be processed; this
   * is retrieved from a child XML element with name specified by "tag";
   * by default, if "opt" is non-zero, it is the canonical "tarname"
   * assigned to the release element itself, unless an alternative
   * specification is provided; if "opt" is zero, no default is
   * assumed, and the return value is NULL if no alternative
   * specification is provided.
   */
  unsigned matched = 0;
  pkgXmlNode *dl = rel->GetChildren();
  while( dl != NULL )
  {
    /* Visit all children of the release specification element,
     * checking for the presence of an expected specification...
     */
    if( dl->IsElementOfType( tag ) )
    {
      /* Found one; ensure it is the only one...
       */
      if( matched++ )
	/*
	 * ...else emit a warning, and ignore this one...
	 */
	dmh_notify( DMH_WARNING, "%s: archive name reassignment ignored\n",
	    rel->GetPropVal( "tarname", "<unknown>" )
	);
      else
	/* ...ok; this is the first "tag" specification,
	 * accept it as the non-default source of the release's
	 * "tarname" property.
	 */
	rel = dl;
    }
    /* Continue, until all children have been visited.
     */
    dl = dl->GetNext();
  }
  /* "rel" now points to the XML element having the appropriate
   * "tarname" specification; return a pointer to it's value.
   */
  return (opt || matched) ? rel->GetPropVal( "tarname", NULL ) : NULL;
}

const char *pkgXmlNode::SourceArchiveName()
{
  /* Applicable only for XML nodes designated as "release".
   *
   * Retrieve the source tarball name, if specified, from the
   * "tarname" property of the contained "source" element, within
   * an XML node designated as a "release" element.
   *
   * Returns a pointer to the text of the "tarname" property of the
   * contained "source" element, or NULL if the containing node does
   * not represent a "release", or if it does not have a contained
   * "source" element specifying a "tarname" property.
   */
  return pkgArchiveName( this, "source", 0 );
}

const char *pkgXmlNode::ArchiveName()
{
  /* Applicable only for XML nodes designated as "release".
   *
   * Retrieve the actual tarball name, if specified, from the
   * "tarname" property of a contained "download" element, within
   * an XML node designated as a "release" element.
   *
   * Returns a pointer to the text of the "tarname" property of the
   * contained "download" element, or to the "tarname" property of
   * the containing "release" element, if it does not contain an
   * alternative specification within a "download" element; if
   * unresolved to either of these, returns NULL.
   */
  return pkgArchiveName( this, "download", 1 );
}

/* $RCSfile: pkgname.cpp,v $: end of file */
