/*
 * $Id: rxvt.h,v 1.1 2002/12/06 23:08:03 earnie Exp $
 */

#ifndef _RXVT_H			/* include once only */
#define _RXVT_H

#include "feature.h"

/*
 *****************************************************************************
 * SYSTEM HACKS
 *****************************************************************************
 */
/* Consistent defines - please report on the necessity
 * @ Unixware: defines (__svr4__)
 */
#if defined (SVR4) && !defined (__svr4__)
# define __svr4__
#endif
#if defined (sun) && !defined (__sun__)
# define __sun__
#endif

/*
 * sun <sys/ioctl.h> isn't properly protected?
 * anyway, it causes problems when <termios.h> is also included
 */
#if defined (__sun__)
# undef HAVE_SYS_IOCTL_H
#endif

/*
 * Solaris defines SRIOCSREDIR in sys/strredir.h .
 * Needed for displaying console messages under solaris
 */
#if defined(__sun) && defined(__SVR4)
#include <sys/strredir.h>
#endif

/*
 *****************************************************************************
 * INCLUDES
 *****************************************************************************
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#ifdef HAVE_STDARG_H
# include <stdarg.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef NO_STRINGS
# ifdef HAVE_STRING_H
#  include <string.h>
# endif
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_UTIL_H
# include <util.h>
#endif
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#if defined (HAVE_SYS_IOCTL_H) && !defined (__sun__)
/* seems to cause problems when <termios.h> is also included on some suns */
# include <sys/ioctl.h>
#endif
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif

#include <sys/wait.h>
#include <sys/stat.h>

#include <X11/Intrinsic.h>	/* Xlib, Xutil, Xresource, Xfuncproto */
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include "rsizes.h"		/* we've pulled in <sys/types.h> - presumed */
#ifndef HAVE_XPOINTER
typedef char           *XPointer;
#endif

#ifdef RXVT_GRAPHICS
# include "rxvtgrx.h"
#endif
#ifdef GREEK_SUPPORT
# include "grkelot.h"
#endif

#ifndef STDIN_FILENO
# define STDIN_FILENO	0
# define STDOUT_FILENO	1
# define STDERR_FILENO	2
#endif

/*
 *****************************************************************************
 * STRUCTURES AND TYPEDEFS
 *****************************************************************************
 */
/* Sanitize menubar info */
#ifndef MENUBAR
# undef MENUBAR_MAX
#endif
#ifndef MENUBAR_MAX
# define MENUBAR_MAX	0
#endif

typedef struct {
    short           state;
    Window          win;
} menuBar_t;

/* If we're using either the fancy scrollbar or menu bars, keep the
 * scrollColor resource.
 */
#if !defined(XTERM_SCROLLBAR) || defined(MENUBAR)
# define KEEP_SCROLLCOLOR 1
#else
# undef KEEP_SCROLLCOLOR
#endif

#ifdef TRANSPARENT
# define KNOW_PARENTS		6
#else
# define KNOW_PARENTS		1
#endif

typedef struct {
    int16_t         width,	/* window width [pixels]                    */
		    height,	/* window height [pixels]                   */
		    fwidth,	/* font width [pixels]                      */
		    fheight,	/* font height [pixels]                     */
		    fprop,	/* font is proportional                     */
		    bprop,	/* treat bold font as proportional          */
		    mprop,	/* treat multichar font as proportional     */
		    ncol, nrow,	/* window size [characters]                 */
		    focus,	/* window has focus                         */
		    mapped,	/* window state mapped?                     */
		    int_bwidth,
		    ext_bwidth,
		    saveLines;	/* number of lines that fit in scrollback   */
    u_int16_t       nscrolled,	/* number of line actually scrolled         */
		    view_start;	/* scrollback view starts here              */
    Window          parent[KNOW_PARENTS], /* parent[0] is our window        */
		    vt;		/* vt100 window                             */
    GC              gc;		/* GC for drawing text                      */
    XFontStruct    *font;	/* main font structure                      */
#ifndef NO_BOLDFONT
    XFontStruct    *boldFont;	/* bold font                                */
#endif
#ifdef MULTICHAR_SET
    XFontStruct    *mfont;	/* Multichar font structure                 */
#endif
    XFontSet        fontset;
#ifdef XPM_BACKGROUND
    Pixmap          pixmap;
#ifdef XPM_BUFFERING
    Pixmap          buf_pixmap;
#endif
#endif
} TermWin_t;

typedef struct {
    short           beg, end;	/* beg/end of slider sub-window */
    short           top, bot;	/* top/bot of slider */
    short           state;	/* scrollbar state */
    Window          win;
} scrollBar_t;


typedef struct grcmd_t {
    char            cmd;
    short           color;
    short           ncoords;
    int            *coords;
    unsigned char  *text;
    struct grcmd_t *next;
} grcmd_t;

typedef struct {
    int16_t         row, col;
} row_col_t;


#ifndef min
# define min(a,b)	(((a) < (b)) ? (a) : (b))
# define max(a,b)	(((a) > (b)) ? (a) : (b))
#endif

#define MAX_IT(current, other)	if ((other) > (current)) (current) = (other)
#define MIN_IT(current, other)	if ((other) < (current)) (current) = (other)
#define SWAP_IT(one, two, tmp)				\
    do {						\
	(tmp) = (one); (one) = (two); (two) = (tmp);	\
    } while (0)

/*
 *****************************************************************************
 * NORMAL DEFINES
 *****************************************************************************
 */

#if defined (NO_OLD_SELECTION) && defined(NO_NEW_SELECTION)
# error if you disable both selection styles, how can you select, silly?
#endif


#ifndef XPM_BACKGROUND
# undef XPM_BUFFERING		 /* disable what can't be used */
#endif

#define APL_CLASS	"XTerm"	/* class name */
#define APL_SUBCLASS	"Rxvt"	/* also check resources under this name */
#define APL_NAME	"rxvt"	/* normal name */

/* COLORTERM, TERM environment variables */
#define COLORTERMENV	"rxvt"
#ifdef XPM_BACKGROUND
# define COLORTERMENVFULL COLORTERMENV "-xpm"
#else
# define COLORTERMENVFULL COLORTERMENV
#endif
#ifndef TERMENV
# ifdef KANJI
#  define TERMENV	"kterm"
# else
#  define TERMENV	"xterm"
# endif
#endif

#if defined (NO_MOUSE_REPORT) && !defined (NO_MOUSE_REPORT_SCROLLBAR)
# define NO_MOUSE_REPORT_SCROLLBAR
#endif

#ifdef NO_RESOURCES
# undef USE_XGETDEFAULT
#endif

/* now look for other badly set stuff */

#if !defined (EACCESS) && defined(EAGAIN)
# define EACCESS EAGAIN
#endif

#ifndef EXIT_SUCCESS			/* missing from <stdlib.h> */
# define EXIT_SUCCESS		0	/* exit function success */
# define EXIT_FAILURE		1	/* exit function failure */
#endif

#define menuBar_esc		10
#define scrollBar_esc		30
#define menuBar_margin		2	/* margin below text */

/* width of scrollBar, menuBar shadow, must be 1 or 2 */
#ifdef HALFSHADOW
# define SHADOW 1
#else
# define SHADOW 2
#endif

#ifdef NEXT_SCROLLBAR
# undef SB_WIDTH
# define SB_WIDTH		19
# define SB_PADDING		1
# define SB_BORDER_WIDTH	1
# define SB_BEVEL_WIDTH_UPPER_LEFT	1
# define SB_BEVEL_WIDTH_LOWER_RIGHT	2
# define SB_LEFT_PADDING	(SB_PADDING + SB_BORDER_WIDTH)
# define SB_MARGIN_SPACE	(SB_PADDING * 2)
# define SB_BUTTON_WIDTH	(SB_WIDTH - SB_MARGIN_SPACE - SB_BORDER_WIDTH)
# define SB_BUTTON_HEIGHT	(SB_BUTTON_WIDTH)
# define SB_BUTTON_SINGLE_HEIGHT	(SB_BUTTON_HEIGHT + SB_PADDING)
# define SB_BUTTON_BOTH_HEIGHT		(SB_BUTTON_SINGLE_HEIGHT * 2)
# define SB_BUTTON_TOTAL_HEIGHT		(SB_BUTTON_BOTH_HEIGHT + SB_PADDING)
# define SB_BUTTON_BEVEL_X	(SB_LEFT_PADDING)
# define SB_BUTTON_FACE_X	(SB_BUTTON_BEVEL_X + SB_BEVEL_WIDTH_UPPER_LEFT)
# define SB_THUMB_MIN_HEIGHT	(SB_BUTTON_WIDTH - (SB_PADDING * 2))
 /*
  *    +-------------+
  *    |             | <---< SB_PADDING
  *    | ::::::::::: |
  *    | ::::::::::: |
  *   '''''''''''''''''
  *   ,,,,,,,,,,,,,,,,,
  *    | ::::::::::: |
  *    | ::::::::::: |
  *    |  +---------------< SB_BEVEL_WIDTH_UPPER_LEFT
  *    |  | :::::::: |
  *    |  V :::: vv-------< SB_BEVEL_WIDTH_LOWER_RIGHT
  *    | +---------+ |
  *    | | ......%%| |
  *    | | ......%%| |
  *    | | ..()..%%| |
  *    | | ......%%| |
  *    | | %%%%%%%%| |
  *    | +---------+ | <.........................
  *    |             | <---< SB_PADDING         :
  *    | +---------+ | <-+..........            :---< SB_BUTTON_TOTAL_HEIGHT
  *    | | ......%%| |   |         :            :
  *    | | ../\..%%| |   |---< SB_BUTTON_HEIGHT :
  *    | | %%%%%%%%| |   |         :            :
  *    | +---------+ | <-+         :            :
  *    |             |             :            :
  *    | +---------+ | <-+         :---< SB_BUTTON_BOTH_HEIGHT
  *    | | ......%%| |   |         :            :
  *    | | ..\/..%%| |   |         :            :
  *    | | %%%%%%%%| |   |---< SB_BUTTON_SINGLE_HEIGHT
  *    | +---------+ |   |         :            :
  *    |             |   |         :            :
  *    +-------------+ <-+.........:............:
  *    ^^|_________| :
  *    ||     |      :
  *    ||     +---< SB_BUTTON_WIDTH
  *    ||            :
  *    |+------< SB_PADDING
  *    |:            :
  *    +----< SB_BORDER_WIDTH
  *     :            :
  *     :............:
  *           |
  *           +---< SB_WIDTH
  */
#else
# ifdef XTERM_SCROLLBAR
#  undef  SB_WIDTH
#  define SB_WIDTH		15
# else
#  if !defined (SB_WIDTH) || (SB_WIDTH < 8)
#   undef SB_WIDTH
#   define SB_WIDTH		11	/* scrollBar width */
#  endif
# endif				/* XTERM_SCROLLBAR */
#endif

#define NO_REFRESH		0	/* Window not visible at all!        */
#define FAST_REFRESH		(1<<1)  /* Fully exposed window              */
#define SLOW_REFRESH		(1<<2)	/* Partially exposed window          */
#define SMOOTH_REFRESH		(1<<3)	/* Do sync'ing to make it smooth     */

#ifdef NO_SECONDARY_SCREEN
# define NSCREENS		0
#else
# define NSCREENS		1
#endif

#define IGNORE			0
#define SAVE			's'
#define RESTORE			'r'

/* special (internal) prefix for font commands */
#define FONT_CMD		'#'
#define FONT_DN			"#-"
#define FONT_UP			"#+"

/* flags for scr_gotorc() */
#define C_RELATIVE		1	/* col movement is relative */
#define R_RELATIVE		2	/* row movement is relative */
#define RELATIVE		(R_RELATIVE|C_RELATIVE)

/* modes for scr_insdel_chars(), scr_insdel_lines() */
#define INSERT			-1	/* don't change these values */
#define DELETE			+1
#define ERASE			+2

/* modes for scr_page() - scroll page. used by scrollbar window */
enum {
    UP,
    DN,
    NO_DIR
};

/* arguments for scr_change_screen() */
enum {
    PRIMARY,
    SECONDARY
};

/* all basic bit-flags in first/lower 16 bits */

#define RS_None			0		/* Normal */
#define RS_fgMask		0x0000001Fu	/* 32 colors */
#define RS_bgMask		0x000003E0u	/* 32 colors */
#define RS_Bold			0x00000400u	/* bold */
#define RS_Blink		0x00000800u	/* blink */
#define RS_RVid			0x00001000u	/* reverse video */
#define RS_Uline		0x00002000u	/* underline */
#define RS_acsFont		0x00004000u	/* ACS graphics char set */
#define RS_ukFont		0x00008000u	/* UK character set */
#define RS_fontMask		(RS_acsFont|RS_ukFont)
#define RS_baseattrMask		(RS_Bold|RS_Blink|RS_RVid|RS_Uline)

/* all other bit-flags in upper 16 bits */

#ifdef MULTICHAR_SET
# define RS_multi0		0x10000000u	/* only multibyte characters */
# define RS_multi1		0x20000000u	/* multibyte 1st byte */
# define RS_multi2		(RS_multi0|RS_multi1)	/* multibyte 2nd byte */
# define RS_multiMask		(RS_multi0|RS_multi1)	/* multibyte mask */
# define IS_MULTI1(r)		(((r) & RS_multiMask) == RS_multi1)
# define IS_MULTI2(r)		(((r) & RS_multiMask) == RS_multi2)
#else
# define RS_multiMask		0
# define IS_MULTI1(r)		(0)
# define IS_MULTI2(r)		(0)
#endif

#define RS_attrMask		(RS_baseattrMask|RS_fontMask|RS_multiMask)

#define	Opt_console		(1LU<<0)
#define Opt_loginShell		(1LU<<1)
#define Opt_iconic		(1LU<<2)
#define Opt_visualBell		(1LU<<3)
#define Opt_mapAlert		(1LU<<4)
#define Opt_reverseVideo	(1LU<<5)
#define Opt_utmpInhibit		(1LU<<6)
#define Opt_scrollBar		(1LU<<7)
#define Opt_scrollBar_right	(1LU<<8)
#define Opt_scrollBar_floating	(1LU<<9)
#define Opt_meta8		(1LU<<10)
#define Opt_scrollTtyOutput	(1LU<<11)
#define Opt_scrollKeypress	(1LU<<12)
#define Opt_transparent		(1LU<<13)
/* place holder used for parsing command-line options */
#define Opt_Reverse		(1LU<<30)
#define Opt_Boolean		(1LU<<31)

/*
 * XTerm escape sequences: ESC ] Ps;Pt BEL
 */
#define XTerm_name		0
#define XTerm_iconName		1
#define XTerm_title		2
#define XTerm_logfile		46	/* not implemented */
#define XTerm_font		50

/*
 * rxvt extensions of XTerm escape sequences: ESC ] Ps;Pt BEL
 */
#define XTerm_Menu		10	/* set menu item */
#define XTerm_Pixmap		20	/* new bg pixmap */
#define XTerm_restoreFG		39	/* change default fg color */
#define XTerm_restoreBG		49	/* change default bg color */
#define XTerm_dumpscreen	55	/* dump scrollback and all of screen */

#define restoreFG		39	/* restore default fg color */
#define restoreBG		49	/* restore default bg color */

/* Words starting with `Color_' are colours.  Others are counts */

enum colour_list {
    Color_fg = 0,
    Color_bg,
    minCOLOR,				/* 2 */
    Color_Black = minCOLOR,
    Color_Red3,
    Color_Green3,
    Color_Yellow3,
    Color_Blue3,
    Color_Magenta3,
    Color_Cyan3,
    maxCOLOR,				/* minCOLOR + 7 */
#ifndef NO_BRIGHTCOLOR
    Color_AntiqueWhite = maxCOLOR,
    minBrightCOLOR,			/* maxCOLOR + 1 */
    Color_Grey25 = minBrightCOLOR,
    Color_Red,
    Color_Green,
    Color_Yellow,
    Color_Blue,
    Color_Magenta,
    Color_Cyan,
    maxBrightCOLOR,			/* minBrightCOLOR + 7 */
    Color_White = maxBrightCOLOR,
#else
    Color_White = maxCOLOR,
#endif
#ifndef NO_CURSORCOLOR
    Color_cursor,
    Color_cursor2,
#endif
    Color_pointer,
    Color_border,
#ifndef NO_BOLDUNDERLINE
    Color_BD,
    Color_UL,
#endif
#ifdef KEEP_SCROLLCOLOR
    Color_scroll,
    Color_trough,
#endif
    NRS_COLORS,				/* */
#ifdef KEEP_SCROLLCOLOR
    Color_topShadow = NRS_COLORS,
    Color_bottomShadow,
    TOTAL_COLORS			/* upto 28 */
#else
    TOTAL_COLORS = NRS_COLORS		/* */
#endif
};

#define DEFAULT_RSTYLE		(RS_None | (Color_fg) | (Color_bg<<5))

/*
 * Resource list
 */
typedef struct {
    const char     *display_name,
		   *term_name,
		   *iconName,
		   *geometry,
		   *reverseVideo,
		   *color[NRS_COLORS],
		   *font[NFONTS],
#ifdef MULTICHAR_SET
		   *mfont[NFONTS],
		   *multichar_encoding,
#endif
		   *name,
		   *title,
#if defined (XPM_BACKGROUND) || (MENUBAR_MAX)
		   *path,
#endif
#ifdef XPM_BACKGROUND
		   *backgroundPixmap,
#endif
#if (MENUBAR_MAX)
		   *menu,
#endif
#ifndef NO_BOLDFONT
		   *boldFont,
#endif
#ifdef GREEK_SUPPORT
		   *greek_keyboard,
#endif
 		   *loginShell,
 		   *scrollBar,
 		   *scrollBar_right,
 		   *scrollBar_floating,
 		   *scrollTtyOutput,
 		   *scrollKeypress,
 		   *saveLines,
 		   *utmpInhibit,
 		   *visualBell,
#if ! defined(NO_MAPALERT) && defined(MAPALERT_OPTION)
		   *mapAlert,
#endif
#ifdef META8_OPTION
		   *meta8,
#endif
#ifndef NO_BACKSPACE_KEY
		   *backspace_key,
#endif
#ifndef NO_DELETE_KEY
		   *delete_key,
#endif
		   *selectstyle,
#ifdef PRINTPIPE
		   *print_pipe,
#endif
#ifdef USE_XIM
		   *preeditType,
		   *inputMethod,
#endif
#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
		   *bigfont_key,
		   *smallfont_key,
#endif
#ifdef TRANSPARENT
		   *transparent,
#endif
#ifndef NO_FRILLS
		   *ext_bwidth,
		   *int_bwidth,
#endif
		   *cutchars,
		   *modifier;
} resource_list;

/*
 * number of graphics points
 * divisible by 2 (num lines)
 * divisible by 4 (num rect)
 */
#define	NGRX_PTS	1000

/*
 *****************************************************************************
 * MACRO DEFINES
 *****************************************************************************
 */
#define MEMSET(x, y, z)		memset((x), (y), (z))
#define MEMCPY(x, y, z)		memcpy((void *)(x), (const void *)(y), (z))
#define MEMMOVE(x, y, z)	memmove((void *)(x), (const void *)(y), (z))
#define STRCASECMP(x, y)	strcasecmp((x), (y))
#define STRNCASECMP(x, y, z)	strncasecmp((x), (y), (z))
#define STRCPY(x, y)		strcpy((char *)(x), (const char *)(y))
#define STRNCPY(x, y, z)	strncpy((char *)(x), (const char *)(y), (z))
#define STRCMP(x, y)		strcmp((const char *)(x), (const char *)(y))
#define STRNCMP(x, y, z)	strncmp((const char *)(x), (const char *)(y), (z))
#define STRCAT(x, y)		strcat((char *)(x), (const char *)(y))
#define STRNCAT(x, y, z)	strncat((char *)(x), (const char *)(y), (z))
#define STRDUP(x)		strdup((const char *)(x))
#define STRLEN(x)		strlen((const char *)(x))
#define STRCHR(x, y)		strchr((const char *)(x), (int)(y))
#define STRRCHR(x, y)		strrchr((const char *)(x), (int)(y))

#define MALLOC(sz)		malloc(sz)
#define CALLOC(n, sz)		calloc((n), (sz))
#define REALLOC(mem, sz)	((mem) ? realloc((mem), (sz)) : malloc(sz))
#define FREE(ptr)		free(ptr)

/* convert pixel dimensions to row/column values */
#define Pixel2Col(x)		Pixel2Width((x) - TermWin.int_bwidth)
#define Pixel2Row(y)		Pixel2Height((y) - TermWin.int_bwidth)
#define Pixel2Width(x)		((x) / TermWin.fwidth)
#define Pixel2Height(y)		((y) / TermWin.fheight)
#define Col2Pixel(col)		(Width2Pixel(col) + TermWin.int_bwidth)
#define Row2Pixel(row)		(Height2Pixel(row) + TermWin.int_bwidth)
#define Width2Pixel(n)		((n) * TermWin.fwidth)
#define Height2Pixel(n)		((n) * TermWin.fheight)

#define TermWin_TotalWidth()	(TermWin.width  + 2 * TermWin.int_bwidth)
#define TermWin_TotalHeight()	(TermWin.height + 2 * TermWin.int_bwidth)

#define Xscreen			DefaultScreen(Xdisplay)
#define Xroot			DefaultRootWindow(Xdisplay)

/* how to build & extract colors and attributes */
#define GET_FGCOLOR(r)		(((r) & RS_fgMask))
#define GET_BGCOLOR(r)		(((r) & RS_bgMask)>>5)
#define GET_ATTR(r)		(((r) & RS_attrMask))
#define GET_BGATTR(r)							\
    (((r) & RS_RVid) ? (((r) & (RS_attrMask & ~RS_RVid))		\
			| (((r) & RS_fgMask)<<5))			\
		     : ((r) & (RS_attrMask | RS_bgMask)))
#define SET_FGCOLOR(r,fg)	(((r) & ~RS_fgMask)  | (fg))
#define SET_BGCOLOR(r,bg)	(((r) & ~RS_bgMask)  | ((bg)<<5))
#define SET_ATTR(r,a)		(((r) & ~RS_attrMask)| (a))

#define scrollbar_visible()	(scrollBar.state)
#define scrollbar_isMotion()	(scrollBar.state == 'm')
#define scrollbar_isUp()	(scrollBar.state == 'U')
#define scrollbar_isDn()	(scrollBar.state == 'D')
#define scrollbar_isUpDn()	isupper (scrollBar.state)
#define isScrollbarWindow(w)	(scrollbar_visible() && (w) == scrollBar.win)

#define scrollbar_setNone()	scrollBar.state = 1
#define scrollbar_setMotion()	scrollBar.state = 'm'
#define scrollbar_setUp()	scrollBar.state = 'U'
#define scrollbar_setDn()	scrollBar.state = 'D'

#ifdef NEXT_SCROLLBAR
# define scrollbar_dnval()	(scrollBar.end + (SB_WIDTH + 1))
# define scrollbar_upButton(y)	((y) > scrollBar.end \
				 && (y) <= scrollbar_dnval())
# define scrollbar_dnButton(y)	((y) > scrollbar_dnval())
# define SCROLL_MINHEIGHT	SB_THUMB_MIN_HEIGHT
#else
# define scrollbar_upButton(y)	((y) < scrollBar.beg)
# define scrollbar_dnButton(y)	((y) > scrollBar.end)
# define SCROLL_MINHEIGHT	10
#endif

#define scrollbar_above_slider(y)	((y) < scrollBar.top)
#define scrollbar_below_slider(y)	((y) > scrollBar.bot)
#define scrollbar_position(y)		((y) - scrollBar.beg)
#define scrollbar_size()		(scrollBar.end - scrollBar.beg - SCROLL_MINHEIGHT)

#if (MENUBAR_MAX > 1)
/* rendition style flags */
# define menubar_visible()	(menuBar.state)
# define menuBar_height()	(TermWin.fheight + SHADOW)
# define menuBar_TotalHeight()	(menuBar_height() + SHADOW + menuBar_margin)
# define isMenuBarWindow(w)	((w) == menuBar.win)
#else
# define isMenuBarWindow(w)	(0)
# define menuBar_height()	(0)
# define menuBar_TotalHeight()	(0)
# define menubar_visible()	(0)
#endif

#ifdef XPM_BACKGROUND
# define XPMClearArea(a, b, c, d, e, f, g)	XClearArea((a), (b), (c), (d), (e), (f), (g))
#else
# define XPMClearArea(a, b, c, d, e, f, g)
#endif

#ifndef STRICT_FONT_CHECKING
# define get_fontwidest(font)	((font)->max_bounds.width)
#endif

#define Gr_ButtonPress(x,y)	Gr_ButtonReport ('P',(x),(y))
#define Gr_ButtonRelease(x,y)	Gr_ButtonReport ('R',(x),(y))
/*
 *****************************************************************************
 * VARIABLES
 *****************************************************************************
 */
#ifdef INTERN
# define EXTERN
#else
# define EXTERN extern
#endif

#ifdef PREFER_24BIT
EXTERN Colormap     Xcmap;
EXTERN int          Xdepth;
EXTERN Visual      *Xvisual;
#else
# define Xcmap			DefaultColormap(Xdisplay,Xscreen)
# define Xdepth			DefaultDepth(Xdisplay,Xscreen)
# define Xvisual		DefaultVisual(Xdisplay,Xscreen)
# ifdef DEBUG_DEPTH
#  undef Xdepth
#  define Xdepth		DEBUG_DEPTH
# endif
#endif

EXTERN TermWin_t	TermWin;
EXTERN scrollBar_t	scrollBar;
EXTERN menuBar_t	menuBar;
EXTERN Display	       *Xdisplay;
EXTERN unsigned long	Options;
EXTERN XSizeHints       szHint;
EXTERN int		sb_shadow;
EXTERN Pixel		PixColors[TOTAL_COLORS];
#ifdef INEXPENSIVE_LOCAL_X_CALLS
EXTERN int              display_is_local;
#endif
EXTERN short		want_refresh;

EXTERN resource_list    rs;

#ifndef NO_BACKSPACE_KEY
EXTERN const char      *key_backspace;
#endif
#ifndef NO_DELETE_KEY
EXTERN const char      *key_delete;
#endif
#ifndef NO_BRIGHTCOLOR
EXTERN unsigned int	colorfgbg;
#endif
#ifdef KEYSYM_RESOURCE
EXTERN const unsigned char *KeySym_map[256];
#endif
#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
EXTERN KeySym		ks_bigfont;
EXTERN KeySym		ks_smallfont;
#endif

/*
 *****************************************************************************
 * PROTOTYPES
 *****************************************************************************
 */
#include "screen.h"

#ifdef PROTOTYPES
# define __PROTO(p)	p
#else
# define __PROTO(p)	()
#endif
#include "protos.h"

#ifndef RXVT_GRAPHICS		/* sync functions with graphics.extpro */
# define Gr_ButtonReport(but, x, y)
# define Gr_do_graphics(cmd, nargs, args, text)
# define Gr_scroll(count)
# define Gr_ClearScreen()
# define Gr_ChangeScreen()
# define Gr_expose(win)
# define Gr_Resize(w, h)
# define Gr_reset()
# define Gr_Displayed()		(0)
#endif

#ifndef MENUBAR			/* sync functions with menubar.extpro */
# define menubar_read(filename)
# define menubar_dispatch(str)
# define menubar_expose()
# define menubar_mapping(map)	(0)
# define menubar_control(ev)
# define map_menuBar(map)
# define create_menuBar(cursor)
# define Resize_menuBar(x, y, width, height)
#endif

#ifndef XPM_BACKGROUND		/* sync functions with xpm.extpro */
# define scale_pixmap(geom)	(0)
# define resize_pixmap()
# define set_bgPixmap(file)	(0)
#endif

#ifndef GREEK_SUPPORT		/* sync functions with grkelot.extpro */
# define greek_init()
# define greek_end()
# define greek_reset()
# define greek_setmode(greek_mode)
# define greek_getmode()	(0)
# define greek_xlat(s, num_chars)	(0)
#endif

#ifdef DEBUG_MALLOC
# include "dmalloc.h"		/* This comes last */
#endif

#endif				/* _RXVT_H */
