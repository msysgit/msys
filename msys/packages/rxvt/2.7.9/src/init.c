/*--------------------------------*-C-*---------------------------------*
 * File:	init.c
 *----------------------------------------------------------------------*
 * $Id: init.c,v 1.2 2004-03-16 13:26:38 earnie Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1992      John Bovey, University of Kent at Canterbury <jdb@ukc.ac.uk>
 *				- original version
 * Copyright (c) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 * 				- extensive modifications
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
 * 				- extensive modifications
 * Copyright (c) 1999      D J Hawkey Jr <hawkeyd@visi.com>
 *				- QNX support
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
/*
 * Initialisation routines.
 */

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "init.h"

#include <signal.h>

const char *const def_colorName[] = {
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
# ifdef XTERM_COLORS
    "Grey90",			/* 7: white             (#E5E5E5) */
# else
    "AntiqueWhite",		/* 7: white             (#FAEBD7) */
# endif
/* high-intensity colors */
# ifdef XTERM_COLORS
    "Grey30",			/* 8: bright black      (#4D4D4D) */
# else
    "Grey25",			/* 8: bright black      (#404040) */
# endif
#endif				/* NO_BRIGHTCOLOR */
    "Red",			/* 1/9: bright red      (#FF0000) */
    "Green",			/* 2/10: bright green   (#00FF00) */
    "Yellow",			/* 3/11: bright yellow  (#FFFF00) */
    "Blue",			/* 4/12: bright blue    (#0000FF) */
    "Magenta",			/* 5/13: bright magenta (#FF00FF) */
    "Cyan",			/* 6/14: bright cyan    (#00FFFF) */
    "White",			/* 7/15: bright white   (#FFFFFF) */
#ifdef TTY_256COLOR
    "rgb:00/00/00",		/* default 16-255 color table     */
    "rgb:00/00/2a",
    "rgb:00/00/55",
    "rgb:00/00/7f",
    "rgb:00/00/aa",
    "rgb:00/00/d4",
    "rgb:00/2a/00",
    "rgb:00/2a/2a",
    "rgb:00/2a/55",
    "rgb:00/2a/7f",
    "rgb:00/2a/aa",
    "rgb:00/2a/d4",
    "rgb:00/55/00",
    "rgb:00/55/2a",
    "rgb:00/55/55",
    "rgb:00/55/7f",
    "rgb:00/55/aa",
    "rgb:00/55/d4",
    "rgb:00/7f/00",
    "rgb:00/7f/2a",
    "rgb:00/7f/55",
    "rgb:00/7f/7f",
    "rgb:00/7f/aa",
    "rgb:00/7f/d4",
    "rgb:00/aa/00",
    "rgb:00/aa/2a",
    "rgb:00/aa/55",
    "rgb:00/aa/7f",
    "rgb:00/aa/aa",
    "rgb:00/aa/d4",
    "rgb:00/d4/00",
    "rgb:00/d4/2a",
    "rgb:00/d4/55",
    "rgb:00/d4/7f",
    "rgb:00/d4/aa",
    "rgb:00/d4/d4",
    "rgb:2a/00/00",
    "rgb:2a/00/2a",
    "rgb:2a/00/55",
    "rgb:2a/00/7f",
    "rgb:2a/00/aa",
    "rgb:2a/00/d4",
    "rgb:2a/2a/00",
    "rgb:2a/2a/2a",
    "rgb:2a/2a/55",
    "rgb:2a/2a/7f",
    "rgb:2a/2a/aa",
    "rgb:2a/2a/d4",
    "rgb:2a/55/00",
    "rgb:2a/55/2a",
    "rgb:2a/55/55",
    "rgb:2a/55/7f",
    "rgb:2a/55/aa",
    "rgb:2a/55/d4",
    "rgb:2a/7f/00",
    "rgb:2a/7f/2a",
    "rgb:2a/7f/55",
    "rgb:2a/7f/7f",
    "rgb:2a/7f/aa",
    "rgb:2a/7f/d4",
    "rgb:2a/aa/00",
    "rgb:2a/aa/2a",
    "rgb:2a/aa/55",
    "rgb:2a/aa/7f",
    "rgb:2a/aa/aa",
    "rgb:2a/aa/d4",
    "rgb:2a/d4/00",
    "rgb:2a/d4/2a",
    "rgb:2a/d4/55",
    "rgb:2a/d4/7f",
    "rgb:2a/d4/aa",
    "rgb:2a/d4/d4",
    "rgb:55/00/00",
    "rgb:55/00/2a",
    "rgb:55/00/55",
    "rgb:55/00/7f",
    "rgb:55/00/aa",
    "rgb:55/00/d4",
    "rgb:55/2a/00",
    "rgb:55/2a/2a",
    "rgb:55/2a/55",
    "rgb:55/2a/7f",
    "rgb:55/2a/aa",
    "rgb:55/2a/d4",
    "rgb:55/55/00",
    "rgb:55/55/2a",
    "rgb:55/55/55",
    "rgb:55/55/7f",
    "rgb:55/55/aa",
    "rgb:55/55/d4",
    "rgb:55/7f/00",
    "rgb:55/7f/2a",
    "rgb:55/7f/55",
    "rgb:55/7f/7f",
    "rgb:55/7f/aa",
    "rgb:55/7f/d4",
    "rgb:55/aa/00",
    "rgb:55/aa/2a",
    "rgb:55/aa/55",
    "rgb:55/aa/7f",
    "rgb:55/aa/aa",
    "rgb:55/aa/d4",
    "rgb:55/d4/00",
    "rgb:55/d4/2a",
    "rgb:55/d4/55",
    "rgb:55/d4/7f",
    "rgb:55/d4/aa",
    "rgb:55/d4/d4",
    "rgb:7f/00/00",
    "rgb:7f/00/2a",
    "rgb:7f/00/55",
    "rgb:7f/00/7f",
    "rgb:7f/00/aa",
    "rgb:7f/00/d4",
    "rgb:7f/2a/00",
    "rgb:7f/2a/2a",
    "rgb:7f/2a/55",
    "rgb:7f/2a/7f",
    "rgb:7f/2a/aa",
    "rgb:7f/2a/d4",
    "rgb:7f/55/00",
    "rgb:7f/55/2a",
    "rgb:7f/55/55",
    "rgb:7f/55/7f",
    "rgb:7f/55/aa",
    "rgb:7f/55/d4",
    "rgb:7f/7f/00",
    "rgb:7f/7f/2a",
    "rgb:7f/7f/55",
    "rgb:7f/7f/7f",
    "rgb:7f/7f/aa",
    "rgb:7f/7f/d4",
    "rgb:7f/aa/00",
    "rgb:7f/aa/2a",
    "rgb:7f/aa/55",
    "rgb:7f/aa/7f",
    "rgb:7f/aa/aa",
    "rgb:7f/aa/d4",
    "rgb:7f/d4/00",
    "rgb:7f/d4/2a",
    "rgb:7f/d4/55",
    "rgb:7f/d4/7f",
    "rgb:7f/d4/aa",
    "rgb:7f/d4/d4",
    "rgb:aa/00/00",
    "rgb:aa/00/2a",
    "rgb:aa/00/55",
    "rgb:aa/00/7f",
    "rgb:aa/00/aa",
    "rgb:aa/00/d4",
    "rgb:aa/2a/00",
    "rgb:aa/2a/2a",
    "rgb:aa/2a/55",
    "rgb:aa/2a/7f",
    "rgb:aa/2a/aa",
    "rgb:aa/2a/d4",
    "rgb:aa/55/00",
    "rgb:aa/55/2a",
    "rgb:aa/55/55",
    "rgb:aa/55/7f",
    "rgb:aa/55/aa",
    "rgb:aa/55/d4",
    "rgb:aa/7f/00",
    "rgb:aa/7f/2a",
    "rgb:aa/7f/55",
    "rgb:aa/7f/7f",
    "rgb:aa/7f/aa",
    "rgb:aa/7f/d4",
    "rgb:aa/aa/00",
    "rgb:aa/aa/2a",
    "rgb:aa/aa/55",
    "rgb:aa/aa/7f",
    "rgb:aa/aa/aa",
    "rgb:aa/aa/d4",
    "rgb:aa/d4/00",
    "rgb:aa/d4/2a",
    "rgb:aa/d4/55",
    "rgb:aa/d4/7f",
    "rgb:aa/d4/aa",
    "rgb:aa/d4/d4",
    "rgb:d4/00/00",
    "rgb:d4/00/2a",
    "rgb:d4/00/55",
    "rgb:d4/00/7f",
    "rgb:d4/00/aa",
    "rgb:d4/00/d4",
    "rgb:d4/2a/00",
    "rgb:d4/2a/2a",
    "rgb:d4/2a/55",
    "rgb:d4/2a/7f",
    "rgb:d4/2a/aa",
    "rgb:d4/2a/d4",
    "rgb:d4/55/00",
    "rgb:d4/55/2a",
    "rgb:d4/55/55",
    "rgb:d4/55/7f",
    "rgb:d4/55/aa",
    "rgb:d4/55/d4",
    "rgb:d4/7f/00",
    "rgb:d4/7f/2a",
    "rgb:d4/7f/55",
    "rgb:d4/7f/7f",
    "rgb:d4/7f/aa",
    "rgb:d4/7f/d4",
    "rgb:d4/aa/00",
    "rgb:d4/aa/2a",
    "rgb:d4/aa/55",
    "rgb:d4/aa/7f",
    "rgb:d4/aa/aa",
    "rgb:d4/aa/d4",
    "rgb:d4/d4/00",
    "rgb:d4/d4/2a",
    "rgb:d4/d4/55",
    "rgb:d4/d4/7f",
    "rgb:d4/d4/aa",
    "rgb:d4/d4/d4",
    "rgb:08/08/08",
    "rgb:12/12/12",
    "rgb:1c/1c/1c",
    "rgb:26/26/26",
    "rgb:30/30/30",
    "rgb:3a/3a/3a",
    "rgb:44/44/44",
    "rgb:4e/4e/4e",
    "rgb:58/58/58",
    "rgb:62/62/62",
    "rgb:6c/6c/6c",
    "rgb:76/76/76",
    "rgb:80/80/80",
    "rgb:8a/8a/8a",
    "rgb:94/94/94",
    "rgb:9e/9e/9e",
    "rgb:a8/a8/a8",
    "rgb:b2/b2/b2",
    "rgb:bc/bc/bc",
    "rgb:c6/c6/c6",
    "rgb:d0/d0/d0",
    "rgb:da/da/da",
    "rgb:e4/e4/e4",
    "rgb:ee/ee/ee",
#endif
#ifndef NO_CURSORCOLOR
    COLOR_CURSOR_BACKGROUND,
    COLOR_CURSOR_FOREGROUND,
#endif				/* ! NO_CURSORCOLOR */
    NULL,			/* Color_pointer                  */
    NULL,			/* Color_border                   */
#ifndef NO_BOLD_UNDERLINE_REVERSE
    NULL,			/* Color_BD                       */
    NULL,			/* Color_UL                       */
    NULL,			/* Color_RV                       */
#endif				/* ! NO_BOLD_UNDERLINE_REVERSE */
#ifdef OPTION_HC
    NULL,
#endif
#ifdef KEEP_SCROLLCOLOR
    COLOR_SCROLLBAR,
    COLOR_SCROLLTROUGH,
#endif				/* KEEP_SCROLLCOLOR */
};

const char *const xa_names[NUM_XA] = {
    "COMPOUND_TEXT",
    "MULTIPLE",	
    "TARGETS",	
    "TEXT",
    "TIMESTAMP",
    "VT_SELECTION",
    "INCR",
    "WM_DELETE_WINDOW",
#ifdef TRANSPARENT
    "_XROOTPMAP_ID",
#endif
#ifdef OFFIX_DND
    "DndProtocol",
    "DndSelection",
#endif
    "CLIPBOARD"
};

/*----------------------------------------------------------------------*/
/* substitute system functions */
#if defined(__svr4__) && ! defined(_POSIX_VERSION)
/* INTPROTO */
int
rxvt_getdtablesize(void)
{
    struct rlimit   rlim;

    getrlimit(RLIMIT_NOFILE, &rlim);
    return rlim.rlim_cur;
}
#endif
/*----------------------------------------------------------------------*/
/* EXTPROTO */
int
rxvt_init_vars(rxvt_t *r)
{
    struct rxvt_hidden *h;

#ifndef NULLS_ARE_NOT_ZEROS
    MEMSET(r, 0, sizeof(rxvt_t));
#endif
    h = r->h = (struct rxvt_hidden *)rxvt_calloc(1, sizeof(struct rxvt_hidden));

    r->PixColors = (Pixel *)rxvt_malloc(sizeof(Pixel) * TOTAL_COLORS);
    if (r->h == NULL || r->PixColors == NULL)
	return -1;
#ifdef NULLS_ARE_NOT_ZEROS
    r->Xdisplay = NULL;
    r->TermWin.fontset = NULL;
#ifndef NO_BOLDFONT
    r->TermWin.boldFont_loaded = NULL;
#endif
    r->h->ttydev = NULL;
    h->xa[XA_COMPOUND_TEXT] = h->xa[XA_MULTIPLE] = h->xa[XA_TARGETS] =
	h->xa[XA_TEXT] = h->xa[XA_TIMESTAMP] = h->xa[XA_VT_SELECTION] =
	h->xa[XA_INCR] = NULL;
    h->locale = NULL;
# ifdef MENUBAR
    h->menubarGC = None;
    h->BuildMenu = NULL;	/* the menu currently being built */
#  if (MENUBAR_MAX > 1)
    h->CurrentBar = NULL;
#  endif			/* (MENUBAR_MAX > 1) */
# endif
# ifdef USE_XIM
    h->Input_Context = NULL;
# endif
    h->v_bufstr = NULL;
# ifdef RXVT_GRAPHICS
    h->gr_root = NULL;
    h->gr_last_id = None;
# endif
    h->buffer = NULL;
    h->compose.compose_ptr = NULL;
#endif				/* NULLS_ARE_NOT_ZEROS */

#if defined(XPM_BACKGROUND) || defined(TRANSPARENT)
    r->TermWin.pixmap = None;
#endif
#ifdef UTMP_SUPPORT
    h->next_utmp_action = SAVE;
#endif
#ifndef NO_SETOWNER_TTYDEV
    h->next_tty_action = SAVE;
#endif
    h->MEvent.time = CurrentTime;
    h->MEvent.button = AnyButton;
    r->Options = DEFAULT_OPTIONS;
    h->want_refresh = 1;
    h->cmd_pid = -1;
    r->cmd_fd = r->tty_fd = r->Xfd = -1;
    h->PrivateModes = h->SavedModes = PrivMode_Default;
    r->TermWin.focus = 1;
    r->TermWin.ncol = 80;
    r->TermWin.nrow = 24;
    r->TermWin.int_bwidth = INTERNALBORDERWIDTH;
    r->TermWin.ext_bwidth = EXTERNALBORDERWIDTH;
    r->TermWin.lineSpace = LINESPACE;
    r->TermWin.saveLines = SAVELINES;
    r->numPixColors = TOTAL_COLORS;
#ifndef NO_NEW_SELECTION
    r->selection_style = NEW_SELECT;
#else
    r->selection_style = OLD_SELECT;
#endif
#ifndef NO_BRIGHTCOLOR
    h->colorfgbg = DEFAULT_RSTYLE;
#endif
#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
    h->ks_bigfont = XK_greater;
    h->ks_smallfont = XK_less;
#endif
#ifdef GREEK_SUPPORT
    h->ks_greekmodeswith = GREEK_KEYBOARD_MODESWITCH;
#endif
    h->refresh_limit = 1;
    h->refresh_type = SLOW_REFRESH;
    h->prev_nrow = h->prev_ncol = 0;
#ifdef MULTICHAR_SET
# ifdef MULTICHAR_ENCODING
    r->encoding_method = MULTICHAR_ENCODING;
# endif
    h->multichar_decode = rxvt_euc2jis;
#endif
    h->oldcursor.row = h->oldcursor.col = -1;
#ifdef XPM_BACKGROUND
/*  h->bgPixmap.w = h->bgPixmap.h = 0; */
    h->bgPixmap.x = h->bgPixmap.y = 50;
    h->bgPixmap.pixmap = None;
#endif
    h->last_bot = h->last_state = -1;
#ifdef MENUBAR
    h->menu_readonly = 1;
# if !(MENUBAR_MAX > 1)
    h->CurrentBar = &(h->BarList);
# endif				/* (MENUBAR_MAX > 1) */
#endif
    return 0;
}

/* EXTPROTO */
void
rxvt_init_secondary(rxvt_t *r)
{
#ifdef TTY_GID_SUPPORT
    struct group   *gr = getgrnam("tty");

    if (gr) {		/* change group ownership of tty to "tty" */
	r->h->ttymode = S_IRUSR | S_IWUSR | S_IWGRP;
	r->h->ttygid = gr->gr_gid;
    } else
#endif				/* TTY_GID_SUPPORT */
    {
	r->h->ttymode = S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH;
	r->h->ttygid = getgid();
    }
#if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
    r->h->locale = setlocale(LC_CTYPE, "");
#endif
}

/*----------------------------------------------------------------------*/
/* EXTPROTO */
const char    **
rxvt_init_resources(rxvt_t *r, int argc, const char *const *argv)
{
    int             i, r_argc;
    char           *val;
    const char    **cmd_argv, **r_argv;
    const char    **rs;

/*
 * Look for -exec option.  Find => split and make cmd_argv[] of command args
 */
    for (r_argc = 0; r_argc < argc; r_argc++)
	if (!STRCMP(argv[r_argc], "-e") || !STRCMP(argv[r_argc], "-exec"))
	    break;
    r_argv = (const char **)rxvt_malloc(sizeof(char *) * (r_argc + 1));

    for (i = 0; i < r_argc; i++)
	r_argv[i] = (const char *)argv[i];
    r_argv[i] = NULL;
    if (r_argc == argc)
	cmd_argv = NULL;
    else {
	cmd_argv = (const char **)rxvt_malloc(sizeof(char *) * (argc - r_argc));

	for (i = 0; i < argc - r_argc - 1; i++)
	    cmd_argv[i] = (const char *)argv[i + r_argc + 1];
	cmd_argv[i] = NULL;
    }

/* clear all resources */
    rs = r->h->rs;
    for (i = 0; i < NUM_RESOURCES;)
	rs[i++] = NULL;

    rs[Rs_name] = rxvt_r_basename(argv[0]);
/*
 * Open display, get options/resources and create the window
 */
    if ((rs[Rs_display_name] = getenv("DISPLAY")) == NULL)
	rs[Rs_display_name] = ":0";

    rxvt_get_options(r, r_argc, r_argv);
    free(r_argv);

#ifdef LOCAL_X_IS_UNIX
    if (rs[Rs_display_name][0] == ':') {
	val = rxvt_malloc(5 + STRLEN(rs[Rs_display_name]));
	STRCPY(val, "unix");
	STRCAT(val, rs[Rs_display_name]);
	r->Xdisplay = XOpenDisplay(val);
	free(val);
    }
#endif

    if (r->Xdisplay == NULL
	&& (r->Xdisplay = XOpenDisplay(rs[Rs_display_name])) == NULL) {
	rxvt_print_error("can't open display %s", rs[Rs_display_name]);
	exit(EXIT_FAILURE);
    }

    rxvt_extract_resources(r, r->Xdisplay, rs[Rs_name]);

/*
 * set any defaults not already set
 */
    if (cmd_argv && cmd_argv[0]) {
	if (!rs[Rs_title])
	    rs[Rs_title] = rxvt_r_basename(cmd_argv[0]);
	if (!rs[Rs_iconName])
	    rs[Rs_iconName] = rs[Rs_title];
    } else {
	if (!rs[Rs_title])
	    rs[Rs_title] = rs[Rs_name];
	if (!rs[Rs_iconName])
	    rs[Rs_iconName] = rs[Rs_name];
    }
    if (rs[Rs_saveLines] && (i = atoi(rs[Rs_saveLines])) >= 0)
	r->TermWin.saveLines = BOUND_POSITIVE_INT16(i);
#ifndef NO_FRILLS
    if (rs[Rs_int_bwidth] && (i = atoi(rs[Rs_int_bwidth])) >= 0)
	r->TermWin.int_bwidth = min(i, 100);	/* arbitrary limit */
    if (rs[Rs_ext_bwidth] && (i = atoi(rs[Rs_ext_bwidth])) >= 0)
	r->TermWin.ext_bwidth = min(i, 100);	/* arbitrary limit */
#endif
#ifndef NO_LINESPACE
    if (rs[Rs_lineSpace] && (i = atoi(rs[Rs_lineSpace])) >= 0)
	r->TermWin.lineSpace = min(i, 100);	/* arbitrary limit */
#endif

/* no point having a scrollbar without having any scrollback! */
    if (!r->TermWin.saveLines)
	r->Options &= ~Opt_scrollBar;

#ifdef PRINTPIPE
    if (!rs[Rs_print_pipe])
	rs[Rs_print_pipe] = PRINTPIPE;
#endif
    if (!rs[Rs_cutchars])
	rs[Rs_cutchars] = CUTCHARS;
#ifndef NO_BACKSPACE_KEY
    if (!rs[Rs_backspace_key])
# ifdef DEFAULT_BACKSPACE
	r->h->key_backspace = DEFAULT_BACKSPACE;
# else
	r->h->key_backspace = "DEC";	/* can toggle between \033 or \177 */
# endif
    else {
	val = STRDUP(rs[Rs_backspace_key]);
	rxvt_Str_trim(val);
	rxvt_Str_escaped(val);
	r->h->key_backspace = val;
    }
#endif
#ifndef NO_DELETE_KEY
    if (!rs[Rs_delete_key])
# ifdef DEFAULT_DELETE
	r->h->key_delete = DEFAULT_DELETE;
# else
	r->h->key_delete = "\033[3~";
# endif
    else {
	val = STRDUP(rs[Rs_delete_key]);
	rxvt_Str_trim(val);
	rxvt_Str_escaped(val);
	r->h->key_delete = val;
    }
#endif
    if (rs[Rs_answerbackstring]) {
	rxvt_Str_trim((char *)rs[Rs_answerbackstring]);
	rxvt_Str_escaped((char *)rs[Rs_answerbackstring]);
    }

    if (rs[Rs_selectstyle]) {
	if (STRNCASECMP(rs[Rs_selectstyle], "oldword", 7) == 0)
	    r->selection_style = OLD_WORD_SELECT;
#ifndef NO_OLD_SELECTION
	else if (STRNCASECMP(rs[Rs_selectstyle], "old", 3) == 0)
	    r->selection_style = OLD_SELECT;
#endif
    }

#ifdef HAVE_SCROLLBARS
    rxvt_setup_scrollbar(r, rs[Rs_scrollBar_align], rs[Rs_scrollstyle],
			 rs[Rs_scrollBar_thickness]);
#endif

#if 0	/* #ifndef NO_BOLDFONT */
    if (rs[Rs_font] == NULL && rs[Rs_boldFont] != NULL) {
	rs[Rs_font] = rs[Rs_boldFont];
	rs[Rs_boldFont] = NULL;
    }
#endif
    rxvt_set_defaultfont(r, rs);

#ifdef XTERM_REVERSE_VIDEO
/* this is how xterm implements reverseVideo */
    if (r->Options & Opt_reverseVideo) {
	if (!rs[Rs_color + Color_fg])
	    rs[Rs_color + Color_fg] = def_colorName[Color_bg];
	if (!rs[Rs_color + Color_bg])
	    rs[Rs_color + Color_bg] = def_colorName[Color_fg];
    }
#endif

    for (i = 0; i < NRS_COLORS; i++)
	if (!rs[Rs_color + i])
	    rs[Rs_color + i] = def_colorName[i];

#ifndef XTERM_REVERSE_VIDEO
/* this is how we implement reverseVideo */
    if (r->Options & Opt_reverseVideo)
	SWAP_IT(rs[Rs_color + Color_fg], rs[Rs_color + Color_bg], const char *);
#endif

/* convenient aliases for setting fg/bg to colors */
    rxvt_color_aliases(r, Color_fg);
    rxvt_color_aliases(r, Color_bg);
#ifndef NO_CURSORCOLOR
    rxvt_color_aliases(r, Color_cursor);
    rxvt_color_aliases(r, Color_cursor2);
#endif				/* NO_CURSORCOLOR */
    rxvt_color_aliases(r, Color_pointer);
    rxvt_color_aliases(r, Color_border);
#ifndef NO_BOLD_UNDERLINE_REVERSE
    rxvt_color_aliases(r, Color_BD);
    rxvt_color_aliases(r, Color_UL);
    rxvt_color_aliases(r, Color_RV);
#endif				/* ! NO_BOLD_UNDERLINE_REVERSE */

    return cmd_argv;
}

/*----------------------------------------------------------------------*/
/* EXTPROTO */
void
rxvt_init_env(rxvt_t *r)
{
    int             i;
    unsigned int    u;
    char           *val;

#ifdef DISPLAY_IS_IP
/* Fixup display_name for export over pty to any interested terminal
 * clients via "ESC[7n" (e.g. shells).  Note we use the pure IP number
 * (for the first non-loopback interface) that we get from
 * rxvt_network_display().  This is more "name-resolution-portable", if you
 * will, and probably allows for faster x-client startup if your name
 * server is beyond a slow link or overloaded at client startup.  Of
 * course that only helps the shell's child processes, not us.
 *
 * Giving out the display_name also affords a potential security hole
 */
    val = rxvt_network_display(r->h->rs[Rs_display_name]);
    r->h->rs[Rs_display_name] = (const char *)val;
    if (val == NULL)
#endif				/* DISPLAY_IS_IP */
	val = XDisplayString(r->Xdisplay);
    if (r->h->rs[Rs_display_name] == NULL)
	r->h->rs[Rs_display_name] = val;	/* use broken `:0' value */

    i = STRLEN(val);
    r->h->env_display = rxvt_malloc((i + 9) * sizeof(char));

    sprintf(r->h->env_display, "DISPLAY=%s", val);

    /* avoiding the math library:
     * i = (int)(ceil(log10((unsigned int)r->TermWin.parent[0]))) */
    for (i = 0, u = (unsigned int)r->TermWin.parent[0]; u; u /= 10, i++) ;
    MAX_IT(i, 1);
    r->h->env_windowid = rxvt_malloc((i + 10) * sizeof(char));

    sprintf(r->h->env_windowid, "WINDOWID=%u",
	    (unsigned int)r->TermWin.parent[0]);

/* add entries to the environment:
 * @ DISPLAY:   in case we started with -display
 * @ WINDOWID:  X window id number of the window
 * @ COLORTERM: terminal sub-name and also indicates its color
 * @ TERM:      terminal name
 * @ TERMINFO:	path to terminfo directory
 */
    putenv(r->h->env_display);
    putenv(r->h->env_windowid);
#ifdef RXVT_TERMINFO
    putenv("TERMINFO=" RXVT_TERMINFO);
#endif
    if (XDEPTH <= 2)
	putenv("COLORTERM=" COLORTERMENV "-mono");
    else
	putenv("COLORTERM=" COLORTERMENVFULL);
    if (r->h->rs[Rs_term_name] != NULL) {
	r->h->env_term = rxvt_malloc((STRLEN(r->h->rs[Rs_term_name]) + 6)
				* sizeof(char));
	sprintf(r->h->env_term, "TERM=%s", r->h->rs[Rs_term_name]);
	putenv(r->h->env_term);
    } else
	putenv("TERM=" TERMENV);

#ifdef HAVE_UNSETENV
/* avoid passing old settings and confusing term size */
    unsetenv("LINES");
    unsetenv("COLUMNS");
    unsetenv("TERMCAP");	/* terminfo should be okay */
#endif				/* HAVE_UNSETENV */
}

/*----------------------------------------------------------------------*/
/*
 * This is more or less stolen straight from XFree86 xterm.
 * This should support all European type languages.
 */
/* EXTPROTO */
void
rxvt_init_xlocale(rxvt_t *r)
{
#ifdef USE_XIM
    if (r->h->locale == NULL)
	rxvt_print_error("Setting locale failed.");
    else {
	Atom            wmlocale;

	wmlocale = XInternAtom(r->Xdisplay, "WM_LOCALE_NAME", False);
	XChangeProperty(r->Xdisplay, r->TermWin.parent[0], wmlocale,
			XA_STRING, 8, PropModeReplace,
			(unsigned char *)r->h->locale, STRLEN(r->h->locale));

	if (XSupportsLocale() != True) {
	    rxvt_print_error("The locale is not supported by Xlib");
	    return;
	}
	rxvt_setTermFontSet(r, 0);

	/* see if we can connect yet */
	rxvt_IMInstantiateCallback(r->Xdisplay, NULL, NULL);

	/* To avoid Segmentation Fault in C locale: Solaris only? */
	if (STRCMP(r->h->locale, "C"))
	    XRegisterIMInstantiateCallback(r->Xdisplay, NULL, NULL, NULL,
					   rxvt_IMInstantiateCallback, NULL);
    }
#endif
}

/*----------------------------------------------------------------------*/
/* EXTPROTO */
void
rxvt_init_command(rxvt_t *r, const char *const *argv)
{
/*
 * Initialize the command connection.
 * This should be called after the X server connection is established.
 */
    int             i;

    for (i = 0; i < NUM_XA; i++)
	r->h->xa[i] = XInternAtom(r->Xdisplay, xa_names[i], False);

/* Enable delete window protocol */
    XSetWMProtocols(r->Xdisplay, r->TermWin.parent[0],
		    &(r->h->xa[XA_WMDELETEWINDOW]), 1);

#ifdef USING_W11LIB
/* enable W11 callbacks */
    W11AddEventHandler(r->Xdisplay, rxvt_W11_process_x_event);
#endif

/* get number of available file descriptors */
#if defined(_POSIX_VERSION) || ! defined(__svr4__)
    r->num_fds = (int)sysconf(_SC_OPEN_MAX);
#else
    r->num_fds = rxvt_getdtablesize();
#endif

#ifdef META8_OPTION
    r->h->meta_char = (r->Options & Opt_meta8 ? 0x80 : C0_ESC);
#endif
    rxvt_get_ourmods(r);
    if (!(r->Options & Opt_scrollTtyOutput))
	r->h->PrivateModes |= PrivMode_TtyOutputInh;
    if (r->Options & Opt_scrollTtyKeypress)
	r->h->PrivateModes |= PrivMode_Keypress;
    if (!(r->Options & Opt_jumpScroll))
	r->h->PrivateModes |= PrivMode_smoothScroll;
#ifndef NO_BACKSPACE_KEY
    if (STRCMP(r->h->key_backspace, "DEC") == 0)
	r->h->PrivateModes |= PrivMode_HaveBackSpace;
#endif
/* add value for scrollBar */
    if (scrollbar_visible(r)) {
	r->h->PrivateModes |= PrivMode_scrollBar;
	r->h->SavedModes |= PrivMode_scrollBar;
    }
    if (menubar_visible(r)) {
	r->h->PrivateModes |= PrivMode_menuBar;
	r->h->SavedModes |= PrivMode_menuBar;
    }
    greek_init();

    r->Xfd = XConnectionNumber(r->Xdisplay);

    if ((r->cmd_fd = rxvt_run_command(r, argv)) < 0) {
	rxvt_print_error("aborting");
	exit(EXIT_FAILURE);
    }
}

/*----------------------------------------------------------------------*/
/* INTPROTO */
void
rxvt_Get_Colours(rxvt_t *r)
{
    int             i;

    for (i = 0; i < (XDEPTH <= 2 ? 2 : NRS_COLORS); i++) {
	XColor          xcol;

	if (!r->h->rs[Rs_color + i])
	    continue;

	if (!rxvt_rXParseAllocColor(r, &xcol, r->h->rs[Rs_color + i])) {
#ifndef XTERM_REVERSE_VIDEO
	    if (i < 2 && (r->Options & Opt_reverseVideo)) {
		r->h->rs[Rs_color + i] = def_colorName[!i];
	    } else
#endif
		r->h->rs[Rs_color + i] = def_colorName[i];
	    if (!r->h->rs[Rs_color + i])
		continue;
	    if (!rxvt_rXParseAllocColor(r, &xcol, r->h->rs[Rs_color + i])) {
		switch (i) {
		case Color_fg:
		case Color_bg:
		    /* fatal: need bg/fg color */
		    rxvt_print_error("aborting");
		    exit(EXIT_FAILURE);
		/* NOTREACHED */
		    break;
#ifndef NO_CURSORCOLOR
		case Color_cursor2:
		    xcol.pixel = r->PixColors[Color_fg];
		    break;
#endif				/* ! NO_CURSORCOLOR */
		case Color_pointer:
		    xcol.pixel = r->PixColors[Color_fg];
		    break;
		default:
		    xcol.pixel = r->PixColors[Color_bg];	/* None */
		    break;
		}
	    }
	}
	r->PixColors[i] = xcol.pixel;
	SET_PIXCOLOR(r->h, i);
    }

    if (XDEPTH <= 2 || !r->h->rs[Rs_color + Color_pointer])
	r->PixColors[Color_pointer] = r->PixColors[Color_fg];
    if (XDEPTH <= 2 || !r->h->rs[Rs_color + Color_border])
	r->PixColors[Color_border] = r->PixColors[Color_fg];

/*
 * get scrollBar/menuBar shadow colors
 *
 * The calculations of topShadow/bottomShadow values are adapted
 * from the fvwm window manager.
 */
#ifdef KEEP_SCROLLCOLOR
    if (XDEPTH <= 2) {	/* Monochrome */
	r->PixColors[Color_scroll] = r->PixColors[Color_fg];
	r->PixColors[Color_topShadow] = r->PixColors[Color_bg];
	r->PixColors[Color_bottomShadow] = r->PixColors[Color_bg];
    } else {
	XColor           xcol[3];
	/* xcol[0] == white
	 * xcol[1] == top shadow
	 * xcol[2] == bot shadow */

	xcol[1].pixel = r->PixColors[Color_scroll];
# ifdef PREFER_24BIT
	xcol[0].red = xcol[0].green = xcol[0].blue = (unsigned short)~0;
	rxvt_rXAllocColor(r, &(xcol[0]), "White");
/*        XFreeColors(r->Xdisplay, XCMAP, &(xcol[0].pixel), 1, ~0); */
	XQueryColors(r->Xdisplay, XCMAP, &(xcol[1]), 1);
# else
	xcol[0].pixel = WhitePixel(r->Xdisplay, Xscreen);
	XQueryColors(r->Xdisplay, XCMAP, xcol, 2);
# endif

	/* bottomShadowColor */
	xcol[2].red = xcol[1].red / 2;
	xcol[2].green = xcol[1].green / 2;
	xcol[2].blue = xcol[1].blue / 2;
	if (!rxvt_rXAllocColor(r, &(xcol[2]), "Color_bottomShadow"))
	    xcol[2].pixel = r->PixColors[Color_Black];
	r->PixColors[Color_bottomShadow] = xcol[2].pixel;

	/* topShadowColor */
	xcol[1].red = max((xcol[0].red / 5), xcol[1].red);
	xcol[1].green = max((xcol[0].green / 5), xcol[1].green);
	xcol[1].blue = max((xcol[0].blue / 5), xcol[1].blue);
	xcol[1].red = min(xcol[0].red, (xcol[1].red * 7) / 5);
	xcol[1].green = min(xcol[0].green, (xcol[1].green * 7) / 5);
	xcol[1].blue = min(xcol[0].blue, (xcol[1].blue * 7) / 5);

	if (!rxvt_rXAllocColor(r, &(xcol[1]), "Color_topShadow"))
	    xcol[1].pixel = r->PixColors[Color_White];
	r->PixColors[Color_topShadow] = xcol[1].pixel;
    }
#endif				/* KEEP_SCROLLCOLOR */
}

/*----------------------------------------------------------------------*/
/* color aliases, fg/bg bright-bold */
/* INTPROTO */
void
rxvt_color_aliases(rxvt_t *r, int idx)
{
    if (r->h->rs[Rs_color + idx] && isdigit(*(r->h->rs[Rs_color + idx]))) {
	int             i = atoi(r->h->rs[Rs_color + idx]);

	if (i >= 8 && i <= 15) {	/* bright colors */
	    i -= 8;
#ifndef NO_BRIGHTCOLOR
	    r->h->rs[Rs_color + idx] = r->h->rs[Rs_color + minBrightCOLOR + i];
	    return;
#endif
	}
	if (i >= 0 && i <= 7)	/* normal colors */
	    r->h->rs[Rs_color + idx] = r->h->rs[Rs_color + minCOLOR + i];
    }
}

/*----------------------------------------------------------------------*/
/*
 * Probe the modifier keymap to get the Meta (Alt) and Num_Lock settings
 * Use resource ``modifier'' to override the Meta modifier
 */
/* INTPROTO */
void
rxvt_get_ourmods(rxvt_t *r)
{
    int             i, j, k;
    int             requestedmeta, realmeta, realalt;
    const char     *cm, *rsmod;
    XModifierKeymap *map;
    KeyCode        *kc;
    const unsigned int modmasks[] =
	{ Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask };

    requestedmeta = realmeta = realalt = 0;
    rsmod = r->h->rs[Rs_modifier];
    if (rsmod
	&& STRCASECMP(rsmod, "mod1") >= 0 && STRCASECMP(rsmod, "mod5") <= 0)
	requestedmeta = rsmod[3] - '0';

    map = XGetModifierMapping(r->Xdisplay);
    kc = map->modifiermap;
    for (i = 1; i < 6; i++) {
	k = (i + 2) * map->max_keypermod;	/* skip shift/lock/control */
	for (j = map->max_keypermod; j--; k++) {
	    if (kc[k] == 0)
		break;
	    switch (XKeycodeToKeysym(r->Xdisplay, kc[k], 0)) {
	    case XK_Num_Lock:
		r->h->ModNumLockMask = modmasks[i - 1];
		/* FALLTHROUGH */
	    default:
		continue;	/* for(;;) */
	    case XK_Meta_L:
	    case XK_Meta_R:
		cm = "meta";
		realmeta = i;
		break;
	    case XK_Alt_L:
	    case XK_Alt_R:
		cm = "alt";
		realalt = i;
		break;
	    case XK_Super_L:
	    case XK_Super_R:
		cm = "super";
		break;
	    case XK_Hyper_L:
	    case XK_Hyper_R:
		cm = "hyper";
		break;
	    }
	    if (rsmod && STRNCASECMP(rsmod, cm, STRLEN(cm)) == 0)
		requestedmeta = i;
	}
    }
    XFreeModifiermap(map);
    i = (requestedmeta ? requestedmeta
		       : (realmeta ? realmeta
				   : (realalt ? realalt : 0)));
    if (i)
	r->h->ModMetaMask = modmasks[i - 1];
}

/*----------------------------------------------------------------------*/
/* rxvt_Create_Windows() - Open and map the window */
/* EXTPROTO */
void
rxvt_Create_Windows(rxvt_t *r, int argc, const char *const *argv)
{
    XClassHint      classHint;
    XWMHints        wmHint;
    XGCValues       gcvalue;

#ifdef PREFER_24BIT
    XSetWindowAttributes attributes;
    XWindowAttributes gattr;

    XCMAP = DefaultColormap(r->Xdisplay, Xscreen);
    XVISUAL = DefaultVisual(r->Xdisplay, Xscreen);

    if (r->Options & Opt_transparent) {
	XGetWindowAttributes(r->Xdisplay, RootWindow(r->Xdisplay, Xscreen),
			     &gattr);
	XDEPTH = gattr.depth;
    } else {
	XDEPTH = DefaultDepth(r->Xdisplay, Xscreen);
/*
 * If depth is not 24, look for a 24bit visual.
 */
	if (XDEPTH != 24) {
	    XVisualInfo     vinfo;

	    if (XMatchVisualInfo(r->Xdisplay, Xscreen, 24, TrueColor, &vinfo)) {
		XDEPTH = 24;
		XVISUAL = vinfo.visual;
		XCMAP = XCreateColormap(r->Xdisplay,
					RootWindow(r->Xdisplay, Xscreen),
					XVISUAL, AllocNone);
	    }
	}
    }
#endif

/* grab colors before netscape does */
    rxvt_Get_Colours(r);

    rxvt_change_font(r, 1, NULL);
    rxvt_window_calc(r, 0, 0);
    r->h->old_width = r->szHint.width;
    r->h->old_height = r->szHint.height;

/* parent window - reverse video so we can see placement errors
 * sub-window placement & size in rxvt_resize_subwindows()
 */

#ifdef PREFER_24BIT
    attributes.background_pixel = r->PixColors[Color_fg];
    attributes.border_pixel = r->PixColors[Color_border];
    attributes.colormap = XCMAP;
    r->TermWin.parent[0] = XCreateWindow(r->Xdisplay, Xroot,
					 r->szHint.x, r->szHint.y,
					 r->szHint.width, r->szHint.height,
					 r->TermWin.ext_bwidth,
					 XDEPTH, InputOutput,
					 XVISUAL,
					 CWBackPixel | CWBorderPixel
					 | CWColormap, &attributes);
#else
    r->TermWin.parent[0] = XCreateSimpleWindow(r->Xdisplay, Xroot,
					       r->szHint.x, r->szHint.y,
					       r->szHint.width,
					       r->szHint.height,
					       r->TermWin.ext_bwidth,
					       r->PixColors[Color_border],
					       r->PixColors[Color_fg]);
#endif
    rxvt_xterm_seq(r, XTerm_title, r->h->rs[Rs_title], CHAR_ST);
    rxvt_xterm_seq(r, XTerm_iconName, r->h->rs[Rs_iconName], CHAR_ST);

    classHint.res_name = (char *)r->h->rs[Rs_name];
    classHint.res_class = (char *)APL_CLASS;

    wmHint.flags = (InputHint | StateHint | WindowGroupHint);
    wmHint.input = True;
    wmHint.initial_state = (r->Options & Opt_iconic ? IconicState
						    : NormalState);
    wmHint.window_group = r->TermWin.parent[0];

    XSetWMProperties(r->Xdisplay, r->TermWin.parent[0], NULL, NULL,
		     (char **)argv, argc, &r->szHint, &wmHint, &classHint);
    XSelectInput(r->Xdisplay, r->TermWin.parent[0],
		 (KeyPressMask
#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
		  | KeyReleaseMask
#endif
		  | FocusChangeMask | VisibilityChangeMask
		  | StructureNotifyMask));

/* vt cursor: Black-on-White is standard, but this is more popular */
    r->TermWin_cursor = XCreateFontCursor(r->Xdisplay, XC_xterm);
    rxvt_recolour_cursor(r);

#if defined(HAVE_SCROLLBARS) || defined(MENUBAR)
/* cursor (menuBar/scrollBar): Black-on-White */
    r->h->cursor_leftptr = XCreateFontCursor(r->Xdisplay, XC_left_ptr);
#endif

/* the vt window */
    r->TermWin.vt = XCreateSimpleWindow(r->Xdisplay, r->TermWin.parent[0],
					r->h->window_vt_x, r->h->window_vt_y,
					TermWin_TotalWidth(),
					TermWin_TotalHeight(),
					0,
					r->PixColors[Color_fg],
					r->PixColors[Color_bg]);
#ifdef DEBUG_X
    XStoreName(r->Xdisplay, r->TermWin.vt, "vt window");
#endif
    XDefineCursor(r->Xdisplay, r->TermWin.vt, r->TermWin_cursor);
    XSelectInput(r->Xdisplay, r->TermWin.vt,
		 (ExposureMask | ButtonPressMask | ButtonReleaseMask
		  | PropertyChangeMask
		  | Button1MotionMask | Button3MotionMask));

#if defined(MENUBAR) && (MENUBAR_MAX > 1)
    if (menuBar_height()) {
	r->menuBar.win = XCreateSimpleWindow(r->Xdisplay, r->TermWin.parent[0],
					     r->h->window_vt_x, 0,
					     TermWin_TotalWidth(),
					     menuBar_TotalHeight(),
					     0,
					     r->PixColors[Color_fg],
					     r->PixColors[Color_scroll]);
#ifdef DEBUG_X
    XStoreName(r->Xdisplay, r->menuBar.win, "menubar");
#endif
	XDefineCursor(r->Xdisplay, r->menuBar.win, r->h->cursor_leftptr);
	XSelectInput(r->Xdisplay, r->menuBar.win,
		     (ExposureMask | ButtonPressMask | ButtonReleaseMask
		      | Button1MotionMask));
    }
#endif
#ifdef XPM_BACKGROUND
    if (r->h->rs[Rs_backgroundPixmap] != NULL
	&& !(r->Options & Opt_transparent)) {
	const char     *p = r->h->rs[Rs_backgroundPixmap];

	if ((p = STRCHR(p, ';')) != NULL) {
	    p++;
	    rxvt_scale_pixmap(r, p);
	}
	rxvt_set_bgPixmap(r, r->h->rs[Rs_backgroundPixmap]);
	rxvt_scr_touch(r, True);
    }
#endif

/* graphics context for the vt window */
    gcvalue.font = r->TermWin.font->fid;
    gcvalue.foreground = r->PixColors[Color_fg];
    gcvalue.background = r->PixColors[Color_bg];
    gcvalue.graphics_exposures = 1;
    r->TermWin.gc = XCreateGC(r->Xdisplay, r->TermWin.vt,
			      GCForeground | GCBackground
			      | GCFont | GCGraphicsExposures, &gcvalue);

#if defined(MENUBAR) || defined(RXVT_SCROLLBAR)
    gcvalue.foreground = r->PixColors[Color_topShadow];
    r->h->topShadowGC = XCreateGC(r->Xdisplay, r->TermWin.vt,
				  GCForeground, &gcvalue);
    gcvalue.foreground = r->PixColors[Color_bottomShadow];
    r->h->botShadowGC = XCreateGC(r->Xdisplay, r->TermWin.vt,
				  GCForeground, &gcvalue);
    gcvalue.foreground = r->PixColors[(XDEPTH <= 2 ? Color_fg
						   : Color_scroll)];
    r->h->scrollbarGC = XCreateGC(r->Xdisplay, r->TermWin.vt,
				  GCForeground, &gcvalue);
#endif
}

/*----------------------------------------------------------------------*/
/*
 * Run the command in a subprocess and return a file descriptor for the
 * master end of the pseudo-teletype pair with the command talking to
 * the slave.
 */
/* INTPROTO */
int
rxvt_run_command(rxvt_t *r, const char *const *argv)
{
    int             i, cfd;

/*
 * Close all unused file descriptors
 * We don't want them, we don't need them.
 */
    if ((i = open("/dev/null", O_RDONLY)) < 0) {
	/* TODO: BOO HISS */
	dup2(STDERR_FILENO, STDIN_FILENO);
    } else if (i > STDIN_FILENO) {
	dup2(i, STDIN_FILENO);
	close(i);
    }
    dup2(STDERR_FILENO, STDOUT_FILENO);
    for (i = STDERR_FILENO + 1; i < r->num_fds; i++) {
	if (
#ifdef __sgi			/* Alex Coventry says we need 4 & 7 too */
	    i == 4 || i == 7 ||
#endif
	    i == r->Xfd)
	    continue;
	close(i);
    }
    if ((r->Xfd < STDERR_FILENO + 1
#ifdef FD_SETSIZE
	 || r->Xfd > FD_SETSIZE
#endif
        ) && dup2(r->Xfd, STDERR_FILENO + 1) != -1) {
	close(r->Xfd);
	r->Xfd = STDERR_FILENO + 1;
    }

/* get master (pty) */
    if ((cfd = rxvt_get_pty(&(r->tty_fd), &(r->h->ttydev))) < 0) {
	rxvt_print_error("can't open pseudo-tty");
	return -1;
    }
#ifdef FD_SETSIZE
    if (r->Xfd > FD_SETSIZE || cfd > FD_SETSIZE) {
	rxvt_print_error("fd too high: %d max", FD_SETSIZE);
	rxvt_clean_exit();
	exit(EXIT_FAILURE);
    }
#endif
    fcntl(cfd, F_SETFL, O_NDELAY);

/* get slave (tty) */
    if (r->tty_fd < 0) {
	rxvt_privileged_ttydev(r, SAVE);
	if ((r->tty_fd = rxvt_get_tty(r->h->ttydev)) < 0) {
	    close(cfd);
	    rxvt_print_error("can't open slave tty %s", r->h->ttydev);
	    return -1;
	}
    }
    rxvt_get_ttymode(&(r->h->tio));

/* install exit handler for cleanup */
#ifdef HAVE_ATEXIT
    atexit(rxvt_clean_exit);
#else
# ifdef HAVE_ON_EXIT
    on_exit(rxvt_clean_exit, NULL);	/* non-ANSI exit handler */
# endif
#endif

    signal(SIGHUP, rxvt_Exit_signal);
#ifndef __svr4__
    signal(SIGINT, rxvt_Exit_signal);
#endif
    signal(SIGQUIT, rxvt_Exit_signal);
    signal(SIGTERM, rxvt_Exit_signal);
    signal(SIGCHLD, rxvt_Child_signal);

/* need to trap SIGURG for SVR4 (Unixware) rlogin */
/* signal (SIGURG, SIG_DFL); */

#if !defined(__QNX__)
/* spin off the command interpreter */
    switch (r->h->cmd_pid = fork()) {
    case -1:
	rxvt_print_error("can't fork");
	return -1;
    case 0:
	close(cfd);		/* only keep r->tty_fd and STDERR open */
	close(r->Xfd);
	if (rxvt_control_tty(r->tty_fd, r->h->ttydev) < 0)
	    rxvt_print_error("could not obtain control of tty");
	else {
	/* Reopen stdin, stdout and stderr over the tty file descriptor */
	    dup2(r->tty_fd, STDIN_FILENO);
	    dup2(r->tty_fd, STDOUT_FILENO);
	    dup2(r->tty_fd, STDERR_FILENO);
	    if (r->tty_fd > 2)
		close(r->tty_fd);
	    rxvt_run_child(r, argv);
	}
	exit(EXIT_FAILURE);
	/* NOTREACHED */
    default:
	close(r->tty_fd);	/* keep STDERR_FILENO, r->cmd_fd, r->Xfd open */
	break;
    }
#else				/* __QNX__ uses qnxspawn() */
    fchmod(r->tty_fd, 0622);
    fcntl(r->tty_fd, F_SETFD, FD_CLOEXEC);
    fcntl(cfd, F_SETFD, FD_CLOEXEC);

    if (rxvt_run_child(r, argv) == -1)
	exit(EXIT_FAILURE);
#endif
/*
 * Reduce r->num_fds to what we use, so select() is more efficient
 */
    r->num_fds = max(STDERR_FILENO, cfd);
    MAX_IT(r->num_fds, r->Xfd);
#ifdef __sgi			/* Alex Coventry says we need 4 & 7 too */
    MAX_IT(r->num_fds, 7);
#endif
    r->num_fds++;		/* counts from 0 */

    rxvt_privileged_utmp(r, SAVE);
    return cfd;
}

/* ------------------------------------------------------------------------- *
 *                          CHILD PROCESS OPERATIONS                         *
 * ------------------------------------------------------------------------- */
/*
 * The only open file descriptor is the slave tty - so no error messages.
 * returns are fatal
 */
/* INTPROTO */
int
rxvt_run_child(rxvt_t *r, const char *const *argv)
{
    char           *login;

    SET_TTYMODE(STDIN_FILENO, &(r->h->tio));	/* init terminal attributes */

    if (r->Options & Opt_console) {	/* be virtual console, fail silently */
#ifdef TIOCCONS
	unsigned int    on = 1;

	ioctl(STDIN_FILENO, TIOCCONS, &on);
#elif defined (SRIOCSREDIR)
	int             fd;

	fd = open(CONSOLE, O_WRONLY, 0);
	if (fd >= 0) {
	    if (ioctl(fd, SRIOCSREDIR, NULL) < 0)
		close(fd);
	}
#endif				/* SRIOCSREDIR */
    }

    /* set window size */
    rxvt_tt_winsize(STDIN_FILENO, r->TermWin.ncol, r->TermWin.nrow);

/* reset signals and spin off the command interpreter */
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
/*
 * mimick login's behavior by disabling the job control signals
 * a shell that wants them can turn them back on
 */
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
#endif				/* SIGTSTP */

#ifndef __QNX__
/* command interpreter path */
    if (argv != NULL) {
# ifdef DEBUG_CMD
	int             i;

	for (i = 0; argv[i]; i++)
	    fprintf(stderr, "argv [%d] = \"%s\"\n", i, argv[i]);
# endif
	execvp(argv[0], (char *const *)argv);
	/* no error message: STDERR is closed! */
    } else {
	const char     *argv0, *shell;

	if ((shell = getenv("SHELL")) == NULL || *shell == '\0')
	    shell = "/bin/sh";

	argv0 = (const char *)rxvt_r_basename(shell);
	if (r->Options & Opt_loginShell) {
	    login = rxvt_malloc((STRLEN(argv0) + 2) * sizeof(char));

	    login[0] = '-';
	    STRCPY(&login[1], argv0);
	    argv0 = login;
	}
	execlp(shell, argv0, NULL);
	/* no error message: STDERR is closed! */
    }
#else				/* __QNX__ uses qnxspawn() */
    {
	char            iov_a[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	char           *command = NULL, fullcommand[_MAX_PATH];
	char          **arg_v, *arg_a[2] = { NULL, NULL };

	if (argv != NULL) {
	    if (access(argv[0], X_OK) == -1) {
		if (STRCHR(argv[0], '/') == NULL) {
		    searchenv(argv[0], "PATH", fullcommand);
		    if (fullcommand[0] != '\0')
			command = fullcommand;
		}
		if (access(command, X_OK) == -1)
		    return -1;
	    } else
		command = argv[0];
	    arg_v = argv;
	} else {
	    if ((command = getenv("SHELL")) == NULL || *command == '\0')
		command = "/bin/sh";

	    arg_a[0] = my_basename(command);
	    if (r->Options & Opt_loginShell) {
		login = rxvt_malloc((STRLEN(arg_a[0]) + 2) * sizeof(char));

		login[0] = '-';
		STRCPY(&login[1], arg_a[0]);
		arg_a[0] = login;
	    }
	    arg_v = arg_a;
	}
	iov_a[0] = iov_a[1] = iov_a[2] = r->tty_fd;
	r->h->cmd_pid = qnx_spawn(0, 0, 0, -1, -1,
				  _SPAWN_SETSID | _SPAWN_TCSETPGRP,
				  command, arg_v, environ, iov_a, 0);
	if (login)
	    free(login);
	close(r->tty_fd);
	return r->cmd_fd;
    }
#endif
    return -1;
}

/* ------------------------------------------------------------------------- *
 *                            GET TTY CURRENT STATE                          *
 * ------------------------------------------------------------------------- */
/* rxvt_get_ttymode() */
/* INTPROTO */
void
rxvt_get_ttymode(ttymode_t *tio)
{
#ifdef HAVE_TERMIOS_H
/*
 * standard System V termios interface
 */
    if (GET_TERMIOS(STDIN_FILENO, tio) < 0) {
	/* return error - use system defaults */
	tio->c_cc[VINTR] = CINTR;
	tio->c_cc[VQUIT] = CQUIT;
	tio->c_cc[VERASE] = CERASE;
	tio->c_cc[VKILL] = CKILL;
	tio->c_cc[VSTART] = CSTART;
	tio->c_cc[VSTOP] = CSTOP;
	tio->c_cc[VSUSP] = CSUSP;
# ifdef VDSUSP
	tio->c_cc[VDSUSP] = CDSUSP;
# endif
# ifdef VREPRINT
	tio->c_cc[VREPRINT] = CRPRNT;
# endif
# ifdef VDISCRD
	tio->c_cc[VDISCRD] = CFLUSH;
# endif
# ifdef VWERSE
	tio->c_cc[VWERSE] = CWERASE;
# endif
# ifdef VLNEXT
	tio->c_cc[VLNEXT] = CLNEXT;
# endif
    }
    tio->c_cc[VEOF] = CEOF;
    tio->c_cc[VEOL] = VDISABLE;
# ifdef VEOL2
    tio->c_cc[VEOL2] = VDISABLE;
# endif
# ifdef VSWTC
    tio->c_cc[VSWTC] = VDISABLE;
# endif
# ifdef VSWTCH
    tio->c_cc[VSWTCH] = VDISABLE;
# endif
# if VMIN != VEOF
    tio->c_cc[VMIN] = 1;
# endif
# if VTIME != VEOL
    tio->c_cc[VTIME] = 0;
# endif

/* input modes */
    tio->c_iflag = (BRKINT | IGNPAR | ICRNL
# ifdef IMAXBEL
		    | IMAXBEL
# endif
	            | IXON);

/* output modes */
    tio->c_oflag = (OPOST | ONLCR);

/* control modes */
    tio->c_cflag = (CS8 | CREAD);

/* line discipline modes */
    tio->c_lflag = (ISIG | ICANON | IEXTEN | ECHO
# if defined (ECHOCTL) && defined (ECHOKE)
		    | ECHOCTL | ECHOKE
# endif
	            | ECHOE | ECHOK);
# else				/* HAVE_TERMIOS_H */

/*
 * sgtty interface
 */

/* get parameters -- gtty */
    if (ioctl(STDIN_FILENO, TIOCGETP, &(tio->sg)) < 0) {
	tio->sg.sg_erase = CERASE;	/* ^H */
	tio->sg.sg_kill = CKILL;	/* ^U */
    }
    tio->sg.sg_flags = (CRMOD | ECHO | EVENP | ODDP);

/* get special characters */
    if (ioctl(STDIN_FILENO, TIOCGETC, &(tio->tc)) < 0) {
	tio->tc.t_intrc = CINTR;	/* ^C */
	tio->tc.t_quitc = CQUIT;	/* ^\ */
	tio->tc.t_startc = CSTART;	/* ^Q */
	tio->tc.t_stopc = CSTOP;	/* ^S */
	tio->tc.t_eofc = CEOF;	/* ^D */
	tio->tc.t_brkc = -1;
    }
/* get local special chars */
    if (ioctl(STDIN_FILENO, TIOCGLTC, &(tio->lc)) < 0) {
	tio->lc.t_suspc = CSUSP;	/* ^Z */
	tio->lc.t_dsuspc = CDSUSP;	/* ^Y */
	tio->lc.t_rprntc = CRPRNT;	/* ^R */
	tio->lc.t_flushc = CFLUSH;	/* ^O */
	tio->lc.t_werasc = CWERASE;	/* ^W */
	tio->lc.t_lnextc = CLNEXT;	/* ^V */
    }
/* get line discipline */
    ioctl(STDIN_FILENO, TIOCGETD, &(tio->line));
# ifdef NTTYDISC
    tio->line = NTTYDISC;
# endif				/* NTTYDISC */
    tio->local = (LCRTBS | LCRTERA | LCTLECH | LPASS8 | LCRTKIL);
#endif				/* HAVE_TERMIOS_H */

/*
 * Debugging
 */
#ifdef DEBUG_TTYMODE
#ifdef HAVE_TERMIOS_H
/* c_iflag bits */
    fprintf(stderr, "Input flags\n");

/* cpp token stringize doesn't work on all machines <sigh> */
# define FOO(flag,name)			\
    if ((tio->c_iflag) & flag)		\
	fprintf (stderr, "%s ", name)

/* c_iflag bits */
    FOO(IGNBRK, "IGNBRK");
    FOO(BRKINT, "BRKINT");
    FOO(IGNPAR, "IGNPAR");
    FOO(PARMRK, "PARMRK");
    FOO(INPCK, "INPCK");
    FOO(ISTRIP, "ISTRIP");
    FOO(INLCR, "INLCR");
    FOO(IGNCR, "IGNCR");
    FOO(ICRNL, "ICRNL");
    FOO(IXON, "IXON");
    FOO(IXOFF, "IXOFF");
# ifdef IUCLC
    FOO(IUCLC, "IUCLC");
# endif
# ifdef IXANY
    FOO(IXANY, "IXANY");
# endif
# ifdef IMAXBEL
    FOO(IMAXBEL, "IMAXBEL");
# endif
    fprintf(stderr, "\n");

# undef FOO
# define FOO(entry, name)					\
    fprintf(stderr, "%-8s = %#04o\n", name, tio->c_cc [entry])

    FOO(VINTR, "VINTR");
    FOO(VQUIT, "VQUIT");
    FOO(VERASE, "VERASE");
    FOO(VKILL, "VKILL");
    FOO(VEOF, "VEOF");
    FOO(VEOL, "VEOL");
# ifdef VEOL2
    FOO(VEOL2, "VEOL2");
# endif
# ifdef VSWTC
    FOO(VSWTC, "VSWTC");
# endif
# ifdef VSWTCH
    FOO(VSWTCH, "VSWTCH");
# endif
    FOO(VSTART, "VSTART");
    FOO(VSTOP, "VSTOP");
    FOO(VSUSP, "VSUSP");
# ifdef VDSUSP
    FOO(VDSUSP, "VDSUSP");
# endif
# ifdef VREPRINT
    FOO(VREPRINT, "VREPRINT");
# endif
# ifdef VDISCRD
    FOO(VDISCRD, "VDISCRD");
# endif
# ifdef VWERSE
    FOO(VWERSE, "VWERSE");
# endif
# ifdef VLNEXT
    FOO(VLNEXT, "VLNEXT");
# endif
    fprintf(stderr, "\n");
# undef FOO
# endif				/* HAVE_TERMIOS_H */
#endif				/* DEBUG_TTYMODE */
}

/*----------------------- end-of-file (C source) -----------------------*/
