/*--------------------------------*-C-*---------------------------------*
 * File:	misc.c
 *----------------------------------------------------------------------*
 * $Id: misc.c,v 1.1 2002/12/06 23:08:03 earnie Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (C) 1996      mj olesen <olesen@me.QueensU.CA> Queen's Univ at Kingston
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
 *----------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "misc.intpro"		/* PROTOS for internal routines */

/* EXTPROTO */
char           *
r_basename(const char *str)
{
    char           *base = STRRCHR(str, '/');

    return (char *)(base ? base + 1 : str);
}

/*
 * Print an error message
 */
/* EXTPROTO */
void
print_error(const char *fmt,...)
{
    va_list         arg_ptr;

    va_start(arg_ptr, fmt);
    fprintf(stderr, APL_NAME ": ");
    vfprintf(stderr, fmt, arg_ptr);
    fprintf(stderr, "\n");
    va_end(arg_ptr);
}

/*
 * check that the first characters of S1 match S2
 *
 * No Match
 *      return: 0
 * Match
 *      return: STRLEN (S2)
 */
/* EXTPROTO */
int
Str_match(const char *s1, const char *s2)
{
    int             n = STRLEN(s2);

    return ((STRNCMP(s1, s2, n) == 0) ? n : 0);
}

/* EXTPROTO */
const char     *
Str_skip_space(const char *str)
{
    if (str)
	while (*str && isspace(*str))
	    str++;
    return str;
}

/*
 * remove leading/trailing space and strip-off leading/trailing quotes.
 * in place.
 */
/* EXTPROTO */
char           *
Str_trim(char *str)
{
    char           *r, *s;
    int             n;

    if (!str)
	return NULL;
    if (!*str)			/* shortcut */
	return str;

/* skip leading spaces */
    for (s = str; *s && isspace(*s); s++) ;
/* goto end of string */
    for (n = 0, r = s; *r++; n++) ;
    r -= 2;
/* dump return */
    if (n > 0 && *r == '\n')
	n--, r--;
/* backtrack along trailing spaces */
    for (; n > 0 && isspace(*r); r--, n--) ;
/* skip matching leading/trailing quotes */
    if (*s == '"' && *r == '"' && n > 1) {
	s++;
	n -= 2;
    }
/* copy back over: forwards copy */
    for (r = str; n; n--)
	*r++ = *s++;
    *r = '\0';

    return str;
}

/*
 * in-place interpretation of string:
 *
 *      backslash-escaped:      "\a\b\E\e\n\r\t", "\octal"
 *      Ctrl chars:     ^@ .. ^_, ^?
 *
 *      Emacs-style:    "M-" prefix
 *
 * Also,
 *      "M-x" prefixed strings, append "\r" if needed
 *      "\E]" prefixed strings (XTerm escape sequence) append "\a" if needed
 *
 * returns the converted string length
 */
/* EXTPROTO */
int
Str_escaped(char *str)
{
    char            ch, *s, *d;
    int             i, num, append = 0;

    if (!str || !*str)
	return 0;

    d = s = str;

    if (*s == 'M' && s[1] == '-') {
	/* Emacs convenience, replace leading `M-..' with `\E..' */
	*d++ = '\033';
	s += 2;
	if (toupper(*s) == 'X')
	    /* append carriage-return for `M-xcommand' */
	    for (*d++ = 'x', append = '\r', s++; isspace(*s); s++) ;
    }
    for (; (ch = *s++);) {
	if (ch == '\\') {
	    ch = *s++;
	    if (ch >= '0' && ch <= '7') {	/* octal */
		num = ch - '0';
		for (i = 0; i < 2; i++, s++) {
		    ch = *s;
		    if (ch < '0' || ch > '7')
			break;
		    num = num * 8 + ch - '0';
		}
		ch = (char)num;
	    } else if (ch == 'a')
		ch = 007;	/* bell */
	    else if (ch == 'b')
		ch = '\b';	/* backspace */
	    else if (ch == 'E' || ch == 'e')
		ch = 033;	/* escape */
	    else if (ch == 'n')
		ch = '\n';	/* newline */
	    else if (ch == 'r')
		ch = '\r';	/* carriage-return */
	    else if (ch == 't')
		ch = '\t';	/* tab */
	} else if (ch == '^') {
	    ch = *s++;
	    ch = toupper(ch);
	    ch = (ch == '?' ? 127 : (ch - '@'));
	}
	*d++ = ch;
    }

/* ESC] is an XTerm escape sequence, must be ^G terminated */
    if (*str == '\0' && str[1] == '\033' && str[2] == ']')
	append = 007;

/* add trailing character as required */
    if (append && d[-1] != append)
	*d++ = append;
    *d = '\0';

    return (d - str);
}

/*----------------------------------------------------------------------*
 * file searching
 */

/* #define DEBUG_SEARCH_PATH */

#if defined (XPM_BACKGROUND) || (MENUBAR_MAX)
/*
 * search for FILE in the current working directory, and within the
 * colon-delimited PATHLIST, adding the file extension EXT if required.
 *
 * FILE is either semi-colon or zero terminated
 */
/* INTPROTO */
char           *
File_search_path(const char *pathlist, const char *file, const char *ext)
{
    int             maxpath, len;
    const char     *p, *path;
    char            name[256];

    if (!access(file, R_OK))	/* found (plain name) in current directory */
	return strdup(file);

/* semi-colon delimited */
    if ((p = STRCHR(file, ';')))
	len = (p - file);
    else
	len = STRLEN(file);

#ifdef DEBUG_SEARCH_PATH
    getcwd(name, sizeof(name));
    fprintf(stderr, "pwd: \"%s\"\n", name);
    fprintf(stderr, "find: \"%.*s\"\n", len, file);
#endif

/* leave room for an extra '/' and trailing '\0' */
    maxpath = sizeof(name) - (len + (ext ? STRLEN(ext) : 0) + 2);
    if (maxpath <= 0)
	return NULL;

/* check if we can find it now */
    STRNCPY(name, file, len);
    name[len] = '\0';

    if (!access(name, R_OK))
	return strdup(name);
    if (ext) {
	STRCAT(name, ext);
	if (!access(name, R_OK))
	    return strdup(name);
    }
    for (path = pathlist; path != NULL && *path != '\0'; path = p) {
	int             n;

	/* colon delimited */
	if ((p = STRCHR(path, ':')) == NULL)
	    p = STRCHR(path, '\0');

	n = (p - path);
	if (*p != '\0')
	    p++;

	if (n > 0 && n <= maxpath) {
	    STRNCPY(name, path, n);
	    if (name[n - 1] != '/')
		name[n++] = '/';
	    name[n] = '\0';
	    STRNCAT(name, file, len);

	    if (!access(name, R_OK))
		return strdup(name);
	    if (ext) {
		STRCAT(name, ext);
		if (!access(name, R_OK))
		    return strdup(name);
	    }
	}
    }
    return NULL;
}

/* EXTPROTO */
char           *
File_find(const char *file, const char *ext)
{
    char           *f;

    if (file == NULL || *file == '\0')
	return NULL;

/* search environment variables here too */
    if ((f = File_search_path(rs.path, file, ext)) == NULL)
#ifdef PATH_ENV
	if ((f = File_search_path(getenv(PATH_ENV), file, ext)) == NULL)
#endif
	    f = File_search_path(getenv("PATH"), file, ext);

#ifdef DEBUG_SEARCH_PATH
    if (f)
	fprintf(stderr, "found: \"%s\"\n", f);
#endif

    return f;
}
#endif				/* defined (XPM_BACKGROUND) || (MENUBAR_MAX) */

/*----------------------------------------------------------------------*
 * miscellaneous drawing routines
 */

/*
 * Draw top/left and bottom/right border shadows around windows
 */
#if (! defined(NEXT_SCROLLBAR) && ! defined(XTERM_SCROLLBAR)) || defined(MENUBAR)
/* EXTPROTO */
void
Draw_Shadow(Window win, GC topShadow, GC botShadow, int x, int y, int w, int h)
{
    int             x1, y1, w1, h1, shadow;

    shadow = (w == 0 || h == 0) ? 1 : SHADOW;
    w1 = w + x - 1;
    h1 = h + y - 1;
    x1 = x;
    y1 = y;
    for (; shadow-- > 0; x1++, y1++, w1--, h1--) {
	XDrawLine(Xdisplay, win, topShadow, x1, y1, w1, y1);
	XDrawLine(Xdisplay, win, topShadow, x1, y1, x1, h1);
    }

    shadow = (w == 0 || h == 0) ? 1 : SHADOW;
    w1 = w + x - 1;
    h1 = h + y - 1;
    x1 = x + 1;
    y1 = y + 1;
    for (; shadow-- > 0; x1++, y1++, w1--, h1--) {
	XDrawLine(Xdisplay, win, botShadow, w1, h1, w1, y1);
	XDrawLine(Xdisplay, win, botShadow, w1, h1, x1, h1);
    }
}
#endif

/* button shapes */
#ifdef MENUBAR
/* EXTPROTO */
void
Draw_Triangle(Window win, GC topShadow, GC botShadow, int x, int y, int w, int type)
{
    switch (type) {
    case 'r':			/* right triangle */
	XDrawLine(Xdisplay, win, topShadow, x, y, x, y + w);
	XDrawLine(Xdisplay, win, topShadow, x, y, x + w, y + w / 2);
	XDrawLine(Xdisplay, win, botShadow, x, y + w, x + w, y + w / 2);
	break;

    case 'l':			/* right triangle */
	XDrawLine(Xdisplay, win, botShadow, x + w, y + w, x + w, y);
	XDrawLine(Xdisplay, win, botShadow, x + w, y + w, x, y + w / 2);
	XDrawLine(Xdisplay, win, topShadow, x, y + w / 2, x + w, y);
	break;

    case 'd':			/* down triangle */
	XDrawLine(Xdisplay, win, topShadow, x, y, x + w / 2, y + w);
	XDrawLine(Xdisplay, win, topShadow, x, y, x + w, y);
	XDrawLine(Xdisplay, win, botShadow, x + w, y, x + w / 2, y + w);
	break;

    case 'u':			/* up triangle */
	XDrawLine(Xdisplay, win, botShadow, x + w, y + w, x + w / 2, y);
	XDrawLine(Xdisplay, win, botShadow, x + w, y + w, x, y + w);
	XDrawLine(Xdisplay, win, topShadow, x, y + w, x + w / 2, y);
	break;
#if 0
    case 's':			/* square */
	XDrawLine(Xdisplay, win, topShadow, x + w, y, x, y);
	XDrawLine(Xdisplay, win, topShadow, x, y, x, y + w);
	XDrawLine(Xdisplay, win, botShadow, x, y + w, x + w, y + w);
	XDrawLine(Xdisplay, win, botShadow, x + w, y + w, x + w, y);
	break;
#endif
    }
}
#endif
/*----------------------- end-of-file (C source) -----------------------*/
