/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar.c
 *----------------------------------------------------------------------*
 * $Id: scrollbar.c,v 1.1 2003-03-05 17:33:37 earnie Exp $
 *
 * Copyright (c) 1997,1998 mj olesen <olesen@me.QueensU.CA>
 * Copyright (c) 1998      Alfredo K. Kojima <kojima@windowmaker.org>
 *				- N*XTstep like scrollbars
 * Copyright (c) 1999-2001 Geoff Wing <gcw@pobox.com>
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
 *----------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "scrollbar.intpro"	/* PROTOS for internal routines */

/*----------------------------------------------------------------------*/

/*
 * Map or unmap a scrollbar.  Returns non-zero upon change of state
 */
/* EXTPROTO */
int
rxvt_scrollbar_mapping(rxvt_t *r, int map)
{
    int             change = 0;
#ifdef HAVE_SCROLLBARS

    if (map && !scrollbar_visible(r)) {
	scrollbar_setIdle();
	if (!r->scrollBar.win)
	    rxvt_Resize_scrollBar(r);
	if (r->scrollBar.win) {
	    XMapWindow(r->Xdisplay, r->scrollBar.win);
	    change = 1;
	}
    } else if (!map && scrollbar_visible(r)) {
	r->scrollBar.state = 0;
	XUnmapWindow(r->Xdisplay, r->scrollBar.win);
	change = 1;
    }
#endif
    return change;
}

/* EXTPROTO */
void
rxvt_Resize_scrollBar(rxvt_t *r)
{
#ifdef HAVE_SCROLLBARS
    int             delayed_init = 0;

#define R_SCROLLBEG_XTERM	0
#define R_SCROLLEND_XTERM	r->szHint.height
#define R_SCROLLBEG_NEXT	0
#define R_SCROLLEND_NEXT	r->szHint.height - (SB_BUTTON_TOTAL_HEIGHT + \
						    SB_PADDING)
#define R_SCROLLBEG_RXVT	(r->scrollBar.width + 1) + r->sb_shadow
#define R_SCROLLEND_RXVT	r->szHint.height - R_SCROLLBEG_RXVT - \
				    (2 * r->sb_shadow)

#if defined(XTERM_SCROLLBAR)
    if (r->scrollBar.style == R_SB_XTERM) {
	r->scrollBar.beg = R_SCROLLBEG_XTERM;
	r->scrollBar.end = R_SCROLLEND_XTERM;
	r->scrollBar.update = rxvt_scrollbar_show_xterm;
    }
#endif
#if defined(NEXT_SCROLLBAR)
    if (r->scrollBar.style == R_SB_NEXT) {
	r->scrollBar.beg = R_SCROLLBEG_NEXT;
	r->scrollBar.end = R_SCROLLEND_NEXT;
	r->scrollBar.update = rxvt_scrollbar_show_next;
    }
#endif
#if defined(RXVT_SCROLLBAR)
    if (r->scrollBar.style == R_SB_RXVT) {
	r->scrollBar.beg = R_SCROLLBEG_RXVT;
	r->scrollBar.end = R_SCROLLEND_RXVT;
	r->scrollBar.update = rxvt_scrollbar_show_rxvt;
    }
#endif

    if (!r->scrollBar.win) {
/* create the scrollbar window */
	r->scrollBar.win = XCreateSimpleWindow(r->Xdisplay,
					       r->TermWin.parent[0],
					       r->h->window_sb_x, 0,
					       scrollbar_TotalWidth(),
					       r->szHint.height,
					       0,
					       r->PixColors[Color_fg],
					       r->PixColors[Color_bg]);
#ifdef DEBUG_X
	XStoreName(r->Xdisplay, r->scrollBar.win, "scrollbar");
#endif
	XDefineCursor(r->Xdisplay, r->scrollBar.win, r->h->cursor_leftptr);
	XSelectInput(r->Xdisplay, r->scrollBar.win,
		     (ExposureMask | ButtonPressMask | ButtonReleaseMask
		      | Button1MotionMask | Button2MotionMask
		      | Button3MotionMask));
	delayed_init = 1;
    }
    rxvt_scrollbar_show(r, 1);
    if (delayed_init)
	XMapWindow(r->Xdisplay, r->scrollBar.win);
#endif
}

/*
 * Update current scrollbar view w.r.t. slider heights, etc.
 */
/* EXTPROTO */
int
rxvt_scrollbar_show(rxvt_t *r, int update)
{
    int             ret = 0;
#ifdef HAVE_SCROLLBARS
    int             top, bot, len, adj;

    if (!scrollbar_visible(r))
	return 0;

    if (update) {
	top = (r->TermWin.nscrolled - r->TermWin.view_start);
	bot = top + (r->TermWin.nrow - 1);
	len = max((r->TermWin.nscrolled + (r->TermWin.nrow - 1)), 1);
	adj = (((bot - top) * scrollbar_size()) % len) > 0 ? 1 : 0;

	r->scrollBar.top = (r->scrollBar.beg + (top * scrollbar_size()) / len);
	r->h->scrollbar_len = ((bot - top) * scrollbar_size()) / len +
			      scrollbar_minheight() + adj;
	r->scrollBar.bot = (r->scrollBar.top + r->h->scrollbar_len);
	/* no change */
	if (r->scrollBar.top == r->h->last_top
	    && r->scrollBar.bot == r->h->last_bot
	    && (r->scrollBar.state == r->h->last_state || !scrollbar_isUpDn()))
	    return 0;
    }

    ret = r->scrollBar.update(r, update, r->h->last_top, r->h->last_bot,
			      r->h->scrollbar_len);

    r->h->last_top = r->scrollBar.top;
    r->h->last_bot = r->scrollBar.bot;
    r->h->last_state = r->scrollBar.state;

#endif
    return ret;
}

/* EXTPROTO */
void
rxvt_setup_scrollbar(rxvt_t *r, const char *scrollalign, const char *scrollstyle, const char *thickness)
{
#ifdef HAVE_SCROLLBARS
    int             i;
    short           style, width;

# if defined(RXVT_SCROLLBAR) || !(defined(NEXT_SCROLLBAR) || defined(XTERM_SCROLLBAR))
    style = R_SB_RXVT;
# else
#  ifdef NEXT_SCROLLBAR
    style = R_SB_NEXT;
#  elif defined(XTERM_SCROLLBAR)
    style = R_SB_XTERM;
#  endif
# endif

# if (defined(NEXT_SCROLLBAR) || defined(XTERM_SCROLLBAR))
    if (scrollstyle) {
#  ifdef NEXT_SCROLLBAR
	if (STRNCASECMP(scrollstyle, "next", 4) == 0)
	    style = R_SB_NEXT;
#  endif
#  ifdef XTERM_SCROLLBAR
	if (STRNCASECMP(scrollstyle, "xterm", 5) == 0)
	    style = R_SB_XTERM;
#  endif
    }
# endif
    if (style == R_SB_NEXT)
	width = SB_WIDTH_NEXT;
    else if (style == R_SB_XTERM)
	width = SB_WIDTH_XTERM;
    else /* if (style == R_SB_RXVT) */
	width = SB_WIDTH_RXVT;

    if (style != R_SB_NEXT)	/* dishonour request - for now */
    if (thickness && (i = atoi(thickness)) >= SB_WIDTH_MINIMUM)
	width = min(i, SB_WIDTH_MAXIMUM);

# if defined(RXVT_SCROLLBAR)
    if (!(r->Options & Opt_scrollBar_floating) && style == R_SB_RXVT)
	r->sb_shadow = SHADOW;
# endif

    r->scrollBar.style = style;
    r->scrollBar.width = width;

    /* r->h->scrollbar_align = R_SB_ALIGN_CENTRE; */
    if (scrollalign) {
	if (STRNCASECMP(scrollalign, "top", 3) == 0)
	    r->h->scrollbar_align = R_SB_ALIGN_TOP;
	else if (STRNCASECMP(scrollalign, "bottom", 6) == 0)
	    r->h->scrollbar_align = R_SB_ALIGN_BOTTOM;
    }
#endif
}

/*----------------------- end-of-file (C source) -----------------------*/
