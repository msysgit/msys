/* ------------------------------------------------------------------------- */
/* --                     CONFIGURE SPECIFIED FEATURES                    -- */
/* ------------------------------------------------------------------------- */
/* Set TERMINFO value to the value given by configure */
#undef RXVT_TERMINFO

/* Set TERM to the value given by configure */
#undef TERMENV

/* Define if you don't want to use our replacement string functions */
#undef NO_STRINGS

/* Define if you don't want any resources read */
#undef NO_RESOURCES 

/* Define if you want to use XGetDefault instead of our internal version
 * which only reads ~/.Xdefaults, if it exists, otherwise ~/.Xresources if it
 * exists, and saves 60-150kB memory */
#undef USE_XGETDEFAULT

/* Define to remove old rxvt (ver 2.20 and before) style selection */
#undef NO_OLD_SELECTION

/* Define to remove xterm style selection */
#undef NO_NEW_SELECTION

/* Define if you want the depth of scrollbars and menus to be less
 * (width of 3d-look shadows and highlights)  --pjh */
#undef HALFSHADOW

#undef MULTICHAR_SET
#undef MULTICHAR_ENCODING

/* Define if you want Menubar support */
#undef MENUBAR

/* Define if you don't want support for the backspace key */
#undef NO_BACKSPACE_KEY

/* Define if you don't want support for the (non-keypad) delete key */
#undef NO_DELETE_KEY

/* Define if you want Rob Nation's own graphic mode */
#undef RXVT_GRAPHICS

/* Define if you want to use NeXT style scrollbars */
#undef NEXT_SCROLLBAR

/* Define if you want to revert to Xterm style scrollbars */
#undef XTERM_SCROLLBAR

/* Define if you want support for Greek Elot-928 & IBM-437 keyboard */
/* see doc/README.greek */
#undef GREEK_SUPPORT

/* Define if you want tty's to be setgid() to the `tty' group */
#undef TTY_GID_SUPPORT

/* Define if you want to have XIM (X Input Method) protocol support
 * This is required for multibyte characters input. */
#undef USE_XIM

/* Define if you want to have utmp/utmpx support */
#undef UTMP_SUPPORT

/* Define if you want to have wtmp support when utmp/utmpx is enabled */
#undef WTMP_SUPPORT

/* Define if you want to have lastlog support when utmp/utmpx is enabled */
#undef LASTLOG_SUPPORT

/* Define if you want to have sexy-looking background pixmaps. Needs libXpm */
#undef XPM_BACKGROUND

/* Define if you want your background use the parent window background */
#undef TRANSPARENT

/* Define if you include <X11/xpm.h> on a normal include path (be careful) */
#undef XPM_INC_X11

/* Disable the secondary screen ("\E[?47h" / "\E[?47l")
 * Many programs use the secondary screen as their workplace. The advantage
 * is, that when you exit those programs, your previous screen contents (in
 * general the shell as you left it) will be shown again. */
#undef NO_SECONDARY_SCREEN

/* Define if you want continual scrolling on when you keep the
 * scrollbar button pressed */
#undef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING

/*
 * Use wheel events (button4 and button5) to scroll.  Even if you don't
 * have a wheeled mouse, this is harmless unless you have an exotic mouse!
 */
#undef NO_MOUSE_WHEEL

/* Define if you don't want handling for rarely used features */
#undef NO_FRILLS
/* ------------------------------------------------------------------------- */
/* --                     CONFIGURE DETECTED FEATURES                     -- */
/* ------------------------------------------------------------------------- */
@TOP@
/* Define if Xlocale support doesn't work */
#undef NO_XLOCALE

/* Define if setlocale (defined to Xsetlocale) doesn't work */
#undef NO_XSETLOCALE
 
/* Define if plain old setlocale doesn't work */
#undef NO_SETLOCALE

/* Define if utmp.h has struct utmp */
#undef HAVE_STRUCT_UTMP

/* Define if struct utmp contains ut_host */
#undef HAVE_UTMP_HOST

/* Define location of utmp */
#undef RXVT_UTMP_FILE

/* Define in utmpx.h has struct utmpx */
#undef HAVE_STRUCT_UTMPX

/* Define if struct utmpx contains ut_host */
#undef HAVE_UTMPX_HOST

/* Define location of utmpx */
#undef RXVT_UTMPX_FILE

/* Define location of wtmp */
#undef RXVT_WTMP_FILE

/* Define location of wtmpx */
#undef RXVT_WTMPX_FILE

/* Define if utmp.h or lastlog.h has struct lastlog */
#undef HAVE_STRUCT_LASTLOG

/* Define if lastlog is provided via a directory */
#undef LASTLOG_IS_DIR

/* Define location of lastlog */
#undef RXVT_LASTLOG_FILE

/* Define location of ttys/ttytab */
#undef TTYTAB_FILENAME

/* Define if you need function prototypes */
#undef PROTOTYPES

/* Define if you have XPointer typedef */
#undef HAVE_XPOINTER

/* Define if you have _GNU_SOURCE getpt() */
#undef HAVE_GETPT

/* Define possible pty types */
#undef PTYS_ARE_NUMERIC
#undef PTYS_ARE_PTMX
#undef PTYS_ARE_PTC
#undef PTYS_ARE__GETPTY
#undef PTYS_ARE_GETPTY
#undef PTYS_ARE_GETPT
#undef PTYS_ARE_CLONE
#undef PTYS_ARE_SEARCHED
