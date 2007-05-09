/*
 * catopen.c
 *
 * $Id: catopen.c,v 1.3 2007-05-09 22:43:51 keithmarshall Exp $
 *
 * Copyright (C) 2006, Keith Marshall
 *
 * This file implements the `catopen' function, required to support
 * POSIX compatible national language message catalogues in MinGW.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last modification: 29-Dec-2006
 *
 *
 * This is free software.  It is provided AS IS, in the hope that it may
 * be useful, but WITHOUT WARRANTY OF ANY KIND, not even an IMPLIED WARRANTY
 * of MERCHANTABILITY, nor of FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 * Permission is granted to redistribute this software, either "as is" or
 * in modified form, under the terms of the GNU General Public License, as
 * published by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to the
 * Free Software Foundation, 51 Franklin St - Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */
#define WIN32_LEAN_AND_MEAN

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include <locale.h>
#include <nl_types.h>
#include <nlspath.h>
#include <msgcat.h>
#include <mctab.h>

#ifndef LC_MESSAGES
/*
 * On Win32 platforms, we don't expect LC_MESSAGES to be defined.
 * For this, and any others which don't define it, substitute LC_CTYPE.
 */
# define LC_MESSAGES  LC_CTYPE
#endif

#include <platform.h>
#ifdef _WIN32
/*
 * For MSVCRT based Win32 platforms,
 * we need to provide the malloc/realloc wrapper function,
 * which is prototyped in platform.h
 */
#include <errno.h>

void *mc_realloc( void *ptr, unsigned size )
{
  void *retptr;
  if( (retptr = realloc( ptr, size )) == NULL )
    errno = ENOMEM;
  return retptr;
}
#endif

/* Now, we define the helper functions, used to implement `catopen'.
 */

int mc_validate( __const char *name )
{
  int fd;

  /* Attempt to open the message catalogue specified by `name',
   * on file descriptor `fd'; if successful, confirm that the file
   * has the correct magic number, and an issue (version) number
   * we can handle.
   */
  if( (fd = open( name, O_RDONLY | O_BINARY )) >= 0 )
  {
    MSGCAT test;
    struct ident supported = { mk_msgcat_tag( MINGW32_CATGETS ) };

    if( (read( fd, &test, sizeof( supported )) == sizeof( supported ))
    &&  (test.mc.magic_number == ((MSGCAT) supported).mc.magic_number)
    &&  (test.mc.version >= ((MSGCAT) supported).mc.version)
    &&  (lseek( fd, (off_t)(0), SEEK_SET ) == (off_t)(0))               )
      /*
       * Message catalogue passed validation;
       * leave the file open, and return the descriptor.
       */
      return fd;

    /* If we get to here,
     * message catalogue validation was unsuccessful,
     * so release the file descriptor...
     */
    close( fd );
  }

  /* ...and fall through, returning an invalid descriptor.
   */
  return (int)(-1);
}

static
int mc_pop_locale( int LC_TYPE, char *working_locale, int retval )
{
  /* This convenience function is used by `mc_nlspath_open', (below),
   * to restore the application's working locale, before returning the
   * status code specified by `retval'.
   */
  setlocale( LC_TYPE, working_locale );
  free( working_locale );
  return retval;
}

static
int mc_check_break_code( wchar_t chk, wchar_t *break_code )
{
  /* Helper function, called by `mc_nlspath_open;
   * it checks a given character against each of the
   * contextually valid delimiters, to establish where
   * to split the LC_MESSAGES string into components.
   */
  if( chk )
  {
    /* If we haven't run out of characters to parse...
     */
    while( *break_code )
    {
      /* Check the current character against each valid delimiter,
       * and return the character code if a match is found.
       */
      if( chk == *break_code++ )
	return (int)(chk);
    }
  }
  /* Fall through on no match,
   * returning zero, to tell `mc_nlspath_open' to keep parsing.
   */
  return 0;
}

static
int mc_nlspath_open( __const char *msgcat, unsigned flags )
{
# define NLS_LOCALE_STRING    (flags & NL_CAT_LOCALE) ? "LC_MESSAGES" : "LANG"
# define mc_select(PREF, ALT) (mblen((PREF), MB_CUR_MAX) > 0) ? (PREF) : (ALT)

  int fd, step, copy_index, headroom;
  wchar_t chk; __const char *subst, *nlspath, *chkptr;
  char nlsname[PATH_MAX + MB_LEN_MAX], *nlscopy;
  char *nls_locale = NULL;

  /* Save the application's working locale, so we can restore it later;
   * ( we need to change it temporarily, because...
   */
  char *saved_locale = strdup( setlocale( LC_CTYPE, NULL ) );

  /* ...we must parse message catalogue and NLSPATH specifiers in the
   * wchar_t domain, while using the system locale, to correctly handle
   * Win32 multibyte character path names ).
   */
  setlocale( LC_CTYPE, "" );
  if( (step = mbtowc( &chk, (chkptr = msgcat), MB_CUR_MAX )) > 0 )
  {
    /* First, check if the given `msgcat' spec begins with a pair of
     * characters which appear to represent a Win32 drive specifier.
     */
    if( (chk != L'/') && (chk != L'\\') )
    {
      /* We assume that it is, when the first character is *not* a
       * directory name separator, and the second *is* a colon...
       */
      chkptr += step;
      if( ((step = mbtowc( &chk, chkptr, MB_CUR_MAX )) > 0) && (chk == L':') )
	/*
	 * ...and we flag this condition, simply by pretending that
	 * we found a normal directory name separator, (slash).
	 */
	chk = L'/';
    }

    /* Scan the given `msgcat' spec, checking for directory name separators...
     */
    while( step > 0 )
    {
      if( (chk == L'/') || (chk == L'\\') )
      {
	/* The `msgcat' spec includes at least one directory name separator,
	 * so pass it back to `catopen', as an exact catalogue file reference.
	 */
	return mc_pop_locale( LC_CTYPE, saved_locale, mc_validate( msgcat ) );
      }
      step = mbtowc( &chk, (chkptr += step), MB_CUR_MAX );
    }

    /* The `msgcat' argument did not specify an explicit path name,
     * so we must search the NLSPATH, to locate a suitable candidate;
     * first check the environment for the user's NLSPATH specification,
     * falling back to the built in default, if none is defined.
     */
    if( (nlspath = getenv( "NLSPATH" )) == NULL )
      nlspath = NLSPATH_DEFAULT;

    /* Parse the NLSPATH, constructing prototypes for message catalogue paths
     * from each component path template in turn; try each generated prototype
     * in turn, for a valid message catalogue match; select and open the first
     * catalogue successfully matched, or bail out if no match found.
     */
    nlscopy = nlsname;
    headroom = sizeof( nlsname );
    do { step = mbtowc( &chk, nlspath, MB_CUR_MAX );
	 switch( chk )
	 {
	   case L'\0':
	   case NLSPATH_SEPARATOR_CHAR:
	     /*
	      *	We reached the end of the current NLSPATH template component;
	      * add a terminator, and attempt to validate a matching message
	      * catalogue; return it immediately if successful.
	      */
	     if( headroom >= MB_CUR_MAX )
	     {
	       wctomb( nlscopy, L'\0' );
	       if( (fd = mc_validate( mc_select( nlsname, msgcat ))) >= 0 )
		 return fd;
	     }
	     /* Couldn't find a valid message catalogue to match the current
	      * NLSPATH prototype; move on to the next template, if any, and
	      * try again.
	      */
	     nlspath += step;
	     nlscopy = nlsname;
	     headroom = sizeof( nlsname );
	     break;

	   case L'%':
	     /*
	      *	Found a substitution meta-character; need to interpret it.
	      */
	     nlspath += step;
	     nlspath += (step = mbtowc( &chk, nlspath, MB_CUR_MAX ));
	     switch( chk )
	     {
	       case L'%':
		 /*
		  * It's a literal `%' character; check we have sufficient space,
		  * then append it to the prototype for the message catalogue name
		  * which we are currently constructing.
		  */
		 if( headroom >= MB_CUR_MAX )
		 {
		   copy_index = wctomb( nlscopy, L'%' );
		   headroom -= copy_index;
		   nlscopy += copy_index;
		 }
		 else headroom = 0;
		 break;

	       case L'N':
		 /*
		  * This is a request to substitute the `msgcat' name, passed to
		  * this `catopen' request, into the assembled name prototype;
		  * again, check we have sufficient space, before proceeding.
		  */
		 subst = msgcat;
		 while( (copy_index = mbtowc( &chk, subst, MB_CUR_MAX )) > 0 )
		 {
		   if( headroom > copy_index )
		   {
		     headroom -= copy_index;
		     while( copy_index-- )
		       *nlscopy++ = *subst++;
		   }
		   else headroom = 0;
		 }
		 break;

	       case L'l':  /* the `language' specified for the locale */
	       case L't':  /* the associated `territory' designation */
	       case L'c':  /* and its `codeset' */
	       case L'L':  /* the entire locale specification */
		 /*
		  * These are requests to substitute the specified components
		  * of the current locale specification, either as specified by
		  * `LC_MESSAGES' or `LANG' definitions in the environment, if
		  * present, otherwise for the system locale.
		  */
		 if(  (nls_locale != NULL)
		 ||  ((nls_locale = getenv( NLS_LOCALE_STRING )) != NULL)
		 ||  ((nls_locale = setlocale( LC_MESSAGES, NULL )) != NULL)  )
		 {
		   subst = nls_locale;
		   wchar_t *break_code = L"_.@";

		   if( chk == L'L' )
		   {
		     break_code += 3;
		   }

		   else if( (chk == L't') || (chk == L'c') )
		   {
		     if( chk == L'c' )
		       ++break_code;
		     do subst += (copy_index = mbtowc( &chk, subst, MB_CUR_MAX ));
		     while( (copy_index > 0) && (chk != *break_code) );
		     if( *++break_code == L'@' )
		       ++break_code;
		   }

		   while( ((copy_index = mbtowc( &chk, subst, MB_CUR_MAX )) > 0)
		   &&     ! mc_check_break_code( chk, break_code )    )
		   {
		     if( headroom > copy_index )
		     {
		       headroom -= copy_index;
		       while( copy_index-- )
			 *nlscopy++ = *subst++;
		     }
		     else headroom = 0;
		   }
		 }
		 break;
	     }
	     break;

	   default:
	     /*
	      * Any regular character is simply copied to the constructed path,
	      * provided there is sufficient space available.
	      */	      
	     if( headroom >= step )
	     {
	       headroom -= step;
	       for( copy_index = 0; copy_index < step; copy_index++ )
		 *nlscopy++ = *nlspath++;
	     }
	     else headroom = 0;
	 }
       }
    while( step > 0 );
  }
    
  /* We fall through to here, if the `msgcat' spec was empty, or NULL;
   * so just return it as is, and leave `catopen' to clean up.
   */
  return mc_pop_locale( LC_CTYPE, saved_locale, (int)(-1) );
}

static
void *mc_open( struct mc_tab *cdt, va_list argv )
{
  /* This call-back function provides the actual `catopen' implementation.
   * It is invoked via `_mctab_', with `argv' specifying the actual path name
   * for the required message catalogue file, followed by the `catopen' flags.
   *
   * It is responsible for assigning a message catalogue descriptor, allocating
   * space in the descriptor table if necessary.  It then opens the requested
   * catalogue file, validating it as a valid message catalogue, allocates
   * memory, and loads the entire catalogue into the allocated space.
   */
  char *catname = va_arg( argv, char * );
  unsigned tab_increment, flags = va_arg( argv, unsigned );

  /* First, check the `catopen' flags, to determine how many new descriptor
   * table slots to create, if we need to expand the table.
   */
  if( (tab_increment = flags & NL_CATD_BLKSIZ_MAX) > 0 )
    cdt->grow_size = tab_increment;

  /* Find the first free slot, if any, in the descriptor table.
   */
  int retval = 0;
  while( (retval < cdt->curr_size) && (cdt->tab[ retval ].fd >= 0) )
    ++retval;

  /* Got an empty slot?
   */
  if( retval == cdt->curr_size )
  {
    /* No: we need to expand the table; this requires us to reallocate the
     * memory block, in which the table is stored, expanding it by the current
     * table increment size, bailing out on failure.
     */
    struct mc_descriptor *tmp = cdt->tab;
    int new_size = retval + cdt->grow_size;
    if( (tmp = mc_realloc( tmp, new_size * sizeof( struct mc_descriptor ))) == NULL )
      return (void *)(-1);

    /* After successfully resizing the message catalogue descriptor table,
     * initialise each new slot...
     */
    cdt->tab = tmp;
    while( retval < new_size )
    {
      /* Marking it as unused...
       */
      tmp[ retval ].fd = -1;

      /* and having no associated message data buffer...
       */
      tmp[ retval++ ].data = NULL;
    }

    /* then fix up the reference for the slot we will use now,
     * and adjust the record of the current table size.
     */
    retval = cdt->curr_size;
    cdt->curr_size = new_size;
  }

  /* Now, we have a message catalogue descriptor we can use,
   * so open the specified file, recording the file descriptor
   * within the message catalogue descriptor.
   */
  if( (cdt->tab[ retval ].fd = mc_nlspath_open( catname, flags )) >= 0 )
  {
    /* After successfully opening the message catalogue file,
     * we allocate the required memory for the message catalogue data buffer,
     * and load the entire contents of the file into this buffer.
     */
    struct stat catinfo;
    struct mc_descriptor *ref = cdt->tab + retval;
    if(  (fstat( ref->fd, &catinfo ) != 0)
    ||  ((ref->data = mc_malloc( catinfo.st_size )) == NULL)
    ||  ((read( ref->fd, ref->data, catinfo.st_size )) < catinfo.st_size)  )
    {
      /* If we get to here, we failed to load the message catalogue,
       * so mark the descriptor as unused, free any resources we allocated,
       * and fall through to bail out.
       */
      _mc_free_( ref );
    }
    else
      /*
       * We should now have a valid, and open, message catalogue descriptor;
       * it is actually of `nl_catd' type, but `_mctab_' requires us to return
       * a generic `void *' pointer; we do that, leaving `catopen' to cast it
       * to the ultimately expected `nl_catd' type.
       */
      return (void *) retval;
  }

  /* If we fall through to here, then something went wrong;
   * there should be nothing to clean up, so just return a `failed' status.
   */
  return (void *)(-1);
}

/* The actual `catopen' implementation is trivial...
 */
nl_catd catopen( __const char *name, int flags )
{
  /* ...with all the hard work done by the `mc_open' call-back from `_mctab_'.
   */
  return (nl_catd)_mctab_( mc_open, name, flags );
}

/* $RCSfile: catopen.c,v $Revision: 1.3 $: end of file */
