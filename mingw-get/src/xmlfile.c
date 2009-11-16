/*
 * xmlfile.c
 *
 * $Id: xmlfile.c,v 1.1 2009-11-16 21:54:30 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Helper function for constructing path names to the XML data files
 * specifying the mingw-get configuration, and installation manifest.
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
#include <stdlib.h>

#include "mkpath.h"

const char *xmlfile( const char *name, const char *modifier )
{
  /* Construct a full path name for the file specified by "name",
   * adding the mandatory ".xml" extension; the path is always based
   * at "${APPROOT}", with "modifier" specifying an optional subdirectory
   * of the standard path, (typically used as a temporary location for
   * internet downloads while in transit).
   */
  const char *datapath = "%R" "var/lib/mingw-get/data" "%/M/%F.xml";
  char *datafile = (char *)(malloc( mkpath( NULL, datapath, name, modifier ) ));

  mkpath( datafile, datapath, name, modifier );
  return (const char *)(datafile);
}

/* $RCSfile: xmlfile.c,v $: end of file */
