/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar.c
 *----------------------------------------------------------------------*
 * $Id: scrollbar.c,v 1.1 2002/12/06 23:08:03 earnie Exp $
 *
 * Copyright (C) 1997,1998 mj olesen <olesen@me.QueensU.CA>
 * Copyright (C) 1998      Alfredo K. Kojima <kojima@windowmaker.org>
 *				- N*XTstep like scrollbars
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

/*----------------------------------------------------------------------*
 */
#ifndef NEXT_SCROLLBAR
static GC       scrollbarGC;

#ifdef XTERM_SCROLLBAR		/* bitmap scrollbar */
static GC       ShadowGC;

static char     sb_bits[] =
{0xaa, 0x0a, 0x55, 0x05};	/* 12x2 bitmap */

#if (SB_WIDTH != 15)
Error, check scrollbar width(SB_WIDTH).It must be 15 for XTERM_SCROLLBAR
#endif

#else				/* XTERM_SCROLLBAR */
static GC       topShadowGC, botShadowGC;

/* draw triangular button with a shadow of SHADOW (1 or 2) pixels */
/* INTPROTO */
void
Draw_button(int x, int y, int state, int dirn)
{
    const unsigned int sz = (SB_WIDTH), sz2 = (SB_WIDTH / 2);
    XPoint          pt[3];
    GC              top, bot;

    switch (state) {
    case +1:
	top = topShadowGC;
	bot = botShadowGC;
	break;
    case -1:
	top = botShadowGC;
	bot = topShadowGC;
	break;
    default:
	top = bot = scrollbarGC;
	break;
    }

/* fill triangle */
    pt[0].x = x;
    pt[1].x = x + sz - 1;
    pt[2].x = x + sz2;
    if (dirn == UP) {
	pt[0].y = pt[1].y = y + sz - 1;
	pt[2].y = y;
    } else {
	pt[0].y = pt[1].y = y;
	pt[2].y = y + sz - 1;
    }
    XFillPolygon(Xdisplay, scrollBar.win, scrollbarGC,
		 pt, 3, Convex, CoordModeOrigin);

/* draw base */
    XDrawLine(Xdisplay, scrollBar.win, (dirn == UP ? bot : top),
	      pt[0].x, pt[0].y, pt[1].x, pt[1].y);

/* draw shadow on left */
    pt[1].x = x + sz2 - 1;
    pt[1].y = y + (dirn == UP ? 0 : sz - 1);
    XDrawLine(Xdisplay, scrollBar.win, top,
	      pt[0].x, pt[0].y, pt[1].x, pt[1].y);

#if (SHADOW > 1)
/* doubled */
    pt[0].x++;
    if (dirn == UP) {
	pt[0].y--;
	pt[1].y++;
    } else {
	pt[0].y++;
	pt[1].y--;
    }
    XDrawLine(Xdisplay, scrollBar.win, top,
	      pt[0].x, pt[0].y, pt[1].x, pt[1].y);
#endif
/* draw shadow on right */
    pt[1].x = x + sz - 1;
/* pt[2].x = x + sz2; */
    pt[1].y = y + (dirn == UP ? sz - 1 : 0);
    pt[2].y = y + (dirn == UP ? 0 : sz - 1);
    XDrawLine(Xdisplay, scrollBar.win, bot,
	      pt[2].x, pt[2].y, pt[1].x, pt[1].y);
#if (SHADOW > 1)
/* doubled */
    pt[1].x--;
    if (dirn == UP) {
	pt[2].y++;
	pt[1].y--;
    } else {
	pt[2].y--;
	pt[1].y++;
    }
    XDrawLine(Xdisplay, scrollBar.win, bot,
	      pt[2].x, pt[2].y, pt[1].x, pt[1].y);
#endif
}
#endif				/* ! XTERM_SCROLLBAR */

#else				/* ! NEXT_SCROLLBAR */
/*
 * N*XTSTEP like scrollbar - written by Alfredo K. Kojima
 */
static GC       blackGC, whiteGC, grayGC, darkGC, stippleGC;
static Pixmap   dimple, upArrow, downArrow, upArrowHi, downArrowHi;

const char *const SCROLLER_DIMPLE[] =
{
    ".%###.",
    "%#%%%%",
    "#%%...",
    "#%..  ",
    "#%.   ",
    ".%.  ."
};

#define SCROLLER_DIMPLE_WIDTH   6
#define SCROLLER_DIMPLE_HEIGHT  6

const char *const SCROLLER_ARROW_UP[] =
{
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

const char *const SCROLLER_ARROW_DOWN[] =
{
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

const char *const HI_SCROLLER_ARROW_UP[] =
{
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

const char *const HI_SCROLLER_ARROW_DOWN[] =
{
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

#define ARROW_WIDTH   13
#define ARROW_HEIGHT  13

#define stp_width 8
#define stp_height 8
const unsigned char stp_bits[] =
    { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };

/* INTPROTO */
Pixmap
renderPixmap(const char *const *data, int width, int height)
{
    int             x, y;
    Pixmap          d;

    d = XCreatePixmap(Xdisplay, scrollBar.win, width, height, Xdepth);

    for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {
	    switch (data[y][x]) {
	    case ' ':
	    case 'w':
		XDrawPoint(Xdisplay, d, whiteGC, x, y);
		break;
	    case '.':
	    case 'l':
		XDrawPoint(Xdisplay, d, grayGC, x, y);
		break;
	    case '%':
	    case 'd':
		XDrawPoint(Xdisplay, d, darkGC, x, y);
		break;
	    case '#':
	    case 'b':
	    default:
		XDrawPoint(Xdisplay, d, blackGC, x, y);
		break;
	    }
	}
    }
    return d;
}

/* INTPROTO */
void
init_scrollbar_stuff(void)
{
    XGCValues       gcvalue;
    XColor          xcol;
    Pixmap          stipple;
    unsigned long   light, dark;

    gcvalue.graphics_exposures = False;

    gcvalue.foreground = BlackPixelOfScreen(DefaultScreenOfDisplay(Xdisplay));
    blackGC = XCreateGC(Xdisplay, scrollBar.win, GCForeground | GCGraphicsExposures,
			&gcvalue);

    gcvalue.foreground = WhitePixelOfScreen(DefaultScreenOfDisplay(Xdisplay));
    whiteGC = XCreateGC(Xdisplay, scrollBar.win, GCForeground | GCGraphicsExposures,
			&gcvalue);

    xcol.red = 0xaeba;
    xcol.green = 0xaaaa;
    xcol.blue = 0xaeba;
    if (!XAllocColor(Xdisplay, Xcmap, &xcol)) {
	print_error("can't allocate %s", "light gray");
	xcol.pixel = PixColors[Color_AntiqueWhite];
    }
    light = gcvalue.foreground = xcol.pixel;
    grayGC = XCreateGC(Xdisplay, scrollBar.win, GCForeground | GCGraphicsExposures,
		       &gcvalue);

    xcol.red = 0x51aa;
    xcol.green = 0x5555;
    xcol.blue = 0x5144;
    if (!XAllocColor(Xdisplay, Xcmap, &xcol)) {
	print_error("can't allocate %s", "dark gray");
	xcol.pixel = PixColors[Color_Grey25];
    }
    dark = gcvalue.foreground = xcol.pixel;
    darkGC = XCreateGC(Xdisplay, scrollBar.win, GCForeground | GCGraphicsExposures,
		       &gcvalue);

    stipple = XCreateBitmapFromData(Xdisplay, scrollBar.win,
				    stp_bits, stp_width, stp_height);

    gcvalue.foreground = dark;
    gcvalue.background = light;
    gcvalue.fill_style = FillStippled;
    gcvalue.stipple = stipple;

/*    XSetWindowBackground(Xdisplay, scrollBar.win, PixColors[Color_Red]); */

    stippleGC = XCreateGC(Xdisplay, scrollBar.win, GCForeground | GCBackground
			  | GCStipple | GCFillStyle | GCGraphicsExposures,
			  &gcvalue);

    dimple = renderPixmap(SCROLLER_DIMPLE, SCROLLER_DIMPLE_WIDTH,
			  SCROLLER_DIMPLE_HEIGHT);

    upArrow = renderPixmap(SCROLLER_ARROW_UP, ARROW_WIDTH, ARROW_HEIGHT);
    downArrow = renderPixmap(SCROLLER_ARROW_DOWN, ARROW_WIDTH, ARROW_HEIGHT);
    upArrowHi = renderPixmap(HI_SCROLLER_ARROW_UP, ARROW_WIDTH, ARROW_HEIGHT);
    downArrowHi = renderPixmap(HI_SCROLLER_ARROW_DOWN, ARROW_WIDTH, ARROW_HEIGHT);

    scrollbar_show(1);
}

/* Draw bevel & arrows */
/* INTPROTO */
void
drawBevel(Drawable d, int x, int y, int w, int h)
{
    XDrawLine(Xdisplay, d, whiteGC, x, y, x + w - 1, y);
    XDrawLine(Xdisplay, d, whiteGC, x, y, x, y + h - 1);

    XDrawLine(Xdisplay, d, blackGC, x + w - 1, y, x + w - 1, y + h - 1);
    XDrawLine(Xdisplay, d, blackGC, x, y + h - 1, x + w - 1, y + h - 1);

    XDrawLine(Xdisplay, d, darkGC, x + 1, y + h - 2, x + w - 2, y + h - 2);
    XDrawLine(Xdisplay, d, darkGC, x + w - 2, y + 1, x + w - 2, y + h - 2);
}

#endif				/* ! NEXT_SCROLLBAR */

/* EXTPROTO */
int
scrollbar_show(int update)
{
    static int      scrollbar_len;	/* length of slider */
    static int      last_top, last_bot, last_state = -1;
#ifndef NEXT_SCROLLBAR
    static short    sb_width;		/* old (drawn) values */
    int             xsb = 0;

    if (!scrollbar_visible())
	return 0;

    if (scrollbarGC == None) {
	XGCValues       gcvalue;

#ifdef XTERM_SCROLLBAR
	sb_width = SB_WIDTH - 1;
	gcvalue.stipple = XCreateBitmapFromData(Xdisplay, scrollBar.win,
						sb_bits, 12, 2);
	if (!gcvalue.stipple) {
	    print_error("can't create bitmap");
	    exit(EXIT_FAILURE);
	}
	gcvalue.fill_style = FillOpaqueStippled;
	gcvalue.foreground = PixColors[Color_fg];
	gcvalue.background = PixColors[Color_bg];

	scrollbarGC = XCreateGC(Xdisplay, scrollBar.win,
				GCForeground | GCBackground |
				GCFillStyle | GCStipple,
				&gcvalue);
	gcvalue.foreground = PixColors[Color_border];
	ShadowGC = XCreateGC(Xdisplay, scrollBar.win, GCForeground, &gcvalue);
#else				/* XTERM_SCROLLBAR */
	sb_width = SB_WIDTH;

	gcvalue.foreground = PixColors[Color_trough];
	if (sb_shadow) {
	    XSetWindowBackground(Xdisplay, scrollBar.win, gcvalue.foreground);
	    XClearWindow(Xdisplay, scrollBar.win);
	}
	gcvalue.foreground = (Xdepth <= 2 ? PixColors[Color_fg]
			      : PixColors[Color_scroll]);
	scrollbarGC = XCreateGC(Xdisplay, scrollBar.win, GCForeground,
				&gcvalue);

	gcvalue.foreground = PixColors[Color_topShadow];
	topShadowGC = XCreateGC(Xdisplay, scrollBar.win,
				GCForeground,
				&gcvalue);

	gcvalue.foreground = PixColors[Color_bottomShadow];
	botShadowGC = XCreateGC(Xdisplay, scrollBar.win,
				GCForeground,
				&gcvalue);
#endif				/* XTERM_SCROLLBAR */
    }
    if (update) {
	int             top = (TermWin.nscrolled - TermWin.view_start);
	int             bot = top + (TermWin.nrow - 1);
	int             len = max((TermWin.nscrolled + (TermWin.nrow - 1)), 1);
	int             adj = ((bot - top) * scrollbar_size()) % len;

	scrollBar.top = (scrollBar.beg + (top * scrollbar_size()) / len);
        scrollbar_len = (((bot - top) * scrollbar_size()) / len +
			 SCROLL_MINHEIGHT + ((adj > 0) ? 1 : 0));
	scrollBar.bot = (scrollBar.top + scrollbar_len);
    /* no change */
	if ((scrollBar.top == last_top) && (scrollBar.bot == last_bot)
	    && ((scrollBar.state == last_state) || (!scrollbar_isUpDn())))
	    return 0;
    }
/* instead of XClearWindow (Xdisplay, scrollBar.win); */
#ifdef XTERM_SCROLLBAR
    xsb = (Options & Opt_scrollBar_right) ? 1 : 0;
#endif
    if (last_top < scrollBar.top)
	XClearArea(Xdisplay, scrollBar.win,
		   sb_shadow + xsb, last_top,
		   sb_width, (scrollBar.top - last_top),
		   False);

    if (scrollBar.bot < last_bot)
	XClearArea(Xdisplay, scrollBar.win,
		   sb_shadow + xsb, scrollBar.bot,
		   sb_width, (last_bot - scrollBar.bot),
		   False);

    last_top = scrollBar.top;
    last_bot = scrollBar.bot;

/* scrollbar slider */
#ifdef XTERM_SCROLLBAR
    XFillRectangle(Xdisplay, scrollBar.win, scrollbarGC,
		   xsb + 1, scrollBar.top,
		   sb_width - 2, scrollbar_len);

    XDrawLine(Xdisplay, scrollBar.win, ShadowGC,
	      xsb ? 0 : sb_width, scrollBar.beg, xsb ? 0 : sb_width,
	      scrollBar.end);
#else
#ifdef SB_BORDER
    {
	int             xofs;

	if (Options & Opt_scrollBar_right)
	    xofs = 0;
	else
	    xofs = (sb_shadow) ? SB_WIDTH : SB_WIDTH - 1;

	XDrawLine(Xdisplay, scrollBar.win, botShadowGC,
		  xofs, 0, xofs, scrollBar.end + SB_WIDTH);
    }
#endif
    XFillRectangle(Xdisplay, scrollBar.win, scrollbarGC,
		   sb_shadow, scrollBar.top,
		   sb_width, scrollbar_len);

    if (sb_shadow)
    /* trough shadow */
	Draw_Shadow(scrollBar.win,
		    botShadowGC, topShadowGC,
		    0, 0,
		    (sb_width + 2 * sb_shadow),
		    (scrollBar.end + (sb_width + 1) + sb_shadow));
/* shadow for scrollbar slider */
    Draw_Shadow(scrollBar.win,
		topShadowGC, botShadowGC,
		sb_shadow, scrollBar.top, sb_width,
		scrollbar_len);

/*
 * Redraw scrollbar arrows
 */
    Draw_button(sb_shadow, sb_shadow,
		(scrollbar_isUp()? -1 : +1), UP);
    Draw_button(sb_shadow, (scrollBar.end + 1),
		(scrollbar_isDn()? -1 : +1), DN);
#endif				/* XTERM_SCROLLBAR */
    last_top = scrollBar.top;
    last_bot = scrollBar.bot;
    last_state = scrollBar.state;

#else				/* NEXT_SCROLLBAR */
    Pixmap          buffer;
    int             height = scrollBar.end + SB_BUTTON_TOTAL_HEIGHT + SB_PADDING;

    if (blackGC == NULL)
	init_scrollbar_stuff();

    if (update) {
	int             top = (TermWin.nscrolled - TermWin.view_start);
	int             bot = top + (TermWin.nrow - 1);
	int             len = max((TermWin.nscrolled + (TermWin.nrow - 1)), 1);
	int             adj = ((bot - top) * scrollbar_size()) % len;

	scrollBar.top = (scrollBar.beg + (top * scrollbar_size()) / len);
        scrollbar_len = (((bot - top) * scrollbar_size()) / len +
			 SCROLL_MINHEIGHT + ((adj > 0) ? 1 : 0));
	scrollBar.bot = (scrollBar.top + scrollbar_len);
    /* no change */
	if ((scrollBar.top == last_top) && (scrollBar.bot == last_bot)
	    && ((scrollBar.state == last_state) || (!scrollbar_isUpDn())))
	    return 0;
    }
/* create double buffer */
    buffer = XCreatePixmap(Xdisplay, scrollBar.win, SB_WIDTH + 1, height, Xdepth);

    last_top = scrollBar.top;
    last_bot = scrollBar.bot;
    last_state = scrollBar.state;

/* draw the background */
    XFillRectangle(Xdisplay, buffer, grayGC, 0, 0, SB_WIDTH + 1, height);
    XDrawRectangle(Xdisplay, buffer, blackGC, 0, -SB_BORDER_WIDTH,
		   SB_WIDTH, height + SB_BORDER_WIDTH);

    if (TermWin.nscrolled > 0) {
	XFillRectangle(Xdisplay, buffer, stippleGC,
		       SB_LEFT_PADDING, SB_PADDING,
		       SB_BUTTON_WIDTH,
		       height - SB_BUTTON_TOTAL_HEIGHT - SB_PADDING);
	XFillRectangle(Xdisplay, buffer, grayGC,
		       SB_LEFT_PADDING, scrollBar.top + SB_PADDING,
		       SB_BUTTON_WIDTH, scrollbar_len);
	drawBevel(buffer, SB_BUTTON_BEVEL_X, scrollBar.top + SB_PADDING,
		  SB_BUTTON_WIDTH, scrollbar_len);
	drawBevel(buffer, SB_BUTTON_BEVEL_X, height - SB_BUTTON_BOTH_HEIGHT,
		  SB_BUTTON_WIDTH, SB_BUTTON_HEIGHT);
	drawBevel(buffer, SB_BUTTON_BEVEL_X, height - SB_BUTTON_SINGLE_HEIGHT,
		  SB_BUTTON_WIDTH, SB_BUTTON_HEIGHT);

	XCopyArea(Xdisplay, dimple, buffer, whiteGC, 0, 0,
		  SCROLLER_DIMPLE_WIDTH, SCROLLER_DIMPLE_HEIGHT,
		  (SB_WIDTH - SCROLLER_DIMPLE_WIDTH) / 2,
		  scrollBar.top + SB_BEVEL_WIDTH_UPPER_LEFT +
		  (scrollbar_len - SCROLLER_DIMPLE_HEIGHT) / 2);

	if (scrollbar_isUp())
	    XCopyArea(Xdisplay, upArrowHi, buffer, whiteGC, 0, 0,
		      ARROW_WIDTH, ARROW_HEIGHT,
		      SB_BUTTON_FACE_X,
		      height - (SB_BUTTON_BOTH_HEIGHT - SB_BEVEL_WIDTH_UPPER_LEFT));
	else
	    XCopyArea(Xdisplay, upArrow, buffer, whiteGC, 0, 0,
		      ARROW_WIDTH, ARROW_HEIGHT,
		      SB_BUTTON_FACE_X,
		      height - (SB_BUTTON_BOTH_HEIGHT - SB_BEVEL_WIDTH_UPPER_LEFT));

	if (scrollbar_isDn())
	    XCopyArea(Xdisplay, downArrowHi, buffer, whiteGC, 0, 0,
		      ARROW_WIDTH, ARROW_HEIGHT,
		      SB_BUTTON_FACE_X,
		      height - (SB_BUTTON_SINGLE_HEIGHT - SB_BEVEL_WIDTH_UPPER_LEFT));
	else
	    XCopyArea(Xdisplay, downArrow, buffer, whiteGC, 0, 0,
		      ARROW_WIDTH, ARROW_HEIGHT,
		      SB_BUTTON_FACE_X,
		      height - (SB_BUTTON_SINGLE_HEIGHT - SB_BEVEL_WIDTH_UPPER_LEFT));
    } else {
	XFillRectangle(Xdisplay, buffer, stippleGC,
		       SB_LEFT_PADDING, SB_PADDING,
		       SB_BUTTON_WIDTH, height - SB_MARGIN_SPACE);
    }

    if (Options & Opt_scrollBar_right)
	XCopyArea(Xdisplay, buffer, scrollBar.win, grayGC, 0, 0,
		  SB_WIDTH + SB_BORDER_WIDTH, height, 0, 0);
    else
	XCopyArea(Xdisplay, buffer, scrollBar.win, grayGC, 0, 0,
		  SB_WIDTH + SB_BORDER_WIDTH, height, -SB_BORDER_WIDTH, 0);

    XFreePixmap(Xdisplay, buffer);
#endif				/* ! NEXT_SCROLLBAR */
    return 1;
}

/* EXTPROTO */
int
scrollbar_mapping(int map)
{
    int             change = 0;

    if (map && !scrollbar_visible()) {
	scrollBar.state = 1;
	if (scrollBar.win == 0)
	    return 0;
	XMapWindow(Xdisplay, scrollBar.win);
	change = 1;
    } else if (!map && scrollbar_visible()) {
	scrollBar.state = 0;
	XUnmapWindow(Xdisplay, scrollBar.win);
	change = 1;
    }
    return change;
}

/* EXTPROTO */
void
map_scrollBar(int map)
{
    if (scrollbar_mapping(map)) {
	resize_all_windows();
	scr_touch(True);
    }
}

/* EXTPROTO */
void
Resize_scrollBar(void)
{
    unsigned int    x, xsb;

    xsb = (SB_WIDTH + 2 * sb_shadow);
    x = (Options & Opt_scrollBar_right) ? (szHint.width - xsb) : 0;
    XMoveResizeWindow(Xdisplay, scrollBar.win, x, 0, xsb, szHint.height);

    scrollBar.beg = 0;
    scrollBar.end = szHint.height;
#ifndef XTERM_SCROLLBAR
/* arrows are as high as wide - leave 1 pixel gap */
# ifdef NEXT_SCROLLBAR
    scrollBar.end -= SB_BUTTON_TOTAL_HEIGHT + SB_PADDING;
# else
    scrollBar.beg += (SB_WIDTH + 1) + sb_shadow;
    scrollBar.end -= (SB_WIDTH + 1) + sb_shadow;
# endif
#endif
    scrollbar_show(1);
}

/*----------------------- end-of-file (C source) -----------------------*/
