/*--------------------------------*-C-*---------------------------------*
 * File:	main.c
 *----------------------------------------------------------------------*
 * $Id: main.c,v 1.1 2002/12/06 23:08:02 earnie Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (C) 1992      John Bovey, University of Canterbury
 *				- original version
 * Copyright (C) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 *				- extensive modifications
 * Copyright (C) 1995      Garrett D'Amore <garrett@netcom.com>
 * Copyright (C) 1997      mj olesen <olesen@me.QueensU.CA>
 *				- extensive modifications
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
#define INTERN			/* assign all global vars to me */
#include "rxvt.h"		/* NECESSARY */
#include "main.intpro"		/* PROTOS for internal routines */

#include <X11/Xatom.h>

static Cursor   TermWin_cursor;	/* cursor for vt window */

static const char * const def_colorName[] =
{
    COLOR_FOREGROUND,
    COLOR_BACKGROUND,
/* low-intensity colors */
    "Black",			/* 0: black             (#000000) */
#ifndef NO_BRIGHTCOLOR
    "Red3",			/* 1: red               (#CD0000) */
    "Green3",			/* 2: green             (#00CD00) */
    "Yellow3",			/* 3: yellow            (#CDCD00) */
    "Blue3",			/* 4: blue              (#0000CD) */
    "Magenta3",			/* 5: magenta           (#CD00CD) */
    "Cyan3",			/* 6: cyan              (#00CDCD) */
    "AntiqueWhite",		/* 7: white             (#FAEBD7) */
/* high-intensity colors */
    "Grey25",			/* 8: bright black      (#404040) */
#endif				/* NO_BRIGHTCOLOR */
    "Red",			/* 1/9: bright red      (#FF0000) */
    "Green",			/* 2/10: bright green   (#00FF00) */
    "Yellow",			/* 3/11: bright yellow  (#FFFF00) */
    "Blue",			/* 4/12: bright blue    (#0000FF) */
    "Magenta",			/* 5/13: bright magenta (#FF00FF) */
    "Cyan",			/* 6/14: bright cyan    (#00FFFF) */
    "White",			/* 7/15: bright white   (#FFFFFF) */
#ifndef NO_CURSORCOLOR
    COLOR_CURSOR_BACKGROUND,
    COLOR_CURSOR_FOREGROUND,
#endif				/* ! NO_CURSORCOLOR */
    NULL,			/* Color_pointer                  */
    NULL			/* Color_border                   */
#ifndef NO_BOLDUNDERLINE
  , NULL,			/* Color_BD                       */
    NULL			/* Color_UL                       */
#endif				/* ! NO_BOLDUNDERLINE */
#ifdef KEEP_SCROLLCOLOR
  , COLOR_SCROLLBAR,
    COLOR_SCROLLTROUGH
#endif				/* KEEP_SCROLLCOLOR */
};

#ifdef MULTICHAR_SET
/* Multicharacter font names, roman fonts sized to match */
static const char * const def_mfontName[] =
{
    MFONT_LIST
};
#endif				/* MULTICHAR_SET */

static const char * const def_fontName[] =
{
    NFONT_LIST
};

/*----------------------------------------------------------------------*/
/* ARGSUSED */
/* INTPROTO */
XErrorHandler
xerror_handler(const Display *display, const XErrorEvent *event)
{
    print_error("XError: Request: %d . %d, Error: %d", event->request_code,
		event->minor_code, event->error_code);
    exit(EXIT_FAILURE);
    /* NOTREACHED */
}

/* color aliases, fg/bg bright-bold */
/* INTPROTO */
void
color_aliases(int idx)
{
    if (rs.color[idx] && isdigit(*(rs.color[idx]))) {
	int             i = atoi(rs.color[idx]);

	if (i >= 8 && i <= 15) {	/* bright colors */
	    i -= 8;
#ifndef NO_BRIGHTCOLOR
	    rs.color[idx] = rs.color[minBrightCOLOR + i];
	    return;
#endif
	}
	if (i >= 0 && i <= 7)	/* normal colors */
	    rs.color[idx] = rs.color[minCOLOR + i];
    }
}

/*
 * find if fg/bg matches any of the normal (low-intensity) colors
 */
/* INTPROTO */
void
set_colorfgbg(void)
{
    unsigned int    i;
    char           *p;
    int             fg = -1, bg = -1;
    static char     env_colorfgbg[] = "COLORFGBG=default;default;bg";

    for (i = Color_Black; i <= Color_White; i++) {
	if (PixColors[Color_fg] == PixColors[i]) {
	    fg = (i - Color_Black);
	    break;
	}
    }
    for (i = Color_Black; i <= Color_White; i++) {
	if (PixColors[Color_bg] == PixColors[i]) {
	    bg = (i - Color_Black);
	    break;
	}
    }

    p = STRCHR(env_colorfgbg, '=');
    p++;
    if (fg >= 0)
	sprintf(p, "%d;", fg);
    else
	STRCPY(p, "default;");
    p = STRCHR(p, '\0');
    if (bg >= 0)
	sprintf(p,
#ifdef XPM_BACKGROUND
		"default;"
#endif
		"%d", bg);
    else
	STRCPY(p, "default");
    putenv(env_colorfgbg);

#ifndef NO_BRIGHTCOLOR
    colorfgbg = DEFAULT_RSTYLE;
    for (i = minCOLOR; i <= maxCOLOR; i++) {
	if (PixColors[Color_fg] == PixColors[i]
# ifndef NO_BOLDUNDERLINE
	    && PixColors[Color_fg] == PixColors[Color_BD]
# endif				/* NO_BOLDUNDERLINE */
    /* if we wanted boldFont to have precedence */
# if 0				/* ifndef NO_BOLDFONT */
	    && TermWin.boldFont == NULL
# endif				/* NO_BOLDFONT */
	    )
	    colorfgbg = SET_FGCOLOR(colorfgbg, i);
	if (PixColors[Color_bg] == PixColors[i])
	    colorfgbg = SET_BGCOLOR(colorfgbg, i);
    }
#endif
}

/* INTPROTO */
void
Get_Colours(void)
{
    int             i;

    for (i = 0; i < (Xdepth <= 2 ? 2 : NRS_COLORS); i++) {
	const char     *msg = "can't load color \"%s\"";
	XColor          xcol;

	if (!rs.color[i])
	    continue;

	if (!XParseColor(Xdisplay, Xcmap, rs.color[i], &xcol)
	    || !XAllocColor(Xdisplay, Xcmap, &xcol)) {
	    print_error(msg, rs.color[i]);
#ifndef XTERM_REVERSE_VIDEO
	    if (i < 2 && (Options & Opt_reverseVideo)) {
		rs.color[i] = def_colorName[!i];
	    } else
#endif
	        rs.color[i] = def_colorName[i];
	    if (!rs.color[i])
		continue;
	    if (!XParseColor(Xdisplay, Xcmap, rs.color[i], &xcol)
		|| !XAllocColor(Xdisplay, Xcmap, &xcol)) {
		print_error(msg, rs.color[i]);
		switch (i) {
		case Color_fg:
		case Color_bg:
		/* fatal: need bg/fg color */
		    print_error("aborting");
		    exit(EXIT_FAILURE);
		    /* NOTREACHED */
		    break;
#ifndef NO_CURSORCOLOR
		case Color_cursor2:
		    xcol.pixel = PixColors[Color_fg];
		    break;
#endif				/* ! NO_CURSORCOLOR */
		case Color_pointer:
		    xcol.pixel = PixColors[Color_fg];
		    break;
		default:
		    xcol.pixel = PixColors[Color_bg];	/* None */
		    break;
		}
	    }
	}
	PixColors[i] = xcol.pixel;
    }

    if (Xdepth <= 2 || !rs.color[Color_pointer])
	PixColors[Color_pointer] = PixColors[Color_fg];
    if (Xdepth <= 2 || !rs.color[Color_border])
	PixColors[Color_border] = PixColors[Color_fg];

/*
 * get scrollBar/menuBar shadow colors
 *
 * The calculations of topShadow/bottomShadow values are adapted
 * from the fvwm window manager.
 */
#ifdef KEEP_SCROLLCOLOR
    if (Xdepth <= 2) {		/* Monochrome */
	PixColors[Color_scroll] = PixColors[Color_fg];
	PixColors[Color_topShadow] = PixColors[Color_bg];
	PixColors[Color_bottomShadow] = PixColors[Color_bg];
    } else {
	XColor          xcol, white;

    /* bottomShadowColor */
	xcol.pixel = PixColors[Color_scroll];
	XQueryColor(Xdisplay, Xcmap, &xcol);

	xcol.red = ((xcol.red) / 2);
	xcol.green = ((xcol.green) / 2);
	xcol.blue = ((xcol.blue) / 2);

	if (!XAllocColor(Xdisplay, Xcmap, &xcol)) {
	    print_error("can't allocate %s", "Color_bottomShadow");
	    xcol.pixel = PixColors[minCOLOR];
	}
	PixColors[Color_bottomShadow] = xcol.pixel;

    /* topShadowColor */
# ifdef PREFER_24BIT
	white.red = white.green = white.blue = (unsigned short) ~0;
	XAllocColor(Xdisplay, Xcmap, &white);
/*        XFreeColors(Xdisplay, Xcmap, &white.pixel, 1, ~0); */
# else
	white.pixel = WhitePixel(Xdisplay, Xscreen);
	XQueryColor(Xdisplay, Xcmap, &white);
# endif

	xcol.pixel = PixColors[Color_scroll];
	XQueryColor(Xdisplay, Xcmap, &xcol);

	xcol.red = max((white.red / 5), xcol.red);
	xcol.green = max((white.green / 5), xcol.green);
	xcol.blue = max((white.blue / 5), xcol.blue);

	xcol.red = min(white.red, (xcol.red * 7) / 5);
	xcol.green = min(white.green, (xcol.green * 7) / 5);
	xcol.blue = min(white.blue, (xcol.blue * 7) / 5);

	if (!XAllocColor(Xdisplay, Xcmap, &xcol)) {
	    print_error("can't allocate %s", "Color_topShadow");
	    xcol.pixel = PixColors[Color_White];
	}
	PixColors[Color_topShadow] = xcol.pixel;
    }
#endif				/* KEEP_SCROLLCOLOR */


}

/* Create_Windows() - Open and map the window */
/* INTPROTO */
void
Create_Windows(int argc, const char * const *argv)
{
    Cursor          cursor;
    XClassHint      classHint;
    XWMHints        wmHint;
#ifdef PREFER_24BIT
    XSetWindowAttributes attributes;

    Xdepth = DefaultDepth(Xdisplay, Xscreen);
    Xcmap = DefaultColormap(Xdisplay, Xscreen);
    Xvisual = DefaultVisual(Xdisplay, Xscreen);
/*
 * If depth is not 24, look for a 24bit visual.
 */
    if (Xdepth != 24) {
	XVisualInfo     vinfo;

	if (XMatchVisualInfo(Xdisplay, Xscreen, 24, TrueColor, &vinfo)) {
	    Xdepth = 24;
	    Xvisual = vinfo.visual;
	    Xcmap = XCreateColormap(Xdisplay, RootWindow(Xdisplay, Xscreen),
				    Xvisual, AllocNone);
	}
    }
#endif

/* grab colors before netscape does */
    Get_Colours();

    change_font(1, NULL);
    szhints_set();

/* parent window - reverse video so we can see placement errors
 * sub-window placement & size in resize_subwindows()
 */

#ifdef PREFER_24BIT
    attributes.background_pixel = PixColors[Color_fg];
    attributes.border_pixel = PixColors[Color_fg];
    attributes.colormap = Xcmap;
    TermWin.parent[0] = XCreateWindow(Xdisplay, Xroot,
				   szHint.x, szHint.y,
				   szHint.width, szHint.height,
				   TermWin.ext_bwidth,
				   Xdepth, InputOutput,
				   Xvisual,
				   CWBackPixel | CWBorderPixel | CWColormap,
				   &attributes);
#else
    TermWin.parent[0] = XCreateSimpleWindow(Xdisplay, Xroot,
					 szHint.x, szHint.y,
					 szHint.width, szHint.height,
					 TermWin.ext_bwidth,
					 PixColors[Color_bg],
					 PixColors[Color_fg]);
#endif
    xterm_seq(XTerm_title, rs.title);
    xterm_seq(XTerm_iconName, rs.iconName);
/* ignore warning about discarded `const' */
    classHint.res_name = (char *) rs.name;
    classHint.res_class = (char *) APL_CLASS;
    wmHint.input = True;
    wmHint.initial_state = (Options & Opt_iconic ? IconicState : NormalState);
    wmHint.window_group = TermWin.parent[0];
    wmHint.flags = (InputHint | StateHint | WindowGroupHint);

    XSetWMProperties(Xdisplay, TermWin.parent[0], NULL, NULL, (char **) argv,
		     argc, &szHint, &wmHint, &classHint);

    XSelectInput(Xdisplay, TermWin.parent[0],
		 (KeyPressMask | FocusChangeMask
		  | VisibilityChangeMask
		  | StructureNotifyMask));

/* vt cursor: Black-on-White is standard, but this is more popular */
    TermWin_cursor = XCreateFontCursor(Xdisplay, XC_xterm);
    {
	XColor          fg, bg;

	fg.pixel = PixColors[Color_pointer];
	XQueryColor(Xdisplay, Xcmap, &fg);
	bg.pixel = PixColors[Color_bg];
	XQueryColor(Xdisplay, Xcmap, &bg);
	XRecolorCursor(Xdisplay, TermWin_cursor, &fg, &bg);
    }

/* cursor (menuBar/scrollBar): Black-on-White */
    cursor = XCreateFontCursor(Xdisplay, XC_left_ptr);

/* the vt window */
    TermWin.vt = XCreateSimpleWindow(Xdisplay, TermWin.parent[0],
				     0, 0,
				     szHint.width, szHint.height,
				     0,
				     PixColors[Color_fg],
				     PixColors[Color_bg]);
#ifdef DEBUG_X
    XStoreName(Xdisplay, TermWin.vt, "vt window");
#endif

    XDefineCursor(Xdisplay, TermWin.vt, TermWin_cursor);
    XSelectInput(Xdisplay, TermWin.vt,
		 (ExposureMask | ButtonPressMask | ButtonReleaseMask |
		  Button1MotionMask | Button3MotionMask));

/* scrollBar: size doesn't matter */
    scrollBar.win = XCreateSimpleWindow(Xdisplay, TermWin.parent[0],
					0, 0,
					1, 1,
					0,
					PixColors[Color_fg],
					PixColors[Color_bg]);
#ifdef DEBUG_X
    XStoreName(Xdisplay, scrollBar.win, "scrollbar");
#endif

    XDefineCursor(Xdisplay, scrollBar.win, cursor);
    XSelectInput(Xdisplay, scrollBar.win,
		 (ExposureMask | ButtonPressMask | ButtonReleaseMask |
		  Button1MotionMask | Button2MotionMask | Button3MotionMask));

    { /* ONLYIF MENUBAR */
	create_menuBar(cursor);
    }

#ifdef XPM_BACKGROUND
    if (rs.backgroundPixmap != NULL && !(Options & Opt_transparent)) {
	const char     *p = rs.backgroundPixmap;

	if ((p = STRCHR(p, ';')) != NULL) {
	    p++;
	    scale_pixmap(p);
	}
	set_bgPixmap(rs.backgroundPixmap);
	scr_touch(True);
    }
# ifdef XPM_BUFFERING
    else {
	set_bgPixmap("");
	scr_touch(True);
    }
# endif
#endif

/* graphics context for the vt window */
    {
	XGCValues       gcvalue;

	gcvalue.font = TermWin.font->fid;
	gcvalue.foreground = PixColors[Color_fg];
	gcvalue.background = PixColors[Color_bg];
	gcvalue.graphics_exposures = 0;
	TermWin.gc = XCreateGC(Xdisplay, TermWin.vt,
			       GCForeground | GCBackground |
			       GCFont | GCGraphicsExposures,
			       &gcvalue);
    }
}
/* window resizing - assuming the parent window is the correct size */
/* INTPROTO */
void
resize_subwindows(int width, int height)
{
    int             x = 0, y = 0;
#ifdef RXVT_GRAPHICS
    int             old_width = TermWin.width, old_height = TermWin.height;
#endif

    TermWin.width = TermWin.ncol * TermWin.fwidth;
    TermWin.height = TermWin.nrow * TermWin.fheight;

    szHint.width = width;
    szHint.height = height;

/* size and placement */
    if (scrollbar_visible()) {
	Resize_scrollBar();
	x = (SB_WIDTH + 2 * sb_shadow);	/* placement of vt window */
	width -= x;
	if (Options & Opt_scrollBar_right)
	    x = 0;		/* scrollbar on right so vt window at left */
    }
    { /* ONLYIF MENUBAR */
	if (menubar_visible()) {
	    y = menuBar_TotalHeight();	/* for placement of vt window */
	    Resize_menuBar(x, 0, width, y);
	}
    }
    XMoveResizeWindow(Xdisplay, TermWin.vt, x, y, width, height + 1);

#ifdef RXVT_GRAPHICS
    if (old_width)
	Gr_Resize(old_width, old_height);
#endif

    scr_clear();
    resize_pixmap();
    XSync(Xdisplay, False);
}

/* EXTPROTO */
void
resize_all_windows(void)
{
    szhints_recalc();
    XSetWMNormalHints(Xdisplay, TermWin.parent[0], &szHint);
    AddToCNQueue(szHint.width, szHint.height);
    XResizeWindow(Xdisplay, TermWin.parent[0], szHint.width, szHint.height);
    resize_window(szHint.width, szHint.height);
}

/*
 * Redraw window after exposure or size change
 * width/height are those of the parent
 */
/* EXTPROTO */
void
resize_window(unsigned int width, unsigned int height)
{
    int             new_ncol, new_nrow;
    static int      old_width, old_height = -1;

    new_ncol = (width - szHint.base_width) / TermWin.fwidth;
    new_nrow = (height - szHint.base_height) / TermWin.fheight;
    if (old_height == -1
	|| (new_ncol != TermWin.ncol)
	|| (new_nrow != TermWin.nrow)) {
	int             curr_screen = -1;

    /* scr_reset only works on the primary screen */
	if (old_height != -1) {	/* this is not the first time thru */
	    selection_clear();
	    curr_screen = scr_change_screen(PRIMARY);
	}
	TermWin.ncol = new_ncol;
	TermWin.nrow = new_nrow;

	resize_subwindows(width, height);
	scr_reset();

	if (curr_screen >= 0)	/* this is not the first time thru */
	    scr_change_screen(curr_screen);
    } else if (width != old_width || height != old_height)
	resize_subwindows(width, height);
    old_width = width;
    old_height = height;
}

/*
 * Set the width/height of the window in characters.  Units are pixels.
 * good for toggling 80/132 columns
 */
/* EXTPROTO */
void
set_widthheight(unsigned int width, unsigned int height)
{
    XWindowAttributes wattr;

    if (width == 0 || height == 0) {
	XGetWindowAttributes(Xdisplay, Xroot, &wattr);
	if (width == 0)
	    width = wattr.width	- szHint.base_width;
	if (height == 0)
	    height = wattr.height - szHint.base_height;
    }

    if (width != TermWin.width || height != TermWin.height) {
	width = szHint.base_width + width;
	height = szHint.base_height + height;

	AddToCNQueue(width, height);
	XResizeWindow(Xdisplay, TermWin.parent[0], width, height);
	resize_window(width, height);
#ifdef USE_XIM
	IMSetStatusPosition();
#endif
    }
}

/* INTPROTO */
void
szhints_set(void)
{
    int             x, y, flags;
    unsigned int    width, height;

    szHint.flags = PMinSize | PResizeInc | PBaseSize | PWinGravity;
    szHint.win_gravity = NorthWestGravity;
    szHint.min_aspect.x = szHint.min_aspect.y = 1;

    flags = (rs.geometry ? XParseGeometry(rs.geometry, &x, &y, &width, &height)
		         : 0);

    if (flags & WidthValue) {
	TermWin.ncol = width;
	szHint.flags |= USSize;
    }
    if (flags & HeightValue) {
	TermWin.nrow = height;
	szHint.flags |= USSize;
    }
    TermWin.width = TermWin.ncol * TermWin.fwidth;
    TermWin.height = TermWin.nrow * TermWin.fheight;
    szhints_recalc();

    if (flags & XValue) {
	if (flags & XNegative) {
	    x += (DisplayWidth(Xdisplay, Xscreen) - szHint.width
		  - 2 * TermWin.ext_bwidth);
	    szHint.win_gravity = NorthEastGravity;
	}
	szHint.x = x;
	szHint.flags |= USPosition;
    }
    if (flags & YValue) {
	if (flags & YNegative) {
	    y += (DisplayHeight(Xdisplay, Xscreen) - szHint.height
		  - 2 * TermWin.ext_bwidth);
	    if (szHint.win_gravity == NorthEastGravity)
		szHint.win_gravity = SouthEastGravity;
	    else
		szHint.win_gravity = SouthWestGravity;
	}
	szHint.y = y;
	szHint.flags |= USPosition;
    }
}

/* INTPROTO */
void
szhints_recalc(void)
{
    szHint.base_width = 2 * TermWin.int_bwidth;
    szHint.base_height = 2 * TermWin.int_bwidth;
    szHint.base_width += (scrollbar_visible() ? (SB_WIDTH + 2 * sb_shadow) : 0);
    szHint.base_height += (menubar_visible() ? menuBar_TotalHeight() : 0);
    szHint.width_inc = TermWin.fwidth;
    szHint.height_inc = TermWin.fheight;
    szHint.min_width = szHint.base_width + szHint.width_inc;
    szHint.min_height = szHint.base_height + szHint.height_inc;
    szHint.width = szHint.base_width + TermWin.width;
    szHint.height = szHint.base_height + TermWin.height;
}

/* xterm sequences - title, iconName, color (exptl) */
/* INTPROTO */
void
set_title(const char *str)
{
#ifndef SMART_WINDOW_TITLE
    XStoreName(Xdisplay, TermWin.parent[0], str);
#else
    char           *name;

    if (XFetchName(Xdisplay, TermWin.parent[0], &name) == 0)
	name = NULL;
    if (name == NULL || STRCMP(name, str))
	XStoreName(Xdisplay, TermWin.parent[0], str);
    if (name)
	XFree(name);
#endif
}

/* INTPROTO */
void
set_iconName(const char *str)
{
#ifndef SMART_WINDOW_TITLE
    XSetIconName(Xdisplay, TermWin.parent[0], str);
#else
    char           *name;

    if (XGetIconName(Xdisplay, TermWin.parent[0], &name))
	name = NULL;
    if (name == NULL || STRCMP(name, str))
	XSetIconName(Xdisplay, TermWin.parent[0], str);
    if (name)
	XFree(name);
#endif
}

#ifdef XTERM_COLOR_CHANGE
/* INTPROTO */
void
set_window_color(int idx, const char *color)
{
    const char     *msg = "can't load color \"%s\"";
    XColor          xcol;
    int             i;

    if (color == NULL || *color == '\0')
	return;

/* handle color aliases */
    if (isdigit(*color)) {
	i = atoi(color);
	if (i >= 8 && i <= 15) {	/* bright colors */
	    i -= 8;
# ifndef NO_BRIGHTCOLOR
	    PixColors[idx] = PixColors[minBrightCOLOR + i];
	    goto Done;
# endif
	}
	if (i >= 0 && i <= 7) {	/* normal colors */
	    PixColors[idx] = PixColors[minCOLOR + i];
	    goto Done;
	}
    }
    if (!XParseColor(Xdisplay, Xcmap, color, &xcol)
	|| !XAllocColor(Xdisplay, Xcmap, &xcol)) {
	print_error(msg, color);
	return;
    }
/* XStoreColor (Xdisplay, Xcmap, XColor*); */

/*
 * FIXME: should free colors here, but no idea how to do it so instead,
 * so just keep gobbling up the colormap
 */
# if 0
    for (i = Color_Black; i <= Color_White; i++)
	if (PixColors[idx] == PixColors[i])
	    break;
    if (i > Color_White) {
    /* fprintf (stderr, "XFreeColors: PixColors [%d] = %lu\n", idx, PixColors [idx]); */
	XFreeColors(Xdisplay, Xcmap, (PixColors + idx), 1,
		    DisplayPlanes(Xdisplay, Xscreen));
    }
# endif

    PixColors[idx] = xcol.pixel;

/* XSetWindowAttributes attr; */
/* Cursor cursor; */
  Done:
    if (idx == Color_bg && !(Options & Opt_transparent))
	XSetWindowBackground(Xdisplay, TermWin.vt, PixColors[Color_bg]);

/* handle Color_BD, scrollbar background, etc. */

    set_colorfgbg();
    {
	XColor          fg, bg;

	fg.pixel = PixColors[Color_pointer];
	XQueryColor(Xdisplay, Xcmap, &fg);
	bg.pixel = PixColors[Color_bg];
	XQueryColor(Xdisplay, Xcmap, &bg);
	XRecolorCursor(Xdisplay, TermWin_cursor, &fg, &bg);
    }
/* the only reasonable way to enforce a clean update */
    scr_poweron();
}
#else
# define set_window_color(idx,color)	((void)0)
#endif				/* XTERM_COLOR_CHANGE */

/*
 * XTerm escape sequences: ESC ] Ps;Pt BEL
 *       0 = change iconName/title
 *       1 = change iconName
 *       2 = change title
 *      46 = change logfile (not implemented)
 *      50 = change font
 *
 * rxvt extensions:
 *      10 = menu (may change in future)
 *      20 = bg pixmap
 *      39 = change default fg color
 *      49 = change default bg color
 *      55 = dump scrollback buffer and all of screen
 */
/* EXTPROTO */
void
xterm_seq(int op, const char *str)
{
    int             changed = 0;
    int             fd;

    assert(str != NULL);
    switch (op) {
    case XTerm_name:
	set_title(str);
    /* FALLTHROUGH */
    case XTerm_iconName:
	set_iconName(str);
	break;
    case XTerm_title:
	set_title(str);
	break;
    case XTerm_Menu:
    /*
     * menubar_dispatch() violates the constness of the string,
     * so DON'T do it here
     */
	break;
    case XTerm_Pixmap:
	if (*str != ';') {
	    scale_pixmap("");	/* reset to default scaling */
	    set_bgPixmap(str);	/* change pixmap */
	    scr_touch(True);
	}
	while ((str = STRCHR(str, ';')) != NULL) {
	    str++;
	    changed += scale_pixmap(str);
	}
	if (changed) {
	    resize_pixmap();
	    scr_touch(True);
	}
	break;

    case XTerm_restoreFG:
	set_window_color(Color_fg, str);
	break;
    case XTerm_restoreBG:
	set_window_color(Color_bg, str);
	break;
    case XTerm_logfile:
	break;
    case XTerm_font:
	change_font(0, str);
	break;
    case XTerm_dumpscreen:	/* no error notices */
	if ((fd = open(str, O_RDWR | O_CREAT | O_EXCL, 0600)) >= 0) {
	    scr_dump(fd);
	    close(fd);
	}
	break;
    }
}

/* change_font() - Switch to a new font */
/*
 * init = 1   - initialize
 *
 * fontname == FONT_UP  - switch to bigger font
 * fontname == FONT_DN  - switch to smaller font
 */
/* EXTPROTO */
void
change_font(int init, const char *fontname)
{
    const char     *msg = "can't load font \"%s\"";
    int             fh, fw, recheckfonts;
    int             idx = 0;	/* index into rs.font[] */
    XFontStruct    *xfont;
    static char    *newfont[NFONTS];
    static int      fnum;		/* logical font number */
#ifndef NO_BOLDFONT
    static XFontStruct *boldFont;
#endif
#ifdef MULTICHAR_SET
    int             i;
    char           *c, *enc;
    char            tmpbuf[64];
#endif

#if (FONT0_IDX == 0)
# define IDX2FNUM(i)	(i)
# define FNUM2IDX(f)	(f)
#else
# define IDX2FNUM(i)	(i == 0 ? FONT0_IDX : (i <= FONT0_IDX ? (i-1) : i))
# define FNUM2IDX(f)	(f == FONT0_IDX ? 0 : (f < FONT0_IDX  ? (f+1) : f))
#endif
#define FNUM_RANGE(i)	(i <= 0 ? 0 : (i >= NFONTS ? (NFONTS-1) : i))

    if (init) {
#ifndef NO_BOLDFONT
	boldFont = NULL;
#endif
	fnum = FONT0_IDX;	/* logical font number */
    } else {
	switch (fontname[0]) {
	case '\0':
	    fnum = FONT0_IDX;
	    fontname = NULL;
	    break;

	/* special (internal) prefix for font commands */
	case FONT_CMD:
	    idx = atoi(fontname + 1);
	    switch (fontname[1]) {
	    case '+':		/* corresponds to FONT_UP */
		fnum += (idx ? idx : 1);
		fnum = FNUM_RANGE(fnum);
		break;

	    case '-':		/* corresponds to FONT_DN */
		fnum += (idx ? idx : -1);
		fnum = FNUM_RANGE(fnum);
		break;

	    default:
		if (fontname[1] != '\0' && !isdigit(fontname[1]))
		    return;
		if (idx < 0 || idx >= (NFONTS))
		    return;
		fnum = IDX2FNUM(idx);
		break;
	    }
	    fontname = NULL;
	    break;

	default:
	    if (fontname != NULL) {
	    /* search for existing fontname */
		for (idx = 0; idx < NFONTS; idx++) {
		    if (!STRCMP(rs.font[idx], fontname)) {
			fnum = IDX2FNUM(idx);
			fontname = NULL;
			break;
		    }
		}
	    } else
		return;
	    break;
	}
    /* re-position around the normal font */
	idx = FNUM2IDX(fnum);

	if (fontname != NULL) {
	    char           *name;

	    xfont = XLoadQueryFont(Xdisplay, fontname);
	    if (!xfont)
		return;

	    name = MALLOC(STRLEN(fontname + 1) * sizeof(char));

	    if (name == NULL) {
		XFreeFont(Xdisplay, xfont);
		return;
	    }
	    STRCPY(name, fontname);
	    if (newfont[idx] != NULL)
		FREE(newfont[idx]);
	    newfont[idx] = name;
	    rs.font[idx] = newfont[idx];
	}
    }
    if (TermWin.font)
	XFreeFont(Xdisplay, TermWin.font);

/* load font or substitute */
    xfont = XLoadQueryFont(Xdisplay, rs.font[idx]);
    if (!xfont) {
	print_error(msg, rs.font[idx]);
	rs.font[idx] = "fixed";
	xfont = XLoadQueryFont(Xdisplay, rs.font[idx]);
	if (!xfont) {
	    print_error(msg, rs.font[idx]);
	    goto Abort;
	}
    }
    TermWin.font = xfont;

#ifndef NO_BOLDFONT
/* fail silently */
    if (init && rs.boldFont != NULL)
	boldFont = XLoadQueryFont(Xdisplay, rs.boldFont);
#endif

/* alter existing GC */
    if (!init) {
	XSetFont(Xdisplay, TermWin.gc, TermWin.font->fid);
	menubar_expose();
    }

/* set the sizes */
    fw = get_fontwidest(TermWin.font);
    fh = TermWin.font->ascent + TermWin.font->descent;
    if (fw == TermWin.font->min_bounds.width)
	TermWin.fprop = 0;	/* Mono-spaced (fixed width) font */
    else
	TermWin.fprop = 1;	/* Proportional font */
    recheckfonts = !(fw == TermWin.fwidth && fh == TermWin.fheight);
    TermWin.fwidth = fw;
    TermWin.fheight = fh;

/* check that size of boldFont is okay */
#ifndef NO_BOLDFONT
    if (recheckfonts) {
	TermWin.boldFont = NULL;
	if (boldFont != NULL) {
	    fw = get_fontwidest(boldFont);
	    fh = boldFont->ascent + boldFont->descent;
	    if (fw <= TermWin.fwidth && fh <= TermWin.fheight)
		TermWin.boldFont = boldFont;
	    TermWin.bprop = !(fw == TermWin.fwidth /* && fh == TermWin.fheight */ );
	}
    }
#endif				/* NO_BOLDFONT */

#ifdef MULTICHAR_SET
    if (TermWin.mfont)
	XFreeFont(Xdisplay, TermWin.mfont);

/* load font or substitute */
    if (rs.mfont[idx] == NULL
	|| (xfont = XLoadQueryFont(Xdisplay, rs.mfont[idx])) == NULL) {
    /* TODO: improve this */
	switch(encoding_method) {
	case GB:
	    c = "-*-%.2d-*-gb2312*-*";
	    enc = "GB";
	    break;
	case BIG5:
	    c = "-*-%.2d-*-big5*-*";
	    enc = "BIG5";
	    break;
	case EUCJ:
	case SJIS:
	    c = "-*-%.2d-*-jisx0208*-*";
	    enc = "EUCJ/SJIS";
	    break;
	case EUCKR:
	    c = "-*-%.2d-*-ksc5601*-*";
	    enc = "EUCKR";
	/* FALLTHROUGH */
	default:
	    break;
	}
	for (i = 0; i < fh / 2; i++) {
	    sprintf(tmpbuf, c, fh - i);
	    xfont = XLoadQueryFont(Xdisplay, tmpbuf);
	    if (xfont) {
		rs.mfont[idx] = MALLOC(STRLEN(tmpbuf) + 1);
		STRCPY(rs.mfont[idx], tmpbuf);
		break;
	    }
	}
	if (xfont == NULL)
	    print_error("no similar multichar font: encoding %s; size %d",
			enc, fh);
    }
    TermWin.mfont = xfont;

    if (recheckfonts)
    /* XXX: This checks what? */
	if (TermWin.mfont != NULL) {
	    fw = get_fontwidest(TermWin.mfont);
	    fh = TermWin.mfont->ascent + TermWin.mfont->descent;
	    if (fw > (TermWin.fwidth * 2) || fh > TermWin.fheight)
		TermWin.mfont = NULL;
	    TermWin.mprop = !(fw == TermWin.fwidth /* && fh == TermWin.fheight */ );
	}
#endif				/* MULTICHAR_SET */

    set_colorfgbg();

    TermWin.width = TermWin.ncol * TermWin.fwidth;
    TermWin.height = TermWin.nrow * TermWin.fheight;

    if (!init) {
	resize_all_windows();
	scr_touch(True);
    }
    return;
  Abort:
    print_error("aborting");	/* fatal problem */
    exit(EXIT_FAILURE);
#undef IDX2FNUM
#undef FNUM2IDX
#undef FNUM_RANGE
    /* NOTREACHED */
}
#ifdef STRICT_FONT_CHECKING
/* INTPROTO */
int
get_fontwidest(XFontStruct *f)
{
    int             i, cw, fw = 0;

    if (f->min_bounds.width == f->max_bounds.width)
	return f->min_bounds.width;
    if (f->per_char == NULL)
	return f->max_bounds.width;
    for (i = f->max_char_or_byte2 - f->min_char_or_byte2; --i >= 0; ) {
	cw = f->per_char[i].width;
	MAX_IT(fw, cw);
    }
    return fw;
}
#endif
/* ------------------------------------------------------------------------- */
/* INTPROTO */
void
init_vars(void)
{
    Xdisplay = NULL;
    Options = Opt_scrollBar | Opt_scrollTtyOutput;
    sb_shadow = 0;
    TermWin.ncol = 80;
    TermWin.nrow = 24;
    TermWin.mapped = 0;
    TermWin.int_bwidth = INTERNALBORDERWIDTH;
    TermWin.ext_bwidth = EXTERNALBORDERWIDTH;
    TermWin.saveLines = SAVELINES;
    want_refresh = 1;
    scrollBar.win = 0;
#if (MENUBAR_MAX)
    menuBar.win = 0;
#endif

#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
/* recognized when combined with HOTKEY */
    ks_bigfont = XK_greater;
    ks_smallfont = XK_less;
#endif
#ifndef NO_BRIGHTCOLOR
    colorfgbg = DEFAULT_RSTYLE;
#endif
#ifndef NO_NEW_SELECTION
    selection_style = NEW_SELECT;
#else
    selection_style = OLD_SELECT;
#endif
}

/* ------------------------------------------------------------------------- */
/* INTPROTO */
const char    **
init_resources(int argc, const char * const *argv)
{
    int             i, r_argc;
    char           *val;
    const char     *tmp;
    const char    **cmd_argv, **r_argv, **t2;

/*
 * Look for -exec option.  Find => split and make cmd_argv[] of command args
 */
    for (r_argc = 0; r_argc < argc; r_argc++)
	if (!STRCMP(argv[r_argc], "-e") || !STRCMP(argv[r_argc], "-exec"))
	    break;
    r_argv = (const char **) MALLOC(sizeof(char *) * (r_argc + 1));
    for (i = 0; i < r_argc; i++)
	r_argv[i] = (const char *) argv[i];
    r_argv[i] = NULL;
    if (r_argc == argc)
	cmd_argv = NULL;
    else {
	cmd_argv = (const char **) MALLOC(sizeof(char *) * (argc - r_argc));
	for (i = 0; i < argc - r_argc - 1; i++)
	    cmd_argv[i] = (const char *) argv[i + r_argc + 1];
	cmd_argv[i] = NULL;
    }

/* clear all resources */
    for (i = 0, t2 = (const char **)&rs; i < sizeof(rs) / sizeof(char *); i++)
	t2[i] = NULL;

    rs.name = r_basename(argv[0]);
    if (cmd_argv != NULL && cmd_argv[0] != NULL)
	rs.iconName = rs.title = r_basename(cmd_argv[0]);
/*
 * Open display, get options/resources and create the window
 */
    if ((rs.display_name = getenv("DISPLAY")) == NULL)
	rs.display_name = ":0";

    get_options(r_argc, r_argv);

    FREE(r_argv);

#ifdef LOCAL_X_IS_UNIX
    if (rs.display_name[0] == ':') {
	val = MALLOC(5 + STRLEN(rs.display_name));
	STRCPY(val, "unix");
	STRCAT(val, rs.display_name);
	Xdisplay = XOpenDisplay(val);
	FREE(val);
    }
#endif
	
    if (Xdisplay == NULL
	&& (Xdisplay = XOpenDisplay(rs.display_name)) == NULL) {
	print_error("can't open display %s", rs.display_name);
	exit(EXIT_FAILURE);
    }
#ifdef INEXPENSIVE_LOCAL_X_CALLS
    /* it's hard to determine further if we're on a local display or not */
    if (rs.display_name[0] == ':'
	|| STRNCMP(rs.display_name, "unix:", 5))
	display_is_local = 1;
    else
	display_is_local = 0;
#endif

    extract_resources(Xdisplay, rs.name);

#if ! defined(XTERM_SCROLLBAR) && ! defined(NEXT_SCROLLBAR)
    if (!(Options & Opt_scrollBar_floating))
	sb_shadow = SHADOW;
#endif

/*
 * set any defaults not already set
 */
    if (!rs.title)
	rs.title = rs.name;
    if (!rs.iconName)
	rs.iconName = rs.title;
    if (rs.saveLines && (i = atoi(rs.saveLines)) >= 0)
	TermWin.saveLines = (int16_t) i;
#ifndef NO_FRILLS
    if (rs.int_bwidth && (i = atoi(rs.int_bwidth)) >= 0)
	TermWin.int_bwidth = (int16_t) i;
    if (rs.ext_bwidth && (i = atoi(rs.ext_bwidth)) >= 0)
	TermWin.ext_bwidth = (int16_t) i;
#endif

/* no point having a scrollbar without having any scrollback! */
    if (!TermWin.saveLines)
	Options &= ~Opt_scrollBar;

#ifdef PRINTPIPE
    if (!rs.print_pipe)
	rs.print_pipe = PRINTPIPE;
#endif
    if (!rs.cutchars)
	rs.cutchars = CUTCHARS;
#ifndef NO_BACKSPACE_KEY
    if (!rs.backspace_key)
# ifdef DEFAULT_BACKSPACE
	key_backspace = DEFAULT_BACKSPACE;
# else
	key_backspace = "DEC";	/* can toggle between \033 or \177 */
# endif
    else {
	val = strdup(rs.backspace_key);
	(void) Str_trim(val);
	(void) Str_escaped(val);
	key_backspace = val;
    }
#endif
#ifndef NO_DELETE_KEY
    if (!rs.delete_key)
# ifdef DEFAULT_DELETE
	key_delete = DEFAULT_DELETE;
# else
	key_delete = "\033[3~";
# endif
    else {
	val = strdup(rs.delete_key);
	(void) Str_trim(val);
	(void) Str_escaped(val);
	key_delete = val;
    }
#endif

    if (rs.selectstyle) {
	if (STRNCASECMP(rs.selectstyle, "oldword", 7) == 0)
	    selection_style = OLD_WORD_SELECT;
#ifndef NO_OLD_SELECTION
	else if (STRNCASECMP(rs.selectstyle, "old", 3) == 0)
	    selection_style = OLD_SELECT;
#endif
    }

#ifndef NO_BOLDFONT
    if (rs.font[0] == NULL && rs.boldFont != NULL) {
	rs.font[0] = rs.boldFont;
	rs.boldFont = NULL;
    }
#endif
    for (i = 0; i < NFONTS; i++) {
	if (!rs.font[i])
	    rs.font[i] = def_fontName[i];
#ifdef MULTICHAR_SET
	if (!rs.mfont[i])
	    rs.mfont[i] = def_mfontName[i];
#endif
    }
    TermWin.fontset = NULL;

#ifdef XTERM_REVERSE_VIDEO
/* this is how xterm implements reverseVideo */
    if (Options & Opt_reverseVideo) {
	if (!rs.color[Color_fg])
	    rs.color[Color_fg] = def_colorName[Color_bg];
	if (!rs.color[Color_bg])
	    rs.color[Color_bg] = def_colorName[Color_fg];
    }
#endif

    for (i = 0; i < NRS_COLORS; i++)
	if (!rs.color[i])
	    rs.color[i] = def_colorName[i];

#ifndef XTERM_REVERSE_VIDEO
/* this is how we implement reverseVideo */
    if (Options & Opt_reverseVideo)
	SWAP_IT(rs.color[Color_fg], rs.color[Color_bg], tmp);
#endif

/* convenient aliases for setting fg/bg to colors */
    color_aliases(Color_fg);
    color_aliases(Color_bg);
#ifndef NO_CURSORCOLOR
    color_aliases(Color_cursor);
    color_aliases(Color_cursor2);
#endif				/* NO_CURSORCOLOR */
    color_aliases(Color_pointer);
    color_aliases(Color_border);
#ifndef NO_BOLDUNDERLINE
    color_aliases(Color_BD);
    color_aliases(Color_UL);
#endif				/* NO_BOLDUNDERLINE */

    return cmd_argv;
}

/* ------------------------------------------------------------------------- */
/* INTPROTO */
void
init_env(void)
{
    int             i;
    unsigned int    u;
    char           *val;
/* these don't need to be static but do so to placate some mem checkers */
    static char    *env_windowid, *env_display, *env_term;

#ifdef DISPLAY_IS_IP
/* Fixup display_name for export over pty to any interested terminal
 * clients via "ESC[7n" (e.g. shells).  Note we use the pure IP number
 * (for the first non-loopback interface) that we get from
 * network_display().  This is more "name-resolution-portable", if you
 * will, and probably allows for faster x-client startup if your name
 * server is beyond a slow link or overloaded at client startup.  Of
 * course that only helps the shell's child processes, not us.
 *
 * Giving out the display_name also affords a potential security hole
 */
    rs.display_name = (const char *) val = network_display(rs.display_name);
    if (val == NULL)
#endif				/* DISPLAY_IS_IP */
	val = XDisplayString(Xdisplay);
    if (rs.display_name == NULL)
	rs.display_name = val;	/* use broken `:0' value */

    i = STRLEN(val);
    env_display = MALLOC((i + 9) * sizeof(char));
    sprintf(env_display, "DISPLAY=%s", val);

    /* avoiding the math library:
     * i = (int)(ceil(log10((unsigned int)TermWin.parent[0]))) */
    for (i = 0, u = (unsigned int)TermWin.parent[0]; u; u /= 10, i++);
    MAX_IT(i, 1);
    env_windowid = MALLOC((i + 10) * sizeof(char));
    sprintf(env_windowid, "WINDOWID=%u", (unsigned int)TermWin.parent[0]);

/* add entries to the environment:
 * @ DISPLAY:   in case we started with -display
 * @ WINDOWID:  X window id number of the window
 * @ COLORTERM: terminal sub-name and also indicates its color
 * @ TERM:      terminal name
 * @ TERMINFO:	path to terminfo directory
 */
    putenv(env_display);
    putenv(env_windowid);
#ifdef RXVT_TERMINFO
    putenv("TERMINFO=" RXVT_TERMINFO);
#endif
    if (Xdepth <= 2)
	putenv("COLORTERM=" COLORTERMENV "-mono");
    else
	putenv("COLORTERM=" COLORTERMENVFULL);
    if (rs.term_name != NULL) {
	env_term = MALLOC((STRLEN(rs.term_name) + 6) * sizeof(char));
	sprintf(env_term, "TERM=%s", rs.term_name);
	putenv(env_term);
    } else
	putenv("TERM=" TERMENV);

#ifdef HAVE_UNSETENV
/* avoid passing old settings and confusing term size */
    unsetenv("LINES");
    unsetenv("COLUMNS");
    unsetenv("TERMCAP");	/* terminfo should be okay */
#endif				/* HAVE_UNSETENV */
}

/* ------------------------------------------------------------------------- */
/* main() */
/* INTPROTO */
int
main(int argc, const char * const *argv)
{
    const char    **cmd_argv;

/*
 * Save and then give up any super-user privileges
 * If we need privileges in any area then we must specifically request it.
 * We should only need to be root in these cases:
 *  1.  write utmp entries on some systems
 *  2.  chown tty on some systems
 */
    privileges(SAVE);
    privileges(IGNORE);

    init_vars();
    cmd_argv = init_resources(argc, argv);

#if (MENUBAR_MAX)
    menubar_read(rs.menu);
#endif
    scrollbar_mapping(Options & Opt_scrollBar);

    Create_Windows(argc, argv);

    init_xlocale();

    scr_reset();		/* initialize screen */
    Gr_reset();			/* reset graphics */

#ifdef DEBUG_X
    XSynchronize(Xdisplay, True);
    XSetErrorHandler((XErrorHandler) abort);
#else
    XSetErrorHandler((XErrorHandler) xerror_handler);
#endif

    if (scrollbar_visible()) {
	Resize_scrollBar();
	XMapWindow(Xdisplay, scrollBar.win);
    }
#if (MENUBAR_MAX)
    if (menubar_visible())
	XMapWindow(Xdisplay, menuBar.win);
#endif
#ifdef TRANSPARENT
    if (Options & Opt_transparent)
	check_our_parents(True);
#endif
    XMapWindow(Xdisplay, TermWin.vt);
    XMapWindow(Xdisplay, TermWin.parent[0]);

    init_env();
    init_command(cmd_argv);

    main_loop();		/* main processing loop */
    return EXIT_SUCCESS;
}
/*----------------------- end-of-file (C source) -----------------------*/
