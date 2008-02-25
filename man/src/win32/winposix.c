/* Copyright (C) 2005
 *   Written by Keith Marshall (keithmarshall@users.sourceforge.net)
 *
 * Provides:
 *   char* win32_path_transform( const char* pathname );
 *
 *     This function replaces all occurrences of the "\" character,
 *     in the specified Win32 "pathname", with the "/" character.
 *
 *   int win32_path_is_absolute( const char* pathname );
 *
 *     This function determines if the specified Win32 "pathname"
 *     represents an absolute location in the Win32 file system.
 *
 *   const char* win32_map_posix_path_name( const char* pathname );
 *
 *     This function maps the POSIX style absolute path name
 *     specified by "pathname" to an equivalent Win32 path name,
 *     with an effective virtual root of the POSIX file system
 *     determined relative to the installation directory of
 *     the calling application exeecutable.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY of any kind; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Please refer
 * to the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin St - Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */
#define  WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <compat.h>
#include <stdlib.h>

static inline
size_t win32_mbsconv( wchar_t *buf, size_t len, const char *mbs )
{
  /* Inline helper to convert path names from the multibyte character
   * domain defined by the system code page, to the wide character domain.
   */
  return MultiByteToWideChar( CP_ACP, 0, mbs, (size_t)(-1), buf, len );
}

static inline
size_t win32_mbsconv_len( const char *s )
{
  /* Inline helper to determine the size of the buffer needed to store
   * the wide character representation of a path name, when converted from
   * the multibyte character domain defined by the system code page.
   */
  return win32_mbsconv( NULL, 0, s );
}

static inline
size_t win32_wcsconv( char *buf, size_t len, const wchar_t *wcs )
{
  /* Inline helper to convert path names from the wide character domain
   * to the multibyte character domain defined by the system code page.
   */
  return WideCharToMultiByte( CP_ACP, 0, wcs, (size_t)(-1), buf, len, NULL, NULL );
}

static inline
size_t win32_wcsconv_len( const wchar_t *wcs )
{
  /* Inline helper to determine the size of the buffer needed to store
   * the multibyte character representation of a path name, in the codeset
   * specified by the system code page, when converted back from the wide
   * character domain.
   */
  return win32_wcsconv( NULL, 0, wcs );
}

static inline
int is_dirsep (wchar_t ch)
{
  /* Inline helper to determine if a specified wide character represents
   * either of the standard dirname separator characters.
   */
  return (ch == L'/') || (ch == L'\\');
}

#define win32_get_dirname_separator() L'/'

static
const wchar_t *wcs_normalised_path( wchar_t *path )
{
  /* Normalise a string, representing a Win32 path name in the wide
   * character domain, stripping off any trailing dirname separators,
   * and replacing any leading or contained runs of such characters
   * by a single instance of the preferred dirname separator, (two
   * such separators are retained, in the case of repeated leading
   * separators).  Normalisation is performed `in place', and all
   * retained dirname separators are consistently replaced by slash
   * or backslash, as established by `win32_set_dirname_separator()',
   * (with backslash as default).
   */

  if( path != NULL )
  {
    wchar_t *src = (wchar_t *)(path);
    wchar_t *dest = (wchar_t *)(path);
    wchar_t dirsep = win32_get_dirname_separator();

    if( is_dirsep( *src ) )
    {
      /* The initial character is a dirname separator;
       * normalise it, but to accommodate a possible UNC path
       * ensure that we don't normalise away the following
       * character, if it is a second separator.
       */
      ++src; *dest++ = dirsep;
    }

    /* Scan the input path name string...
     */
    do { while( is_dirsep( *src ) )
         {
	   /* ...normalising and collapsing contiguous runs
	    * of dirname separator characters...
	    */
	   *dest = dirsep;
           ++src;
	 }
	 if( *src && (*dest == dirsep) )
	 {
	   /* ...retaining any internal, but not any trailing
	    * dirname separators...
	    */
	   ++dest;
	 }
	 /* ...and rewriting `in place'...
	  */
	 *dest++ = *src;

	 /* ...inclusively, until we reach the terminating NUL.
	  */
       } while( *src++ );
  }

  /* Return the normalised path name string.
   */
  return path;
}

char *win32_path_transform( const char *pathname )
{
  /* Public function to fix up Win32 path names, so that they conform
   * to the POSIX convention of using "/" as the directory separator,
   * (but leave "D:" as a drive designator, if it is present).
   */
  if( pathname && *pathname )
  {
    size_t len = win32_mbsconv_len( pathname );
    wchar_t path[len]; win32_mbsconv( path, len, pathname );

    len = win32_wcsconv_len( wcs_normalised_path( path ) );
    if( len < strlen( pathname ) + 2 )
      (void) win32_wcsconv( (char *)(pathname), len, path );
  }
  return (char *) pathname;
}

int win32_path_is_absolute( const char *pathname )
{
  /* Public function to check if a specified Win32 path,
   * which may, or may not have been transformed by win32_path_transform(),
   * represents an absolute reference to a file system location.
   */
  size_t len = win32_mbsconv_len( pathname );
  wchar_t path[len]; win32_mbsconv( path, len, pathname );
  return is_dirsep( path[0] ) || ((path[1] == L':') && is_dirsep( path[2] ));
}

static inline
size_t approot_len( size_t set )
{
  /* Private function encapsulating storage for the length of the
   * application's `root path name' string, and providing read/write
   * access for `win32_application_root()' to set it.
   */
  static size_t value =  (size_t)(0);

  if( set > (size_t)(0) )
    value = set - (size_t)(1);
  return value;
}

static inline
size_t win32_application_root_length( void )
{
  /* Public accessor function, providing read-only look up for
   * the length of the application's `root path name' string, as
   * set by `win32_application_root()'.
   */
  return approot_len( (size_t)(0) );
}

static inline
size_t dirname_len( const wchar_t *path, size_t len )
{
  /* Local inline helper to establish the length of the `dirname'
   * component of a path name in the wide character domain.
   */
  while( (len > 0) && ! is_dirsep( path[len] ) )
    --len;
  return len;
}

static inline
size_t dirname_strip( wchar_t *path, size_t len )
{
  /* Local inline helper to discard trailing dirname separators
   * from the end of a path name in the wide character domain.
   */
  while( (len > 0) && is_dirsep( path[len] ) )
    path[len--] = L'\0';
  return len;
}

static
const char *win32_application_root( void )
{
  /* `locale-safe' function for finding an application's root path.
   */
  static char *approot = NULL;

  if( approot == NULL )
  {
    /* The application's root path has not yet been evaluated;
     * establish a local buffer, in which to construct a wchar_t
     * representation of the normalised application root path name,
     * derived from the MSVCRT global variable `_pgmptr'.
     */
    size_t len; wchar_t buf[len = win32_mbsconv_len( _pgmptr )];

    /* Copy the content of `_pgmptr' to this buffer, transforming it to
     * the wchar_t domain in the process, and adjust `len' to the offset
     * of its last character.
     */
    if( buf[len = win32_mbsconv( buf, len, _pgmptr ) - 1] == L'\0' )
      --len;

    /* Discard any trailing dirname separator characters, and locate the
     * final dirname separator, preceding the basename of the residual path.
     */
    len = dirname_len( buf, dirname_strip( buf, len ) );
    if( is_dirsep( buf[len] ) )
    {
      size_t offset;

      /* When we have distinct dirname and basename components, then we
       * discard the basename, and also any newly exposed trailing dirname
       * separator characters.
       */
      len = dirname_len( buf, dirname_strip( buf, len ) );

      /* Check if the leaf directory name is `bin' or `sbin'...
       */
      offset = len + 1;
      if( (buf[offset] == L's') || (buf[offset] == L'S') )
	++offset;
      if( _wcsicmp( buf + offset, L"bin" ) == 0 )
	/*
	 * ...and if so, drop it, to retain just the effective
	 * `prefix' directory path for the application.
	 */
	(void) dirname_strip( buf, len );
#     if 0
      {
	/* ...and if so, drop it and check for a `local' parent...
	 */
	len = dirname_len( buf, dirname_strip( buf, len ) );
	if( is_dirsep( buf[len] ) && (_wcsicmp( buf + len + 1, L"local" ) == 0) )
	{
	  /* ...also dropping that...
	   */
	  len = dirname_len( buf, dirname_strip( buf, len ) );
	}
	/* ...together with any `usr' parent.
	 */
	if( is_dirsep( buf[len] ) && (_wcsicmp( buf + len + 1, L"usr" ) == 0) )
	  (void) dirname_strip( buf, len );
      }
#     endif
    }

    /* Transform back to the multibyte character domain, and cache the
     * resultant path name together with its length, for immediate and
     * possible later return.
     */
    len = win32_wcsconv_len( wcs_normalised_path( buf ) );
    if( (approot = malloc( len )) != NULL )
      (void) approot_len( win32_wcsconv( approot, len, buf ) );
  }

  /* Whether evaluated on this occasion, or by a previous call,
   * return the resultant path name, as stored in the local cache.
   */
  return approot;
}

const char *win32_map_posix_path_name( const char *name )
{
  /* Resolve the absolute Win32 path name for a specified absolute
   * POSIX-style path name, where this POSIX-style path is deemed to
   * be absolute w.r.t. the `application root' directory.
   *
   * The return string is dynamically allocated, but with a local
   * reference pointer, thus the caller should not free it; call this
   * function again, with `name == NULL', to free the memory.
   */
  static char *retpath = NULL;

  if( retpath != NULL )
  {
    /* We have a result left over from a previous call;
     * first we must discard it, and free the associated memory.
     */
    free( (void *)(retpath) );
    retpath = NULL;
  }

  if( name != NULL )
  {
    /* We have a new path name to map;
     * transform to the wide character domain and normalise.
     */
    size_t len; wchar_t buf[len = win32_mbsconv_len( name )];

    (void) win32_mbsconv( buf, len, name );
    len = win32_wcsconv_len( wcs_normalised_path( buf ) );

    if( is_dirsep( *buf ) )
    {
      /* Provided the input path name is representative of
       * an *absolute* POSIX path name, we make it relative to
       * the application root...
       */
      const char *prefix = win32_application_root();
      size_t prefix_len = win32_application_root_length();

      if( (retpath = malloc( prefix_len + len )) != NULL )
      {
	/* ...transforming back to the multibyte character domain,
	 * within the buffer allocated for return.
	 */
	memcpy( retpath, prefix, prefix_len );
	(void) win32_wcsconv( retpath + prefix_len, len, buf );
      }
    }

    else if( (retpath = malloc( len )) != NULL )
    {
      /* The input path *isn't* an absolute POSIX path,
       * so simply make a normalised copy of the input path name
       * in the smaller buffer allocated for return.
       */
      (void) win32_wcsconv( retpath, len, buf );
    }
  }

  /* Return the normalised path name, now stored in `retpath'.
   */
  return retpath;
}

/* $RCSfile: winposix.c,v $: end of file */
