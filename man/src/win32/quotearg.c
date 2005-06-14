/* Copyright (C) 2004, Free Software Foundation, Inc.
     Written by Keith Marshall (keith.d.marshall@ntlworld.com)

The code defined in this file is a generalisation of code originally
written by the author, assisted by Jeff Conrad (jeff_conrad@msn.com),
as a component of GNU Troff (groff).

This is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

This software is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY of any kind; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Please refer to
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this software; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin St - Fifth Floor, Boston, MA 02110-1301,
USA. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

#undef FALSE
#undef TRUE

extern char** get_quoted_argv( char* const* argv );
extern void purge_quoted_argv( char** argv );

static enum {FALSE=0, TRUE}
needs_quoting( const char *string )
{
  /* scan 'string' to see if it needs quoting for MSVC 'spawn'/'exec'
   * (i.e. if it contains whitespace or embedded quotes)
   */

  if( string == NULL )          /* ignore NULL strings */
    return( FALSE );

  if( *string == '\0' )         /* protect zero length arguments */
    return( TRUE );             /* so they will not be discarded */

  while( *string )
  {
    /* scan non-NULL strings, up to '\0' terminator,
     * returning 'TRUE' if quote or white space found.
     */

    if( (*string == '"') || isspace( *string ) )
      return( TRUE );

    /* otherwise, continue scanning to end of string */

    ++string;
  }

  /* fall through, if no quotes or white space found,
   * in which case, return 'FALSE'.
   */

  return( FALSE );
}
      
static char*
quote_arg( char* string )
{
  /* Enclose arguments in double quotes so that the parsing done in the
   * MSVC runtime startup code doesn't split them at whitespace.  Escape
   * embedded double quotes so that they emerge intact from the parsing.
   */

  int backslashes;
  char *quoted, *p, *q;

  if( needs_quoting( string ))
  {
    /* need to create a quoted copy of 'string' ...
     * maximum buffer space needed is twice the original length,
     * plus two enclosing quotes and one '\0' terminator.
     */
    
    if( (quoted = malloc( 2 * strlen( string ) + 3 )) == NULL )
    {
      /* can't continue ...
       * (couldn't get a buffer to store the result) ...
       * Microsoft implementation of 'malloc()' does NOT set 'errno',
       * so, correct this deficiency, and bail out gracefully.
       */

      errno = ENOMEM;
      return( NULL );
    }

    /* ok to proceed ...
     * insert the opening quote, then copy the source string,
     * adding escapes as required.
     */

    *quoted = '"';
    for( backslashes = 0, p = (char *) string, q = quoted ; *p ; p++ )
    {
      if( *p == '\\' )
      {
	/* just count backslashes when we find them ...
	 * we will copy them out later, when we know if the count
	 * needs to be adjusted, to escape an embedded quote.
	 */
	
	++backslashes;
      }
      else if( *p == '"' )
      {
	/* this embedded quote character must be escaped,
	 * but first double up any immediately preceding backslashes,
	 * with one extra, as the escape character.
	 */

	for( backslashes += backslashes + 1 ; backslashes ; backslashes-- )
	  *++q = '\\';

	/* and now, add the quote character itself */

	*++q = '"';
      }
      else
      {
	/* any other character is simply copied,
	 * but first, if we have any pending backslashes,
	 * we must now insert them, without any count adjustment
	 */

	while( backslashes )
	{
	  *++q = '\\';
	  --backslashes;
	}

	/* and then, copy the current character */

	*++q = *p;
      }
    }

    /* at end of argument ...
     * if any backslashes remain to be copied out, append them now,
     * doubling the actual count to protect against reduction by MSVC,
     * as a consequence of the immediately following closing quote.
     */

    for( backslashes += backslashes ; backslashes ; backslashes-- )
      *++q = '\\';

    /* finally ...
     * add the closing quote, terminate the quoted string,
     * and adjust its size to what was actually required,
     * ready for return.
     */

    *++q = '"';
    *++q = '\0';
    if( (string = realloc( quoted, strlen( quoted ) + 1 )) == NULL )
    {
      /* but bail out gracefully, on error,
       * again correcting Microsoft's deficiency
       * in failing to set 'errno'
       */

      errno = ENOMEM;
    }
  }

  /* 'string' now refers to the argument,
   * quoted and escaped, as required.
   */

  return( string );
}

void
purge_quoted_argv( char **argv )
{
  /* to avoid memory leaks ...
   * free all memory previously allocated by 'quoted_arg()',
   * within the scope of the referring argument vector, 'argv'.
   */

  char **argp;

  if( argv )
  {
    for( argp = argv ; *argp ; argp++ )
    {
      /* any argument beginning with a double quote
       * SHOULD have been allocated by 'quoted_arg()' ...
       */
      
      if( **argp == '"' )
        free( *argp );          /* so free its allocation */
    }

    /* and finally ...
     * free the memory allocated to the entire argument vector,
     * (it was a temporary copy of the original unquoted 'argv',
     *  and is no longer usable).
     */

    free( argv );
  }
}

char**
get_quoted_argv( char* const* argv )
{
  /* create a temporary copy of an original 'argv'
   * with individual arguments appropriately wrapped in double quotes,
   * so that the parsing done in the MSVC runtime startup code doesn't
   * split them at whitespace.
   */

  int i;                /* used as an index into 'argv' or 'quoted_argv' */
  int argc = 0;         /* initialise argument count; may be none  */
  char **quoted_argv;   /* used to build a quoted local copy of 'argv' */

  /* first count the number of arguments
   * which are actually present in the passed 'argv' */

  if( argv )
    for( quoted_argv = (char **)argv ; *quoted_argv ; ++argc, ++quoted_argv )
      ;

  /* if we now have an argument count,
   * then we may create a quoted copy of the argument vector.
   */
  
  if( argc )
  {
    /* we do have at least one argument ...
     * we will use a copy of the 'argv', in which to do the quoting,
     * so we must allocate space for it */

    if( (quoted_argv = malloc(( argc + 1 ) * sizeof( char ** ))) == NULL )
    {
      /* if we didn't get enough space,
       * then ensure that 'errno' is set appropriately,
       * (Microsoft's 'malloc' implementation neglects to do this),
       * and bail out gracefully */

      errno = ENOMEM;
      return( NULL );
    }

    /* now copy the passed 'argv' into our new vector,
     * quoting its contents as required */
    
    for( i = 0 ; i < argc ; i++ )
      if( (quoted_argv[i] = quote_arg( argv[i] )) == NULL )
      {
        /* failure to quote any single argument ...
         * invalidates the entire copy of the argument vector,
         * so, clean up, and bail out gracefully.
         */

        purge_quoted_argv( quoted_argv );
        return( NULL );
      }
  }
  else

    /* we didn't find any arguments,
     * but at least one is required, as the child program name,
     * so, bail out.
     */

    return( NULL );

  /* when we get to here ...
   * we have successfully created a quoted copy of 'argv',
   * so, terminate it with a NULL pointer, and return it.
   */

  quoted_argv[argc] = NULL;
  return( quoted_argv );
}

/* quotearg.c: end of file */
