/* config.h.  Generated automatically by configure.  */
/* autoconf/config.h.in.  Generated automatically from autoconf/configure.in by autoheader.  */
/* ------------------------------------------------------------------------- */
/* --                     CONFIGURE SPECIFIED FEATURES                    -- */
/* ------------------------------------------------------------------------- */
/* Set TERMINFO value to the value given by configure */
/* #undef RXVT_TERMINFO */

/* Set TERM to the value given by configure */
/* #undef TERMENV */

/* Define if you don't want to use our replacement string functions */
#define NO_STRINGS 1

/* Define if you don't want any resources read */
/* #undef NO_RESOURCES */ 

/* Define if you want to use XGetDefault instead of our internal version
 * which only reads ~/.Xdefaults, if it exists, otherwise ~/.Xresources if it
 * exists, and saves 60-150kB memory */
/* #undef USE_XGETDEFAULT */

/* Define to remove old rxvt (ver 2.20 and before) style selection */
#define NO_OLD_SELECTION 1

/* Define to remove xterm style selection */
/* #undef NO_NEW_SELECTION */

/* Define if you want the depth of scrollbars and menus to be less
 * (width of 3d-look shadows and highlights)  --pjh */
/* #undef HALFSHADOW */

/* #undef MULTICHAR_SET */
/* #undef MULTICHAR_ENCODING */

/* Define if you want Menubar support */
/* #undef MENUBAR */

/* Define if you don't want support for the backspace key */
/* #undef NO_BACKSPACE_KEY */

/* Define if you don't want support for the (non-keypad) delete key */
/* #undef NO_DELETE_KEY */

/* Define if you want Rob Nation's own graphic mode */
/* #undef RXVT_GRAPHICS */

/* Define if you want to use NeXT style scrollbars */
/* #undef NEXT_SCROLLBAR */

/* Define if you want to revert to Xterm style scrollbars */
/* #undef XTERM_SCROLLBAR */

/* Define if you want support for Greek Elot-928 & IBM-437 keyboard */
/* see doc/README.greek */
/* #undef GREEK_SUPPORT */

/* Define if you want tty's to be setgid() to the `tty' group */
/* #undef TTY_GID_SUPPORT */

/* Define if you want to have XIM (X Input Method) protocol support
 * This is required for multibyte characters input. */
/* #undef USE_XIM */

/* Define if you want to have utmp/utmpx support */
/* #undef UTMP_SUPPORT */

/* Define if you want to have wtmp support when utmp/utmpx is enabled */
/* #undef WTMP_SUPPORT */

/* Define if you want to have lastlog support when utmp/utmpx is enabled */
/* #undef LASTLOG_SUPPORT */

/* Define if you want to have sexy-looking background pixmaps. Needs libXpm */
/* #undef XPM_BACKGROUND */

/* Define if you want your background use the parent window background */
/* #undef TRANSPARENT */

/* Define if you include <X11/xpm.h> on a normal include path (be careful) */
/* #undef XPM_INC_X11 */

/* Disable the secondary screen ("\E[?47h" / "\E[?47l")
 * Many programs use the secondary screen as their workplace. The advantage
 * is, that when you exit those programs, your previous screen contents (in
 * general the shell as you left it) will be shown again. */
/* #undef NO_SECONDARY_SCREEN */

/* Define if you want continual scrolling on when you keep the
 * scrollbar button pressed */
#define NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING 1

/*
 * Use wheel events (button4 and button5) to scroll.  Even if you don't
 * have a wheeled mouse, this is harmless unless you have an exotic mouse!
 */
/* #undef NO_MOUSE_WHEEL */

/* Define if you don't want handling for rarely used features */
#define NO_FRILLS 1
/* ------------------------------------------------------------------------- */
/* --                     CONFIGURE DETECTED FEATURES                     -- */
/* ------------------------------------------------------------------------- */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef mode_t */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* Define if the X Window System is missing or not being used.  */
#define X_DISPLAY_MISSING 1

/* Define if Xlocale support doesn't work */
#define NO_XLOCALE 1

/* Define if setlocale (defined to Xsetlocale) doesn't work */
#define NO_XSETLOCALE 1

 
/* Define if plain old setlocale doesn't work */
/* #undef NO_SETLOCALE */

/* Define if utmp.h has struct utmp */
/* #undef HAVE_STRUCT_UTMP */

/* Define if struct utmp contains ut_host */
/* #undef HAVE_UTMP_HOST */

/* Define location of utmp */
/* #undef RXVT_UTMP_FILE */

/* Define in utmpx.h has struct utmpx */
/* #undef HAVE_STRUCT_UTMPX */

/* Define if struct utmpx contains ut_host */
/* #undef HAVE_UTMPX_HOST */

/* Define location of utmpx */
/* #undef RXVT_UTMPX_FILE */

/* Define location of wtmp */
/* #undef RXVT_WTMP_FILE */

/* Define location of wtmpx */
/* #undef RXVT_WTMPX_FILE */

/* Define if utmp.h or lastlog.h has struct lastlog */
#define HAVE_STRUCT_LASTLOG 1

/* Define if lastlog is provided via a directory */
/* #undef LASTLOG_IS_DIR */

/* Define location of lastlog */
/* #undef RXVT_LASTLOG_FILE */

/* Define location of ttys/ttytab */
/* #undef TTYTAB_FILENAME */

/* Define if you need function prototypes */
#define PROTOTYPES 1

/* Define if you have XPointer typedef */
/* #undef HAVE_XPOINTER */
#define HAVE_XPOINTER 1

/* Define if you have _GNU_SOURCE getpt() */
/* #undef HAVE_GETPT */

/* Define possible pty types */
/* #undef PTYS_ARE_NUMERIC */
/* #undef PTYS_ARE_PTMX */
/* #undef PTYS_ARE_PTC */
/* #undef PTYS_ARE__GETPTY */
/* #undef PTYS_ARE_GETPTY */
/* #undef PTYS_ARE_GETPT */
/* #undef PTYS_ARE_CLONE */
#define PTYS_ARE_SEARCHED 1

/* The number of bytes in a char.  */
#define SIZEOF_CHAR 1

/* The number of bytes in a int.  */
#define SIZEOF_INT 4

/* The number of bytes in a int *.  */
#define SIZEOF_INT_P 4

/* The number of bytes in a long.  */
#define SIZEOF_LONG 4

/* The number of bytes in a long long.  */
#define SIZEOF_LONG_LONG 8

/* The number of bytes in a short.  */
#define SIZEOF_SHORT 2

/* Define if you have the _getpty function.  */
/* #undef HAVE__GETPTY */

/* Define if you have the atexit function.  */
#define HAVE_ATEXIT 1

/* Define if you have the grantpt function.  */
#define HAVE_GRANTPT 1

/* Define if you have the revoke function.  */
/* #undef HAVE_REVOKE */

/* Define if you have the seteuid function.  */
#define HAVE_SETEUID 1

/* Define if you have the setpgid function.  */
#define HAVE_SETPGID 1

/* Define if you have the setpgrp function.  */
#define HAVE_SETPGRP 1

/* Define if you have the setreuid function.  */
/* #undef HAVE_SETREUID */

/* Define if you have the setsid function.  */
#define HAVE_SETSID 1

/* Define if you have the setutent function.  */
/* #undef HAVE_SETUTENT */

/* Define if you have the unlockpt function.  */
#define HAVE_UNLOCKPT 1

/* Define if you have the unsetenv function.  */
#define HAVE_UNSETENV 1

/* Define if you have the <assert.h> header file.  */
#define HAVE_ASSERT_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <grp.h> header file.  */
#define HAVE_GRP_H 1

/* Define if you have the <lastlog.h> header file.  */
#define HAVE_LASTLOG_H 1

/* Define if you have the <libc.h> header file.  */
/* #undef HAVE_LIBC_H */

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/byteorder.h> header file.  */
/* #undef HAVE_SYS_BYTEORDER_H */

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <sys/select.h> header file.  */
#define HAVE_SYS_SELECT_H 1

/* Define if you have the <sys/sockio.h> header file.  */
/* #undef HAVE_SYS_SOCKIO_H */

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <termios.h> header file.  */
#define HAVE_TERMIOS_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1
