/*--------------------------------*-C-*--------------------------------------*
 * File:	screen.c
 *---------------------------------------------------------------------------*
 * $Id: screen.c,v 1.1 2002-12-06 23:08:03 earnie Exp $
 *
 * Copyright (C) 1997,1999 Geoff Wing <gcw@pobox.com>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *--------------------------------------------------------------------------*/
/*
 * We handle _all_ screen updates and selections
 */

#include "../config.h"		/* NECESSARY */
#define INTERN_SCREEN
#include "rxvt.h"		/* NECESSARY */
#include "screen.intpro"	/* PROTOS for internal routines */

#include <X11/Xatom.h>
#include <X11/Xmd.h>		/* get the typedef for CARD32 */

/* ------------------------------------------------------------------------- */
static char     charsets[4];
static short    current_screen;
static rend_t   rstyle;
static short    rvideo;

#ifdef MULTICHAR_SET
short           multi_byte;	/* set ==> currently using 2 bytes per glyph */
short           lost_multi;	/* set ==> we only got half a glyph */
enum {
    SBYTE, WBYTE
} chstat;

#define RESET_CHSTAT			\
    if (chstat == WBYTE)		\
	chstat = SBYTE, lost_multi = 1

KNOWN_ENCODINGS known_encodings[] = {
    { SJIS, sjis2jis, "sjis" },
    { EUCJ, euc2jis, "eucj" },
    { GB, euc2jis, "gb" },
    { BIG5, big5dummy, "big5" },
    { EUCKR, euc2jis, "kr" },
    { 0, NULL, NULL }
};
#else
# define RESET_CHSTAT
#endif

/* ------------------------------------------------------------------------- */
#define PROP_SIZE		4096
#define TABSIZE			8	/* default tab size */

#ifdef DEBUG_SCREEN
# define D_SCREEN(x)		fprintf x ; fputc('\n', stderr)
#else
# define D_SCREEN(x)
#endif
#ifdef DEBUG_SELECT
# define D_SELECT(x)		fprintf x ; fputc('\n', stderr)
#else
# define D_SELECT(x)
#endif

/* ------------------------------------------------------------------------- *
 *             GENERAL SCREEN AND SELECTION UPDATE ROUTINES                  *
 * ------------------------------------------------------------------------- */
#define ZERO_SCROLLBACK						\
    if ((Options & Opt_scrollTtyOutput) == Opt_scrollTtyOutput)	\
	TermWin.view_start = 0
#define CLEAR_SELECTION						\
    selection.beg.row = selection.beg.col			\
	= selection.end.row = selection.end.col = 0
#define CLEAR_ALL_SELECTION					\
    selection.beg.row = selection.beg.col			\
	= selection.mark.row = selection.mark.col		\
	= selection.end.row = selection.end.col = 0

#define ROW_AND_COL_IS_AFTER(A, B, C, D)				\
    (((A) > (C)) || (((A) == (C)) && ((B) > (D))))
#define ROW_AND_COL_IS_BEFORE(A, B, C, D)				\
    (((A) < (C)) || (((A) == (C)) && ((B) < (D))))
#define ROW_AND_COL_IN_ROW_AFTER(A, B, C, D)				\
    (((A) == (C)) && ((B) > (D)))
#define ROW_AND_COL_IN_ROW_AT_OR_AFTER(A, B, C, D)			\
    (((A) == (C)) && ((B) >= (D)))
#define ROW_AND_COL_IN_ROW_BEFORE(A, B, C, D)				\
    (((A) == (C)) && ((B) < (D)))
#define ROW_AND_COL_IN_ROW_AT_OR_BEFORE(A, B, C, D)			\
    (((A) == (C)) && ((B) <= (D)))

/* these must be row_col_t */
#define ROWCOL_IS_AFTER(X, Y)						\
    ROW_AND_COL_IS_AFTER((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IS_BEFORE(X, Y)						\
    ROW_AND_COL_IS_BEFORE((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_AFTER(X, Y)					\
    ROW_AND_COL_IN_ROW_AFTER((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_BEFORE(X, Y)					\
    ROW_AND_COL_IN_ROW_BEFORE((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_AT_OR_AFTER(X, Y)					\
    ROW_AND_COL_IN_ROW_AT_OR_AFTER((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_AT_OR_BEFORE(X, Y)				\
    ROW_AND_COL_IN_ROW_AT_OR_BEFORE((X).row, (X).col, (Y).row, (Y).col)

/*
 * CLEAR_ROWS : clear <num> rows starting from row <row>
 * CLEAR_CHARS: clear <num> chars starting from pixel position <x,y>
 * ERASE_ROWS : set <num> rows starting from row <row> to the foreground colour
 */
#if defined(XPM_BACKGROUND) && defined(XPM_BUFFERING)

#define drawBuffer	(TermWin.buf_pixmap)

#define CLEAR_ROWS(row, num)						\
    if (TermWin.mapped)							\
	XCopyArea(Xdisplay, TermWin.pixmap, drawBuffer, TermWin.gc,	\
		  Col2Pixel(0), Row2Pixel(row),				\
		  TermWin.width, Height2Pixel(num),			\
		  Col2Pixel(0), Row2Pixel(row))

#define CLEAR_CHARS(x, y, num)						\
    if (TermWin.mapped)							\
	XCopyArea(Xdisplay, TermWin.pixmap, drawBuffer, TermWin.gc,	\
		  x, y, Width2Pixel(num), Height2Pixel(1), x, y)

#else				/* XPM_BUFFERING && XPM_BACKGROUND */

#define drawBuffer	(TermWin.vt)

#define CLEAR_ROWS(row, num)						\
    if (TermWin.mapped)							\
	XClearArea(Xdisplay, drawBuffer, 0, Row2Pixel(row),		\
		   TermWin_TotalWidth(), Height2Pixel(num), False)

#define CLEAR_CHARS(x, y, num)						\
    if (TermWin.mapped)							\
	XClearArea(Xdisplay, drawBuffer, x, y,				\
		   Width2Pixel(num), Height2Pixel(1), False)

#endif				/* XPM_BUFFERING && XPM_BACKGROUND */

#define ERASE_ROWS(row, num)						\
    XFillRectangle(Xdisplay, drawBuffer, TermWin.gc,			\
		   0, Row2Pixel(row),					\
		   TermWin_TotalWidth(), Height2Pixel(num))

/* ------------------------------------------------------------------------- *
 *                        SCREEN `COMMON' ROUTINES                           *
 * ------------------------------------------------------------------------- */
/* Fill part/all of a line with blanks. */
/* INTPROTO */
void
blank_line(text_t * et, rend_t * er, int width, rend_t efs)
{
    MEMSET(et, ' ', width);
    for (; width--;)
	*er++ = efs;
}

/* ------------------------------------------------------------------------- */
/* Fill a full line with blanks - make sure it is allocated first */
/* INTPROTO */
void
blank_screen_mem(text_t ** tp, rend_t ** rp, int row, rend_t efs)
{
    int             width = TermWin.ncol;
    rend_t         *er;

    if (tp[row] == NULL) {
	tp[row] = MALLOC(sizeof(text_t) * TermWin.ncol);
	rp[row] = MALLOC(sizeof(rend_t) * TermWin.ncol);
    }
    MEMSET(tp[row], ' ', width);
    for (er = rp[row]; width--;)
	*er++ = efs;
}

/* ------------------------------------------------------------------------- *
 *                          SCREEN INITIALISATION                            *
 * ------------------------------------------------------------------------- */
static int      prev_nrow = -1, prev_ncol = -1;

/* EXTPROTO */
void
scr_reset(void)
{
    int             i, j, k, total_rows, prev_total_rows;
    rend_t          setrstyle;

    D_SCREEN((stderr, "scr_reset()"));

    TermWin.view_start = 0;
    RESET_CHSTAT;

    if (TermWin.ncol == prev_ncol && TermWin.nrow == prev_nrow)
	return;

    want_refresh = 1;

    if (TermWin.ncol <= 0)
	TermWin.ncol = 80;
    if (TermWin.nrow <= 0)
	TermWin.nrow = 24;
#ifdef DEBUG_STRICT
    assert(TermWin.saveLines >= 0);
#else				/* drive with your eyes closed */
    MAX_IT(TermWin.saveLines, 0);
#endif

    total_rows = TermWin.nrow + TermWin.saveLines;
    prev_total_rows = prev_nrow + TermWin.saveLines;

    screen.tscroll = 0;
    screen.bscroll = (TermWin.nrow - 1);

    if (prev_nrow == -1) {
/*
 * A: first time called so just malloc everything : don't rely on realloc
 *    Note: this is still needed so that all the scrollback lines are NULL
 */
	screen.text = CALLOC(total_rows, sizeof(text_t *));
	buf_text = CALLOC(total_rows, sizeof(text_t *));
	drawn_text = CALLOC(TermWin.nrow, sizeof(text_t *));
	swap.text = CALLOC(TermWin.nrow, sizeof(text_t *));

	screen.tlen = CALLOC(total_rows, sizeof(int16_t));
	buf_tlen = CALLOC(total_rows, sizeof(int16_t));
	swap.tlen = CALLOC(TermWin.nrow, sizeof(int16_t));

	screen.rend = CALLOC(total_rows, sizeof(rend_t *));
	buf_rend = CALLOC(total_rows, sizeof(rend_t *));
	drawn_rend = CALLOC(TermWin.nrow, sizeof(rend_t *));
	swap.rend = CALLOC(TermWin.nrow, sizeof(rend_t *));

	for (i = 0; i < TermWin.nrow; i++) {
	    j = i + TermWin.saveLines;
	    blank_screen_mem(screen.text, screen.rend, j, DEFAULT_RSTYLE);
	    blank_screen_mem(swap.text, swap.rend, i, DEFAULT_RSTYLE);
	    screen.tlen[j] = swap.tlen[i] = 0;
	    blank_screen_mem(drawn_text, drawn_rend, i, DEFAULT_RSTYLE);
	}
	TermWin.nscrolled = 0;	/* no saved lines */
	rstyle = DEFAULT_RSTYLE;
	screen.flags = Screen_DefaultFlags;
	screen.s_cur.row = screen.s_cur.col = 0;
	screen.s_charset = 0;
	screen.s_charset_char = 'B';
	screen.s_rstyle = rstyle;
#if NSCREENS
	swap.flags = Screen_DefaultFlags;
	swap.s_cur.row = swap.s_cur.col = 0;
	swap.s_charset = 0;
	swap.s_charset_char = 'B';
	swap.s_rstyle = rstyle;
#endif
	selection.text = NULL;
	selection.len = 0;
	selection.op = SELECTION_CLEAR;
	selection.screen = PRIMARY;
	selection.clicks = 0;
	CLEAR_ALL_SELECTION;
	MEMSET(charsets, 'B', sizeof(charsets));
	current_screen = PRIMARY;
	rvideo = 0;
#ifdef MULTICHAR_SET
# ifdef MULTICHAR_ENCODING
        encoding_method = MULTICHAR_ENCODING;
# endif
	multi_byte = 0;
	lost_multi = 0;
	chstat = SBYTE;
#endif

    } else {
/*
 * B1: add or delete rows as appropriate
 */
	setrstyle = DEFAULT_RSTYLE | (rvideo ? RS_RVid : 0);

	if (TermWin.nrow < prev_nrow) {
	/* delete rows */
	    k = min(TermWin.nscrolled, prev_nrow - TermWin.nrow);
	    scroll_text(0, prev_nrow - 1, k, 1);
	    for (i = TermWin.nrow; i < prev_nrow; i++) {
		j = i + TermWin.saveLines;
		if (screen.text[j])
		    FREE(screen.text[j]);
		if (screen.rend[j])
		    FREE(screen.rend[j]);
		if (swap.text[i])
		    FREE(swap.text[i]);
		if (swap.rend[i])
		    FREE(swap.rend[i]);
		FREE(drawn_text[i]);
		FREE(drawn_rend[i]);
	    }
	/* we have fewer rows so fix up cursor position */
	    MIN_IT(screen.cur.row, TermWin.nrow - 1);
	    MIN_IT(swap.cur.row, TermWin.nrow - 1);

	    scr_reset_realloc();	/* REALLOC _last_ */

	} else if (TermWin.nrow > prev_nrow) {
	/* add rows */
	    scr_reset_realloc();	/* REALLOC _first_ */

	    k = min(TermWin.nscrolled, TermWin.nrow - prev_nrow);
	    for (i = prev_total_rows; i < total_rows - k; i++) {
		screen.tlen[i] = 0;
		screen.text[i] = NULL;
		blank_screen_mem(screen.text, screen.rend, i, setrstyle);
	    }
	    for (i = total_rows - k; i < total_rows; i++) {
		screen.tlen[i] = 0;
		screen.text[i] = NULL;
		screen.rend[i] = NULL;
	    }
	    for (i = prev_nrow; i < TermWin.nrow; i++) {
		swap.tlen[i] = 0;
		swap.text[i] = NULL;
		drawn_text[i] = NULL;
		blank_screen_mem(swap.text, swap.rend, i, setrstyle);
		blank_screen_mem(drawn_text, drawn_rend, i, setrstyle);
	    }
	    if (k > 0) {
		scroll_text(0, TermWin.nrow - 1, -k, 1);
		screen.cur.row += k;
		TermWin.nscrolled -= k;
		for (i = TermWin.saveLines - TermWin.nscrolled; k--; i--)
		    if (screen.text[i] == NULL) {
			blank_screen_mem(screen.text, screen.rend, i,
					 setrstyle);
			screen.tlen[i] = 0;
		    }
	    }
#ifdef DEBUG_STRICT
	    assert(screen.cur.row < TermWin.nrow);
	    assert(swap.cur.row < TermWin.nrow);
#else				/* drive with your eyes closed */
	    MIN_IT(screen.cur.row, TermWin.nrow - 1);
	    MIN_IT(swap.cur.row, TermWin.nrow - 1);
#endif
	}
/* B2: resize columns */
	if (TermWin.ncol != prev_ncol) {
	    for (i = 0; i < total_rows; i++) {
		if (screen.text[i]) {
		    screen.text[i] = REALLOC(screen.text[i],
					     TermWin.ncol * sizeof(text_t));
		    screen.rend[i] = REALLOC(screen.rend[i],
					     TermWin.ncol * sizeof(rend_t));
		    MIN_IT(screen.tlen[i], TermWin.ncol);
		    if (TermWin.ncol > prev_ncol)
			blank_line(&(screen.text[i][prev_ncol]),
				   &(screen.rend[i][prev_ncol]),
				   TermWin.ncol - prev_ncol, setrstyle);
		}
	    }
	    for (i = 0; i < TermWin.nrow; i++) {
		drawn_text[i] = REALLOC(drawn_text[i],
					TermWin.ncol * sizeof(text_t));
		drawn_rend[i] = REALLOC(drawn_rend[i],
					TermWin.ncol * sizeof(rend_t));
		if (swap.text[i]) {
		    swap.text[i] = REALLOC(swap.text[i],
					   TermWin.ncol * sizeof(text_t));
		    swap.rend[i] = REALLOC(swap.rend[i],
					   TermWin.ncol * sizeof(rend_t));
		    MIN_IT(swap.tlen[i], TermWin.ncol);
		    if (TermWin.ncol > prev_ncol)
			blank_line(&(swap.text[i][prev_ncol]),
				   &(swap.rend[i][prev_ncol]),
				   TermWin.ncol - prev_ncol, setrstyle);
		}
		if (TermWin.ncol > prev_ncol)
		    blank_line(&(drawn_text[i][prev_ncol]),
			       &(drawn_rend[i][prev_ncol]),
			       TermWin.ncol - prev_ncol, setrstyle);
	    }
	    MIN_IT(screen.cur.col, TermWin.ncol - 1);
	    MIN_IT(swap.cur.col, TermWin.ncol - 1);
	}
	if (tabs)
	    FREE(tabs);
    }

    tabs = MALLOC(TermWin.ncol * sizeof(char));

    for (i = 0; i < TermWin.ncol; i++)
	tabs[i] = (i % TABSIZE == 0) ? 1 : 0;

    prev_nrow = TermWin.nrow;
    prev_ncol = TermWin.ncol;

    tt_resize();
}

/* INTPROTO */
void
scr_reset_realloc(void)
{
    int             total_rows;

    total_rows = TermWin.nrow + TermWin.saveLines;
/* *INDENT-OFF* */
    screen.text = REALLOC(screen.text, total_rows   * sizeof(text_t *));
    buf_text    = REALLOC(buf_text   , total_rows   * sizeof(text_t *));
    drawn_text  = REALLOC(drawn_text , TermWin.nrow * sizeof(text_t *));
    swap.text   = REALLOC(swap.text  , TermWin.nrow * sizeof(text_t *));

    screen.tlen = REALLOC(screen.tlen, total_rows   * sizeof(int16_t));
    buf_tlen    = REALLOC(buf_tlen   , total_rows   * sizeof(int16_t));
    swap.tlen   = REALLOC(swap.tlen  , total_rows   * sizeof(int16_t));

    screen.rend = REALLOC(screen.rend, total_rows   * sizeof(rend_t *));
    buf_rend    = REALLOC(buf_rend   , total_rows   * sizeof(rend_t *));
    drawn_rend  = REALLOC(drawn_rend , TermWin.nrow * sizeof(rend_t *));
    swap.rend   = REALLOC(swap.rend  , TermWin.nrow * sizeof(rend_t *));
/* *INDENT-ON* */
}

/* ------------------------------------------------------------------------- */
/*
 * Free everything.  That way malloc debugging can find leakage.
 */
/* EXTPROTO */
void
scr_release(void)
{
    int             i, total_rows;

    total_rows = TermWin.nrow + TermWin.saveLines;
    for (i = 0; i < total_rows; i++) {
	if (screen.text[i]) {	/* then so is screen.rend[i] */
	    FREE(screen.text[i]);
	    FREE(screen.rend[i]);
	}
    }
    for (i = 0; i < TermWin.nrow; i++) {
	FREE(drawn_text[i]);
	FREE(drawn_rend[i]);
	FREE(swap.text[i]);
	FREE(swap.rend[i]);
    }
    FREE(screen.text);
    FREE(screen.tlen);
    FREE(screen.rend);
    FREE(drawn_text);
    FREE(drawn_rend);
    FREE(swap.text);
    FREE(swap.tlen);
    FREE(swap.rend);
    FREE(buf_text);
    FREE(buf_tlen);
    FREE(buf_rend);
    FREE(tabs);

/* NULL these so if anything tries to use them, we'll know about it */
    screen.text = drawn_text = swap.text = NULL;
    screen.rend = drawn_rend = swap.rend = NULL;
    screen.tlen = swap.tlen = buf_tlen = NULL;
    buf_text = NULL;
    buf_rend = NULL;
    tabs = NULL;
}

/* ------------------------------------------------------------------------- */
/* EXTPROTO */
void
scr_poweron(void)
{
    D_SCREEN((stderr, "scr_poweron()"));

    MEMSET(charsets, 'B', sizeof(charsets));
    rvideo = 0;
    swap.tscroll = 0;
    swap.bscroll = TermWin.nrow - 1;
    screen.cur.row = screen.cur.col = swap.cur.row = swap.cur.col = 0;
    screen.charset = swap.charset = 0;
    screen.flags = swap.flags = Screen_DefaultFlags;

    scr_cursor(SAVE);

    scr_release();
    prev_nrow = -1;
    prev_ncol = -1;
    scr_reset();

    scr_clear();
    scr_refresh(SLOW_REFRESH);
    Gr_reset();
}

/* ------------------------------------------------------------------------- *
 *                         PROCESS SCREEN COMMANDS                           *
 * ------------------------------------------------------------------------- */
/*
 * Save and Restore cursor
 * XTERM_SEQ: Save cursor   : ESC 7
 * XTERM_SEQ: Restore cursor: ESC 8
 */
/* EXTPROTO */
void
scr_cursor(int mode)
{
    screen_t       *s;
    D_SCREEN((stderr, "scr_cursor(%c)", mode));

#if NSCREENS && !defined(NO_SECONDARY_SCREEN_CURSOR)
    if (current_screen == SECONDARY)
	s = &swap;
    else
#endif
	s = &screen;
    switch (mode) {
    case SAVE:
	s->s_cur.row = s->cur.row;
	s->s_cur.col = s->cur.col;
	s->s_rstyle = rstyle;
	s->s_charset = s->charset;
	s->s_charset_char = charsets[s->charset];
	break;
    case RESTORE:
	want_refresh = 1;
	s->cur.row = s->s_cur.row;
	s->cur.col = s->s_cur.col;
	rstyle = s->s_rstyle;
	s->charset = s->s_charset;
	charsets[s->charset] = s->s_charset_char;
	set_font_style();
	break;
    }
/* boundary check in case screen size changed between SAVE and RESTORE */
    MIN_IT(s->cur.row, TermWin.nrow - 1);
    MIN_IT(s->cur.col, TermWin.ncol - 1);
#ifdef DEBUG_STRICT
    assert(s->cur.row >= 0);
    assert(s->cur.col >= 0);
#else				/* drive with your eyes closed */
    MAX_IT(s->cur.row, 0);
    MAX_IT(s->cur.col, 0);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Swap between primary and secondary screens
 * XTERM_SEQ: Primary screen  : ESC [ ? 4 7 h
 * XTERM_SEQ: Secondary screen: ESC [ ? 4 7 l
 */
/* EXTPROTO */
int
scr_change_screen(int scrn)
{
    int             i, tmp;
#if NSCREENS
    int             offset;
    text_t         *t0;
    rend_t         *r0;
    int16_t         l0;
#endif

    want_refresh = 1;

    D_SCREEN((stderr, "scr_change_screen(%d)", scrn));

    TermWin.view_start = 0;
    RESET_CHSTAT;

    if (current_screen == scrn)
	return current_screen;

    selection_check(2);		/* check for boundary cross */

    SWAP_IT(current_screen, scrn, tmp);
#if NSCREENS
    offset = TermWin.saveLines;
    for (i = TermWin.nrow; i--;) {
	SWAP_IT(screen.text[i + offset], swap.text[i], t0);
	SWAP_IT(screen.tlen[i + offset], swap.tlen[i], l0);
	SWAP_IT(screen.rend[i + offset], swap.rend[i], r0);
    }
    SWAP_IT(screen.cur.row, swap.cur.row, l0);
    SWAP_IT(screen.cur.col, swap.cur.col, l0);
# ifdef DEBUG_STRICT
    assert(screen.cur.row >= 0);
    assert(screen.cur.col >= 0);
    assert(screen.cur.row < TermWin.nrow);
    assert(screen.cur.col < TermWin.ncol);
# else				/* drive with your eyes closed */
    MAX_IT(screen.cur.row, 0);
    MAX_IT(screen.cur.col, 0);
    MIN_IT(screen.cur.row, TermWin.nrow - 1);
    MIN_IT(screen.cur.col, TermWin.ncol - 1);
# endif
    SWAP_IT(screen.charset, swap.charset, l0);
    SWAP_IT(screen.flags, swap.flags, tmp);
    screen.flags |= Screen_VisibleCursor;
    swap.flags |= Screen_VisibleCursor;

    if (Gr_Displayed()) {
	Gr_scroll(0);
	Gr_ChangeScreen();
    }
#else
# ifdef SCROLL_ON_NO_SECONDARY
    if (Gr_Displayed())
	Gr_ClearScreen();
    if (current_screen == PRIMARY) {
	if (!Gr_Displayed())
	    scroll_text(0, (TermWin.nrow - 1), TermWin.nrow, 0);
	for (i = TermWin.saveLines; i < TermWin.nrow + TermWin.saveLines; i++)
	    if (screen.text[i] == NULL) {
		blank_screen_mem(screen.text, screen.rend, i, DEFAULT_RSTYLE);
		screen.tlen[i] = 0;
	    }
    }
# endif
#endif
    return scrn;
}

/* ------------------------------------------------------------------------- */
/*
 * Change the colour for following text
 */
/* EXTPROTO */
void
scr_color(unsigned int color, unsigned int Intensity)
{
    if (color == restoreFG)
	color = Color_fg;
    else if (color == restoreBG)
	color = Color_bg;
    else {
	if (Xdepth <= 2) {	/* Monochrome - ignore colour changes */
	    switch (Intensity) {
	    case RS_Bold:
		color = Color_fg;
		break;
	    case RS_Blink:
		color = Color_bg;
		break;
	    }
	} else {
#ifndef NO_BRIGHTCOLOR
	    if ((rstyle & Intensity) && color >= minCOLOR && color <= maxCOLOR)
		color += (minBrightCOLOR - minCOLOR);
	    else if (color >= minBrightCOLOR && color <= maxBrightCOLOR) {
		if (rstyle & Intensity)
		    return;
		color -= (minBrightCOLOR - minCOLOR);
	    }
#endif
	}
    }
    switch (Intensity) {
    case RS_Bold:
	rstyle = SET_FGCOLOR(rstyle, color);
	break;
    case RS_Blink:
	rstyle = SET_BGCOLOR(rstyle, color);
	break;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Change the rendition style for following text
 */
/* EXTPROTO */
void
scr_rendition(int set, int style)
{
    unsigned int    color;
    rend_t          font_attr;

    if (set) {
/* A: Set style */
	rstyle |= style;
	switch (style) {
	case RS_RVid:
	    if (rvideo)
		rstyle &= ~RS_RVid;
	    break;
#ifndef NO_BRIGHTCOLOR
	case RS_Bold:
	    color = GET_FGCOLOR(rstyle);
	    scr_color((color == Color_fg ? GET_FGCOLOR(colorfgbg) : color),
		      RS_Bold);
	    break;
	case RS_Blink:
	    color = GET_BGCOLOR(rstyle);
	    scr_color((color == Color_bg ? GET_BGCOLOR(colorfgbg) : color),
		      RS_Blink);
	    break;
#endif
	}
    } else {
/* B: Unset style */
	font_attr = rstyle & RS_fontMask;
	rstyle &= ~style;

	switch (style) {
	case ~RS_None:		/* default fg/bg colours */
	    rstyle = DEFAULT_RSTYLE | font_attr;
	/* FALLTHROUGH */
	case RS_RVid:
	    if (rvideo)
		rstyle |= RS_RVid;
	    break;
#ifndef NO_BRIGHTCOLOR
	case RS_Bold:
	    color = GET_FGCOLOR(rstyle);
	    if (color >= minBrightCOLOR && color <= maxBrightCOLOR) {
		scr_color(color, RS_Bold);
		if (((rstyle ^ colorfgbg) & RS_fgMask) == 0)
		    scr_color(restoreFG, RS_Bold);
	    }
	    break;
	case RS_Blink:
	    color = GET_BGCOLOR(rstyle);
	    if (color >= minBrightCOLOR && color <= maxBrightCOLOR) {
		scr_color(color, RS_Blink);
		if (((rstyle ^ colorfgbg) & RS_bgMask) == 0)
		    scr_color(restoreBG, RS_Blink);
	    }
	    break;
#endif
	}
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Scroll text between <row1> and <row2> inclusive, by <count> lines
 * count positive ==> scroll up
 * count negative ==> scroll down
 * spec == 0 for normal routines
 */
/* INTPROTO */
int
scroll_text(int row1, int row2, int count, int spec)
{
    int             i, j;

    want_refresh = 1;
    D_SCREEN((stderr, "scroll_text(%d,%d,%d,%d): %s", row1, row2, count, spec, (current_screen == PRIMARY) ? "Primary" : "Secondary"));

    if (count == 0 || (row1 > row2))
	return 0;

    if ((count > 0) && (row1 == 0) && (current_screen == PRIMARY)) {
	TermWin.nscrolled += count;
	MIN_IT(TermWin.nscrolled, TermWin.saveLines);
    } else if (!spec)
	row1 += TermWin.saveLines;
    row2 += TermWin.saveLines;

    if (selection.op && current_screen == selection.screen) {
	i = selection.beg.row + TermWin.saveLines;
	j = selection.end.row + TermWin.saveLines;
	if ((i < row1 && j > row1)
	    || (i < row2 && j > row2)
	    || (i - count < row1 && i >= row1)
	    || (i - count > row2 && i <= row2)
	    || (j - count < row1 && j >= row1)
	    || (j - count > row2 && j <= row2)) {
	    CLEAR_ALL_SELECTION;
	    selection.op = SELECTION_CLEAR;	/* XXX: too aggressive? */
	} else if (j >= row1 && j <= row2) {
	/* move selected region too */
	    selection.beg.row -= count;
	    selection.end.row -= count;
	    selection.mark.row -= count;
	}
    }
    selection_check(0);		/* _after_ TermWin.nscrolled update */

    if (count > 0) {
/* A: scroll up */

	MIN_IT(count, row2 - row1 + 1);
/* A1: Copy lines that will get clobbered by the rotation */
	for (i = 0, j = row1; i < count; i++, j++) {
	    buf_text[i] = screen.text[j];
	    buf_tlen[i] = screen.tlen[j];
	    buf_rend[i] = screen.rend[j];
	}
/* A2: Rotate lines */
	for (j = row1; (j + count) <= row2; j++) {
	    screen.text[j] = screen.text[j + count];
	    screen.tlen[j] = screen.tlen[j + count];
	    screen.rend[j] = screen.rend[j + count];
	}
/* A3: Resurrect lines */
	for (i = 0; i < count; i++, j++) {
	    screen.text[j] = buf_text[i];
	    screen.tlen[j] = buf_tlen[i];
	    screen.rend[j] = buf_rend[i];
	}
    } else if (count < 0) {
/* B: scroll down */

	count = min(-count, row2 - row1 + 1);
/* B1: Copy lines that will get clobbered by the rotation */
	for (i = 0, j = row2; i < count; i++, j--) {
	    buf_text[i] = screen.text[j];
	    buf_tlen[i] = screen.tlen[j];
	    buf_rend[i] = screen.rend[j];
	}
/* B2: Rotate lines */
	for (j = row2; (j - count) >= row1; j--) {
	    screen.text[j] = screen.text[j - count];
	    screen.tlen[j] = screen.tlen[j - count];
	    screen.rend[j] = screen.rend[j - count];
	}
/* B3: Resurrect lines */
	for (i = 0, j = row1; i < count; i++, j++) {
	    screen.text[j] = buf_text[i];
	    screen.tlen[j] = buf_tlen[i];
	    screen.rend[j] = buf_rend[i];
	}
	count = -count;
    }
    if (Gr_Displayed())
	Gr_scroll(count);
    return count;
}

/* ------------------------------------------------------------------------- */
/*
 * A safe scroll text routine
 */
/* EXTPROTO */
void
scr_scroll_text(int count)
{
    int             row, erow;

    if (count == 0)
	return;
    count = scroll_text(screen.tscroll, screen.bscroll, count, 0);
/* XXX: Ummm, no?  care needed with [bt]scroll, yes? */
    if (count > 0) {
	row = TermWin.nrow - count + TermWin.saveLines;
	erow = TermWin.nrow + TermWin.saveLines;
    } else {
	row = TermWin.saveLines;
	erow = TermWin.saveLines - count;
    }
    for (; row < erow; row++)
	if (screen.text[row] == NULL)
	    blank_screen_mem(screen.text, screen.rend, row, rstyle);
}

/* ------------------------------------------------------------------------- */
/*
 * Add text given in <str> of length <len> to screen struct
 */
/* EXTPROTO */
void
scr_add_lines(const unsigned char *str, int nlines, int len)
{
    char            c;
    int             i, j, row, last_col, checksel, clearsel;
    text_t         *stp;
    rend_t         *srp;

    if (len <= 0)		/* sanity */
	return;

    want_refresh = 1;
    last_col = TermWin.ncol;

    D_SCREEN((stderr, "scr_add_lines(*,%d,%d)", nlines, len));
    ZERO_SCROLLBACK;
    if (nlines > 0) {
	nlines += (screen.cur.row - screen.bscroll);
	if ((nlines > 0)
	    && (screen.tscroll == 0)
	    && (screen.bscroll == (TermWin.nrow - 1))) {
	/* _at least_ this many lines need to be scrolled */
	    scroll_text(screen.tscroll, screen.bscroll, nlines, 0);
	    for (i = nlines, j = screen.bscroll + TermWin.saveLines; i--; j--) {
		blank_screen_mem(screen.text, screen.rend, j, rstyle);
		screen.tlen[j] = 0;
	    }
	    screen.cur.row -= nlines;
	}
    }
#ifdef DEBUG_STRICT
    assert(screen.cur.col < last_col);
    assert(screen.cur.row < TermWin.nrow);
    assert(screen.cur.row >= -TermWin.nscrolled);
#else				/* drive with your eyes closed */
    MIN_IT(screen.cur.col, last_col - 1);
    MIN_IT(screen.cur.row, TermWin.nrow - 1);
    MAX_IT(screen.cur.row, -TermWin.nscrolled);
#endif
    row = screen.cur.row + TermWin.saveLines;

    checksel = (selection.op && current_screen == selection.screen) ? 1 : 0;
    clearsel = 0;

    stp = screen.text[row];
    srp = screen.rend[row];

#ifdef MULTICHAR_SET
    if (lost_multi && screen.cur.col > 0
	&& IS_MULTI1(srp[screen.cur.col - 1])
	&& *str != '\n' && *str != '\r' && *str != '\t')
	chstat = WBYTE;
#endif

    for (i = 0; i < len;) {
	c = str[i++];
	switch (c) {
	case '\t':
	    scr_tab(1);
	    continue;
	case '\n':
	    if (screen.tlen[row] != -1)		/* XXX: think about this */
		MAX_IT(screen.tlen[row], screen.cur.col);
	    screen.flags &= ~Screen_WrapNext;
	    if (screen.cur.row == screen.bscroll) {
		scroll_text(screen.tscroll, screen.bscroll, 1, 0);
		j = screen.bscroll + TermWin.saveLines;
		blank_screen_mem(screen.text, screen.rend, j,
				 rstyle);
		screen.tlen[j] = 0;
	    } else if (screen.cur.row < (TermWin.nrow - 1))
		row = (++screen.cur.row) + TermWin.saveLines;
	    stp = screen.text[row];	/* _must_ refresh */
	    srp = screen.rend[row];	/* _must_ refresh */
	    RESET_CHSTAT;
	    continue;
	case '\r':
	    if (screen.tlen[row] != -1)		/* XXX: think about this */
		MAX_IT(screen.tlen[row], screen.cur.col);
	    screen.flags &= ~Screen_WrapNext;
	    screen.cur.col = 0;
	    RESET_CHSTAT;
	    continue;
	default:
#ifdef MULTICHAR_SET
	    rstyle &= ~RS_multiMask;
	    if (chstat == WBYTE) {
		rstyle |= RS_multi2;	/* multibyte 2nd byte */
		chstat = SBYTE;
		if ((encoding_method == EUCJ) || (encoding_method == GB))
		    c |= 0x80;	/* maybe overkill, but makes it selectable */
	    } else if (chstat == SBYTE) {
		if (multi_byte || (c & 0x80)) {	/* multibyte 1st byte */
		    rstyle |= RS_multi1;
		    chstat = WBYTE;
		    if ((encoding_method == EUCJ) || (encoding_method == GB))
			c |= 0x80; /* maybe overkill, but makes selectable */
		}
	    } else
#endif
	    if (c == 127)
		continue;	/* yummmm..... */
	    break;
	}

	if (checksel
	    && !ROWCOL_IS_BEFORE(screen.cur, selection.beg)
	    && ROWCOL_IS_BEFORE(screen.cur, selection.end)) {
	    checksel = 0;
	    clearsel = 1;
	}
	if (screen.flags & Screen_WrapNext) {
	    screen.tlen[row] = -1;
	    if (screen.cur.row == screen.bscroll) {
		scroll_text(screen.tscroll, screen.bscroll, 1, 0);
		j = screen.bscroll + TermWin.saveLines;
		blank_screen_mem(screen.text, screen.rend, j, rstyle);
		screen.tlen[j] = 0;
	    } else if (screen.cur.row < (TermWin.nrow - 1))
		row = (++screen.cur.row) + TermWin.saveLines;
	    stp = screen.text[row];	/* _must_ refresh */
	    srp = screen.rend[row];	/* _must_ refresh */
	    screen.cur.col = 0;
	    screen.flags &= ~Screen_WrapNext;
	}
	if (screen.flags & Screen_Insert)
	    scr_insdel_chars(1, INSERT);
#ifdef MULTICHAR_SET
	if (IS_MULTI1(rstyle)
	    && screen.cur.col > 0
	    && IS_MULTI1(srp[screen.cur.col - 1])) {
	    stp[screen.cur.col - 1] = ' ';
	    srp[screen.cur.col - 1] &= ~RS_multiMask;
	} else if (IS_MULTI2(rstyle)
	    && screen.cur.col < (last_col - 1)
	    && IS_MULTI2(srp[screen.cur.col + 1])) {
	    stp[screen.cur.col + 1] = ' ';
	    srp[screen.cur.col + 1] &= ~RS_multiMask;
	}
#endif
	stp[screen.cur.col] = c;
	srp[screen.cur.col] = rstyle;
	if (screen.cur.col < (last_col - 1))
	    screen.cur.col++;
	else {
	    screen.tlen[row] = last_col;
	    if (screen.flags & Screen_Autowrap)
		screen.flags |= Screen_WrapNext;
	    else
		screen.flags &= ~Screen_WrapNext;
	}
    }
    if (screen.tlen[row] != -1)	/* XXX: think about this */
	MAX_IT(screen.tlen[row], screen.cur.col);

/*
 * If we wrote anywhere in the selected area, kill the selection
 * XXX: should we kill the mark too?  Possibly, but maybe that
 *      should be a similar check.
 */
    if (clearsel)
	CLEAR_SELECTION;

#ifdef DEBUG_STRICT
    assert(screen.cur.row >= 0);
#else				/* drive with your eyes closed */
    MAX_IT(screen.cur.row, 0);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Process Backspace.  Move back the cursor back a position, wrap if have to
 * XTERM_SEQ: CTRL-H
 */
/* EXTPROTO */
void
scr_backspace(void)
{
    RESET_CHSTAT;
    want_refresh = 1;
    if (screen.cur.col == 0) {
	if (screen.cur.row > 0) {
#ifdef SUPPORT_BROKEN_APPS_WHICH_RELY_ON_UNDEFINED_BW_BEHAVIOUR_AS_XTERM
	    screen.flags &= ~Screen_WrapNext;
#else
	    screen.cur.col = TermWin.ncol - 1;
	    screen.cur.row--;
#endif
	} else
	    screen.flags &= ~Screen_WrapNext;
    } else if (screen.flags & Screen_WrapNext) {
	screen.flags &= ~Screen_WrapNext;
    } else
	scr_gotorc(0, -1, RELATIVE);
}

/* ------------------------------------------------------------------------- */
/*
 * Process Horizontal Tab
 * count: +ve = forward; -ve = backwards
 * XTERM_SEQ: CTRL-I
 */
/* EXTPROTO */
void
scr_tab(int count)
{
    int             i, x;

    want_refresh = 1;
    RESET_CHSTAT;
    x = screen.cur.col;
    if (count == 0)
	return;
    else if (count > 0) {
	for (i = x + 1; i < TermWin.ncol; i++) {
	    if (tabs[i]) {
		x = i;
		if (!--count)
		    break;
	    }
	}
	if (count)
	    x = TermWin.ncol - 1;
    } else if (count < 0) {
	for (i = x - 1; i >= 0; i--) {
	    if (tabs[i]) {
		x = i;
		if (!++count)
		    break;
	    }
	}
	if (count)
	    x = 0;
    }
    if (x != screen.cur.col)
	scr_gotorc(0, x, R_RELATIVE);
}
/* ------------------------------------------------------------------------- */
/*
 * Process DEC Back Index
 * XTERM_SEQ: ESC 6
 * Move cursor left in row.  If we're at the left boundary, shift everything
 * in that row right.  Clear left column.
 */
#ifndef NO_FRILLS
/* EXTPROTO */
void
scr_backindex(void)
{
    int             i, row;
    text_t         *t0;
    rend_t         *r0;

    want_refresh = 1;
    if (screen.cur.col > 0)
	scr_gotorc(0, screen.cur.col - 1, R_RELATIVE);
    else {
	row = screen.cur.row + TermWin.saveLines;
	if (screen.tlen[row] == 0)
	    return;		/* um, yeah? */
	else if (screen.tlen[row] < TermWin.ncol - 1)
	    screen.tlen[row]++;
	t0 = screen.text[row];
	r0 = screen.rend[row];
	for (i = TermWin.ncol; i-- > 1;) {
	    t0[i] = t0[i - 1];
	    r0[i] = r0[i - 1];
	}
	t0[0] = ' ';
	r0[0] = DEFAULT_RSTYLE;
/* TODO: Multi check on last character */
    }
}
#endif
/* ------------------------------------------------------------------------- */
/*
 * Process DEC Forward Index
 * XTERM_SEQ: ESC 9
 * Move cursor right in row.  If we're at the right boundary, shift everything
 * in that row left.  Clear right column.
 */
#ifndef NO_FRILLS
/* EXTPROTO */
void
scr_forwardindex(void)
{
    int             i, row;
    text_t         *t0;
    rend_t         *r0;

    want_refresh = 1;
    if (screen.cur.col < TermWin.ncol - 1)
	scr_gotorc(0, screen.cur.col + 1, R_RELATIVE);
    else {
	row = screen.cur.row + TermWin.saveLines;
	if (screen.tlen[row] == 0)
	    return;		/* um, yeah? */
	else if (screen.tlen[row] > 0)
	    screen.tlen[row]--;
	else
	    screen.tlen[row] = TermWin.ncol - 1;
	t0 = screen.text[row];
	r0 = screen.rend[row];
	for (i = 0; i < TermWin.ncol - 2; i++) {
	    t0[i] = t0[i + 1];
	    r0[i] = r0[i + 1];
	}
	t0[i] = ' ';
	r0[i] = DEFAULT_RSTYLE;
/* TODO: Multi check on first character */
    }
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Goto Row/Column
 */
/* EXTPROTO */
void
scr_gotorc(int row, int col, int relative)
{
    want_refresh = 1;
    ZERO_SCROLLBACK;
    RESET_CHSTAT;
    if (Gr_Displayed())
	Gr_scroll(0);

    D_SCREEN((stderr, "scr_gotorc(r:%d,c:%d,%d): from (r:%d,c:%d)", row, col, relative, screen.cur.row, screen.cur.col));

    screen.cur.col = ((relative & C_RELATIVE) ? (screen.cur.col + col) : col);
    MAX_IT(screen.cur.col, 0);
    MIN_IT(screen.cur.col, TermWin.ncol - 1);

    if (screen.flags & Screen_WrapNext)
	screen.flags &= ~Screen_WrapNext;
    if (relative & R_RELATIVE) {
	if (row > 0) {
	    if (screen.cur.row <= screen.bscroll
		&& (screen.cur.row + row) > screen.bscroll)
		screen.cur.row = screen.bscroll;
	    else
		screen.cur.row += row;
	} else if (row < 0) {
	    if (screen.cur.row >= screen.tscroll
		&& (screen.cur.row + row) < screen.tscroll)
		screen.cur.row = screen.tscroll;
	    else
		screen.cur.row += row;
	}
    } else {
	if (screen.flags & Screen_Relative) {	/* relative origin mode */
	    screen.cur.row = row + screen.tscroll;
	    MIN_IT(screen.cur.row, screen.bscroll);
	} else
	    screen.cur.row = row;
    }
    MAX_IT(screen.cur.row, 0);
    MIN_IT(screen.cur.row, TermWin.nrow - 1);
}

/* ------------------------------------------------------------------------- */
/*
 * direction  should be UP or DN
 */
/* EXTPROTO */
void
scr_index(int direction)
{
    int             dirn;

    want_refresh = 1;
    dirn = ((direction == UP) ? 1 : -1);
    D_SCREEN((stderr, "scr_index(%d)", dirn));

    ZERO_SCROLLBACK;
    RESET_CHSTAT;
    if (Gr_Displayed())
	Gr_scroll(0);

    if (screen.flags & Screen_WrapNext) {
	screen.flags &= ~Screen_WrapNext;
    }
    if ((screen.cur.row == screen.bscroll && direction == UP)
	|| (screen.cur.row == screen.tscroll && direction == DN)) {
	scroll_text(screen.tscroll, screen.bscroll, dirn, 0);
	if (direction == UP)
	    dirn = screen.bscroll + TermWin.saveLines;
	else
	    dirn = screen.tscroll + TermWin.saveLines;
	blank_screen_mem(screen.text, screen.rend, dirn, rstyle);
	screen.tlen[dirn] = 0;
    } else
	screen.cur.row += dirn;
    MAX_IT(screen.cur.row, 0);
    MIN_IT(screen.cur.row, TermWin.nrow - 1);
    selection_check(0);
}

/* ------------------------------------------------------------------------- */
/*
 * Erase part or whole of a line
 * XTERM_SEQ: Clear line to right: ESC [ 0 K
 * XTERM_SEQ: Clear line to left : ESC [ 1 K
 * XTERM_SEQ: Clear whole line   : ESC [ 2 K
 */
/* EXTPROTO */
void
scr_erase_line(int mode)
{
    int             row, col, num;

    want_refresh = 1;
    D_SCREEN((stderr, "scr_erase_line(%d) at screen row: %d", mode, screen.cur.row));
    ZERO_SCROLLBACK;
    RESET_CHSTAT;
    if (Gr_Displayed())
	Gr_scroll(0);
    selection_check(1);

    if (screen.flags & Screen_WrapNext)
	screen.flags &= ~Screen_WrapNext;

    row = TermWin.saveLines + screen.cur.row;
    switch (mode) {
    case 0:			/* erase to end of line */
	col = screen.cur.col;
	num = TermWin.ncol - col;
	MIN_IT(screen.tlen[row], col);
	if (ROWCOL_IN_ROW_AT_OR_AFTER(selection.beg, screen.cur)
	    || ROWCOL_IN_ROW_AT_OR_AFTER(selection.end, screen.cur))
	    CLEAR_SELECTION;
	break;
    case 1:			/* erase to beginning of line */
	col = 0;
	num = screen.cur.col + 1;
	if (ROWCOL_IN_ROW_AT_OR_BEFORE(selection.beg, screen.cur)
	    || ROWCOL_IN_ROW_AT_OR_BEFORE(selection.end, screen.cur))
	    CLEAR_SELECTION;
	break;
    case 2:			/* erase whole line */
	col = 0;
	num = TermWin.ncol;
	screen.tlen[row] = 0;
	if (selection.beg.row <= screen.cur.row
	    && selection.end.row >= screen.cur.row)
	    CLEAR_SELECTION;
	break;
    default:
	return;
    }
    if (screen.text[row])
	blank_line(&(screen.text[row][col]), &(screen.rend[row][col]), num,
		   rstyle & ~RS_Uline);
    else
	blank_screen_mem(screen.text, screen.rend, row, rstyle & ~RS_Uline);
}

/* ------------------------------------------------------------------------- */
/*
 * Erase part of whole of the screen
 * XTERM_SEQ: Clear screen after cursor : ESC [ 0 J
 * XTERM_SEQ: Clear screen before cursor: ESC [ 1 J
 * XTERM_SEQ: Clear whole screen        : ESC [ 2 J
 */
/* EXTPROTO */
void
scr_erase_screen(int mode)
{
    int             row, num, row_offset;
    rend_t          ren;
    long            gcmask;
    XGCValues       gcvalue;

    want_refresh = 1;
    D_SCREEN((stderr, "scr_erase_screen(%d) at screen row: %d", mode, screen.cur.row));
    ZERO_SCROLLBACK;
    RESET_CHSTAT;
    row_offset = TermWin.saveLines;

    switch (mode) {
    case 0:			/* erase to end of screen */
	selection_check(1);
	scr_erase_line(0);
	row = screen.cur.row + 1;	/* possible OOB */
	num = TermWin.nrow - row;
	break;
    case 1:			/* erase to beginning of screen */
	selection_check(3);
	scr_erase_line(1);
	row = 0;		/* possible OOB */
	num = screen.cur.row;
	break;
    case 2:			/* erase whole screen */
	selection_check(3);
	Gr_ClearScreen();
	row = 0;
	num = TermWin.nrow;
	break;
    default:
	return;
    }
    if (selection.op && current_screen == selection.screen
	&& ((selection.beg.row >= row && selection.beg.row <= row + num)
	    || (selection.end.row >= row && selection.end.row <= row + num)))
	CLEAR_SELECTION;
    if (row >= 0 && row < TermWin.nrow) {	/* check OOB */
	MIN_IT(num, (TermWin.nrow - row));
	if (rstyle & (RS_RVid | RS_Uline))
	    ren = (rend_t)~RS_None;
	else if (GET_BGCOLOR(rstyle) == Color_bg) {
	    ren = DEFAULT_RSTYLE;
	    CLEAR_ROWS(row, num);
	} else {
	    ren = (rstyle & (RS_fgMask | RS_bgMask));
	    gcvalue.foreground = PixColors[GET_BGCOLOR(ren)];
	    gcmask = GCForeground;
	    XChangeGC(Xdisplay, TermWin.gc, gcmask, &gcvalue);
	    ERASE_ROWS(row, num);
	    gcvalue.foreground = PixColors[Color_fg];
	    XChangeGC(Xdisplay, TermWin.gc, gcmask, &gcvalue);
	}
	for (; num--; row++) {
	    blank_screen_mem(screen.text, screen.rend, row + row_offset,
			     rstyle & ~RS_Uline);
	    screen.tlen[row + row_offset] = 0;
	    blank_line(drawn_text[row], drawn_rend[row], TermWin.ncol, ren);
	}
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Fill the screen with `E's
 * XTERM_SEQ: Screen Alignment Test: ESC # 8
 */
/* EXTPROTO */
void
scr_E(void)
{
    int             i, j;
    text_t         *t;
    rend_t         *r, fs;

    want_refresh = 1;
    ZERO_SCROLLBACK;
    RESET_CHSTAT;
    selection_check(3);

    fs = rstyle;
    for (i = TermWin.saveLines; i < TermWin.nrow + TermWin.saveLines; i++) {
	t = screen.text[i];
	r = screen.rend[i];
	for (j = 0; j < TermWin.ncol; j++) {
	    *t++ = 'E';
	    *r++ = fs;
	}
	screen.tlen[i] = TermWin.ncol;	/* make the `E's selectable */
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> lines
 */
/* EXTPROTO */
void
scr_insdel_lines(int count, int insdel)
{
    int             end;

    ZERO_SCROLLBACK;
    RESET_CHSTAT;
    if (Gr_Displayed())
	Gr_scroll(0);
    selection_check(1);

    if (screen.cur.row > screen.bscroll)
	return;

    end = screen.bscroll - screen.cur.row + 1;
    if (count > end) {
	if (insdel == DELETE)
	    return;
	else if (insdel == INSERT)
	    count = end;
    }
    if (screen.flags & Screen_WrapNext)
	screen.flags &= ~Screen_WrapNext;

    scroll_text(screen.cur.row, screen.bscroll, insdel * count, 0);

/* fill the inserted or new lines with rstyle. TODO: correct for delete? */
    if (insdel == DELETE)
	end = screen.bscroll + TermWin.saveLines;
    else if (insdel == INSERT)
	end = screen.cur.row + count - 1 + TermWin.saveLines;
    for (; count--;) {
	blank_screen_mem(screen.text, screen.rend, end, rstyle);
	screen.tlen[end--] = 0;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> characters from the current position
 */
/* EXTPROTO */
void
scr_insdel_chars(int count, int insdel)
{
    int             col, row;
    rend_t          tr;

    want_refresh = 1;
    ZERO_SCROLLBACK;
#if 0
    RESET_CHSTAT;
#endif
    if (Gr_Displayed())
	Gr_scroll(0);

    if (count <= 0)
	return;

    selection_check(1);
    MIN_IT(count, (TermWin.ncol - screen.cur.col));

    row = screen.cur.row + TermWin.saveLines;
    screen.flags &= ~Screen_WrapNext;

    switch (insdel) {
    case INSERT:
	for (col = TermWin.ncol - 1; (col - count) >= screen.cur.col; col--) {
	    screen.text[row][col] = screen.text[row][col - count];
	    screen.rend[row][col] = screen.rend[row][col - count];
	}
	if (screen.tlen[row] != -1) {
	    screen.tlen[row] += count;
	    MIN_IT(screen.tlen[row], TermWin.ncol);
	}
	if (selection.op && current_screen == selection.screen
	    && ROWCOL_IN_ROW_AT_OR_AFTER(selection.beg, screen.cur)) {
	    if (selection.end.row != screen.cur.row
		|| (selection.end.col + count >= TermWin.ncol))
		CLEAR_SELECTION;
	    else {		/* shift selection */
		selection.beg.col += count;
		selection.mark.col += count;	/* XXX: yes? */
		selection.end.col += count;
	    }
	}
	blank_line(&(screen.text[row][screen.cur.col]),
		   &(screen.rend[row][screen.cur.col]),
		   count, rstyle);
	break;
    case ERASE:
	screen.cur.col += count;	/* don't worry if > TermWin.ncol */
	selection_check(1);
	screen.cur.col -= count;
	blank_line(&(screen.text[row][screen.cur.col]),
		   &(screen.rend[row][screen.cur.col]),
		   count, rstyle);
	break;
    case DELETE:
	tr = screen.rend[row][TermWin.ncol - 1]
	     & (RS_fgMask | RS_bgMask | RS_baseattrMask);
	for (col = screen.cur.col; (col + count) < TermWin.ncol; col++) {
	    screen.text[row][col] = screen.text[row][col + count];
	    screen.rend[row][col] = screen.rend[row][col + count];
	}
	blank_line(&(screen.text[row][TermWin.ncol - count]),
		   &(screen.rend[row][TermWin.ncol - count]),
		   count, tr);
	if (screen.tlen[row] == -1)	/* break line continuation */
	    screen.tlen[row] = TermWin.ncol;
	screen.tlen[row] -= count;
	MAX_IT(screen.tlen[row], 0);
	if (selection.op && current_screen == selection.screen
	    && ROWCOL_IN_ROW_AT_OR_AFTER(selection.beg, screen.cur)) {
	    if (selection.end.row != screen.cur.row
		|| (screen.cur.col >= selection.beg.col - count)
		|| selection.end.col >= TermWin.ncol)
		CLEAR_SELECTION;
	    else {
	    /* shift selection */
		selection.beg.col -= count;
		selection.mark.col -= count;	/* XXX: yes? */
		selection.end.col -= count;
	    }
	}
	break;
    }
#if 0
    if (IS_MULTI2(screen.rend[row][0])) {
	screen.rend[row][0] &= ~RS_multiMask;
	screen.text[row][0] = ' ';
    }
    if (IS_MULTI1(screen.rend[row][TermWin.ncol - 1])) {
	screen.rend[row][TermWin.ncol - 1] &= ~RS_multiMask;
	screen.text[row][TermWin.ncol - 1] = ' ';
    }
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Set the scrolling region
 * XTERM_SEQ: Set region <top> - <bot> inclusive: ESC [ <top> ; <bot> r
 */
/* EXTPROTO */
void
scr_scroll_region(int top, int bot)
{
    MAX_IT(top, 0);
    MIN_IT(bot, TermWin.nrow - 1);
    if (top > bot)
	return;
    screen.tscroll = top;
    screen.bscroll = bot;
    scr_gotorc(0, 0, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Make the cursor visible/invisible
 * XTERM_SEQ: Make cursor visible  : ESC [ ? 25 h
 * XTERM_SEQ: Make cursor invisible: ESC [ ? 25 l
 */
/* EXTPROTO */
void
scr_cursor_visible(int mode)
{
    want_refresh = 1;
    if (mode)
	screen.flags |= Screen_VisibleCursor;
    else
	screen.flags &= ~Screen_VisibleCursor;
}

/* ------------------------------------------------------------------------- */
/*
 * Set/unset automatic wrapping
 * XTERM_SEQ: Set Wraparound  : ESC [ ? 7 h
 * XTERM_SEQ: Unset Wraparound: ESC [ ? 7 l
 */
/* EXTPROTO */
void
scr_autowrap(int mode)
{
    if (mode)
	screen.flags |= Screen_Autowrap;
    else
	screen.flags &= ~Screen_Autowrap;
}

/* ------------------------------------------------------------------------- */
/*
 * Set/unset margin origin mode
 * Absolute mode: line numbers are counted relative to top margin of screen
 *      and the cursor can be moved outside the scrolling region.
 * Relative mode: line numbers are relative to top margin of scrolling region
 *      and the cursor cannot be moved outside.
 * XTERM_SEQ: Set Absolute: ESC [ ? 6 h
 * XTERM_SEQ: Set Relative: ESC [ ? 6 l
 */
/* EXTPROTO */
void
scr_relative_origin(int mode)
{
    if (mode)
	screen.flags |= Screen_Relative;
    else
	screen.flags &= ~Screen_Relative;
    scr_gotorc(0, 0, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Set insert/replace mode
 * XTERM_SEQ: Set Insert mode : ESC [ ? 4 h
 * XTERM_SEQ: Set Replace mode: ESC [ ? 4 l
 */
/* EXTPROTO */
void
scr_insert_mode(int mode)
{
    if (mode)
	screen.flags |= Screen_Insert;
    else
	screen.flags &= ~Screen_Insert;
}

/* ------------------------------------------------------------------------- */
/*
 * Set/Unset tabs
 * XTERM_SEQ: Set tab at current column  : ESC H
 * XTERM_SEQ: Clear tab at current column: ESC [ 0 g
 * XTERM_SEQ: Clear all tabs             : ESC [ 3 g
 */
/* EXTPROTO */
void
scr_set_tab(int mode)
{
    if (mode < 0)
	MEMSET(tabs, 0, TermWin.ncol * sizeof(char));

    else if (screen.cur.col < TermWin.ncol)
	tabs[screen.cur.col] = (mode ? 1 : 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Set reverse/normal video
 * XTERM_SEQ: Reverse video: ESC [ ? 5 h
 * XTERM_SEQ: Normal video : ESC [ ? 5 l
 */
/* EXTPROTO */
void
scr_rvideo_mode(int mode)
{
    int             i, j;
    rend_t         *r;

    if (rvideo != mode) {
	rvideo = mode;
	rstyle ^= RS_RVid;

	for (i = 0; i < TermWin.nrow; i++) {
	    r = screen.rend[TermWin.saveLines + i];
	    for (j = 0; j < TermWin.ncol; j++)
		*r++ ^= RS_RVid;
	}
	scr_refresh(SLOW_REFRESH);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Report current cursor position
 * XTERM_SEQ: Report position: ESC [ 6 n
 */
/* EXTPROTO */
void
scr_report_position(void)
{
    tt_printf("\033[%d;%dR", screen.cur.row + 1, screen.cur.col + 1);
}

/* ------------------------------------------------------------------------- *
 *                                  FONTS                                    *
 * ------------------------------------------------------------------------- */

/*
 * Set font style
 */
/* INTPROTO */
void
set_font_style(void)
{
    rstyle &= ~RS_fontMask;
    switch (charsets[screen.charset]) {
    case '0':			/* DEC Special Character & Line Drawing Set */
	rstyle |= RS_acsFont;
	break;
    case 'A':			/* United Kingdom (UK) */
	rstyle |= RS_ukFont;
	break;
    case 'B':			/* United States (USASCII) */
	break;
    case '<':			/* Multinational character set */
	break;
    case '5':			/* Finnish character set */
	break;
    case 'C':			/* Finnish character set */
	break;
    case 'K':			/* German character set */
	break;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Choose a font
 * XTERM_SEQ: Invoke G0 character set: CTRL-O
 * XTERM_SEQ: Invoke G1 character set: CTRL-N
 * XTERM_SEQ: Invoke G2 character set: ESC N
 * XTERM_SEQ: Invoke G3 character set: ESC O
 */
/* EXTPROTO */
void
scr_charset_choose(int set)
{
    screen.charset = set;
    set_font_style();
}

/* ------------------------------------------------------------------------- */
/*
 * Set a font
 * XTERM_SEQ: Set G0 character set: ESC ( <C>
 * XTERM_SEQ: Set G1 character set: ESC ) <C>
 * XTERM_SEQ: Set G2 character set: ESC * <C>
 * XTERM_SEQ: Set G3 character set: ESC + <C>
 * See set_font_style for possible values for <C>
 */
/* EXTPROTO */
void
scr_charset_set(int set, unsigned int ch)
{
#ifdef MULTICHAR_SET
    multi_byte = (set < 0);
    set = abs(set);
#endif
    charsets[set] = (unsigned char)ch;
    set_font_style();
}

/* ------------------------------------------------------------------------- *
 *          MULTIPLE-CHARACTER FONT SET MANIPULATION FUNCTIONS               *
 * ------------------------------------------------------------------------- */
#ifdef MULTICHAR_SET
static void     (*multichar_decode) (unsigned char *str, int len) = euc2jis;

/* INTPROTO */
void
euc2jis(unsigned char *str, int len)
{
    register int    i;

    for (i = 0; i < len; i++)
	str[i] &= 0x7F;
}

/* ------------------------------------------------------------------------- */
/* INTPROTO */
void
sjis2jis(unsigned char *str, int len)
{
    register int    i;
    unsigned char  *high, *low;

    for (i = 0; i < len; i += 2, str += 2) {
	high = str;
	low = str + 1;
	(*high) -= (*high > 0x9F ? 0xB1 : 0x71);
	*high = (*high) * 2 + 1;
	if (*low > 0x9E) {
	    *low -= 0x7E;
	    (*high)++;
	} else {
	    if (*low > 0x7E)
		(*low)--;
	    *low -= 0x1F;
	}
    }
}

/* INTPROTO */
void
big5dummy(unsigned char *str, int len)
{
}

/* EXTPROTO */
void
set_multichar_encoding(const char *str)
{
    KNOWN_ENCODINGS *a;

    if (str && *str) {
	for (a = known_encodings; a->name; a++) {
	    if (STRCASECMP(str, a->name) == 0) {
		encoding_method = a->method;
		multichar_decode = a->func;
		break;
	    }
	}
    }
}
#endif				/* MULTICHAR_SET */

/* ------------------------------------------------------------------------- *
 *                           GRAPHICS COLOURS                                *
 * ------------------------------------------------------------------------- */

#ifdef RXVT_GRAPHICS
/* EXTPROTO */
int
scr_get_fgcolor(void)
{
    return GET_FGCOLOR(rstyle);
}

/* ------------------------------------------------------------------------- */
/* EXTPROTO */
int
scr_get_bgcolor(void)
{
    return GET_BGCOLOR(rstyle);
}
#endif

/* ------------------------------------------------------------------------- *
 *                        MAJOR SCREEN MANIPULATION                          *
 * ------------------------------------------------------------------------- */

/*
 * Refresh an area
 */
enum {
    PART_BEG = 0,
    PART_END,
#if defined(XPM_BACKGROUND) && defined(XPM_BUFFERING)
    FULL_BEG,
    FULL_END,
#endif
    RC_COUNT
};
/* EXTPROTO */
void
scr_expose(int x, int y, int width, int height, Bool refresh)
{
    int             i;
    row_col_t       rc[RC_COUNT];

    if (drawn_text == NULL)	/* sanity check */
	return;

/* round down */
    rc[PART_BEG].col = Pixel2Col(x);
    rc[PART_BEG].row = Pixel2Row(y);
/* round up */
    rc[PART_END].col = Pixel2Width(x + width + TermWin.fwidth - 1);
    rc[PART_END].row = Pixel2Row(y + height + TermWin.fheight - 1);

#if defined(XPM_BACKGROUND) && defined(XPM_BUFFERING)
/* round down */
    rc[FULL_END].col = Pixel2Width(x + width);
    rc[FULL_END].row = Pixel2Row(y + height);
/* round up */
    rc[FULL_BEG].col = Pixel2Col(x + TermWin.fwidth - 1);
    rc[FULL_BEG].row = Pixel2Row(y + TermWin.fheight - 1);
#endif

/* sanity checks */
    for (i = PART_BEG; i < RC_COUNT; i++) {
	MAX_IT(rc[i].col, 0);
	MAX_IT(rc[i].row, 0);
	MIN_IT(rc[i].col, TermWin.ncol - 1);
	MIN_IT(rc[i].row, TermWin.nrow - 1);
    }

    D_SCREEN((stderr, "scr_expose(x:%d, y:%d, w:%d, h:%d) area (c:%d,r:%d)-(c:%d,r:%d)", x, y, width, height, rc[PART_BEG}.col, rc[PART_BEG].row, rc[PART_END].col, rc[PART_END].row));

#if defined(XPM_BACKGROUND) && defined(XPM_BUFFERING)
/* supposedly we're exposed - so `clear' the fully exposed clear areas */
    x = Col2Pixel(rc[FULL_BEG].col);
    y = Row2Pixel(rc[FULL_BEG].row);
    width = Width2Pixel(rc[FULL_END].col - rc[FULL_BEG].col + 1);
    height = Height2Pixel(rc[FULL_END].row - rc[FULL_BEG].row + 1);
    XCopyArea(Xdisplay, TermWin.pixmap, drawBuffer, TermWin.gc,
	      x, y, width, height, x, y);
#endif

    for (i = rc[PART_BEG].row; i <= rc[PART_END].row; i++)
	MEMSET(&(drawn_text[i][rc[PART_BEG].col]), 0,
	       rc[PART_END].col - rc[PART_BEG].col + 1);

    if (refresh)
	scr_refresh(SLOW_REFRESH);
}

/* ------------------------------------------------------------------------- */
/*
 * Refresh the entire screen
 */
/* EXTPROTO */
void
scr_touch(Bool refresh)
{
    scr_expose(0, 0, TermWin.width, TermWin.height, refresh);
}

/* ------------------------------------------------------------------------- */
/*
 * Move the display so that the line represented by scrollbar value Y is at
 * the top of the screen
 */
/* EXTPROTO */
int
scr_move_to(int y, int len)
{
    int             start;

    want_refresh = 1;
    start = TermWin.view_start;
    if (y >= len)
	TermWin.view_start = 0;
    else {
	TermWin.view_start = ((len - y)
			      * (TermWin.nrow - 1 + TermWin.nscrolled) / len);
	if (TermWin.view_start < TermWin.nrow)
	    TermWin.view_start = 0;
	else
	    TermWin.view_start -= (TermWin.nrow - 1);
    }
    D_SCREEN((stderr, "scr_move_to(%d, %d) view_start:%d", y, len, TermWin.view_start));

    MIN_IT(TermWin.view_start, TermWin.nscrolled);

    if (Gr_Displayed())
	Gr_scroll(0);
    return (TermWin.view_start - start);
}
/* ------------------------------------------------------------------------- */
/*
 * Page the screen up/down nlines
 * direction  should be UP or DN
 */
/* EXTPROTO */
int
scr_page(int direction, int nlines)
{
    u_int16_t       start;

    D_SCREEN((stderr, "scr_page(%s, %d) view_start:%d", ((direction == UP) ? "UP" : "DN"), nlines, TermWin.view_start));

    start = TermWin.view_start;
    MAX_IT(nlines, 1);
    MIN_IT(nlines, TermWin.nrow);
    if (direction == UP)
	TermWin.view_start = TermWin.view_start < TermWin.nscrolled - nlines
			     ? TermWin.view_start + nlines
			     : TermWin.nscrolled;
    else
	TermWin.view_start = TermWin.view_start > nlines 
			     ? TermWin.view_start - nlines
			     : 0;

    if (Gr_Displayed())
	Gr_scroll(0);
    if (TermWin.view_start != start)
	want_refresh = 1;
    return (int)(TermWin.view_start - start);
}

/* ------------------------------------------------------------------------- */
/* EXTPROTO */
void
scr_bell(void)
{
#ifndef NO_BELL
# ifndef NO_MAPALERT
#  ifdef MAPALERT_OPTION
    if (Options & Opt_mapAlert)
#  endif
	XMapWindow(Xdisplay, TermWin.parent[0]);
# endif
    if (Options & Opt_visualBell) {
	scr_rvideo_mode(!rvideo);	/* scr_refresh() also done */
	scr_rvideo_mode(!rvideo);	/* scr_refresh() also done */
    } else
	XBell(Xdisplay, 0);
#endif
}

/* ------------------------------------------------------------------------- */
/* ARGSUSED */
/* EXTPROTO */
void
scr_printscreen(int fullhist)
{
#ifdef PRINTPIPE
    int             i, r, nrows, row_offset;
    text_t         *t;
    FILE           *fd;

    if ((fd = popen_printer()) == NULL)
	return;
    nrows = TermWin.nrow;
    row_offset = TermWin.saveLines;
    if (!fullhist)
	row_offset -= TermWin.view_start;
    else {
	nrows += TermWin.nscrolled;
	row_offset -= TermWin.nscrolled;
    }

    for (r = 0; r < nrows; r++) {
	t = screen.text[r + row_offset];
	for (i = TermWin.ncol - 1; i >= 0; i--)
	    if (!isspace(t[i]))
		break;
	fprintf(fd, "%.*s\n", (i + 1), t);
    }
    pclose_printer(fd);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Refresh the screen
 * drawn_text/drawn_rend contain the screen information before the update.
 * screen.text/screen.rend contain what the screen will change to.
 */

#define DRAW_STRING(Func, x, y, str, len)				\
    Func(Xdisplay, drawBuffer, TermWin.gc, (x), (y), (str), (len))

#if defined (NO_BRIGHTCOLOR) || defined (VERYBOLD)
# define MONO_BOLD(x)	((x) & (RS_Bold|RS_Blink))
#else
# define MONO_BOLD(x)	(((x) & RS_Bold) && fore == Color_fg)
#endif

#define FONT_WIDTH(X, Y)						\
    (X)->per_char[(Y) - (X)->min_char_or_byte2].width
#define FONT_RBEAR(X, Y)						\
    (X)->per_char[(Y) - (X)->min_char_or_byte2].rbearing
#define FONT_LBEAR(X, Y)						\
    (X)->per_char[(Y) - (X)->min_char_or_byte2].lbearing
#define IS_FONT_CHAR(X, Y)						\
    ((Y) >= (X)->min_char_or_byte2 && (Y) <= (X)->max_char_or_byte2)

/* EXTPROTO */
void
scr_refresh(int type)
{
    int             i, j,	/* tmp                                       */
                    col, row,	/* column/row we're processing               */
                    scrrow,	/* screen row offset                         */
                    row_offset,	/* basic offset in screen structure          */
                    currow,	/* cursor row at appropriate offset          */
                    clearfirst,	/* first character writes before cell        */
                    clearlast,	/* last character writes beyond cell         */
                    len, wlen,	/* text length screen/buffer                 */
                    fprop,	/* proportional font used                    */
                    rvid,	/* reverse video this position               */
                    rend,	/* rendition                                 */
                    fore, back,	/* desired foreground/background             */
                    wbyte,	/* we're in multibyte                        */
		    fontdiff,	/* current font size different to base font  */
                    morecur = 0,
		    xpixel,	/* x offset for start of drawing (font)      */
                    ypixel,	/* y offset for start of drawing (font)      */
		    ypixelc;	/* y offset for top of drawing               */
    long            gcmask,	/* Graphics Context mask                     */
                    gcmaskf;
    unsigned long   ltmp;
    rend_t          rt1, rt2;	/* tmp rend values                           */
#ifndef NO_CURSORCOLOR
    rend_t          ccol1,	/* Cursor colour                             */
                    ccol2,	/* Cursor colour2                            */
                    cc1;	/* store colours at cursor position(s)       */
# ifdef MULTICHAR_SET
    rend_t          cc2;	/* store colours at cursor position(s)       */
    int             inmulti;
# endif
#endif
    rend_t         *drp, *srp;	/* drawn-rend-pointer, screen-rend-pointer   */
    text_t         *dtp, *stp;	/* drawn-text-pointer, screen-text-pointer   */
    XGCValues       gcvalue;	/* Graphics Context values                   */
#ifndef NO_PIXEL_DROPPING_AVOIDANCE
    XFontStruct    *wf;		/* which font are we in                      */
#endif
#ifndef NO_BOLDFONT
    int             bfont = 0;	/* we've changed font to bold font           */
#endif
    static char    *buffer = NULL;
    static int      currmaxcol = 0;
#ifdef MULTICHAR_SET
    static int      oldcursormulti = 0;
#endif
    static row_col_t oldcursor = {-1, -1};
				/* is there an old outline cursor on screen? */
    int             (*draw_string) (), (*draw_image_string) ();

    if (type == NO_REFRESH || !TermWin.mapped)
	return;

/*
 * A: set up vars
 */
    if (currmaxcol < TermWin.ncol) {
	currmaxcol = TermWin.ncol;
	buffer = REALLOC(buffer, (sizeof(char) * (currmaxcol + 1)));
    }
    row_offset = TermWin.saveLines - TermWin.view_start;
    gcvalue.foreground = PixColors[Color_fg];
    gcvalue.background = PixColors[Color_bg];
/*
 * always go back to the base font - it's much safer
 */
    wbyte = 0;
    XSetFont(Xdisplay, TermWin.gc, TermWin.font->fid);
    draw_string = XDrawString;
    draw_image_string = XDrawImageString;
    clearfirst = clearlast = 0;

/*
 * B: reverse any characters which are selected
 */
    scr_reverse_selection();

#ifndef NO_PIXEL_DROPPING_AVOIDANCE
/*
 * C: pixel dropping avoidance.  Do this before the main refresh.  Do passes
 *    forwards and backwards across each line at the start, requiring a refresh
 *    of anything that will need to be refreshed due to pixels being dropped
 *    into our area by a neighbour character which has now been changed.
 */
# ifdef NO_BOLDFONT
    wf = TermWin.font;
# endif
    MEMSET(buffer, 0, currmaxcol);
    for (row = 0; row < TermWin.nrow; row++) {
	scrrow = row + row_offset;
	stp = screen.text[scrrow];
	srp = screen.rend[scrrow];
	dtp = drawn_text[row];
	drp = drawn_rend[row];
/*
 * C1: Left droppings
 */
	for (col = 0; col < TermWin.ncol; col++) {
	    if ((dtp[col] == stp[col] && drp[col] == srp[col])
		|| dtp[col] == ' ')
		continue;
# if ! defined (NO_BRIGHTCOLOR) && ! defined (VERYBOLD)
	    fore = GET_FGCOLOR(drp[col]);
# endif
# ifndef NO_BOLDFONT
	    if (MONO_BOLD(drp[col]) && TermWin.boldFont)
		wf = TermWin.boldFont;
	    else
		wf = TermWin.font;
# endif
	    if (wf->per_char == NULL
		|| (IS_FONT_CHAR(wf, dtp[col])
		    && FONT_LBEAR(wf, dtp[col]) < 0)) {
		if (col == 0)
		    clearfirst = 1;
		else
		    buffer[col - 1] = 1;
# if defined(MULTICHAR_SET) && ! defined(NO_BOLDOVERSTRIKE_MULTI)
		if (IS_MULTI1(srp[col])) {
		    col++;
		    /* wbyte = 1; */
		    continue;
		}
# endif
	    }
	}
/*
 * C2: Right droppings and write in buffered left results
 */
	for (col = TermWin.ncol - 1; col >= 0; col--) {
	    if (buffer[col + 1] == 0) {
		if ((dtp[col] == stp[col] && drp[col] == srp[col])
	            || dtp[col] == ' ')
		    continue;
	    }
# if ! defined (NO_BRIGHTCOLOR) && ! defined (VERYBOLD)
	    fore = GET_FGCOLOR(drp[col]);
# endif
	    i = !!MONO_BOLD(drp[col]);	/* 0 or 1 */
# ifndef NO_BOLDFONT
	    wf = (i && TermWin.boldFont) ? TermWin.boldFont : TermWin.font;
# endif
	    if (buffer[col + 1]
		|| wf->per_char == NULL
		|| (IS_FONT_CHAR(wf, dtp[col])
		    && (FONT_WIDTH(wf, dtp[col])
			< (FONT_RBEAR(wf, dtp[col]) + i)))) {
		buffer[col + 1] = 0;
		if (col == TermWin.ncol - 1)
		    clearlast = 1;
		else
		    dtp[col + 1] = 0;
# if defined(MULTICHAR_SET) && ! defined(NO_BOLDOVERSTRIKE_MULTI)
		if (IS_MULTI2(srp[col])) {
		    col--;
		    /* wbyte = 1; */
		    continue;
		}
# endif
	    }
	}
    }
#endif				/* NO_PIXEL_DROPPING_AVOIDANCE */

/*
 * D: set the cursor character(s)
 */
    currow = screen.cur.row + TermWin.saveLines;
    if ((screen.flags & Screen_VisibleCursor) && TermWin.focus) {
	srp = &(screen.rend[currow][screen.cur.col]);
	*srp ^= RS_RVid;
#ifndef NO_CURSORCOLOR
	cc1 = *srp & (RS_fgMask | RS_bgMask);
	if (Xdepth <= 2 || !rs.color[Color_cursor])
	    ccol1 = Color_fg;
	else
	    ccol1 = Color_cursor;
	if (Xdepth <= 2 || !rs.color[Color_cursor2])
	    ccol2 = Color_bg;
	else
	    ccol2 = Color_cursor2;
	*srp = SET_FGCOLOR(*srp, ccol1);
	*srp = SET_BGCOLOR(*srp, ccol2);
#endif
#ifdef MULTICHAR_SET
	rt1 = *srp & RS_multiMask;
	if (rt1 == RS_multi1) {
	    if (screen.cur.col < TermWin.ncol - 2 && IS_MULTI2(srp[1]))
		morecur = 1;
	} else if (rt1 == RS_multi2) {
	    if (screen.cur.col > 0 && IS_MULTI1(srp[-1]))
		morecur = -1;
	}
	if (morecur) {
	    srp += morecur;
	    *srp ^= RS_RVid;
	}
# ifndef NO_CURSORCOLOR
	if (morecur) {
	    cc2 = *srp & (RS_fgMask | RS_bgMask);
	    *srp = SET_FGCOLOR(*srp, ccol1);
	    *srp = SET_BGCOLOR(*srp, ccol2);
	}
# endif
#endif
    }
    i = 0;
    if (oldcursor.row != -1) {
    /* make sure no outline cursor is left around */
	if (screen.cur.row + TermWin.view_start != oldcursor.row
	    || screen.cur.col != oldcursor.col) {
	    if (oldcursor.row < TermWin.nrow && oldcursor.col < TermWin.ncol) {
		drawn_text[oldcursor.row][oldcursor.col] = 0;
#ifdef MULTICHAR_SET
		if (oldcursormulti) {
		    col = oldcursor.col + oldcursormulti;
		    if (col < TermWin.ncol)
			drawn_text[oldcursor.row][col] = 0;
		}
#endif
	    }
	    if (TermWin.focus || !(screen.flags & Screen_VisibleCursor))
		oldcursor.row = -1;
	    else
		i = 1;
	}
    } else if (!TermWin.focus)
	i = 1;
    if (i) {
	if (screen.cur.row + TermWin.view_start >= TermWin.nrow)
	    oldcursor.row = -1;
	else {
	    oldcursor.row = screen.cur.row + TermWin.view_start;
	    oldcursor.col = screen.cur.col;
#ifdef MULTICHAR_SET
	    oldcursormulti = morecur;
#endif
	}
    }
/*
 * E: OK, now the real pass
 */
    for (row = 0; row < TermWin.nrow; row++) {
	scrrow = row + row_offset;
	stp = screen.text[scrrow];
	srp = screen.rend[scrrow];
	dtp = drawn_text[row];
	drp = drawn_rend[row];
	for (col = 0; col < TermWin.ncol; col++) {
	/* compare new text with old - if exactly the same then continue */
	    rt1 = srp[col];	/* screen rendition */
	    rt2 = drp[col];	/* drawn rendition  */
	    if (stp[col] == dtp[col]	/* must match characters to skip */
		&& ((rt1 == rt2)	/* either rendition the same or  */
		    || (stp[col] == ' '	/* space w/ no bg change */
			&& GET_BGATTR(rt1) == GET_BGATTR(rt2)))) {
#ifdef MULTICHAR_SET
	    /* if first byte is Kanji then compare second bytes */
		if (!IS_MULTI1(rt1))
		    continue;
		else if (stp[col + 1] == dtp[col + 1]) {
		/* assume no corrupt characters on the screen */
		    col++;
		    continue;
		}
#else
		continue;
#endif
	    }
	/* redraw one or more characters */
	    dtp[col] = stp[col];
	    rend = drp[col] = srp[col];

	    len = 0;
	    buffer[len++] = stp[col];
	    ypixelc = Row2Pixel(row);
	    ypixel = ypixelc + TermWin.font->ascent;
	    xpixel = Col2Pixel(col);
	    fontdiff = 0;
	    wlen = 1;

/*
 * Find out the longest string we can write out at once
 */
	    fprop = TermWin.fprop;
#ifndef NO_BOLDFONT
	    if (MONO_BOLD(rend) && TermWin.boldFont != NULL)
		fprop = TermWin.bprop;
#endif
#ifndef MULTICHAR_SET
	    if (fprop == 0) {
#else
	    if (IS_MULTI1(rend)
		&& col < TermWin.ncol - 1 && IS_MULTI2(srp[col + 1])) {
		fprop = TermWin.mprop;
		if (!wbyte && TermWin.mfont) {
		    wbyte = 1;
		    XSetFont(Xdisplay, TermWin.gc, TermWin.mfont->fid);
		    fontdiff = TermWin.mprop;
		    draw_string = XDrawString16;
		    draw_image_string = XDrawImageString16;
		}
		if (TermWin.mfont == NULL) {
		    buffer[0] = buffer[1] = ' ';
		    len = 2;
		    col++;
		} else {
		/* double stepping - we're in multibyte font mode */
		    for (; ++col < TermWin.ncol;) {
		    /* XXX: could check sanity on 2nd byte */
			dtp[col] = stp[col];
			drp[col] = srp[col];
			buffer[len++] = stp[col];
			col++;
			if (fprop)	/* proportional multibyte font mode */
			    break;
			if ((col == TermWin.ncol) || (srp[col] != rend))
			    break;
			if ((stp[col] == dtp[col])
			    && (srp[col] == drp[col])
			    && (stp[col + 1] == dtp[col + 1]))
			    break;
			if (len == currmaxcol)
			    break;
			dtp[col] = stp[col];
			drp[col] = srp[col];
			buffer[len++] = stp[col];
		    }
		    col--;
		}
		if (buffer[0] & 0x80)
		    multichar_decode(buffer, len);
		wlen = len / 2;
	    } else {
		if (IS_MULTI1(rend)) {
		/* XXX : maybe do the same thing for RS_multi2 */
		/* corrupt character - you're outta there */
		    rend &= ~RS_multiMask;
		    drp[col] = rend;	/* TODO check: may also want */
		    dtp[col] = ' ';	/* to poke into stp/srp      */
		    buffer[0] = ' ';
		}
		if (wbyte) {
		    wbyte = 0;
		    XSetFont(Xdisplay, TermWin.gc, TermWin.font->fid);
		    draw_string = XDrawString;
		    draw_image_string = XDrawImageString;
		}
		if (fprop == 0) {
#endif
		/* single stepping - `normal' mode */
		    for (j = 0; ++col < TermWin.ncol - 1;) {
			if (rend != srp[col])
			    break;
			if (len == currmaxcol)
			    break;
			if ((stp[col] == dtp[col]) && (srp[col] == drp[col])) {
#ifdef INEXPENSIVE_LOCAL_X_CALLS
			    if (display_is_local)
				break;
#endif
			    j++;
			} else {
			    j = 0;
			    dtp[col] = stp[col];
			    drp[col] = srp[col];
			}
			buffer[len++] = stp[col];
		    }
		    col--;	/* went one too far.  move back */
		    len -= j;	/* dump any matching trailing chars */
		    wlen = len;
#ifdef MULTICHAR_SET
		}
#endif
	    }			/* for(;;) */
	    buffer[len] = '\0';

/*
 * Determine the attributes for the string
 */
	    fore = GET_FGCOLOR(rend);
	    back = GET_BGCOLOR(rend);
	    rend = GET_ATTR(rend);
	    gcmask = 0;
	    rvid = (rend & RS_RVid) ? 1 : 0;

	    switch (rend & RS_fontMask) {
	    case RS_acsFont:
		for (i = 0; i < len; i++)
		    if (buffer[i] == 0x5f)
			buffer[i] = 0x7f;
		    else if (buffer[i] > 0x5f && buffer[i] < 0x7f)
			buffer[i] -= 0x5f;
		break;
	    case RS_ukFont:
		for (i = 0; i < len; i++)
		    if (buffer[i] == '#')
			buffer[i] = 0x1e;
		break;
	    }
	    if (rvid)
		SWAP_IT(fore, back, i);
	    if (back != Color_bg) {
		gcvalue.background = PixColors[back];
		gcmask |= GCBackground;
	    }
	    if (fore != Color_fg) {
		gcvalue.foreground = PixColors[fore];
		gcmask |= GCForeground;
	    }
#ifndef NO_BOLDUNDERLINE
	    else if (rend & RS_Bold) {
		if (Xdepth > 2 && rs.color[Color_BD]
		    && PixColors[fore] != PixColors[Color_BD]
		    && PixColors[back] != PixColors[Color_BD]) {
		    gcvalue.foreground = PixColors[Color_BD];
		    gcmask |= GCForeground;
# ifndef VERYBOLD
		    rend &= ~RS_Bold;	/* we've taken care of it */
# endif
		}
	    } else if (rend & RS_Uline) {
		if (Xdepth > 2 && rs.color[Color_UL]
		    && PixColors[fore] != PixColors[Color_UL]
		    && PixColors[back] != PixColors[Color_UL]) {
		    gcvalue.foreground = PixColors[Color_UL];
		    gcmask |= GCForeground;
		    rend &= ~RS_Uline;	/* we've taken care of it */
		}
	    }
#endif
	    if (gcmask)
		XChangeGC(Xdisplay, TermWin.gc, gcmask, &gcvalue);
#ifndef NO_BOLDFONT
	    if (!wbyte && MONO_BOLD(rend) && TermWin.boldFont != NULL) {
		bfont = 1;
		XSetFont(Xdisplay, TermWin.gc, TermWin.boldFont->fid);
		fontdiff = TermWin.bprop;
		rend &= ~RS_Bold;	/* we've taken care of it */
	    } else if (bfont) {
		bfont = 0;
		XSetFont(Xdisplay, TermWin.gc, TermWin.font->fid);
	    }
#endif
/*
 * Actually do the drawing of the string here
 */
	    if (back == Color_bg) {
		CLEAR_CHARS(xpixel, ypixelc, len);
		DRAW_STRING(draw_string, xpixel, ypixel, buffer, wlen);
	    } else {
		if (fprop || fontdiff) {
		    gcmaskf = GCForeground;
		    ltmp = gcvalue.foreground;
		    gcvalue.foreground = gcvalue.background;
		    XChangeGC(Xdisplay, TermWin.gc, gcmaskf, &gcvalue);
		    XFillRectangle(Xdisplay, drawBuffer, TermWin.gc,
				   xpixel, ypixelc,
				   Width2Pixel(len), Height2Pixel(1));
		    gcvalue.foreground = ltmp;
		    XChangeGC(Xdisplay, TermWin.gc, gcmaskf, &gcvalue);
		    DRAW_STRING(draw_string, xpixel, ypixel, buffer, wlen);
		} else
		    DRAW_STRING(draw_image_string, xpixel, ypixel, buffer, wlen);
	    }
#ifndef NO_BOLDOVERSTRIKE
# ifdef NO_BOLDOVERSTRIKE_MULTI
	    if (!wbyte)
# endif
		if (MONO_BOLD(rend))
		    DRAW_STRING(draw_string, xpixel + 1, ypixel, buffer, wlen);
#endif
	    if ((rend & RS_Uline) && (TermWin.font->descent > 1))
		XDrawLine(Xdisplay, drawBuffer, TermWin.gc,
			  xpixel, ypixel + 1,
			  xpixel + Width2Pixel(len) - 1, ypixel + 1);
	    if (gcmask) {	/* restore normal colours */
		gcvalue.foreground = PixColors[Color_fg];
		gcvalue.background = PixColors[Color_bg];
		XChangeGC(Xdisplay, TermWin.gc, gcmask, &gcvalue);
	    }
	}
    }

/*
 * F: cleanup cursor and display outline cursor in necessary
 */
    if (screen.flags & Screen_VisibleCursor) {
	if (TermWin.focus) {
	    srp = &(screen.rend[currow][screen.cur.col]);
	    *srp ^= RS_RVid;
#ifndef NO_CURSORCOLOR
	    *srp = (*srp & ~(RS_fgMask | RS_bgMask)) | cc1;
#endif
	    if (morecur) {
		srp += morecur;
		*srp ^= RS_RVid;
#if defined(MULTICHAR_SET) && ! defined(NO_CURSORCOLOR)
		*srp = (*srp & ~(RS_fgMask | RS_bgMask)) | cc2;
#endif
	    }
	} else if (oldcursor.row >= 0) {
	    col = oldcursor.col + morecur;
	    wbyte = morecur ? 1 : 0;
#ifndef NO_CURSORCOLOR
	    gcmask = 0;
	    if (Xdepth > 2 && rs.color[Color_cursor]) {
		gcvalue.foreground = PixColors[Color_cursor];
		gcmask = GCForeground;
		XChangeGC(Xdisplay, TermWin.gc, gcmask, &gcvalue);
		gcvalue.foreground = PixColors[Color_fg];
	    }
#endif
	    XDrawRectangle(Xdisplay, drawBuffer, TermWin.gc,
			   Col2Pixel(col), Row2Pixel(oldcursor.row),
			   Width2Pixel(1 + wbyte) - 1,
			   Height2Pixel(1) - 1);
#ifndef NO_CURSORCOLOR
	    if (gcmask)		/* restore normal colours */
		XChangeGC(Xdisplay, TermWin.gc, gcmask, &gcvalue);
#endif
	}
    }
/*
 * G: cleanup selection
 */
    scr_reverse_selection();

/*
 * H: other general cleanup
 */
#if defined(XPM_BACKGROUND) && defined(XPM_BUFFERING)
    XClearWindow(Xdisplay, TermWin.vt);
#else
    if (clearfirst)		/* clear the whole screen height */
	XClearArea(Xdisplay, TermWin.vt, 0, 0,
		   TermWin.int_bwidth, TermWin_TotalHeight(), False);
    if (clearlast)		/* clear the whole screen height */
	XClearArea(Xdisplay, TermWin.vt, TermWin.width + TermWin.int_bwidth,
		   0, TermWin.int_bwidth, TermWin_TotalHeight(), False);
#endif
    if (type & SMOOTH_REFRESH)
	XSync(Xdisplay, False);

    want_refresh = 0;		/* screen is current */
}

/* EXTPROTO */
void
scr_clear(void)
{
    if (!TermWin.mapped)
	return;
#ifdef TRANSPARENT
    if (Options & Opt_transparent) {
	int             i;

	for (i = KNOW_PARENTS; i--;)
	    if (TermWin.parent[i] != None)
		XClearWindow(Xdisplay, TermWin.parent[i]);
    }
#endif
    XClearWindow(Xdisplay, TermWin.vt);
}

/* ------------------------------------------------------------------------- */
/* INTPROTO */
void
scr_reverse_selection(void)
{
    int             i, col, row, end_row;
    rend_t         *srp;

    end_row = TermWin.saveLines - TermWin.view_start;
    if (selection.op && current_screen == selection.screen) {
	i = selection.beg.row + TermWin.saveLines;
	row = selection.end.row + TermWin.saveLines;
	if (i >= end_row)
	    col = selection.beg.col;
	else {
	    col = 0;
	    i = end_row;
	}
	end_row += TermWin.nrow;
	for (; i < row && i < end_row; i++, col = 0)
	    for (srp = screen.rend[i]; col < TermWin.ncol; col++)
		srp[col] ^= RS_RVid;
	if (i == row && i < end_row)
	    for (srp = screen.rend[i]; col < selection.end.col; col++)
		srp[col] ^= RS_RVid;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Dump the whole scrollback and screen to the passed filedescriptor.  The
 * invoking routine must close the fd.
 */
/* EXTPROTO */
void
scr_dump(int fd)
{
    int             row, towrite, width, wrote;
    char            r[] = "\n";

    for (row = TermWin.saveLines - TermWin.nscrolled;
	 row < TermWin.saveLines + TermWin.nrow - 1; row++) {
	width = screen.tlen[row] >= 0 ? screen.tlen[row] : TermWin.ncol;
	for (towrite = width; towrite > 0; towrite -= wrote) {
	    wrote = write(fd, &(screen.text[row][width - towrite]), towrite);
	    if (wrote < 0)
		return;		/* XXX: death, no report */
	}
	if (screen.tlen[row] >= 0)
	    for (wrote = 0; wrote == 0; )
		if ((wrote = write(fd, r, 1)) < 0)
		    return;	/* XXX: death, no report */
    }
}


/* ------------------------------------------------------------------------- *
 *                           CHARACTER SELECTION                             *
 * ------------------------------------------------------------------------- */

/*
 * -TermWin.nscrolled <= (selection row) <= TermWin.nrow - 1
 */
/* INTPROTO */
void
selection_check(int check_more)
{
    row_col_t       pos;

    if (!selection.op)
	return;
    if ((selection.beg.row < -TermWin.nscrolled)
	|| (selection.beg.row >= TermWin.nrow)
	|| (selection.mark.row < -TermWin.nscrolled)
	|| (selection.mark.row >= TermWin.nrow)
	|| (selection.end.row < -TermWin.nscrolled)
	|| (selection.end.row >= TermWin.nrow))
	CLEAR_ALL_SELECTION;

    if (check_more == 1 && current_screen == selection.screen) {
    /* check for cursor position */
	pos.row = screen.cur.row;
	pos.col = screen.cur.col;
	if (!ROWCOL_IS_BEFORE(pos, selection.beg)
	    && ROWCOL_IS_BEFORE(pos, selection.end))
	    CLEAR_SELECTION;
    } else if (check_more == 2) {
	pos.row = 0;
	pos.col = 0;
	if (ROWCOL_IS_BEFORE(selection.beg, pos)
	    && ROWCOL_IS_AFTER(selection.end, pos))
	    CLEAR_SELECTION;
    } else if (check_more == 3) {
	pos.row = 0;
	pos.col = 0;
	if (ROWCOL_IS_AFTER(selection.end, pos))
	    CLEAR_SELECTION;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Paste a selection direct to the command
 */
/* INTPROTO */
void
PasteIt(const unsigned char *data, unsigned int nitems)
{
    int             num;
    const unsigned char *p;
    const unsigned char cr = '\r';

    for (p = data, num = 0; nitems--; p++)
	if (*p != '\n')
	    num++;
	else {
	    tt_write(data, num);
	    tt_write(&cr, 1);
	    data += (num + 1);
	    num = 0;
	}
    if (num)
	tt_write(data, num);
}

/* ------------------------------------------------------------------------- */
/*
 * Respond to a notification that a primary selection has been sent
 * EXT: SelectionNotify
 */
/* EXTPROTO */
void
selection_paste(Window win, unsigned int prop, int Delete)
{
    long            nread;
    unsigned long   bytes_after;
    XTextProperty   ct;
#ifdef MULTICHAR_SET
    int             dummy_count;
    char          **cl;
#endif

    D_SELECT((stderr, "selection_paste(%08x, %u, %d)", win, prop, Delete));
    if (prop == None)
	return;
    for (nread = 0, bytes_after = 1; bytes_after > 0; nread += ct.nitems) {
	if ((XGetWindowProperty(Xdisplay, win, prop, (nread / 4), PROP_SIZE,
				Delete, AnyPropertyType, &ct.encoding,
				&ct.format, &ct.nitems, &bytes_after,
				&ct.value) != Success)) {
	    XFree(ct.value);
	    return;
	}
	if (ct.value == NULL)
	    continue;
#ifdef MULTICHAR_SET
	if (XmbTextPropertyToTextList(Xdisplay, &ct, &cl,
				      &dummy_count) == Success && cl) {
	    PasteIt(cl[0], STRLEN(cl[0]));
	    XFreeStringList(cl);
	} else
#endif
	PasteIt(ct.value, ct.nitems);
	XFree(ct.value);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Request the current primary selection
 * EXT: button 2 release
 */
/* EXTPROTO */
void
selection_request(Time tm, int x, int y)
{
    Atom            prop;
#ifdef MULTICHAR_SET
    Atom            ct;
#endif

    D_SELECT((stderr, "selection_request(%ld, %d, %d)", tm, x, y));
    if (x < 0 || x >= TermWin.width || y < 0 || y >= TermWin.height)
	return;			/* outside window */

    if (selection.text != NULL)
	PasteIt(selection.text, selection.len);		/* internal selection */
    else if (XGetSelectionOwner(Xdisplay, XA_PRIMARY) == None)
	selection_paste(Xroot, XA_CUT_BUFFER0, False);
    else {
	prop = XInternAtom(Xdisplay, "VT_SELECTION", False);
#ifdef MULTICHAR_SET
	ct = XInternAtom(Xdisplay, "COMPOUND_TEXT", False);
	XConvertSelection(Xdisplay, XA_PRIMARY, ct, prop, TermWin.vt, tm);
#else
	XConvertSelection(Xdisplay, XA_PRIMARY, XA_STRING, prop, TermWin.vt,
			  tm);
#endif
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Clear all selected text
 * EXT: SelectionClear
 */
/* EXTPROTO */
void
selection_clear(void)
{
    D_SELECT((stderr, "selection_clear()"));

    want_refresh = 1;
    if (selection.text)
	FREE(selection.text);
    selection.text = NULL;
    selection.len = 0;
    CLEAR_SELECTION;
}

/* ------------------------------------------------------------------------- */
/*
 * Copy a selection into the cut buffer
 * EXT: button 1 or 3 release
 */
/* EXTPROTO */
void
selection_make(Time tm)
{
    int             i, col, end_col, row, end_row;
    unsigned char  *new_selection_text;
    char           *str;
    text_t         *t;

    D_SELECT((stderr, "selection_make(): selection.op=%d, selection.clicks=%d", selection.op, selection.clicks));
    switch (selection.op) {
    case SELECTION_CONT:
	break;
    case SELECTION_INIT:
	CLEAR_SELECTION;
    /* FALLTHROUGH */
    case SELECTION_BEGIN:
	selection.op = SELECTION_DONE;
    /* FALLTHROUGH */
    default:
	return;
    }
    selection.op = SELECTION_DONE;

    if (selection.clicks == 4)
	return;			/* nothing selected, go away */

    i = (selection.end.row - selection.beg.row + 1) * (TermWin.ncol + 1) + 1;
    str = MALLOC(i * sizeof(char));

    new_selection_text = (unsigned char *)str;

    col = max(selection.beg.col, 0);
    row = selection.beg.row + TermWin.saveLines;
    end_row = selection.end.row + TermWin.saveLines;
/*
 * A: rows before end row
 */
    for (; row < end_row; row++) {
	t = &(screen.text[row][col]);
	if ((end_col = screen.tlen[row]) == -1)
	    end_col = TermWin.ncol;
	for (; col < end_col; col++)
	    *str++ = *t++;
	col = 0;
	if (screen.tlen[row] != -1)
	    *str++ = '\n';
    }
/*
 * B: end row
 */
    t = &(screen.text[row][col]);
    end_col = screen.tlen[row];
    if (end_col == -1 || selection.end.col <= end_col)
	end_col = selection.end.col;
    MIN_IT(end_col, TermWin.ncol);	/* CHANGE */
    for (; col < end_col; col++)
	*str++ = *t++;
#ifndef NO_OLD_SELECTION
    if (selection_style == OLD_SELECT)
	if (end_col == TermWin.ncol)
	    *str++ = '\n';
#endif
#ifndef NO_NEW_SELECTION
    if (selection_style != OLD_SELECT)
	if (end_col != selection.end.col)
	    *str++ = '\n';
#endif
    *str = '\0';
    if ((i = STRLEN((char *)new_selection_text)) == 0) {
	FREE(new_selection_text);
	return;
    }
    selection.len = i;
    if (selection.text)
	FREE(selection.text);
    selection.text = new_selection_text;

    XSetSelectionOwner(Xdisplay, XA_PRIMARY, TermWin.vt, tm);
    if (XGetSelectionOwner(Xdisplay, XA_PRIMARY) != TermWin.vt)
	print_error("can't get primary selection");
    XChangeProperty(Xdisplay, Xroot, XA_CUT_BUFFER0, XA_STRING, 8,
		    PropModeReplace, selection.text, selection.len);
    D_SELECT((stderr, "selection_make(): selection.len=%d", selection.len));
}

/* ------------------------------------------------------------------------- */
/*
 * Mark or select text based upon number of clicks: 1, 2, or 3
 * EXT: button 1 press
 */
/* EXTPROTO */
void
selection_click(int clicks, int x, int y)
{
    D_SELECT((stderr, "selection_click(%d, %d, %d)", clicks, x, y));

    clicks = ((clicks - 1) % 3) + 1;
    selection.clicks = clicks;	/* save clicks so extend will work */

    selection_start_colrow(Pixel2Col(x), Pixel2Row(y));
    if (clicks == 2 || clicks == 3)
	selection_extend_colrow(selection.mark.col,
				selection.mark.row + TermWin.view_start,
				0,	/* button 3     */
				1,	/* button press */
				0);	/* click change */
}

/* ------------------------------------------------------------------------- */
/*
 * Mark a selection at the specified col/row
 */
/* INTPROTO */
void
selection_start_colrow(int col, int row)
{
    want_refresh = 1;
    selection.mark.col = col;
    selection.mark.row = row - TermWin.view_start;
    MAX_IT(selection.mark.row, -TermWin.nscrolled);
    MIN_IT(selection.mark.row, TermWin.nrow - 1);
    MAX_IT(selection.mark.col, 0);
    MIN_IT(selection.mark.col, TermWin.ncol - 1);

    if (selection.op) {		/* clear the old selection */
	selection.beg.row = selection.end.row = selection.mark.row;
	selection.beg.col = selection.end.col = selection.mark.col;
    }
    selection.op = SELECTION_INIT;
    selection.screen = current_screen;
}

/* ------------------------------------------------------------------------- */
/*
 * Word select: select text for 2 clicks
 * We now only find out the boundary in one direction
 */

/* what do we want: spaces/tabs are delimiters or cutchars or non-cutchars */
#define DELIMIT_TEXT(x) \
    (((x) == ' ' || (x) == '\t') ? 2 : (STRCHR(rs.cutchars, (x)) != NULL))
#ifdef MULTICHAR_SET
# define DELIMIT_REND(x)	(((x) & RS_multiMask) ? 1 : 0)
#else
# define DELIMIT_REND(x)	1
#endif

/* INTPROTO */
void
selection_delimit_word(int dirn, const row_col_t * mark, row_col_t * ret)
{
    int             col, row, dirnadd, tcol, trow, w1, w2;
    row_col_t       bound;
    text_t         *stp;
    rend_t         *srp;

    if (selection.clicks != 2)
	return;			/* Go away: we only handle double clicks */

    if (dirn == UP) {
	bound.row = TermWin.saveLines - TermWin.nscrolled - 1;
	bound.col = 0;
	dirnadd = -1;
    } else {
	bound.row = TermWin.saveLines + TermWin.nrow;
	bound.col = TermWin.ncol - 1;
	dirnadd = 1;
    }
    row = mark->row + TermWin.saveLines;
    col = mark->col;
    MAX_IT(col, 0);
/* find the edge of a word */
    stp = &(screen.text[row][col]);
    w1 = DELIMIT_TEXT(*stp);

    if (selection_style != NEW_SELECT) {
	if (w1 == 1) {
	    stp += dirnadd;
	    if (DELIMIT_TEXT(*stp) == 1)
		goto Old_Word_Selection_You_Die;
	    col += dirnadd;
	}
	w1 = 0;
    }
    srp = (&screen.rend[row][col]);
    w2 = DELIMIT_REND(*srp);

    for (;;) {
	for (; col != bound.col; col += dirnadd) {
	    stp += dirnadd;
	    if (DELIMIT_TEXT(*stp) != w1)
		break;
	    srp += dirnadd;
	    if (DELIMIT_REND(*srp) != w2)
		break;
	}
	if ((col == bound.col) && (row != bound.row)) {
	    if (screen.tlen[(row - (dirn == UP))] == -1) {
		trow = row + dirnadd;
		tcol = (dirn == UP) ? (TermWin.ncol - 1) : 0;
		if (screen.text[trow] == NULL)
		    break;
		stp = &(screen.text[trow][tcol]);
		srp = &(screen.rend[trow][tcol]);
		if (DELIMIT_TEXT(*stp) != w1 || DELIMIT_REND(*srp) != w2)
		    break;
		row = trow;
		col = tcol;
		continue;
	    }
	}
	break;
    }
  Old_Word_Selection_You_Die:
    D_SELECT((stderr, "selection_delimit_word(%s,...) @ (r:%3d, c:%3d) has boundary (r:%3d, c:%3d)", (dirn == UP ? "up	" : "down"), mark->row, mark->col, row - TermWin.saveLines, col));

    if (dirn == DN)
	col++;			/* put us on one past the end */

/* Poke the values back in */
    ret->row = row - TermWin.saveLines;
    ret->col = col;
}

/* ------------------------------------------------------------------------- */
/*
 * Extend the selection to the specified x/y pixel location
 * EXT: button 3 press; button 1 or 3 drag
 * flag == 0 ==> button 1
 * flag == 1 ==> button 3 press
 * flag == 2 ==> button 3 motion
 */
/* EXTPROTO */
void
selection_extend(int x, int y, int flag)
{
    int             col, row;

    col = Pixel2Col(x);
    row = Pixel2Row(y);
    MAX_IT(row, 0);
    MIN_IT(row, TermWin.nrow - 1);
    MAX_IT(col, 0);
    MIN_IT(col, TermWin.ncol);
#ifndef NO_NEW_SELECTION
/*
 * If we're selecting characters (single click) then we must check first
 * if we are at the same place as the original mark.  If we are then
 * select nothing.  Otherwise, if we're to the right of the mark, you have to
 * be _past_ a character for it to be selected.
 */
    if (selection_style != OLD_SELECT) {
	if (((selection.clicks % 3) == 1) && !flag
	    && (col == selection.mark.col
		&& (row == selection.mark.row + TermWin.view_start))) {
	/* select nothing */
	    selection.beg.row = selection.end.row = 0;
	    selection.beg.col = selection.end.col = 0;
	    selection.clicks = 4;
	    D_SELECT((stderr, "selection_extend() selection.clicks = 4"));
	    return;
	}
    }
#endif
    if (selection.clicks == 4)
	selection.clicks = 1;
    selection_extend_colrow(col, row,
			    !!flag,	/* ? button 3      */
			    flag == 1 ? 1 : 0,	/* ? button press  */
			    0);	/* no click change */
}

#ifdef MULTICHAR_SET
/* INTPROTO */
void
selection_adjust_kanji(void)
{
    int             c, r;

    if (selection.beg.col > 0) {
	r = selection.beg.row + TermWin.saveLines;
	c = selection.beg.col;
	if (IS_MULTI2(screen.rend[r][c]) && IS_MULTI1(screen.rend[r][c - 1]))
	    selection.beg.col--;
    }
    if (selection.end.col < TermWin.ncol) {
	r = selection.end.row + TermWin.saveLines;
	c = selection.end.col;
	if (IS_MULTI1(screen.rend[r][c - 1]) && IS_MULTI2(screen.rend[r][c]))
	    selection.end.col++;
    }
}
#endif				/* MULTICHAR_SET */

/* ------------------------------------------------------------------------- */
/*
 * Extend the selection to the specified col/row
 */
/* INTPROTO */
void
selection_extend_colrow(int col, int row, int button3, int buttonpress, int clickchange)
{
    int             end_col;
    row_col_t       pos;
    enum {
	LEFT, RIGHT
    } closeto =     RIGHT;

    D_SELECT((stderr, "selection_extend_colrow(c:%d, r:%d, %d, %d) clicks:%d, op:%d", col, row, button3, buttonpress, selection.clicks, selection.op));
    D_SELECT((stderr, "selection_extend_colrow() ENT  b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", selection.beg.row, selection.beg.col, selection.mark.row, selection.mark.col, selection.end.row, selection.end.col));

    want_refresh = 1;
    switch (selection.op) {
    case SELECTION_INIT:
	CLEAR_SELECTION;
	selection.op = SELECTION_BEGIN;
    /* FALLTHROUGH */
    case SELECTION_BEGIN:
	if (row != selection.mark.row || col != selection.mark.col
	    || (!button3 && buttonpress))
	    selection.op = SELECTION_CONT;
	break;
    case SELECTION_DONE:
	selection.op = SELECTION_CONT;
    /* FALLTHROUGH */
    case SELECTION_CONT:
	break;
    case SELECTION_CLEAR:
	selection_start_colrow(col, row);
    /* FALLTHROUGH */
    default:
	return;
    }
    if (selection.beg.col == selection.end.col
	&& selection.beg.col != selection.mark.col
	&& selection.beg.row == selection.end.row
	&& selection.beg.row != selection.mark.row) {
	selection.beg.col = selection.end.col = selection.mark.col;
	selection.beg.row = selection.end.row = selection.mark.row;
	D_SELECT((stderr, "selection_extend_colrow() ENT2 b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", selection.beg.row, selection.beg.col, selection.mark.row, selection.mark.col, selection.end.row, selection.end.col));
    }

    pos.col = col;
    pos.row = row;

    pos.row -= TermWin.view_start;	/* adjust for scroll */

#ifndef NO_OLD_SELECTION
/*
 * This mimics some of the selection behaviour of version 2.20 and before.
 * There are no ``selection modes'', button3 is always character extension.
 * Note: button3 drag is always available, c.f. v2.20
 * Selection always terminates (left or right as appropriate) at the mark.
 */
    if (selection_style == OLD_SELECT) {
	static int      hate_those_clicks = 0;	/* a.k.a. keep mark position */

	if (selection.clicks == 1 || button3) {
	    if (hate_those_clicks) {
		hate_those_clicks = 0;
		if (selection.clicks == 1) {
		    selection.beg.row = selection.mark.row;
		    selection.beg.col = selection.mark.col;
		} else {
		    selection.mark.row = selection.beg.row;
		    selection.mark.col = selection.beg.col;
		}
	    }
	    if (ROWCOL_IS_BEFORE(pos, selection.mark)) {
		selection.end.row = selection.mark.row;
		selection.end.col = selection.mark.col + 1;
		selection.beg.row = pos.row;
		selection.beg.col = pos.col;
	    } else {
		selection.beg.row = selection.mark.row;
		selection.beg.col = selection.mark.col;
		selection.end.row = pos.row;
		selection.end.col = pos.col + 1;
	    }
# ifdef MULTICHAR_SET
	    selection_adjust_kanji();
# endif				/* MULTICHAR_SET */
	} else if (selection.clicks == 2) {
	    selection_delimit_word(UP, &(selection.mark), &(selection.beg));
	    selection_delimit_word(DN, &(selection.mark), &(selection.end));
	    hate_those_clicks = 1;
	} else if (selection.clicks == 3) {
	    selection.beg.row = selection.end.row = selection.mark.row;
	    selection.beg.col = 0;
	    selection.end.col = TermWin.ncol;
	    hate_those_clicks = 1;
	}
	D_SELECT((stderr, "selection_extend_colrow() EXIT b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", selection.beg.row, selection.beg.col, selection.mark.row, selection.mark.col, selection.end.row, selection.end.col));
	return;
    }
#endif				/* ! NO_OLD_SELECTION */
#ifndef NO_NEW_SELECTION
/* selection_style must not be OLD_SELECT to get here */
/*
 * This is mainly xterm style selection with a couple of differences, mainly
 * in the way button3 drag extension works.
 * We're either doing: button1 drag; button3 press; or button3 drag
 *  a) button1 drag : select around a midpoint/word/line - that point/word/line
 *     is always at the left/right edge of the selection.
 *  b) button3 press: extend/contract character/word/line at whichever edge of
 *     the selection we are closest to.
 *  c) button3 drag : extend/contract character/word/line - we select around
 *     a point/word/line which is either the start or end of the selection
 *     and it was decided by whichever point/word/line was `fixed' at the
 *     time of the most recent button3 press
 */
    if (button3 && buttonpress) {	/* button3 press */
    /*
     * first determine which edge of the selection we are closest to
     */
	if (ROWCOL_IS_BEFORE(pos, selection.beg)
	    || (!ROWCOL_IS_AFTER(pos, selection.end)
		&& (((pos.col - selection.beg.col)
		     + ((pos.row - selection.beg.row) * TermWin.ncol))
		    < ((selection.end.col - pos.col)
		       + ((selection.end.row - pos.row) * TermWin.ncol)))))
	    closeto = LEFT;
	if (closeto == LEFT) {
	    selection.beg.row = pos.row;
	    selection.beg.col = pos.col;
	    selection.mark.row = selection.end.row;
	    selection.mark.col = selection.end.col - (selection.clicks == 2);
	} else {
	    selection.end.row = pos.row;
	    selection.end.col = pos.col;
	    selection.mark.row = selection.beg.row;
	    selection.mark.col = selection.beg.col;
	}
    } else {			/* button1 drag or button3 drag */
	if (ROWCOL_IS_AFTER(selection.mark, pos)) {
	    if ((selection.mark.row == selection.end.row)
		&& (selection.mark.col == selection.end.col)
		&& clickchange && selection.clicks == 2)
		selection.mark.col--;
	    selection.beg.row = pos.row;
	    selection.beg.col = pos.col;
	    selection.end.row = selection.mark.row;
	    selection.end.col = selection.mark.col + (selection.clicks == 2);
	} else {
	    selection.beg.row = selection.mark.row;
	    selection.beg.col = selection.mark.col;
	    selection.end.row = pos.row;
	    selection.end.col = pos.col;
	}
    }

    if (selection.clicks == 1) {
	end_col = screen.tlen[selection.beg.row + TermWin.saveLines];
	if (end_col != -1 && selection.beg.col > end_col) {
#if 1
	    selection.beg.col = TermWin.ncol;
#else
	    if (selection.beg.row != selection.end.row)
		selection.beg.col = TermWin.ncol;
	    else
		selection.beg.col = selection.mark.col;
#endif
	}
	end_col = screen.tlen[selection.end.row + TermWin.saveLines];
	if (end_col != -1 && selection.end.col > end_col)
	    selection.end.col = TermWin.ncol;

# ifdef MULTICHAR_SET
	selection_adjust_kanji();
# endif				/* MULTICHAR_SET */
    } else if (selection.clicks == 2) {
	if (ROWCOL_IS_AFTER(selection.end, selection.beg))
	    selection.end.col--;
	selection_delimit_word(UP, &(selection.beg), &(selection.beg));
	selection_delimit_word(DN, &(selection.end), &(selection.end));
    } else if (selection.clicks == 3) {
	if (ROWCOL_IS_AFTER(selection.mark, selection.beg))
	    selection.mark.col++;
	selection.beg.col = 0;
	selection.end.col = TermWin.ncol;
    }
    if (button3 && buttonpress) {	/* mark may need to be changed */
	if (closeto == LEFT) {
	    selection.mark.row = selection.end.row;
	    selection.mark.col = selection.end.col - (selection.clicks == 2);
	} else {
	    selection.mark.row = selection.beg.row;
	    selection.mark.col = selection.beg.col;
	}
    }
    D_SELECT((stderr, "selection_extend_colrow() EXIT b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", selection.beg.row, selection.beg.col, selection.mark.row, selection.mark.col, selection.end.row, selection.end.col));
#endif				/* ! NO_NEW_SELECTION */
}

/* ------------------------------------------------------------------------- */
/*
 * Double click on button 3 when already selected
 * EXT: button 3 double click
 */
/* EXTPROTO */
void
selection_rotate(int x, int y)
{
    selection.clicks = selection.clicks % 3 + 1;
    selection_extend_colrow(Pixel2Col(x), Pixel2Row(y), 1, 0, 1);
}

/* ------------------------------------------------------------------------- */
/*
 * On some systems, the Atom typedef is 64 bits wide.  We need to have a type
 * that is exactly 32 bits wide, because a format of 64 is not allowed by
 * the X11 protocol.
 */
typedef CARD32  Atom32;

/* ------------------------------------------------------------------------- */
/*
 * Respond to a request for our current selection
 * EXT: SelectionRequest
 */
/* EXTPROTO */
void
selection_send(const XSelectionRequestEvent * rq)
{
    XEvent          ev;
    Atom32          target_list[4];
    Atom            target;
    static Atom     xa_targets = None;
    static Atom     xa_compound_text = None;
    static Atom     xa_text = None;
    XTextProperty   ct;
    XICCEncodingStyle style;
    char           *cl[4];

    if (xa_text == None)
	xa_text = XInternAtom(Xdisplay, "TEXT", False);
    if (xa_compound_text == None)
	xa_compound_text = XInternAtom(Xdisplay, "COMPOUND_TEXT", False);
    if (xa_targets == None)
	xa_targets = XInternAtom(Xdisplay, "TARGETS", False);

    ev.xselection.type = SelectionNotify;
    ev.xselection.property = None;
    ev.xselection.display = rq->display;
    ev.xselection.requestor = rq->requestor;
    ev.xselection.selection = rq->selection;
    ev.xselection.target = rq->target;
    ev.xselection.time = rq->time;

    if (rq->target == xa_targets) {
	target_list[0] = (Atom32) xa_targets;
	target_list[1] = (Atom32) XA_STRING;
	target_list[2] = (Atom32) xa_text;
	target_list[3] = (Atom32) xa_compound_text;
	XChangeProperty(Xdisplay, rq->requestor, rq->property, rq->target,
			(8 * sizeof(target_list[0])), PropModeReplace,
			(unsigned char *)target_list,
			(sizeof(target_list) / sizeof(target_list[0])));
	ev.xselection.property = rq->property;
    } else if (rq->target == XA_STRING
	       || rq->target == xa_compound_text
	       || rq->target == xa_text) {
	if (rq->target == XA_STRING) {
	    style = XStringStyle;
	    target = XA_STRING;
	} else {
	    target = xa_compound_text;
	    style = (rq->target == xa_compound_text) ? XCompoundTextStyle
						     : XStdICCTextStyle;
	}
	cl[0] = selection.text;
	XmbTextListToTextProperty(Xdisplay, cl, 1, style, &ct);
	XChangeProperty(Xdisplay, rq->requestor, rq->property,
			target, 8, PropModeReplace,
			ct.value, (int)ct.nitems);
	ev.xselection.property = rq->property;
    }
    XSendEvent(Xdisplay, rq->requestor, False, 0, &ev);
}

/* ------------------------------------------------------------------------- *
 *                              MOUSE ROUTINES                               *
 * ------------------------------------------------------------------------- */

/*
 * return col/row values corresponding to x/y pixel values
 */
/* EXTPROTO */
void
pixel_position(int *x, int *y)
{
    *x = Pixel2Col(*x);
/* MAX_IT(*x, 0); MIN_IT(*x, TermWin.ncol - 1); */
    *y = Pixel2Row(*y);
/* MAX_IT(*y, 0); MIN_IT(*y, TermWin.nrow - 1); */
}

/* ------------------------------------------------------------------------- */
/* ARGSUSED */
/* INTPROTO */
void
mouse_tracking(int report, int x, int y, int firstrow, int lastrow)
{
/* TODO */
}

/* ------------------------------------------------------------------------- *
 *                              DEBUG ROUTINES                               *
 * ------------------------------------------------------------------------- */
/* ARGSUSED */
/* INTPROTO */
void
debug_PasteIt(unsigned char *data, int nitems)
{
/* TODO */
}

/* ------------------------------------------------------------------------- */
#if 0
/* INTPROTO */
void
debug_colors(void)
{
    int             color;
    const char     *name[] =
    { "fg", "bg",
      "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white" };

    fprintf(stderr, "Color ( ");
    if (rstyle & RS_RVid)
	fprintf(stderr, "rvid ");
    if (rstyle & RS_Bold)
	fprintf(stderr, "bold ");
    if (rstyle & RS_Blink)
	fprintf(stderr, "blink ");
    if (rstyle & RS_Uline)
	fprintf(stderr, "uline ");
    fprintf(stderr, "): ");

    color = GET_FGCOLOR(rstyle);
#ifndef NO_BRIGHTCOLOR
    if (color >= minBrightCOLOR && color <= maxBrightCOLOR) {
	color -= (minBrightCOLOR - minCOLOR);
	fprintf(stderr, "bright ");
    }
#endif
    fprintf(stderr, "%s on ", name[color]);

    color = GET_BGCOLOR(rstyle);
#ifndef NO_BRIGHTCOLOR
    if (color >= minBrightCOLOR && color <= maxBrightCOLOR) {
	color -= (minBrightCOLOR - minCOLOR);
	fprintf(stderr, "bright ");
    }
#endif
    fprintf(stderr, "%s\n", name[color]);
}
#endif

#ifdef USE_XIM
/* EXTPROTO */
void
setPosition(XPoint * pos)
{
    XWindowAttributes xwa;

    XGetWindowAttributes(Xdisplay, TermWin.vt, &xwa);
    pos->x = Col2Pixel(screen.cur.col) + xwa.x;
    pos->y = Height2Pixel((screen.cur.row + 1)) + xwa.y;
}
#endif
