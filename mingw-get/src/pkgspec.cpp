/*
 * pkgspec.cpp
 *
 * $Id: pkgspec.cpp,v 1.1 2009-11-16 21:54:30 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Implementation for the "pkgTarName" class, as declared in header
 * file "pkginfo.h".
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
#include "pkginfo.h"
#include "vercmp.h"

#include <string.h>

/* Constructors...
 */
pkgSpecs::pkgSpecs( const char *tarname )
{
  /* Parse the given tarball name, storing its constituent element
   * decomposition within the class' local "pkginfo" array structure.
   */
  content = get_pkginfo( tarname ? tarname : "", specs );
}

pkgSpecs::pkgSpecs( pkgXmlNode *release )
{
  /* Retrieve the "tarname" from an XML "release" specification,
   * then construct the "pkgSpecs" as if it were specified directly.
   */
  const char *tarname = release ? release->GetPropVal( "tarname", NULL ) : NULL;
  content = get_pkginfo( tarname ? tarname : "", specs );
}

/* Copy constructor...
 */
static
void *clone_specs( char *content, pkginfo_t const src, pkginfo_t dst )
{
  /* Local helper function performs a deep copy of the "content" buffer,
   * and assigns the "specs" pointers to refer to it; this is the action
   * required to implement the copy constructor, and it is also used by
   * the assignment operator implentation.
   */
  char *rtn;
  int count = PACKAGE_TAG_COUNT;

  /* Find the last allocated pointer in the source "specs" list; this
   * tells us where to find the last string in the "content" buffer...
   */
  while( (count > 0) && (src[--count] == NULL) )
    ;

  /* ...whence we may compute the size of the buffer, and allocate
   * a new buffer, into which to copy the data.
   */
  count = src[count] + strlen( src[count] ) - content;
  if( (rtn = (char *)(malloc( count + 1))) != NULL )
  {
    /* On successful buffer allocation, copy the data,
     * then walk the list of pointers...
     */
    rtn = (char *)(memcpy( rtn, content, count ));
    for( count = 0; count < PACKAGE_TAG_COUNT; ++count )
    {
      if( src[count] == NULL )
	/*
	 * ...propagating NULL pointers "as are"...
	 */
	dst[count] = NULL;

      else
	/* ...and non-NULL adjusted, as necessary,
	 * to point into the copied data buffer...
	 */
	dst[count] = (char *)(rtn) + (src[count] - content);
    }
  }
  /* ...ultimately, returning the base address of the new buffer.
   */
  return (void *)(rtn);
}
/* Formal implementation of the copy constructor...
 */
pkgSpecs::pkgSpecs( const pkgSpecs& src )
{
  /* ...requires no more than a call to the local helper function.
   */
  content = clone_specs( (char *)(src.content), src.specs, specs );
}

/* Assignment operator...
 */
pkgSpecs& pkgSpecs::operator=( const pkgSpecs& rhs )
{
  /* Provided the lhs and rhs represent distinct objects...
   */
  if( this != &rhs )
  {
    /* ...this is much the same as the copy constructor, except that,
     * while the constructor is guaranteed to be creating a new object,
     * assignment may be replacing an existing lhs object; this will
     * own a dynamically allocated data buffer, which must be freed,
     * to avoid leaking memory.
     */
    free( content );
    content = clone_specs( (char *)(rhs.content), rhs.specs, specs );
  }
  return *this;
}

/* Destructor...
 */
pkgSpecs::~pkgSpecs()
{
  /* Need to free the dynamic memory associated with the "specs" array,
   * and in which the actual tarname decomposition is stored.
   */
  free( content );
}

/* Comparison operators...
 */
static inline
pkgVersionInfo version( pkgSpecs& pkg )
{
  /* Local helper, to construct package version descriptors
   * for use in version comparison operator implementations.
   */
  return pkgVersionInfo( pkg.GetPackageVersion(), pkg.GetPackageBuild() );
}

bool pkgSpecs::operator<( pkgSpecs& rhs )
{
  /* Check if the given package release is less recent, as indicated
   * by its version and build date/serial number, than another.
   */
  return version( *this ) < version( rhs );
}

bool pkgSpecs::operator<=( pkgSpecs& rhs )
{
  /* Check if the given package release is no more recent, as indicated
   * by its version and build date/serial number, than another.
   */
  return version( *this ) <= version( rhs );
}

bool pkgSpecs::operator>=( pkgSpecs& rhs )
{
  /* Check if the given package release is no less recent, as indicated
   * by its version and build date/serial number, than another.
   */
  return version( *this ) >= version( rhs );
}

bool pkgSpecs::operator>( pkgSpecs& rhs )
{
  /* Check if the given package release is more recent, as indicated
   * by its version and build date/serial number, than another.
   */
  return version( *this ) > version( rhs );
}

/* $RCSfile: pkgspec.cpp,v $: end of file */
