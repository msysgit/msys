/*--------------------------------*-C-*--------------------------------------*
 * File:	screen.c
 *---------------------------------------------------------------------------*
 * $Id: screen.c,v 1.1 2003/03/05 17:33:37 earnie Exp $
 *
 * Copyright (c) 1997-2001 Geoff Wing <gcw@pobox.com>
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

#include <X11/Xmd.h>		/* get the typedef for CARD32 */

/* ------------------------------------------------------------------------- */
#ifdef MULTICHAR_SET

#define RESET_CHSTAT(H)				\
    if ((H)->chstat == WBYTE)			\
	(H)->chstat = SBYTE, (H)->lost_multi = 1

const KNOWN_ENCODINGS known_encodings[] = {
    { SJIS, rxvt_sjis2jis, "sjis" },
    { EUCJ, rxvt_euc2jis, "eucj" },
    { GB, rxvt_euc2jis, "gb" },
    { BIG5, rxvt_decodedummy, "big5" },
    { EUCKR, rxvt_euc2jis, "kr" },
    { NOENC, rxvt_decodedummy, "noenc" },
    { 0, NULL, NULL }
};
#else
# define RESET_CHSTAT(H)
#endif

/* ------------------------------------------------------------------------- */
#define PROP_SIZE		16384
#define TABSIZE			8	/* default tab size */

/* ------------------------------------------------------------------------- *
 *             GENERAL SCREEN AND SELECTION UPDATE ROUTINES                  *
 * ------------------------------------------------------------------------- */
#define ZERO_SCROLLBACK(R)						\
    if (((R)->Options & Opt_scrollTtyOutput) == Opt_scrollTtyOutput)	\
	(R)->TermWin.view_start = 0
#define CLEAR_SELECTION(R)						\
    (R)->selection.beg.row = (R)->selection.beg.col			\
	= (R)->selection.end.row = (R)->selection.end.col = 0
#define CLEAR_ALL_SELECTION(R)						\
    (R)->selection.beg.row = (R)->selection.beg.col			\
	= (R)->selection.mark.row = (R)->selection.mark.col		\
	= (R)->selection.end.row = (R)->selection.end.col = 0

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
#define drawBuffer	(r->TermWin.vt)

#define CLEAR_ROWS(row, num)						\
    if (r->TermWin.mapped)						\
	XClearArea(r->Xdisplay, drawBuffer, r->TermWin.int_bwidth,	\
		   Row2Pixel(row), (unsigned int)r->TermWin.width,	\
		   (unsigned int)Height2Pixel(num), False)

#define CLEAR_CHARS(x, y, num)						\
    if (r->TermWin.mapped)						\
	XClearArea(r->Xdisplay, drawBuffer, x, y,			\
		   (unsigned int)Width2Pixel(num),			\
		   (unsigned int)Height2Pixel(1), False)

#define ERASE_ROWS(row, num)						\
    XFillRectangle(r->Xdisplay, drawBuffer, r->TermWin.gc,		\
		   r->TermWin.int_bwidth, Row2Pixel(row),		\
		   (unsigned int)r->TermWin.width,			\
		   (unsigned int)Height2Pixel(num))

/* ------------------------------------------------------------------------- *
 *                        SCREEN `COMMON' ROUTINES                           *
 * ------------------------------------------------------------------------- */
/* Fill part/all of a line with blanks. */
/* INTPROTO */
void
rxvt_blank_line(text_t *et, rend_t *er, unsigned int width, rend_t efs)
{
    MEMSET(et, ' ', (size_t)width);
    efs &= ~RS_baseattrMask;
    for (; width--;)
	*er++ = efs;
}

/* ------------------------------------------------------------------------- */
/* Fill a full line with blanks - make sure it is allocated first */
/* INTPROTO */
void
rxvt_blank_screen_mem(rxvt_t *r, text_t **tp, rend_t **rp, unsigned int row, rend_t efs)
{
    int             width = r->TermWin.ncol;
    rend_t         *er;

#ifdef DEBUG_STRICT
    assert((tp[row] && rp[row]) || (tp[row] == NULL && rp[row] == NULL));
#endif
    if (tp[row] == NULL) {
	tp[row] = rxvt_malloc(sizeof(text_t) * width);
	rp[row] = rxvt_malloc(sizeof(rend_t) * width);
    }
    MEMSET(tp[row], ' ', width);
    efs &= ~RS_baseattrMask;
    for (er = rp[row]; width--;)
	*er++ = efs;
}

/* ------------------------------------------------------------------------- *
 *                          SCREEN INITIALISATION                            *
 * ------------------------------------------------------------------------- */
/* EXTPROTO */
void
rxvt_scr_reset(rxvt_t *r)
{
    unsigned int    ncol, nrow, prev_ncol, prev_nrow,
		    total_rows, prev_total_rows;
    unsigned int    p, q;
    int             k;
    rend_t          setrstyle;

    D_SCREEN((stderr, "rxvt_scr_reset()"));

    r->TermWin.view_start = 0;
    RESET_CHSTAT(r->h);
    r->h->num_scr = 0;

    prev_ncol = r->h->prev_ncol;
    prev_nrow = r->h->prev_nrow;
    if (r->TermWin.ncol == 0)
	r->TermWin.ncol = 80;
    if (r->TermWin.nrow == 0)
	r->TermWin.nrow = 24;
    ncol = r->TermWin.ncol;
    nrow = r->TermWin.nrow;
    if (ncol == prev_ncol && nrow == prev_nrow)
	return;

    r->h->want_refresh = 1;

    total_rows = nrow + r->TermWin.saveLines;
    prev_total_rows = prev_nrow + r->TermWin.saveLines;

    r->screen.tscroll = 0;
    r->screen.bscroll = nrow - 1;

    if (prev_nrow == 0) {
/*
 * A: first time called so just malloc everything : don't rely on realloc
 *    Note: this is still needed so that all the scrollback lines are NULL
 */
	r->screen.text = rxvt_calloc(total_rows, sizeof(text_t *));
	r->buf_text = rxvt_calloc(total_rows, sizeof(text_t *));
	r->drawn_text = rxvt_calloc(nrow, sizeof(text_t *));
	r->swap.text = rxvt_calloc(nrow, sizeof(text_t *));

	r->screen.tlen = rxvt_calloc(total_rows, sizeof(int16_t));
	r->swap.tlen = rxvt_calloc(nrow, sizeof(int16_t));

	r->screen.rend = rxvt_calloc(total_rows, sizeof(rend_t *));
	r->buf_rend = rxvt_calloc(total_rows, sizeof(rend_t *));
	r->drawn_rend = rxvt_calloc(nrow, sizeof(rend_t *));
	r->swap.rend = rxvt_calloc(nrow, sizeof(rend_t *));

	for (p = 0; p < nrow; p++) {
	    q = p + r->TermWin.saveLines;
	    rxvt_blank_screen_mem(r, r->screen.text, r->screen.rend,
				  q, DEFAULT_RSTYLE);
	    rxvt_blank_screen_mem(r, r->swap.text, r->swap.rend,
				  p, DEFAULT_RSTYLE);
	    r->screen.tlen[q] = r->swap.tlen[p] = 0;
	    rxvt_blank_screen_mem(r, r->drawn_text, r->drawn_rend,
				  p, DEFAULT_RSTYLE);
	}
	MEMSET(r->h->charsets, 'B', sizeof(r->h->charsets));
	r->TermWin.nscrolled = 0;	/* no saved lines */
	r->h->rstyle = DEFAULT_RSTYLE;
	r->screen.flags = Screen_DefaultFlags;
	r->screen.cur.row = r->screen.cur.col = 0;
	r->screen.charset = 0;
	r->h->current_screen = PRIMARY;
	rxvt_scr_cursor(r, SAVE);
#if NSCREENS
	r->swap.flags = Screen_DefaultFlags;
	r->swap.cur.row = r->swap.cur.col = 0;
	r->swap.charset = 0;
	r->h->current_screen = SECONDARY;
	rxvt_scr_cursor(r, SAVE);
	r->h->current_screen = PRIMARY;
#endif
	r->selection.text = NULL;
	r->selection.len = 0;
	r->selection.op = SELECTION_CLEAR;
	r->selection.screen = PRIMARY;
	r->selection.clicks = 0;
	CLEAR_ALL_SELECTION(r);
	r->h->rvideo = 0;
#ifdef MULTICHAR_SET
	r->h->multi_byte = 0;
	r->h->lost_multi = 0;
	r->h->chstat = SBYTE;
#endif

    } else {
/*
 * B1: add or delete rows as appropriate
 */
	setrstyle = DEFAULT_RSTYLE;

	if (nrow < prev_nrow) {
	    /* delete rows */
	    k = min(r->TermWin.nscrolled, prev_nrow - nrow);
	    rxvt_scroll_text(r, 0, (int)prev_nrow - 1, k, 1);
	    for (p = nrow; p < prev_nrow; p++) {
		q = p + r->TermWin.saveLines;
		if (r->screen.text[q]) {
#ifdef DEBUG_STRICT
		    assert(r->screen.rend[q]);
#endif
		    free(r->screen.text[q]);
		    free(r->screen.rend[q]);
		}
		if (r->swap.text[p]) {
#ifdef DEBUG_STRICT
		    assert(r->swap.rend[p]);
#endif
		    free(r->swap.text[p]);
		    free(r->swap.rend[p]);
		}
#ifdef DEBUG_STRICT
		assert(r->drawn_text[p] && r->drawn_rend[p]);
#endif
		free(r->drawn_text[p]);
		free(r->drawn_rend[p]);
	    }
	    /* we have fewer rows so fix up cursor position */
	    MIN_IT(r->screen.cur.row, (int32_t)nrow - 1);
	    MIN_IT(r->swap.cur.row, (int32_t)nrow - 1);

	    rxvt_scr_reset_realloc(r);	/* realloc _last_ */

	} else if (nrow > prev_nrow) {
	    /* add rows */
	    rxvt_scr_reset_realloc(r);	/* realloc _first_ */

	    k = min(r->TermWin.nscrolled, nrow - prev_nrow);
	    for (p = prev_total_rows; p < total_rows; p++) {
		r->screen.tlen[p] = 0;
		r->screen.text[p] = NULL;
		r->screen.rend[p] = NULL;
	    }
	    for (p = prev_total_rows; p < total_rows - k; p++)
		rxvt_blank_screen_mem(r, r->screen.text, r->screen.rend,
				      p, setrstyle);
	    for (p = prev_nrow; p < nrow; p++) {
		r->swap.tlen[p] = 0;
		r->swap.text[p] = NULL;
		r->swap.rend[p] = NULL;
		r->drawn_text[p] = NULL;
		r->drawn_rend[p] = NULL;
		rxvt_blank_screen_mem(r, r->swap.text, r->swap.rend,
				      p, setrstyle);
		rxvt_blank_screen_mem(r, r->drawn_text, r->drawn_rend,
				      p, setrstyle);
	    }
	    if (k > 0) {
		rxvt_scroll_text(r, 0, (int)nrow - 1, -k, 1);
		r->screen.cur.row += k;
		r->screen.s_cur.row += k;
		r->TermWin.nscrolled -= k;
	    }
#ifdef DEBUG_STRICT
	    assert(r->screen.cur.row < r->TermWin.nrow);
	    assert(r->swap.cur.row < r->TermWin.nrow);
#else				/* drive with your eyes closed */
	    MIN_IT(r->screen.cur.row, nrow - 1);
	    MIN_IT(r->swap.cur.row, nrow - 1);
#endif
	}
/* B2: resize columns */
	if (ncol != prev_ncol) {
	    for (p = 0; p < total_rows; p++) {
		if (r->screen.text[p]) {
		    r->screen.text[p] = rxvt_realloc(r->screen.text[p],
						     ncol * sizeof(text_t));
		    r->screen.rend[p] = rxvt_realloc(r->screen.rend[p],
						     ncol * sizeof(rend_t));
		    MIN_IT(r->screen.tlen[p], (int16_t)ncol);
		    if (ncol > prev_ncol)
			rxvt_blank_line(&(r->screen.text[p][prev_ncol]),
					&(r->screen.rend[p][prev_ncol]),
					ncol - prev_ncol,
					setrstyle);
		}
	    }
	    for (p = 0; p < nrow; p++) {
		r->drawn_text[p] = rxvt_realloc(r->drawn_text[p],
						ncol * sizeof(text_t));
		r->drawn_rend[p] = rxvt_realloc(r->drawn_rend[p],
						ncol * sizeof(rend_t));
		if (r->swap.text[p]) {
		    r->swap.text[p] = rxvt_realloc(r->swap.text[p],
						   ncol * sizeof(text_t));
		    r->swap.rend[p] = rxvt_realloc(r->swap.rend[p],
						   ncol * sizeof(rend_t));
		    MIN_IT(r->swap.tlen[p], (int16_t)ncol);
		    if (ncol > prev_ncol)
			rxvt_blank_line(&(r->swap.text[p][prev_ncol]),
					&(r->swap.rend[p][prev_ncol]),
					ncol - prev_ncol, setrstyle);
		}
		if (ncol > prev_ncol)
		    rxvt_blank_line(&(r->drawn_text[p][prev_ncol]),
				    &(r->drawn_rend[p][prev_ncol]),
				    ncol - prev_ncol, setrstyle);
	    }
	    MIN_IT(r->screen.cur.col, (int16_t)ncol - 1);
	    MIN_IT(r->swap.cur.col, (int16_t)ncol - 1);
	}
	if (r->tabs)
	    free(r->tabs);
    }

    r->tabs = rxvt_malloc(ncol * sizeof(char));

    for (p = 0; p < ncol; p++)
	r->tabs[p] = (p % TABSIZE == 0) ? 1 : 0;

    r->h->prev_nrow = nrow;
    r->h->prev_ncol = ncol;

    rxvt_tt_winsize(r->cmd_fd, r->TermWin.ncol, r->TermWin.nrow);
}

/* INTPROTO */
void
rxvt_scr_reset_realloc(rxvt_t *r)
{
    u_int16_t       total_rows, nrow;

    nrow = r->TermWin.nrow;
    total_rows = nrow + r->TermWin.saveLines;
/* *INDENT-OFF* */
    r->screen.text = rxvt_realloc(r->screen.text, total_rows * sizeof(text_t *));
    r->buf_text    = rxvt_realloc(r->buf_text   , total_rows * sizeof(text_t *));
    r->drawn_text  = rxvt_realloc(r->drawn_text , nrow       * sizeof(text_t *));
    r->swap.text   = rxvt_realloc(r->swap.text  , nrow       * sizeof(text_t *));

    r->screen.tlen = rxvt_realloc(r->screen.tlen, total_rows * sizeof(int16_t));
    r->swap.tlen   = rxvt_realloc(r->swap.tlen  , total_rows * sizeof(int16_t));

    r->screen.rend = rxvt_realloc(r->screen.rend, total_rows * sizeof(rend_t *));
    r->buf_rend    = rxvt_realloc(r->buf_rend   , total_rows * sizeof(rend_t *));
    r->drawn_rend  = rxvt_realloc(r->drawn_rend , nrow       * sizeof(rend_t *));
    r->swap.rend   = rxvt_realloc(r->swap.rend  , nrow       * sizeof(rend_t *));
/* *INDENT-ON* */
}

/* ------------------------------------------------------------------------- */
/*
 * Free everything.  That way malloc debugging can find leakage.
 */
/* EXTPROTO */
void
rxvt_scr_release(rxvt_t *r)
{
    u_int16_t       total_rows;
    int             i;

    total_rows = r->TermWin.nrow + r->TermWin.saveLines;
    for (i = 0; i < total_rows; i++) {
	if (r->screen.text[i]) {	/* then so is r->screen.rend[i] */
	    free(r->screen.text[i]);
#ifdef DEBUG_STRICT
	    assert(r->screen.rend[i]);
#endif
	    free(r->screen.rend[i]);
	}
    }
    for (i = 0; i < r->TermWin.nrow; i++) {
	free(r->drawn_text[i]);
	free(r->drawn_rend[i]);
	free(r->swap.text[i]);
	free(r->swap.rend[i]);
    }
    free(r->screen.text);
    free(r->screen.tlen);
    free(r->screen.rend);
    free(r->drawn_text);
    free(r->drawn_rend);
    free(r->swap.text);
    free(r->swap.tlen);
    free(r->swap.rend);
    free(r->buf_text);
    free(r->buf_rend);
    free(r->tabs);

/* NULL these so if anything tries to use them, we'll know about it */
    r->screen.text = r->drawn_text = r->swap.text = NULL;
    r->screen.rend = r->drawn_rend = r->swap.rend = NULL;
    r->screen.tlen = r->swap.tlen = NULL;
    r->buf_text = NULL;
    r->buf_rend = NULL;
    r->tabs = NULL;
}

/* ------------------------------------------------------------------------- */
/*
 * Hard reset
 */
/* EXTPROTO */
void
rxvt_scr_poweron(rxvt_t *r)
{
    D_SCREEN((stderr, "rxvt_scr_poweron()"));

    rxvt_scr_release(r);
    r->h->prev_nrow = r->h->prev_ncol = 0;
    rxvt_scr_reset(r);

    rxvt_scr_clear(r);
    rxvt_scr_refresh(r, SLOW_REFRESH);
    rxvt_Gr_reset(r);
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
rxvt_scr_cursor(rxvt_t *r, int mode)
{
    screen_t       *s;

    D_SCREEN((stderr, "rxvt_scr_cursor(%c)", mode));

#if NSCREENS && !defined(NO_SECONDARY_SCREEN_CURSOR)
    if (r->h->current_screen == SECONDARY)
	s = &(r->swap);
    else
#endif
	s = &(r->screen);
    switch (mode) {
    case SAVE:
	s->s_cur.row = s->cur.row;
	s->s_cur.col = s->cur.col;
	s->s_rstyle = r->h->rstyle;
	s->s_charset = s->charset;
	s->s_charset_char = r->h->charsets[s->charset];
	break;
    case RESTORE:
	r->h->want_refresh = 1;
	s->cur.row = s->s_cur.row;
	s->cur.col = s->s_cur.col;
	s->flags &= ~Screen_WrapNext;
	r->h->rstyle = s->s_rstyle;
	s->charset = s->s_charset;
	r->h->charsets[s->charset] = s->s_charset_char;
	rxvt_set_font_style(r);
	break;
    }
/* boundary check in case screen size changed between SAVE and RESTORE */
    MIN_IT(s->cur.row, r->TermWin.nrow - 1);
    MIN_IT(s->cur.col, r->TermWin.ncol - 1);
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
rxvt_scr_change_screen(rxvt_t *r, int scrn)
{
    int             i;
#if NSCREENS
    int             offset;
#endif

    r->h->want_refresh = 1;

    D_SCREEN((stderr, "rxvt_scr_change_screen(%d)", scrn));

    r->TermWin.view_start = 0;
    RESET_CHSTAT(r->h);

    if (r->h->current_screen == scrn)
	return r->h->current_screen;

    rxvt_selection_check(r, 2);	/* check for boundary cross */

    SWAP_IT(r->h->current_screen, scrn, int);
#if NSCREENS
    r->h->num_scr = 0;
    offset = r->TermWin.saveLines;
    for (i = r->h->prev_nrow; i--;) {
	SWAP_IT(r->screen.text[i + offset], r->swap.text[i], text_t *);
	SWAP_IT(r->screen.tlen[i + offset], r->swap.tlen[i], int16_t);
	SWAP_IT(r->screen.rend[i + offset], r->swap.rend[i], rend_t *);
    }
    SWAP_IT(r->screen.cur.row, r->swap.cur.row, int16_t);
    SWAP_IT(r->screen.cur.col, r->swap.cur.col, int16_t);
# ifdef DEBUG_STRICT
    assert((r->screen.cur.row >= 0) && (r->screen.cur.row < r->h->prev_nrow));
    assert((r->screen.cur.col >= 0) && (r->screen.cur.col < r->h->prev_ncol));
# else				/* drive with your eyes closed */
    MAX_IT(r->screen.cur.row, 0);
    MIN_IT(r->screen.cur.row, r->h->prev_nrow - 1);
    MAX_IT(r->screen.cur.col, 0);
    MIN_IT(r->screen.cur.col, r->h->prev_ncol - 1);
# endif
    SWAP_IT(r->screen.charset, r->swap.charset, int16_t);
    SWAP_IT(r->screen.flags, r->swap.flags, int);
    r->screen.flags |= Screen_VisibleCursor;
    r->swap.flags |= Screen_VisibleCursor;

    if (rxvt_Gr_Displayed(r)) {
	rxvt_Gr_scroll(r, 0);
	rxvt_Gr_ChangeScreen(r);
    }
#else
# ifdef SCROLL_ON_NO_SECONDARY
    if (rxvt_Gr_Displayed(r))
	rxvt_Gr_ClearScreen(r);
    if (r->h->current_screen == PRIMARY && !rxvt_Gr_Displayed(r))
	rxvt_scroll_text(r, 0, (r->h->prev_nrow - 1), r->h->prev_nrow, 0);
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
rxvt_scr_color(rxvt_t *r, unsigned int color, int fgbg)
{
    color &= RS_fgMask;
    if (fgbg == Color_fg)
	r->h->rstyle = SET_FGCOLOR(r->h->rstyle, color);
    else 
	r->h->rstyle = SET_BGCOLOR(r->h->rstyle, color);
}

/* ------------------------------------------------------------------------- */
/*
 * Change the rendition style for following text
 */
/* EXTPROTO */
void
rxvt_scr_rendition(rxvt_t *r, int set, int style)
{
    if (set)
	r->h->rstyle |= style;
    else if (style == ~RS_None)
	r->h->rstyle = DEFAULT_RSTYLE | (r->h->rstyle & RS_fontMask);
    else
	r->h->rstyle &= ~style;
}

/* ------------------------------------------------------------------------- */
/*
 * Scroll text between <row1> and <row2> inclusive, by <count> lines
 * count positive ==> scroll up
 * count negative ==> scroll down
 * spec == 0 for normal routines
 */
/* EXTPROTO */
int
rxvt_scroll_text(rxvt_t *r, int row1, int row2, int count, int spec)
{
    int             i, j;
    long            nscrolled;

    if (count == 0 || (row1 > row2))
	return 0;

    r->h->want_refresh = 1;
    D_SCREEN((stderr, "rxvt_scroll_text(%d,%d,%d,%d): %s", row1, row2, count, spec, (r->h->current_screen == PRIMARY) ? "Primary" : "Secondary"));

    if ((count > 0) && (row1 == 0) && (r->h->current_screen == PRIMARY)) {
	nscrolled = (long)r->TermWin.nscrolled + (long)count;;
	if (nscrolled > (long)r->TermWin.saveLines)
	    r->TermWin.nscrolled = r->TermWin.saveLines;
	else
	    r->TermWin.nscrolled = (u_int16_t)nscrolled;
	if ((r->Options & Opt_scrollWithBuffer)
	    && r->TermWin.view_start != 0
	    && r->TermWin.view_start != r->TermWin.saveLines)
	    rxvt_scr_page(r, UP, count);
    } else if (!spec)
	row1 += r->TermWin.saveLines;
    row2 += r->TermWin.saveLines;

    if (r->selection.op && r->h->current_screen == r->selection.screen) {
	i = r->selection.beg.row + r->TermWin.saveLines;
	j = r->selection.end.row + r->TermWin.saveLines;
	if ((i < row1 && j > row1)
	    || (i < row2 && j > row2)
	    || (i - count < row1 && i >= row1)
	    || (i - count > row2 && i <= row2)
	    || (j - count < row1 && j >= row1)
	    || (j - count > row2 && j <= row2)) {
	    CLEAR_ALL_SELECTION(r);
	    r->selection.op = SELECTION_CLEAR;	/* XXX: too aggressive? */
	} else if (j >= row1 && j <= row2) {
	    /* move selected region too */
	    r->selection.beg.row -= count;
	    r->selection.end.row -= count;
	    r->selection.mark.row -= count;
	}
    }
    rxvt_selection_check(r, 0);	/* _after_ r->TermWin.nscrolled update */

    r->h->num_scr += count;
    j = count;
    if (count < 0)
	count = -count;
    i = row2 - row1 + 1;
    MIN_IT(count, i);

    if (j > 0) {
/* A: scroll up */

/* A1: Copy lines that will get clobbered by the rotation */
	for (i = 0, j = row1; i < count; i++, j++) {
	    r->buf_text[i] = r->screen.text[j];
	    r->buf_rend[i] = r->screen.rend[j];
	}
/* A2: Rotate lines */
	for (j = row1, i = j + count; i <= row2; i++, j++) {
	    r->screen.tlen[j] = r->screen.tlen[i];
	    r->screen.text[j] = r->screen.text[i];
	    r->screen.rend[j] = r->screen.rend[i];
	}
	j = row2 - count + 1, i = count;
    } else /* if (j < 0) */ {
/* B: scroll down */

/* B1: Copy lines that will get clobbered by the rotation */
	for (i = 0, j = row2; i < count; i++, j--) {
	    r->buf_text[i] = r->screen.text[j];
	    r->buf_rend[i] = r->screen.rend[j];
	}
/* B2: Rotate lines */
	for (j = row2, i = j - count; i >= row1; i--, j--) {
	    r->screen.tlen[j] = r->screen.tlen[i];
	    r->screen.text[j] = r->screen.text[i];
	    r->screen.rend[j] = r->screen.rend[i];
	}
	j = row1, i = count;
	count = -count;
    }

/* C: Resurrect lines */
    for (; i--; j++) {
	r->screen.tlen[j] = 0;
	r->screen.text[j] = r->buf_text[i];
	r->screen.rend[j] = r->buf_rend[i];
	if (!spec)		/* line length may not equal TermWin.ncol */
	    rxvt_blank_screen_mem(r, r->screen.text, r->screen.rend,
				  (unsigned int)j, r->h->rstyle);
    }

    if (rxvt_Gr_Displayed(r))
	rxvt_Gr_scroll(r, count);
    return count;
}

/* ------------------------------------------------------------------------- */
/*
 * Add text given in <str> of length <len> to screen struct
 */
/* EXTPROTO */
void
rxvt_scr_add_lines(rxvt_t *r, const unsigned char *str, int nlines, int len)
{
    unsigned char   checksel, clearsel;
    char            c;
    int             i, row, last_col;
    text_t         *stp;
    rend_t         *srp;
    struct rxvt_hidden *h = r->h;

    if (len <= 0)		/* sanity */
	return;

    h->want_refresh = 1;
    last_col = r->TermWin.ncol;

    D_SCREEN((stderr, "rxvt_scr_add_lines(%d,%d)", nlines, len));
    ZERO_SCROLLBACK(r);
    if (nlines > 0) {
	nlines += (r->screen.cur.row - r->screen.bscroll);
	if ((nlines > 0)
	    && (r->screen.tscroll == 0)
	    && (r->screen.bscroll == (r->TermWin.nrow - 1))) {
	    /* _at least_ this many lines need to be scrolled */
	    rxvt_scroll_text(r, r->screen.tscroll, r->screen.bscroll, nlines,
			     0);
	    r->screen.cur.row -= nlines;
	}
    }
#ifdef DEBUG_STRICT
    assert(r->screen.cur.col < last_col);
    assert((r->screen.cur.row < r->TermWin.nrow)
	   && (r->screen.cur.row >= -(int32_t)r->TermWin.nscrolled));
#else				/* drive with your eyes closed */
    MIN_IT(r->screen.cur.col, last_col - 1);
    MIN_IT(r->screen.cur.row, r->TermWin.nrow - 1);
    MAX_IT(r->screen.cur.row, -(int32_t)r->TermWin.nscrolled);
#endif
    row = r->screen.cur.row + r->TermWin.saveLines;

    checksel = (r->selection.op
		&& h->current_screen == r->selection.screen) ? 1 : 0;
    clearsel = 0;

    stp = r->screen.text[row];
    srp = r->screen.rend[row];

#ifdef MULTICHAR_SET
    if (h->lost_multi && r->screen.cur.col > 0
	&& IS_MULTI1(srp[r->screen.cur.col - 1])
	&& *str != '\n' && *str != '\r' && *str != '\t')
	h->chstat = WBYTE;
#endif

    for (i = 0; i < len;) {
	c = str[i++];
	switch (c) {
	case '\t':
	    rxvt_scr_tab(r, 1);
	    continue;
	case '\n':
	    if (r->screen.tlen[row] != -1)	/* XXX: think about this */
		MAX_IT(r->screen.tlen[row], r->screen.cur.col);
	    r->screen.flags &= ~Screen_WrapNext;
	    if (r->screen.cur.row == r->screen.bscroll)
		rxvt_scroll_text(r, r->screen.tscroll, r->screen.bscroll, 1, 0);
	    else if (r->screen.cur.row < (r->TermWin.nrow - 1))
		row = (++r->screen.cur.row) + r->TermWin.saveLines;
	    stp = r->screen.text[row];	/* _must_ refresh */
	    srp = r->screen.rend[row];	/* _must_ refresh */
	    RESET_CHSTAT(h);
	    continue;
	case '\r':
	    if (r->screen.tlen[row] != -1)	/* XXX: think about this */
		MAX_IT(r->screen.tlen[row], r->screen.cur.col);
	    r->screen.flags &= ~Screen_WrapNext;
	    r->screen.cur.col = 0;
	    RESET_CHSTAT(h);
	    continue;
	default:
#ifdef MULTICHAR_SET
	    if (r->encoding_method == NOENC) {
		if (c == 127)
		    continue;
		break;
	    }
	    h->rstyle &= ~RS_multiMask;
	    if (h->chstat == WBYTE) {
		h->rstyle |= RS_multi2;	/* multibyte 2nd byte */
		h->chstat = SBYTE;
		if ((r->encoding_method == EUCJ) || (r->encoding_method == GB))
		    c |= 0x80;	/* maybe overkill, but makes it selectable */
	    } else if (h->chstat == SBYTE) {
		if (h->multi_byte || (c & 0x80)) {	/* multibyte 1st byte */
		    h->rstyle |= RS_multi1;
		    h->chstat = WBYTE;
		    if ((r->encoding_method == EUCJ)
			|| (r->encoding_method == GB))
			c |= 0x80;	/* maybe overkill, but makes selectable */
		}
	    } else
#endif
	    if (c == 127)
		continue;	/* yummmm..... */
	    break;
	}

	if (checksel		/* see if we're writing within selection */
	    && !ROWCOL_IS_BEFORE(r->screen.cur, r->selection.beg)
	    && ROWCOL_IS_BEFORE(r->screen.cur, r->selection.end)) {
	    checksel = 0;
	    clearsel = 1;
	}
	if (r->screen.flags & Screen_WrapNext) {
	    r->screen.tlen[row] = -1;
	    if (r->screen.cur.row == r->screen.bscroll)
		rxvt_scroll_text(r, r->screen.tscroll, r->screen.bscroll, 1, 0);
	    else if (r->screen.cur.row < (r->TermWin.nrow - 1))
		row = (++r->screen.cur.row) + r->TermWin.saveLines;
	    stp = r->screen.text[row];	/* _must_ refresh */
	    srp = r->screen.rend[row];	/* _must_ refresh */
	    r->screen.cur.col = 0;
	    r->screen.flags &= ~Screen_WrapNext;
	}
	if (r->screen.flags & Screen_Insert)
	    rxvt_scr_insdel_chars(r, 1, INSERT);
#ifdef MULTICHAR_SET
	if (IS_MULTI1(h->rstyle)
	    && r->screen.cur.col > 0 && IS_MULTI1(srp[r->screen.cur.col - 1])) {
	    stp[r->screen.cur.col - 1] = ' ';
	    srp[r->screen.cur.col - 1] &= ~RS_multiMask;
	} else if (IS_MULTI2(h->rstyle)
		   && r->screen.cur.col < (last_col - 1)
		   && IS_MULTI2(srp[r->screen.cur.col + 1])) {
	    stp[r->screen.cur.col + 1] = ' ';
	    srp[r->screen.cur.col + 1] &= ~RS_multiMask;
	}
#endif
	stp[r->screen.cur.col] = c;
	srp[r->screen.cur.col] = h->rstyle;
	if (r->screen.cur.col < (last_col - 1))
	    r->screen.cur.col++;
	else {
	    r->screen.tlen[row] = last_col;
	    if (r->screen.flags & Screen_Autowrap)
		r->screen.flags |= Screen_WrapNext;
	}
    }
    if (r->screen.tlen[row] != -1)	/* XXX: think about this */
	MAX_IT(r->screen.tlen[row], r->screen.cur.col);

/*
 * If we wrote anywhere in the selected area, kill the selection
 * XXX: should we kill the mark too?  Possibly, but maybe that
 *      should be a similar check.
 */
    if (clearsel)
	CLEAR_SELECTION(r);

#ifdef DEBUG_STRICT
    assert(r->screen.cur.row >= 0);
#else				/* drive with your eyes closed */
    MAX_IT(r->screen.cur.row, 0);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Process Backspace.  Move back the cursor back a position, wrap if have to
 * XTERM_SEQ: CTRL-H
 */
/* EXTPROTO */
void
rxvt_scr_backspace(rxvt_t *r)
{
    RESET_CHSTAT(r->h);
    r->h->want_refresh = 1;
    if (r->screen.cur.col == 0) {
	if (r->screen.cur.row > 0) {
#ifdef TERMCAP_HAS_BW
	    r->screen.cur.col = r->TermWin.ncol - 1;
	    r->screen.cur.row--;
	    return;
#endif
	}
    } else if ((r->screen.flags & Screen_WrapNext) == 0)
	rxvt_scr_gotorc(r, 0, -1, RELATIVE);
    r->screen.flags &= ~Screen_WrapNext;
}

/* ------------------------------------------------------------------------- */
/*
 * Process Horizontal Tab
 * count: +ve = forward; -ve = backwards
 * XTERM_SEQ: CTRL-I
 */
/* EXTPROTO */
void
rxvt_scr_tab(rxvt_t *r, int count)
{
    int             i, x;

    D_SCREEN((stderr, "rxvt_scr_tab(%d)", count));
    r->h->want_refresh = 1;
    RESET_CHSTAT(r->h);
    i = x = r->screen.cur.col;
    if (count == 0)
	return;
    else if (count > 0) {
	for (; ++i < r->TermWin.ncol; )
	    if (r->tabs[i]) {
		x = i;
		if (!--count)
		    break;
	    }
	if (count)
	    x = r->TermWin.ncol - 1;
    } else /* if (count < 0) */ {
	for (; --i >= 0; )
	    if (r->tabs[i]) {
		x = i;
		if (!++count)
		    break;
	    }
	if (count)
	    x = 0;
    }
    if (x != r->screen.cur.col)
	rxvt_scr_gotorc(r, 0, x, R_RELATIVE);
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
rxvt_scr_backindex(rxvt_t *r)
{
    if (r->screen.cur.col > 0)
	rxvt_scr_gotorc(r, 0, -1, R_RELATIVE | C_RELATIVE);
    else {
	if (r->screen.tlen[r->screen.cur.row + r->TermWin.saveLines] == 0)
	    return;		/* um, yeah? */
	rxvt_scr_insdel_chars(r, 1, INSERT);
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
rxvt_scr_forwardindex(rxvt_t *r)
{
    int             row;

    if (r->screen.cur.col < r->TermWin.ncol - 1)
	rxvt_scr_gotorc(r, 0, 1, R_RELATIVE | C_RELATIVE);
    else {
	row = r->screen.cur.row + r->TermWin.saveLines;
	if (r->screen.tlen[row] == 0)
	    return;		/* um, yeah? */
	else if (r->screen.tlen[row] == -1)
	    r->screen.tlen[row] = r->TermWin.ncol;
	rxvt_scr_gotorc(r, 0, 0, R_RELATIVE);
	rxvt_scr_insdel_chars(r, 1, DELETE);
	rxvt_scr_gotorc(r, 0, r->TermWin.ncol - 1, R_RELATIVE);
    }
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Goto Row/Column
 */
/* EXTPROTO */
void
rxvt_scr_gotorc(rxvt_t *r, int row, int col, int relative)
{
    r->h->want_refresh = 1;
    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r->h);
    if (rxvt_Gr_Displayed(r))
	rxvt_Gr_scroll(r, 0);

    D_SCREEN((stderr, "rxvt_scr_gotorc(r:%s%d,c:%s%d): from (r:%d,c:%d)", (relative & R_RELATIVE ? "+" : ""), row, (relative & C_RELATIVE ? "+" : ""), col, r->screen.cur.row, r->screen.cur.col));

    r->screen.cur.col = ((relative & C_RELATIVE) ? (r->screen.cur.col + col)
						 : col);
    MAX_IT(r->screen.cur.col, 0);
    MIN_IT(r->screen.cur.col, r->TermWin.ncol - 1);

    r->screen.flags &= ~Screen_WrapNext;
    if (relative & R_RELATIVE) {
	if (row > 0) {
	    if (r->screen.cur.row <= r->screen.bscroll
		&& (r->screen.cur.row + row) > r->screen.bscroll)
		r->screen.cur.row = r->screen.bscroll;
	    else
		r->screen.cur.row += row;
	} else if (row < 0) {
	    if (r->screen.cur.row >= r->screen.tscroll
		&& (r->screen.cur.row + row) < r->screen.tscroll)
		r->screen.cur.row = r->screen.tscroll;
	    else
		r->screen.cur.row += row;
	}
    } else {
	if (r->screen.flags & Screen_Relative) {	/* relative origin mode */
	    r->screen.cur.row = row + r->screen.tscroll;
	    MIN_IT(r->screen.cur.row, r->screen.bscroll);
	} else
	    r->screen.cur.row = row;
    }
    MAX_IT(r->screen.cur.row, 0);
    MIN_IT(r->screen.cur.row, r->TermWin.nrow - 1);
}

/* ------------------------------------------------------------------------- */
/*
 * direction  should be UP or DN
 */
/* EXTPROTO */
void
rxvt_scr_index(rxvt_t *r, enum page_dirn direction)
{
    int             dirn;

    r->h->want_refresh = 1;
    dirn = ((direction == UP) ? 1 : -1);
    D_SCREEN((stderr, "rxvt_scr_index(%d)", dirn));

    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r->h);
    if (rxvt_Gr_Displayed(r))
	rxvt_Gr_scroll(r, 0);

    r->screen.flags &= ~Screen_WrapNext;
    if ((r->screen.cur.row == r->screen.bscroll && direction == UP)
	|| (r->screen.cur.row == r->screen.tscroll && direction == DN))
	rxvt_scroll_text(r, r->screen.tscroll, r->screen.bscroll, dirn, 0);
    else
	r->screen.cur.row += dirn;
    MAX_IT(r->screen.cur.row, 0);
    MIN_IT(r->screen.cur.row, r->TermWin.nrow - 1);
    rxvt_selection_check(r, 0);
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
rxvt_scr_erase_line(rxvt_t *r, int mode)
{
    unsigned int    row, col, num;

    r->h->want_refresh = 1;
    D_SCREEN((stderr, "rxvt_scr_erase_line(%d) at screen row: %d", mode, r->screen.cur.row));
    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r->h);
    if (rxvt_Gr_Displayed(r))
	rxvt_Gr_scroll(r, 0);
    rxvt_selection_check(r, 1);

    r->screen.flags &= ~Screen_WrapNext;

    row = r->TermWin.saveLines + r->screen.cur.row;
    switch (mode) {
    case 0:			/* erase to end of line */
	col = r->screen.cur.col;
	num = r->TermWin.ncol - col;
	MIN_IT(r->screen.tlen[row], (int16_t)col);
	if (ROWCOL_IN_ROW_AT_OR_AFTER(r->selection.beg, r->screen.cur)
	    || ROWCOL_IN_ROW_AT_OR_AFTER(r->selection.end, r->screen.cur))
	    CLEAR_SELECTION(r);
	break;
    case 1:			/* erase to beginning of line */
	col = 0;
	num = r->screen.cur.col + 1;
	if (ROWCOL_IN_ROW_AT_OR_BEFORE(r->selection.beg, r->screen.cur)
	    || ROWCOL_IN_ROW_AT_OR_BEFORE(r->selection.end, r->screen.cur))
	    CLEAR_SELECTION(r);
	break;
    case 2:			/* erase whole line */
	col = 0;
	num = r->TermWin.ncol;
	r->screen.tlen[row] = 0;
	if (r->selection.beg.row <= r->screen.cur.row
	    && r->selection.end.row >= r->screen.cur.row)
	    CLEAR_SELECTION(r);
	break;
    default:
	return;
    }
    if (r->screen.text[row])
	rxvt_blank_line(&(r->screen.text[row][col]),
			&(r->screen.rend[row][col]), num, r->h->rstyle);
    else
	rxvt_blank_screen_mem(r, r->screen.text, r->screen.rend, row,
			      r->h->rstyle);
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
rxvt_scr_erase_screen(rxvt_t *r, int mode)
{
    int             num;
    int32_t         row, row_offset;
    rend_t          ren;
    XGCValues       gcvalue;

    r->h->want_refresh = 1;
    D_SCREEN((stderr, "rxvt_scr_erase_screen(%d) at screen row: %d", mode, r->screen.cur.row));
    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r->h);
    row_offset = (int32_t)r->TermWin.saveLines;

    switch (mode) {
    case 0:			/* erase to end of screen */
	rxvt_selection_check(r, 1);
	rxvt_scr_erase_line(r, 0);
	row = r->screen.cur.row + 1;	/* possible OOB */
	num = r->TermWin.nrow - row;
	break;
    case 1:			/* erase to beginning of screen */
	rxvt_selection_check(r, 3);
	rxvt_scr_erase_line(r, 1);
	row = 0;
	num = r->screen.cur.row;
	break;
    case 2:			/* erase whole screen */
	rxvt_selection_check(r, 3);
	rxvt_Gr_ClearScreen(r);
	row = 0;
	num = r->TermWin.nrow;
	break;
    default:
	return;
    }
    r->h->refresh_type |= REFRESH_BOUNDS;
    if (r->selection.op && r->h->current_screen == r->selection.screen
	&& ((r->selection.beg.row >= row && r->selection.beg.row <= row + num)
	    || (r->selection.end.row >= row
		&& r->selection.end.row <= row + num)))
	CLEAR_SELECTION(r);
    if (row >= r->TermWin.nrow)	/* Out Of Bounds */
	return;
    MIN_IT(num, (r->TermWin.nrow - row));
    if (r->h->rstyle & (RS_RVid | RS_Uline))
	ren = (rend_t) ~RS_None;
    else if (GET_BASEBG(r->h->rstyle) == Color_bg) {
	ren = DEFAULT_RSTYLE;
	CLEAR_ROWS(row, num);
    } else {
	ren = (r->h->rstyle & (RS_fgMask | RS_bgMask));
	gcvalue.foreground = r->PixColors[GET_BGCOLOR(r->h->rstyle)];
	XChangeGC(r->Xdisplay, r->TermWin.gc, GCForeground, &gcvalue);
	ERASE_ROWS(row, num);
	gcvalue.foreground = r->PixColors[Color_fg];
	XChangeGC(r->Xdisplay, r->TermWin.gc, GCForeground, &gcvalue);
    }
    for (; num--; row++) {
	rxvt_blank_screen_mem(r, r->screen.text, r->screen.rend,
			      (unsigned int)(row + row_offset), r->h->rstyle);
	r->screen.tlen[row + row_offset] = 0;
	rxvt_blank_line(r->drawn_text[row], r->drawn_rend[row],
			(unsigned int)r->TermWin.ncol, ren);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Fill the screen with `E's
 * XTERM_SEQ: Screen Alignment Test: ESC # 8
 */
/* EXTPROTO */
void
rxvt_scr_E(rxvt_t *r)
{
    int             i, j, k;
    rend_t         *r1, fs;

    r->h->want_refresh = 1;
    r->h->num_scr_allow = 0;
    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r->h);
    rxvt_selection_check(r, 3);

    fs = r->h->rstyle;
    for (k = r->TermWin.saveLines, i = r->TermWin.nrow; i--; k++) {
	r->screen.tlen[k] = r->TermWin.ncol;	/* make the `E's selectable */
	MEMSET(r->screen.text[k], 'E', r->TermWin.ncol);
	for (r1 = r->screen.rend[k], j = r->TermWin.ncol; j--; )
	    *r1++ = fs;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> lines
 */
/* EXTPROTO */
void
rxvt_scr_insdel_lines(rxvt_t *r, int count, int insdel)
{
    int             end;

    ZERO_SCROLLBACK(r);
    RESET_CHSTAT(r->h);
    if (rxvt_Gr_Displayed(r))
	rxvt_Gr_scroll(r, 0);
    rxvt_selection_check(r, 1);

    if (r->screen.cur.row > r->screen.bscroll)
	return;

    end = r->screen.bscroll - r->screen.cur.row + 1;
    if (count > end) {
	if (insdel == DELETE)
	    return;
	else if (insdel == INSERT)
	    count = end;
    }
    r->screen.flags &= ~Screen_WrapNext;

    rxvt_scroll_text(r, r->screen.cur.row, r->screen.bscroll, insdel * count,
		     0);
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> characters from the current position
 */
/* EXTPROTO */
void
rxvt_scr_insdel_chars(rxvt_t *r, int count, int insdel)
{
    int             col, row;
    rend_t          tr;
    text_t         *stp;
    rend_t         *srp;
    int16_t        *slp;

    r->h->want_refresh = 1;
    ZERO_SCROLLBACK(r);
#if 0
    RESET_CHSTAT(r->h);
#endif
    if (rxvt_Gr_Displayed(r))
	rxvt_Gr_scroll(r, 0);

    if (count <= 0)
	return;

    rxvt_selection_check(r, 1);
    MIN_IT(count, (r->TermWin.ncol - r->screen.cur.col));

    row = r->screen.cur.row + r->TermWin.saveLines;
    r->screen.flags &= ~Screen_WrapNext;

    stp = r->screen.text[row];
    srp = r->screen.rend[row];
    slp = &(r->screen.tlen[row]);
    switch (insdel) {
    case INSERT:
	for (col = r->TermWin.ncol - 1; (col - count) >= r->screen.cur.col;
	     col--) {
	    stp[col] = stp[col - count];
	    srp[col] = srp[col - count];
	}
	if (*slp != -1) {
	    *slp += count;
	    MIN_IT(*slp, r->TermWin.ncol);
	}
	if (r->selection.op && r->h->current_screen == r->selection.screen
	    && ROWCOL_IN_ROW_AT_OR_AFTER(r->selection.beg, r->screen.cur)) {
	    if (r->selection.end.row != r->screen.cur.row
		|| (r->selection.end.col + count >= r->TermWin.ncol))
		CLEAR_SELECTION(r);
	    else {		/* shift selection */
		r->selection.beg.col += count;
		r->selection.mark.col += count;	/* XXX: yes? */
		r->selection.end.col += count;
	    }
	}
	rxvt_blank_line(&(stp[r->screen.cur.col]), &(srp[r->screen.cur.col]),
			(unsigned int)count, r->h->rstyle);
	break;
    case ERASE:
	r->screen.cur.col += count;	/* don't worry if > r->TermWin.ncol */
	rxvt_selection_check(r, 1);
	r->screen.cur.col -= count;
	rxvt_blank_line(&(stp[r->screen.cur.col]), &(srp[r->screen.cur.col]),
			(unsigned int)count, r->h->rstyle);
	break;
    case DELETE:
	tr = srp[r->TermWin.ncol - 1]
	     & (RS_fgMask | RS_bgMask | RS_baseattrMask);
	for (col = r->screen.cur.col; (col + count) < r->TermWin.ncol; col++) {
	    stp[col] = stp[col + count];
	    srp[col] = srp[col + count];
	}
	rxvt_blank_line(&(stp[r->TermWin.ncol - count]),
			&(srp[r->TermWin.ncol - count]),
			(unsigned int)count, tr);
	if (*slp == -1)	/* break line continuation */
	    *slp = r->TermWin.ncol;
	*slp -= count;
	MAX_IT(*slp, 0);
	if (r->selection.op && r->h->current_screen == r->selection.screen
	    && ROWCOL_IN_ROW_AT_OR_AFTER(r->selection.beg, r->screen.cur)) {
	    if (r->selection.end.row != r->screen.cur.row
		|| (r->screen.cur.col >= r->selection.beg.col - count)
		|| r->selection.end.col >= r->TermWin.ncol)
		CLEAR_SELECTION(r);
	    else {
		/* shift selection */
		r->selection.beg.col -= count;
		r->selection.mark.col -= count;	/* XXX: yes? */
		r->selection.end.col -= count;
	    }
	}
	break;
    }
#if 0
    if (IS_MULTI2(srp[0])) {
	srp[0] &= ~RS_multiMask;
	stp[0] = ' ';
    }
    if (IS_MULTI1(srp[r->TermWin.ncol - 1])) {
	srp[r->TermWin.ncol - 1] &= ~RS_multiMask;
	stp[r->TermWin.ncol - 1] = ' ';
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
rxvt_scr_scroll_region(rxvt_t *r, int top, int bot)
{
    MAX_IT(top, 0);
    MIN_IT(bot, r->TermWin.nrow - 1);
    if (top > bot)
	return;
    r->screen.tscroll = top;
    r->screen.bscroll = bot;
    rxvt_scr_gotorc(r, 0, 0, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Make the cursor visible/invisible
 * XTERM_SEQ: Make cursor visible  : ESC [ ? 25 h
 * XTERM_SEQ: Make cursor invisible: ESC [ ? 25 l
 */
/* EXTPROTO */
void
rxvt_scr_cursor_visible(rxvt_t *r, int mode)
{
    r->h->want_refresh = 1;
    if (mode)
	r->screen.flags |= Screen_VisibleCursor;
    else
	r->screen.flags &= ~Screen_VisibleCursor;
}

/* ------------------------------------------------------------------------- */
/*
 * Set/unset automatic wrapping
 * XTERM_SEQ: Set Wraparound  : ESC [ ? 7 h
 * XTERM_SEQ: Unset Wraparound: ESC [ ? 7 l
 */
/* EXTPROTO */
void
rxvt_scr_autowrap(rxvt_t *r, int mode)
{
    if (mode)
	r->screen.flags |= Screen_Autowrap;
    else
	r->screen.flags &= ~(Screen_Autowrap | Screen_WrapNext);
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
rxvt_scr_relative_origin(rxvt_t *r, int mode)
{
    if (mode)
	r->screen.flags |= Screen_Relative;
    else
	r->screen.flags &= ~Screen_Relative;
    rxvt_scr_gotorc(r, 0, 0, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Set insert/replace mode
 * XTERM_SEQ: Set Insert mode : ESC [ ? 4 h
 * XTERM_SEQ: Set Replace mode: ESC [ ? 4 l
 */
/* EXTPROTO */
void
rxvt_scr_insert_mode(rxvt_t *r, int mode)
{
    if (mode)
	r->screen.flags |= Screen_Insert;
    else
	r->screen.flags &= ~Screen_Insert;
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
rxvt_scr_set_tab(rxvt_t *r, int mode)
{
    if (mode < 0)
	MEMSET(r->tabs, 0, r->TermWin.ncol * sizeof(char));
    else if (r->screen.cur.col < r->TermWin.ncol)
	r->tabs[r->screen.cur.col] = (mode ? 1 : 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Set reverse/normal video
 * XTERM_SEQ: Reverse video: ESC [ ? 5 h
 * XTERM_SEQ: Normal video : ESC [ ? 5 l
 */
/* EXTPROTO */
void
rxvt_scr_rvideo_mode(rxvt_t *r, int mode)
{
    XGCValues       gcvalue;

    if (r->h->rvideo != mode) {
	r->h->rvideo = mode;
	SWAP_IT(r->PixColors[Color_fg], r->PixColors[Color_bg], Pixel);
#if defined(XPM_BACKGROUND)
	if (r->h->bgPixmap.pixmap == None)
#endif
#if defined(TRANSPARENT)
	    if (!(r->Options & Opt_transparent) || r->h->am_transparent == 0)
#endif
	    XSetWindowBackground(r->Xdisplay, r->TermWin.vt,
				 r->PixColors[Color_bg]);

	gcvalue.foreground = r->PixColors[Color_fg];
	gcvalue.background = r->PixColors[Color_bg];
	XChangeGC(r->Xdisplay, r->TermWin.gc, GCBackground | GCForeground,
		  &gcvalue);
	rxvt_scr_clear(r);
	rxvt_scr_touch(r, True);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Report current cursor position
 * XTERM_SEQ: Report position: ESC [ 6 n
 */
/* EXTPROTO */
void
rxvt_scr_report_position(rxvt_t *r)
{
    rxvt_tt_printf(r, "\033[%d;%dR", r->screen.cur.row + 1,
		   r->screen.cur.col + 1);
}

/* ------------------------------------------------------------------------- *
 *                                  FONTS                                    *
 * ------------------------------------------------------------------------- */

/*
 * Set font style
 */
/* INTPROTO */
void
rxvt_set_font_style(rxvt_t *r)
{
    r->h->rstyle &= ~RS_fontMask;
    switch (r->h->charsets[r->screen.charset]) {
    case '0':			/* DEC Special Character & Line Drawing Set */
	r->h->rstyle |= RS_acsFont;
	break;
    case 'A':			/* United Kingdom (UK) */
	r->h->rstyle |= RS_ukFont;
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
rxvt_scr_charset_choose(rxvt_t *r, int set)
{
    r->screen.charset = set;
    rxvt_set_font_style(r);
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
rxvt_scr_charset_set(rxvt_t *r, int set, unsigned int ch)
{
#ifdef MULTICHAR_SET
    r->h->multi_byte = !!(set < 0);
    set = abs(set);
#endif
    r->h->charsets[set] = (unsigned char)ch;
    rxvt_set_font_style(r);
}

/* ------------------------------------------------------------------------- *
 *          MULTIPLE-CHARACTER FONT SET MANIPULATION FUNCTIONS               *
 * ------------------------------------------------------------------------- */
#ifdef MULTICHAR_SET
/* EXTPROTO */
void
rxvt_euc2jis(unsigned char *str, int len)
{
    int             i;

    for (i = 0; i < len; i++)
	str[i] &= 0x7F;
}

/* ------------------------------------------------------------------------- */
/* INTPROTO */
void
rxvt_sjis2jis(unsigned char *str, int len)
{
    int             i;
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
rxvt_decodedummy(unsigned char *str __attribute__((unused)), int len __attribute__((unused)))
{
}

/* EXTPROTO */
void
rxvt_set_multichar_encoding(rxvt_t *r, const char *str)
{
    KNOWN_ENCODINGS *a;

    if (str && *str) {
	for (a = (KNOWN_ENCODINGS *)known_encodings; a->name; a++) {
	    if (STRCASECMP(str, a->name) == 0) {
		r->encoding_method = a->method;
		r->h->multichar_decode = a->func;
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
rxvt_scr_get_fgcolor(rxvt_t *r)
{
    return GET_FGCOLOR(r->h->rstyle);
}

/* ------------------------------------------------------------------------- */
/* EXTPROTO */
int
rxvt_scr_get_bgcolor(rxvt_t *r)
{
    return GET_BGCOLOR(r->h->rstyle);
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
    RC_COUNT
};

/* EXTPROTO */
void
rxvt_scr_expose(rxvt_t *r, int x, int y, int width, int height, Bool refresh)
{
    int             i;
    row_col_t       rc[RC_COUNT];

    if (r->drawn_text == NULL)	/* sanity check */
	return;

#ifdef DEBUG_STRICT
    x = max(x, (int)r->TermWin.int_bwidth);
    x = min(x, (int)r->TermWin.width);
    y = max(y, (int)r->TermWin.int_bwidth);
    y = min(y, (int)r->TermWin.height);
#endif

/* round down */
    rc[PART_BEG].col = Pixel2Col(x);
    rc[PART_BEG].row = Pixel2Row(y);
/* round up */
    rc[PART_END].col = Pixel2Width(x + width + r->TermWin.fwidth - 1);
    rc[PART_END].row = Pixel2Row(y + height + r->TermWin.fheight - 1);

/* sanity checks */
    for (i = PART_BEG; i < RC_COUNT; i++) {
	MIN_IT(rc[i].col, r->TermWin.ncol - 1);
	MIN_IT(rc[i].row, r->TermWin.nrow - 1);
    }

    D_SCREEN((stderr, "rxvt_scr_expose(x:%d, y:%d, w:%d, h:%d) area (c:%d,r:%d)-(c:%d,r:%d)", x, y, width, height, rc[PART_BEG].col, rc[PART_BEG].row, rc[PART_END].col, rc[PART_END].row));

    for (i = rc[PART_BEG].row; i <= rc[PART_END].row; i++)
	MEMSET(&(r->drawn_text[i][rc[PART_BEG].col]), 0,
	       rc[PART_END].col - rc[PART_BEG].col + 1);

    if (refresh)
	rxvt_scr_refresh(r, SLOW_REFRESH | REFRESH_BOUNDS);
}

/* ------------------------------------------------------------------------- */
/*
 * Refresh the entire screen
 */
/* EXTPROTO */
void
rxvt_scr_touch(rxvt_t *r, Bool refresh)
{
    rxvt_scr_expose(r, 0, 0, r->TermWin.width, r->TermWin.height, refresh);
}

/* ------------------------------------------------------------------------- */
/*
 * Move the display so that the line represented by scrollbar value Y is at
 * the top of the screen
 */
/* EXTPROTO */
int
rxvt_scr_move_to(rxvt_t *r, int y, int len)
{
    long            p = 0;
    u_int16_t       oldviewstart;

    oldviewstart = r->TermWin.view_start;
    if (y < len) {
	p = (r->TermWin.nrow + r->TermWin.nscrolled) * (len - y) / len;
	p -= (long)(r->TermWin.nrow - 1);
	p = max(p, 0);
    }
    r->TermWin.view_start = (u_int16_t)min(p, r->TermWin.nscrolled);
    D_SCREEN((stderr, "rxvt_scr_move_to(%d, %d) view_start:%d", y, len, r->TermWin.view_start));

    return rxvt_scr_changeview(r, oldviewstart);
}

/* ------------------------------------------------------------------------- */
/*
 * Page the screen up/down nlines
 * direction should be UP or DN
 */
/* EXTPROTO */
int
rxvt_scr_page(rxvt_t *r, enum page_dirn direction, int nlines)
{
    int             n;
    u_int16_t       oldviewstart;

    D_SCREEN((stderr, "rxvt_scr_page(%s, %d) view_start:%d", ((direction == UP) ? "UP" : "DN"), nlines, r->TermWin.view_start));
#ifdef DEBUG_STRICT
    assert((nlines >= 1) && (nlines <= r->TermWin.nrow));
#endif
    oldviewstart = r->TermWin.view_start;
    if (direction == UP) {
	n = r->TermWin.view_start + nlines;
	r->TermWin.view_start = min(n, r->TermWin.nscrolled);
    } else {
	n = r->TermWin.view_start - nlines;
	r->TermWin.view_start = max(n, 0);
    }
    return rxvt_scr_changeview(r, oldviewstart);
}

/* INTPROTO */
int
rxvt_scr_changeview(rxvt_t *r, u_int16_t oldviewstart)
{
    if (r->TermWin.view_start != oldviewstart) {
	r->h->want_refresh = 1;
	if (rxvt_Gr_Displayed(r))
	    rxvt_Gr_scroll(r, 0);
	r->h->num_scr -= (r->TermWin.view_start - oldviewstart);
    }
    return (int)(r->TermWin.view_start - oldviewstart);
}

/* ------------------------------------------------------------------------- */
/* EXTPROTO */
void
rxvt_scr_bell(rxvt_t *r)
{
#ifndef NO_BELL
# ifndef NO_MAPALERT
#  ifdef MAPALERT_OPTION
    if (r->Options & Opt_mapAlert)
#  endif
	XMapWindow(r->Xdisplay, r->TermWin.parent[0]);
# endif
    if (r->Options & Opt_visualBell) {
	rxvt_scr_rvideo_mode(r, !r->h->rvideo); /* refresh also done */
	rxvt_scr_rvideo_mode(r, !r->h->rvideo); /* refresh also done */
    } else
	XBell(r->Xdisplay, 0);
#endif
}

/* ------------------------------------------------------------------------- */
/* ARGSUSED */
/* EXTPROTO */
void
rxvt_scr_printscreen(rxvt_t *r, int fullhist)
{
#ifdef PRINTPIPE
    int             i, r1, nrows, row_offset;
    text_t         *t;
    FILE           *fd;

    if ((fd = rxvt_popen_printer(r)) == NULL)
	return;
    nrows = r->TermWin.nrow;
    row_offset = r->TermWin.saveLines;
    if (!fullhist)
	row_offset -= r->TermWin.view_start;
    else {
	nrows += r->TermWin.nscrolled;
	row_offset -= r->TermWin.nscrolled;
    }

    for (r1 = 0; r1 < nrows; r1++) {
	t = r->screen.text[r1 + row_offset];
	for (i = r->TermWin.ncol - 1; i >= 0; i--)
	    if (!isspace(t[i]))
		break;
	fprintf(fd, "%.*s\n", (i + 1), t);
    }
    rxvt_pclose_printer(fd);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Refresh the screen
 * r->drawn_text/r->drawn_rend contain the screen information before the update.
 * r->screen.text/r->screen.rend contain what the screen will change to.
 */

#define DRAW_STRING(Func, x, y, str, len)				\
    Func(r->Xdisplay, drawBuffer, r->TermWin.gc, (x), (y), (str), (len))

#if defined (NO_BRIGHTCOLOR) || defined (VERYBOLD)
# define MONO_BOLD(x)		((x) & (RS_Bold|RS_Blink))
# define MONO_BOLD_FG(x, fg)	MONO_BOLD(x)
#else
# define MONO_BOLD(x)						\
    (((x) & (RS_Bold | RS_fgMask)) == (RS_Bold | Color_fg))
# define MONO_BOLD_FG(x, fg)	(((x) & RS_Bold) && (fg) == Color_fg)
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
rxvt_scr_refresh(rxvt_t *r, unsigned char refresh_type)
{
    unsigned char   clearfirst,	/* first character writes before cell        */
		    clearlast,	/* last character writes beyond cell         */
		    must_clear,	/* use draw_string not draw_image_string     */
#ifndef NO_BOLDFONT
		    bfont,	/* we've changed font to bold font           */
#endif
		    rvid,	/* reverse video this position               */
		    wbyte;	/* we're in multibyte                        */
    char            morecur = 0;/*                                           */
#ifdef TTY_256COLOR
    u_int16_t	    fore, back;	/* desired foreground/background             */
#else
    unsigned char   fore, back;	/* desired foreground/background             */
#endif
    int16_t         col, row,	/* column/row we're processing               */
                    ocrow,	/* old cursor row                            */
		    len, wlen;	/* text length screen/buffer                 */
    int             i,		/* tmp                                       */
		    row_offset;	/* basic offset in screen structure          */
#ifndef NO_CURSORCOLOR
    rend_t	    cc1;	/* store colours at cursor position(s)       */
# ifdef MULTICHAR_SET
    rend_t          cc2;	/* store colours at cursor position(s)       */
# endif
#endif
    XGCValues       gcvalue;	/* Graphics Context values                   */
    rend_t         *drp, *srp;	/* drawn-rend-pointer, screen-rend-pointer   */
    text_t         *dtp, *stp;	/* drawn-text-pointer, screen-text-pointer   */
    char           *buffer;	/* local copy of r->h->buffer                */
    struct rxvt_hidden *h = r->h;
    int             (*draw_string) () = XDrawString;
    int             (*draw_image_string) () = XDrawImageString;

    if (refresh_type == NO_REFRESH || !r->TermWin.mapped)
	return;

/*
 * A: set up vars
 */
    clearfirst = clearlast = must_clear = wbyte = 0;
#ifndef NO_BOLDFONT
    bfont = 0;
#endif

    if (h->currmaxcol < r->TermWin.ncol) {
	h->currmaxcol = r->TermWin.ncol;
	h->buffer = rxvt_realloc(h->buffer, sizeof(char) * (h->currmaxcol + 1));
    }
    buffer = h->buffer;
    h->refresh_count = 0;

    row_offset = r->TermWin.saveLines - r->TermWin.view_start;
/*
 * always go back to the base font - it's much safer
 */
    XSetFont(r->Xdisplay, r->TermWin.gc, r->TermWin.font->fid);

    if ((refresh_type & REFRESH_BOUNDS)) {
	clearfirst = clearlast = 1;
	h->refresh_type &= ~REFRESH_BOUNDS;
    }
#if defined(XPM_BACKGROUND)
    must_clear |= (h->bgPixmap.pixmap != None);
#endif
#if defined(TRANSPARENT)
    must_clear |= ((r->Options & Opt_transparent) && h->am_transparent);
#endif
    ocrow = h->oldcursor.row; /* is there an old outline cursor on screen? */

    /* set base colours to avoid check in "single glyph writing" below */
    gcvalue.foreground = r->PixColors[Color_fg];
    gcvalue.background = r->PixColors[Color_bg];

/*
 * B: reverse any characters which are selected
 */
    rxvt_scr_reverse_selection(r);

/*
 * C: set the cursor character(s)
 */
    {
	unsigned char   setoldcursor;
	rend_t          ccol1,	/* Cursor colour       */
	                ccol2;	/* Cursor colour2      */

	if ((r->screen.flags & Screen_VisibleCursor) && r->TermWin.focus) {
	    srp = &(r->screen.rend[r->screen.cur.row + r->TermWin.saveLines]
				  [r->screen.cur.col]);
	    *srp ^= RS_RVid;
#ifndef NO_CURSORCOLOR
	    cc1 = *srp & (RS_fgMask | RS_bgMask);
	    if (XDEPTH > 2 && ISSET_PIXCOLOR(h, Color_cursor))
		ccol1 = Color_cursor;
	    else
#ifdef CURSOR_COLOR_IS_RENDITION_COLOR
		ccol1 = GET_FGCOLOR(h->rstyle);
#else
		ccol1 = Color_fg;
#endif
	    if (XDEPTH > 2 && ISSET_PIXCOLOR(h, Color_cursor))
		ccol2 = Color_cursor2;
	    else
#ifdef CURSOR_COLOR_IS_RENDITION_COLOR
		ccol2 = GET_BGCOLOR(h->rstyle);
#else
		ccol2 = Color_bg;
#endif
	    *srp = SET_FGCOLOR(*srp, ccol1);
	    *srp = SET_BGCOLOR(*srp, ccol2);
#endif
#ifdef MULTICHAR_SET
	    if (IS_MULTI1(*srp)) {
		if (r->screen.cur.col < r->TermWin.ncol - 2
		    && IS_MULTI2(*++srp))
		    morecur = 1;
	    } else if (IS_MULTI2(*srp)) {
		if (r->screen.cur.col > 0 && IS_MULTI1(*--srp))
		    morecur = -1;
	    }
	    if (morecur) {
		*srp ^= RS_RVid;
	    }
# ifndef NO_CURSORCOLOR
	    if (morecur) {
		cc2 = *srp & (RS_fgMask | RS_bgMask);
		*srp = SET_FGCOLOR(*srp, morecur==-1?Color_fg:ccol1);
		*srp = SET_BGCOLOR(*srp, morecur==-1?Color_bg:ccol2);
	    }
	    if (morecur==-1) {
	        /* overwrite the color attribute of the other byte */
	        *(srp-morecur) &= ~(RS_fgMask | RS_bgMask);
	        *(srp-morecur) |= *srp & (RS_fgMask | RS_bgMask);
	    }
# endif
#endif
	}

	/* make sure no outline cursor is left around */
	setoldcursor = 0;
	if (ocrow != -1) {
	    if (r->screen.cur.row + r->TermWin.view_start != ocrow
		|| r->screen.cur.col != h->oldcursor.col) {
		if (ocrow < r->TermWin.nrow
		    && h->oldcursor.col < r->TermWin.ncol) {
		    r->drawn_rend[ocrow][h->oldcursor.col] ^= (RS_RVid | RS_Uline);
#ifdef MULTICHAR_SET
		    if (h->oldcursormulti) {
			col = h->oldcursor.col + h->oldcursormulti;
			if (col < r->TermWin.ncol)
			    r->drawn_rend[ocrow][col] ^= (RS_RVid | RS_Uline);
		    }
#endif
		}
		if (r->TermWin.focus
		    || !(r->screen.flags & Screen_VisibleCursor))
		    h->oldcursor.row = -1;
		else
		    setoldcursor = 1;
	    }
	} else if (!r->TermWin.focus)
	    setoldcursor = 1;
	if (setoldcursor) {
	    if (r->screen.cur.row + r->TermWin.view_start >= r->TermWin.nrow)
		h->oldcursor.row = -1;
	    else {
		h->oldcursor.row = r->screen.cur.row + r->TermWin.view_start;
		h->oldcursor.col = r->screen.cur.col;
#ifdef MULTICHAR_SET
		h->oldcursormulti = morecur;
#endif
	    }
	}
    }

#ifndef NO_SLOW_LINK_SUPPORT
/*
 * D: CopyArea pass - very useful for slower links
 *    This has been deliberately kept simple.
 */
    i = h->num_scr;
    if (refresh_type == FAST_REFRESH && h->num_scr_allow && i
	&& abs(i) < r->TermWin.nrow && !must_clear) {
	int16_t         nits;
	int             j;
	rend_t         *drp2;
	text_t         *dtp2;

	j = r->TermWin.nrow;
	wlen = len = -1;
	row = i > 0 ? 0 : j - 1;
	for (; j-- >= 0; row += (i > 0 ? 1 : -1)) {
	    if (row + i >= 0 && row + i < r->TermWin.nrow && row + i != ocrow) {
		stp = r->screen.text[row + row_offset];
		srp = r->screen.rend[row + row_offset];
		dtp = r->drawn_text[row];
		dtp2 = r->drawn_text[row + i];
		drp = r->drawn_rend[row];
		drp2 = r->drawn_rend[row + i];
		for (nits = 0, col = r->TermWin.ncol; col--; )
		    if (stp[col] != dtp2[col] || srp[col] != drp2[col])
			nits--;
		    else if (stp[col] != dtp[col] || srp[col] != drp[col])
			nits++;
		if (nits > 8) {	/* XXX: arbitrary choice */
		    for (col = r->TermWin.ncol; col--; ) {
			*dtp++ = *dtp2++;
			*drp++ = *drp2++;
		    }
		    if (len == -1)
			len = row;
		    wlen = row;
		    continue;
		}
	    }
	    if (len != -1) {
		/* also comes here at end if needed because of >= above */
		if (wlen < len)
		    SWAP_IT(wlen, len, int);
		D_SCREEN((stderr, "rxvt_scr_refresh(): XCopyArea: %d -> %d (height: %d)", len + i, len, wlen - len + 1));
		XCopyArea(r->Xdisplay, r->TermWin.vt, r->TermWin.vt,
			  r->TermWin.gc, 0, Row2Pixel(len + i),
			  (unsigned int)TermWin_TotalWidth(),
			  (unsigned int)Height2Pixel(wlen - len + 1),
			  0, Row2Pixel(len));
		len = -1;
	    }
	}
    }
#endif


/*
 * E: main pass across every character
 */
    for (row = 0; row < r->TermWin.nrow; row++) {
	unsigned char   clear_next = 0;
	int             j,
			xpixel,	 /* x offset for start of drawing (font) */
		        ypixel,	 /* y offset for start of drawing (font) */
		        ypixelc; /* y offset for top of drawing          */
	unsigned long   gcmask;	 /* Graphics Context mask                */
	const XFontStruct *wf;	/* which font are we in                      */

	stp = r->screen.text[row + row_offset];
	srp = r->screen.rend[row + row_offset];
	dtp = r->drawn_text[row];
	drp = r->drawn_rend[row];

#ifndef NO_PIXEL_DROPPING_AVOIDANCE
/*
 * E1: pixel dropping avoidance.  Do this before the main refresh on the line.
 *     Require a refresh where pixels may have been dropped into our
 *     area by a neighbour character which has now changed
 *     TODO: this could be integrated into E2 but might be too messy
 */
# ifdef NO_BOLDFONT
	wf = r->TermWin.font;
# endif
	for (col = 0; col < r->TermWin.ncol; col++) {
	    unsigned char   is_font_char, is_same_char;
	    text_t          t;

	    t = dtp[col];
	    is_same_char = (t == stp[col] && drp[col] == srp[col]);
	    if (!clear_next
		&& (is_same_char
		    || t == 0	/* screen cleared elsewhere */
		    || t == ' '))
		continue;

	    if (clear_next) {	/* previous char caused change here */
		clear_next = 0;
		dtp[col] = 0;
		if (is_same_char)	/* don't cascade into next char */
		    continue;
	    }
	    j = MONO_BOLD(drp[col]) ? 1 : 0;
# ifndef NO_BOLDFONT
	    wf = (j && r->TermWin.boldFont) ? r->TermWin.boldFont
					    : r->TermWin.font;
# endif
/*
 * TODO: consider if anything special needs to happen with:
 * #if defined(MULTICHAR_SET) && ! defined(NO_BOLDOVERSTRIKE_MULTI)
 */
	    is_font_char = (wf->per_char && IS_FONT_CHAR(wf, t)) ? 1 : 0;
	    if (!is_font_char || FONT_LBEAR(wf, t) < 0) {
		if (col == 0)
		    clearfirst = 1;
		else
		    dtp[col - 1] = 0;
	    }
	    if (!is_font_char
		|| (FONT_WIDTH(wf, t) < (FONT_RBEAR(wf, t) + j))) {
		if (col == r->TermWin.ncol - 1)
		    clearlast = 1;
		else
		    clear_next = 1;
	    }
	}
#endif				/* NO_PIXEL_DROPPING_AVOIDANCE */

/*
 * E2: OK, now the real pass
 */
	ypixelc = (int)Row2Pixel(row);
	ypixel = ypixelc + r->TermWin.font->ascent;

	for (col = 0; col < r->TermWin.ncol; col++) {
	    unsigned char   fontdiff,/* current font size != base font size */
			    fprop;   /* proportional font used              */
	    rend_t          rend;    /* rendition value                     */

	    /* compare new text with old - if exactly the same then continue */
	    rend = srp[col];	/* screen rendition (target rendtion) */
	    if (stp[col] == dtp[col]	/* Must match characters to skip. */
		&& (rend == drp[col]	/* Either rendition the same or   */
		    || (stp[col] == ' '	/* space w/ no background change  */
			&& GET_BGATTR(rend) == GET_BGATTR(drp[col])))) {
		if (!IS_MULTI1(rend))
		    continue;
#ifdef MULTICHAR_SET
		else {	/* first byte is Kanji so compare second bytes */
		    if (stp[col + 1] == dtp[col + 1]) {
		    /* assume no corrupt characters on the screen */
			col++;
			continue;
		    }
		}
#endif
	    }
	    /* redraw one or more characters */

	    fontdiff = 0;
	    len = 0;
	    buffer[len++] = dtp[col] = stp[col];
	    drp[col] = rend;
	    xpixel = Col2Pixel(col);

/*
 * Find out the longest string we can write out at once
 */
#ifndef NO_BOLDFONT
	    if (MONO_BOLD(rend) && r->TermWin.boldFont != NULL)
		fprop = (r->TermWin.propfont & PROPFONT_BOLD);
	    else
#endif
	    fprop = (r->TermWin.propfont & PROPFONT_NORMAL);
#ifdef MULTICHAR_SET
	    if (IS_MULTI1(rend)
		&& col < r->TermWin.ncol - 1 && IS_MULTI2(srp[col + 1])) {
		if (!wbyte && r->TermWin.mfont) {
		    wbyte = 1;
		    XSetFont(r->Xdisplay, r->TermWin.gc,
			     r->TermWin.mfont->fid);
		    fontdiff = (r->TermWin.propfont & PROPFONT_MULTI);
		    draw_string = XDrawString16;
		    draw_image_string = XDrawImageString16;
		}
		if (r->TermWin.mfont == NULL) {
		    buffer[0] = buffer[1] = ' ';
		    len = 2;
		    col++;
		} else {
		    /* double stepping - we're in multibyte font mode */
		    for (; ++col < r->TermWin.ncol;) {
			/* XXX: could check sanity on 2nd byte */
			dtp[col] = stp[col];
			drp[col] = srp[col];
			buffer[len++] = stp[col];
			col++;
			if (fprop)	/* proportional multibyte font mode */
			    break;
			if ((col == r->TermWin.ncol) || (srp[col] != rend))
			    break;
			if ((stp[col] == dtp[col])
			    && (srp[col] == drp[col])
			    && (stp[col + 1] == dtp[col + 1]))
			    break;
			if (len == h->currmaxcol)
			    break;
			dtp[col] = stp[col];
			drp[col] = srp[col];
			buffer[len++] = stp[col];
		    }
		    col--;
		}
		if (buffer[0] & 0x80)
		    (h->multichar_decode)(buffer, len);
		wlen = len / 2;
	    } else {
		if (rend & RS_multi1) {
		    /* corrupt character - you're outta there */
		    rend &= ~RS_multiMask;
		    drp[col] = rend;	/* TODO check: may also want */
		    dtp[col] = ' ';	/* to poke into stp/srp      */
		    buffer[0] = ' ';
		}
		if (wbyte) {
		    wbyte = 0;
		    XSetFont(r->Xdisplay, r->TermWin.gc, r->TermWin.font->fid);
		    draw_string = XDrawString;
		    draw_image_string = XDrawImageString;
		}
#else
	    {
#endif
		if (!fprop) {
		    /* single stepping - `normal' mode */
		    for (i = 0; ++col < r->TermWin.ncol - 1;) {
			if (rend != srp[col])
			    break;
			buffer[len++] = stp[col];
			if ((stp[col] != dtp[col]) || (srp[col] != drp[col])) {
			    if (must_clear && (i++ > (len / 2)))
				break;
			    dtp[col] = stp[col];
			    drp[col] = srp[col];
			    i = 0;
			} else if (must_clear || (stp[col] != ' ' && ++i > 32))
			    break;
		    }
		    col--;	/* went one too far.  move back */
		    len -= i;	/* dump any matching trailing chars */
		}
		wlen = len;
	    }
	    buffer[len] = '\0';

/*
 * Determine the attributes for the string
 */
	    fore = GET_FGCOLOR(rend);
	    back = GET_BGCOLOR(rend);
	    rend = GET_ATTR(rend);

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
			buffer[i] = 0x1e;	/* pound sign */
		break;
	    }

	    rvid = (rend & RS_RVid) ? 1 : 0;
#ifdef OPTION_HC
	    if (!rvid && (rend & RS_Blink)) {
		if (XDEPTH > 2 && ISSET_PIXCOLOR(h, Color_HC)
		    && r->PixColors[fore] != r->PixColors[Color_HC]
		    && r->PixColors[back] != r->PixColors[Color_HC])
		    back = Color_HC;
		else
		    rvid = !rvid;	/* fall back */
	    }
#endif
	    if (rvid) {
#ifdef TTY_256COLOR
		SWAP_IT(fore, back, u_int16_t);
#else
		SWAP_IT(fore, back, unsigned char);
#endif
#ifndef NO_BOLD_UNDERLINE_REVERSE
		if (XDEPTH > 2 && ISSET_PIXCOLOR(h, Color_RV)
		    && r->PixColors[fore] != r->PixColors[Color_RV])
		    back = Color_RV;
#endif
	    }
	    gcmask = 0;
	    if (back != Color_bg) {
		gcvalue.background = r->PixColors[back];
		gcmask = GCBackground;
	    }
	    if (fore != Color_fg) {
		gcvalue.foreground = r->PixColors[fore];
		gcmask |= GCForeground;
	    }
#ifndef NO_BOLD_UNDERLINE_REVERSE
	    else if (rend & RS_Bold) {
		if (XDEPTH > 2 && ISSET_PIXCOLOR(h, Color_BD)
		    && r->PixColors[fore] != r->PixColors[Color_BD]
		    && r->PixColors[back] != r->PixColors[Color_BD]) {
		    gcvalue.foreground = r->PixColors[Color_BD];
		    gcmask |= GCForeground;
# ifndef VERYBOLD
		    rend &= ~RS_Bold;	/* we've taken care of it */
# endif
		}
	    } else if (rend & RS_Uline) {
		if (XDEPTH > 2 && ISSET_PIXCOLOR(h, Color_UL)
		    && r->PixColors[fore] != r->PixColors[Color_UL]
		    && r->PixColors[back] != r->PixColors[Color_UL]) {
		    gcvalue.foreground = r->PixColors[Color_UL];
		    gcmask |= GCForeground;
		    rend &= ~RS_Uline;	/* we've taken care of it */
		}
	    }
#endif
	    if (gcmask)
		XChangeGC(r->Xdisplay, r->TermWin.gc, gcmask, &gcvalue);
#ifndef NO_BOLDFONT
	    if (!wbyte && MONO_BOLD_FG(rend, fore)
		&& r->TermWin.boldFont != NULL) {
		bfont = 1;
		XSetFont(r->Xdisplay, r->TermWin.gc, r->TermWin.boldFont->fid);
		fontdiff = (r->TermWin.propfont & PROPFONT_BOLD);
		rend &= ~RS_Bold;	/* we've taken care of it */
	    } else if (bfont) {
		bfont = 0;
		XSetFont(r->Xdisplay, r->TermWin.gc, r->TermWin.font->fid);
	    }
#endif
/*
 * Actually do the drawing of the string here
 */
	    if (back == Color_bg && must_clear) {
		CLEAR_CHARS(xpixel, ypixelc, len);
		for (i = 0; i < len; i++)	/* don't draw empty strings */
		    if (buffer[i] != ' ') {
			DRAW_STRING(draw_string, xpixel, ypixel, buffer, wlen);
			break;
		    }
	    } else if (fprop || fontdiff) {	/* single glyph writing */
		unsigned long   gctmp;

		gctmp = gcvalue.foreground;
		gcvalue.foreground = gcvalue.background;
		XChangeGC(r->Xdisplay, r->TermWin.gc, GCForeground, &gcvalue);
		XFillRectangle(r->Xdisplay, drawBuffer, r->TermWin.gc,
			       xpixel, ypixelc, (unsigned int)Width2Pixel(len),
			       (unsigned int)(Height2Pixel(1)
					      - r->TermWin.lineSpace));
		gcvalue.foreground = gctmp;
		XChangeGC(r->Xdisplay, r->TermWin.gc, GCForeground, &gcvalue);
		DRAW_STRING(draw_string, xpixel, ypixel, buffer, wlen);
	    } else
		DRAW_STRING(draw_image_string, xpixel, ypixel, buffer, wlen);

#ifndef NO_BOLDOVERSTRIKE
# ifdef NO_BOLDOVERSTRIKE_MULTI
	    if (!wbyte)
# endif
		if (MONO_BOLD_FG(rend, fore))
		    DRAW_STRING(draw_string, xpixel + 1, ypixel, buffer, wlen);
#endif
	    if ((rend & RS_Uline) && (r->TermWin.font->descent > 1))
		XDrawLine(r->Xdisplay, drawBuffer, r->TermWin.gc,
			  xpixel, ypixel + 1,
			  xpixel + Width2Pixel(len) - 1, ypixel + 1);
	    if (gcmask) {	/* restore normal colours */
		gcvalue.foreground = r->PixColors[Color_fg];
		gcvalue.background = r->PixColors[Color_bg];
		XChangeGC(r->Xdisplay, r->TermWin.gc, gcmask, &gcvalue);
	    }
	}			/* for (col....) */
    }				/* for (row....) */

/*
 * G: cleanup cursor and display outline cursor in necessary
 */
    if (r->screen.flags & Screen_VisibleCursor) {
	if (r->TermWin.focus) {
	    srp = &(r->screen.rend[r->screen.cur.row + r->TermWin.saveLines]
				  [r->screen.cur.col]);
	    *srp ^= RS_RVid;
#ifndef NO_CURSORCOLOR
	    *srp = (*srp & ~(RS_fgMask | RS_bgMask)) | cc1;
#endif
#ifdef MULTICHAR_SET
	    if (morecur) {
		srp += morecur;
		*srp ^= RS_RVid;
# ifndef NO_CURSORCOLOR
		*srp = (*srp & ~(RS_fgMask | RS_bgMask)) | cc2;
# endif
	    }
#endif
	} else if (h->oldcursor.row >= 0) {
#ifndef NO_CURSORCOLOR
	    unsigned long   gcmask;	/* Graphics Context mask */

	    gcmask = 0;
	    if (XDEPTH > 2 && ISSET_PIXCOLOR(h, Color_cursor)) {
		gcvalue.foreground = r->PixColors[Color_cursor];
		gcmask = GCForeground;
		XChangeGC(r->Xdisplay, r->TermWin.gc, gcmask, &gcvalue);
		gcvalue.foreground = r->PixColors[Color_fg];
	    }
#endif
	    XDrawRectangle(r->Xdisplay, drawBuffer, r->TermWin.gc,
			   Col2Pixel(h->oldcursor.col + morecur),
			   Row2Pixel(h->oldcursor.row),
			   (unsigned int)(Width2Pixel(1 + (morecur ? 1 : 0))
					  - 1),
			   (unsigned int)(Height2Pixel(1)
					  - r->TermWin.lineSpace - 1));
#ifndef NO_CURSORCOLOR
	    if (gcmask)		/* restore normal colours */
		XChangeGC(r->Xdisplay, r->TermWin.gc, gcmask, &gcvalue);
#endif
	}
    }
/*
 * H: cleanup selection
 */
    rxvt_scr_reverse_selection(r);

/*
 * I: other general cleanup
 */
    if (clearfirst)		/* clear the whole screen height */
	XClearArea(r->Xdisplay, r->TermWin.vt, 0, 0,
		   (unsigned int)r->TermWin.int_bwidth,
		   (unsigned int)TermWin_TotalHeight(), False);
    if (clearlast)		/* clear the whole screen height */
	XClearArea(r->Xdisplay, r->TermWin.vt,
		   r->TermWin.width + r->TermWin.int_bwidth, 0,
		   (unsigned int)r->TermWin.int_bwidth,
		   (unsigned int)TermWin_TotalHeight(), False);
    if (refresh_type & SMOOTH_REFRESH)
	XSync(r->Xdisplay, False);

    h->num_scr = 0;
    h->num_scr_allow = 1;
    h->want_refresh = 0;	/* screen is current */
}
/* ------------------------------------------------------------------------- */

/* EXTPROTO */
void
rxvt_scr_clear(rxvt_t *r)
{
    if (!r->TermWin.mapped)
	return;
    r->h->num_scr_allow = 0;
    r->h->want_refresh = 1;
#ifdef TRANSPARENT
    if ((r->Options & Opt_transparent) && (r->h->am_pixmap_trans == 0)) {
	int             i;

	if (!(r->Options & Opt_transparent_all))
	    i = 0;
	else
	    i = (int)(sizeof(r->TermWin.parent) / sizeof(Window));
	for (; i--;)
	    if (r->TermWin.parent[i] != None)
		XClearWindow(r->Xdisplay, r->TermWin.parent[i]);
    }
#endif
    XClearWindow(r->Xdisplay, r->TermWin.vt);
}

/* ------------------------------------------------------------------------- */
/* INTPROTO */
void
rxvt_scr_reverse_selection(rxvt_t *r)
{
    int             i, col, row, end_row;
    rend_t         *srp;

    if (r->selection.op && r->h->current_screen == r->selection.screen) {
	end_row = r->TermWin.saveLines - r->TermWin.view_start;
	i = r->selection.beg.row + r->TermWin.saveLines;
	row = r->selection.end.row + r->TermWin.saveLines;
	if (i >= end_row)
	    col = r->selection.beg.col;
	else {
	    col = 0;
	    i = end_row;
	}
	end_row += r->TermWin.nrow;
	for (; i < row && i < end_row; i++, col = 0)
	    for (srp = r->screen.rend[i]; col < r->TermWin.ncol; col++)
#ifndef OPTION_HC
		srp[col] ^= RS_RVid;
#else
		srp[col] ^= RS_Blink;
#endif
	if (i == row && i < end_row)
	    for (srp = r->screen.rend[i]; col < r->selection.end.col; col++)
#ifndef OPTION_HC
		srp[col] ^= RS_RVid;
#else
		srp[col] ^= RS_Blink;
#endif
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Dump the whole scrollback and screen to the passed filedescriptor.  The
 * invoking routine must close the fd.
 */
/* EXTPROTO */
void
rxvt_scr_dump(rxvt_t *r, int fd)
{
    int             row, wrote;
    unsigned int    width, towrite;
    char            r1[] = "\n";

    for (row = r->TermWin.saveLines - r->TermWin.nscrolled;
	 row < r->TermWin.saveLines + r->TermWin.nrow - 1; row++) {
	width = r->screen.tlen[row] >= 0 ? r->screen.tlen[row]
					 : r->TermWin.ncol;
	for (towrite = width; towrite; towrite -= wrote) {
	    wrote = write(fd, &(r->screen.text[row][width - towrite]),
			  towrite);
	    if (wrote < 0)
		return;		/* XXX: death, no report */
	}
	if (r->screen.tlen[row] >= 0)
	    if (write(fd, r1, 1) <= 0)
		return;	/* XXX: death, no report */
    }
}

/* ------------------------------------------------------------------------- *
 *                           CHARACTER SELECTION                             *
 * ------------------------------------------------------------------------- */

/*
 * -r->TermWin.nscrolled <= (selection row) <= r->TermWin.nrow - 1
 */
/* EXTPROTO */
void
rxvt_selection_check(rxvt_t *r, int check_more)
{
    row_col_t       pos;

    if (!r->selection.op)
	return;

    pos.row = pos.col = 0;
    if ((r->selection.beg.row < -(int32_t)r->TermWin.nscrolled)
	|| (r->selection.beg.row >= r->TermWin.nrow)
	|| (r->selection.mark.row < -(int32_t)r->TermWin.nscrolled)
	|| (r->selection.mark.row >= r->TermWin.nrow)
	|| (r->selection.end.row < -(int32_t)r->TermWin.nscrolled)
	|| (r->selection.end.row >= r->TermWin.nrow)
        || (check_more == 1
	    && r->h->current_screen == r->selection.screen
	    && !ROWCOL_IS_BEFORE(r->screen.cur, r->selection.beg)
	    && ROWCOL_IS_BEFORE(r->screen.cur, r->selection.end))
	|| (check_more == 2
	    && ROWCOL_IS_BEFORE(r->selection.beg, pos)
	    && ROWCOL_IS_AFTER(r->selection.end, pos))
	|| (check_more == 3
	    && ROWCOL_IS_AFTER(r->selection.end, pos))
	|| (check_more == 4	/* screen width change */
	    && (r->selection.beg.row != r->selection.end.row
		|| r->selection.end.col > r->TermWin.ncol)))
	CLEAR_SELECTION(r);
}

/* ------------------------------------------------------------------------- */
/*
 * Paste a selection direct to the command fd
 */
/* INTPROTO */
void
rxvt_PasteIt(rxvt_t *r, const unsigned char *data, unsigned int nitems)
{
    unsigned int    i, j, n;
    unsigned char  *ds = rxvt_malloc(PROP_SIZE);
    
/* convert normal newline chars into common keyboard Return key sequence */
    for (i = 0; i < nitems; i += PROP_SIZE) {
	n = min(nitems - i, PROP_SIZE);
	MEMCPY(ds, data + i, n);
	for (j = 0; j < n; j++)
	    if (ds[j] == '\n')
		ds[j] = '\r';
	rxvt_tt_write(r, ds, (int)n);
    }
    free(ds);
}

/* ------------------------------------------------------------------------- */
/*
 * Respond to a notification that a primary selection has been sent
 * EXT: SelectionNotify
 */
/* EXTPROTO */
int
rxvt_selection_paste(rxvt_t *r, Window win, Atom prop, Bool delete_prop)
{
    long            nread = 0;
    unsigned long   bytes_after;
    XTextProperty   ct;
#ifdef MULTICHAR_SET
    int             dummy_count;
    char          **cl;
#endif

    D_SELECT((stderr, "rxvt_selection_paste(%08lx, %lu, %d), wait=%2x", win, (unsigned long)prop, (int)delete_prop, r->h->selection_wait));

    if (prop == None) {		/* check for failed XConvertSelection */
#ifdef MULTICHAR_SET
	if ((r->h->selection_type & Sel_CompoundText)) {
	    int             selnum = r->h->selection_type & Sel_whereMask;

	    r->h->selection_type = 0;
	    if (selnum != Sel_direct)
		rxvt_selection_request_other(r, XA_STRING, selnum);
	}
#endif
	return 0;
    }
    for (;;) {
	if (XGetWindowProperty(r->Xdisplay, win, prop, (long)(nread / 4),
			       (long)(PROP_SIZE / 4), delete_prop,
			       AnyPropertyType, &ct.encoding, &ct.format,
			       &ct.nitems, &bytes_after,
			       &ct.value) != Success)
	    break;
	if (ct.encoding == 0) {
	    D_SELECT((stderr, "rxvt_selection_paste: property didn't exist!"));
	    break;
	}
	if (ct.value == NULL) {
	    D_SELECT((stderr, "rxvt_selection_paste: property shooting blanks!"));
	    continue;
	}
	if (ct.nitems == 0) {
	    D_SELECT((stderr, "rxvt_selection_paste: property empty - also INCR end"));
	    if (r->h->selection_wait == Sel_normal && nread == 0) {
	    /*
	     * pass through again trying CUT_BUFFER0 if we've come from
	     * XConvertSelection() but nothing was presented
	     */
		D_SELECT((stderr, "rxvt_selection_request: pasting CUT_BUFFER0"));
		rxvt_selection_paste(r, Xroot, XA_CUT_BUFFER0, False);
	    }
	    nread = -1;		/* discount any previous stuff */
	    break;
	}
	nread += ct.nitems;
#ifdef MULTICHAR_SET
	if (XmbTextPropertyToTextList(r->Xdisplay, &ct, &cl,
				      &dummy_count) == Success && cl) {
	    rxvt_PasteIt(r, cl[0], STRLEN(cl[0]));
	    XFreeStringList(cl);
	} else
#endif
	    rxvt_PasteIt(r, ct.value, (unsigned int)ct.nitems);
	if (bytes_after == 0)
	    break;
	XFree(ct.value);
    }
    if (ct.value)
	XFree(ct.value);
    if (r->h->selection_wait == Sel_normal)
	r->h->selection_wait = Sel_none;
    D_SELECT((stderr, "rxvt_selection_paste: bytes written: %ld", nread));
    return (int)nread;
}

/*
 * INCR support originally provided by Paul Sheer <psheer@obsidian.co.za>
 */
/* EXTPROTO */
void
rxvt_selection_property(rxvt_t *r, Window win, Atom prop)
{
    int             reget_time = 0;

    if (prop == None)
	return;
    D_SELECT((stderr, "rxvt_selection_property(%08lx, %lu)", win, (unsigned long)prop));
    if (r->h->selection_wait == Sel_normal) {
	int             a, afmt;
	Atom            atype;
	unsigned long   bytes_after, nitems;
	unsigned char  *s = NULL;

	a = XGetWindowProperty(r->Xdisplay, win, prop, 0L, 1L, False,
			       r->h->xa[XA_INCR], &atype, &afmt, &nitems,
			       &bytes_after, &s);
	if (s)
	    XFree(s);
	if (a != Success)
	    return;
#ifndef __CYGWIN32__
	if (atype == r->h->xa[XA_INCR]) {	/* start an INCR transfer */
	    D_SELECT((stderr, "rxvt_selection_property: INCR: starting transfer"));
	    XDeleteProperty(r->Xdisplay, win, prop);
	    XFlush(r->Xdisplay);
	    reget_time = 1;
	    r->h->selection_wait = Sel_incr;
	}
#endif
    } else if (r->h->selection_wait == Sel_incr) {
	reget_time = 1;
	if (rxvt_selection_paste(r, win, prop, True) == -1) {
	    D_SELECT((stderr, "rxvt_selection_property: INCR: clean end"));
	    r->h->selection_wait = Sel_none;
	    r->h->timeout[TIMEOUT_INCR].tv_sec = 0;	/* turn off timer */
	}
    }
    if (reget_time) {	/* received more data so reget time */
	(void)gettimeofday(&(r->h->timeout[TIMEOUT_INCR]), NULL);
	r->h->timeout[TIMEOUT_INCR].tv_sec += 10;	/* ten seconds wait */
    }
}
/* ------------------------------------------------------------------------- */
/*
 * Request the current selection: 
 * Order: > internal selection if available
 *        > PRIMARY, SECONDARY, CLIPBOARD if ownership is claimed (+)
 *        > CUT_BUFFER0
 * (+) if ownership is claimed but property is empty, rxvt_selection_paste()
 *     will auto fallback to CUT_BUFFER0
 * EXT: button 2 release
 */
/* EXTPROTO */
void
rxvt_selection_request(rxvt_t *r, Time tm, int x, int y)
{
    D_SELECT((stderr, "rxvt_selection_request(%lu, %d, %d)", tm, x, y));
    if (x < 0 || x >= r->TermWin.width || y < 0 || y >= r->TermWin.height)
	return;			/* outside window */

    if (r->selection.text != NULL) {	/* internal selection */
	D_SELECT((stderr, "rxvt_selection_request: pasting internal"));
	rxvt_PasteIt(r, r->selection.text, r->selection.len);
	return;
    } else {
	int             i;

	r->h->selection_request_time = tm;
	r->h->selection_wait = Sel_normal;
	for (i = Sel_Primary; i <= Sel_Clipboard; i++) {
#ifdef MULTICHAR_SET
	    r->h->selection_type = Sel_CompoundText;
#else
	    r->h->selection_type = 0;
#endif
	    if (rxvt_selection_request_other(r,
#ifdef MULTICHAR_SET
					     r->h->xa[XA_COMPOUND_TEXT],
#else
					     XA_STRING,
#endif
					     i))
		return;
	}
    }
    r->h->selection_wait = Sel_none;	/* don't loop in rxvt_selection_paste() */
    D_SELECT((stderr, "rxvt_selection_request: pasting CUT_BUFFER0"));
    rxvt_selection_paste(r, Xroot, XA_CUT_BUFFER0, False);
}

/* INTPROTO */
int
rxvt_selection_request_other(rxvt_t *r, Atom target, int selnum)
{
    Atom            sel;
#ifdef DEBUG_SELECT
    char           *debug_xa_names[] = { "PRIMARY", "SECONDARY", "CLIPBOARD" };
#endif

    r->h->selection_type |= selnum;
    if (selnum == Sel_Primary)
	sel = XA_PRIMARY;
    else if (selnum == Sel_Secondary)
	sel = XA_SECONDARY;
    else
	sel = r->h->xa[XA_CLIPBOARD];
    if (XGetSelectionOwner(r->Xdisplay, sel) != None) {
	D_SELECT((stderr, "rxvt_selection_request_other: pasting %s", debug_xa_names[selnum]));
	XConvertSelection(r->Xdisplay, sel, target, r->h->xa[XA_VT_SELECTION],
			  r->TermWin.vt, r->h->selection_request_time);
	return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
/*
 * Clear all selected text
 * EXT: SelectionClear
 */
/* EXTPROTO */
void
rxvt_selection_clear(rxvt_t *r)
{
    D_SELECT((stderr, "rxvt_selection_clear()"));

    r->h->want_refresh = 1;
    if (r->selection.text)
	free(r->selection.text);
    r->selection.text = NULL;
    r->selection.len = 0;
    CLEAR_SELECTION(r);
}

/* ------------------------------------------------------------------------- */
/*
 * Copy a selection into the cut buffer
 * EXT: button 1 or 3 release
 */
/* EXTPROTO */
void
rxvt_selection_make(rxvt_t *r, Time tm)
{
    int             i, col, end_col, row, end_row;
    unsigned char  *new_selection_text;
    char           *str;
    text_t         *t;

    D_SELECT((stderr, "rxvt_selection_make(): r->selection.op=%d, r->selection.clicks=%d", r->selection.op, r->selection.clicks));
    switch (r->selection.op) {
    case SELECTION_CONT:
	break;
    case SELECTION_INIT:
	CLEAR_SELECTION(r);
    /* FALLTHROUGH */
    case SELECTION_BEGIN:
	r->selection.op = SELECTION_DONE;
    /* FALLTHROUGH */
    default:
	return;
    }
    r->selection.op = SELECTION_DONE;

    if (r->selection.clicks == 4)
	return;			/* nothing selected, go away */

    i = (r->selection.end.row - r->selection.beg.row + 1)
	* (r->TermWin.ncol + 1) + 1;
    str = rxvt_malloc(i * sizeof(char));

    new_selection_text = (unsigned char *)str;

    col = r->selection.beg.col;
    MAX_IT(col, 0);
    row = r->selection.beg.row + r->TermWin.saveLines;
    end_row = r->selection.end.row + r->TermWin.saveLines;
/*
 * A: rows before end row
 */
    for (; row < end_row; row++, col = 0) {
	t = &(r->screen.text[row][col]);
	if ((end_col = r->screen.tlen[row]) == -1)
	    end_col = r->TermWin.ncol;
	for (; col < end_col; col++)
	    *str++ = *t++;
	if (r->screen.tlen[row] != -1)
	    *str++ = '\n';
    }
/*
 * B: end row
 */
    t = &(r->screen.text[row][col]);
    end_col = r->screen.tlen[row];
    if (end_col == -1 || r->selection.end.col <= end_col)
	end_col = r->selection.end.col;
    MIN_IT(end_col, r->TermWin.ncol);	/* CHANGE */
    for (; col < end_col; col++)
	*str++ = *t++;
#ifndef NO_OLD_SELECTION
    if (r->selection_style == OLD_SELECT)
	if (end_col == r->TermWin.ncol)
	    *str++ = '\n';
#endif
#ifndef NO_NEW_SELECTION
    if (r->selection_style != OLD_SELECT)
	if (end_col != r->selection.end.col)
	    *str++ = '\n';
#endif
    *str = '\0';
    if ((i = STRLEN((char *)new_selection_text)) == 0) {
	free(new_selection_text);
	return;
    }
    r->selection.len = i;
    if (r->selection.text)
	free(r->selection.text);
    r->selection.text = new_selection_text;

    XSetSelectionOwner(r->Xdisplay, XA_PRIMARY, r->TermWin.vt, tm);
    if (XGetSelectionOwner(r->Xdisplay, XA_PRIMARY) != r->TermWin.vt)
	rxvt_print_error("can't get primary selection");
    XChangeProperty(r->Xdisplay, Xroot, XA_CUT_BUFFER0, XA_STRING, 8,
		    PropModeReplace, r->selection.text, (int)r->selection.len);
    r->h->selection_time = tm;
    D_SELECT((stderr, "rxvt_selection_make(): r->selection.len=%d", r->selection.len));
}

/* ------------------------------------------------------------------------- */
/*
 * Mark or select text based upon number of clicks: 1, 2, or 3
 * EXT: button 1 press
 */
/* EXTPROTO */
void
rxvt_selection_click(rxvt_t *r, int clicks, int x, int y)
{
    D_SELECT((stderr, "rxvt_selection_click(%d, %d, %d)", clicks, x, y));

    clicks = ((clicks - 1) % 3) + 1;
    r->selection.clicks = clicks;	/* save clicks so extend will work */

    rxvt_selection_start_colrow(r, Pixel2Col(x), Pixel2Row(y));
    if (clicks == 2 || clicks == 3)
	rxvt_selection_extend_colrow(r, r->selection.mark.col,
				     r->selection.mark.row
				     + r->TermWin.view_start,
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
rxvt_selection_start_colrow(rxvt_t *r, int col, int row)
{
    r->h->want_refresh = 1;
    r->selection.mark.col = col;
    r->selection.mark.row = row - r->TermWin.view_start;
    MAX_IT(r->selection.mark.row, -(int32_t)r->TermWin.nscrolled);
    MIN_IT(r->selection.mark.row, r->TermWin.nrow - 1);
    MAX_IT(r->selection.mark.col, 0);
    MIN_IT(r->selection.mark.col, r->TermWin.ncol - 1);

    if (r->selection.op) {	/* clear the old selection */
	r->selection.beg.row = r->selection.end.row = r->selection.mark.row;
	r->selection.beg.col = r->selection.end.col = r->selection.mark.col;
    }
    r->selection.op = SELECTION_INIT;
    r->selection.screen = r->h->current_screen;
}

/* ------------------------------------------------------------------------- */
/*
 * Word select: select text for 2 clicks
 * We now only find out the boundary in one direction
 */

/* what do we want: spaces/tabs are delimiters or cutchars or non-cutchars */
#define DELIMIT_TEXT(x) \
    (((x) == ' ' || (x) == '\t') ? 2 : (STRCHR(r->h->rs[Rs_cutchars], (x)) != NULL))
#ifdef MULTICHAR_SET
# define DELIMIT_REND(x)	(((x) & RS_multiMask) ? 1 : 0)
#else
# define DELIMIT_REND(x)	1
#endif

/* INTPROTO */
void
rxvt_selection_delimit_word(rxvt_t *r, enum page_dirn dirn, const row_col_t *mark, row_col_t *ret)
{
    int             col, row, dirnadd, tcol, trow, w1, w2;
    row_col_t       bound;
    text_t         *stp;
    rend_t         *srp;

    if (dirn == UP) {
	bound.row = r->TermWin.saveLines - r->TermWin.nscrolled - 1;
	bound.col = 0;
	dirnadd = -1;
    } else {
	bound.row = r->TermWin.saveLines + r->TermWin.nrow;
	bound.col = r->TermWin.ncol - 1;
	dirnadd = 1;
    }
    row = mark->row + r->TermWin.saveLines;
    col = mark->col;
    MAX_IT(col, 0);
/* find the edge of a word */
    stp = &(r->screen.text[row][col]);
    w1 = DELIMIT_TEXT(*stp);

    if (r->selection_style != NEW_SELECT) {
	if (w1 == 1) {
	    stp += dirnadd;
	    if (DELIMIT_TEXT(*stp) == 1)
		goto Old_Word_Selection_You_Die;
	    col += dirnadd;
	}
	w1 = 0;
    }
    srp = (&r->screen.rend[row][col]);
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
	    if (r->screen.tlen[(row - (dirn == UP ? 1 : 0))] == -1) {
		trow = row + dirnadd;
		tcol = dirn == UP ? r->TermWin.ncol - 1 : 0;
		if (r->screen.text[trow] == NULL)
		    break;
		stp = &(r->screen.text[trow][tcol]);
		srp = &(r->screen.rend[trow][tcol]);
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
    D_SELECT((stderr, "rxvt_selection_delimit_word(%s,...) @ (r:%3d, c:%3d) has boundary (r:%3d, c:%3d)", (dirn == UP ? "up	" : "down"), mark->row, mark->col, row - r->TermWin.saveLines, col));

    if (dirn == DN)
	col++;			/* put us on one past the end */

/* Poke the values back in */
    ret->row = row - r->TermWin.saveLines;
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
rxvt_selection_extend(rxvt_t *r, int x, int y, int flag)
{
    int             col, row;

    col = Pixel2Col(x);
    row = Pixel2Row(y);
    MAX_IT(row, 0);
    MIN_IT(row, r->TermWin.nrow - 1);
    MAX_IT(col, 0);
    MIN_IT(col, r->TermWin.ncol);
#ifndef NO_NEW_SELECTION
/*
 * If we're selecting characters (single click) then we must check first
 * if we are at the same place as the original mark.  If we are then
 * select nothing.  Otherwise, if we're to the right of the mark, you have to
 * be _past_ a character for it to be selected.
 */
    if (r->selection_style != OLD_SELECT) {
	if (((r->selection.clicks % 3) == 1) && !flag
	    && (col == r->selection.mark.col
		&& (row == r->selection.mark.row + r->TermWin.view_start))) {
	    /* select nothing */
	    r->selection.beg.row = r->selection.end.row = 0;
	    r->selection.beg.col = r->selection.end.col = 0;
	    r->selection.clicks = 4;
	    r->h->want_refresh = 1;
	    D_SELECT((stderr, "rxvt_selection_extend() r->selection.clicks = 4"));
	    return;
	}
    }
#endif
    if (r->selection.clicks == 4)
	r->selection.clicks = 1;
    rxvt_selection_extend_colrow(r, col, row, !!flag,	/* ? button 3      */
				 flag == 1 ? 1 : 0,	/* ? button press  */
				 0);	/* no click change */
}

#ifdef MULTICHAR_SET
/* INTPROTO */
void
rxvt_selection_adjust_kanji(rxvt_t *r)
{
    int             c1, r1;

    if (r->selection.beg.col > 0) {
	r1 = r->selection.beg.row + r->TermWin.saveLines;
	c1 = r->selection.beg.col;
	if (IS_MULTI2(r->screen.rend[r1][c1])
	    && IS_MULTI1(r->screen.rend[r1][c1 - 1]))
	    r->selection.beg.col--;
    }
    if (r->selection.end.col < r->TermWin.ncol) {
	r1 = r->selection.end.row + r->TermWin.saveLines;
	c1 = r->selection.end.col;
	if (IS_MULTI1(r->screen.rend[r1][c1 - 1])
	    && IS_MULTI2(r->screen.rend[r1][c1]))
	    r->selection.end.col++;
    }
}
#endif				/* MULTICHAR_SET */

/* ------------------------------------------------------------------------- */
/*
 * Extend the selection to the specified col/row
 */
/* INTPROTO */
void
rxvt_selection_extend_colrow(rxvt_t *r, int32_t col, int32_t row, int button3, int buttonpress, int clickchange)
{
    int16_t         ncol = r->TermWin.ncol;
    int             end_col;
    row_col_t       pos;
    enum {
	LEFT, RIGHT
    } closeto = RIGHT;

    D_SELECT((stderr, "rxvt_selection_extend_colrow(c:%d, r:%d, %d, %d) clicks:%d, op:%d", col, row, button3, buttonpress, r->selection.clicks, r->selection.op));
    D_SELECT((stderr, "rxvt_selection_extend_colrow() ENT  b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", r->selection.beg.row, r->selection.beg.col, r->selection.mark.row, r->selection.mark.col, r->selection.end.row, r->selection.end.col));

    r->h->want_refresh = 1;
    switch (r->selection.op) {
    case SELECTION_INIT:
	CLEAR_SELECTION(r);
	r->selection.op = SELECTION_BEGIN;
    /* FALLTHROUGH */
    case SELECTION_BEGIN:
	if (row != r->selection.mark.row || col != r->selection.mark.col
	    || (!button3 && buttonpress))
	    r->selection.op = SELECTION_CONT;
	break;
    case SELECTION_DONE:
	r->selection.op = SELECTION_CONT;
    /* FALLTHROUGH */
    case SELECTION_CONT:
	break;
    case SELECTION_CLEAR:
	rxvt_selection_start_colrow(r, col, row);
    /* FALLTHROUGH */
    default:
	return;
    }
    if (r->selection.beg.col == r->selection.end.col
	&& r->selection.beg.col != r->selection.mark.col
	&& r->selection.beg.row == r->selection.end.row
	&& r->selection.beg.row != r->selection.mark.row) {
	r->selection.beg.col = r->selection.end.col = r->selection.mark.col;
	r->selection.beg.row = r->selection.end.row = r->selection.mark.row;
	D_SELECT((stderr, "rxvt_selection_extend_colrow() ENT2 b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", r->selection.beg.row, r->selection.beg.col, r->selection.mark.row, r->selection.mark.col, r->selection.end.row, r->selection.end.col));
    }

    pos.col = col;
    pos.row = row;

    pos.row -= r->TermWin.view_start;	/* adjust for scroll */

#ifndef NO_OLD_SELECTION
/*
 * This mimics some of the selection behaviour of version 2.20 and before.
 * There are no ``selection modes'', button3 is always character extension.
 * Note: button3 drag is always available, c.f. v2.20
 * Selection always terminates (left or right as appropriate) at the mark.
 */
    if (r->selection_style == OLD_SELECT) {
	if (r->selection.clicks == 1 || button3) {
	    if (r->h->hate_those_clicks) {
		r->h->hate_those_clicks = 0;
		if (r->selection.clicks == 1) {
		    r->selection.beg.row = r->selection.mark.row;
		    r->selection.beg.col = r->selection.mark.col;
		} else {
		    r->selection.mark.row = r->selection.beg.row;
		    r->selection.mark.col = r->selection.beg.col;
		}
	    }
	    if (ROWCOL_IS_BEFORE(pos, r->selection.mark)) {
		r->selection.end.row = r->selection.mark.row;
		r->selection.end.col = r->selection.mark.col + 1;
		r->selection.beg.row = pos.row;
		r->selection.beg.col = pos.col;
	    } else {
		r->selection.beg.row = r->selection.mark.row;
		r->selection.beg.col = r->selection.mark.col;
		r->selection.end.row = pos.row;
		r->selection.end.col = pos.col + 1;
	    }
# ifdef MULTICHAR_SET
	    rxvt_selection_adjust_kanji(r);
# endif				/* MULTICHAR_SET */
	} else if (r->selection.clicks == 2) {
	    rxvt_selection_delimit_word(r, UP, &(r->selection.mark),
					&(r->selection.beg));
	    rxvt_selection_delimit_word(r, DN, &(r->selection.mark),
					&(r->selection.end));
	    r->h->hate_those_clicks = 1;
	} else if (r->selection.clicks == 3) {
	    r->selection.beg.row = r->selection.end.row = r->selection.mark.row;
	    r->selection.beg.col = 0;
	    r->selection.end.col = ncol;
	    r->h->hate_those_clicks = 1;
	}
	D_SELECT((stderr, "rxvt_selection_extend_colrow() EXIT b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", r->selection.beg.row, r->selection.beg.col, r->selection.mark.row, r->selection.mark.col, r->selection.end.row, r->selection.end.col));
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
 *     is always at the left/right edge of the r->selection.
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
	if (ROWCOL_IS_BEFORE(pos, r->selection.beg)
	    || (!ROWCOL_IS_AFTER(pos, r->selection.end)
		&& (((pos.col - r->selection.beg.col)
		     + ((pos.row - r->selection.beg.row) * ncol))
		    < ((r->selection.end.col - pos.col)
		       + ((r->selection.end.row - pos.row) * ncol)))))
	     closeto = LEFT;
	if (closeto == LEFT) {
	    r->selection.beg.row = pos.row;
	    r->selection.beg.col = pos.col;
	    r->selection.mark.row = r->selection.end.row;
	    r->selection.mark.col = r->selection.end.col
				    - (r->selection.clicks == 2);
	} else {
	    r->selection.end.row = pos.row;
	    r->selection.end.col = pos.col;
	    r->selection.mark.row = r->selection.beg.row;
	    r->selection.mark.col = r->selection.beg.col;
	}
    } else {			/* button1 drag or button3 drag */
	if (ROWCOL_IS_AFTER(r->selection.mark, pos)) {
	    if ((r->selection.mark.row == r->selection.end.row)
		&& (r->selection.mark.col == r->selection.end.col)
		&& clickchange && r->selection.clicks == 2)
		r->selection.mark.col--;
	    r->selection.beg.row = pos.row;
	    r->selection.beg.col = pos.col;
	    r->selection.end.row = r->selection.mark.row;
	    r->selection.end.col = r->selection.mark.col
				   + (r->selection.clicks == 2);
	} else {
	    r->selection.beg.row = r->selection.mark.row;
	    r->selection.beg.col = r->selection.mark.col;
	    r->selection.end.row = pos.row;
	    r->selection.end.col = pos.col;
	}
    }

    if (r->selection.clicks == 1) {
	end_col = r->screen.tlen[r->selection.beg.row + r->TermWin.saveLines];
	if (end_col != -1 && r->selection.beg.col > end_col) {
#if 1
	    r->selection.beg.col = ncol;
#else
	    if (r->selection.beg.row != r->selection.end.row)
		r->selection.beg.col = ncol;
	    else
		r->selection.beg.col = r->selection.mark.col;
#endif
	}
	end_col = r->screen.tlen[r->selection.end.row + r->TermWin.saveLines];
	if (end_col != -1 && r->selection.end.col > end_col)
	    r->selection.end.col = ncol;

# ifdef MULTICHAR_SET
	rxvt_selection_adjust_kanji(r);
# endif				/* MULTICHAR_SET */
    } else if (r->selection.clicks == 2) {
	if (ROWCOL_IS_AFTER(r->selection.end, r->selection.beg))
	    r->selection.end.col--;
	rxvt_selection_delimit_word(r, UP, &(r->selection.beg),
				    &(r->selection.beg));
	rxvt_selection_delimit_word(r, DN, &(r->selection.end),
				    &(r->selection.end));
    } else if (r->selection.clicks == 3) {
#ifndef NO_FRILLS
	if ((r->Options & Opt_tripleclickwords)) {
	    int             end_row;

	    rxvt_selection_delimit_word(r, UP, &(r->selection.beg),
					&(r->selection.beg));
	    end_row = r->screen.tlen[r->selection.mark.row
				     + r->TermWin.saveLines];
	    for (end_row = r->selection.mark.row; end_row < r->TermWin.nrow;
		 end_row++) {
		end_col = r->screen.tlen[end_row + r->TermWin.saveLines];
		if (end_col != -1) {
		    r->selection.end.row = end_row;
		    r->selection.end.col = end_col;
		    rxvt_selection_remove_trailing_spaces(r);
		    break;
		}
	    }
	} else
#endif
	{
	    if (ROWCOL_IS_AFTER(r->selection.mark, r->selection.beg))
		r->selection.mark.col++;
	    r->selection.beg.col = 0;
	    r->selection.end.col = ncol;
	}
    }
    if (button3 && buttonpress) {	/* mark may need to be changed */
	if (closeto == LEFT) {
	    r->selection.mark.row = r->selection.end.row;
	    r->selection.mark.col = r->selection.end.col
				    - (r->selection.clicks == 2);
	} else {
	    r->selection.mark.row = r->selection.beg.row;
	    r->selection.mark.col = r->selection.beg.col;
	}
    }
    D_SELECT((stderr, "rxvt_selection_extend_colrow() EXIT b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", r->selection.beg.row, r->selection.beg.col, r->selection.mark.row, r->selection.mark.col, r->selection.end.row, r->selection.end.col));
#endif				/* ! NO_NEW_SELECTION */
}

#ifndef NO_FRILLS
/* INTPROTO */
void
rxvt_selection_remove_trailing_spaces(rxvt_t *r)
{
    int32_t         end_col, end_row;
    text_t         *stp; 

    end_col = r->selection.end.col;
    end_row = r->selection.end.row;
    for ( ; end_row >= r->selection.beg.row; ) {
	stp = r->screen.text[end_row + r->TermWin.saveLines];
	while (--end_col >= 0) {
	    if (stp[end_col] != ' ' && stp[end_col] != '\t')
		break;
	}
	if (end_col >= 0
	    || r->screen.tlen[end_row - 1 + r->TermWin.saveLines] != -1) {
	    r->selection.end.col = end_col + 1;
	    r->selection.end.row = end_row;
	    break;
	}
	end_row--;
	end_col = r->TermWin.ncol;
    }
    if (r->selection.mark.row > r->selection.end.row) {
	r->selection.mark.row = r->selection.end.row;
	r->selection.mark.col = r->selection.end.col;
    } else if (r->selection.mark.row == r->selection.end.row
	       && r->selection.mark.col > r->selection.end.col)
	r->selection.mark.col = r->selection.end.col;
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Double click on button 3 when already selected
 * EXT: button 3 double click
 */
/* EXTPROTO */
void
rxvt_selection_rotate(rxvt_t *r, int x, int y)
{
    r->selection.clicks = r->selection.clicks % 3 + 1;
    rxvt_selection_extend_colrow(r, Pixel2Col(x), Pixel2Row(y), 1, 0, 1);
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
rxvt_selection_send(rxvt_t *r, const XSelectionRequestEvent *rq)
{
    XSelectionEvent ev;
#ifdef USE_XIM
    Atom32          target_list[4];
#else
    Atom32          target_list[3];
#endif
    Atom            target;
    XTextProperty   ct;
    XICCEncodingStyle style;
    char           *cl[2], dummy[1];

    ev.type = SelectionNotify;
    ev.property = None;
    ev.display = rq->display;
    ev.requestor = rq->requestor;
    ev.selection = rq->selection;
    ev.target = rq->target;
    ev.time = rq->time;

    if (rq->target == r->h->xa[XA_TARGETS]) {
	target_list[0] = (Atom32) r->h->xa[XA_TARGETS];
	target_list[1] = (Atom32) XA_STRING;
	target_list[2] = (Atom32) r->h->xa[XA_TEXT];
#ifdef USE_XIM
	target_list[3] = (Atom32) r->h->xa[XA_COMPOUND_TEXT];
#endif
	XChangeProperty(r->Xdisplay, rq->requestor, rq->property, XA_ATOM,
			(8 * sizeof(target_list[0])), PropModeReplace,
			(unsigned char *)target_list,
			(sizeof(target_list) / sizeof(target_list[0])));
	ev.property = rq->property;
    } else if (rq->target == r->h->xa[XA_MULTIPLE]) {
	/* TODO: Handle MULTIPLE */
    } else if (rq->target == r->h->xa[XA_TIMESTAMP] && r->selection.text) {
	XChangeProperty(r->Xdisplay, rq->requestor, rq->property, XA_INTEGER,
			(8 * sizeof(Time)), PropModeReplace,
			(unsigned char *)&r->h->selection_time, 1);
	ev.property = rq->property;
    } else if (rq->target == XA_STRING
	       || rq->target == r->h->xa[XA_COMPOUND_TEXT]
	       || rq->target == r->h->xa[XA_TEXT]) {
#ifdef USE_XIM
	short           freect = 0;
#endif
	int             selectlen;

#ifdef USE_XIM
	if (rq->target != XA_STRING) {
	    target = r->h->xa[XA_COMPOUND_TEXT];
	    style = (rq->target == r->h->xa[XA_COMPOUND_TEXT])
		    ? XCompoundTextStyle : XStdICCTextStyle;
	} else
#endif
	{
	    target = XA_STRING;
	    style = XStringStyle;
	}
	if (r->selection.text) {
	    cl[0] = (char *)r->selection.text;
	    selectlen = r->selection.len;
	} else {
	    cl[0] = dummy;
	    *dummy = '\0';
	    selectlen = 0;
	}
#ifdef USE_XIM
	if (XmbTextListToTextProperty(r->Xdisplay, cl, 1, style, &ct)
	    == Success)		/* if we failed to convert then send it raw */
	    freect = 1;
	else
#endif
	{
	    ct.value = (unsigned char *)cl[0];
	    ct.nitems = selectlen;
	}
	XChangeProperty(r->Xdisplay, rq->requestor, rq->property,
			target, 8, PropModeReplace,
			ct.value, (int)ct.nitems);
	ev.property = rq->property;
#ifdef USE_XIM
	if (freect)
	    XFree(ct.value);
#endif
    }
    XSendEvent(r->Xdisplay, rq->requestor, False, 0L, (XEvent *)&ev);
}

/* ------------------------------------------------------------------------- *
 *                              MOUSE ROUTINES                               *
 * ------------------------------------------------------------------------- */

/*
 * return col/row values corresponding to x/y pixel values
 */
/* EXTPROTO */
void
rxvt_pixel_position(rxvt_t *r, int *x, int *y)
{
    *x = Pixel2Col(*x);
/* MAX_IT(*x, 0); MIN_IT(*x, r->TermWin.ncol - 1); */
    *y = Pixel2Row(*y);
/* MAX_IT(*y, 0); MIN_IT(*y, r->TermWin.nrow - 1); */
}
/* ------------------------------------------------------------------------- */
#ifdef USE_XIM
/* EXTPROTO */
void
rxvt_setPosition(rxvt_t *r, XPoint *pos)
{
    XWindowAttributes xwa;

    XGetWindowAttributes(r->Xdisplay, r->TermWin.vt, &xwa);
    pos->x = Col2Pixel(r->screen.cur.col) + xwa.x;
    pos->y = Height2Pixel((r->screen.cur.row + 1)) + xwa.y
	     - r->TermWin.lineSpace;
}
#endif
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- *
 *                              DEBUG ROUTINES                               *
 * ------------------------------------------------------------------------- */
#if 0
/* INTPROTO */
void
rxvt_debug_colors(void)
{
    int             color;
    const char     *name[] = {
	"fg", "bg",
	"black", "red", "green", "yellow", "blue", "magenta", "cyan", "white"
    };

    fprintf(stderr, "Color ( ");
    if (r->h->rstyle & RS_RVid)
	fprintf(stderr, "rvid ");
    if (r->h->rstyle & RS_Bold)
	fprintf(stderr, "bold ");
    if (r->h->rstyle & RS_Blink)
	fprintf(stderr, "blink ");
    if (r->h->rstyle & RS_Uline)
	fprintf(stderr, "uline ");
    fprintf(stderr, "): ");

    color = GET_FGCOLOR(r->h->rstyle);
#ifndef NO_BRIGHTCOLOR
    if (color >= minBrightCOLOR && color <= maxBrightCOLOR) {
	color -= (minBrightCOLOR - minCOLOR);
	fprintf(stderr, "bright ");
    }
#endif
    fprintf(stderr, "%s on ", name[color]);

    color = GET_BGCOLOR(r->h->rstyle);
#ifndef NO_BRIGHTCOLOR
    if (color >= minBrightCOLOR && color <= maxBrightCOLOR) {
	color -= (minBrightCOLOR - minCOLOR);
	fprintf(stderr, "bright ");
    }
#endif
    fprintf(stderr, "%s\n", name[color]);
}
#endif
