/*--------------------------------*-C-*---------------------------------*
 * File:	xdefaults.c
 *----------------------------------------------------------------------*
 * $Id: xdefaults.c,v 1.1 2002-12-06 23:08:03 earnie Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (C) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 *				- original version
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
/*----------------------------------------------------------------------*
 * get resources from ~/.Xdefaults or ~/.Xresources with the memory-saving
 * default or with XGetDefault() (#define USE_XGETDEFAULT)
 *----------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "version.h"
#include "xdefaults.intpro"	/* PROTOS for internal routines */

/* #define DEBUG_RESOURCES */

static const char *const xnames[2] = { ".Xdefaults", ".Xresources" };

/*{{{ monolithic option/resource structure: */
/*
 * `string' options MUST have a usage argument
 * `switch' and `boolean' options have no argument
 * if there's no desc(ription), it won't appear in usage()
 */

/* INFO() - descriptive information only */
#define INFO(opt, arg, desc)					\
    {0, NULL, NULL, (opt), (arg), (desc)}

/* STRG() - command-line option, with/without resource */
#define STRG(rsp, kw, opt, arg, desc)				\
    {0, &(rsp), (kw), (opt), (arg), (desc)}

/* RSTRG() - resource/long-option */
#define RSTRG(rsp, kw, arg)					\
    {0, &(rsp), (kw), NULL, (arg), NULL}

/* BOOL() - regular boolean `-/+' flag */
#define BOOL(rsp, kw, opt, flag, desc)				\
    {(Opt_Boolean|(flag)), &(rsp), (kw), (opt), NULL, (desc)}

/* SWCH() - `-' flag */
#define SWCH(opt, flag, desc)					\
    {(flag), NULL, NULL, (opt), NULL, (desc)}

/* convenient macros */
#define optList_STRLEN(i)						\
    (optList[i].flag ? 0 : (optList[i].arg ? STRLEN (optList[i].arg) : 1))
#define optList_isBool(i)						\
    (optList[i].flag & Opt_Boolean)
#define optList_isReverse(i)						\
    (optList[i].flag & Opt_Reverse)
#define optList_size()							\
    (sizeof(optList) / sizeof(optList[0]))

static const struct {
    const unsigned long flag;	/* Option flag */
    const char    **dp;		/* data pointer */
    const char     *kw;		/* keyword */
    const char     *opt;	/* option */
    const char     *arg;	/* argument */
    const char     *desc;	/* description */
} optList[] = {
    STRG(rs.display_name, NULL, "d", NULL, NULL),	/* short form */
    STRG(rs.display_name, NULL, "display", "string", "X server to contact"),
    STRG(rs.term_name, "termName", "tn", "string",
	 "value of the TERM environment variable"),
    STRG(rs.geometry, NULL, "g", NULL, NULL),	/* short form */
    STRG(rs.geometry, "geometry", "geometry", "geometry",
	 "size (in characters) and position"),
    SWCH("C", Opt_console, "intercept console messages"),
    SWCH("iconic", Opt_iconic, "start iconic"),
    SWCH("ic", Opt_iconic, NULL),	/* short form */
    BOOL(rs.reverseVideo, "reverseVideo", "rv", Opt_reverseVideo,
	 "reverse video"),
    BOOL(rs.loginShell, "loginShell", "ls", Opt_loginShell, "login shell"),
    BOOL(rs.scrollBar, "scrollBar", "sb", Opt_scrollBar, "scrollbar"),
    BOOL(rs.scrollBar_right, "scrollBar_right", "sr", Opt_scrollBar_right,
	 "scrollbar right"),
    BOOL(rs.scrollBar_floating, "scrollBar_floating", "st",
	 Opt_scrollBar_floating, "scrollbar without a trough"),
    BOOL(rs.scrollTtyOutput, "scrollTtyOutput", NULL, Opt_scrollTtyOutput,
	 NULL),
    SWCH("si", Opt_Boolean | Opt_Reverse | Opt_scrollTtyOutput,
	 "scroll-on-tty-output inhibit"),
    BOOL(rs.scrollKeypress, "scrollTtyKeypress", "sk", Opt_scrollKeypress,
	 "scroll-on-keypress"),
#ifdef TRANSPARENT
    BOOL(rs.transparent, "inheritPixmap", "ip", Opt_transparent,
	 "inherit parent pixmap"),
    SWCH("tr", Opt_transparent, NULL),
#endif
    BOOL(rs.utmpInhibit, "utmpInhibit", "ut", Opt_utmpInhibit, "utmp inhibit"),
#ifndef NO_BELL
    BOOL(rs.visualBell, "visualBell", "vb", Opt_visualBell, "visual bell"),
# if ! defined(NO_MAPALERT) && defined(MAPALERT_OPTION)
    BOOL(rs.mapAlert, "mapAlert", NULL, Opt_mapAlert, NULL),
# endif
#endif
#ifdef META8_OPTION
    BOOL(rs.meta8, "meta8", NULL, Opt_meta8, NULL),
#endif
    STRG(rs.color[Color_bg], "background", "bg", "color", "background color"),
    STRG(rs.color[Color_fg], "foreground", "fg", "color", "foreground color"),
    RSTRG(rs.color[minCOLOR + 0], "color0", "color"),
    RSTRG(rs.color[minCOLOR + 1], "color1", "color"),
    RSTRG(rs.color[minCOLOR + 2], "color2", "color"),
    RSTRG(rs.color[minCOLOR + 3], "color3", "color"),
    RSTRG(rs.color[minCOLOR + 4], "color4", "color"),
    RSTRG(rs.color[minCOLOR + 5], "color5", "color"),
    RSTRG(rs.color[minCOLOR + 6], "color6", "color"),
    RSTRG(rs.color[minCOLOR + 7], "color7", "color"),
#ifndef NO_BRIGHTCOLOR
    RSTRG(rs.color[minBrightCOLOR + 0], "color8", "color"),
    RSTRG(rs.color[minBrightCOLOR + 1], "color9", "color"),
    RSTRG(rs.color[minBrightCOLOR + 2], "color10", "color"),
    RSTRG(rs.color[minBrightCOLOR + 3], "color11", "color"),
    RSTRG(rs.color[minBrightCOLOR + 4], "color12", "color"),
    RSTRG(rs.color[minBrightCOLOR + 5], "color13", "color"),
    RSTRG(rs.color[minBrightCOLOR + 6], "color14", "color"),
    RSTRG(rs.color[minBrightCOLOR + 7], "color15", "color"),
#endif				/* NO_BRIGHTCOLOR */
#ifndef NO_BOLDUNDERLINE
    RSTRG(rs.color[Color_BD], "colorBD", "color"),
    RSTRG(rs.color[Color_UL], "colorUL", "color"),
#endif				/* NO_BOLDUNDERLINE */
#ifdef KEEP_SCROLLCOLOR
    RSTRG(rs.color[Color_scroll], "scrollColor", "color"),
    RSTRG(rs.color[Color_trough], "troughColor", "color"),
#endif				/* KEEP_SCROLLCOLOR */
#if defined (XPM_BACKGROUND) || (MENUBAR_MAX)
    RSTRG(rs.path, "path", "search path"),
#endif				/* defined (XPM_BACKGROUND) || (MENUBAR_MAX) */
#ifdef XPM_BACKGROUND
    STRG(rs.backgroundPixmap, "backgroundPixmap", "pixmap", "file[;geom]",
	 "background pixmap"),
#endif				/* XPM_BACKGROUND */
#if (MENUBAR_MAX)
    RSTRG(rs.menu, "menu", "name[;tag]"),
#endif
#ifndef NO_BOLDFONT
    STRG(rs.boldFont, "boldFont", "fb", "fontname", "bold text font"),
#endif
    STRG(rs.font[0], "font", "fn", "fontname", "normal text font"),
/* fonts: command-line option = resource name */
#if NFONTS > 1
    RSTRG(rs.font[1], "font1", "fontname"),
#endif
#if NFONTS > 2
    RSTRG(rs.font[2], "font2", "fontname"),
#endif
#if NFONTS > 3
    RSTRG(rs.font[3], "font3", "fontname"),
#endif
#if NFONTS > 4
    RSTRG(rs.font[4], "font4", "fontname"),
#endif
#if NFONTS > 5
    RSTRG(rs.font[5], "font5", "fontname"),
#endif
#if NFONTS > 6
    RSTRG(rs.font[6], "font6", "fontname"),
#endif
#if NFONTS > 7
    RSTRG(rs.font[7], "font7", "fontname"),
#endif
#ifdef MULTICHAR_SET
    STRG(rs.mfont[0], "mfont", "fm", "fontname", "multichar font"),
/* fonts: command-line option = resource name */
# if NFONTS > 1
    RSTRG(rs.mfont[1], "mfont1", "fontname"),
# endif
# if NFONTS > 2
    RSTRG(rs.mfont[2], "mfont2", "fontname"),
# endif
# if NFONTS > 3
    RSTRG(rs.mfont[3], "mfont3", "fontname"),
# endif
# if NFONTS > 4
    RSTRG(rs.mfont[4], "mfont4", "fontname"),
# endif
# if NFONTS > 5
    RSTRG(rs.mfont[5], "mfont5", "fontname"),
# endif
# if NFONTS > 6
    RSTRG(rs.mfont[6], "mfont6", "fontname"),
# endif
# if NFONTS > 7
    RSTRG(rs.mfont[7], "mfont7", "fontname"),
# endif
#endif				/* MULTICHAR_SET */
#ifdef MULTICHAR_SET
    STRG(rs.multichar_encoding, "multichar_encoding", "km", "mode",
	 "multiple-character font encoding; mode = eucj | sjis | big5 | gb | kr"),
#endif				/* MULTICHAR_SET */
#ifdef USE_XIM
    STRG(rs.preeditType, "preeditType", "pt", "style",
	 "input style of input method; style = OverTheSpot | OffTheSpot | Root"),
    STRG(rs.inputMethod, "inputMethod", "im", "name", "name of input method"),
#endif				/* USE_XIM */
#ifdef GREEK_SUPPORT
    STRG(rs.greek_keyboard, "greek_keyboard", "grk", "mode",
	 "greek keyboard mapping; mode = iso | ibm"),
#endif
    STRG(rs.name, NULL, "name", "string",
	 "client instance, icon, and title strings"),
    STRG(rs.title, "title", "title", "string", "title name for window"),
    STRG(rs.title, NULL, "T", NULL, NULL),	/* short form */
    STRG(rs.iconName, "iconName", "n", "string", "icon name for window"),
#ifndef NO_CURSORCOLOR
    STRG(rs.color[Color_cursor], "cursorColor", "cr", "color", "cursor color"),
/* command-line option = resource name */
    RSTRG(rs.color[Color_cursor2], "cursorColor2", "color"),
#endif				/* NO_CURSORCOLOR */
    STRG(rs.color[Color_pointer], "pointerColor", "pr", "color",
	 "pointer color"),
    STRG(rs.color[Color_border], "borderColor", "bd", "color",
	 "border color"),
    STRG(rs.saveLines, "saveLines", "sl", "number",
	 "number of scrolled lines to save"),
#ifndef NO_FRILLS
    STRG(rs.ext_bwidth, "externalBorder", "w", "number",
	 "external border in pixels"),
    STRG(rs.ext_bwidth, NULL, "bw", NULL, NULL),
    STRG(rs.ext_bwidth, NULL, "borderwidth", NULL, NULL),
    STRG(rs.int_bwidth, "internalBorder", "b", "number",
	 "internal border in pixels"),
#endif
#ifndef NO_BACKSPACE_KEY
    RSTRG(rs.backspace_key, "backspacekey", "string"),
#endif
#ifndef NO_DELETE_KEY
    RSTRG(rs.delete_key, "deletekey", "string"),
#endif
    RSTRG(rs.selectstyle, "selectstyle", "string"),
#ifdef PRINTPIPE
    RSTRG(rs.print_pipe, "print-pipe", "string"),
#endif
#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
    RSTRG(rs.bigfont_key, "bigfont_key", "keysym"),
    RSTRG(rs.smallfont_key, "smallfont_key", "keysym"),
#endif
    STRG(rs.modifier, "modifier", "mod", "modifier",
	 "alt | meta | hyper | super | mod1 | ... | mod5"),
#ifdef CUTCHAR_RESOURCE
    RSTRG(rs.cutchars, "cutchars", "string"),
#endif				/* CUTCHAR_RESOURCE */
    INFO("e", "command arg ...", "command to execute")
};

#undef INFO
#undef STRG
#undef RSTRG
#undef SWCH
#undef BOOL
/*}}} */

/*{{{ usage: */
/*----------------------------------------------------------------------*/
/* EXTPROTO */
void
usage(int type)
{
    int             i, col;

#define INDENT 30
    fprintf(stderr, "\nUsage v%s : (", VERSION);
#ifdef XPM_BACKGROUND
# ifdef XPM_BUFFERING
    fprintf(stderr, "XPM-buffer,");
# else
    fprintf(stderr, "XPM,");
# endif
#endif
#ifdef UTMP_SUPPORT
    fprintf(stderr, "utmp,");
#endif
#ifdef MENUBAR
    fprintf(stderr, "menubar,");
#endif
#ifdef USE_XIM
    fprintf(stderr, "XIM,");
#endif
#ifdef MULTICHAR_SET
    fprintf(stderr, "multichar languages,");
#endif
#ifdef XTERM_SCROLLBAR
    fprintf(stderr, "XTerm-scrollbar,");
#endif
#ifdef GREEK_SUPPORT
    fprintf(stderr, "Greek,");
#endif
#ifdef RXVT_GRAPHICS
    fprintf(stderr, "graphics,");
#endif
#ifdef NO_BACKSPACE_KEY
    fprintf(stderr, "no backspace,");
#endif
#ifdef NO_DELETE_KEY
    fprintf(stderr, "no delete,");
#endif
#ifdef NO_RESOURCES
    fprintf(stderr, "NoResources");
#else
# ifdef USE_XGETDEFAULT
    fprintf(stderr, "XGetDefaults");
# else
    fprintf(stderr, xnames[0]);
# endif
#endif

    fprintf(stderr, ")\n%s", APL_NAME);
    switch (type) {
    case 0:			/* brief listing */
	fprintf(stderr, " [-help]\n");
	col = 3;
	for (i = 0; i < optList_size(); i++) {
	    if (optList[i].desc != NULL) {
		int             len = 2;

		if (!optList_isBool(i)) {
		    len = optList_STRLEN(i);
		    if (len > 0)
			len++;	/* account for space */
		}
		len += 4 + STRLEN(optList[i].opt);

		col += len;
		if (col > 79) {	/* assume regular width */
		    fprintf(stderr, "\n");
		    col = 3 + len;
		}
		fprintf(stderr, " [-");
		if (optList_isBool(i))
		    fprintf(stderr, "/+");
		fprintf(stderr, "%s", optList[i].opt);
		if (optList_STRLEN(i))
		    fprintf(stderr, " %s]", optList[i].arg);
		else
		    fprintf(stderr, "]");
	    }
	}
	fprintf(stderr, "\n\n");
	break;

    case 1:			/* full command-line listing */
	fprintf(stderr, " [options] [-e command args]\n\n"
		"where options include:\n");

	for (i = 0; i < optList_size(); i++)
	    if (optList[i].desc != NULL)
		fprintf(stderr, "    %s%s %-*s%s%s\n",
			(optList_isBool(i) ? "-/+" : "-"),
			optList[i].opt, (INDENT - STRLEN(optList[i].opt)
					 + (optList_isBool(i) ? 0 : 2)),
			(optList[i].arg ? optList[i].arg : ""),
			(optList_isBool(i) ? "turn on/off " : ""),
			optList[i].desc);
	fprintf(stderr, "\n    --help to list long-options\n\n");
	break;

    case 2:			/* full resource listing */
	fprintf(stderr,
		" [options] [-e command args]\n\n"
		"where resources (long-options) include:\n");

	for (i = 0; i < optList_size(); i++)
	    if (optList[i].kw != NULL)
		fprintf(stderr, "    %s: %*s\n",
			optList[i].kw,
			(INDENT - STRLEN(optList[i].kw)),
			(optList_isBool(i) ? "boolean" : optList[i].arg));

#ifdef KEYSYM_RESOURCE
	fprintf(stderr, "    " "keysym.sym" ": %*s\n",
		(INDENT - sizeof("keysym.sym") + 1), "keysym");
#endif
	fprintf(stderr, "\n    -help to list options\n\n");
	break;
    }
    exit(EXIT_FAILURE);
    /* NOTREACHED */
}

/*}}} */

/*{{{ get command-line options before getting resources */
/* EXTPROTO */
void
get_options(int argc, const char *const *argv)
{
    int             i, bad_option = 0;
    static const char On[3] = "ON", Off[4] = "OFF";

    for (i = 1; i < argc; i++) {
	int             entry, longopt = 0;
	const char     *flag, *opt;

	opt = argv[i];
#ifdef DEBUG_RESOURCES
	fprintf(stderr, "argv[%d] = %s: ", i, opt);
#endif
	if (*opt == '-') {
	    flag = On;
	    if (*++opt == '-')
		longopt = *opt++;	/* long option */
	} else if (*opt == '+') {
	    flag = Off;
	    if (*++opt == '+')
		longopt = *opt++;	/* long option */
	} else {
	    bad_option = 1;
	    print_error("bad option \"%s\"", opt);
	    continue;
	}

	if (!STRCMP(opt, "help"))
	    usage(longopt ? 2 : 1);
	if (!STRCMP(opt, "h"))
	    usage(0);

	/* feature: always try to match long-options */
	for (entry = 0; entry < optList_size(); entry++)
	    if ((optList[entry].kw && !STRCMP(opt, optList[entry].kw))
		|| (!longopt
		    && optList[entry].opt && !STRCMP(opt, optList[entry].opt)))
		break;

	if (entry < optList_size()) {
	    if (optList_isReverse(entry))
		flag = flag == On ? Off : On;
	    if (optList_STRLEN(entry)) {	/* string value */
		const char     *str = argv[++i];

#ifdef DEBUG_RESOURCES
		fprintf(stderr, "string (%s,%s) = ",
			optList[entry].opt ? optList[entry].opt : "nil",
			optList[entry].kw ? optList[entry].kw : "nil");
#endif
		if (flag == On && str && optList[entry].dp) {
#ifdef DEBUG_RESOURCES
		    fprintf(stderr, "\"%s\"\n", str);
#endif
		    *(optList[entry].dp) = str;
		    /*
		     * special cases are handled in main.c:main() to allow
		     * X resources to set these values before we settle for
		     * default values
		     */
		}
#ifdef DEBUG_RESOURCES
		else
		    fprintf(stderr, "???\n");
#endif
	    } else {		/* boolean value */
#ifdef DEBUG_RESOURCES
		fprintf(stderr, "boolean (%s,%s) = %s\n",
			optList[entry].opt, optList[entry].kw, flag);
#endif
		if (flag == On)
		    Options |= (optList[entry].flag);
		else
		    Options &= ~(optList[entry].flag);

		if (optList[entry].dp)
		    *(optList[entry].dp) = flag;
	    }
	} else
#ifdef KEYSYM_RESOURCE
	    /* if (!STRNCMP(opt, "keysym.", sizeof("keysym.") - 1)) */
	if (Str_match(opt, "keysym.")) {
	    const char     *str = argv[++i];

	    if (str != NULL)
		parse_keysym(opt + sizeof("keysym.") - 1, str);
	} else
#endif
	{
	    /*
	     * various old-style options, just ignore
	     * Obsolete since about Jan 96,
	     * so they can probably eventually be removed
	     */
	    const char     *msg = "bad";

	    if (longopt) {
		opt--;
		bad_option = 1;
	    } else if (!STRCMP(opt, "7") || !STRCMP(opt, "8")
#ifdef GREEK_SUPPORT
		       /* obsolete 12 May 1996 (v2.17) */
		       || !Str_match(opt, "grk")
#endif
		)
		msg = "obsolete";
	    else
		bad_option = 1;

	    print_error("%s option \"%s\"", msg, --opt);
	}
    }

    if (bad_option)
	usage(0);
}

/*}}} */

#ifndef NO_RESOURCES
/*----------------------------------------------------------------------*/

# ifdef KEYSYM_RESOURCE
/*
 * Define key from XrmEnumerateDatabase.
 *   quarks will be something like
 *      "rxvt" "keysym" "0xFF01"
 *   value will be a string
 */
/* ARGSUSED */
/* INTPROTO */
Bool
define_key(XrmDatabase *database, XrmBindingList bindings, XrmQuarkList quarks, XrmRepresentation *type, XrmValue *value, XPointer closure)
{
    int             last;

    for (last = 0; quarks[last] != NULLQUARK; last++)	/* look for last quark in list */
	;
    last--;
    parse_keysym(XrmQuarkToString(quarks[last]), (char *)value->addr);
    return False;
}

/*
 * look for something like this (XK_Delete)
 * rxvt*keysym.0xFFFF: "\177"
 *
 * arg will be
 *      NULL for ~/.Xdefaults and
 *      non-NULL for command-line options (need to allocate)
 */
#define NEWARGLIM	500	/* `reasonable' size */
/* INTPROTO */
int
parse_keysym(const char *str, const char *arg)
{
    int             n, sym;
    char           *key_string, *newarg = NULL;
    char            newargstr[NEWARGLIM];

    if (arg == NULL) {
	if ((n = Str_match(str, "keysym.")) == 0)
	    return 0;
	str += n;		/* skip `keysym.' */
    }
/* some scanf() have trouble with a 0x prefix */
    if (isdigit(str[0])) {
	if (str[0] == '0' && toupper(str[1]) == 'X')
	    str += 2;
	if (arg) {
	    if (sscanf(str, (STRCHR(str, ':') ? "%x:" : "%x"), &sym) != 1)
		return -1;
	} else {
	    if (sscanf(str, "%x:", &sym) != 1)
		return -1;

	    /* cue to ':', it's there since sscanf() worked */
	    STRNCPY(newargstr, STRCHR(str, ':') + 1, NEWARGLIM - 1);
	    newargstr[NEWARGLIM - 1] = '\0';
	    newarg = newargstr;
	}
    } else {
	/*
	 * convert keysym name to keysym number
	 */
	STRNCPY(newargstr, str, NEWARGLIM - 1);
	newargstr[NEWARGLIM - 1] = '\0';
	if (arg == NULL) {
	    if ((newarg = STRCHR(newargstr, ':')) == NULL)
		return -1;
	    *newarg++ = '\0';	/* terminate keysym name */
	}
	if ((sym = XStringToKeysym(newargstr)) == None)
	    return -1;
    }

    if (sym < 0xFF00 || sym > 0xFFFF)	/* we only do extended keys */
	return -1;
    sym &= 0xFF;
    if (KeySym_map[sym] != NULL)	/* already set ? */
	return -1;

    if (newarg == NULL) {
	STRNCPY(newargstr, arg, NEWARGLIM - 1);
	newargstr[NEWARGLIM - 1] = '\0';
	newarg = newargstr;
    }
    (void)Str_trim(newarg);
    if (*newarg == '\0' || (n = Str_escaped(newarg)) == 0)
	return -1;
    MIN_IT(n, 255);
    key_string = MALLOC((n + 1) * sizeof(char));

    key_string[0] = n;
    STRNCPY(key_string + 1, newarg, n);
    KeySym_map[sym] = (unsigned char *)key_string;

    return 1;
}

# endif				/* KEYSYM_RESOURCE */

# ifndef USE_XGETDEFAULT
/*{{{ get_xdefaults() */
/*
 * the matching algorithm used for memory-save fake resources
 */
/* INTPROTO */
void
get_xdefaults(FILE *stream, const char *name)
{
    unsigned int    len;
    char           *str, buffer[256];

    if (stream == NULL)
	return;
    len = STRLEN(name);
    while ((str = fgets(buffer, sizeof(buffer), stream)) != NULL) {
	unsigned int    entry, n;

	while (*str && isspace(*str))
	    str++;		/* leading whitespace */

	if ((str[len] != '*' && str[len] != '.')
	    || (len && STRNCMP(str, name, len)))
	    continue;
	str += (len + 1);	/* skip `name*' or `name.' */

# ifdef KEYSYM_RESOURCE
	if (!parse_keysym(str, NULL))
# endif				/* KEYSYM_RESOURCE */
	    for (entry = 0; entry < optList_size(); entry++) {
		const char     *kw = optList[entry].kw;

		if (kw == NULL)
		    continue;
		n = STRLEN(kw);
		if (str[n] == ':' && Str_match(str, kw)) {
		    /* skip `keyword:' */
		    str += (n + 1);
		    (void)Str_trim(str);
		    n = STRLEN(str);
		    if (n && *(optList[entry].dp) == NULL) {
			/* not already set */
			int             s;
			char           *p = MALLOC((n + 1) * sizeof(char));

			STRCPY(p, str);
			*(optList[entry].dp) = p;
			if (optList_isBool(entry)) {
			    s = STRCASECMP(str, "TRUE") == 0
				|| STRCASECMP(str, "YES") == 0
				|| STRCASECMP(str, "ON") == 0
				|| STRCASECMP(str, "1") == 0;
			    if (optList_isReverse(entry))
				s = !s;
			    if (s)
				Options |= (optList[entry].flag);
			    else
				Options &= ~(optList[entry].flag);
			}
		    }
		    break;
		}
	    }
    }
    rewind(stream);
}

/*}}} */
# endif				/* ! USE_XGETDEFAULT */
#endif				/* NO_RESOURCES */

/*{{{ read the resources files */
/*
 * using XGetDefault() or the hand-rolled replacement
 */
/* ARGSUSED */
/* EXTPROTO */
void
extract_resources(Display *display, const char *name)
{
#ifndef NO_RESOURCES
# ifdef USE_XGETDEFAULT
/*
 * get resources using the X library function
 */
    int             entry;

#  ifdef XrmEnumOneLevel
    int             i;
    char           *displayResource, *ptr, *xe;
    XrmName         name_prefix[3];
    XrmClass        class_prefix[3];
    XrmDatabase     database, rdb1;
    char            fname[1024];

    XrmInitialize();
    database = NULL;

/* Get any Xserver defaults */

    displayResource = XResourceManagerString(display);
    if (displayResource != NULL)
	database = XrmGetStringDatabase(displayResource);

/* Add in ~/.Xdefaults & ~/.Xresources */

    if ((ptr = (char *)getenv("HOME")) == NULL)
	ptr = ".";

    for (i = 0; i < (sizeof(xnames) / sizeof(xnames[0])); i++) {
	sprintf(fname, "%-.*s/%s", sizeof(fname) - STRLEN(xnames[i]) - 2, ptr,
		xnames[i]);
	if ((rdb1 = XrmGetFileDatabase(fname)) != NULL)
	    XrmMergeDatabases(rdb1, &database);
    }

/* Add in XENVIRONMENT file */

    if ((xe = (char *)getenv("XENVIRONMENT")) != NULL
	&& (rdb1 = XrmGetFileDatabase(xe)) != NULL)
	XrmMergeDatabases(rdb1, &database);

/* Add in Rxvt file */

    if ((rdb1 = XrmGetFileDatabase(XAPPLOADDIR "/" APL_SUBCLASS)) != NULL)
	XrmMergeDatabases(rdb1, &database);

    XrmSetDatabase(display, database);
#  endif

/*
 * Query resources for options that affect us
 */
    for (entry = 0; entry < optList_size(); entry++) {
	int             s;
	char           *p;
	const char     *kw = optList[entry].kw;

	if (kw == NULL || *(optList[entry].dp) != NULL)
	    continue;		/* previously set */
	if ((p = XGetDefault(display, name, kw)) != NULL
	    || (p = XGetDefault(display, APL_SUBCLASS, kw)) != NULL
	    || (p = XGetDefault(display, APL_CLASS, kw)) != NULL) {
	    *optList[entry].dp = p;

	    if (optList_isBool(entry)) {
		s = STRCASECMP(p, "TRUE") == 0
		    || STRCASECMP(p, "YES") == 0
		    || STRCASECMP(p, "ON") == 0
		    || STRCASECMP(p, "1") == 0;
		if (optList_isReverse(entry))
		    s = !s;
		if (s)
		    Options |= (optList[entry].flag);
		else
		    Options &= ~(optList[entry].flag);
	    }
	}
    }

/*
 * [R5 or later]: enumerate the resource database
 */
#  ifdef XrmEnumOneLevel
#   ifdef KEYSYM_RESOURCE
    name_prefix[0] = XrmStringToName(name);
    name_prefix[1] = XrmStringToName("keysym");
    name_prefix[2] = NULLQUARK;
    class_prefix[0] = XrmStringToName(APL_SUBCLASS);
    class_prefix[1] = XrmStringToName("Keysym");
    class_prefix[2] = NULLQUARK;
    XrmEnumerateDatabase(XrmGetDatabase(display), name_prefix, class_prefix,
			 XrmEnumOneLevel, define_key, NULL);
    name_prefix[0] = XrmStringToName(APL_CLASS);
    name_prefix[1] = XrmStringToName("keysym");
    class_prefix[0] = XrmStringToName(APL_CLASS);
    class_prefix[1] = XrmStringToName("Keysym");
    XrmEnumerateDatabase(XrmGetDatabase(display), name_prefix, class_prefix,
			 XrmEnumOneLevel, define_key, NULL);
#   endif
#  endif

# else				/* USE_XGETDEFAULT */
/* get resources the hard way, but save lots of memory */
    FILE           *fd = NULL;
    char           *home;

    if ((home = getenv("HOME")) != NULL) {
	int             i, len = STRLEN(home) + 2;
	char           *f = NULL;

	for (i = 0; i < (sizeof(xnames) / sizeof(xnames[0])); i++) {
	    f = REALLOC(f, (len + STRLEN(xnames[i])) * sizeof(char));

	    sprintf(f, "%s/%s", home, xnames[i]);

	    if ((fd = fopen(f, "r")) != NULL)
		break;
	}
	FREE(f);
    }
/*
 * The normal order to match resources is the following:
 * @ global resources (partial match, ~/.Xdefaults)
 * @ application file resources (XAPPLOADDIR/Rxvt)
 * @ class resources (~/.Xdefaults)
 * @ private resources (~/.Xdefaults)
 *
 * However, for the hand-rolled resources, the matching algorithm
 * checks if a resource string value has already been allocated
 * and won't overwrite it with (in this case) a less specific
 * resource value.
 *
 * This avoids multiple allocation.  Also, when we've called this
 * routine command-line string options have already been applied so we
 * needn't to allocate for those resources.
 *
 * So, search in resources from most to least specific.
 *
 * Also, use a special sub-class so that we can use either or both of
 * "XTerm" and "Rxvt" as class names.
 */

    get_xdefaults(fd, name);
    get_xdefaults(fd, APL_SUBCLASS);

#  if defined(XAPPLOADDIR) && defined(USE_XAPPLOADDIR)
    {
	FILE           *ad = fopen(XAPPLOADDIR "/" APL_SUBCLASS, "r");

	if (ad != NULL) {
	    get_xdefaults(ad, "");
	    fclose(ad);
	}
    }
#  endif			/* XAPPLOADDIR */

    get_xdefaults(fd, APL_CLASS);
    get_xdefaults(fd, "");	/* partial match */
    if (fd != NULL)
	fclose(fd);
# endif				/* USE_XGETDEFAULT */
#endif				/* NO_RESOURCES */

/*
 * even without resources, at least do this setup for command-line
 * options and command-line long options
 */
#ifdef MULTICHAR_SET
    set_multichar_encoding(rs.multichar_encoding);
#endif
#ifdef GREEK_SUPPORT
/* this could be a function in grkelot.c */
/* void set_greek_keyboard (const char * str); */
    if (rs.greek_keyboard) {
	if (!STRCMP(rs.greek_keyboard, "iso"))
	    greek_setmode(GREEK_ELOT928);	/* former -grk9 */
	else if (!STRCMP(rs.greek_keyboard, "ibm"))
	    greek_setmode(GREEK_IBM437);	/* former -grk4 */
    }
#endif				/* GREEK_SUPPORT */

#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
    {
	KeySym          sym;

	if (rs.bigfont_key
	    && ((sym = XStringToKeysym(rs.bigfont_key)) != 0))
	    ks_bigfont = sym;
	if (rs.smallfont_key
	    && ((sym = XStringToKeysym(rs.smallfont_key)) != 0))
	    ks_smallfont = sym;
    }
#endif
}

/*}}} */
/*----------------------- end-of-file (C source) -----------------------*/
