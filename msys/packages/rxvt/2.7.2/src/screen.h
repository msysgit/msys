/*
 * $Id: screen.h,v 1.1 2002-12-06 23:08:03 earnie Exp $
 */

#ifndef _SCREEN_H		/* include once only */
#define _SCREEN_H

typedef unsigned char text_t;

#if defined(MULTICHAR_SET)
#define rend_t		u_int32_t
#else
#define rend_t		u_int16_t
#endif

/*
 * screen accounting:
 * screen_t elements
 *   text:      Contains all text information including the scrollback buffer.
 *              Each line is length TermWin.ncol
 *   tlen:      The length of the line or -1 for wrapped lines.
 *   rend:      Contains rendition information: font, bold, colour, etc.
 * * Note: Each line for both text and rend are only allocated on demand, and
 *         text[x] is allocated <=> rend[x] is allocated  for all x.
 *   row:       Cursor row position                   : 0 <= row < TermWin.nrow
 *   col:       Cursor column position                : 0 <= col < TermWin.ncol
 *   tscroll:   Scrolling region top row inclusive    : 0 <= row < TermWin.nrow
 *   bscroll:   Scrolling region bottom row inclusive : 0 <= row < TermWin.nrow
 *
 * selection_t elements
 *   clicks:    1, 2 or 3 clicks - 4 indicates a special condition of 1 where
 *              nothing is selected
 *   beg:       row/column of beginning of selection  : never past mark
 *   mark:      row/column of initial click           : never past end
 *   end:       row/column of one character past end of selection
 * * Note: -TermWin.nscrolled <= beg.row <= mark.row <= end.row < TermWin.nrow
 * * Note: col == -1 ==> we're left of screen
 *
 * TermWin.saveLines:
 *              Maximum number of lines in the scrollback buffer.
 *              This is fixed for each rxvt instance.
 * TermWin.nscrolled:
 *              Actual number of lines we've used of the scrollback buffer
 *              0 <= TermWin.nscrolled <= TermWin.saveLines
 * TermWin.view_start:
 *              Offset back into the scrollback buffer for out current view
 *              0 <= TermWin.view_start <= TermWin.nscrolled
 *
 * Layout of text/rend information in the screen_t text/rend structures:
 *   Rows [0] ... [TermWin.saveLines - 1]
 *     scrollback region : we're only here if TermWin.view_start != 0
 *   Rows [TermWin.saveLines] ... [TermWin.saveLines + TermWin.nrow - 1]
 *     normal `unscrolled' screen region
 */
typedef struct {
    text_t        **text;	/* _all_ the text                            */
    int16_t	   *tlen;	/* length of each text line                  */
    rend_t        **rend;	/* rendition, uses RS_ flags                 */
    row_col_t       cur;	/* cursor position on the screen             */
    int16_t         tscroll,	/* top of settable scroll region             */
                    bscroll,	/* bottom of settable scroll region          */
		    charset;	/* character set number [0..3]               */
    unsigned int    flags;	/* see below                                 */
    row_col_t	    s_cur;	/* saved cursor position                     */
    int16_t	    s_charset;	/* saved character set number [0..3]         */
    char            s_charset_char;
    rend_t          s_rstyle;	/* saved rendition style                     */
} screen_t;

typedef struct {
    unsigned char  *text;	/* selected text                             */
    int             len;	/* length of selected text                   */
    enum {
	SELECTION_CLEAR = 0,	/* nothing selected                          */
	SELECTION_INIT,		/* marked a point                            */
	SELECTION_BEGIN,	/* started a selection                       */
	SELECTION_CONT,		/* continued selection                       */
	SELECTION_DONE		/* selection put in CUT_BUFFER0              */
    } op;			/* current operation                         */
    short           screen;	/* screen being used                         */
    short           clicks;	/* number of clicks                          */
    row_col_t       beg, mark, end;
} selection_t;

typedef enum {
    OLD_SELECT, OLD_WORD_SELECT, NEW_SELECT
} sstyle_t;

/* ------------------------------------------------------------------------- */

/* screen_t flags */
#define Screen_Relative		(1<<0)	/* relative origin mode flag         */
#define Screen_VisibleCursor	(1<<1)	/* cursor visible?                   */
#define Screen_Autowrap		(1<<2)	/* auto-wrap flag                    */
#define Screen_Insert		(1<<3)	/* insert mode (vs. overstrike)      */
#define Screen_WrapNext		(1<<4)	/* need to wrap for next char?       */
#define Screen_DefaultFlags	(Screen_VisibleCursor|Screen_Autowrap)

/* ------------------------------------------------------------------------- */

typedef enum {
    EUCJ, SJIS,			/* Japanese KANJI methods                    */
    BIG5, CNS,			/* Chinese BIG5 methods: CNS not implemented */
    GB,				/* Chinese GB method                         */
    EUCKR			/* Korean method                             */
} ENC_METHOD;

typedef struct {
    short          method;
    void (*func)(unsigned char *, int);
    char          *name;
} KNOWN_ENCODINGS;

/* ------------------------------------------------------------------------- *
 *                             MODULE VARIABLES                              *
 * ------------------------------------------------------------------------- */

#ifdef INTERN_SCREEN
# define EXTSCR
#else
# define EXTSCR	extern
#endif

/* This tells what's actually on the screen */
EXTSCR text_t    **drawn_text;
EXTSCR rend_t    **drawn_rend;
EXTSCR text_t    **buf_text;
EXTSCR rend_t    **buf_rend;
EXTSCR int16_t    *buf_tlen;
EXTSCR char       *tabs;	/* a 1 for a location with a tab-stop */
EXTSCR screen_t    screen;
EXTSCR screen_t    swap;
EXTSCR selection_t selection;
EXTSCR sstyle_t    selection_style;
#ifdef MULTICHAR_SET
EXTSCR ENC_METHOD  encoding_method;
#endif


#endif				/* repeat inclusion protection */
