/*
 * catopen.c
 *
 * $Id: catopen.c,v 1.6 2007-11-11 17:31:31 keithmarshall Exp $
 *
 * Copyright (C) 2006, Keith Marshall
 *
 * This file implements the `catopen' function, required to support
 * POSIX compatible national language message catalogues in MinGW.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last modification: 10-Nov-2007
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

#ifdef HAVE_WINDOWS_H
#include <windows.h>
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

#ifdef _WIN32
/*
 * On Win32 platforms, we need to work around the lack of any definitive
 * standard for a file system hierarchy; to accomplish this, we need some
 * additional helper functions, for analysing and modifying path names,
 * performing all operations in the wide character domain.
 */
#define mc_is_dirsep(C)  (((C) == L'/') || ((C) == L'\\'))

static inline
size_t mc_dirname_strip( wchar_t *path, size_t len )
{
  /* Local inline helper to discard trailing dirname separators
   * from the end of a path name in the wide character domain.
   */
  while( (len > 0) && mc_is_dirsep( path[len] ) )
    path[len--] = L'\0';
  return len;
}

static inline
size_t mc_dirname_len( wchar_t *path, size_t len )
{
  /* Local inline helper to establish the length of the `dirname'
   * component of a path name in the wide character domain.
   */
  len = mc_dirname_strip( path, len );
  while( (len > 0) && ! mc_is_dirsep( path[len] ) )
    --len;
  return len;
}

static inline
size_t mc_mbstowcs( wchar_t *buf, size_t len, const char *mbs )
{
  /* Inline helper to convert path names from the multibyte character
   * domain defined by the system code page, to the wide character domain.
   */
  return MultiByteToWideChar( CP_ACP, 0, mbs, (size_t)(-1), buf, len );
}

static inline
size_t mc_wcstombs( char *buf, size_t len, const wchar_t *wcs )
{
  /* Inline helper to convert path names from the wide character domain
   * to the multibyte character domain defined by the system code page.
   */
  return WideCharToMultiByte( CP_ACP, 0, wcs, (size_t)(-1), buf, len, NULL, NULL );
}

static
int mc_validate_mapped( __const char *name )
{
  /* Wrapper function for calls to `mc_validate', on Win32 hosts.
   * We assume that, for an application installed into a directory
   * designated by `${prefix}', that its message catalogues will be
   * in `${prefix}/name'; thus, for any `name' argument which is in
   * the form of a POSIX absolute path name, (i.e. it begins with a
   * dirname separator as its first character), we deduce `${prefix}'
   * from the path name of the executable file, and we prefix it to
   * the `name' argument, before passing this modified argument to
   * `mc_validate', for consideration as a possible location for
   * the message catalogue to be opened.
   */
  static char *mapped_name = NULL;
  static size_t root_len = (size_t)(-1);

  wchar_t chk;

  if( name == NULL )
  {
    /* No catalogue path name to map;
     * we may assume that this is a request to...
     */
    if( mapped_name != NULL )
    {
      /* ...release temporary resources, allocated while
       * mapping catalogue paths into the file system space
       * in which the calling application is installed.
       */
      free( mapped_name );
      mapped_name = NULL;
      root_len = (size_t)(-1);
    }
    /* If this was not a deliberate request to free resources,
     * then an error has occurred, so return accordingly.
     */
    return (int)(-1);
  }

  /* If we get to here,
   * then we have a message catalogue to locate...
   */
  if( (mbtowc( &chk, name, MB_CUR_MAX ) > 0) && ! mc_is_dirsep( chk ) )
    /*
     * The message catalogue path specified in this request is
     * not absolute, as a POSIX path, so handle it literally.
     */
    return mc_validate( name );

  /* ...and if here, then the path is specified by an
   * absolute POSIX style path name...
   */
  if( mapped_name == NULL )
  {
    /* ...but we don't yet know the Win32 path name for the effective
     * root directory in our emulated POSIX file system hierarchy; we
     * must deduce this from the path name of the calling executable.
     */
    size_t path_len = strlen( name ) + 1;
    wchar_t chroot[ root_len = mc_mbstowcs( NULL, 0, _pgmptr ) ];
    (void) mc_mbstowcs( chroot, root_len, _pgmptr );

    /* Discard the executable file name, from the end of this path name.
     */
    root_len = mc_dirname_len( chroot, root_len - 1 );
    if( mc_is_dirsep( chroot[ root_len ] ) )
    {
      size_t offset;

      /* When we have distinct dirname and basename components, then we
       * discard the basename, and also any newly exposed trailing dirname
       * separator characters.
       */
      root_len = mc_dirname_len( chroot, root_len );

      /* Check if the leaf directory name is `bin' or `sbin'...
       * The executable may be installed as `${prefix}/bin/appname.exe',
       * or as `${prefix}/sbin/appname.exe'; thus, to identify `${prefix}',
       * we must prune away this final directory name, and its preceeding
       * dirname separator(s), in addition to the executable name, which
       * we've already discarded.
       */
      offset = root_len + 1;
      if( (chroot[ offset ] == L's') || (chroot[ offset ] == L'S') )
	++offset;
      if( _wcsicmp( chroot + offset, L"bin" ) == 0 )
	(void) mc_dirname_strip( chroot, root_len );
    }

    /* Transform this name back to the multibyte character domain,
     * computing the resultant length of the application root prefix;
     * allocate memory to accommodate it, with the message catalogue
     * path name concatenated, and populate it.
     */
    root_len = mc_wcstombs( NULL, 0, chroot ) - 1;
    mapped_name = mc_realloc( mapped_name, root_len + path_len );
    (void) mc_wcstombs( mapped_name, root_len + 1, chroot );
    (void) memcpy( mapped_name + root_len, name, path_len );
  }

  else
  {
    /* We had already identified the application's root prefix,
     * during a previous search, so now we simply adjust the memory
     * allocation, and insert the new message catalogue path name,
     * at the appropriate offset.
     */
    size_t path_len = strlen( name ) + 1;
    mapped_name = mc_realloc( mapped_name, root_len + path_len );
    (void) memcpy( mapped_name + root_len, name, path_len );
  }

  /* However we arrived at the mapped message catalogue path name,
   * we hand it off for validation.
   */
  return mc_validate( mapped_name );
}

/* Throughout the remainder of this module,
 * redirect references to `mc_validate' through `mc_validate_mapped'.
 */
#define mc_validate( name )  mc_validate_mapped( name )

static inline
nl_catd mc_resolved( nl_catd fd )
{
  /* A simple wrapper function, which allows `catopen' to release
   * the memory temporarily allocated by `mc_validate_mapped', when
   * searching for message catalogues on Win32 file systems.
   */
  (void) mc_validate_mapped( NULL );
  return fd;
}

#else
/*
 * On non-Win32 platforms, there are no file system hierarchy issues
 * to be resolved, so `mc_resolved' may simply evaluate its argument,
 * and transparently leave the result inline.
 */
#define mc_resolved( expr )  expr

/* On these platforms, we accept only '/' as the dirname separator.
 */
#define mc_is_dirsep(C)  ((C) == L'/')
#endif

static inline
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
char *mc_nl_cat_locale_getenv( int flags )
{
  char *retptr;

  /* Retrieve any appropriate LC_MESSAGES or LANG setting string
   * from the process environment, for use in interpreting NLSPATH.
   */
  if( flags & NL_CAT_LOCALE )
  {
    /* When this flag is set, then we use the LC_MESSAGES setting,
     * if any, noting that this may be overridden by LC_ALL.
     */
    if( (((retptr = getenv( "LC_ALL" )) != NULL) && (*retptr != '\0'))
    ||  (((retptr = getenv( "LC_MESSAGES" )) != NULL) && (*retptr != '\0'))  )
      return retptr;
  }
  /* When NL_CAT_LOCALE is not set, or if LC_MESSAGES is not defined,
   * fall back to using the LANG specification.
   */
  if( ((retptr = getenv( "LANG" )) != NULL) && (*retptr != '\0') )
    return retptr;

  /* Fall through, if there is no appropriate specification
   * in the process environment, returning NULL.
   */
  return NULL;
}

static
int mc_nlspath_open( __const char *msgcat, unsigned flags )
{
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
    if( ! mc_is_dirsep( chk ) )
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
      if( mc_is_dirsep( chk ) )
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
		 ||  ((nls_locale = mc_nl_cat_locale_getenv( flags )) != NULL)
		 ||  ((nls_locale = setlocale( LC_MESSAGES, NULL )) != NULL)    )
		 {
		   wchar_t *break_code = L"_.@";
		   subst = nls_locale;

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
  int retval = 0;

  /* First, check the `catopen' flags, to determine how many new descriptor
   * table slots to create, if we need to expand the table.
   */
  if( (tab_increment = flags & NL_CATD_BLKSIZ_MAX) > 0 )
    cdt->grow_size = tab_increment;

  /* Find the first free slot, if any, in the descriptor table.
   */
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
  return mc_resolved( (nl_catd)_mctab_( mc_open, name, flags ) );
}

/* $RCSfile: catopen.c,v $Revision: 1.6 $: end of file */
