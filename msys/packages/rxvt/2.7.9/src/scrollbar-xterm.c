/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar-xterm.c
 *----------------------------------------------------------------------*
 * $Id: scrollbar-xterm.c,v 1.1 2003/03/05 17:33:37 earnie Exp $
 *
 * Copyright (c) 1997,1998 mj olesen <olesen@me.QueensU.CA>
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
#include "scrollbar-xterm.intpro"	/* PROTOS for internal routines */

/*----------------------------------------------------------------------*/
#define x_stp_width	8
#define x_stp_height	2
const unsigned char x_stp_bits[] = { 0x55, 0xaa };

/* EXTPROTO */
int
rxvt_scrollbar_show_xterm(rxvt_t *r, int update __attribute__((unused)), int last_top, int last_bot, int scrollbar_len)
{
    int             xsb = 0;
    int             sbwidth = r->scrollBar.width - 1;

    if ((r->scrollBar.init & R_SB_XTERM) == 0) {
	XGCValues       gcvalue;

	r->scrollBar.init |= R_SB_XTERM;
	gcvalue.stipple = XCreateBitmapFromData(r->Xdisplay, r->scrollBar.win,
						(char *)x_stp_bits, x_stp_width,
						x_stp_height);
	if (!gcvalue.stipple) {
	    rxvt_print_error("can't create bitmap");
	    exit(EXIT_FAILURE);
	}
	gcvalue.fill_style = FillOpaqueStippled;
	gcvalue.foreground = r->PixColors[Color_fg];
	gcvalue.background = r->PixColors[Color_bg];

	r->h->xscrollbarGC = XCreateGC(r->Xdisplay, r->scrollBar.win,
				       GCForeground | GCBackground
				       | GCFillStyle | GCStipple, &gcvalue);
	gcvalue.foreground = r->PixColors[Color_border];
	r->h->ShadowGC = XCreateGC(r->Xdisplay, r->scrollBar.win, GCForeground,
				   &gcvalue);
    }
/* instead of XClearWindow (r->Xdisplay, r->scrollBar.win); */
    xsb = (r->Options & Opt_scrollBar_right) ? 1 : 0;
    if (last_top < r->scrollBar.top)
	XClearArea(r->Xdisplay, r->scrollBar.win,
		   r->sb_shadow + xsb, last_top,
		   sbwidth, (r->scrollBar.top - last_top), False);

    if (r->scrollBar.bot < last_bot)
	XClearArea(r->Xdisplay, r->scrollBar.win,
		   r->sb_shadow + xsb, r->scrollBar.bot,
		   sbwidth, (last_bot - r->scrollBar.bot), False);

/* scrollbar slider */
    XFillRectangle(r->Xdisplay, r->scrollBar.win, r->h->xscrollbarGC,
		   xsb + 1, r->scrollBar.top, sbwidth - 2, scrollbar_len);

    XDrawLine(r->Xdisplay, r->scrollBar.win, r->h->ShadowGC,
	      xsb ? 0 : sbwidth, r->scrollBar.beg,
	      xsb ? 0 : sbwidth, r->scrollBar.end);
    return 1;
}
/*----------------------- end-of-file (C source) -----------------------*/
