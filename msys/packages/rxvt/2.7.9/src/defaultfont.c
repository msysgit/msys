/*--------------------------------*-C-*---------------------------------*
 * File:	defaultfont.c
 *----------------------------------------------------------------------*
 * Copyright (c) 2001      Tomohiro KUBOTA <kubota@debian.org>
 *				- original version
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *---------------------------------------------------------------------*/
/*
 * Setting default font and encoding according to user's locale (LC_CTYPE).
 */

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "defaultfont.h"

#define ENCODINGBUFLEN 100

/*
 * Table to convert from encoding names to enc_label.
 * This table is useful to normalize encoding names
 * and support various return value from nl_langinfo(3).
 *
 * The encoding names here are "truncated" names,
 * where all alphabets are uppercase and all '-' and
 * '_' are eliminated.
 */

const struct name2encoding n2e[] = {
#ifdef MULTICHAR_SET
    { "EUCJP",		ENC_EUCJ },
    { "UJIS",		ENC_EUCJ },
    { "SHIFTJIS",	ENC_SJIS },
    { "SJIS",		ENC_SJIS },
    { "EUCKR",		ENC_EUCKR },
    { "EUCCN",		ENC_GB },
    { "GB2312",		ENC_GB },
    { "GB",		ENC_GB },
    { "BIG5",		ENC_BIG5 },
    { "BIGFIVE",	ENC_BIG5 },
    { "BIG5HKSCS",	ENC_BIG5 },
#endif				/* MULTICHAR_SET */
    { "KOI8R",		ENC_KOI8R },
    { "KOI8U",		ENC_KOI8U },
    { "ISO88591",	ENC_ISO8859_1 },
    { "ISO88592",	ENC_ISO8859_2 },
    { "ISO88593",	ENC_ISO8859_3 },
    { "ISO88594",	ENC_ISO8859_4 },
    { "ISO88595",	ENC_ISO8859_5 },
    { "ISO88596",	ENC_ISO8859_6 },
    { "ISO88597",	ENC_ISO8859_7 },
    { "ISO88598",	ENC_ISO8859_8 },
    { "ISO88599",	ENC_ISO8859_9 },
    { "ISO885910",	ENC_ISO8859_10 },
    { "ISO885911",	ENC_ISO8859_11 },
    { "ISO885912",	ENC_ISO8859_12 },
    { "ISO885913",	ENC_ISO8859_13 },
    { "ISO885914",	ENC_ISO8859_14 },
    { "ISO885915",	ENC_ISO8859_15 },
    { NULL,		ENC_DUMMY }
};

/*
 * This table converts from locale names to enc_label.
 *
 * This table is used to know which encoding is used
 * as the default in the current user environment
 * (LC_CTYPE locale), since it is the standard way
 * for users to specify encoding by LANG/LC_CTYPE/LC_ALL
 * variables (i.e., LC_CTYPE locale).  Consult locale(7).
 *
 * This table is used when nl_langinfo(3) is not available
 * or it fails.
 *
 * locale names whose "encoding" part are listed in n2e[]
 * can be omitted here, because "encoding" part is checked
 * separately before l2e[] check.
 *
 * Note that longer locale names must be written earlier
 * than shorter locale names in this table, because
 * strncmp(3) is used for seek for this table.
 */

const struct name2encoding l2e[] = {
#ifdef MULTICHAR_SET
    { "ja_JP.EUC",	ENC_EUCJ },
    { "ja_JP",		ENC_EUCJ },
    { "ko_KR.EUC",	ENC_EUCKR },
    { "ko_KR",		ENC_EUCKR },
    { "zh_CN.EUC",	ENC_GB },
    { "zh_CN",		ENC_GB },
    { "zh_TW",		ENC_BIG5 },
#endif				/* MULTICHAR_SET */
    { "da",		ENC_ISO8859_1 },
    { "de",		ENC_ISO8859_1 },
    { "en",		ENC_ISO8859_1 },
    { "fi",		ENC_ISO8859_1 },
    { "fr",		ENC_ISO8859_1 },
    { "is",		ENC_ISO8859_1 },
    { "it",		ENC_ISO8859_1 },
    { "la",		ENC_ISO8859_1 },
    { "lt",		ENC_ISO8859_1 },
    { "nl",		ENC_ISO8859_1 },
    { "no",		ENC_ISO8859_1 },
    { "pt",		ENC_ISO8859_1 },
    { "sv",		ENC_ISO8859_1 },
    { "cs",		ENC_ISO8859_2 },
    { "hr",		ENC_ISO8859_2 },
    { "hu",		ENC_ISO8859_2 },
    { "la",		ENC_ISO8859_2 },
    { "lt",		ENC_ISO8859_2 },
    { "pl",		ENC_ISO8859_2 },
    { "sl",		ENC_ISO8859_2 },
    { "ru",		ENC_KOI8R },	/* ISO8859-5 ? */
    { "uk",		ENC_KOI8U },
#if 0
    { "vi",		ENC_VISCII },
    { "th",		ENC_TIS620 },
#endif
    { NULL,		ENC_DUMMY }
};

/*
 *  Default font name for each language.
 *  I'd like these names edited by native speakers.
 *
 *  enc_label   -->   ENCODING_METHOD and font informations
 *                    which as needed for Rxvt to work.
 */

const struct defaultfont defaultfont[] = {
#ifdef MULTICHAR_SET
    { ENC_EUCJ,		"eucj",	{NFONT_LIST_EUCJ},  {MFONT_LIST_EUCJ} },
    { ENC_SJIS,		"sjis",	{NFONT_LIST_EUCJ},  {MFONT_LIST_EUCJ} },
    { ENC_GB,		"gb",	{NFONT_LIST_GB},    {MFONT_LIST_GB} },
    { ENC_BIG5,		"big5",	{NFONT_LIST_BIG5},  {MFONT_LIST_BIG5} },
    { ENC_EUCKR,	"kr",	{NFONT_LIST_EUCKR}, {MFONT_LIST_EUCKR} },
#endif				/* MULTICHAR_SET */
#if 0
/* sample for font specification when common rule for ISO-8859-x
 * is not satisfying */
    { ENC_ISO8859_1,	"noenc",{NFONT_LIST_1},     {MFONT_LIST_NULL} },
#endif
    { ENC_KOI8R,	"noenc",{NFONT_LIST_KOI8R}, {MFONT_LIST_NULL} },
    { ENC_KOI8U,	"noenc",{NFONT_LIST_KOI8U}, {MFONT_LIST_NULL} },
    { ENC_DUMMY,	"noenc",{MFONT_LIST_NULL},  {MFONT_LIST_NULL} }
};

/* special common rule for ISO-8859-x */
const char *const defaultfont_8859[] = {
    NFONT_LIST_ISO8859X
};

/* fallback defaults */
const char *const def_fontName[] = {
    NFONT_LIST
};

#ifdef MULTICHAR_SET
const char *const def_mfontName[] = {
    MFONT_LIST
};
#endif

/*----------------------------------------------------------------------*/
/* EXTPROTO */
void
rxvt_set_defaultfont(rxvt_t *r, const char *rs[])
{
    char           *locale = r->h->locale;
    char           *encoding_str = NULL;
    char            encoding_buf[ENCODINGBUFLEN];
    char           *p, *p2;
    enum enc_label  encoding = ENC_DUMMY;
    int             j, k;

    r->h->fnum = FONT0_IDX;

#ifdef MULTICHAR_SET
/*
 * Check if encoding is determined manually by -km option or
 * multichar_encoding resource.  If yes, rxvt_set_defaultfont
 * must not override it.  However, default font setting is needed.
 */
    if (r->h->rs[Rs_multichar_encoding] != NULL) {
	for (j = 0; j < MAX_NFONTS; j++) {
	    if (rs[Rs_font + j] == NULL)
		rs[Rs_font + j] = def_fontName[j];
	    if (rs[Rs_mfont + j] == NULL)
		rs[Rs_mfont + j] = def_mfontName[j];
	}
	return;
    } 
#endif

#ifdef HAVE_NL_LANGINFO
# if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
/* if locale is not valid, nl_langinfo() returns ASCII. */
    if (locale == NULL)
	encoding_str = NULL;
    else
# endif
    encoding_str = nl_langinfo(CODESET);
#endif				/* HAVE_NL_LANGINFO */

/*
 * emulate LC_CTYPE locale behavior when setlocale fails or
 * is not available; AFTER check of nl_langinfo() because
 * nl_langinfo() is useful only when setlocale(LC_CTYPE) works.
 */
    if (locale == NULL) {
	if ((locale = getenv("LC_ALL")) == NULL)
	    if ((locale = getenv("LC_CTYPE")) == NULL)
		if ((locale = getenv("LANG")) == NULL)
		    locale = "C";	/* failsafe */
	r->h->locale = locale;
    }

/*
 * Check nl_langinfo() first.  If nl_langinfo() is not available,
 * encoding part of LC_CTYPE locale is used.
 */
    if (encoding_str && *encoding_str)
	STRNCPY(encoding_buf, encoding_str, ENCODINGBUFLEN);
    else {
	p = STRCHR(locale, '.');
	if (p) {
	    STRNCPY(encoding_buf, p + 1, ENCODINGBUFLEN);
	    p = STRCHR(encoding_buf, '@');
	    if (p)
		*p = 0;
	} else
	    STRNCPY(encoding_buf, locale, ENCODINGBUFLEN);
    }
    encoding_buf[ENCODINGBUFLEN - 1] = 0;
    for (p = p2 = encoding_buf; 1; p++, p2++) {
	while (*p2 == '_' || *p2 == '-')
	    p2++;
	if (!*p2)
	    break;
	*p = toupper(*p2);
    }
    *p = 0;
    for (j = 0; n2e[j].name; j++)
	if (!STRCMP(encoding_buf, n2e[j].name)) {
	    encoding = n2e[j].encoding;
	    break;
	}

/* Next, check "language"/"country" part of locale name. */
    if (encoding == ENC_DUMMY)
	for (j = 0; l2e[j].name; j++) {
	    if (!STRNCMP(locale, l2e[j].name, strlen(l2e[j].name))) {
		encoding = l2e[j].encoding;
		break;
	    }
	}

/*
 * Build rs[Rs_font], rs[Rs_mfont], r->encoding_method,
 * and r->h->multichar_decode
 */
    for (j = 0; defaultfont[j].enc_label != ENC_DUMMY; j++) {
	if (encoding == defaultfont[j].enc_label) {
#ifdef MULTICHAR_SET
	    rxvt_set_multichar_encoding(r, defaultfont[j].encoding_method);
#endif
	    for (k = 0; k < MAX_NFONTS; k++) {
		if (rs[Rs_font + k] == NULL)
		    rs[Rs_font + k] = defaultfont[j].font[k];
#ifdef MULTICHAR_SET
		if (rs[Rs_mfont + k] == NULL)
		    rs[Rs_mfont + k] = defaultfont[j].mfont[k];
#endif
	    }
	    return;
	}
    }


/*
 * fallback for unknown encodings.  ISO-8559-* gets special treatment
 */

#ifdef MULTICHAR_SET
    rxvt_set_multichar_encoding(r, "noenc");
#endif
    if (encoding >= ENC_ISO8859_1 && encoding <= ENC_ISO8859_LAST) {
    /* fallback for ISO-8859-* encodings */
	k = encoding - ENC_ISO8859_1 + 1;
	MIN_IT(k, 99999);
    } else
    /* fallback for "C", "POSIX", and invalid locales */
	k = 0;

    for (j = 0; j < MAX_NFONTS; j++) {
	if (rs[Rs_font + j] == NULL) {
	    if (k == 0)
		rs[Rs_font + j] = def_fontName[j];
	    else {
	    /* couple of wasted bytes each but lots of future expansion */
		rs[Rs_font + j] = rxvt_malloc(STRLEN(defaultfont_8859[j]) + 4);
		sprintf((char *)rs[Rs_font + j], defaultfont_8859[j], k);
	    }
	}
#ifdef MULTICHAR_SET
	if (rs[Rs_mfont + j] == NULL)
	    rs[Rs_mfont + j] = def_mfontName[j];
#endif
    }
}
