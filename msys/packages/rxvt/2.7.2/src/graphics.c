/*--------------------------------*-C-*---------------------------------*
 * File:	graphics.c
 *----------------------------------------------------------------------*
 * $Id: graphics.c,v 1.1 2002-12-06 23:08:02 earnie Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (C) 1994      Rob Nation <nation@rocket.sanders.lockheed.com>
 *				- original version
 * Copyright (C) 1997      Raul Garcia Garcia <rgg@tid.es>
 * Copyright (C) 1997,1998 mj olesen <olesen@me.queensu.ca>
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
#ifdef RXVT_GRAPHICS
#include "rxvt.h"		/* NECESSARY */

typedef struct grwin_t {
    Window          win;
    int             x, y;
    unsigned int    w, h;
    short           screen;
    grcmd_t        *graphics;
    struct grwin_t *prev, *next;
} grwin_t;

#include "graphics.intpro"	/* PROTOS for internal routines */
#include <X11/cursorfont.h>

/* commands:
 * 'C' = Clear
 * 'F' = Fill
 * 'G' = Geometry
 * 'L' = Line
 * 'P' = Points
 * 'T' = Text
 * 'W' = Window
 */

#ifndef GRX_SCALE
# define GRX_SCALE	10000
#endif

static int      graphics_up = 0;
static grwin_t *gr_root = NULL;

/*----------------------------------------------------------------------*
 * local functions
 */
/* ARGSUSED */
/* INTPROTO */
void
Gr_NewWindow(int nargs, int args[])
{
    int             x, y;
    unsigned int    w, h;
    Window          win;
    grwin_t        *grwin;
    Cursor          cursor;

    if (nargs != 4) {
	print_error("NewWindow: 4 args needed, got %d\n", nargs);
	return;
    }
    x = args[0] * TermWin.width / GRX_SCALE + TermWin.int_bwidth;
    y = args[1] * TermWin.height / GRX_SCALE + TermWin.int_bwidth;
    w = args[2] * TermWin.width / GRX_SCALE;
    h = args[3] * TermWin.height / GRX_SCALE;

    win = XCreateSimpleWindow(Xdisplay, TermWin.vt,
			      x, y, w, h,
			      0,
			      PixColors[Color_fg],
			      PixColors[Color_bg]);

    cursor = XCreateFontCursor(Xdisplay, XC_crosshair);
    XDefineCursor(Xdisplay, win, cursor);
    XMapWindow(Xdisplay, win);
    XSelectInput(Xdisplay, win, ExposureMask);

    grwin = (grwin_t *) MALLOC(sizeof(grwin_t));
    grwin->win = win;
    grwin->x = x;
    grwin->y = y;
    grwin->w = w;
    grwin->h = h;
    grwin->screen = 0;
    grwin->prev = NULL;
    grwin->next = gr_root;
    if (grwin->next)
	grwin->next->prev = grwin;
    gr_root = grwin;
    grwin->graphics = NULL;
    graphics_up++;

    tt_printf("\033W%ld\n", (long)grwin->win);
}

/* ARGSUSED */
/* INTPROTO */
void
Gr_ClearWindow(grwin_t *grwin)
{
    grcmd_t        *cmd, *next;

    for (cmd = grwin->graphics; cmd != NULL; cmd = next) {
	next = cmd->next;
	free(cmd->coords);
	if (cmd->text != NULL)
	    free(cmd->text);
	free(cmd);
    }
    grwin->graphics = NULL;
    XClearWindow(Xdisplay, grwin->win);
}

/*
 * arg [0] = x
 * arg [1] = y
 * arg [2] = alignment
 * arg [3] = strlen (text)
 */
/* ARGSUSED */
/* INTPROTO */
void
Gr_Text(grwin_t *grwin, grcmd_t *data)
{
    int             x, y, align;

    if (data->ncoords < 4 || data->text == NULL || *(data->text) == '\0')
	return;

    x = data->coords[0] * grwin->w / GRX_SCALE;
    y = data->coords[1] * grwin->h / GRX_SCALE;
    align = data->coords[2];

    if (align & RIGHT_TEXT)
	x -= XTextWidth(TermWin.font, data->text, data->coords[3]);
    else if (align & HCENTER_TEXT)
	x -= (XTextWidth(TermWin.font, data->text, data->coords[3]) >> 1);

    if (align & TOP_TEXT)
	y += TermWin.font->ascent;
    else if (align & BOTTOM_TEXT)
	y -= TermWin.font->descent;

    if (align & VCENTER_TEXT)
	y -= TermWin.font->descent
	     + ((TermWin.font->ascent + TermWin.font->descent) >> 1);
    if (align & VCAPS_CENTER_TEXT)
	y += (TermWin.font->ascent >> 1);

    XPMClearArea(Xdisplay, grwin->win, x, y, Width2Pixel(data->coords[3]),
		 Height2Pixel(1), 0);
    XDrawString(Xdisplay, grwin->win, TermWin.gc, x, y,
		data->text, data->coords[3]);
}

/* ARGSUSED */
/* INTPROTO */
void
Gr_Geometry(grwin_t *grwin, grcmd_t *data)
{
    if (grwin)
	tt_printf("\033G%ld %d %d %u %u %d %d %ld %ld %d\n",
		  (long)grwin->win,
		  grwin->x, grwin->y, grwin->w, grwin->h,
		  TermWin.fwidth,
		  TermWin.fheight,
		  (long)GRX_SCALE * TermWin.fwidth / grwin->w,
		  (long)GRX_SCALE * TermWin.fheight / grwin->h,
		  Xdepth);
    else			/* rxvt terminal window size */
	tt_printf("\033G0 0 0 %d %d %d %d %ld %ld %d\n",
		  TermWin.width - 2 * TermWin.int_bwidth,
		  TermWin.height - 2 * TermWin.int_bwidth,
		  TermWin.fwidth,
		  TermWin.fheight,
		  (long)GRX_SCALE * TermWin.fwidth
		        / (TermWin.width - 2 * TermWin.int_bwidth),
		  (long)GRX_SCALE * TermWin.fheight
		        / (TermWin.height - 2 * TermWin.int_bwidth),
		  Xdepth);
}

/* ARGSUSED */
/* INTPROTO */
void
Gr_DestroyWindow(grwin_t *grwin)
{
    grcmd_t        *cmd, *next;

    if (grwin == NULL)
	return;

    for (cmd = grwin->graphics; cmd; cmd = next) {
	next = cmd->next;
	free(cmd->coords);
	if (cmd->text != NULL)
	    free(cmd->text);
	free(cmd);
    }

    XDestroyWindow(Xdisplay, grwin->win);
    if (grwin->next != NULL)
	grwin->next->prev = grwin->prev;
    if (grwin->prev != NULL)
	grwin->prev->next = grwin->next;
    else
	gr_root = grwin->next;
    free(grwin);

    graphics_up--;
}

/* ARGSUSED */
/* INTPROTO */
void
Gr_Dispatch(grwin_t *grwin, grcmd_t *data)
{
    int             i, n;
    union {
	XPoint          pt[NGRX_PTS / 2];
	XRectangle      rect[NGRX_PTS / 4];
    } xdata;

    if (data->color != Color_fg) {
	XGCValues       gcv;

	gcv.foreground = PixColors[data->color];
	XChangeGC(Xdisplay, TermWin.gc, GCForeground, &gcv);
    }
    if (grwin)
	switch (data->cmd) {
	case 'L':
	    if (data->ncoords > 3) {
		for (n = i = 0; i < data->ncoords; i += 2, n++) {
		    xdata.pt[n].x = data->coords[i] * grwin->w / GRX_SCALE;
		    xdata.pt[n].y = data->coords[i + 1] * grwin->h / GRX_SCALE;
		}
		XDrawLines(Xdisplay,
		       grwin->win, TermWin.gc, xdata.pt, n, CoordModeOrigin);
	    }
	    break;

	case 'P':
	    if (data->ncoords > 3) {
		for (n = i = 0; i < data->ncoords; i += 2, n++) {
		    xdata.pt[n].x = data->coords[i] * grwin->w / GRX_SCALE;
		    xdata.pt[n].y = data->coords[i + 1] * grwin->h / GRX_SCALE;
		}
		XDrawPoints(Xdisplay,
		       grwin->win, TermWin.gc, xdata.pt, n, CoordModeOrigin);
	    }
	    break;

	case 'F':
	    if (data->ncoords > 0) {
		for (n = i = 0; i < data->ncoords; i += 4, n++) {
		    xdata.rect[n].x = data->coords[i] * grwin->w / GRX_SCALE;
		    xdata.rect[n].y = data->coords[i + 1] * grwin->h
				      / GRX_SCALE;
		    xdata.rect[n].width = ((data->coords[i + 2]
					    - data->coords[i] + 1) *
					   grwin->w / GRX_SCALE);
		    xdata.rect[n].height = ((data->coords[i + 3]
					     - data->coords[i + 1] + 1) *
					    grwin->h / GRX_SCALE);
		    XPMClearArea(Xdisplay, grwin->win,
				 xdata.rect[n].x, xdata.rect[n].y,
				 xdata.rect[n].width, xdata.rect[n].height,
				 0);
		}
		XFillRectangles(Xdisplay, grwin->win, TermWin.gc, xdata.rect,
				n);
	    }
	    break;
	case 'T':
	    Gr_Text(grwin, data);
	    break;
	case 'C':
	    Gr_ClearWindow(grwin);
	    break;
	}
    if (data->color != Color_fg) {
	XGCValues       gcv;

	gcv.foreground = PixColors[Color_fg];
	XChangeGC(Xdisplay, TermWin.gc, GCForeground, &gcv);
    }
}

/* ARGSUSED */
/* INTPROTO */
void
Gr_Redraw(grwin_t *grwin)
{
    grcmd_t        *cmd;

    for (cmd = grwin->graphics; cmd != NULL; cmd = cmd->next)
	Gr_Dispatch(grwin, cmd);
}

/*----------------------------------------------------------------------*
 * end of static functions
 */
/* ARGSUSED */
/* EXTPROTO */
void
Gr_ButtonReport(int but, int x, int y)
{
    grwin_t        *grwin;

    for (grwin = gr_root; grwin != NULL; grwin = grwin->next)
	if ((x > grwin->x)
	    && (y > grwin->y)
	    && (x < grwin->x + grwin->w)
	    && (y < grwin->y + grwin->h))
	    break;

    if (grwin == NULL)
	return;

    x = GRX_SCALE * (x - grwin->x) / grwin->w;
    y = GRX_SCALE * (y - grwin->y) / grwin->h;
    tt_printf("\033%c%ld;%d;%d;\n", but, (long)grwin->win, x, y);
}

/* ARGSUSED */
/* EXTPROTO */
void
Gr_do_graphics(int cmd, int nargs, int args[], unsigned char *text)
{
    static Window   last_id = None;
    long            win_id;
    grwin_t        *grwin;
    grcmd_t        *newcmd, *oldcmd;
    int             i;

    if (cmd == 'W') {
	Gr_NewWindow(nargs, args);
	return;
    }
    win_id = (nargs > 0) ? (Window) args[0] : None;

    if ((cmd == 'G') && (win_id == None)) {
	Gr_Geometry(NULL, NULL);
	return;
    }
    if ((win_id == None) && (last_id != None))
	win_id = last_id;

    if (win_id == None)
	return;

    grwin = gr_root;
    while ((grwin != NULL) && (grwin->win != win_id))
	grwin = grwin->next;

    if (grwin == NULL)
	return;

    if (cmd == 'G') {
	Gr_Geometry(grwin, NULL);
	return;
    }
    nargs--;
    args++;			/* skip over window id */

/* record this new command */
    newcmd = (grcmd_t *) MALLOC(sizeof(grcmd_t));
    newcmd->ncoords = nargs;
    newcmd->coords = (int *)MALLOC((newcmd->ncoords * sizeof(int)));

    newcmd->next = NULL;
    newcmd->cmd = cmd;
    newcmd->color = scr_get_fgcolor();
    newcmd->text = text;

    for (i = 0; i < newcmd->ncoords; i++)
	newcmd->coords[i] = args[i];

/*
 * If newcmd == fill, and rectangle is full window, drop all prior
 * commands.
 */
    if ((newcmd->cmd == 'F') && (grwin) && (grwin->graphics)) {
	for (i = 0; i < newcmd->ncoords; i += 4) {
	    if ((newcmd->coords[i] == 0)
		&& (newcmd->coords[i + 1] == 0)
		&& (newcmd->coords[i + 2] == GRX_SCALE)
		&& (newcmd->coords[i + 3] == GRX_SCALE)) {
	    /* drop previous commands */
		oldcmd = grwin->graphics;
		while (oldcmd->next != NULL) {
		    grcmd_t        *tmp = oldcmd;

		    oldcmd = oldcmd->next;
		    free(tmp);
		}
		grwin->graphics = NULL;
	    }
	}
    }
/* insert new command into command list */
    oldcmd = grwin->graphics;
    if (oldcmd == NULL)
	grwin->graphics = newcmd;
    else {
	while (oldcmd->next != NULL)
	    oldcmd = oldcmd->next;
	oldcmd->next = newcmd;
    }
    Gr_Dispatch(grwin, newcmd);
}

/* ARGSUSED */
/* EXTPROTO */
void
Gr_scroll(int count)
{
    static short    prev_start = 0;
    grwin_t        *grwin, *next;

    if ((count == 0) && (prev_start == TermWin.view_start))
	return;

    prev_start = TermWin.view_start;

    for (grwin = gr_root; grwin != NULL; grwin = next) {
	next = grwin->next;
	grwin->y -= (count * TermWin.fheight);
	if ((grwin->y + grwin->h) < -(TermWin.saveLines * TermWin.fheight))
	    Gr_DestroyWindow(grwin);
	else
	    XMoveWindow(Xdisplay, grwin->win,
			grwin->x,
			grwin->y + (TermWin.view_start * TermWin.fheight));
    }
}

/* EXTPROTO */
void
Gr_ClearScreen(void)
{
    grwin_t        *grwin, *next;

    for (grwin = gr_root; grwin != NULL; grwin = next) {
	next = grwin->next;
	if ((grwin->screen == 0) && (grwin->y + grwin->h > 0)) {
	    if (grwin->y >= 0)
		Gr_DestroyWindow(grwin);
	    else
		XResizeWindow(Xdisplay, grwin->win,
			      grwin->w, -grwin->y);
	}
    }
}

/* EXTPROTO */
void
Gr_ChangeScreen(void)
{
    grwin_t        *grwin, *next;

    for (grwin = gr_root; grwin != NULL; grwin = next) {
	next = grwin->next;
	if (grwin->y + grwin->h > 0) {
	    if (grwin->screen == 1) {
		XMapWindow(Xdisplay, grwin->win);
		grwin->screen = 0;
	    } else {
		XUnmapWindow(Xdisplay, grwin->win);
		grwin->screen = 1;
	    }
	}
    }
}

/* ARGSUSED */
/* EXTPROTO */
void
Gr_expose(Window win)
{
    grwin_t        *grwin;

    for (grwin = gr_root; grwin != NULL; grwin = grwin->next) {
	if (grwin->win == win) {
	    Gr_Redraw(grwin);
	    break;
	}
    }
}

/* ARGSUSED */
/* EXTPROTO */
void
Gr_Resize(int w, int h)
{
    grwin_t        *grwin;

    for (grwin = gr_root; grwin != NULL; grwin = grwin->next) {
	if (TermWin.height != h) {
	    grwin->y += (TermWin.height - h);
	    XMoveWindow(Xdisplay, grwin->win,
			grwin->x,
			grwin->y + (TermWin.view_start * TermWin.fheight));
	}
	Gr_Redraw(grwin);
    }
}

/* EXTPROTO */
void
Gr_reset(void)
{
    grwin_t        *grwin, *next;

    for (grwin = gr_root; grwin != NULL; grwin = next) {
	next = grwin->next;
	Gr_DestroyWindow(grwin);
    }

    graphics_up = 0;
}

/* EXTPROTO */
int
Gr_Displayed(void)
{
    return graphics_up;
}
#endif
/*----------------------- end-of-file (C source) -----------------------*/
