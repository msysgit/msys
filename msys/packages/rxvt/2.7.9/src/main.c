/*--------------------------------*-C-*---------------------------------*
 * File:	main.c
 *----------------------------------------------------------------------*
 * $Id: main.c,v 1.1 2003/03/05 17:33:37 earnie Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1992      John Bovey, University of Kent at Canterbury <jdb@ukc.ac.uk>
 *				- original version
 * Copyright (c) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 *				- extensive modifications
 * Copyright (c) 1995      Garrett D'Amore <garrett@netcom.com>
 * Copyright (c) 1997      mj olesen <olesen@me.QueensU.CA>
 *				- extensive modifications
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
 *				- extensive modifications
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
#include "main.intpro"		/* PROTOS for internal routines */

#include <signal.h>

#ifdef TTY_GID_SUPPORT
# include <grp.h>
#endif

#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif

/*----------------------------------------------------------------------*/
/* rxvt_init() */
/* LIBPROTO */
rxvt_t         *
rxvt_init(int argc, const char *const *argv)
{
    const char    **cmd_argv;
    rxvt_t         *r;

    r = (rxvt_t *)rxvt_calloc(1, sizeof(rxvt_t));
    rxvt_set_r(r);		/* only assignment to _rxvt_vars */
    if (rxvt_init_vars(r) < 0) {
	free(r);
	return NULL;
    }

/*
 * Save and then give up any super-user privileges
 * If we need privileges in any area then we must specifically request it.
 * We should only need to be root in these cases:
 *  1.  write utmp entries on some systems
 *  2.  chown tty on some systems
 */
    rxvt_privileges(r, SAVE);
    rxvt_privileges(r, IGNORE);

    rxvt_init_secondary(r);

    cmd_argv = rxvt_init_resources(r, argc, argv);

#if (MENUBAR_MAX)
    rxvt_menubar_read(r, r->h->rs[Rs_menu]);
#endif
#ifdef HAVE_SCROLLBARS
    if (r->Options & Opt_scrollBar)
      scrollbar_setIdle();	/* set existence for size calculations */
#endif

    rxvt_Create_Windows(r, argc, argv);

    rxvt_init_xlocale(r);

    rxvt_scr_reset(r);		/* initialize screen */
    rxvt_Gr_reset(r);		/* reset graphics */

#ifdef DEBUG_X
    XSynchronize(r->Xdisplay, True);
    XSetErrorHandler((XErrorHandler) abort);
#else
    XSetErrorHandler((XErrorHandler) rxvt_xerror_handler);
#endif

#ifdef HAVE_SCROLLBARS
    if (r->Options & Opt_scrollBar)
	rxvt_Resize_scrollBar(r);	/* create and map scrollbar */
#endif
#if (MENUBAR_MAX)
    if (menubar_visible(r))
	XMapWindow(r->Xdisplay, r->menuBar.win);
#endif
#ifdef TRANSPARENT
    if (r->Options & Opt_transparent) {
	XSelectInput(r->Xdisplay, Xroot, PropertyChangeMask);
	rxvt_check_our_parents(r);
    }
#endif
    XMapWindow(r->Xdisplay, r->TermWin.vt);
    XMapWindow(r->Xdisplay, r->TermWin.parent[0]);

    rxvt_init_env(r);
    rxvt_init_command(r, cmd_argv);
    return r;
}

/* ------------------------------------------------------------------------- *
 *                       SIGNAL HANDLING & EXIT HANDLER                      *
 * ------------------------------------------------------------------------- */
/*
 * Catch a SIGCHLD signal and exit if the direct child has died
 */
/* ARGSUSED */
/* EXTPROTO */
RETSIGTYPE
rxvt_Child_signal(int sig __attribute__((unused)))
{
    int             pid, save_errno = errno;
    rxvt_t         *r;

    do {
	errno = 0;
    } while ((pid = waitpid(-1, NULL, WNOHANG)) == -1 && errno == EINTR);

    r = rxvt_get_r();
    if (pid == r->h->cmd_pid)
	exit(EXIT_SUCCESS);

    errno = save_errno;
    signal(SIGCHLD, rxvt_Child_signal);
}

/*
 * Catch a fatal signal and tidy up before quitting
 */
/* EXTPROTO */
RETSIGTYPE
rxvt_Exit_signal(int sig)
{
    signal(sig, SIG_DFL);
#ifdef DEBUG_CMD
    rxvt_print_error("signal %d", sig);
#endif
    rxvt_clean_exit();
    kill(getpid(), sig);
}

/* ARGSUSED */
/* INTPROTO */
int
rxvt_xerror_handler(const Display *display __attribute__((unused)), const XErrorEvent *event)
{
    rxvt_t         *r = rxvt_get_r();

    if (r->h->allowedxerror == -1) {
	r->h->allowedxerror = event->error_code;
	return 0;		/* ignored anyway */
    }
    rxvt_print_error("XError: Request: %d . %d, Error: %d",
		     event->request_code, event->minor_code,
		     event->error_code);
    /* XXX: probably should call rxvt_clean_exit() bypassing X routines */
    exit(EXIT_FAILURE);
    /* NOTREACHED */
}

/*----------------------------------------------------------------------*/
/*
 * Exit gracefully, clearing the utmp entry and restoring tty attributes
 * TODO: if debugging, this should free up any known resources if we can
 */
/* EXTPROTO */
void
rxvt_clean_exit(void)
{
    rxvt_t         *r = rxvt_get_r();

#ifdef DEBUG_SCREEN
    rxvt_scr_release(r);
#endif
    rxvt_privileged_ttydev(r, RESTORE);
    rxvt_privileged_utmp(r, RESTORE);
#ifdef USE_XIM
    if (r->h->Input_Context != NULL) {
	XDestroyIC(r->h->Input_Context);
	r->h->Input_Context = NULL;
    }
#endif
}

/* ------------------------------------------------------------------------- *
 *                         MEMORY ALLOCATION WRAPPERS                        *
 * ------------------------------------------------------------------------- */
/* EXTPROTO */
void           *
rxvt_malloc(size_t size)
{
     void           *p;

     p = malloc(size);
     if (p)
	return p;

     fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
     rxvt_clean_exit();
     exit(EXIT_FAILURE);
     /* NOTREACHED */
}

/* EXTPROTO */
void           *
rxvt_calloc(size_t number, size_t size)
{
     void           *p;

     p = calloc(number, size);
     if (p)
	return p;

     fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
     rxvt_clean_exit();
     exit(EXIT_FAILURE);
     /* NOTREACHED */
}

/* EXTPROTO */
void           *
rxvt_realloc(void *ptr, size_t size)
{
     void           *p;

     if (ptr)
	p = realloc(ptr, size);
     else
	p = malloc(size);
     if (p)
	return p;

     fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
     rxvt_clean_exit();
     exit(EXIT_FAILURE);
     /* NOTREACHED */
}
/* ------------------------------------------------------------------------- *
 *                            PRIVILEGED OPERATIONS                          *
 * ------------------------------------------------------------------------- */
/* take care of suid/sgid super-user (root) privileges */
/* INTPROTO */
void
rxvt_privileges(rxvt_t *r, int mode)
{
#if ! defined(__CYGWIN32__)
# if !defined(HAVE_SETEUID) && defined(HAVE_SETREUID)
/* setreuid() is the poor man's setuid(), seteuid() */
#  define seteuid(a)	setreuid(-1, (a))
#  define setegid(a)	setregid(-1, (a))
#  define HAVE_SETEUID
# endif
# ifdef HAVE_SETEUID
    switch (mode) {
    case IGNORE:
	/*
	 * change effective uid/gid - not real uid/gid - so we can switch
	 * back to root later, as required
	 */
	seteuid(getuid());
	setegid(getgid());
	break;
    case SAVE:
	r->h->euid = geteuid();
	r->h->egid = getegid();
	break;
    case RESTORE:
	seteuid(r->h->euid);
	setegid(r->h->egid);
	break;
    }
# else
    switch (mode) {
    case IGNORE:
	setuid(getuid());
	setgid(getgid());
    /* FALLTHROUGH */
    case SAVE:
    /* FALLTHROUGH */
    case RESTORE:
	break;
    }
# endif
#endif
}

#ifdef UTMP_SUPPORT
/* EXTPROTO */
void
rxvt_privileged_utmp(rxvt_t *r, char action)
{
    struct rxvt_hidden *h = r->h;

    D_MAIN((stderr, "rxvt_privileged_utmp(%c); waiting for: %c (pid: %d)", action, h->next_utmp_action, getpid()));
    if (h->next_utmp_action != action
	|| (action != SAVE && action != RESTORE)
	|| (r->Options & Opt_utmpInhibit)
	|| h->ttydev == NULL
	|| *h->ttydev == '\0')
	return;

    rxvt_privileges(r, RESTORE);
    if (action == SAVE) {
	h->next_utmp_action = RESTORE;
	rxvt_makeutent(r, h->ttydev, h->rs[Rs_display_name]);
    } else {		/* action == RESTORE */
	h->next_utmp_action = IGNORE;
	rxvt_cleanutent(r);
    }
    rxvt_privileges(r, IGNORE);
}
#endif

#ifndef NO_SETOWNER_TTYDEV
/* EXTPROTO */
void
rxvt_privileged_ttydev(rxvt_t *r, char action)
{
    struct rxvt_hidden *h = r->h;

    D_MAIN((stderr, "rxvt_privileged_ttydev(r, %c); waiting for: %c (pid: %d)", action, h->next_tty_action, getpid()));
    if (h->next_tty_action != action
	|| (action != SAVE && action != RESTORE)
	|| h->ttydev == NULL
	|| *h->ttydev == '\0')
	return;

    rxvt_privileges(r, RESTORE);

    if (action == SAVE) {
	h->next_tty_action = RESTORE;
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
/* store original tty status for restoration rxvt_clean_exit() -- rgg 04/12/95 */
	if (lstat(h->ttydev, &h->ttyfd_stat) < 0)	/* you lose out */
	    h->next_tty_action = IGNORE;
	else
# endif
	{
	    chown(h->ttydev, getuid(), h->ttygid); /* fail silently */
	    chmod(h->ttydev, h->ttymode);
# ifdef HAVE_REVOKE
	    revoke(h->ttydev);
# endif
	}
    } else {			/* action == RESTORE */
	h->next_tty_action = IGNORE;
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
	chmod(h->ttydev, h->ttyfd_stat.st_mode);
	chown(h->ttydev, h->ttyfd_stat.st_uid, h->ttyfd_stat.st_gid);
# else
	chmod(h->ttydev,
	      (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
	chown(h->ttydev, 0, 0);
# endif
    }

    rxvt_privileges(r, IGNORE);

# ifndef RESET_TTY_TO_COMMON_DEFAULTS
    D_MAIN((stderr, "%s \"%s\": mode %03o, uid %d, gid %d", action == RESTORE ? "Restoring" : (action == SAVE ? "Saving" : "UNKNOWN ERROR for"), h->ttydev, h->ttyfd_stat.st_mode, h->ttyfd_stat.st_uid, h->ttyfd_stat.st_gid));
# endif
}
#endif

/*----------------------------------------------------------------------*/
/*
 * window size/position calculcations for XSizeHint and other storage.
 * if width/height are non-zero then override calculated width/height
 */
/* EXTPROTO */
void
rxvt_window_calc(rxvt_t *r, unsigned int width, unsigned int height)
{
    short           recalc_x, recalc_y;
    int             x, y, sb_w, mb_h, flags;
    unsigned int    w, h;
    unsigned int    max_width, max_height;

    r->szHint.flags = PMinSize | PResizeInc | PBaseSize | PWinGravity;
    r->szHint.win_gravity = NorthWestGravity;
    /* r->szHint.min_aspect.x = r->szHint.min_aspect.y = 1; */

    recalc_x = recalc_y = 0;
    flags = 0;
    if (!r->h->parsed_geometry) {
	r->h->parsed_geometry = 1;
	if (r->h->rs[Rs_geometry])
	    flags = XParseGeometry(r->h->rs[Rs_geometry], &x, &y, &w, &h);
	if (flags & WidthValue) {
	    r->TermWin.ncol = BOUND_POSITIVE_INT16(w);
	    r->szHint.flags |= USSize;
	}
	if (flags & HeightValue) {
	    r->TermWin.nrow = BOUND_POSITIVE_INT16(h);
	    r->szHint.flags |= USSize;
	}
	if (flags & XValue) {
	    r->szHint.x = x;
	    r->szHint.flags |= USPosition;
	    if (flags & XNegative) {
		recalc_x = 1;
		r->szHint.win_gravity = NorthEastGravity;
	    }
	}
	if (flags & YValue) {
	    r->szHint.y = y;
	    r->szHint.flags |= USPosition;
	    if (flags & YNegative) {
		recalc_y = 1;
		if (r->szHint.win_gravity == NorthEastGravity)
		    r->szHint.win_gravity = SouthEastGravity;
		else
		    r->szHint.win_gravity = SouthWestGravity;
	    }
	}
    }
/* TODO: BOUNDS */
    r->TermWin.width = r->TermWin.ncol * r->TermWin.fwidth;
    r->TermWin.height = r->TermWin.nrow * r->TermWin.fheight;
    max_width = MAX_COLS * r->TermWin.fwidth;
    max_height = MAX_ROWS * r->TermWin.fheight;

    r->szHint.base_width = r->szHint.base_height = 2 * r->TermWin.int_bwidth;

    sb_w = mb_h = 0;
    r->h->window_vt_x = r->h->window_vt_y = 0;
    if (scrollbar_visible(r)) {
	sb_w = scrollbar_TotalWidth();
	r->szHint.base_width += sb_w;
	if (!(r->Options & Opt_scrollBar_right))
	    r->h->window_vt_x = sb_w;
    }
    if (menubar_visible(r)) {
	mb_h = menuBar_TotalHeight();
	r->szHint.base_height += mb_h;
	r->h->window_vt_y = mb_h;
    }
    r->szHint.width_inc = r->TermWin.fwidth;
    r->szHint.height_inc = r->TermWin.fheight;
    r->szHint.min_width = r->szHint.base_width + r->szHint.width_inc;
    r->szHint.min_height = r->szHint.base_height + r->szHint.height_inc;

    if (width && width - r->szHint.base_width < max_width) {
	r->szHint.width = width;
	r->TermWin.width = width - r->szHint.base_width;
    } else {
	MIN_IT(r->TermWin.width, max_width);
	r->szHint.width = r->szHint.base_width + r->TermWin.width;
    }
    if (height && height - r->szHint.base_height < max_height) {
	r->szHint.height = height;
	r->TermWin.height = height - r->szHint.base_height;
    } else {
	MIN_IT(r->TermWin.height, max_height);
	r->szHint.height = r->szHint.base_height + r->TermWin.height;
    }
    if (scrollbar_visible(r) && (r->Options & Opt_scrollBar_right))
	r->h->window_sb_x = r->szHint.width - sb_w;

    if (recalc_x)
	r->szHint.x += (DisplayWidth(r->Xdisplay, Xscreen)
			- r->szHint.width - 2 * r->TermWin.ext_bwidth);
    if (recalc_y)
	r->szHint.y += (DisplayHeight(r->Xdisplay, Xscreen)
			- r->szHint.height - 2 * r->TermWin.ext_bwidth);

    r->TermWin.ncol = r->TermWin.width / r->TermWin.fwidth;
    r->TermWin.nrow = r->TermWin.height / r->TermWin.fheight;
    return;
}

/*----------------------------------------------------------------------*/
/*
 * Tell the teletype handler what size the window is.
 * Called after a window size change.
 */
/* EXTPROTO */
void
rxvt_tt_winsize(int fd, unsigned short col, unsigned short row)
{
    struct winsize  ws;

    if (fd < 0)
	return;
    ws.ws_col = col;
    ws.ws_row = row;
    ws.ws_xpixel = ws.ws_ypixel = 0;
    ioctl(fd, TIOCSWINSZ, &ws);
}

/*----------------------------------------------------------------------*/
/* rxvt_change_font() - Switch to a new font */
/*
 * init = 1   - initialize
 *
 * fontname == FONT_UP  - switch to bigger font
 * fontname == FONT_DN  - switch to smaller font
 */
/* EXTPROTO */
void
rxvt_change_font(rxvt_t *r, int init, const char *fontname)
{
    const char     *msg = "can't load font \"%s\"";
    int             fh, fw, recheckfonts;
    int             pf;
    int             idx = 0;	/* index into r->h->rs[Rs_font] */
    XFontStruct    *xfont;
#ifdef MULTICHAR_SET
    int             i;
    char           *c, *enc;
    char            tmpbuf[64];
#endif

#define IDX2FNUM(i)	((i) == 0 ? FONT0_IDX : ((i) <= FONT0_IDX ? ((i)-1) : (i)))
#define FNUM2IDX(f)	((f) == FONT0_IDX ? 0 : ((f) < FONT0_IDX ? ((f)+1) : (f)))

    if (!init) {
	pf = r->h->fnum;
	switch (fontname[0]) {
	case '\0':
	    r->h->fnum = FONT0_IDX;
	    fontname = NULL;
	    break;

	    /* special (internal) prefix for font commands */
	case FONT_CMD:
	    idx = atoi(fontname + 1);
	    switch (fontname[1]) {
	    case '+':		/* corresponds to FONT_UP */
		rxvt_font_up_down(r, (idx ? idx : 1), 1);
		break;

	    case '-':		/* corresponds to FONT_DN */
		rxvt_font_up_down(r, (idx ? idx : 1), -1);
		break;

	    default:
		if (fontname[1] != '\0' && !isdigit(fontname[1]))
		    return;
		if (idx < 0 || idx >= MAX_NFONTS)
		    return;
		r->h->fnum = IDX2FNUM(idx);
		break;
	    }
	    fontname = NULL;
	    break;

	default:
	    if (fontname == NULL)
		return;
	    else
	    	/* search for existing fontname */
		for (idx = 0; idx < MAX_NFONTS; idx++)
		    if (r->h->rs[Rs_font + idx] == NULL) continue;
		    if (!STRCMP(r->h->rs[Rs_font + idx], fontname)) {
			r->h->fnum = IDX2FNUM(idx);
			fontname = NULL;
			break;
		    }
	    break;
	}
	/* re-position around the normal font */
	idx = FNUM2IDX(r->h->fnum);

	if (pf == r->h->fnum)
	    return;		/* no change */

	if (fontname != NULL) {
	    char           *name;

	    xfont = XLoadQueryFont(r->Xdisplay, fontname);
	    if (!xfont)
		return;

	    name = rxvt_malloc(STRLEN(fontname + 1) * sizeof(char));

	    if (name == NULL) {
		XFreeFont(r->Xdisplay, xfont);
		return;
	    }
	    STRCPY(name, fontname);
	    if (r->h->newfont[idx] != NULL)
		free(r->h->newfont[idx]);
	    r->h->newfont[idx] = name;
	    r->h->rs[Rs_font + idx] = r->h->newfont[idx];
	}
    }
    if (r->TermWin.font)
	XFreeFont(r->Xdisplay, r->TermWin.font);

/* load font or substitute */
    xfont = XLoadQueryFont(r->Xdisplay, r->h->rs[Rs_font + idx]);
    if (!xfont) {
	rxvt_print_error(msg, r->h->rs[Rs_font + idx]);
	r->h->rs[Rs_font + idx] = "fixed";
	xfont = XLoadQueryFont(r->Xdisplay, "fixed");
	if (!xfont) {
	    rxvt_print_error(msg, "fixed");
	    goto Abort;
	}
    }
    r->TermWin.font = xfont;

#ifndef NO_BOLDFONT
/* fail silently */
    if (init && r->h->rs[Rs_boldFont] != NULL)
	r->TermWin.boldFont_loaded = XLoadQueryFont(r->Xdisplay,
						    r->h->rs[Rs_boldFont]);
#endif

/* alter existing GC */
    if (!init)
	XSetFont(r->Xdisplay, r->TermWin.gc, r->TermWin.font->fid);

/* set the sizes */
    fw = rxvt_get_fontwidest(r->TermWin.font);
    fh = r->TermWin.font->ascent + r->TermWin.font->descent
	 + r->TermWin.lineSpace;
    if (fw == r->TermWin.font->min_bounds.width)
	r->TermWin.propfont &= ~PROPFONT_NORMAL;/* Mono-spaced font */
    else
	r->TermWin.propfont |= PROPFONT_NORMAL;	/* Proportional font */
    recheckfonts = !(fw == r->TermWin.fwidth && fh == r->TermWin.fheight);
    r->TermWin.fwidth = fw;
    r->TermWin.fheight = fh;

/* check that size of boldFont is okay */
#ifndef NO_BOLDFONT
    if (recheckfonts) {
	r->TermWin.boldFont = NULL;
	if (r->TermWin.boldFont_loaded != NULL) {
	    fw = rxvt_get_fontwidest(r->TermWin.boldFont_loaded);
	    fh = r->TermWin.boldFont_loaded->ascent
		 + r->TermWin.boldFont_loaded->descent;
	    if (fw <= r->TermWin.fwidth && fh <= r->TermWin.fheight)
		r->TermWin.boldFont = r->TermWin.boldFont_loaded;
	    if (fw == r->TermWin.fwidth /* && fh == r->TermWin.fheight */ )
		r->TermWin.propfont &= ~PROPFONT_BOLD;
	    else
		r->TermWin.propfont |= PROPFONT_BOLD;
	}
    }
#endif				/* NO_BOLDFONT */

#ifdef MULTICHAR_SET
    if (r->TermWin.mfont)
	XFreeFont(r->Xdisplay, r->TermWin.mfont);

    xfont = NULL;
/* load font or substitute */
    if (r->h->rs[Rs_mfont + idx] == NULL
	|| (xfont = XLoadQueryFont(r->Xdisplay,
				   r->h->rs[Rs_mfont + idx])) == NULL) {
	/* TODO: this is now mainly handled in rxvt_set_defaultfont() */
	i = 0;
	c = enc = "";
	switch (r->encoding_method) {
	case GB:
	    c = "-*-r-*-%.2d-*-gb2312.1980-0";
	    enc = "GB";
	    break;
	case BIG5:
	    c = "-*-r-*-%.2d-*-big5-0";
	    enc = "BIG5";
	    break;
	case EUCJ:
	case SJIS:
	    c = "-*-%.2d-*-jisx0208*-*";
	    enc = "EUCJ/SJIS";
	    break;
	case EUCKR:
	    c = "-*-%.2d-*-ksc5601*-*";
	    enc = "KR";
	    break;
	default:
	    i = fh;		/* jump past next two sections */
	    break;
	}
	for (; i < fh / 2; i++) {
	    sprintf(tmpbuf, c, fh - i);
	    xfont = XLoadQueryFont(r->Xdisplay, tmpbuf);
	    if (xfont) {
		r->h->rs[Rs_mfont + idx] = rxvt_malloc(STRLEN(tmpbuf) + 1);
		STRCPY(r->h->rs[Rs_mfont + idx], tmpbuf);
		break;
	    }
	}
	if (xfont == NULL && i != fh)
	    rxvt_print_error("no similar multichar font: encoding %s; size %d",
			     enc, fh);
    }
    r->TermWin.mfont = xfont;

    if (recheckfonts)
	/* XXX: This checks what? */
	if (r->TermWin.mfont != NULL) {
	    fw = rxvt_get_fontwidest(r->TermWin.mfont);
	    fh = r->TermWin.mfont->ascent + r->TermWin.mfont->descent;
	    if (fw > (r->TermWin.fwidth * 2) || fh > r->TermWin.fheight)
		r->TermWin.mfont = NULL;
	    if (fw == r->TermWin.fwidth /* && fh == r->TermWin.fheight */ )
		r->TermWin.propfont &= ~PROPFONT_MULTI;
	    else
		r->TermWin.propfont |= PROPFONT_MULTI;
	}
#endif				/* MULTICHAR_SET */

    /* Fontset setting is only valid when xlocale initialized. Since
     * rxvt_init_xlocale() is called after here, so we don't set fontset when
     * initialization, but let it set by rxvt_init_xlocale() */
    if (init)
	rxvt_setTermFontSet(r, -1);
    else
	rxvt_setTermFontSet(r, idx);

    rxvt_set_colorfgbg(r);

    if (!init) {
	rxvt_resize_all_windows(r, 0, 0, 0);
	rxvt_scr_touch(r, True);
    }
    return;
  Abort:
    rxvt_print_error("aborting");	/* fatal problem */
    exit(EXIT_FAILURE);
}

/* INTPROTO */
void
rxvt_font_up_down(rxvt_t *r, int n, int direction)
{
    const char     *p;
    int             initial, j;

    for (j = 0; j < n; j++) {
	initial = r->h->fnum;
	for (;;) {
	    r->h->fnum += direction;
	    if (r->h->fnum == MAX_NFONTS || r->h->fnum == -1) {
		r->h->fnum = initial;
		return;
	    }
	    p = r->h->rs[Rs_font + FNUM2IDX(r->h->fnum)];
	    if (p != NULL && STRLEN(p) > 1)
		break;
	}
    }
}
#undef IDX2FNUM
#undef FNUM2IDX

#ifdef STRICT_FONT_CHECKING
/* INTPROTO */
int
rxvt_get_fontwidest(XFontStruct *f)
{
    int             i, cw, fw = 0;

    if (f->min_bounds.width == f->max_bounds.width)
	return f->min_bounds.width;
    if (f->per_char == NULL)
	return f->max_bounds.width;
    for (i = f->max_char_or_byte2 - f->min_char_or_byte2; --i >= 0;) {
	cw = f->per_char[i].width;
	MAX_IT(fw, cw);
    }
    return fw;
}
#endif

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* xterm sequences - title, iconName, color (exptl) */
/* EXTPROTO */
void
rxvt_set_title(rxvt_t *r, const char *str)
{
#ifndef SMART_WINDOW_TITLE
    XStoreName(r->Xdisplay, r->TermWin.parent[0], str);
#else
    char           *name;

    if (XFetchName(r->Xdisplay, r->TermWin.parent[0], &name) == 0)
	name = NULL;
    if (name == NULL || STRCMP(name, str))
	XStoreName(r->Xdisplay, r->TermWin.parent[0], str);
    if (name)
	XFree(name);
#endif
}

/* EXTPROTO */
void
rxvt_set_iconName(rxvt_t *r, const char *str)
{
#ifndef SMART_WINDOW_TITLE
    XSetIconName(r->Xdisplay, r->TermWin.parent[0], str);
#else
    char           *name;

    if (XGetIconName(r->Xdisplay, r->TermWin.parent[0], &name))
	name = NULL;
    if (name == NULL || STRCMP(name, str))
	XSetIconName(r->Xdisplay, r->TermWin.parent[0], str);
    if (name)
	XFree(name);
#endif
}

#ifdef XTERM_COLOR_CHANGE
/* EXTPROTO */
void
rxvt_set_window_color(rxvt_t *r, int idx, const char *color)
{
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
	    r->PixColors[idx] = r->PixColors[minBrightCOLOR + i];
	    SET_PIXCOLOR(r->h, idx);
	    goto Done;
# endif
	}
	if (i >= 0 && i <= 7) {	/* normal colors */
	    r->PixColors[idx] = r->PixColors[minCOLOR + i];
	    SET_PIXCOLOR(r->h, idx);
	    goto Done;
	}
    }
    if (!rxvt_rXParseAllocColor(r, &xcol, color))
	return;
/* XStoreColor (r->Xdisplay, XCMAP, XColor*); */

/*
 * FIXME: should free colors here, but no idea how to do it so instead,
 * so just keep gobbling up the colormap
 */
# if 0
    for (i = Color_Black; i <= Color_White; i++)
	if (r->PixColors[idx] == r->PixColors[i])
	    break;
    if (i > Color_White) {
	/* fprintf (stderr, "XFreeColors: r->PixColors [%d] = %lu\n", idx, r->PixColors [idx]); */
	XFreeColors(r->Xdisplay, XCMAP, (r->PixColors + idx), 1,
		    DisplayPlanes(r->Xdisplay, Xscreen));
    }
# endif

    r->PixColors[idx] = xcol.pixel;
    SET_PIXCOLOR(r->h, idx);

/* XSetWindowAttributes attr; */
/* Cursor cursor; */
  Done:
    if (idx == Color_bg && !(r->Options & Opt_transparent))
	XSetWindowBackground(r->Xdisplay, r->TermWin.vt,
			     r->PixColors[Color_bg]);

/* handle Color_BD, scrollbar background, etc. */

    rxvt_set_colorfgbg(r);
    rxvt_recolour_cursor(r);
/* the only reasonable way to enforce a clean update */
    rxvt_scr_touch(r, False);
}

#else
# define rxvt_set_window_color(r, idx,color)	((void)0)
#endif				/* XTERM_COLOR_CHANGE */

/* EXTPROTO */
void
rxvt_recolour_cursor(rxvt_t *r)
{
    XColor          xcol[2];

    xcol[0].pixel = r->PixColors[Color_pointer];
    xcol[1].pixel = r->PixColors[Color_bg];
    XQueryColors(r->Xdisplay, XCMAP, xcol, 2);
    XRecolorCursor(r->Xdisplay, r->TermWin_cursor, &(xcol[0]), &(xcol[1]));
}

/*----------------------------------------------------------------------*/
/*
 * find if fg/bg matches any of the normal (low-intensity) colors
 */
/* INTPROTO */
void
rxvt_set_colorfgbg(rxvt_t *r)
{
    unsigned int    i;
    const char     *xpmb = "\0";
    char            fstr[sizeof("default") + 1], bstr[sizeof("default") + 1];

    r->h->env_colorfgbg = rxvt_malloc(sizeof("COLORFGBG=default;default;bg")
				      + 1);
    STRCPY(fstr, "default");
    STRCPY(bstr, "default");
    for (i = Color_Black; i <= Color_White; i++)
	if (r->PixColors[Color_fg] == r->PixColors[i]) {
	    sprintf(fstr, "%d", (i - Color_Black));
	    break;
	}
    for (i = Color_Black; i <= Color_White; i++)
	if (r->PixColors[Color_bg] == r->PixColors[i]) {
	    sprintf(bstr, "%d", (i - Color_Black));
#ifdef XPM_BACKGROUND
	    xpmb = "default;";
#endif
	    break;
	}
    sprintf(r->h->env_colorfgbg, "COLORFGBG=%s;%s%s", fstr, xpmb, bstr);
    putenv(r->h->env_colorfgbg);

#ifndef NO_BRIGHTCOLOR
    r->h->colorfgbg = DEFAULT_RSTYLE;
    for (i = minCOLOR; i <= maxCOLOR; i++) {
	if (r->PixColors[Color_fg] == r->PixColors[i]
# ifndef NO_BOLD_UNDERLINE_REVERSE
	    && r->PixColors[Color_fg] == r->PixColors[Color_BD]
# endif				/* ! NO_BOLD_UNDERLINE_REVERSE */
	    /* if we wanted boldFont to have precedence */
# if 0				/* ifndef NO_BOLDFONT */
	    && r->TermWin.boldFont == NULL
# endif				/* NO_BOLDFONT */
	    )
	    r->h->colorfgbg = SET_FGCOLOR(r->h->colorfgbg, i);
	if (r->PixColors[Color_bg] == r->PixColors[i])
	    r->h->colorfgbg = SET_BGCOLOR(r->h->colorfgbg, i);
    }
#endif
}

/*----------------------------------------------------------------------*/
/*
 * Colour determination for low colour displays, routine from
 *     Hans de Goede <hans@highrise.nl>
 */

/* EXTPROTO */
int
rxvt_rXParseAllocColor(rxvt_t *r, XColor *screen_in_out, const char *colour)
{
    int             res = 0;

    if (!XParseColor(r->Xdisplay, XCMAP, colour, screen_in_out))
	rxvt_print_error("can't determine colour: %s", colour);
    else
	res = rxvt_rXAllocColor(r, screen_in_out, colour);
    return res;
}

/* EXTPROTO */
int
rxvt_rXAllocColor(rxvt_t *r, XColor *screen_in_out, const char *colour)
{
    int             res;

    if ((res = XAllocColor(r->Xdisplay, XCMAP, screen_in_out)))
	return res;

    /* try again with closest match */
    if (XDEPTH >= 4 && XDEPTH <= 8) {
	int             i, numcol;
	int             best_pixel = 0;
	unsigned long   best_diff, diff;
	XColor         *colors;

#define rSQR(x)		((x)*(x))

	best_diff = 0;
	numcol = 0x01 << XDEPTH;
	if ((colors = rxvt_malloc(numcol * sizeof(XColor)))) {
	    for (i = 0; i < numcol; i++)
		colors[i].pixel = i;

	    XQueryColors(r->Xdisplay, XCMAP, colors, numcol);
	    for (i = 0; i < numcol; i++) {
		diff = rSQR(screen_in_out->red - colors[i].red)
		       + rSQR(screen_in_out->green - colors[i].green)
		       + rSQR(screen_in_out->blue - colors[i].blue);
		if (i == 0 || diff < best_diff) {
		    best_pixel = colors[i].pixel;
		    best_diff = diff;
		}
	    }
	    *screen_in_out = colors[best_pixel];
	    free(colors);
	    res = XAllocColor(r->Xdisplay, XCMAP, screen_in_out);
	}
    }
    if (res == 0)
	rxvt_print_error("can't allocate colour: %s", colour);
    return res;
}

/* -------------------------------------------------------------------- *
 * -                         WINDOW RESIZING                          - *
 * -------------------------------------------------------------------- */
/* EXTPROTO */
void
rxvt_resize_all_windows(rxvt_t *r, unsigned int width, unsigned int height, int ignoreparent)
{
    int             fix_screen;
#ifdef SMART_RESIZE	
    int             old_width = r->szHint.width,
		    old_height = r->szHint.height;
#endif

    rxvt_window_calc(r, width, height);
    XSetWMNormalHints(r->Xdisplay, r->TermWin.parent[0], &r->szHint);
    if (!ignoreparent) {
#ifdef SMART_RESIZE	
/*
 * resize by Marius Gedminas <marius.gedminas@uosis.mif.vu.lt>
 * reposition window on resize depending on placement on screen
 */
	int             x, y, x1, y1;
	int             dx, dy;
	unsigned int    unused_w1, unused_h1, unused_b1, unused_d1;
	Window          unused_cr;

	XTranslateCoordinates(r->Xdisplay, r->TermWin.parent[0], Xroot,
			      0, 0, &x, &y, &unused_cr);
	XGetGeometry(r->Xdisplay, r->TermWin.parent[0], &unused_cr, &x1, &y1,
		     &unused_w1, &unused_h1, &unused_b1, &unused_d1);
	/*
	 * if Xroot isn't the parent window, a WM will probably have offset
	 * our position for handles and decorations.  Counter it
	 */
	if (x1 != x || y1 != y) {
	    x -= x1;
	    y -= y1;
	}

	x1 = (DisplayWidth(r->Xdisplay, Xscreen) - old_width) / 2;
	y1 = (DisplayHeight(r->Xdisplay, Xscreen) - old_height) / 2;
	dx = old_width - r->szHint.width;
	dy = old_height - r->szHint.height;

	/* Check position of the center of the window */
	if (x < x1)		/* left half */
	    dx = 0;
	else if (x == x1)	/* exact center */
	    dx /= 2;
	if (y < y1)		/* top half */
	    dy = 0;
	else if (y == y1)	/* exact center */
	    dy /= 2;

	XMoveResizeWindow(r->Xdisplay, r->TermWin.parent[0], x + dx, y + dy,
			  r->szHint.width, r->szHint.height);
#else
	XResizeWindow(r->Xdisplay, r->TermWin.parent[0], r->szHint.width,
		      r->szHint.height);
#endif
    }

    fix_screen = (r->TermWin.ncol != r->h->prev_ncol
		  || r->TermWin.nrow != r->h->prev_nrow);
    if (fix_screen || width != r->h->old_width || height != r->h->old_height) {
	if (scrollbar_visible(r)) {
	    XMoveResizeWindow(r->Xdisplay, r->scrollBar.win, r->h->window_sb_x,
			      0, scrollbar_TotalWidth(), r->szHint.height);
	    rxvt_Resize_scrollBar(r);
	}
	if (menubar_visible(r))
	    XMoveResizeWindow(r->Xdisplay, r->menuBar.win, r->h->window_vt_x,
			      0, TermWin_TotalWidth(), menuBar_TotalHeight());
	XMoveResizeWindow(r->Xdisplay, r->TermWin.vt, r->h->window_vt_x,
			  r->h->window_vt_y, TermWin_TotalWidth(),
			  TermWin_TotalHeight());
#ifdef RXVT_GRAPHICS
	if (r->h->old_height)
	    rxvt_Gr_Resize(r, r->h->old_width - r->szHint.base_width,
			   r->h->old_height - r->szHint.base_height);
#endif
	rxvt_scr_clear(r);
	rxvt_resize_pixmap(r);
    }

    if (fix_screen || r->h->old_height == 0) {
	int             curr_screen = -1;
	u_int16_t       old_ncol = r->h->prev_ncol;

	/* scr_reset only works on the primary screen */
	if (r->h->old_height) 	/* this is not the first time through */
	    curr_screen = rxvt_scr_change_screen(r, PRIMARY);
	rxvt_scr_reset(r);
	if (curr_screen >= 0) {	/* this is not the first time through */
	    rxvt_scr_change_screen(r, curr_screen);
	    rxvt_selection_check(r, (old_ncol != r->TermWin.ncol ? 4 : 0));
	}
    }

    r->h->old_width = r->szHint.width;
    r->h->old_height = r->szHint.height;

#ifdef USE_XIM
    rxvt_IMSetStatusPosition(r);
#endif
}

/*
 * Set the width/height of the vt window in characters.  Units are pixels.
 * good for toggling 80/132 columns
 */
/* EXTPROTO */
void
rxvt_set_widthheight(rxvt_t *r, unsigned int width, unsigned int height)
{
    XWindowAttributes wattr;

    if (width == 0 || height == 0) {
	XGetWindowAttributes(r->Xdisplay, Xroot, &wattr);
	if (width == 0)
	    width = wattr.width - r->szHint.base_width;
	if (height == 0)
	    height = wattr.height - r->szHint.base_height;
    }
    if (width != r->TermWin.width || height != r->TermWin.height) {
	width += r->szHint.base_width;
	height += r->szHint.base_height;
	rxvt_resize_all_windows(r, width, height, 0);
    }
}

/* -------------------------------------------------------------------- *
 * -                      X INPUT METHOD ROUTINES                     - *
 * -------------------------------------------------------------------- */
#ifdef USE_XIM
/* INTPROTO */
void
rxvt_setSize(rxvt_t *r, XRectangle *size)
{
    size->x = r->TermWin.int_bwidth;
    size->y = r->TermWin.int_bwidth;
    size->width = Width2Pixel(r->TermWin.ncol);
    size->height = Height2Pixel(r->TermWin.nrow);
}

/* INTPROTO */
void
rxvt_setColor(rxvt_t *r, unsigned long *fg, unsigned long *bg)
{
    *fg = r->PixColors[Color_fg];
    *bg = r->PixColors[Color_bg];
}

/* Checking whether input method is running. */
/* INTPROTO */
Bool
rxvt_IMisRunning(rxvt_t *r)
{
    char           *p;
    Atom            atom;
    Window          win;
    char            server[IMBUFSIZ];

    /* get current locale modifier */
    if ((p = XSetLocaleModifiers(NULL)) != NULL) {
	STRCPY(server, "@server=");
	STRNCAT(server, &(p[4]), IMBUFSIZ - 9);	/* skip "@im=" */
	if ((p = STRCHR(server + 1, '@')) != NULL)	/* first one only */
	    *p = '\0';

	atom = XInternAtom(r->Xdisplay, server, False);
	win = XGetSelectionOwner(r->Xdisplay, atom);
	if (win != None)
	    return True;
    }
    return False;
}

/* EXTPROTO */
void
rxvt_IMSendSpot(rxvt_t *r)
{
    XPoint          spot;
    XVaNestedList   preedit_attr;

    if (r->h->Input_Context == NULL
	|| !r->TermWin.focus
	|| !(r->h->input_style & XIMPreeditPosition)
	|| !(r->h->event_type == KeyPress
	     || r->h->event_type == Expose
	     || r->h->event_type == NoExpose
	     || r->h->event_type == SelectionNotify
	     || r->h->event_type == ButtonRelease
	     || r->h->event_type == FocusIn)
	|| !rxvt_IMisRunning(r))
	return;

    rxvt_setPosition(r, &spot);

    preedit_attr = XVaCreateNestedList(0, XNSpotLocation, &spot, NULL);
    XSetICValues(r->h->Input_Context, XNPreeditAttributes, preedit_attr, NULL);
    XFree(preedit_attr);
}

/* EXTPROTO */
void
rxvt_setTermFontSet(rxvt_t *r, int idx)
{
    char           *string;
    long            length;
    XFontSet        prev_fontset;
    int             success = 0;

    if (idx < 0 || idx >= MAX_NFONTS)
	return;
    D_MAIN((stderr, "rxvt_setTermFontSet()"));
    prev_fontset = r->TermWin.fontset;
    r->TermWin.fontset = NULL;

    length = 0;
    if (r->h->rs[Rs_font + idx])
	length += STRLEN(r->h->rs[Rs_font + idx]) + 1;
# ifdef MULTICHAR_SET
    if (r->h->rs[Rs_mfont + idx])
	length += STRLEN(r->h->rs[Rs_mfont + idx]) + 1;
# endif
    if (length == 0 || (string = rxvt_malloc(length + 1)) == NULL)
	r->TermWin.fontset = NULL;
    else {
	int             missing_charsetcount;
	char          **missing_charsetlist, *def_string;

	string[0] = '\0';
	if (r->h->rs[Rs_font + idx]) {
	    STRCAT(string, r->h->rs[Rs_font + idx]);
	    STRCAT(string, ",");
	}
# ifdef MULTICHAR_SET
	if (r->h->rs[Rs_mfont + idx]) {
	    STRCAT(string, r->h->rs[Rs_mfont + idx]);
	    STRCAT(string, ",");
	}
# endif
	string[STRLEN(string) - 1] = '\0';
	r->TermWin.fontset = XCreateFontSet(r->Xdisplay, string,
					    &missing_charsetlist,
					    &missing_charsetcount,
					    &def_string);
	free(string);
	if (r->TermWin.fontset != NULL)
	    success = 1;
    }

    if (success) {
	if (prev_fontset != NULL)
	    XFreeFontSet(r->Xdisplay, prev_fontset);
    } else
	r->TermWin.fontset = prev_fontset;
}

/* INTPROTO */
void
rxvt_setPreeditArea(rxvt_t *r, XRectangle *preedit_rect, XRectangle *status_rect, XRectangle *needed_rect)
{
    int             mbh, vtx = 0;

    if (scrollbar_visible(r) && !(r->Options & Opt_scrollBar_right))
	vtx = scrollbar_TotalWidth();
    mbh = menubar_visible(r) ? menuBar_TotalHeight() : 0;
    mbh -= r->TermWin.lineSpace;

    preedit_rect->x = needed_rect->width + vtx;
    preedit_rect->y = Height2Pixel(r->TermWin.nrow - 1) + mbh;

    preedit_rect->width = Width2Pixel(r->TermWin.ncol + 1) - needed_rect->width
	    		  + vtx;
    preedit_rect->height = Height2Pixel(1);

    status_rect->x = vtx;
    status_rect->y = Height2Pixel(r->TermWin.nrow - 1) + mbh;

    status_rect->width = needed_rect->width ? needed_rect->width
					    : Width2Pixel(r->TermWin.ncol + 1);
    status_rect->height = Height2Pixel(1);
}

/* ARGSUSED */
/* INTPROTO */
void
rxvt_IMDestroyCallback(XIM xim __attribute__((unused)), XPointer client_data __attribute__((unused)), XPointer call_data __attribute__((unused)))
{
    rxvt_t         *r = rxvt_get_r();

    r->h->Input_Context = NULL;
    /* To avoid Segmentation Fault in C locale: Solaris only? */
    if (STRCMP(r->h->locale, "C"))
	XRegisterIMInstantiateCallback(r->Xdisplay, NULL, NULL, NULL,
				       rxvt_IMInstantiateCallback, NULL);
}

/*
 * X manual pages and include files don't match on some systems:
 * some think this is an XIDProc and others an XIMProc so we can't
 * use the first argument - need to update this to be nice for
 * both types via some sort of configure detection
 */
/* ARGSUSED */
/* EXTPROTO */
void
rxvt_IMInstantiateCallback(Display *unused __attribute__((unused)), XPointer client_data __attribute__((unused)), XPointer call_data __attribute__((unused)))
{
    int             i, found, had_im;
    const char     *p;
    char          **s;
    rxvt_t         *r = rxvt_get_r();
    char            buf[IMBUFSIZ];

    D_MAIN((stderr, "rxvt_IMInstantiateCallback()"));
    if (r->h->Input_Context)
	return;

    found = had_im = 0;
    p = r->h->rs[Rs_inputMethod];
    if (p && *p) {
	had_im = 1;
	s = rxvt_splitcommastring(p);
	for (i = 0; s[i]; i++) {
	    if (*s[i]) {
		STRCPY(buf, "@im=");
		STRNCAT(buf, s[i], IMBUFSIZ - 5);
		if ((p = XSetLocaleModifiers(buf)) != NULL && *p
		    && (rxvt_IM_get_IC(r) == True)) {
		    found = 1;
		    break;
		}
	    }
	}
	for (i = 0; s[i]; i++)
	    free(s[i]);
	free(s);
    }
    if (found)
	return;

/* try with XMODIFIERS env. var. */
    if ((p = XSetLocaleModifiers("")) != NULL && *p) {
	rxvt_IM_get_IC(r);
	return;
    }

/* try with no modifiers base IF the user didn't specify an IM */
    if (!had_im && (p = XSetLocaleModifiers("@im=none")) != NULL && *p
	&& rxvt_IM_get_IC(r) == True)
	return;
}

/*
 * Try to open a XIM with the current modifiers, then see if we can
 * open a suitable preedit type
 */
/* INTPROTO */
Bool
rxvt_IM_get_IC(rxvt_t *r)
{
    int             i, j, found;
    XIM             xim;
    XPoint          spot;
    XRectangle      rect, status_rect, needed_rect;
    unsigned long   fg, bg;
    const char     *p;
    char          **s;
    XIMStyles      *xim_styles;
    XVaNestedList   preedit_attr, status_attr;
    XIMCallback     ximcallback;
    struct rxvt_hidden *h = r->h;

    D_MAIN((stderr, "rxvt_IM_get_IC()"));
    xim = XOpenIM(r->Xdisplay, NULL, NULL, NULL);
    if (xim == NULL)
	return False;

    xim_styles = NULL;
    if (XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL)
	|| !xim_styles || !xim_styles->count_styles) {
	XCloseIM(xim);
	return False;
    }

    p = h->rs[Rs_preeditType] ? h->rs[Rs_preeditType]
			      : "OverTheSpot,OffTheSpot,Root";
    s = rxvt_splitcommastring(p);
    for (i = found = 0; !found && s[i]; i++) {
	if (!STRCMP(s[i], "OverTheSpot"))
	    h->input_style = (XIMPreeditPosition | XIMStatusNothing);
	else if (!STRCMP(s[i], "OffTheSpot"))
	    h->input_style = (XIMPreeditArea | XIMStatusArea);
	else if (!STRCMP(s[i], "Root"))
	    h->input_style = (XIMPreeditNothing | XIMStatusNothing);

	for (j = 0; j < xim_styles->count_styles; j++)
	    if (h->input_style == xim_styles->supported_styles[j]) {
		found = 1;
		break;
	    }
    }
    for (i = 0; s[i]; i++)
	free(s[i]);
    free(s);
    XFree(xim_styles);

    if (!found) {
	XCloseIM(xim);
	return False;
    }

    ximcallback.callback = rxvt_IMDestroyCallback;

    /* XXX: not sure why we need this (as well as IC one below) */
    XSetIMValues(xim, XNDestroyCallback, &ximcallback, NULL);

    preedit_attr = status_attr = NULL;

    if (h->input_style & XIMPreeditPosition) {
	rxvt_setSize(r, &rect);
	rxvt_setPosition(r, &spot);
	rxvt_setColor(r, &fg, &bg);

	preedit_attr = XVaCreateNestedList(0, XNArea, &rect,
					   XNSpotLocation, &spot,
					   XNForeground, fg,
					   XNBackground, bg,
					   XNFontSet, r->TermWin.fontset,
					   NULL);
    } else if (h->input_style & XIMPreeditArea) {
	rxvt_setColor(r, &fg, &bg);

	/*
	 * The necessary width of preedit area is unknown
	 * until create input context.
	 */
	needed_rect.width = 0;

	rxvt_setPreeditArea(r, &rect, &status_rect, &needed_rect);

	preedit_attr = XVaCreateNestedList(0, XNArea, &rect,
					   XNForeground, fg,
					   XNBackground, bg,
					   XNFontSet, r->TermWin.fontset,
					   NULL);
	status_attr = XVaCreateNestedList(0, XNArea, &status_rect,
					  XNForeground, fg,
					  XNBackground, bg,
					  XNFontSet, r->TermWin.fontset, NULL);
    }
    h->Input_Context = XCreateIC(xim, XNInputStyle, h->input_style,
				 XNClientWindow, r->TermWin.parent[0],
				 XNFocusWindow, r->TermWin.parent[0],
				 XNDestroyCallback, &ximcallback,
				 preedit_attr ? XNPreeditAttributes : NULL,
				 preedit_attr,
				 status_attr ? XNStatusAttributes : NULL,
				 status_attr, NULL);
    if (preedit_attr)
	XFree(preedit_attr);
    if (status_attr)
	XFree(status_attr);
    if (h->Input_Context == NULL) {
	rxvt_print_error("failed to create input context");
	XCloseIM(xim);
	return False;
    }
    if (h->input_style & XIMPreeditArea)
	rxvt_IMSetStatusPosition(r);
    D_MAIN((stderr, "rxvt_IM_get_IC() - successful connection"));
    return True;
}

/* EXTPROTO */
void
rxvt_IMSetStatusPosition(rxvt_t *r)
{
    XRectangle      preedit_rect, status_rect, *needed_rect;
    XVaNestedList   preedit_attr, status_attr;

    if (r->h->Input_Context == NULL
	|| !r->TermWin.focus
	|| !(r->h->input_style & XIMPreeditArea)
	|| !rxvt_IMisRunning(r))
	return;

    /* Getting the necessary width of preedit area */
    status_attr = XVaCreateNestedList(0, XNAreaNeeded, &needed_rect, NULL);
    XGetICValues(r->h->Input_Context, XNStatusAttributes, status_attr, NULL);
    XFree(status_attr);

    rxvt_setPreeditArea(r, &preedit_rect, &status_rect, needed_rect);

    preedit_attr = XVaCreateNestedList(0, XNArea, &preedit_rect, NULL);
    status_attr = XVaCreateNestedList(0, XNArea, &status_rect, NULL);

    XSetICValues(r->h->Input_Context,
		 XNPreeditAttributes, preedit_attr,
		 XNStatusAttributes, status_attr, NULL);

    XFree(preedit_attr);
    XFree(status_attr);
}
#endif				/* USE_XIM */

/*----------------------------------------------------------------------*/
static rxvt_t  *_rxvt_vars = NULL;

/* EXTPROTO */
rxvt_t         *
rxvt_get_r(void)
{
    return _rxvt_vars;
}
/* INTPROTO */
void
rxvt_set_r(rxvt_t *r)
{
    _rxvt_vars = r;
}

/*----------------------- end-of-file (C source) -----------------------*/
