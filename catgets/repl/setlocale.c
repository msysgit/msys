/*
 * setlocale.c
 *
 * $Id: setlocale.c,v 1.1 2007-04-06 22:34:56 keithmarshall Exp $
 *
 * Provides the `_mingw_setlocale' function, which wraps the standard MSVCRT
 * `setlocale' function, offering extended support for locale specifications
 * defined in environment variables, with ISO-639/ISO-3166 syntax capability,
 * and rudimentary support for the LC_MESSAGES category, required by POSIX,
 * but which is unrecognised in standard Win32; (this extension supports it,
 * by mapping it internally to LC_CTYPE).
 *
 *
 * Contributed by:  Keith Marshall  <keithmarshall@users.sourceforge.net>
 *
 *
 * This module is provided as a component of the MinGW Runtime Library.
 * This is free software.  The original author hereby relinquishes copyright
 * on the source code, releasing it to the public domain.  You may copy, modify
 * and/or redistribute this software freely, without restriction of copyright.
 *
 * This software is provided "as is", in the hope that it may be useful,
 * but WITHOUT WARRANTY OF ANY KIND, not even any implied warranty of
 * MERCHANTABILITY, nor of FITNESS FOR ANY PARTICULAR PURPOSE.  At no
 * time will the author accept any form of liability for any damages,
 * however caused, resulting from the use of this software.
 *
 *
 * Author's Note:
 *   This software is based on a technique proposed to me by Tor Lillqvist.
 * In developing it, I have drawn extensively on documentation which is made
 * freely available on MSDN; I have referred to no other software, however
 * licensed, as any source of inspiration or reference.
 *
 */
#define WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <string.h>
#include <windows.h>
/*
 * Enable MinGW extensions to standard Microsoft locale support; i.e. specify
 * that we wish to include `MINGW32_LC_MESSAGES' and `MINGW32_LC_ENVVARS' as
 * supported `MINGW32_LC_EXTENSIONS'.
 */
#define MINGW32_LC_EXTENSIONS  MINGW32_LC_MESSAGES
/*
 * Note that we didn't include `MINGW32_LC_ENVVARS' here!  It is implicit
 * within the implementation, but must *not* be specified explicitly; doing
 * so would cause `_mingw_setlocale' to recursively call itself, instead of
 * calling the MSVCRT `setlocale' function, as intended.  This recursion
 * would loop indefinitely, until the application crashes as a result of
 * stack overflow.
 */
#include <locale.h>

#ifndef MINGW32_LC_MESSAGES
/*
 * The version of MinGW's locale.h distributed to support this extension
 * should have defined this already; catch any attempts to compile against
 * an obsolete version of the header.
 */
#warning locale.h does not implement MINGW32_LC_EXTENSIONS
#error   cannot compile; MINGW32_LC_EXTENSIONS are required
#define  MINGW32_LC_MESSAGES  0
#endif

#ifndef LC_MESSAGES
/*
 * Like MINGW32_LC_MESSAGES, this should have been defined in locale.h,
 * but might not be, if we used an obsolete version of the header file,
 * so better make sure.
 */
#define LC_MESSAGES LC_CTYPE
#endif

#ifndef MINGW32_LC_REMAP
/*
 * We need this macro, to normalise the LC category code passed to `setlocale',
 * but, like those above, it requires a locale.h with MINGW32_LC_EXTENSIONS.
 * If we don't have it, define it to pass an unmodified LC code.
 */
#define MINGW32_LC_REMAP(LC_CATEGORY)  (LC_CATEGORY)
#endif

/*
 * Function: nl_category_name
 *
 * Description:
 *   Map the locale category codes, defined in locale.h, to the standard
 *   names for the environment variables POXIX uses to represent them.
 *
 * Usage:
 *   name = nl_category_name( category )
 *
 * Inputs:
 *   category  (int)  the locale category code defined in locale.h.
 *
 * Global Input References:
 *   None.
 *
 * Returns:
 *   A statically allocated string, representing the the POSIX environment
 *   variable name associated with the specified category, or NULL in the
 *   case of an invalid category specification.
 */
static
char *nl_category_name( int category )
{
  static
  char *nl_category_names[] = 
  {
    /* This is the list of names for the standard categories; they must
     * be listed in the order the categories are enumerated in locale.h
     */
    "LC_ALL",
    "LC_COLLATE",
    "LC_CTYPE",
    "LC_MONETARY",
    "LC_NUMERIC",
    "LC_TIME"
  };

  /* Handle LC_MESSAGES as a special case...
   * ( it is *not* a standard Win32 locale category ).
   */
  if( (category & MINGW32_LC_MESSAGES) && (category == LC_MESSAGES) )
    return "LC_MESSAGES";
  /*
   * Confirm that the requested category index is valid,
   * returning a NULL pointer if not.
   */
  if( (unsigned)(category) > LC_MAX )
    return NULL;
  /*
   * Otherwise, simply return the name from the standard list.
   */
  return nl_category_names[ category ];
}

/* We need to provide global storage for `nl_locale_id' and `nl_env_name',
 * because we must manipulate them within a callback function used by the
 * Microsoft `EnumSystemLocales' API, which makes no provision for passing
 * data between the callback and the caller; however, we can keep them
 * private, (exposed only within this compilation unit).
 *
 *
 * `nl_env_name' is an array of pointers to the elements of a locale specification,
 * as it is defined in the process environment.
 */
static char *nl_env_name[4];
enum { LC_ENUM_LANGUAGE = 0, LC_ENUM_TERRITORY, LC_ENUM_CODESET, LC_ENUM_MODIFIERS };
/*
 * `nl_locale_id' is set by the callback function `nl_locale_id_lookup', to allow
 * the Microsoft LCID matching `nl_env_name' to be passed back to the caller of the
 * `EnumSystemLocales' API function; we also define the `LCID_UNKNOWN' value, which
 * we return if we cannot map `nl_env_name' to any Microsoft LCID.
 */
static  LCID nl_locale_id;
#define LCID_UNKNOWN (LCID)(-1)

/*
 * Function: nl_locale_lookup
 *
 * Description: 
 *   A private helper function, invoked indirectly through the `EnumSystemLocales'
 *   API, to retrieve the Microsoft LCID matching a given locale specification.
 *
 * Usage (not to be invoked directly from user code):
 *   nl_locale_lookup( LPTSTR locale_ref )
 *
 * Inputs:
 *   locale_ref (LPTSTR)    potential LCID match, encoded as a hexadecimal string
 *
 * Global Input References:
 *   nl_env_name (char **)  list of locale specification components to match
 *
 * Returns:
 *   Status code for `EnumSystemLocales', indicating whether or not `locale_ref'
 *   successfully matched the locale specification given in `nl_env_name, so that
 *   searching for a match can continue if required, or stop immediately when a
 *   match is found; global variable `nl_locale_id' is set to the matching LCID
 *   on a successful match, or to `LCID_UNKNOWN' otherwise.
 */
static
BOOL CALLBACK nl_locale_lookup( LPTSTR locale )
{
  /* This callback is invoked via `EnumSystemLocales',
   * to identify and save the LCID for the system locale, (if any),
   * which matches the ISO-639 language and ISO-3166 territory codes set by
   * the user, using the POSIX style LC_CATEGORY environment variables.
   */

#  define NL_ISO_NAME_LEN        9
#  define NUM_BASE_HEXADECIMAL  16

#  define LC_ENUM_RESOLVED       0
#  define LC_ENUM_TRY_AGAIN      1

  /* Aarrrrrrgh!!!
   * `EnumSystemLocales' gives us the LCID as a string, (representing the
   * numeric value in hexadecimal format), but we need it as a numeric LCID,
   * which is an unsigned long int.
   */

  char *ep;
  LCID lcid = strtoul( locale, &ep, NUM_BASE_HEXADECIMAL );

  /* Check to confirm if the ISO-639/ISO-3166 identity codes for this LCID
   * match the language and territory settings we found in the environment;
   * if not bail out, requesting `EnumSystemLocales' to try another LCID.
   */
  char iso_name[ NL_ISO_NAME_LEN ];
  if( (GetLocaleInfo( lcid, LOCALE_SISO639LANGNAME, iso_name, NL_ISO_NAME_LEN ))
  &&  (strcmp( iso_name, nl_env_name[ LC_ENUM_LANGUAGE ] ) != 0)  )
    return LC_ENUM_TRY_AGAIN;

  if( (nl_env_name[ LC_ENUM_TERRITORY ] != NULL)
  &&  (GetLocaleInfo( lcid, LOCALE_SISO3166CTRYNAME, iso_name, NL_ISO_NAME_LEN ))
  &&  (strcmp( iso_name, nl_env_name[ LC_ENUM_TERRITORY ] ) != 0)  )
    return LC_ENUM_TRY_AGAIN;

  /* If we get to here, then we found the LCID which corresponds to
   * the ISO language and territory codes specified in the environment;
   * save the matching LCID where our callback initiator can find it.
   */
  nl_locale_id = lcid;
  return LC_ENUM_RESOLVED;
}

/*
 * Function: nl_locale_id_lookup
 *
 * Description:
 *   A private helper function; it wraps a call to the `EnumSystemLocales' API,
 *   to make an `nl_locale_lookup' callback, ensuring that the `nl_locale_id'
 *   global variable is set to `LCID_UNKNOWN', in the event that the requested
 *   LCID lookup is unresolved.
 *
 * Usage:
 *   nl_locale_id_lookup( void )
 *
 * Inputs:
 *   None.
 *
 * Global Input References:
 *   nl_env_name (char **)  list of locale specification components to match
 *
 * Returns:
 *   Global variable `nl_locale_id' set to the LCID for the locale which
 *   matches `nl_env_name', if any, or to `LCID_UNKNOWN' otherwise; this
 *   also becomes the function return value.
 */
static
LCID nl_locale_id_lookup()
{
  /* Initialise the global return value repository for a failed lookup,
   * before handing the request to `nl_locale_lookup', via `EnumSystemLocales.
   */
  nl_locale_id = LCID_UNKNOWN;
  EnumSystemLocales( nl_locale_lookup, LCID_SUPPORTED );
  /*
   * Retrieve the eventual return value from the global repository.
   */
  return nl_locale_id;
}

/*
 * Function: nl_locale_name_lookup
 *
 * Description:
 *   A private helper function; used by `_mingw_setlocale' to encode the
 *   language and territory components for a given LCID, in a format which
 *   facilitates the construction of a Win32 compatible locale specification;
 *   uses Microsoft's `GetLocaleInfo' API to query the locale database.
 *
 * Usage:
 *   name = nl_locale_name_lookup( prefix, lcid, classification )
 *
 * Inputs:
 *   prefix         (char *)  text to paste into the return string, in
 *                            front of the locale component name.
 *   lcid           (LCID)    the LCID code of the locale to query.
 *   classification (int)     the `GetLocaleInfo' query index for the
 *                            component name to be retrieved; for this
 *                            application it is `LOCALE_SENGLANGUAGE'
 *                            or `LOCALE_SENGCOUNTRY'.
 *
 * Global Input References:
 *   None.
 *
 * Returns:
 *   A dynamically allocated buffer, containing the requested locale
 *   component name, appropriately prefixed, or a statically allocated
 *   empty string, if the requested component name cannot be resolved.
 */
static
char *nl_locale_name_lookup( char *prefix, LCID lcid, int classification )
{
  /* Allocate a temporary local buffer, for construction of the return string.
   */
  int minlen = prefix ? strlen( prefix ) : 0;
  int maxlen = GetLocaleInfo( lcid, classification, NULL, 0 );
  char retstring[ minlen + maxlen ];
  /*
   * Insert the specified prefix text, if any, into the temporary buffer.
   */
  if( minlen > 0 )
    strcpy( retstring, prefix );
  /*
   * Look up the requested locale component name, appending it to any prefix
   * text in the local buffer; on success, copy the local buffer to dynamically
   * allocated storage, for return.
   */
  if( GetLocaleInfo( lcid, classification, retstring + minlen, maxlen ) > 0 )
    return strdup( retstring );
  /*
   * Fall through on lookup failure,
   * returning a statically allocated empty string.
   */
  return "";
}

/*
 * Function: nl_codeset_normalise
 *
 * Description:
 *   A private helper function; it is used by `_mingw_setlocale' to format
 *   a given codepage specification in a form suitable for use in a Win32
 *   compatible locale specification, to be passed to `setlocale'.
 *
 *   The given codepage specification my include an optional case blind
 *   `CP' prefix, followed by the codepage number; if present, this prefix
 *   is removed from the normalised return string.
 *
 * Usage:
 *   name = nl_codeset_normalise( prefix, codeset )
 *
 * Inputs:
 *   prefix   (char *)  optional text to place before the normalised
 *                      codepage reference, in the return string.
 *   codeset  (char *)  the codepage number, encoded as a decimal string,
 *                      with optional `CP' prefix.
 *
 * Global Input References:
 *   None.
 *
 * Returns:
 *   A dynamically allocated buffer, containing the normalised codepage
 *   reference, with specified prefix attached, or a statically allocated
 *   empty string, if no codepage is specified.
 */
static
char *nl_codeset_normalise( char *prefix, char *codeset )
{
  if( codeset != NULL )
  {
    /* Codeset was specified ...
     * Strip any optional `CP' prefix from the input specification,
     * to obtain the normalised codepage reference.
     */
    if( (tolower( codeset[0] ) == 'c') && (tolower( codeset[1] ) == 'p') )
      codeset += 2;
    /*
     * Check for a non-empty residual codepage specification ...
     */
    if( *codeset )
    {
      /* And, when present, handle any additional prefix specification ...
       */
      if( (prefix != NULL) && *prefix )
      {
	/* A non-empty prefix string was specified ...
	 * Here, we allocate a temporary local buffer in which we will
	 * assemble the return string.
	 */
	char retstring[ 1 + prefix ? strlen( prefix ) : 0 + strlen( codeset ) ];
	/*
	 * Now we copy the residual codeset specification,
	 * together with the additionally specified prefix string,
	 * into the temporary local buffer, and hence to a
	 * dynamically allocated buffer for return.
	 */
	return strdup( strcat( strcpy( retstring, prefix ), codeset ));
      }
      else
	/*
	 * When there is no additional prefix specified,
	 * we simply copy the residual normalised codepage specification
	 * to a dynamically allocated buffer for return.
	 */
	return strdup( codeset );
    }
  }
  /* If we didn't have anything to return,
   * fall through, to return a statically allocated empty string.
   */
  return "";
}

/*
 * Function: _mingw_setlocale
 *
 * Description:
 *   A publically available wrapper for the system supplied `setlocale' function,
 *   adding support for locale specifications defined in the process environment.
 *
 *   Any compilation unit which defines `MINGW32_LC_EXTENSIONS', with a bitwise
 *   OR'ed value which includes `MINGW32_LC_ENVVARS', *before* #including the
 *   locale.h header, will redirect calls to `setlocale' through this wrapper.
 *   If this definition of `MINGW32_LC_EXTENSIONS' also includes the bit pattern
 *   specified by `MINGW32_LC_MESSAGES', then `setlocale' calls will recognise the
 *   LC_MESSAGES category, setting LC_CTYPE to the locale specified for this
 *   additional, (non-standard in Win32), category.
 *
 * Usage:
 *   spec = _mingw_setlocale( category, locale )
 *
 * Inputs:
 *   category  (int)     any one of the locale category codes defined in locale.h,
 *                       as specified for the `setlocale' function itself.
 *   locale    (char *)  text specifying the locale to activate, as specified for
 *                       the `setlocale' function; extends default behaviour to
 *                       accommodate ISO-639/ISO-3166 language/territory specs.
 *
 * Global Input References:
 *   None.
 *
 * Returns:
 *   The string returned by `setlocale', when called with the same values of the
 *   `category' and `locale' arguments, after substitution of locale specifications
 *   from the process environment, if `locale' is passed as an empty string, and
 *   transformation of ISO-639/ISO-3166 codes to Win32 compatible equivalents.
 */ 
char *_mingw_setlocale( int category, const char *locale )
{
  /* Publically exposed wrapper function, intercepting calls to `setlocale'
   * so we can support the POSIX compliant mechanism for specifying locales,
   * using environment variables.
   */
  if( locale && (*locale == '\0') )
  {
    /* This is the `setlocale( LC_CATEGORY, "" )' case,
     * which needs special handling, for POSIX-like behaviour;
     * we must get the `locale' specification by inspection of the environment,
     * using specifications from variables prioritised as follows:
     *
     * LC_ALL overrides all individual settings;
     * LC_CTYPE, LC_COLLATE, etc. apply to their respective
     * individual categories, if LC_ALL is undefined;
     * LANG is used as a final fall back provision,
     * while default Microsoft behaviour is last resort.
     */
    char *ref;
    if( ((ref = getenv( nl_category_name( LC_ALL ))) != NULL)
    ||  ((ref = getenv( nl_category_name( category ))) != NULL)
    ||  ((ref = getenv( "LANG" )) != NULL)  )
    {
      /* We found an appropriate locale specification in the environment ...
       */
      if( (nl_env_name[ LC_ENUM_LANGUAGE ] = ref = strdup( ref )) != NULL )
      {
	/* Take the working copy of it, which we just placed into `ref',
	 * and decompose it into language, territory, codeset and modifier elements,
	 * storing each in its turn into the global `nl_env_name' array ...
	 */
	char *brkchrs = "_.@";
	char **element = nl_env_name;
	while( *ref && *brkchrs )
	{
	  /* Scanning left to right within the `ref' string,
	   * check each character in turn for a match to any one of the delimiters,
	   * which are defined in `brkchrs' ...
	   */
	  char *mark = brkchrs;
	  while( *mark && (*mark != *ref) )
	    /*
	     * Stepping through the delimiter list,
	     * until a match is found, or all delimiters have been tried.
	     */
	    ++mark;

	  if( *mark )
	  {
	    /* We found a delimiter ...
	     * If it isn't the first in `brkchrs', then there are unspecified
	     * elements within the given locale specification, so arrange for
	     * their `nl_env_name' entries to point to an empty string.
	     */
	    while( *brkchrs++ != *ref )
	      *++element = ref;

	    /* Now insert a string terminator at the current break point in `ref'.
	     * This serves a twofold purpose, viz. it marks the end of the last
	     * non-empty element scanned in `ref', and it serves as the empty
	     * string entity for missing elements in the locale specification.
	     *
	     * After, inserting the terminator for the preceding element, we then
	     * post-increment the `ref' pointer, assigning the ensuing reference
	     * as the starting point for the locale specification element which
	     * is expected to follow the specific delimiter character detected.
	     */
	    *ref++ = '\0';
	    *++element = ref;
	  }
	  /* Otherwise, if the current character in `ref' doesn't match any delimiter,
	   * then we simply advance to the next available character in `ref'.
	   */
	  else ref++;
	}
	/* With the locale specification parsed,
	 * attempt to translate from ISO-639/ISO-3166 to Microsoft format ...
	 */
	if( nl_locale_id_lookup() != LCID_UNKNOWN )
	{
	  /* Using the Microsoft LCID which was found to match the ISO-639/ISO-3166
	   * elements of the locale specification which was found in the environment,
	   * generate a Microsoft compatible locale specification ...
	   */
	  char *language = nl_locale_name_lookup( NULL, nl_locale_id, LOCALE_SENGLANGUAGE );
	  char *territory = nl_locale_name_lookup( "_", nl_locale_id, LOCALE_SENGCOUNTRY );
	  char *codeset = nl_codeset_normalise( ".", nl_env_name[ LC_ENUM_CODESET ] );
	  char locale[ 1 + strlen( language ) + strlen( territory ) + strlen( codeset ) ];
	  sprintf( locale, "%s%s%s", language, territory, codeset );
	  /*
	   * Free all dynamically allocated memory,
	   * which we needed only for temporary storage of locale specification strings,
	   * so we don't introduce any inadvertent memory leaks.
	   */
	  if( (language != NULL) && *language ) free( language );
	  if( (territory != NULL) && *territory ) free( territory );
	  if( (codeset != NULL) && *codeset ) free( codeset );
	  /*
	   * The LC_ENUM_LANGUAGE entry in the `nl_env_name' array also holds a pointer
	   * to the original dynamic buffer we created, in which to parse `ref', and this
	   * is of no further use.
	   */
	  free( nl_env_name[ LC_ENUM_LANGUAGE ] );
	  /*
	   * Now we can use the standard MSVCRT `setlocale' function,
	   * to switch to the locale which we identified from the environment.
	   * We must do this here, while the converted `locale' specification
	   * remains in scope.
	   */
	  return setlocale( MINGW32_LC_REMAP( category ), locale );
	}
	/* If we get to here, then we found a locale specification in the environment,
	 * but we failed to identify a Microsoft compatible representation of it.
	 * We still have a temporary parse buffer for it, with a reference pointer at
	 * `nl_env_name[ LC_ENUM_LANGUAGE ]', and this is of no further use.
	 */
	free( nl_env_name[ LC_ENUM_LANGUAGE ] );
      }
    }
  }
  /* All other cases, besides the one special instance above,
   * are simply passed through to the standard `setlocale' function,
   * without any additional fuss.
   */
  return setlocale( MINGW32_LC_REMAP( category ), locale );
}

/* $RCSfile: setlocale.c,v $Revision: 1.1 $: end of file */
