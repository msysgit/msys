/*--------------------------------*-C-*---------------------------------*
 * File:	xpm.c
 *----------------------------------------------------------------------*
 * $Id: xpm.c,v 1.1 2002-12-06 23:08:03 earnie Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (C) 1997      Carsten Haitzler <raster@zip.com.au>
 * Copyright (C) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (C) 1998,1999 Geoff Wing <gcw@pobox.com>
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
 *---------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "xpm.intpro"		/* PROTOS for internal routines */

#ifdef XPM_BACKGROUND
# ifdef XPM_INC_X11
#  include <X11/xpm.h>
# else
#  include <xpm.h>
# endif

typedef struct {
    short           w, h, x, y;
    Pixmap          pixmap;
} bgPixmap_t;

static bgPixmap_t bgPixmap = { 0, 0, 50, 50, None };

/* the originally loaded pixmap and its scaling */
static XpmAttributes xpmAttr;

/*
 * These GEOM strings indicate absolute size/position:
 * @ `WxH+X+Y'
 * @ `WxH+X'    -> Y = X
 * @ `WxH'      -> Y = X = 50
 * @ `W+X+Y'    -> H = W
 * @ `W+X'      -> H = W, Y = X
 * @ `W'        -> H = W, X = Y = 50
 * @ `0xH'      -> H *= H/100, X = Y = 50 (W unchanged)
 * @ `Wx0'      -> W *= W/100, X = Y = 50 (H unchanged)
 * @ `=+X+Y'    -> (H, W unchanged)
 * @ `=+X'      -> Y = X (H, W unchanged)
 *
 * These GEOM strings adjust position relative to current position:
 * @ `+X+Y'
 * @ `+X'       -> Y = X
 *
 * And this GEOM string is for querying current scale/position:
 * @ `?'
 */
/* EXTPROTO */
int
scale_pixmap(const char *geom)
{
    int             w = 0, h = 0, x = 0, y = 0;
    int             n, flags, changed = 0;
    char           *p;
    static char     str[] = "[1000x1000+1000+1000]";

    if (geom == NULL)
	return 0;
    if (!STRCMP(geom, "?")) {
	sprintf(str, "[%dx%d+%d+%d]",	/* can't presume snprintf? */
		(bgPixmap.w < 9999 ? bgPixmap.w : 9999),
		(bgPixmap.h < 9999 ? bgPixmap.h : 9999),
		(bgPixmap.x < 9999 ? bgPixmap.x : 9999),
		(bgPixmap.y < 9999 ? bgPixmap.y : 9999));
	xterm_seq(XTerm_title, str);
	return 0;
    }

    if ((p = STRCHR(geom, ';')) == NULL)
	p = STRCHR(geom, '\0');
    n = (p - geom);
    if (n >= sizeof(str) - 1)
	return 0;
    STRNCPY(str, geom, n);
    str[n] = '\0';

    flags = XParseGeometry(str, &x, &y, (unsigned int *)&w,
			   (unsigned int *)&h);
    if (!flags) {
	flags |= WidthValue;
	w = 0;
    }				/* default is tile */
    if (flags & WidthValue) {
	if (!(flags & XValue))
	    x = 50;

	if (!(flags & HeightValue))
	    h = w;

	if (w && !h) {
	    w = bgPixmap.w * ((float)w / 100);
	    h = bgPixmap.h;
	} else if (h && !w) {
	    w = bgPixmap.w;
	    h = bgPixmap.h * ((float)h / 100);
	}
	if (w > 1000)
	    w = 1000;
	if (h > 1000)
	    h = 1000;

	if (bgPixmap.w != w) {
	    bgPixmap.w = w;
	    changed++;
	}
	if (bgPixmap.h != h) {
	    bgPixmap.h = h;
	    changed++;
	}
    }
    if (!(flags & YValue)) {
	if (flags & XNegative)
	    flags |= YNegative;
	y = x;
    }

    if (!(flags & WidthValue) && geom[0] != '=') {
	x += bgPixmap.x;
	y += bgPixmap.y;
    } else {
	if (flags & XNegative)
	    x += 100;
	if (flags & YNegative)
	    y += 100;
    }
    MIN_IT(x, 100);
    MIN_IT(y, 100);
    MAX_IT(x, 0);
    MAX_IT(y, 0);
    if (bgPixmap.x != x) {
	bgPixmap.x = x;
	changed++;
    }
    if (bgPixmap.y != y) {
	bgPixmap.y = y;
	changed++;
    }
    return changed;
}

/* EXTPROTO */
void
resize_pixmap(void)
{
    XGCValues       gcvalue;
    GC              gc;
    int             tiled = 0;
    unsigned int    width = TermWin_TotalWidth();
    unsigned int    height = TermWin_TotalHeight();

    if (TermWin.pixmap)
	XFreePixmap(Xdisplay, TermWin.pixmap);

# ifndef XPM_BUFFERING
    if (bgPixmap.pixmap == None) {	/* So be it: I'm not using pixmaps */
	TermWin.pixmap = None;
	if (!(Options & Opt_transparent))
	    XSetWindowBackground(Xdisplay, TermWin.vt, PixColors[Color_bg]);
	return;
    }
# endif

    gcvalue.foreground = PixColors[Color_bg];
    gc = XCreateGC(Xdisplay, TermWin.vt, GCForeground, &gcvalue);

    if (bgPixmap.pixmap) {	/* we have a specified pixmap */
	int             w = bgPixmap.w, h = bgPixmap.h,
			x = bgPixmap.x, y = bgPixmap.y;

	/*
	 * don't zoom pixmap too much nor expand really small pixmaps
	 */
	if (w > 1000 || h > 1000)
	    w = 1;
	else if (width > (10 * xpmAttr.width)
		 || height > (10 * xpmAttr.height))
	    w = 0;		/* tile */

	if (w == 0) {
	    /* basic X tiling - let the X server do it */
	    TermWin.pixmap = XCreatePixmap(Xdisplay, TermWin.vt,
					   xpmAttr.width, xpmAttr.height,
					   Xdepth);
	    XCopyArea(Xdisplay, bgPixmap.pixmap, TermWin.pixmap,
		      gc, 0, 0, xpmAttr.width, xpmAttr.height, 0, 0);
	    tiled = 1;
	} else {
	    float           p, incr;
	    Pixmap          tmp;

	    TermWin.pixmap = XCreatePixmap(Xdisplay, TermWin.vt,
					   width, height, Xdepth);
	    /*
	     * horizontal scaling
	     */
	    incr = (float)xpmAttr.width;
	    p = 0;

	    if (w == 1) {	/* display one image, no horizontal scaling */
		incr = width;
		if (xpmAttr.width <= width) {
		    w = xpmAttr.width;
		    x = x * (width - w) / 100;	/* beware! order */
		    w += x;
		} else {
		    x = 0;
		    w = width;
		}
	    } else if (w < 10) {	/* fit W images across screen */
		incr *= w;
		x = 0;
		w = width;
	    } else {
		incr *= 100.0 / w;
		if (w < 100) {	/* contract */
		    w = (w * width) / 100;
		    if (x >= 0) {	/* position */
			float           pos;

			pos = (float)x / 100 * width - (w / 2);

			x = (width - w);
			if (pos <= 0)
			    x = 0;
			else if (pos < x)
			    x = pos;
		    } else {
			x = x * (width - w) / 100;	/* beware! order */
		    }
		    w += x;
		} else {	/* expand */
		    if (x > 0) {	/* position */
			float           pos;

			pos = (float)x / 100 * xpmAttr.width - (incr / 2);
			p = xpmAttr.width - (incr);
			if (pos <= 0)
			    p = 0;
			else if (pos < p)
			    p = pos;
		    }
		    x = 0;
		    w = width;
		}
	    }
	    incr /= width;

	    tmp = XCreatePixmap(Xdisplay, TermWin.vt,
				width, xpmAttr.height, Xdepth);
	    XFillRectangle(Xdisplay, tmp, gc, 0, 0, width, xpmAttr.height);

	    for ( /*nil */ ; x < w; x++, p += incr) {
		if (p >= xpmAttr.width)
		    p = 0;
		if (x < 0)
		    continue;
		/* copy one column from the original pixmap to the tmp pixmap */
		XCopyArea(Xdisplay, bgPixmap.pixmap, tmp, gc,
			  (int)p, 0, 1, xpmAttr.height, x, 0);
	    }

	    /*
	     * vertical scaling
	     */
	    incr = (float)xpmAttr.height;
	    p = 0;

	    if (h == 1) {	/* display one image, no vertical scaling */
		incr = height;
		if (xpmAttr.height <= height) {
		    h = xpmAttr.height;
		    y = y * (height - h) / 100;	/* beware! order */
		    h += y;
		} else {
		    y = 0;
		    h = height;
		}
	    } else if (h < 10) {	/* fit H images across screen */
		incr *= h;
		y = 0;
		h = height;
	    } else {
		incr *= 100.0 / h;
		if (h < 100) {	/* contract */
		    h = (h * height) / 100;
		    if (y >= 0) {	/* position */
			float           pos;

			pos = (float)y / 100 * height - (h / 2);

			y = (height - h);
			if (pos < 0.0f)
			    y = 0;
			else if (pos < y)
			    y = pos;
		    } else {
			y = y * (height - h) / 100;	/* beware! order */
		    }
		    h += y;
		} else {	/* expand */
		    if (y > 0) {	/* position */
			float           pos;

			pos = (float)y / 100 * xpmAttr.height - (incr / 2);
			p = xpmAttr.height - (incr);
			if (pos < 0)
			    p = 0;
			else if (pos < p)
			    p = pos;
		    }
		    y = 0;
		    h = height;
		}
	    }
	    incr /= height;

	    if (y > 0)
		XFillRectangle(Xdisplay, TermWin.pixmap, gc, 0, 0, width, y);
	    if (h < height)
		XFillRectangle(Xdisplay, TermWin.pixmap, gc, 0, h,
			       width, height - h + 1);
	    for ( /*nil */ ; y < h; y++, p += incr) {
		if (p >= xpmAttr.height)
		    p = 0;
		if (y < 0)
		    continue;
		/* copy one row from the tmp pixmap to the main pixmap */
		XCopyArea(Xdisplay, tmp, TermWin.pixmap, gc,
			  0, (int)p, width, 1, 0, y);
	    }
	    XFreePixmap(Xdisplay, tmp);
	}
    }
# ifdef XPM_BUFFERING
    if (TermWin.buf_pixmap)
	XFreePixmap(Xdisplay, TermWin.buf_pixmap);
    TermWin.buf_pixmap = XCreatePixmap(Xdisplay, TermWin.vt,
				       width, height, Xdepth);
    if (!bgPixmap.pixmap) {	/* we don't have a specified pixmap */
	TermWin.pixmap = XCreatePixmap(Xdisplay, TermWin.vt,
				       width, height, Xdepth);
	XFillRectangle(Xdisplay, TermWin.pixmap, gc, 0, 0, width, height);
    }
    if (tiled == 0) {
	XCopyArea(Xdisplay, TermWin.pixmap, TermWin.buf_pixmap, gc,
		  0, 0, width, height, 0, 0);
    } else {
	int             i, j, w, h;

	for (j = 0; j < height; j += xpmAttr.height) {
	    h = (j + xpmAttr.height > height) ? (height - j) : xpmAttr.height;
	    for (i = 0; i < width; i += xpmAttr.width) {
		w = (i + xpmAttr.width > width) ? (width - i) : xpmAttr.width;
		XCopyArea(Xdisplay, TermWin.pixmap, TermWin.buf_pixmap, gc,
			  0, 0, w, h, i, j);
	    }
	}
	XFreePixmap(Xdisplay, TermWin.pixmap);
	TermWin.pixmap = XCreatePixmap(Xdisplay, TermWin.vt,
				       width, height, Xdepth);
	XCopyArea(Xdisplay, TermWin.buf_pixmap, TermWin.pixmap, gc,
		  0, 0, width, height, 0, 0);
    }
    XSetWindowBackgroundPixmap(Xdisplay, TermWin.vt, TermWin.buf_pixmap);
# else				/* XPM_BUFFERING */
    XSetWindowBackgroundPixmap(Xdisplay, TermWin.vt, TermWin.pixmap);
# endif				/* XPM_BUFFERING */
    XFreeGC(Xdisplay, gc);

    XClearWindow(Xdisplay, TermWin.vt);

    XSync(Xdisplay, False);
}

/* EXTPROTO */
Pixmap
set_bgPixmap(const char *file)
{
    char           *f;

    assert(file != NULL);

    if (bgPixmap.pixmap != None) {
	XFreePixmap(Xdisplay, bgPixmap.pixmap);
	bgPixmap.pixmap = None;
    }
    XSetWindowBackground(Xdisplay, TermWin.vt, PixColors[Color_bg]);

    if (*file != '\0') {
/*      XWindowAttributes attr; */

	/*
	 * we already have the required attributes
	 */
/*      XGetWindowAttributes(Xdisplay, TermWin.vt, &attr); */

	xpmAttr.closeness = 30000;
	xpmAttr.colormap = Xcmap;
	xpmAttr.visual = Xvisual;
	xpmAttr.depth = Xdepth;
	xpmAttr.valuemask = (XpmCloseness | XpmColormap | XpmVisual |
			     XpmDepth | XpmSize | XpmReturnPixels);

	/* search environment variables here too */
	if ((f = (char *)File_find(file, ".xpm")) == NULL
	    || XpmReadFileToPixmap(Xdisplay, Xroot, f,
				   &bgPixmap.pixmap, NULL, &xpmAttr)) {
	    char           *p;

	    /* semi-colon delimited */
	    if ((p = STRCHR(file, ';')) == NULL)
		p = STRCHR(file, '\0');

	    print_error("couldn't load XPM file \"%.*s\"", (p - file), file);
	}
	FREE(f);
    }
    resize_pixmap();
    return bgPixmap.pixmap;
}

#endif				/* XPM_BACKGROUND */
