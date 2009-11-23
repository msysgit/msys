/*
 * keyword.c
 *
 * $Id: keyword.c,v 1.1 2009-11-23 20:44:25 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Implementation of "has_keyword()" function; this is used to check
 * for the presence of a specified keyword with a wihtespace separated
 * list, appearing as an XML property string.
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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define FALSE  0
#define TRUE   !FALSE

static inline
char *safe_strdup( const char *src )
{
  /* Duplicate a "C" string into dynamically allocated memory,
   * safely handling a NULL source reference.
   */
  return src ? strdup( src ) : NULL;
}

int has_keyword( const char *keywords, const char *wanted )
{
  /* Check the given "keywords" list for the presence of
   * the "wanted" keyword.
   */
  char *inspect;
  if( (inspect = safe_strdup( keywords )) != NULL )
  {
    /* We've found a non-empty list of keywords to inspect;
     * initialise a pointer to the first entry for matching...
     */
    char *match = inspect;
    while( *match )
    {
      /* We haven't yet checked all of the available keywords;
       * locate the end of the current inspection reference...
       */
      char *brk = match;
      while( *brk && ! isspace( *brk ) )
	++brk;

      /* ...and append a NUL terminator.
       */
      if( *brk )
	*brk++ = '\0';

      /* Check the currently selected alias...
       */
      if( strcmp( match, wanted ) == 0 )
      {
	/* ...and if it's a match, then immediately release the
	 * scratch-pad memory we used for the keyword comparisons,
	 * and return "true".
	 */
	free( (void *)(inspect) );
	return TRUE;
      }

      /* Otherwise, proceed to check the next keyword, if any.
       */
      match = brk;
    }

    /* If we get to here, then all assigned aliases have been
     * checked, without finding a match; the scratch-pad memory
     * remains allocated, so release it, before falling through
     * to return "false".
     */
    free( (void *)(inspect) );
  }
  /* Return "false" in all cases where no matching name can be found.
   */
  return FALSE;
}

/* $RCSfile: keyword.c,v $: end of file */
