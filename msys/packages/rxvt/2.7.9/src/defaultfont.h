/*
 * $Id: defaultfont.h,v 1.1 2003/03/05 17:33:36 earnie Exp $
 */

#ifndef _DEFAULTFONT_H_
#define _DEFAULTFONT_H_

#ifdef HAVE_XSETLOCALE
# define X_LOCALE
# include <X11/Xlocale.h>
#else
# ifdef HAVE_SETLOCALE
#  include <locale.h>
# endif
#endif				/* HAVE_XLOCALE */

#ifdef HAVE_NL_LANGINFO
# include <langinfo.h>
#endif

#include "feature.h"

#include "defaultfont.intpro"	/* PROTOS for internal routines */

/*
 * List of encoding labels.
 * Note "encoding" is not "character set" nor "encoding method".
 *
 * In Rxvt, "encoding" is implemented as a pair of "encoding method"
 * (implemented as ENC_METHOD in screen.h) and font specification,
 * i.e., defaultfont[] in this file.
 *
 * This type is used only in this file.
 */

enum enc_label {
    ENC_SJIS, ENC_EUCJ, ENC_GB, ENC_BIG5, ENC_EUCKR,
    ENC_ISO8859_1, ENC_ISO8859_2, ENC_ISO8859_3, ENC_ISO8859_4,
    ENC_ISO8859_5, ENC_ISO8859_6, ENC_ISO8859_7, ENC_ISO8859_8,
    ENC_ISO8859_9, ENC_ISO8859_10, ENC_ISO8859_11, ENC_ISO8859_12,
    ENC_ISO8859_13, ENC_ISO8859_14, ENC_ISO8859_15,
    ENC_KOI8R, ENC_KOI8U,
    /* ENC_VISCII, ENC_VSCII, ENC_TIS620, ENC_UTF8, */
    ENC_DUMMY
};

#define ENC_ISO8859_LAST ENC_ISO8859_15


/*
 * Used for tables of locale/encoding names -> encodng labels.
 */

struct name2encoding {
    const char     *name;
    const enum enc_label encoding;
};

/*
 * Used for tables of encoding labels -> Rxvt internal informations.
 *
 */

struct defaultfont {
    const enum enc_label enc_label;
    const char     *encoding_method;
    const char     *font[MAX_NFONTS];
    const char     *mfont[MAX_NFONTS];
};


/*
 *****************************************************************************
 * FONT DEFINITIONS
 *****************************************************************************
 */

/*
 * Default fonts when encoding is not valid or not specified.
 */

#define NFONT_LIST \
  "7x14", "6x10", "6x13", "8x13", "8x16", "10x20", "12x24"
#ifdef MULTICHAR_SET		/* multichar glyph language support */
# define MFONT_LIST	NULL, NULL,  NULL, NULL, NULL, NULL, NULL
#endif


/*
 * ASCII font definitions (only used in this file)
 */

#define A_12 "-misc-fixed-medium-r-semicondensed--12-110-75-75-c-60-iso8859-1"
#define A_14 "-misc-fixed-medium-r-normal--14-130-75-75-c-70-iso8859-1"
#define A_16 "-etl-fixed-medium-r-normal--16-160-72-72-c-80-iso8859-1"
#define A_18 "-misc-fixed-medium-r-normal--18-170-75-75-c-90-iso8859-1"
#define A_20 "-misc-fixed-medium-r-normal--20-200-75-75-c-100-iso8859-1"
#define A_24 "-etl-fixed-medium-r-normal--24-240-72-72-c-120-iso8859-1"
#define MFONT_LIST_NULL NULL, NULL, NULL, NULL, NULL, NULL, NULL

/* 
 * Font definitions for supported encodings
 *
 * Font Usage Policy (suggested):
 *    1. Use fonts available from XFree86 as much as possible.
 *    2. Use popular fonts in the language community as much as possible.
 *    3. Use "OpenSource" (by Open Source Definition,
 *       http://www.opensource.org/) fonts as much as possible.
 *
 * Comments are the source of these fonts.
 * xf      XFree86 distribution
 * pd      public domain fonts from ftp://ftp.gnu.org/pub/gnu/
 * ak      public domain "a12k12" fonts
 * na      "naga10" from http://gondow-www.cs.titech.ac.jp/~snagao/fonts/
 * cr      "Xcyr" fonts from http://sawsoft.newmail.ru/LS/
 * ba      "baekmuk" fonts from ftp://ftp.mizi.co.kr/pub/baekmuk
 *
 * These definitions should be brushed up by native speakers.
 */

#define NFONT_LIST_EUCJ \
  A_14, \
  "-misc-fixed-medium-r-normal--10-90-75-75-c-50-iso8859-1",/*na*/\
  A_12, A_16, A_18, A_24, NULL
#define MFONT_LIST_EUCJ \
  "-misc-fixed-medium-r-normal--14-130-75-75-c-140-jisx0208.1983-0",/*xf*/\
  "-misc-fixed-medium-r-normal--10-90-75-75-c-100-jisx0208.1983-0",/*na*/\
  "-misc-fixed-medium-r-normal--12-110-75-75-c-120-jisx0208.1983-0",/*ak*/\
  "-jis-fixed-medium-r-normal--16-150-75-75-c-160-jisx0208.1983-0",/*xf*/\
  "-jis-gothic-medium-r-normal--18-170-75-75-c-180-jisx0208.1983-0",/*pd*/\
  "-jis-fixed-medium-r-normal--24-230-75-75-c-240-jisx0208.1983-0",/*xf*/\
  NULL

#define NFONT_LIST_GB A_16, NULL, NULL, A_24, NULL, NULL, NULL
#define MFONT_LIST_GB \
  "-isas-song ti-medium-r-normal--16-160-72-72-c-160-gb2312.1980-0",/*xf*/\
  NULL, NULL, \
  "-isas-song ti-medium-r-normal--24-240-72-72-c-240-gb2312.1980-0",/*xf*/\
  NULL, NULL, NULL

#define NFONT_LIST_BIG5 A_16, NULL, NULL, A_24, NULL, NULL, NULL
#define MFONT_LIST_BIG5 \
  "-eten-fixed-medium-r-normal--16-150-75-75-c-160-big5-0",/*pd*/\
  NULL, NULL, \
  "-eten-fixed-medium-r-normal--24-230-75-75-c-240-big5-0",/*pd*/\
  NULL, NULL, NULL

#define NFONT_LIST_EUCKR A_16, A_12, A_14, A_18, A_20, A_24, NULL
#define MFONT_LIST_EUCKR \
  "-daewoo-mincho-medium-r-normal--16-120-100-100-c-160-ksc5601.1987-0",/*xf*/\
  "-baekmuk-batang-medium-r-normal--12-120-75-75-m-120-ksc5601.1987-0",/*ba*/\
  "-baekmuk-batang-medium-r-normal--14-140-75-75-m-140-ksc5601.1987-0",/*ba*/\
  "-baekmuk-batang-medium-r-normal--18-180-75-75-m-180-ksc5601.1987-0",/*ba*/\
  "-baekmuk-batang-medium-r-normal--20-200-75-75-m-200-ksc5601.1987-0",/*ba*/\
  "-daewoo-mincho-medium-r-normal--24-170-100-100-c-240-ksc5601.1987-0",/*xf*/\
  NULL

#define NFONT_LIST_KOI8R \
  "-misc-fixed-medium-r-normal--14-130-75-75-c-70-koi8-r",/*xf*/\
  "-misc-fixed-medium-r-normal--10-100-75-75-c-60-koi8-r",/*xf*/\
  "-misc-fixed-medium-r-semicondensed--13-120-75-75-c-60-koi8-r",/*xf*/\
  "-misc-fixed-medium-r-normal--13-120-75-75-c-80-koi8-r",/*xf*/\
  "-misc-fixed-medium-r-normal--15-140-75-75-c-90-koi8-r",/*xf*/\
  "-misc-fixed-medium-r-normal--18-120-100-100-c-90-koi8-r",/*xf*/\
  "-misc-fixed-medium-r-normal--20-200-75-75-c-100-koi8-r" /*xf*/

#define NFONT_LIST_KOI8U \
  "-cronyx-fixed-medium-r-normal--14-130-75-75-c-70-koi8-u",/*cr*/\
  "-cronyx-fixed-medium-r-normal--10-100-75-75-c-60-koi8-u",/*cr*/\
  "-cronyx-fixed-medium-r-semicondensed--13-120-75-75-c-60-koi8-u",/*cr*/\
  "-cronyx-fixed-medium-r-normal--13-120-75-75-c-80-koi8-u",/*cr*/\
  "-cronyx-fixed-medium-r-normal--15-140-75-75-c-90-koi8-u",/*cr*/\
  "-cronyx-fixed-medium-r-normal--18-120-100-100-c-90-koi8-u",/*cr*/\
  "-cronyx-fixed-medium-r-normal--20-200-75-75-c-100-koi8-u" /*cr*/

/* special common rule for ISO-8859-* */

#define NFONT_LIST_ISO8859X \
  "-misc-fixed-medium-r-normal--14-130-75-75-c-70-iso8859-%d",       /*xf*/ \
  "-misc-fixed-medium-r-normal--10-100-75-75-c-60-iso8859-%d",       /*xf*/ \
  "-misc-fixed-medium-r-semicondensed--13-120-75-75-c-60-iso8859-%d",/*xf*/ \
  "-misc-fixed-medium-r-normal--13-120-75-75-c-80-iso8859-%d",       /*xf*/ \
  "-misc-fixed-medium-r-normal--16-120-100-100-c-80-iso8859-%d",     /*xf*/ \
  "-misc-fixed-medium-r-normal--20-200-75-75-c-100-iso8859-%d",      /*xf*/ \
  "-misc-fixed-medium-r-normal--24-170-100-100-c-120-iso8859-%d"     /*xf*/

#endif /* _DEFAULTFONT_H_ */
