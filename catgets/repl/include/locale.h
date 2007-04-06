#ifndef	_LOCALE_H_
/* 
 * locale.h
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Functions and types for localization (ie. changing the appearance of
 * output based on the standards of a certain country).
 *
 */
#define	_LOCALE_H_

/* All the headers include this file. */
#include <_mingw.h>

/*
 * NOTE: I have tried to test this, but I am limited by my knowledge of
 *       locale issues. The structure does not bomb if you look at the
 *       values, and 'decimal_point' even seems to be correct. But the
 *       rest of the values are, by default, not particularly useful
 *       (read meaningless and not related to the international settings
 *       of the system).
 */

enum { LC_ALL = 0, LC_COLLATE, LC_CTYPE, LC_MONETARY, LC_NUMERIC, LC_TIME };
enum { LC_MIN = LC_ALL, LC_MAX = LC_TIME };

#ifndef RC_INVOKED

/* According to C89 std, NULL is defined in locale.h too.  */
#define __need_NULL
#include <stddef.h>

/*
 * The structure returned by 'localeconv'.
 */
struct lconv
{
	char*	decimal_point;
	char*	thousands_sep;
	char*	grouping;
	char*	int_curr_symbol;
	char*	currency_symbol;
	char*	mon_decimal_point;
	char*	mon_thousands_sep;
	char*	mon_grouping;
	char*	positive_sign;
	char*	negative_sign;
	char	int_frac_digits;
	char	frac_digits;
	char	p_cs_precedes;
	char	p_sep_by_space;
	char	n_cs_precedes;
	char	n_sep_by_space;
	char	p_sign_posn;
	char	n_sign_posn;
};

#ifdef	__cplusplus
extern "C" {
#endif

_CRTIMP  char* __cdecl setlocale (int, const char*);
_CRTIMP struct lconv* __cdecl localeconv (void);

/*
 * MinGW provides the `_mingw_setlocale' wrapper, to add environment variable
 * interpretation, and optional LC_MESSAGES support.
 */
extern char* __cdecl _mingw_setlocale (int, const char*);
/*
 * optional MinGW extensions are enabled by MINGW32_LC_EXTENSIONS
 */
#ifndef  MINGW32_LC_EXTENSIONS
# define MINGW32_LC_EXTENSIONS  0
#endif
/*
 * Supported MinGW extensions are:
 */
#define  MINGW32_LC_ENVVARS   0x40  /* environment variables specify locales */
#define  MINGW32_LC_MESSAGES  0x10  /* simulate LC_MESSAGES using LC_CTYPE   */
/*
 * Enable extensions:
 */
#if MINGW32_LC_EXTENSIONS & MINGW32_LC_MESSAGES
/*
 * Define LC_MESSAGES, only if this extension is requested.
 */
# define LC_MESSAGES  MINGW32_LC_MESSAGES | LC_CTYPE
/*
 * We also need a remapping macro, to pass this category code to standard
 * functions which will not recognise it; we simply pass LC_CTYPE instead.
 */
# define MINGW32_LC_REMAP(C)  (((C) == LC_MESSAGES) ? LC_CTYPE : (C))
#else
/*
 * When MINGW32_LC_MESSAGES is not enabled, we still need the remapping
 * macro, but it simply passes its argument unchanged.
 */
# define MINGW32_LC_REMAP(C)  (C)
#endif

#if MINGW32_LC_EXTENSIONS & MINGW32_LC_ENVVARS
/*
 * This extension causes all `setlocale' calls in the compilation unit to be
 * transparently be redirected to `_mingw_setlocale'; the same capability may
 * be accessed, without enabling the extension, by calling `_mingw_setlocale'
 * directly, where the feature is required.
 */
# define setlocale  _mingw_setlocale
#endif

#ifndef _WLOCALE_DEFINED  /* also declared in wchar.h */
# define __need_wchar_t
# include <stddef.h>
  _CRTIMP wchar_t* __cdecl _wsetlocale(int, const wchar_t*);
# define _WLOCALE_DEFINED
#endif /* ndef _WLOCALE_DEFINED */

#ifdef	__cplusplus
}
#endif

#endif	/* Not RC_INVOKED */

#endif	/* Not _LOCALE_H_ */

