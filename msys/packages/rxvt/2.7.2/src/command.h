/* ways to deal with getting/setting termios structure */
#ifdef HAVE_TERMIOS_H
/* termios interface */
# include <termios.h>
typedef struct termios ttymode_t;

# ifdef TCSANOW			/* POSIX */
#  define GET_TERMIOS(fd,tios)	tcgetattr (fd, tios)
#  define SET_TERMIOS(fd,tios)		\
	cfsetospeed (tios, BAUDRATE),	\
	cfsetispeed (tios, BAUDRATE),	\
	tcsetattr (fd, TCSANOW, tios)
# else
#  ifdef TIOCSETA
#   define GET_TERMIOS(fd,tios)	ioctl (fd, TIOCGETA, tios)
#   define SET_TERMIOS(fd,tios)		\
	tios->c_cflag |= BAUDRATE,	\
	ioctl (fd, TIOCSETA, tios)
#  else
#   define GET_TERMIOS(fd,tios)	ioctl (fd, TCGETS, tios)
#   define SET_TERMIOS(fd,tios)		\
	tios->c_cflag |= BAUDRATE,	\
	ioctl (fd, TCSETS, tios)
#  endif
# endif
# define SET_TTYMODE(fd,tios)		SET_TERMIOS (fd, tios)
#else
/* sgtty interface */
# include <sgtty.h>
typedef struct {
    struct sgttyb   sg;
    struct tchars   tc;
    struct ltchars  lc;
    int             line;
    int             local;
} ttymode_t;

# define SET_TTYMODE(fd,tt)				\
	tt->sg.sg_ispeed = tt->sg.sg_ospeed = BAUDRATE,	\
	ioctl (fd, TIOCSETP, &(tt->sg)),		\
	ioctl (fd, TIOCSETC, &(tt->tc)),		\
	ioctl (fd, TIOCSLTC, &(tt->lc)),		\
	ioctl (fd, TIOCSETD, &(tt->line)),		\
	ioctl (fd, TIOCLSET, &(tt->local))
#endif				/* HAVE_TERMIOS_H */

#ifdef OFFIX_DND
# include <X11/Xatom.h>
# define DndFile	2
# define DndDir		5
# define DndLink	7
#endif

#include <X11/keysym.h>
#include <X11/keysymdef.h>
#ifndef NO_XLOCALE
# if (XtSpecificationRelease < 6)
#  define NO_XLOCALE
# endif
#endif

#ifndef NO_XSETLOCALE
# define X_LOCALE
# include <X11/Xlocale.h>
#else
# ifndef NO_SETLOCALE
#  include <locale.h>
# endif
#endif				/* NO_XLOCALE */

#ifdef TTY_GID_SUPPORT
# include <grp.h>
#endif

#ifdef PTYS_ARE_PTMX
# include <sys/resource.h>	/* for struct rlimit */
# include <sys/stropts.h>	/* for I_PUSH */
# define _NEW_TTY_CTRL		/* to get proper defines in <termios.h> */
#endif

#ifdef __QNX__			/* autoconf someday */
# include <process.h>
#endif


/* use the fastest baud-rate */
#ifdef B38400
# define BAUDRATE	B38400
#else
# ifdef B19200
#  define BAUDRATE	B19200
# else
#  define BAUDRATE	B9600
# endif
#endif

/* Disable special character functions */
#ifdef _POSIX_VDISABLE
# define VDISABLE	_POSIX_VDISABLE
#else
# define VDISABLE	255
#endif

/*----------------------------------------------------------------------*
 * system default characters if defined and reasonable
 */
#ifndef CINTR
# define CINTR		'\003'	/* ^C */
#endif
#ifndef CQUIT
# define CQUIT		'\034'	/* ^\ */
#endif
#ifndef CERASE
# ifdef linux
#  define CERASE	'\177'	/* ^? */
# else
#  define CERASE	'\010'	/* ^H */
# endif
#endif
#ifndef CKILL
# define CKILL		'\025'	/* ^U */
#endif
#ifndef CEOF
# define CEOF		'\004'	/* ^D */
#endif
#ifndef CSTART
# define CSTART		'\021'	/* ^Q */
#endif
#ifndef CSTOP
# define CSTOP		'\023'	/* ^S */
#endif
#ifndef CSUSP
# define CSUSP		'\032'	/* ^Z */
#endif
#ifndef CDSUSP
# define CDSUSP		'\031'	/* ^Y */
#endif
#ifndef CRPRNT
# define CRPRNT		'\022'	/* ^R */
#endif
#ifndef CFLUSH
# define CFLUSH		'\017'	/* ^O */
#endif
#ifndef CWERASE
# define CWERASE	'\027'	/* ^W */
#endif
#ifndef CLNEXT
# define CLNEXT		'\026'	/* ^V */
#endif

#ifndef VDISCRD
# ifdef VDISCARD
#  define VDISCRD	VDISCARD
# endif
#endif

#ifndef VWERSE
# ifdef VWERASE
#  define VWERSE	VWERASE
# endif
#endif

#ifndef O_NOCTTY
# define O_NOCTTY	0
#endif
#ifndef O_NDELAY
# define O_NDELAY	O_NONBLOCK	/* QNX, at least */
#endif
#ifndef ONLCR
# define ONLCR		0		/* QNX, at least */
#endif

#define KBUFSZ		512	/* size of keyboard mapping buffer */
#define STRING_MAX	512	/* max string size for process_xterm_seq() */
#define ESC_ARGS	32	/* max # of args for esc sequences */

/* a large REFRESH_PERIOD causes problems with `cat' */
#define REFRESH_PERIOD		1

#ifndef MULTICLICK_TIME
# define MULTICLICK_TIME	500
#endif
#ifndef SCROLLBAR_INITIAL_DELAY
# ifdef NEXT_SCROLLER
#  define SCROLLBAR_INITIAL_DELAY	20
# else
#  define SCROLLBAR_INITIAL_DELAY	40
# endif
#endif
#ifndef SCROLLBAR_CONTINUOUS_DELAY
# define SCROLLBAR_CONTINUOUS_DELAY	2
#endif

/* time factor to slow down a `jumpy' mouse */
#define MOUSE_THRESHOLD		50
#define CONSOLE		"/dev/console"	/* console device */

/*
 * key-strings: if only these keys were standardized <sigh>
 */
#ifdef LINUX_KEYS
# define KS_HOME	"\033[1~"	/* Home == Find */
# define KS_END		"\033[4~"	/* End == Select */
#else
# define KS_HOME	"\033[7~"	/* Home */
# define KS_END		"\033[8~"	/* End */
#endif

#ifdef SCROLL_ON_SHIFT
# define SCROLL_SHIFTKEY (shft)
#else
# define SCROLL_SHIFTKEY 0
#endif
#ifdef SCROLL_ON_CTRL
# define SCROLL_CTRLKEY  (ctrl)
#else
# define SCROLL_CTRLKEY 0
#endif
#ifdef SCROLL_ON_META
# define SCROLL_METAKEY  (meta)
#else
# define SCROLL_METAKEY 0
#endif
#define IS_SCROLL_MOD  (SCROLL_SHIFTKEY || SCROLL_CTRLKEY || SCROLL_METAKEY)

typedef struct XCNQueue_t {
    struct XCNQueue_t *next;
    int16_t         width, height;
} XCNQueue_t;

/*
 * ESC-Z processing:
 *
 * By stealing a sequence to which other xterms respond, and sending the
 * same number of characters, but having a distinguishable sequence,
 * we can avoid having a timeout (when not under an rxvt) for every login
 * shell to auto-set its DISPLAY.
 *
 * This particular sequence is even explicitly stated as obsolete since
 * about 1985, so only very old software is likely to be confused, a
 * confusion which can likely be remedied through termcap or TERM. Frankly,
 * I doubt anyone will even notice.  We provide a #ifdef just in case they
 * don't care about auto-display setting.  Just in case the ancient
 * software in question is broken enough to be case insensitive to the 'c'
 * character in the answerback string, we make the distinguishing
 * characteristic be capitalization of that character. The length of the
 * two strings should be the same so that identical read(2) calls may be
 * used.
 */
#define VT100_ANS	"\033[?1;2c"	/* vt100 answerback */
#ifndef ESCZ_ANSWER
# define ESCZ_ANSWER	VT100_ANS	/* obsolete ANSI ESC[c */
#endif

/* DEC private modes */
#define PrivMode_132		(1LU<<0)
#define PrivMode_132OK		(1LU<<1)
#define PrivMode_rVideo		(1LU<<2)
#define PrivMode_relOrigin	(1LU<<3)
#define PrivMode_Screen		(1LU<<4)
#define PrivMode_Autowrap	(1LU<<5)
#define PrivMode_aplCUR		(1LU<<6)
#define PrivMode_aplKP		(1LU<<7)
#define PrivMode_HaveBackSpace	(1LU<<8)
#define PrivMode_BackSpace	(1LU<<9)
#define PrivMode_ShiftKeys	(1LU<<10)
#define PrivMode_VisibleCursor	(1LU<<11)
#define PrivMode_MouseX10	(1LU<<12)
#define PrivMode_MouseX11	(1LU<<13)
#define PrivMode_scrollBar	(1LU<<14)
#define PrivMode_menuBar	(1LU<<15)
#define PrivMode_TtyOutputInh	(1LU<<16)
#define PrivMode_Keypress	(1LU<<17)
/* too annoying to implement X11 highlight tracking */
/* #define PrivMode_MouseX11Track       (1LU<<18) */

#define PrivMode_mouse_report	(PrivMode_MouseX10|PrivMode_MouseX11)
#define PrivMode(test,bit)		\
    if (test)				\
	PrivateModes |= (bit);		\
    else				\
	PrivateModes &= ~(bit)

#define PrivMode_Default						 \
(PrivMode_Autowrap|PrivMode_aplKP|PrivMode_ShiftKeys|PrivMode_VisibleCursor)

/* command input buffering */
#ifndef BUFSIZ
# define BUFSIZ		4096
#endif
static unsigned char cmdbuf_base[BUFSIZ], *cmdbuf_ptr, *cmdbuf_endp;

#define IMBUFSIZ	128	/* input modifier buffer sizes */

#include "command.intpro"	/* PROTOS for internal routines */
