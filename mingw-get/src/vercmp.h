#ifndef VERCMP_H
/*
 * vercmp.h
 *
 * $Id: vercmp.h,v 1.1 2009-11-16 21:54:30 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Public interface for the package version comparator module, as
 * implemented in the "C++" language file "vercmp.cpp".
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
#define VERCMP_H  1

#include <stdlib.h>

enum
{ /* The constituent elements of a package version number,
   * and build serial number, in sequential order as they appear
   * within a correctly formed package tarball name.
   */
  VERSION_MAJOR = 0,
  VERSION_MINOR,
  VERSION_PATCHLEVEL,
  SNAPSHOT_DATE,
  SERIAL_NUMBER,

  /* Not a real element of the version/serial number sequence,
   * but this final entry in the enumeration provides a convenient
   * counter for the number of real elements listed above.
   */
  VERSION_ELEMENT_COUNT
};

struct version_t
{
  /* Internal structure, used to capture an individual element
   * of a decomposed version number, or build serial number.
   */
  unsigned long	    value;
  const char*	    suffix;
};

#ifdef __cplusplus
/*
 * Define the properties of the "C++" class API...
 */
class pkgVersionInfo
{
  /* A class for capture and manipulation of package version numbers
   * in decomposed "major.minor.patch-datetamp-sequence" form.
   */
  public:
    /* Constructor...
     * This expects either one or two "char *" arguments:
     * the first is the package version number, in "major.minor.patch"
     * format; the second is build serial number in "datestamp-sequence"
     * format.  If the second is omitted, the build serial number may
     * be appended to the first, in the full format as above.
     */
    pkgVersionInfo( const char* version = "", const char* build = NULL );

    /* Package version comparison operators.
     */
    bool operator<( const pkgVersionInfo& );
    bool operator<=( const pkgVersionInfo& );
    bool operator==( const pkgVersionInfo& );
    bool operator!=( const pkgVersionInfo& );
    bool operator>=( const pkgVersionInfo& );
    bool operator>( const pkgVersionInfo& );

  private:
    /* The decomposed version/serial number elements.
     */
    struct version_t version_elements[VERSION_ELEMENT_COUNT];

    /* An internal comparison helper function
     */
    long Compare( const pkgVersionInfo&, int );
};

#endif /* __cplusplus */
#endif /* VERCMP_H: $RCSfile: vercmp.h,v $: end of file */
