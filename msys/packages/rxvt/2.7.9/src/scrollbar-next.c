/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar-next.c
 *----------------------------------------------------------------------*
 * $Id: scrollbar-next.c,v 1.1 2003/03/05 17:33:37 earnie Exp $
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
#include "scrollbar-next.intpro"	/* PROTOS for internal routines */

/*----------------------------------------------------------------------*/
#define n_stp_width	8
#define n_stp_height	2
const unsigned char n_stp_bits[] = { 0x55, 0xaa };

/*
 * N*XTSTEP like scrollbar - written by Alfredo K. Kojima
 */
#define SCROLLER_DIMPLE_WIDTH   6
#define SCROLLER_DIMPLE_HEIGHT  6
#define ARROW_WIDTH   13
#define ARROW_HEIGHT  13

const char     *const SCROLLER_DIMPLE[] = {
    ".%###.",
    "%#%%%%",
    "#%%...",
    "#%..  ",
    "#%.   ",
    ".%.  ."
};
const char     *const SCROLLER_ARROW_UP[] = {
    ".............",
    ".............",
    "......%......",
    "......#......",
    ".....%#%.....",
    ".....###.....",
    "....%###%....",
    "....#####....",
    "...%#####%...",
    "...#######...",
    "..%#######%..",
    ".............",
    "............."
};
const char     *const SCROLLER_ARROW_DOWN[] = {
    ".............",
    ".............",
    "..%#######%..",
    "...#######...",
    "...%#####%...",
    "....#####....",
    "....%###%....",
    ".....###.....",
    ".....%#%.....",
    "......#......",
    "......%......",
    ".............",
    "............."
};
const char     *const HI_SCROLLER_ARROW_UP[] = {
    "             ",
    "             ",
    "      %      ",
    "      %      ",
    "     %%%     ",
    "     %%%     ",
    "    %%%%%    ",
    "    %%%%%    ",
    "   %%%%%%%   ",
    "   %%%%%%%   ",
    "  %%%%%%%%%  ",
    "             ",
    "             "
};
const char     *const HI_SCROLLER_ARROW_DOWN[] = {
    "             ",
    "             ",
    "  %%%%%%%%%  ",
    "   %%%%%%%   ",
    "   %%%%%%%   ",
    "    %%%%%    ",
    "    %%%%%    ",
    "     %%%     ",
    "     %%%     ",
    "      %      ",
    "      %      ",
    "             ",
    "             "
};

/* INTPROTO */
Pixmap
rxvt_renderPixmap(rxvt_t *r, const char *const *data, int width, int height)
{
    char            a;
    int             x, y;
    Pixmap          d;
    GC              pointcolour;

    d = XCreatePixmap(r->Xdisplay, r->scrollBar.win, width, height, XDEPTH);

    for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {
	    if ((a = data[y][x]) == ' ' || a == 'w')
		pointcolour = r->h->whiteGC;
	    else if (a == '.' || a == 'l')
		pointcolour = r->h->grayGC;
	    else if (a == '%' || a == 'd')
		pointcolour = r->h->darkGC;
	    else		/* if (a == '#' || a == 'b' || a) */
		pointcolour = r->h->blackGC;
	    XDrawPoint(r->Xdisplay, d, pointcolour, x, y);
	}
    }
    return d;
}

/* INTPROTO */
void
rxvt_init_scrollbar_stuff(rxvt_t *r)
{
    XGCValues       gcvalue;
    XColor          xcol;
    Pixmap          stipple;
    unsigned long   light, dark;

    gcvalue.graphics_exposures = False;

    gcvalue.foreground = r->PixColors[Color_Black];
    r->h->blackGC = XCreateGC(r->Xdisplay, r->scrollBar.win,
			      GCForeground | GCGraphicsExposures, &gcvalue);

    gcvalue.foreground = r->PixColors[Color_White];
    r->h->whiteGC = XCreateGC(r->Xdisplay, r->scrollBar.win,
			      GCForeground | GCGraphicsExposures, &gcvalue);

    xcol.red = 0xaeba;
    xcol.green = 0xaaaa;
    xcol.blue = 0xaeba;
    if (!rxvt_rXAllocColor(r, &xcol, "light gray"))
	xcol.pixel = r->PixColors[Color_AntiqueWhite];
    light = gcvalue.foreground = xcol.pixel;
    r->h->grayGC = XCreateGC(r->Xdisplay, r->scrollBar.win,
			     GCForeground | GCGraphicsExposures, &gcvalue);

    xcol.red = 0x51aa;
    xcol.green = 0x5555;
    xcol.blue = 0x5144;
    if (!rxvt_rXAllocColor(r, &xcol, "dark gray"))
	xcol.pixel = r->PixColors[Color_Grey25];
    dark = gcvalue.foreground = xcol.pixel;
    r->h->darkGC = XCreateGC(r->Xdisplay, r->scrollBar.win,
			     GCForeground | GCGraphicsExposures, &gcvalue);

    stipple = XCreateBitmapFromData(r->Xdisplay, r->scrollBar.win,
				    (char *)n_stp_bits, n_stp_width,
				    n_stp_height);

    gcvalue.foreground = dark;
    gcvalue.background = light;
    gcvalue.fill_style = FillOpaqueStippled;
    gcvalue.stipple = stipple;

/*    XSetWindowBackground(r->Xdisplay, r->scrollBar.win, r->PixColors[Color_Red]); */

    r->h->stippleGC = XCreateGC(r->Xdisplay, r->scrollBar.win,
				GCForeground | GCBackground | GCStipple
				| GCFillStyle | GCGraphicsExposures, &gcvalue);

    r->h->dimple = rxvt_renderPixmap(r, SCROLLER_DIMPLE, SCROLLER_DIMPLE_WIDTH,
				     SCROLLER_DIMPLE_HEIGHT);

    r->h->upArrow = rxvt_renderPixmap(r, SCROLLER_ARROW_UP, ARROW_WIDTH,
				      ARROW_HEIGHT);
    r->h->downArrow = rxvt_renderPixmap(r, SCROLLER_ARROW_DOWN, ARROW_WIDTH,
					ARROW_HEIGHT);
    r->h->upArrowHi = rxvt_renderPixmap(r, HI_SCROLLER_ARROW_UP, ARROW_WIDTH,
					ARROW_HEIGHT);
    r->h->downArrowHi = rxvt_renderPixmap(r, HI_SCROLLER_ARROW_DOWN,
					  ARROW_WIDTH, ARROW_HEIGHT);
}

/* Draw bevel & arrows */
/* INTPROTO */
void
rxvt_drawBevel(rxvt_t *r, Drawable d, int x1, int y1, int w, int h)
{
    int             x2, y2;

    x2 = x1 + w - 1;		/* right  point */
    y2 = y1 + h - 1;		/* bottom point */
/* white top and left */
    XDrawLine(r->Xdisplay, d, r->h->whiteGC, x1, y1, x2, y1);
    XDrawLine(r->Xdisplay, d, r->h->whiteGC, x1, y1, x1, y2);
/* black bottom and right */
    XDrawLine(r->Xdisplay, d, r->h->blackGC, x1, y2, x2, y2);
    XDrawLine(r->Xdisplay, d, r->h->blackGC, x2, y1, x2, y2);
/* dark inside bottom and right */
    x1++, y1++, x2--, y2--;	/* move in one point */
    XDrawLine(r->Xdisplay, d, r->h->darkGC, x1, y2, x2, y2);
    XDrawLine(r->Xdisplay, d, r->h->darkGC, x2, y1, x2, y2);
}

/* EXTPROTO */
int
rxvt_scrollbar_show_next(rxvt_t *r, int update, int last_top, int last_bot, int scrollbar_len)
{
    int             height = r->scrollBar.end + SB_BUTTON_TOTAL_HEIGHT + SB_PADDING;
    Drawable        s;

    if ((r->scrollBar.init & R_SB_NEXT) == 0) {
	r->scrollBar.init |= R_SB_NEXT;
	rxvt_init_scrollbar_stuff(r);
    }

    if (r->TermWin.nscrolled == 0 || !update) {
	XFillRectangle(r->Xdisplay, r->scrollBar.win, r->h->grayGC, 0, 0,
		       SB_WIDTH_NEXT + 1, height);
	XDrawRectangle(r->Xdisplay, r->scrollBar.win, r->h->blackGC, 0,
		       -SB_BORDER_WIDTH, SB_WIDTH_NEXT,
		       height + SB_BORDER_WIDTH);
	XFillRectangle(r->Xdisplay, r->scrollBar.win, r->h->stippleGC,
		       SB_LEFT_PADDING, 0, SB_BUTTON_WIDTH, height);
    }
    if (r->TermWin.nscrolled) {
	if (last_top < r->scrollBar.top || !update)
	    XFillRectangle(r->Xdisplay, r->scrollBar.win, r->h->stippleGC,
			   SB_LEFT_PADDING, SB_PADDING + last_top,
			   SB_BUTTON_WIDTH, r->scrollBar.top - last_top);
	if (r->scrollBar.bot < last_bot || !update)
	    XFillRectangle(r->Xdisplay, r->scrollBar.win, r->h->stippleGC,
			   SB_LEFT_PADDING, r->scrollBar.bot + SB_PADDING,
			   SB_BUTTON_WIDTH, (last_bot - r->scrollBar.bot));
	XFillRectangle(r->Xdisplay, r->scrollBar.win, r->h->grayGC,
		       SB_LEFT_PADDING, r->scrollBar.top + SB_PADDING,
		       SB_BUTTON_WIDTH, scrollbar_len);
	XCopyArea(r->Xdisplay, r->h->dimple, r->scrollBar.win, r->h->whiteGC, 0, 0,
		  SCROLLER_DIMPLE_WIDTH, SCROLLER_DIMPLE_HEIGHT,
		  (SB_WIDTH_NEXT - SCROLLER_DIMPLE_WIDTH) / 2,
		  r->scrollBar.top + SB_BEVEL_WIDTH_UPPER_LEFT +
		  (scrollbar_len - SCROLLER_DIMPLE_HEIGHT) / 2);

	rxvt_drawBevel(r, r->scrollBar.win, SB_BUTTON_BEVEL_X,
		       r->scrollBar.top + SB_PADDING, SB_BUTTON_WIDTH,
		       scrollbar_len);
	rxvt_drawBevel(r, r->scrollBar.win, SB_BUTTON_BEVEL_X,
		       height - SB_BUTTON_BOTH_HEIGHT, SB_BUTTON_WIDTH,
		       SB_BUTTON_HEIGHT);
	rxvt_drawBevel(r, r->scrollBar.win, SB_BUTTON_BEVEL_X,
		       height - SB_BUTTON_SINGLE_HEIGHT, SB_BUTTON_WIDTH,
		       SB_BUTTON_HEIGHT);

	s = (scrollbar_isUp()) ? r->h->upArrowHi : r->h->upArrow;
	XCopyArea(r->Xdisplay, s, r->scrollBar.win, r->h->whiteGC, 0, 0,
		  ARROW_WIDTH, ARROW_HEIGHT, SB_BUTTON_FACE_X,
		  height - SB_BUTTON_BOTH_HEIGHT + SB_BEVEL_WIDTH_UPPER_LEFT);

	s = (scrollbar_isDn()) ? r->h->downArrowHi : r->h->downArrow;
	XCopyArea(r->Xdisplay, s, r->scrollBar.win, r->h->whiteGC, 0, 0,
		  ARROW_WIDTH, ARROW_HEIGHT, SB_BUTTON_FACE_X,
		  height - SB_BUTTON_SINGLE_HEIGHT + SB_BEVEL_WIDTH_UPPER_LEFT);
    }
    return 1;
}
/*----------------------- end-of-file (C source) -----------------------*/
