/*--------------------------------*-C-*---------------------------------*
 * File:	command.c
 *----------------------------------------------------------------------*
 * $Id: command.c,v 1.1 2002/12/06 23:08:02 earnie Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (C) 1992      John Bovey, University of Kent at Canterbury <jdb@ukc.ac.uk>
 *				- original version
 * Copyright (C) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 * 				- extensive modifications
 * Copyright (C) 1995      Garrett D'Amore <garrett@netcom.com>
 *				- vt100 printing
 * Copyright (C) 1995      Steven Hirsch <hirsch@emba.uvm.edu>
 *				- X11 mouse report mode and support for
 *				  DEC "private mode" save/restore functions.
 * Copyright (C) 1995      Jakub Jelinek <jj@gnu.ai.mit.edu>
 *				- key-related changes to handle Shift+function
 *				  keys properly.
 * Copyright (C) 1997      MJ Olesen <olesen@me.queensu.ca>
 *				- extensive modifications
 * Copyright (C) 1997      Raul Garcia Garcia <rgg@tid.es>
 *				- modification and cleanups for Solaris 2.x
 *				  and Linux 1.2.x
 * Copyright (C) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (C) 1998,1999 Geoff Wing <gcw@pobox.com>
 * 				- extensive modifications
 * Copyright (C) 1998      Alfredo K. Kojima <kojima@windowmaker.org>
 * Copyright (C) 1999      D J Hawkey Jr <hawkeyd@visi.com>
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
 *----------------------------------------------------------------------*/

/*{{{ includes: */
#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "version.h"
#include "command.h"

#ifdef DEBUG_CMD
# define D_CMD(x)		fprintf x ; fputc('\n', stderr)
#else
# define D_CMD(x)
#endif
#ifdef DEBUG_X
# define D_X(x)			fprintf x ; fputc('\n', stderr)
#else
# define D_X(x)
#endif


/*{{{ local variables */
static char    *ttydev = NULL;	/* pty/tty name */
static int      cmd_fd = -1;	/* pty file descriptor; connected to command */
static int      tty_fd = -1;	/* tty file descriptor; connected to child */
static pid_t    cmd_pid = -1;	/* process id of child */
static int      Xfd = -1;	/* file descriptor of X server connection */
static int      num_fds = 0;	/* number of file descriptors being used */
static short    changettyowner = 1;
static ttymode_t tio;

#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
static int      scroll_arrow_delay;
#endif
#ifdef META8_OPTION
static unsigned char meta_char;	/* Alt-key prefix */
#endif
static unsigned int ModMetaMask, ModNumLockMask;

static unsigned long PrivateModes = PrivMode_Default;
static unsigned long SavedModes = PrivMode_Default;

static int      refresh_count = 0, refresh_limit = 1, refresh_type = SLOW_REFRESH;

static Atom     wmDeleteWindow;
/* OffiX Dnd (drag 'n' drop) support */
#ifdef OFFIX_DND
static Atom     DndProtocol, DndSelection;
#endif				/* OFFIX_DND */
#ifdef TRANSPARENT
static Atom     xrootpmapid;
static short    am_transparent = 0;
static short    want_full_refresh = 0;
#endif

#ifndef NO_XLOCALE
static XIC      Input_Context;	/* input context */
#endif				/* NO_XLOCALE */

static XCNQueue_t *XCNQueue = NULL;

static char    *v_buffer;	/* pointer to physical buffer */
static char    *v_bufstr = NULL;	/* beginning of area to write */
static char    *v_bufptr;	/* end of area to write */
static char    *v_bufend;	/* end of physical buffer */

#ifdef USE_XIM
static XIMStyle input_style = 0;
static int      event_type;
#endif
/*----------------------------------------------------------------------*/

/*{{{ substitute system functions */
#if defined(__svr4__) && ! defined(_POSIX_VERSION)
/* INTPROTO */
int
getdtablesize(void)
{
    struct rlimit   rlim;

    getrlimit(RLIMIT_NOFILE, &rlim);
    return rlim.rlim_cur;
}
#endif
/*}}} */

/* ------------------------------------------------------------------------- *
 *                            PRIVILEGED OPERATIONS                          *
 * ------------------------------------------------------------------------- */
/* take care of suid/sgid super-user (root) privileges */
/* EXTPROTO */
void
privileges(int mode)
{
#if ! defined(__CYGWIN__) && ! defined(__MSYS__)
# if !defined(HAVE_SETEUID) && defined(HAVE_SETREUID)
/* setreuid() is the poor man's setuid(), seteuid() */
#  define seteuid(a)	setreuid(-1, (a))
#  define setegid(a)	setregid(-1, (a))
#  define HAVE_SETEUID
# endif
# ifdef HAVE_SETEUID
    static uid_t    euid;
    static gid_t    egid;

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
	euid = geteuid();
	egid = getegid();
	break;
    case RESTORE:
	seteuid(euid);
	setegid(egid);
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

#ifndef UTMP_SUPPORT
# define privileged_utmp(action)	(0)	/* yuck! don't like this */
#else
/* INTPROTO */
void
privileged_utmp(int action)
{
    static int      next_action = SAVE;

    D_CMD((stderr, "privileged_utmp(%c); waiting for: %c (pid: %d)", action, next_action, getpid()));
    if (next_action != action || action == IGNORE)
	return;
    if (ttydev == NULL || *ttydev == '\0')
	return;
    if (!(Options & Opt_utmpInhibit)) {
	privileges(RESTORE);
	if (action == SAVE) {
	    next_action = RESTORE;
	    makeutent(ttydev, rs.display_name);
	} else if (action == RESTORE) {
	    next_action = IGNORE;
	    cleanutent();
	}
	privileges(IGNORE);
    }
}
#endif

#if defined (__CYGWIN__) || (__MSYS__)
# define privileged_ttydev(action)	(0)	/* yuck! don't like this */
#else
/* INTPROTO */
void
privileged_ttydev(int action)
{
    unsigned int    mode;
    gid_t           gid;
    static int      next_action = SAVE;
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
    static struct stat ttyfd_stat;	/* original status of our tty */
# endif

    D_CMD((stderr, "privileged_ttydev(%c); waiting for: %c (pid: %d)", action, next_action, getpid()));
    if (next_action != action || action == IGNORE)
	return;
    if (ttydev == NULL || *ttydev == '\0')
	return;
    if (changettyowner) {
	if (action == RESTORE) {
	    next_action = IGNORE;

	    privileges(RESTORE);
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
	    (void)chmod(ttydev, ttyfd_stat.st_mode);
	    (void)chown(ttydev, ttyfd_stat.st_uid, ttyfd_stat.st_gid);
# else
	    (void)chmod(ttydev, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
				 | S_IROTH | S_IWOTH));
	    (void)chown(ttydev, 0, 0);
# endif
	    privileges(IGNORE);
	} else if (action == SAVE) {
	    next_action = RESTORE;

# ifndef RESET_TTY_TO_COMMON_DEFAULTS
/* store original tty status for restoration clean_exit() -- rgg 04/12/95 */
	    if (lstat(ttydev, &ttyfd_stat) < 0) {	/* you lose out */
		next_action = IGNORE;
		return;
	    }
# endif

	    mode = S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH;
	    gid = getgid();
# ifdef TTY_GID_SUPPORT
	    {
		struct group   *gr = getgrnam("tty");

		if (gr) {
		/* change group ownership of tty to "tty" */
		    mode = S_IRUSR | S_IWUSR | S_IWGRP;
		    gid = gr->gr_gid;
		}
	    }
# endif				/* TTY_GID_SUPPORT */
	    privileges(RESTORE);
	    (void)chown(ttydev, getuid(), gid);	/* fail silently */
	    (void)chmod(ttydev, mode);
# ifdef HAVE_REVOKE
	    (void)revoke(ttydev);
# endif
	    privileges(IGNORE);
	}
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
	D_CMD((stderr, "%s \"%s\": mode %03o, uid %d, gid %d", action == RESTORE ? "Restoring" : (action == SAVE ? "Saving" : "UNKNOWN ERROR for"), ttydev, ttyfd_stat.st_mode, ttyfd_stat.st_uid, ttyfd_stat.st_gid));
# endif
    }
}
#endif

/* ------------------------------------------------------------------------- *
 *                       SIGNAL HANDLING & EXIT HANDLER                      *
 * ------------------------------------------------------------------------- */
/*
 * Catch a SIGCHLD signal and exit if the direct child has died
 */
/* ARGSUSED */
/* INTPROTO */
RETSIGTYPE
Child_signal(int unused)
{
    int             pid, save_errno = errno;

    do {
	errno = 0;
    } while ((pid = waitpid(-1, NULL, WNOHANG)) == -1 && errno == EINTR);

    if (pid == cmd_pid)
	exit(EXIT_SUCCESS);

    errno = save_errno;
    signal(SIGCHLD, Child_signal);
}

/*
 * Catch a fatal signal and tidy up before quitting
 */
/* INTPROTO */
RETSIGTYPE
Exit_signal(int sig)
{
#ifdef DEBUG_CMD
    print_error("signal %d", sig);
#endif
    signal(sig, SIG_DFL);
    kill(getpid(), sig);
}

/*
 * Exit gracefully, clearing the utmp entry and restoring tty attributes
 * TODO: if debugging, this should free up any known resources if we can
 */
/* INTPROTO */
void
clean_exit(void)
{
#ifdef DEBUG_SCREEN
    scr_release();
#endif
    privileged_ttydev(RESTORE);
    privileged_utmp(RESTORE);
#ifdef USE_XIM
    if (Input_Context != NULL)
        XDestroyIC(Input_Context);
#endif
}

/* ------------------------------------------------------------------------- *
 *                  GET PSEUDO TELETYPE - MASTER AND SLAVE                   *
 * ------------------------------------------------------------------------- */
/*
 * On failure, returns -1.
 * If successful, file descriptors cmd_fd and tty_fd point to the master
 * and slave parts respectively; and ttydev is the name of the slave.
 */
/* INTPROTO */
int
get_ptytty(void)
{
    int             pfd;
    char           *ptydev;

#ifdef PTYS_ARE__GETPTY
    ptydev = ttydev = _getpty(&pfd, O_RDWR | O_NDELAY | O_NOCTTY, 0622, 0);
    if (ptydev != NULL)
	goto Found_pty;
#endif
#ifdef PTYS_ARE_GETPTY
    while ((ptydev = getpty()) != NULL)
	if ((pfd = open(ptydev, O_RDWR | O_NOCTTY, 0)) >= 0) {
	    ttydev = ptydev;
	    goto Found_pty;
	}
#endif
#if defined(HAVE_GRANTPT) && defined(HAVE_UNLOCKPT)
# if defined(PTYS_ARE_GETPT) || defined(PTYS_ARE_PTMX) || defined(__CYGWIN__) || defined(__MSYS__)
    {
	extern char    *ptsname();

#  ifdef PTYS_ARE_GETPT
	if ((pfd = getpt()) >= 0) {
#  else
	if ((pfd = open("/dev/ptmx", O_RDWR | O_NOCTTY, 0)) >= 0) {
#  endif
	    if (grantpt(pfd) == 0	/* change slave permissions */
		&& unlockpt(pfd) == 0) {	/* slave now unlocked */
		ptydev = ttydev = ptsname(pfd);	/* get slave's name */
		changettyowner = 0;
		goto Found_pty;
	    }
	    close(pfd);
	}
    }
# endif
#endif
#ifdef PTYS_ARE_PTC
    if ((pfd = open("/dev/ptc", O_RDWR | O_NOCTTY, 0)) >= 0) {
	ptydev = ttydev = ttyname(pfd);
	goto Found_pty;
    }
#endif
#ifdef PTYS_ARE_CLONE
    if ((pfd = open("/dev/ptym/clone", O_RDWR | O_NOCTTY, 0)) >= 0) {
	ptydev = ttydev = ptsname(pfd);
	goto Found_pty;
    }
#endif
#ifdef PTYS_ARE_NUMERIC
    {
	int             idx;
	char           *c1, *c2;
	char            pty_name[] = "/dev/ptyp???";
	char            tty_name[] = "/dev/ttyp???";

	ptydev = pty_name;
	ttydev = tty_name;

	c1 = &(pty_name[sizeof(pty_name) - 4]);
	c2 = &(tty_name[sizeof(tty_name) - 4]);
	for (idx = 0; idx < 256; idx++) {
	    sprintf(c1, "%d", idx);
	    sprintf(c2, "%d", idx);
	    if (access(ttydev, F_OK) < 0) {
		idx = 256;
		break;
	    }
	    if ((pfd = open(ptydev, O_RDWR | O_NOCTTY, 0)) >= 0) {
		if (access(ttydev, R_OK | W_OK) == 0) {
		    ttydev = strdup(tty_name);
		    goto Found_pty;
		}
		close(pfd);
	    }
	}
    }
#endif
#ifdef PTYS_ARE_SEARCHED
    {
	int             len;
	const char     *c1, *c2;
	char            pty_name[] = "/dev/pty??";
	char            tty_name[] = "/dev/tty??";

	len = sizeof(pty_name) - 3;
	ptydev = pty_name;
	ttydev = tty_name;

# define PTYCHAR1	"pqrstuvwxyz"
# define PTYCHAR2	"0123456789abcdef"
	for (c1 = PTYCHAR1; *c1; c1++) {
	    ptydev[len] = ttydev[len] = *c1;
	    for (c2 = PTYCHAR2; *c2; c2++) {
		ptydev[len + 1] = ttydev[len + 1] = *c2;
		if ((pfd = open(ptydev, O_RDWR | O_NOCTTY, 0)) >= 0) {
		    if (access(ttydev, R_OK | W_OK) == 0) {
			ttydev = strdup(tty_name);
			goto Found_pty;
		    }
		    close(pfd);
		}
	    }
	}
    }
#endif

    print_error("can't open pseudo-tty");
    return -1;

  Found_pty:
    cmd_fd = pfd;
    fcntl(cmd_fd, F_SETFL, O_NDELAY);
    privileged_ttydev(SAVE);
    if ((tty_fd = open(ttydev, O_RDWR | O_NOCTTY, 0)) < 0) {
	print_error("can't open slave tty %s", ttydev);
	close(pfd);
	return -1;
    }
#if defined(PTYS_ARE_PTMX) && defined(I_PUSH)
/*
 * Push STREAMS modules:
 *    ptem: pseudo-terminal hardware emulation module.
 *    ldterm: standard terminal line discipline.
 *    ttcompat: V7, 4BSD and XENIX STREAMS compatibility module.
 */
    if (!changettyowner) {
	D_CMD((stderr, "get_ptytty(): STREAMS pushing"));
	ioctl(tty_fd, I_PUSH, "ptem");
	ioctl(tty_fd, I_PUSH, "ldterm");
	ioctl(tty_fd, I_PUSH, "ttcompat");
    }
#endif

    get_ttymode(&tio);

    return 0;
}
/*}}} */

/* ------------------------------------------------------------------------- *
 *                          CHILD PROCESS OPERATIONS                         *
 * ------------------------------------------------------------------------- */
/*
 * The only open file descriptor is the slave tty - so no error messages.
 * returns are fatal
 */
/* INTPROTO */
int
run_child(const char *const *argv)
{
    int             fd;
    char           *login;

#ifndef __QNX__
/* ---------------------------------------- */
# ifdef HAVE_SETSID
    setsid();
# endif
# if defined(HAVE_SETPGID)
    setpgid(0, 0);
# elif defined(HAVE_SETPGRP)
    setpgrp(0, 0);
# endif
/* ---------------------------------------- */
# ifdef TIOCNOTTY
    fd = open("/dev/tty", O_RDWR | O_NOCTTY);
    D_CMD((stderr, "run_child(): Voiding tty associations: previous=%s", fd < 0 ? "no" : "yes"));
    if (fd >= 0) {
	ioctl(fd, TIOCNOTTY, 0);	/* void tty associations */
	close(fd);
    }
# endif
/* ---------------------------------------- */
    fd = open("/dev/tty", O_RDWR | O_NOCTTY);
    D_CMD((stderr, "run_child(): /dev/tty has controlling tty? %s", fd < 0 ? "no (good)" : "yes (bad)"));
    if (fd >= 0)
	close(fd);		/* ouch: still have controlling tty */
/* ---------------------------------------- */
# if defined(TIOCSCTTY)
    if (ioctl(tty_fd, TIOCSCTTY, 0) < 0 && errno != EINVAL)
# elif defined(TIOCSETCTTY)
    if (ioctl(tty_fd, TIOCSETCTTY, 0) < 0)
# endif
/* ---------------------------------------- */
/*
 * If we can't force the only open file descriptor to be the controlling
 * terminal, close it and open a new one: _some_ systems make it automatically
 * the controlling terminal
 */
    {
	close(tty_fd);
	tty_fd = open(ttydev, O_RDWR, 0);
	D_CMD((stderr, "run_child(): couldn't set controlling terminal, trying again: %s", tty_fd < 0 ? "no (bad)" : "yes (good)"));
    }
/* ---------------------------------------- */
    fd = open("/dev/tty", O_WRONLY);
    D_CMD((stderr, "run_child(): do we have controlling tty now: %s", fd < 0 ? "no (fatal)" : "yes (good)"));
    if (fd < 0)
	return -1;		/* fatal */
    close(fd);
/* ---------------------------------------- */
# if 0
    close(tty_fd);
    tty_fd = open(ttydev, O_RDWR, 0);
    D_CMD((stderr, "run_child(): reopening tty: %s", tty_fd < 0 ? "no (fatal)" : "yes (good)"));
    if (tty_fd < 0)
	return -1;
# endif
/* ---------------------------------------- */
/* Reopen stdin, stdout and stderr over the tty file descriptor */
    dup2(tty_fd, STDIN_FILENO);
    dup2(tty_fd, STDOUT_FILENO);
    dup2(tty_fd, STDERR_FILENO);
    if (tty_fd > 2)
	close(tty_fd);
#endif				/* ! __QNX__ */

    SET_TTYMODE(STDIN_FILENO, &tio);	/* initialize terminal attributes */

    if (Options & Opt_console) {	/* be virtual console, fail silently */
#ifdef TIOCCONS
	unsigned int    on = 1;

	ioctl(STDIN_FILENO, TIOCCONS, &on);
#elif defined (SRIOCSREDIR)
	fd = open(CONSOLE, O_WRONLY, 0);
	if (fd < 0 || ioctl(fd, SRIOCSREDIR, 0) < 0) {
	    if (fd >= 0)
		close(fd);
	}
#endif				/* SRIOCSREDIR */
    }

    tt_winsize(STDIN_FILENO);	/* set window size */

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

	argv0 = (const char *)r_basename(shell);
	if (Options & Opt_loginShell) {
	    login = MALLOC((STRLEN(argv0) + 2) * sizeof(char));

	    login[0] = '-';
	    STRCPY(&login[1], argv0);
	    argv0 = login;
	}
	execlp(shell, argv0, NULL);
	/* no error message: STDERR is closed! */
    }
#else				/* __QNX__ uses qnxspawn() */
    {
	char            iov_a[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	char           *command = NULL, fullcommand[_MAX_PATH];
	char          **arg_v, *arg_a[2] = {NULL, NULL};

	if (argv != NULL) {
	    if (access(argv[0], X_OK) == -1) {
		if (strchr(argv[0], '/') == NULL) {
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
	    if (Options & Opt_loginShell) {
		login = MALLOC((strlen(arg_a[0]) + 2) * sizeof(char));
		login[0] = '-';
		STRCPY(&login[1], arg_a[0]);
		arg_a[0] = login;
	    }
	    arg_v = arg_a;
	}
	iov_a[0] = iov_a[1] = iov_a[2] = tty_fd;
	cmd_pid = qnx_spawn(0, 0, 0, -1, -1, _SPAWN_SETSID | _SPAWN_TCSETPGRP,
			    command, arg_v, environ, iov_a, 0);
	if (login)
	    FREE(login);
	close(tty_fd);
	return cmd_pid;
    }
#endif
    return -1;
}

/*}}} */

/*{{{ get_ttymode() */
/* INTPROTO */
void
get_ttymode(ttymode_t * tio)
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
    tio->c_iflag = (BRKINT | IGNPAR | ICRNL | IXON
# ifdef IMAXBEL
		    | IMAXBEL
# endif
	);

/* output modes */
    tio->c_oflag = (OPOST | ONLCR);

/* control modes */
    tio->c_cflag = (CS8 | CREAD);

/* line discipline modes */
    tio->c_lflag = (ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK
# if defined (ECHOCTL) && defined (ECHOKE)
		    | ECHOCTL | ECHOKE
# endif
	);
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
	tio->tc.t_eofc = CEOF;		/* ^D */
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
/*}}} */

/*{{{ run_command() */
/*
 * Run the command in a subprocess and return a file descriptor for the
 * master end of the pseudo-teletype pair with the command talking to
 * the slave.
 */
/* INTPROTO */
void
run_command(const char *const *argv)
{
    int             i;

    if (get_ptytty() < 0)
	return;

/* install exit handler for cleanup */
#ifdef HAVE_ATEXIT
    atexit(clean_exit);
#else
# if defined (__sun__)
    on_exit(clean_exit, NULL);	/* non-ANSI exit handler */
# endif
#endif

/*
 * Close all unused file descriptors.
 * We don't want them, we don't need them.
 */
    for (i = 0; i < num_fds; i++) {
	if (i == STDERR_FILENO || i == cmd_fd || i == tty_fd || i == Xfd)
	    continue;
	close(i);
    }

    signal(SIGHUP, Exit_signal);
#ifndef __svr4__
    signal(SIGINT, Exit_signal);
#endif
    signal(SIGQUIT, Exit_signal);
    signal(SIGTERM, Exit_signal);
    signal(SIGCHLD, Child_signal);

/* need to trap SIGURG for SVR4 (Unixware) rlogin */
/* signal (SIGURG, SIG_DFL); */

#ifndef __QNX__
/* spin off the command interpreter */
    switch (cmd_pid = fork()) {
    case -1:
	print_error("can't fork");
	cmd_fd = -1;
	return;
    case 0:
	close(cmd_fd);		/* only keep tty_fd open */
	close(Xfd);
#ifndef DEBUG_CMD
	close(STDERR_FILENO);
#endif
	run_child(argv);
	exit(EXIT_FAILURE);
    /* NOTREACHED */
    default:
	close(tty_fd);		/* keep STDERR_FILENO, cmd_fd, Xfd open */
	break;
    }
#else				/* __QNX__ uses qnxspawn() */
    fchmod(tty_fd, 0622);
    fcntl(tty_fd, F_SETFD, FD_CLOEXEC);
    fcntl(cmd_fd, F_SETFD, FD_CLOEXEC);

    if (run_child(argv) == -1)
	exit(EXIT_FAILURE);
#endif
/*
 * Reduce num_fds to what we use, so select() is more efficient
 */
    num_fds = max(STDERR_FILENO, cmd_fd);
    MAX_IT(num_fds, Xfd);
    num_fds++;			/* counts from 0 */

    privileged_utmp(SAVE);
}
/*}}} */

/*
 * Probe the modifier keymap to get the Meta (Alt) and Num_Lock settings
 * Use resource ``modifier'' to override the modifier
 */
/* INTPROTO */
void
get_ourmods(void)
{
    int             i, j, k, m;
    int             got_meta, got_numlock;
    XModifierKeymap *map;
    KeyCode        *kc;
    unsigned int    modmasks[] =
			{ Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask };

    got_meta = got_numlock = m = 0;
    if (rs.modifier
	&& rs.modifier[0] == 'm'
	&& rs.modifier[1] == 'o'
	&& rs.modifier[2] == 'd'
	&& rs.modifier[3] >= '1' && rs.modifier[3] <= '5'
	&& !rs.modifier[4]) {
	ModMetaMask = modmasks[(rs.modifier[3] - '1')];
	got_meta = 1;
    }
    map = XGetModifierMapping(Xdisplay);
    kc = map->modifiermap;
    for (i = 3; i < 8; i++) {
	k = i * map->max_keypermod;
	for (j = 0; j < map->max_keypermod; j++, k++) {
	    if (kc[k] == 0)
		break;
	    switch (XKeycodeToKeysym(Xdisplay, kc[k], 0)) {
	    case XK_Num_Lock:
		if (!got_numlock) {
		    ModNumLockMask = modmasks[i - 3];
		    got_numlock = 1;
		}
		break;
	    case XK_Meta_L:
	    case XK_Meta_R:
		if (rs.modifier
		    && !STRNCASECMP(rs.modifier, "meta", 4)) {
		    ModMetaMask = modmasks[i - 3];
		    got_meta = 1;
		    break;
		}
	    /* FALLTHROUGH */
	    case XK_Alt_L:
	    case XK_Alt_R:
		if (rs.modifier
		    && !STRNCASECMP(rs.modifier, "alt", 3)) {
		    ModMetaMask = modmasks[i - 3];
		    got_meta = 1;
		    break;
		}
		m = modmasks[i - 3];
		break;
	    case XK_Super_L:
	    case XK_Super_R:
		if (rs.modifier
		    && !STRNCASECMP(rs.modifier, "super", 5)) {
		    ModMetaMask = modmasks[i - 3];
		    got_meta = 1;
		}
		break;
	    case XK_Hyper_L:
	    case XK_Hyper_R:
		if (rs.modifier
		    && !STRNCASECMP(rs.modifier, "hyper", 5)) {
		    ModMetaMask = modmasks[i - 3];
		    got_meta = 1;
		}
	    /* FALLTHROUGH */
	    default:
		break;
	    }
	}
	if (got_meta && got_numlock)
	    break;
    }
    XFreeModifiermap(map);
    if (!got_meta && m)
	ModMetaMask = m;
}

/*{{{ init_command() */
/* EXTPROTO */
void
init_command(const char *const *argv)
{
/*
 * Initialize the command connection.
 * This should be called after the X server connection is established.
 */

/* Enable delete window protocol */
    wmDeleteWindow = XInternAtom(Xdisplay, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(Xdisplay, TermWin.parent[0], &wmDeleteWindow, 1);

#ifdef OFFIX_DND
/* Enable OffiX Dnd (drag 'n' drop) protocol */
    DndProtocol = XInternAtom(Xdisplay, "DndProtocol", False);
    DndSelection = XInternAtom(Xdisplay, "DndSelection", False);
#endif				/* OFFIX_DND */
#ifdef TRANSPARENT
    xrootpmapid = XInternAtom(Xdisplay, "_XROOTPMAP_ID", False);
#endif

/* get number of available file descriptors */
#if defined(_POSIX_VERSION) || ! defined(__svr4__)
    num_fds = (int)sysconf(_SC_OPEN_MAX);
#else
    num_fds = getdtablesize();
#endif

#ifdef META8_OPTION
    meta_char = (Options & Opt_meta8 ? 0x80 : 033);
#endif
    get_ourmods();
    if (!(Options & Opt_scrollTtyOutput))
	PrivateModes |= PrivMode_TtyOutputInh;
    if (Options & Opt_scrollKeypress)
	PrivateModes |= PrivMode_Keypress;
#ifndef NO_BACKSPACE_KEY
    if (STRCMP(key_backspace, "DEC") == 0)
	PrivateModes |= PrivMode_HaveBackSpace;
#endif
/* add value for scrollBar */
    if (scrollbar_visible()) {
	PrivateModes |= PrivMode_scrollBar;
	SavedModes |= PrivMode_scrollBar;
    }
    if (menubar_visible()) {
	PrivateModes |= PrivMode_menuBar;
	SavedModes |= PrivMode_menuBar;
    }
    greek_init();

    Xfd = XConnectionNumber(Xdisplay);
    cmdbuf_ptr = cmdbuf_endp = cmdbuf_base;

    run_command(argv);
    if (cmd_fd < 0) {
	print_error("aborting");
	exit(EXIT_FAILURE);
    }
}
/*}}} */

/*{{{ Xlocale */
/*
 * This is more or less stolen straight from XFree86 xterm.
 * This should support all European type languages.
 */
/* EXTPROTO */
void
init_xlocale(void)
{
    char           *locale = NULL;

#if !defined(NO_XSETLOCALE) || !defined(NO_SETLOCALE)
    locale = setlocale(LC_CTYPE, "");
#endif
#ifdef USE_XIM
    if (locale == NULL)
	print_error("Setting locale failed.");
    else {
    /* To avoid Segmentation Fault in C locale */
	setTermFontSet();
# ifdef MULTICHAR_SET
	if (STRCMP(locale, "C"))
# endif
	    XRegisterIMInstantiateCallback(Xdisplay, NULL, NULL, NULL,
					   IMInstantiateCallback, NULL);
    }
#endif
}
/*}}} */

/*{{{ window resizing */
/*
 * Tell the teletype handler what size the window is.
 * Called after a window size change.
 */
/* INTPROTO */
void
tt_winsize(int fd)
{
    struct winsize  ws;

    if (fd < 0)
	return;

    ws.ws_col = (unsigned short)TermWin.ncol;
    ws.ws_row = (unsigned short)TermWin.nrow;
    ws.ws_xpixel = ws.ws_ypixel = 0;
    ioctl(fd, TIOCSWINSZ, &ws);
}

/* EXTPROTO */
void
tt_resize(void)
{
    tt_winsize(cmd_fd);
}

/*}}} */

/*{{{ Convert the keypress event into a string */
/* INTPROTO */
void
lookup_key(XEvent * ev)
{
    int             ctrl, meta, shft, len;
    KeySym          keysym;
    static XComposeStatus compose = {NULL, 0};
    static unsigned char kbuf[KBUFSZ];
    static int      numlock_state = 0;
#ifdef DEBUG_CMD
    static int      debug_key = 1;	/* accessible by a debugger only */
#endif
#ifdef GREEK_SUPPORT
    static short    greek_mode = 0;
#endif
#ifdef USE_XIM
    int             valid_keysym;
#endif

/*
 * use Num_Lock to toggle Keypad on/off.  If Num_Lock is off, allow an
 * escape sequence to toggle the Keypad.
 *
 * Always permit `shift' to override the current setting
 */
    shft = (ev->xkey.state & ShiftMask);
    ctrl = (ev->xkey.state & ControlMask);
    meta = (ev->xkey.state & ModMetaMask);
    if (numlock_state || (ev->xkey.state & ModNumLockMask)) {
	numlock_state = (ev->xkey.state & ModNumLockMask);
	PrivMode((!numlock_state), PrivMode_aplKP);
    }
#ifdef USE_XIM
    len = 0;
    if (Input_Context != NULL) {
	Status          status_return;

	kbuf[0] = '\0';
	len = XmbLookupString(Input_Context, &ev->xkey, (char *)kbuf,
			      sizeof(kbuf), &keysym, &status_return);
	valid_keysym = ((status_return == XLookupKeySym)
			|| (status_return == XLookupBoth));
    } else {
	len = XLookupString(&ev->xkey, (char *)kbuf, sizeof(kbuf), &keysym,
			    &compose);
	valid_keysym = 1;
    }
#else				/* USE_XIM */
    len = XLookupString(&ev->xkey, (char *)kbuf, sizeof(kbuf), &keysym,
			&compose);
/*
 * map unmapped Latin[2-4]/Katakana/Arabic/Cyrillic/Greek entries -> Latin1
 * good for installations with correct fonts, but without XLOCALE
 */
    if (!len && (keysym >= 0x0100) && (keysym < 0x0800)) {
	len = 1;
	kbuf[0] = (keysym & 0xFF);
    }
#endif				/* USE_XIM */

    if (len && (Options & Opt_scrollKeypress))
	TermWin.view_start = 0;

#ifdef USE_XIM
    if (valid_keysym) {
#endif

/* for some backwards compatibility */
#if defined(HOTKEY_CTRL) || defined(HOTKEY_META)
# ifdef HOTKEY_CTRL
	if (ctrl) {
# else
	if (meta) {
# endif
	    if (keysym == ks_bigfont) {
		change_font(0, FONT_UP);
		return;
	    } else if (keysym == ks_smallfont) {
		change_font(0, FONT_DN);
		return;
	    }
	}
#endif

	if (TermWin.saveLines) {
#ifdef UNSHIFTED_SCROLLKEYS
	    if (!ctrl && !meta) {
#else
	    if (IS_SCROLL_MOD) {
#endif
		int             lnsppg;

#ifdef PAGING_CONTEXT_LINES
		lnsppg = TermWin.nrow - PAGING_CONTEXT_LINES;
#else
		lnsppg = TermWin.nrow * 4 / 5;
#endif
		if (keysym == XK_Prior) {
		    scr_page(UP, lnsppg);
		    return;
		} else if (keysym == XK_Next) {
		    scr_page(DN, lnsppg);
		    return;
		}
	    }
#ifdef SCROLL_ON_UPDOWN_KEYS
	    if (IS_SCROLL_MOD) {
		if (keysym == XK_Up) {
		    scr_page(UP, 1);
		    return;
		} else if (keysym == XK_Down) {
		    scr_page(DN, 1);
		    return;
		}
	    }
#endif
	}

	if (shft) {
	/* Shift + F1 - F10 generates F11 - F20 */
	    if (keysym >= XK_F1 && keysym <= XK_F10) {
		keysym += (XK_F11 - XK_F1);
		shft = 0;	/* turn off Shift */
	    } else if (!ctrl && !meta && (PrivateModes & PrivMode_ShiftKeys)) {
		switch (keysym) {
		/* normal XTerm key bindings */
		case XK_Insert:	/* Shift+Insert = paste mouse selection */
		    selection_request(ev->xkey.time, 0, 0);
		    return;
		/* rxvt extras */
		case XK_KP_Add:	/* Shift+KP_Add = bigger font */
		    change_font(0, FONT_UP);
		    return;
		case XK_KP_Subtract:	/* Shift+KP_Subtract = smaller font */
		    change_font(0, FONT_DN);
		    return;
		}
	    }
	}
#ifdef PRINTPIPE
	if (keysym == XK_Print) {
	    scr_printscreen(ctrl | shft);
	    return;
	}
#endif
#ifdef GREEK_SUPPORT
	if (keysym == XK_Mode_switch) {
	    greek_mode = !greek_mode;
	    if (greek_mode) {
		xterm_seq(XTerm_title,
			  (greek_getmode() == GREEK_ELOT928 ? "[Greek: iso]"
			   : "[Greek: ibm]"));
		greek_reset();
	    } else
		xterm_seq(XTerm_title, APL_NAME "-" VERSION);
	    return;
	}
#endif

	if (keysym >= 0xFF00 && keysym <= 0xFFFF) {
#ifdef KEYSYM_RESOURCE
	    if (!(shft | ctrl) && KeySym_map[keysym - 0xFF00] != NULL) {
		unsigned int    len;
		const unsigned char *kbuf;
		const unsigned char ch = '\033';

		kbuf = (KeySym_map[keysym - 0xFF00]);
		len = *kbuf++;

	    /* escape prefix */
		if (meta)
# ifdef META8_OPTION
		    if (meta_char == 033)
# endif
			tt_write(&ch, 1);
		tt_write(kbuf, len);
		return;
	    } else
#endif
		switch (keysym) {
#ifndef NO_BACKSPACE_KEY
		case XK_BackSpace:
		    if (PrivateModes & PrivMode_HaveBackSpace) {
			len = 1;
			kbuf[0] = ((!!(PrivateModes & PrivMode_BackSpace)
				    ^ !!(shft | ctrl)) ? '\b' : '\177');
		    } else
			len = STRLEN(STRCPY(kbuf, key_backspace));
		    break;
#endif
#ifndef NO_DELETE_KEY
		case XK_Delete:
		    len = STRLEN(STRCPY(kbuf, key_delete));
		    break;
#endif
		case XK_Tab:
		    if (shft) {
			len = 3;
			STRCPY(kbuf, "\033[Z");
		    }
		    break;

#ifdef XK_KP_Home
		case XK_KP_Home:
		/* allow shift to override */
		    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			len = 3;
			STRCPY(kbuf, "\033Ow");
			break;
		    }
		/* FALLTHROUGH */
#endif
		case XK_Home:
		    len = STRLEN(STRCPY(kbuf, KS_HOME));
		    break;

#ifdef XK_KP_Left
		case XK_KP_Up:		/* \033Ox or standard */
		case XK_KP_Down:	/* \033Ow or standard */
		case XK_KP_Right:	/* \033Ov or standard */
		case XK_KP_Left:	/* \033Ot or standard */
		    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			len = 3;
			STRCPY(kbuf, "\033OZ");
			kbuf[2] = ("txvw"[keysym - XK_KP_Left]);
			break;
		    } else
		    /* translate to std. cursor key */
			keysym = XK_Left + (keysym - XK_KP_Left);
		/* FALLTHROUGH */
#endif
		case XK_Up:	/* "\033[A" */
		case XK_Down:	/* "\033[B" */
		case XK_Right:	/* "\033[C" */
		case XK_Left:	/* "\033[D" */
		    len = 3;
		    STRCPY(kbuf, "\033[@");
		    kbuf[2] = ("DACB"[keysym - XK_Left]);
		/* do Shift first */
		    if (shft)
			kbuf[2] = ("dacb"[keysym - XK_Left]);
		    else if (ctrl) {
			kbuf[1] = 'O';
			kbuf[2] = ("dacb"[keysym - XK_Left]);
		    } else if (PrivateModes & PrivMode_aplCUR)
			kbuf[1] = 'O';
		    break;

#ifndef UNSHIFTED_SCROLLKEYS
# ifdef XK_KP_Prior
		case XK_KP_Prior:
		/* allow shift to override */
		    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			len = 3;
			STRCPY(kbuf, "\033Oy");
			break;
		    }
		/* FALLTHROUGH */
# endif
		case XK_Prior:
		    len = 4;
		    STRCPY(kbuf, "\033[5~");
		    break;
# ifdef XK_KP_Next
		case XK_KP_Next:
		/* allow shift to override */
		    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			len = 3;
			STRCPY(kbuf, "\033Os");
			break;
		    }
		/* FALLTHROUGH */
# endif
		case XK_Next:
		    len = 4;
		    STRCPY(kbuf, "\033[6~");
		    break;
#endif
#ifdef XK_KP_End
		case XK_KP_End:
		/* allow shift to override */
		    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			len = 3;
			STRCPY(kbuf, "\033Oq");
			break;
		    }
		/* FALLTHROUGH */
#endif
		case XK_End:
		    len = STRLEN(STRCPY(kbuf, KS_END));
		    break;

		case XK_Select:
		    len = 4;
		    STRCPY(kbuf, "\033[4~");
		    break;
#ifdef DXK_Remove		/* support for DEC remove like key */
		case DXK_Remove:
		/* FALLTHROUGH */
#endif
		case XK_Execute:
		    len = 4;
		    STRCPY(kbuf, "\033[3~");
		    break;
		case XK_Insert:
		    len = 4;
		    STRCPY(kbuf, "\033[2~");
		    break;

		case XK_Menu:
		    len = 5;
		    STRCPY(kbuf, "\033[29~");
		    break;
		case XK_Find:
		    len = 4;
		    STRCPY(kbuf, "\033[1~");
		    break;
		case XK_Help:
		    len = 5;
		    STRCPY(kbuf, "\033[28~");
		    break;

		case XK_KP_Enter:
		/* allow shift to override */
		    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			len = 3;
			STRCPY(kbuf, "\033OM");
		    } else {
			len = 1;
			kbuf[0] = '\r';
		    }
		    break;

#ifdef XK_KP_Begin
		case XK_KP_Begin:
		    len = 3;
		    STRCPY(kbuf, "\033Ou");
		    break;

		case XK_KP_Insert:
		    len = 3;
		    STRCPY(kbuf, "\033Op");
		    break;

		case XK_KP_Delete:
		    len = 3;
		    STRCPY(kbuf, "\033On");
		    break;
#endif

		case XK_KP_F1:	/* "\033OP" */
		case XK_KP_F2:	/* "\033OQ" */
		case XK_KP_F3:	/* "\033OR" */
		case XK_KP_F4:	/* "\033OS" */
		    len = 3;
		    STRCPY(kbuf, "\033OP");
		    kbuf[2] += (keysym - XK_KP_F1);
		    break;

		case XK_KP_Multiply:	/* "\033Oj" : "*" */
		case XK_KP_Add:		/* "\033Ok" : "+" */
		case XK_KP_Separator:	/* "\033Ol" : "," */
		case XK_KP_Subtract:	/* "\033Om" : "-" */
		case XK_KP_Decimal:	/* "\033On" : "." */
		case XK_KP_Divide:	/* "\033Oo" : "/" */
		case XK_KP_0:		/* "\033Op" : "0" */
		case XK_KP_1:		/* "\033Oq" : "1" */
		case XK_KP_2:		/* "\033Or" : "2" */
		case XK_KP_3:		/* "\033Os" : "3" */
		case XK_KP_4:		/* "\033Ot" : "4" */
		case XK_KP_5:		/* "\033Ou" : "5" */
		case XK_KP_6:		/* "\033Ov" : "6" */
		case XK_KP_7:		/* "\033Ow" : "7" */
		case XK_KP_8:		/* "\033Ox" : "8" */
		case XK_KP_9:		/* "\033Oy" : "9" */
		/* allow shift to override */
		    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			len = 3;
			STRCPY(kbuf, "\033Oj");
			kbuf[2] += (keysym - XK_KP_Multiply);
		    } else {
			len = 1;
			kbuf[0] = ('*' + (keysym - XK_KP_Multiply));
		    }
		    break;

#define FKEY(n, fkey)							\
    len = 5;								\
    sprintf((char *) kbuf,"\033[%02d~", (int)((n) + (keysym - fkey)))

		case XK_F1:	/* "\033[11~" */
		case XK_F2:	/* "\033[12~" */
		case XK_F3:	/* "\033[13~" */
		case XK_F4:	/* "\033[14~" */
		case XK_F5:	/* "\033[15~" */
		    FKEY(11, XK_F1);
		    break;

		case XK_F6:	/* "\033[17~" */
		case XK_F7:	/* "\033[18~" */
		case XK_F8:	/* "\033[19~" */
		case XK_F9:	/* "\033[20~" */
		case XK_F10:	/* "\033[21~" */
		    FKEY(17, XK_F6);
		    break;

		case XK_F11:	/* "\033[23~" */
		case XK_F12:	/* "\033[24~" */
		case XK_F13:	/* "\033[25~" */
		case XK_F14:	/* "\033[26~" */
		    FKEY(23, XK_F11);
		    break;

		case XK_F15:	/* "\033[28~" */
		case XK_F16:	/* "\033[29~" */
		    FKEY(28, XK_F15);
		    break;

		case XK_F17:	/* "\033[31~" */
		case XK_F18:	/* "\033[32~" */
		case XK_F19:	/* "\033[33~" */
		case XK_F20:	/* "\033[34~" */
		case XK_F21:	/* "\033[35~" */
		case XK_F22:	/* "\033[36~" */
		case XK_F23:	/* "\033[37~" */
		case XK_F24:	/* "\033[38~" */
		case XK_F25:	/* "\033[39~" */
		case XK_F26:	/* "\033[40~" */
		case XK_F27:	/* "\033[41~" */
		case XK_F28:	/* "\033[42~" */
		case XK_F29:	/* "\033[43~" */
		case XK_F30:	/* "\033[44~" */
		case XK_F31:	/* "\033[45~" */
		case XK_F32:	/* "\033[46~" */
		case XK_F33:	/* "\033[47~" */
		case XK_F34:	/* "\033[48~" */
		case XK_F35:	/* "\033[49~" */
		    FKEY(31, XK_F17);
		    break;
#undef FKEY
		}
	/*
	 * Pass meta for all function keys, if 'meta' option set
	 */
#ifdef META8_OPTION
	    if (meta && (meta_char == 0x80) && len > 0)
		kbuf[len - 1] |= 0x80;
#endif
	} else if (ctrl && keysym == XK_minus) {
	    len = 1;
	    kbuf[0] = '\037';	/* Ctrl-Minus generates ^_ (31) */
	} else {
#ifdef META8_OPTION
	/* set 8-bit on */
	    if (meta && (meta_char == 0x80)) {
		unsigned char  *ch;

		for (ch = kbuf; ch < kbuf + len; ch++)
		    *ch |= 0x80;
		meta = 0;
	    }
#endif
#ifdef GREEK_SUPPORT
	    if (greek_mode)
		len = greek_xlat(kbuf, len);
#endif
	/* nil */ ;
	}
#ifdef USE_XIM
    }
#endif

    if (len <= 0)
	return;			/* not mapped */

/*
 * these modifications only affect the static keybuffer
 * pass Shift/Control indicators for function keys ending with `~'
 *
 * eg,
 *   Prior = "ESC[5~"
 *   Shift+Prior = "ESC[5~"
 *   Ctrl+Prior = "ESC[5^"
 *   Ctrl+Shift+Prior = "ESC[5@"
 * Meta adds an Escape prefix (with META8_OPTION, if meta == <escape>).
 */
    if (kbuf[0] == '\033' && kbuf[1] == '[' && kbuf[len - 1] == '~')
	kbuf[len - 1] = (shft ? (ctrl ? '@' : '$') : (ctrl ? '^' : '~'));

/* escape prefix */
    if (meta
#ifdef META8_OPTION
	&& (meta_char == 033)
#endif
	) {
	const unsigned char ch = '\033';

	tt_write(&ch, 1);
    }
#ifdef DEBUG_CMD
    if (debug_key) {		/* Display keyboard buffer contents */
	char           *p;
	int             i;

	fprintf(stderr, "key 0x%04X [%d]: `", (unsigned int)keysym, len);
	for (i = 0, p = kbuf; i < len; i++, p++)
	    fprintf(stderr, (*p >= ' ' && *p < '\177' ? "%c" : "\\%03o"), *p);
	fprintf(stderr, "'\n");
    }
#endif				/* DEBUG_CMD */
    tt_write(kbuf, len);
}
/*}}} */

#if (MENUBAR_MAX)
/*{{{ cmd_write(), cmd_getc() */
/* attempt to `write' COUNT to the input buffer */
/* EXTPROTO */
unsigned int
cmd_write(const unsigned char *str, unsigned int count)
{
# if 1				/* append to end of buffer */
    int             n, s;

    n = cmdbuf_ptr - cmdbuf_base;
    s = cmdbuf_base + BUFSIZ - 1 - cmdbuf_endp;
    if (n > 0 && s < count) {
	MEMMOVE(cmdbuf_base, cmdbuf_ptr, cmdbuf_endp - cmdbuf_ptr);
	cmdbuf_ptr = cmdbuf_base;
	cmdbuf_endp -= n;
	s += n;
    }
    MIN_IT(count, s);		/* silent truncation */
    for (; count--;)
	*cmdbuf_endp++ = *str++;
# else				/* insert at beginning of buffer */
    int             n;

    n = (count - (cmdbuf_ptr - cmdbuf_base));
/* need to insert more chars than space available in the front */
    if (n > 0) {
    /* try to get more space from the end */
	unsigned char  *endp;

	endp = (cmdbuf_base + sizeof(cmdbuf_base) - 1);		/* max pointer */
	MIN_IT(n, endp - cmdbuf_ptr);	/* max # chars to insert */
	MIN_IT(cmdbuf_endp, endp - n);	/* truncate end if needed */

	MEMMOVE(cmdbuf_ptr + n, cmdbuf_ptr, n);
	cmdbuf_ptr += n;
	cmdbuf_endp += n;
    }
    while (count-- && cmdbuf_ptr > cmdbuf_base)
	*--cmdbuf_ptr = str[count];	/* sneak one in */
# endif

    return 0;
}
#endif				/* MENUBAR_MAX */
/* cmd_getc() - Return next input character */
/*
 * Return the next input character after first passing any keyboard input
 * to the command.
 */
/* INTPROTO */
unsigned char
cmd_getc(void)
{
#define TIMEOUT_USEC	5000
    fd_set          readfds;
    int             retval;
    int             quick_timeout;
    struct timeval  value;

/*
 * If there have been a lot of new lines, then update the screen
 * What the heck I'll cheat and only refresh less than every page-full.
 * the number of pages between refreshes is refresh_limit, which
 * is incremented here because we must be doing flat-out scrolling.
 *
 * refreshing should be correct for small scrolls, because of the
 * time-out
 */
    if (refresh_count >= (refresh_limit * (TermWin.nrow - 1))) {
	if (refresh_limit < REFRESH_PERIOD)
	    refresh_limit++;
	refresh_count = 0;
	scr_refresh(refresh_type);
    }

    if (cmdbuf_ptr < cmdbuf_endp)	/* characters already read in */
	return (*cmdbuf_ptr++);

    for (;;) {
	if (v_bufstr < v_bufptr)	/* output any pending chars */
	    tt_write(NULL, 0);

	while (XPending(Xdisplay)) {	/* process pending X events */
	    XProcessEvent(Xdisplay);
	/* in case button actions pushed chars to cmdbuf */
	    if (cmdbuf_ptr < cmdbuf_endp)
		return (*cmdbuf_ptr++);
	}

#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
	if (scrollbar_isUp()) {
	    if (!scroll_arrow_delay-- && scr_page(UP, 1)) {
		scroll_arrow_delay = SCROLLBAR_CONTINUOUS_DELAY;
		refresh_type |= SMOOTH_REFRESH;
	    }
	} else if (scrollbar_isDn()) {
	    if (!scroll_arrow_delay-- && scr_page(DN, 1)) {
		scroll_arrow_delay = SCROLLBAR_CONTINUOUS_DELAY;
		refresh_type |= SMOOTH_REFRESH;
	    }
	}
#endif				/* NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING */

    /* Nothing to do! */
	FD_ZERO(&readfds);
	FD_SET(cmd_fd, &readfds);
	FD_SET(Xfd, &readfds);
	value.tv_usec = TIMEOUT_USEC;
	value.tv_sec = 0;

	quick_timeout = want_refresh;
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
	quick_timeout |= scrollbar_isUpDn();
#endif
#ifdef TRANSPARENT
	quick_timeout |= want_full_refresh;
#endif
	retval = select(num_fds, &readfds, NULL, NULL,
			(quick_timeout ? &value : NULL));

    /* See if we can read from the application */
	if (FD_ISSET(cmd_fd, &readfds)) {
	    int             count, n;

	    cmdbuf_ptr = cmdbuf_endp = cmdbuf_base;
	    for (count = BUFSIZ; count; count -= n, cmdbuf_endp += n)
		if ((n = read(cmd_fd, cmdbuf_endp, count)) <= 0)
		    break;
	    if (count != BUFSIZ)	/* some characters read in */
		return (*cmdbuf_ptr++);
	}
    /* select statement timed out - we're not hard and fast scrolling */
	if (retval == 0) {
	    refresh_count = 0;
	    refresh_limit = 1;
	}
#ifdef TRANSPARENT
	if (want_full_refresh) {
	    want_full_refresh = 0;
	    scr_clear();
	    scr_touch(False);
	    want_refresh = 1;
	}
#endif
	if (want_refresh) {
	    scr_refresh(refresh_type);
	    scrollbar_show(1);
#ifdef USE_XIM
	    IMSendSpot();
#endif
	}
    }
/* NOTREACHED */
}
/*}}} */

/*
 * the 'essential' information for reporting Mouse Events
 * pared down from XButtonEvent
 */
static struct {
    int             clicks;
    Time            time;	/* milliseconds */
    unsigned int    state;	/* key or button mask */
    unsigned int    button;	/* detail */
} MEvent = {
    0, CurrentTime, 0, AnyButton
};

/* INTPROTO */
void
mouse_report(const XButtonEvent * ev)
{
    int             button_number, key_state = 0;
    int             x, y;

    x = ev->x;
    y = ev->y;
    pixel_position(&x, &y);

    button_number = (MEvent.button == AnyButton ? 3
						: (MEvent.button - Button1));

    if (PrivateModes & PrivMode_MouseX10) {
    /*
     * do not report ButtonRelease
     * no state info allowed
     */
	key_state = 0;
	if (button_number == 3)
	    return;
    } else {
    /* XTerm mouse reporting needs these values:
     *   4 = Shift
     *   8 = Meta
     *  16 = Control
     * plus will add in our own Double-Click reporting
     *  32 = Double Click
     */
	key_state = ((MEvent.state & ShiftMask) ? 4 : 0)
		     + ((MEvent.state & ModMetaMask) ? 8 : 0)
		     + ((MEvent.state & ControlMask) ? 16 : 0);
#ifdef MOUSE_REPORT_DOUBLECLICK
	key_state += ((MEvent.clicks > 1) ? 32 : 0);
#endif
    }

#ifdef DEBUG_MOUSEREPORT
    fprintf(stderr, "Mouse [");
    if (key_state & 16)
	fputc('C', stderr);
    if (key_state & 4)
	fputc('S', stderr);
    if (key_state & 8)
	fputc('A', stderr);
    if (key_state & 32)
	fputc('2', stderr);
    fprintf(stderr, "]: <%d>, %d/%d\n",
	    button_number,
	    x + 1,
	    y + 1);
#else
    tt_printf("\033[M%c%c%c",
	      (32 + button_number + key_state),
	      (32 + x + 1),
	      (32 + y + 1));
#endif
}

/*{{{ process an X event */
/* INTPROTO */
void
process_x_event(XEvent * ev)
{
    int             reportmode;
    Window          unused_root, unused_child;
    int             unused_root_x, unused_root_y;
    unsigned int    unused_mask;
    static int      bypass_keystate = 0;
/* Hops - csr offset in thumb/slider to give proper Scroll behaviour */
    static int      csrO = 0;

#ifdef DEBUG_X
    const char * const eventnames[] =
    {				/* mason - this matches my system */
	"",
	"",
	"KeyPress",
	"KeyRelease",
	"ButtonPress",
	"ButtonRelease",
	"MotionNotify",
	"EnterNotify",
	"LeaveNotify",
	"FocusIn",
	"FocusOut",
	"KeymapNotify",
	"Expose",
	"GraphicsExpose",
	"NoExpose",
	"VisibilityNotify",
	"CreateNotify",
	"DestroyNotify",
	"UnmapNotify",
	"MapNotify",
	"MapRequest",
	"ReparentNotify",
	"ConfigureNotify",
	"ConfigureRequest",
	"GravityNotify",
	"ResizeRequest",
	"CirculateNotify",
	"CirculateRequest",
	"PropertyNotify",
	"SelectionClear",
	"SelectionRequest",
	"SelectionNotify",
	"ColormapNotify",
	"ClientMessage",
	"MappingNotify"
    };
    D_X((stderr, "Event: %-16s, %s p:%08lx,%s vt:%08lx,%s sb:%08lx", eventnames[ev->type], (ev->xany.window == TermWin.parent[0] ? "->" : "  "), TermWin.parent[0], (ev->xany.window == TermWin.vt ? "->" : "  "), TermWin.vt, (ev->xany.window == scrollBar.win ? "->" : "  "), scrollBar.win));
#endif

    switch (ev->type) {
    case KeyPress:
	lookup_key(ev);
	break;

    case ClientMessage:
	if (ev->xclient.format == 32 && ev->xclient.data.l[0] == wmDeleteWindow)
	    exit(EXIT_SUCCESS);
#ifdef OFFIX_DND
    /* OffiX Dnd (drag 'n' drop) protocol */
	if (ev->xclient.message_type == DndProtocol
	    && (ev->xclient.data.l[0] == DndFile
		|| ev->xclient.data.l[0] == DndDir
		|| ev->xclient.data.l[0] == DndLink)) {
	/* Get Dnd data */
	    Atom            ActualType;
	    int             ActualFormat;
	    unsigned char  *data;
	    unsigned long   Size, RemainingBytes;

	    XGetWindowProperty(Xdisplay, Xroot,
			       DndSelection,
			       0L, 1000000L,
			       False, AnyPropertyType,
			       &ActualType, &ActualFormat,
			       &Size, &RemainingBytes,
			       &data);
	    XChangeProperty(Xdisplay, Xroot,
			    XA_CUT_BUFFER0, XA_STRING,
			    8, PropModeReplace,
			    data, STRLEN(data));
	    selection_paste(Xroot, XA_CUT_BUFFER0, True);
	    XSetInputFocus(Xdisplay, Xroot, RevertToNone, CurrentTime);
	}
#endif				/* OFFIX_DND */
	break;

    case MappingNotify:
	XRefreshKeyboardMapping(&(ev->xmapping));
	break;

    /*
     * XXX: this is not the _current_ arrangement
     * Here's my conclusion:
     * If the window is completely unobscured, use bitblt's
     * to scroll. Even then, they're only used when doing partial
     * screen scrolling. When partially obscured, we have to fill
     * in the GraphicsExpose parts, which means that after each refresh,
     * we need to wait for the graphics expose or Noexpose events,
     * which ought to make things real slow!
     */
    case VisibilityNotify:
	switch (ev->xvisibility.state) {
	case VisibilityUnobscured:
	    refresh_type = FAST_REFRESH;
	    break;
	case VisibilityPartiallyObscured:
	    refresh_type = SLOW_REFRESH;
	    break;
	default:
	    refresh_type = NO_REFRESH;
	    break;
	}
	break;

    case FocusIn:
	if (!TermWin.focus) {
	    TermWin.focus = 1;
	    want_refresh = 1;
#ifndef NO_XLOCALE
	    if (Input_Context != NULL)
		XSetICFocus(Input_Context);
#endif
	}
	break;

    case FocusOut:
	if (TermWin.focus) {
	    TermWin.focus = 0;
	    want_refresh = 1;
#ifndef NO_XLOCALE
	    if (Input_Context != NULL)
		XUnsetICFocus(Input_Context);
#endif
	}
	break;

    case ConfigureNotify:
	if (ev->xconfigure.window != TermWin.parent[0])
	    break;
#ifdef TRANSPARENT		/* XXX: maybe not needed - leave in for now */
	if (Options & Opt_transparent) {
	    check_our_parents(False);
	    if (am_transparent)
		want_full_refresh = 1;
	}
#endif
	if (!RemoveFromCNQueue(ev->xconfigure.width, ev->xconfigure.height)) {
	    resize_window(ev->xconfigure.width, ev->xconfigure.height);
#ifdef USE_XIM
	    IMSetStatusPosition();
#endif
	}
	break;

    case SelectionClear:
	selection_clear();
	break;

    case SelectionNotify:
	selection_paste(ev->xselection.requestor, ev->xselection.property,
			True);
	break;

    case SelectionRequest:
	selection_send(&(ev->xselectionrequest));
	break;

    case UnmapNotify:
	TermWin.mapped = 0;
	break;

    case MapNotify:
	TermWin.mapped = 1;
	break;

#ifdef TRANSPARENT
    case PropertyNotify:
	{
	/*
	 * if user used some Esetroot compatible prog to set the root
	 * bg, use the property to determine that. We don't use it's
	 * value, yet
	 */
	    if (ev->xproperty.atom != xrootpmapid)
		break;
	}
    /* FALLTHROUGH */

    case ReparentNotify:
	if ((Options & Opt_transparent)
	    && check_our_parents(False)) {	/* parents change then clear screen */
	    if (am_transparent)
		want_full_refresh = 1;
	}
	break;
#endif				/* TRANSPARENT */

    case GraphicsExpose:
    case Expose:
	if (ev->xany.window == TermWin.vt) {
	    scr_expose(ev->xexpose.x, ev->xexpose.y,
		       ev->xexpose.width, ev->xexpose.height, True);
	} else {
	    XEvent          unused_xevent;

	    while (XCheckTypedWindowEvent(Xdisplay, ev->xany.window,
					  Expose,
					  &unused_xevent)) ;
	    while (XCheckTypedWindowEvent(Xdisplay, ev->xany.window,
					  GraphicsExpose,
					  &unused_xevent)) ;
	    if (isScrollbarWindow(ev->xany.window)) {
		scrollbar_setNone();
		scrollbar_show(0);
	    }
	    if (menubar_visible() && isMenuBarWindow(ev->xany.window))
		menubar_expose();
	    Gr_expose(ev->xany.window);
	}
	break;

    case ButtonPress:
	bypass_keystate = (ev->xbutton.state & (ModMetaMask | ShiftMask));
	reportmode = (bypass_keystate ?
		      0 : (PrivateModes & PrivMode_mouse_report));

	if (ev->xany.window == TermWin.vt) {
	    if (ev->xbutton.subwindow != None)
		Gr_ButtonPress(ev->xbutton.x, ev->xbutton.y);
	    else {
		if (reportmode) {
		/* mouse report from vt window */
		/* save the xbutton state (for ButtonRelease) */
		    MEvent.state = ev->xbutton.state;
#ifdef MOUSE_REPORT_DOUBLECLICK
		    if (ev->xbutton.button == MEvent.button
			&& (ev->xbutton.time - MEvent.time < MULTICLICK_TIME)) {
		    /* same button, within alloted time */
			MEvent.clicks++;
			if (MEvent.clicks > 1) {
			/* only report double clicks */
			    MEvent.clicks = 2;
			    mouse_report(&(ev->xbutton));

			/* don't report the release */
			    MEvent.clicks = 0;
			    MEvent.button = AnyButton;
			}
		    } else {
		    /* different button, or time expired */
			MEvent.clicks = 1;
			MEvent.button = ev->xbutton.button;
			mouse_report(&(ev->xbutton));
		    }
#else
		    MEvent.button = ev->xbutton.button;
		    mouse_report(&(ev->xbutton));
#endif				/* MOUSE_REPORT_DOUBLECLICK */
		} else {
		    if (ev->xbutton.button != MEvent.button)
			MEvent.clicks = 0;
		    switch (ev->xbutton.button) {
		    case Button1:
			if (MEvent.button == Button1
			    && (ev->xbutton.time - MEvent.time < MULTICLICK_TIME))
			    MEvent.clicks++;
			else
			    MEvent.clicks = 1;
			selection_click(MEvent.clicks, ev->xbutton.x,
					ev->xbutton.y);
			MEvent.button = Button1;
			break;

		    case Button3:
			if (MEvent.button == Button3
			    && (ev->xbutton.time - MEvent.time < MULTICLICK_TIME))
			    selection_rotate(ev->xbutton.x, ev->xbutton.y);
			else
			    selection_extend(ev->xbutton.x, ev->xbutton.y, 1);
			MEvent.button = Button3;
			break;
		    }
		}
		MEvent.time = ev->xbutton.time;
		return;
	    }
	}
	if (isScrollbarWindow(ev->xany.window)) {
	    scrollbar_setNone();
	/*
	 * Rxvt-style scrollbar:
	 * move up if mouse is above slider
	 * move dn if mouse is below slider
	 *
	 * XTerm-style scrollbar:
	 * Move display proportional to pointer location
	 * pointer near top -> scroll one line
	 * pointer near bot -> scroll full page
	 */
#ifndef NO_SCROLLBAR_REPORT
	    if (reportmode) {
	    /*
	     * Mouse report disabled scrollbar:
	     * arrow buttons - send up/down
	     * click on scrollbar - send pageup/down
	     */
		if (scrollbar_upButton(ev->xbutton.y))
		    tt_printf("\033[A");
		else if (scrollbar_dnButton(ev->xbutton.y))
		    tt_printf("\033[B");
		else
		    switch (ev->xbutton.button) {
		    case Button2:
			tt_printf("\014");
			break;
		    case Button1:
			tt_printf("\033[6~");
			break;
		    case Button3:
			tt_printf("\033[5~");
			break;
		    }
	    } else
#endif				/* NO_SCROLLBAR_REPORT */
	    {
		if (scrollbar_upButton(ev->xbutton.y)) {
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
		    scroll_arrow_delay = SCROLLBAR_INITIAL_DELAY;
#endif
		    if (scr_page(UP, 1))
			scrollbar_setUp();
		} else if (scrollbar_dnButton(ev->xbutton.y)) {
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
		    scroll_arrow_delay = SCROLLBAR_INITIAL_DELAY;
#endif
		    if (scr_page(DN, 1))
			scrollbar_setDn();
		} else
		    switch (ev->xbutton.button) {
		    case Button2:
#if ! defined(FUNKY_SCROLL_BEHAVIOUR)
		    /* align to thumb centre */
			csrO = (scrollBar.bot - scrollBar.top) / 2;
#elif ! defined(XTERM_SCROLLBAR)
			if (scrollbar_above_slider(ev->xbutton.y)
			    || scrollbar_below_slider(ev->xbutton.y))
#endif				/* FUNKY_SCROLL_BEHAVIOUR */
			    scr_move_to(scrollbar_position(ev->xbutton.y) - csrO,
					scrollbar_size());
			scrollbar_setMotion();
			break;

		    case Button1:
#ifndef FUNKY_SCROLL_BEHAVIOUR
		    /* ptr offset in thumb */
			csrO = ev->xbutton.y - scrollBar.top;
#endif
		    /* FALLTHROUGH */

		    case Button3:
#ifndef XTERM_SCROLLBAR
			if (scrollbar_above_slider(ev->xbutton.y))
# ifdef RXVT_SCROLL_FULL
			    scr_page(UP, TermWin.nrow - 1);
# else
			    scr_page(UP, TermWin.nrow / 4);
# endif
			else if (scrollbar_below_slider(ev->xbutton.y))
# ifdef RXVT_SCROLL_FULL
			    scr_page(DN, TermWin.nrow - 1);
# else
			    scr_page(DN, TermWin.nrow / 4);
# endif
			else
			    scrollbar_setMotion();
#else				/* XTERM_SCROLLBAR */
			scr_page((ev->xbutton.button == Button1 ? DN : UP),
				 (TermWin.nrow *
				  scrollbar_position(ev->xbutton.y) /
				  scrollbar_size())
			    );
#endif				/* XTERM_SCROLLBAR */
			break;
		    }
	    }
	    return;
	}
	if (isMenuBarWindow(ev->xany.window)) {
	    menubar_control(&(ev->xbutton));
	    return;
	}
	break;

    case ButtonRelease:
	csrO = 0;		/* reset csr Offset */
	reportmode = bypass_keystate ? 0
	    : (PrivateModes & PrivMode_mouse_report);

	if (scrollbar_isUpDn()) {
	    scrollbar_setNone();
	    scrollbar_show(0);
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
	    refresh_type &= ~SMOOTH_REFRESH;
#endif
	}
	if (ev->xany.window == TermWin.vt) {
	    if (ev->xbutton.subwindow != None)
		Gr_ButtonRelease(ev->xbutton.x, ev->xbutton.y);
	    else {
		if (reportmode) {
		/* mouse report from vt window */
#ifdef MOUSE_REPORT_DOUBLECLICK
		/* only report the release of 'slow' single clicks */
		    if (MEvent.button != AnyButton
			&& (ev->xbutton.button != MEvent.button
		    || (ev->xbutton.time - MEvent.time > MULTICLICK_TIME / 2))
			) {
			MEvent.clicks = 0;
			MEvent.button = AnyButton;
			mouse_report(&(ev->xbutton));
		    }
#else				/* MOUSE_REPORT_DOUBLECLICK */
		    MEvent.button = AnyButton;
		    mouse_report(&(ev->xbutton));
#endif				/* MOUSE_REPORT_DOUBLECLICK */
		    return;
		}
	    /*
	     * dumb hack to compensate for the failure of click-and-drag
	     * when overriding mouse reporting
	     */
		if (PrivateModes & PrivMode_mouse_report
		    && bypass_keystate
		    && ev->xbutton.button == Button1
		    && MEvent.clicks <= 1)
		    selection_extend(ev->xbutton.x, ev->xbutton.y, 0);

		switch (ev->xbutton.button) {
		case Button1:
		case Button3:
		    selection_make(ev->xbutton.time);
		    break;
		case Button2:
		    selection_request(ev->xbutton.time,
				      ev->xbutton.x, ev->xbutton.y);
		    break;
#ifndef NO_MOUSE_WHEEL
		case Button4:
		case Button5:
		    {
			int             i, v;

			i = (ev->xbutton.state & ShiftMask) ? 1 : 5;
			v = (ev->xbutton.button == Button4) ? UP : DN;
# ifdef JUMP_MOUSE_WHEEL
			scr_page(v, i);
			scr_refresh(SMOOTH_REFRESH);
			scrollbar_show(1);
# else
			for (; i--;) {
			    scr_page(v, 1);
			    scr_refresh(SMOOTH_REFRESH);
			    scrollbar_show(1);
			}
# endif
		    }
		    break;
#endif
		}
	    }
	} else if (isMenuBarWindow(ev->xany.window)) {
	    menubar_control(&(ev->xbutton));
	}
	break;

    case MotionNotify:
	if (isMenuBarWindow(ev->xany.window)) {
	    menubar_control(&(ev->xbutton));
	    break;
	}
	if ((PrivateModes & PrivMode_mouse_report) && !(bypass_keystate))
	    break;

	if (ev->xany.window == TermWin.vt) {
	    if ((ev->xbutton.state & (Button1Mask | Button3Mask))) {
		while (XCheckTypedWindowEvent(Xdisplay, TermWin.vt,
					      MotionNotify, ev)) ;
		XQueryPointer(Xdisplay, TermWin.vt,
			      &unused_root, &unused_child,
			      &unused_root_x, &unused_root_y,
			      &(ev->xbutton.x), &(ev->xbutton.y),
			      &unused_mask);
#ifdef MOUSE_THRESHOLD
	    /* deal with a `jumpy' mouse */
		if ((ev->xmotion.time - MEvent.time) > MOUSE_THRESHOLD)
#endif
		    selection_extend((ev->xbutton.x), (ev->xbutton.y),
				  (ev->xbutton.state & Button3Mask) ? 2 : 0);
	    }
	} else if (isScrollbarWindow(ev->xany.window) && scrollbar_isMotion()) {
	    while (XCheckTypedWindowEvent(Xdisplay, scrollBar.win,
					  MotionNotify, ev)) ;
	    XQueryPointer(Xdisplay, scrollBar.win,
			  &unused_root, &unused_child,
			  &unused_root_x, &unused_root_y,
			  &(ev->xbutton.x), &(ev->xbutton.y),
			  &unused_mask);
	    scr_move_to(scrollbar_position(ev->xbutton.y) - csrO,
			scrollbar_size());
	    scr_refresh(refresh_type);
	    refresh_count = refresh_limit = 0;
	    scrollbar_show(1);
	}
	break;
    }
}

#ifdef TRANSPARENT
/*
 * Check our parents are still who we think they are.
 */
/* EXTPROTO */
int
check_our_parents(Bool first_run)
{
    int             i, pchanged;
    unsigned int    n;
    Window          root, oldp, *list;

/* Get all X ops out of the queue so that our information is up-to-date. */
    XSync(Xdisplay, False);

/*
 * Make the frame window set by the window manager have
 * the root background. Some window managers put multiple nested frame
 * windows for each client, so we have to take care about that.
 */
    pchanged = 0;
    D_X((stderr, "InheritPixmap Seeking to  %08lx", Xroot));
    for (i = 1; i < KNOW_PARENTS; i++) {
	oldp = TermWin.parent[i];
	XQueryTree(Xdisplay, TermWin.parent[i - 1], &root,
		   &TermWin.parent[i], &list, &n);
	XFree(list);
	D_X((stderr, "InheritPixmap Parent[%d] = %08lx", i, TermWin.parent[i]));
	if (TermWin.parent[i] == Xroot) {
	    if (oldp != None)
		pchanged = 1;
	    break;
        }
	if (oldp != TermWin.parent[i])
	    pchanged = 1;
    }
    n = 0;
    if (first_run == True || pchanged) {
	XWindowAttributes wattr;
	int             d;

	XGetWindowAttributes(Xdisplay, Xroot, &wattr);
	d = wattr.depth;
	for ( ; n < i; n++) {
	    XGetWindowAttributes(Xdisplay, TermWin.parent[n], &wattr);
	    D_X((stderr, "InheritPixmap Checking Parent[%d]: %s", n, (wattr.depth == d && wattr.class != InputOnly) ? "OK" : "FAIL"));
	    if (wattr.depth != d || wattr.class == InputOnly) {
		n = KNOW_PARENTS + 1;
		break;
	    }
	}
    }
    if (n > KNOW_PARENTS) {
	D_X((stderr, "InheritPixmap Turning off"));
	XSetWindowBackground(Xdisplay, TermWin.parent[0], PixColors[Color_fg]);
	XSetWindowBackground(Xdisplay, TermWin.vt, PixColors[Color_bg]);
	am_transparent = 0;
    /* XXX: also turn off Opt_transparent? */
    } else {
	D_X((stderr, "InheritPixmap Turning on (%d parents)", i - 1));
	for (n = 0; n < i; n++)
	    XSetWindowBackgroundPixmap(Xdisplay, TermWin.parent[n],
				       ParentRelative);
	am_transparent = 1;
	if (first_run == True) {
	    XSetWindowBackgroundPixmap(Xdisplay, TermWin.vt, ParentRelative);
	    XSelectInput(Xdisplay, Xroot, PropertyChangeMask);
	}
    }

    for (; i < KNOW_PARENTS; i++)
	TermWin.parent[i] = None;
    return pchanged;
}
#endif

/*}}} */

/*
 * Send printf() formatted output to the command.
 * Only use for small ammounts of data.
 */
/* EXTPROTO */
void
tt_printf(const char *fmt,...)
{
    va_list         arg_ptr;
    unsigned char   buf[256];

    va_start(arg_ptr, fmt);
    vsprintf(buf, fmt, arg_ptr);
    va_end(arg_ptr);
    tt_write(buf, STRLEN(buf));
}

/*{{{ print pipe */
/*----------------------------------------------------------------------*/
#ifdef PRINTPIPE
/* EXTPROTO */
FILE           *
popen_printer(void)
{
    FILE           *stream = popen(rs.print_pipe, "w");

    if (stream == NULL)
	print_error("can't open printer pipe");
    return stream;
}

/* EXTPROTO */
int
pclose_printer(FILE *stream)
{
    fflush(stream);
/* pclose() reported not to work on SunOS 4.1.3 */
# if defined (__sun__)		/* TODO: RESOLVE THIS */
/* pclose works provided SIGCHLD handler uses waitpid */
    return pclose(stream);	/* return fclose (stream); */
# else
    return pclose(stream);
# endif
}

/*
 * simulate attached vt100 printer
 */
/* INTPROTO */
void
process_print_pipe(void)
{
    int             done;
    FILE           *fd;

    if ((fd = popen_printer()) == NULL)
	return;

/*
 * Send all input to the printer until either ESC[4i or ESC[?4i
 * is received.
 */
    for (done = 0; !done;) {
	unsigned char   buf[8];
	unsigned char   ch;
	unsigned int    i, len;

	if ((ch = cmd_getc()) != '\033') {
	    if (putc(ch, fd) == EOF)
		break;		/* done = 1 */
	} else {
	    len = 0;
	    buf[len++] = ch;

	    if ((buf[len++] = cmd_getc()) == '[') {
		if ((ch = cmd_getc()) == '?') {
		    buf[len++] = '?';
		    ch = cmd_getc();
		}
		if ((buf[len++] = ch) == '4') {
		    if ((buf[len++] = cmd_getc()) == 'i')
			break;	/* done = 1 */
		}
	    }
	    for (i = 0; i < len; i++)
		if (putc(buf[i], fd) == EOF) {
		    done = 1;
		    break;
		}
	}
    }
    pclose_printer(fd);
}
#endif				/* PRINTPIPE */
/*}}} */

/*{{{ process escape sequences */
/* INTPROTO */
void
process_escape_seq(void)
{
    unsigned char   ch = cmd_getc();

    switch (ch) {
    /* case 1:        do_tek_mode (); break; */
    case '#':
	if (cmd_getc() == '8')
	    scr_E();
	break;
    case '(':
	scr_charset_set(0, cmd_getc());
	break;
    case ')':
	scr_charset_set(1, cmd_getc());
	break;
    case '*':
	scr_charset_set(2, cmd_getc());
	break;
    case '+':
	scr_charset_set(3, cmd_getc());
	break;
#ifdef MULTICHAR_SET
    case '$':
	scr_charset_set(-2, cmd_getc());
	break;
#endif
#ifndef NO_FRILLS
    case '6':
	scr_backindex();
	break;
#endif
    case '7':
	scr_cursor(SAVE);
	break;
    case '8':
	scr_cursor(RESTORE);
	break;
#ifndef NO_FRILLS
    case '9':
	scr_forwardindex();
	break;
#endif
    case '=':
    case '>':
	PrivMode((ch == '='), PrivMode_aplKP);
	break;
    case '@':
	(void)cmd_getc();
	break;
    case 'D':
	scr_index(UP);
	break;
    case 'E':
	scr_add_lines((const unsigned char *)"\n\r", 1, 2);
	break;
    case 'G':
	process_graphics();
	break;
    case 'H':
	scr_set_tab(1);
	break;
    case 'M':
	scr_index(DN);
	break;
    /*case 'N': scr_single_shift (2);   break; */
    /*case 'O': scr_single_shift (3);   break; */
    case 'Z':
	tt_printf(ESCZ_ANSWER);
	break;			/* steal obsolete ESC [ c */
    case '[':
	process_csi_seq();
	break;
    case ']':
	process_xterm_seq();
	break;
    case 'c':
	scr_poweron();
	scrollbar_show(1);
	break;
    case 'n':
	scr_charset_choose(2);
	break;
    case 'o':
	scr_charset_choose(3);
	break;
    }
}
/*}}} */

/*{{{ process CSI (code sequence introducer) sequences `ESC[' */
/* *INDENT-OFF* */
enum {
    CSI_ICH = '@',
             CSI_CUU, CSI_CUD, CSI_CUF, CSI_CUB, CSI_CNL, CSI_CPL, CSI_CHA,
    CSI_CUP, CSI_CHT, CSI_ED , CSI_EL , CSI_IL , CSI_DL , CSI_EF , CSI_EA ,
    CSI_DCH, CSI_SEM, CSI_CPR, CSI_SU , CSI_SD , CSI_NP , CSI_PP , CSI_CTC,
    CSI_ECH, CSI_CVT, CSI_CBT, CSI_091, CSI_092, CSI_093, CSI_094, CSI_095,
    CSI_HPA, CSI_HPR, CSI_REP, CSI_DA , CSI_VPA, CSI_VPR, CSI_HVP, CSI_TBC,
    CSI_SM , CSI_MC , CSI_106, CSI_107, CSI_RM , CSI_SGR, CSI_DSR, CSI_DAQ,
    CSI_112, CSI_113, CSI_114, CSI_115, CSI_116, CSI_117, CSI_118, CSI_119,
    CSI_120, CSI_121, CSI_122, CSI_123, CSI_124, CSI_125, CSI_126, CSI_127
};

#define make_byte(b7,b6,b5,b4,b3,b2,b1,b0)			\
    (((b7) << 7) | ((b6) << 6) | ((b5) << 5) | ((b4) << 4)	\
     | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | (b0))
#define get_byte_array_bit(array, bit)				\
    (!!((array)[(bit) / 8] & (128 >> ((bit) & 7))))

char csi_defaults[] = {
    make_byte(1,1,1,1,1,1,1,1),	/* @, A, B, C, D, E, F, G, */
    make_byte(1,1,0,0,1,1,0,0),	/* H, I, J, K, L, M, N, O, */
    make_byte(1,0,1,1,1,1,1,1),	/* P, Q, R, S, T, U, V, W, */
    make_byte(1,0,1,0,0,0,1,0),	/* X, Y, Z, [, \, ], ^, _, */
    make_byte(1,1,1,0,1,1,1,0),	/* `, a, b, c, d, e, f, g, */
    make_byte(0,0,0,0,0,0,0,0),	/* h, i, j, k, l, m, n, o, */
    make_byte(0,0,0,0,0,0,0,0),	/* p, q, r, s, t, u, v, w, */
    make_byte(0,0,0,0,0,0,0,0)	/* x, y, z, {, |, }, ~,    */
};
/* *INDENT-ON* */

/* INTPROTO */
void
process_csi_seq(void)
{
    unsigned char   ch, priv, i;
    unsigned int    nargs;
    int             arg[ESC_ARGS];

    for (nargs = ESC_ARGS; nargs > 0;)
	arg[--nargs] = 0;

    priv = 0;
    ch = cmd_getc();
    if (ch >= '<' && ch <= '?') {	/* '<' '=' '>' '?' */
	priv = ch;
	ch = cmd_getc();
    }
/* read any numerical arguments */
    do {
	int             n = 0;

	if (isdigit(ch)) {
	    for (; isdigit(ch); ch = cmd_getc())
		n = n * 10 + (ch - '0');
	    if (nargs < ESC_ARGS)
		arg[nargs++] = n;
	}
	if (ch == '\b') {
	    scr_backspace();
	} else if (ch == 033) {
	    process_escape_seq();
	    return;
	} else if (ch < ' ') {
	    scr_add_lines(&ch, 0, 1);
	    return;
	}
	if (ch < CSI_ICH)
	    ch = cmd_getc();
    } while (ch >= ' ' && ch < CSI_ICH);
    if (ch == 033) {
	process_escape_seq();
	return;
    }
    if (ch < ' ' || ch > CSI_127)
	return;

    if (priv == '?')
	if (ch == 'h' || ch == 'l' || ch == 'r' || ch == 's' || ch == 't') {
	    process_terminal_mode(ch, priv, nargs, arg);
	    return;
	}
	
    if (arg[0] == 0) {
	i = ch - CSI_ICH;
	arg[0] = get_byte_array_bit(csi_defaults, i);
    }

    switch (ch) {
#ifdef PRINTPIPE
    case CSI_MC:		/* printing */
	switch (arg[0]) {
	case 0:
	    scr_printscreen(0);
	    break;
	case 5:
	    process_print_pipe();
	    break;
	}
	break;
#endif
    case CSI_CUU:		/* up <n> */
    case CSI_VPR:		/* up <n> */
	scr_gotorc(-arg[0], 0, RELATIVE);
	break;
    case CSI_CUD:		/* down <n> */
	scr_gotorc(arg[0], 0, RELATIVE);
	break;
    case CSI_CUF:		/* right <n> */
    case CSI_HPR:		/* right <n> */
	scr_gotorc(0, arg[0], RELATIVE);
	break;
    case CSI_CUB:		/* left <n> */
	scr_gotorc(0, -arg[0], RELATIVE);
	break;
    case CSI_CNL:		/* down <n> & to first column */
	scr_gotorc(arg[0], 0, R_RELATIVE);
	break;
    case CSI_CPL:		/* up <n> & to first column */
	scr_gotorc(-arg[0], 0, R_RELATIVE);
	break;
    case CSI_CHA:		/* move to col <n> */
    case CSI_HPA:		/* move to col <n> */
	scr_gotorc(0, arg[0] - 1, R_RELATIVE);
	break;
    case CSI_VPA:		/* move to row <n> */
	scr_gotorc(arg[0] - 1, 0, C_RELATIVE);
	break;
    case CSI_CUP:		/* position cursor */
    case CSI_HVP:		/* position cursor */
	scr_gotorc(arg[0] - 1, nargs < 2 ? 0 : (arg[1] - 1), 0);
	break;
    case CSI_CHT:		/* cursor horizontal tab */
	scr_tab(arg[0]);
	break;
    case CSI_CBT:		/* cursor backward tab */
	scr_tab(-arg[0]);
	break;
    case CSI_ED:		/* erase in display */
	scr_erase_screen(arg[0]);
	break;
    case CSI_EL:		/* erase in line */
	scr_erase_line(arg[0]);
	break;
    case CSI_ICH:		/* insert char */
	scr_insdel_chars(arg[0], INSERT);
	break;
    case CSI_IL:		/* insert line */
	scr_insdel_lines(arg[0], INSERT);
	break;
    case CSI_DL:		/* delete line */
	scr_insdel_lines(arg[0], DELETE);
	break;
    case CSI_ECH:		/* erase char */
	scr_insdel_chars(arg[0], ERASE);
	break;
    case CSI_DCH:		/* delete char */
	scr_insdel_chars(arg[0], DELETE);
	break;
    case CSI_SD:		/* scroll down */
    case CSI_094:
	scr_scroll_text(-arg[0]);
	break;
    case CSI_SU:		/* scroll up */
	scr_scroll_text(arg[0]);
	break;
    case CSI_DA:		/* device attributes */
	if (priv == '>')	/* secondary device attributes */
	    tt_printf("\033[>%d;%s;0c", 'R', VSTRING);
	else
	    tt_printf(VT100_ANS);
	break;
    case CSI_SGR:		/* select graphic rendition */
	process_sgr_mode(nargs, arg);
	break;
    case CSI_DSR:		/* device status report */
	switch (arg[0]) {
	case 5:
	    tt_printf("\033[0n");
	    break;		/* ready */
	case 6:
	    scr_report_position();
	    break;
#if defined (ENABLE_DISPLAY_ANSWER)
	case 7:
	    tt_printf("%s\n", rs.display_name);
	    break;
#endif
	case 8:
	    xterm_seq(XTerm_title, APL_NAME "-" VERSION);
	    break;
	}
	break;
    case CSI_114:		/* DECSTBM: set top and bottom margins */
	if (nargs < 2 || arg[0] >= arg[1])
	    scr_scroll_region(0, 10000);
	else
	    scr_scroll_region(arg[0] - 1, arg[1] - 1);
	break;
#ifndef NO_FRILLS
    case CSI_116:
	process_window_ops(arg, nargs);
	break;
#endif
    case CSI_TBC:		/* tab clear */
	switch (arg[0]) {
	case 0:
	    scr_set_tab(0);
	    break;		/* delete tab */
	case 3:
	    scr_set_tab(-1);
	    break;		/* clear all tabs */
	}
	break;
    case CSI_CTC:		/* cursor tab control */
	switch (arg[0]) {
	case 0:
	    scr_set_tab(1);
	    break;		/* = ESC H */
	case 2:
	    scr_set_tab(0);
	    break;		/* = ESC [ 0 g */
	case 5:
	    scr_set_tab(-1);
	    break;		/* = ESC [ 3 g */
	}
	break;
    case CSI_RM:
	scr_insert_mode(0);
	break;
    case CSI_SM:
	scr_insert_mode(1);
	break;
    case CSI_120:		/* DECREQTPARM */
	if (arg[0] == 0 || arg[0] == 1)
	    tt_printf("\033[%d;1;1;112;112;1;0x", arg[0] + 2);
    /* FALLTHROUGH */
    default:
	break;
    }
}
/*}}} */

#ifndef NO_FRILLS
/* ARGSUSED */
/* INTPROTO */
void
process_window_ops(const int *args, int nargs)
{
    int             x, y;
    char           *s;
    XWindowAttributes wattr;
    Window          wdummy;

    if (nargs == 0)
	return;
    switch (args[0]) {
    /*
     * commands
     */
    case 1:			/* deiconify window */
	XMapWindow(Xdisplay, TermWin.parent[0]);
	break;
    case 2:			/* iconify window */
	XIconifyWindow(Xdisplay, TermWin.parent[0], DefaultScreen(Xdisplay));
	break;
    case 3:			/* set position (pixels) */
	AddToCNQueue(szHint.width, szHint.height);
	XMoveWindow(Xdisplay, TermWin.parent[0], args[1], args[2]);
	break;
    case 4:			/* set size (pixels) */
	set_widthheight(args[2], args[1]);
	break;
    case 5:			/* raise window */
	XRaiseWindow(Xdisplay, TermWin.parent[0]);
	break;
    case 6:			/* lower window */
	XLowerWindow(Xdisplay, TermWin.parent[0]);
	break;
    case 7:			/* refresh window */
	scr_touch(True);
	break;
    case 8:			/* set size (chars) */
	set_widthheight(args[2] * TermWin.fwidth, args[1] * TermWin.fheight);
	break;
    default:
	if (args[0] >= 24)	/* set height (chars) */
	    set_widthheight(TermWin.width, args[1] * TermWin.fheight);
	break;
    /*
     * reports - some output format copied from XTerm
     */
    case 11:			/* report window state */
	XGetWindowAttributes(Xdisplay, TermWin.parent[0], &wattr);
	tt_printf("\033[%dt", wattr.map_state == IsViewable ? 1 : 2);
	break;
    case 13:			/* report window position */
	XGetWindowAttributes(Xdisplay, TermWin.parent[0], &wattr);
	XTranslateCoordinates(Xdisplay, TermWin.parent[0], wattr.root,
			      -wattr.border_width, -wattr.border_width,
			      &x, &y, &wdummy);
	tt_printf("\033[3;%d;%dt", x, y);
	break;
    case 14:			/* report window size (pixels) */
	XGetWindowAttributes(Xdisplay, TermWin.parent[0], &wattr);
	tt_printf("\033[4;%d;%dt", wattr.height, wattr.width);
	break;
    case 18:			/* report window size (chars) */
	tt_printf("\033[8;%d;%dt", TermWin.nrow, TermWin.ncol);
	break;
    case 20:			/* report icon label */
	XGetIconName(Xdisplay, TermWin.parent[0], &s);
	tt_printf("\033]L%s\033\\", s ? s : "");
	break;
    case 21:			/* report window title */
	XFetchName(Xdisplay, TermWin.parent[0], &s);
	tt_printf("\033]l%s\033\\", s ? s : "");
	break;
    }
}
#endif

/*{{{ process xterm text parameters sequences `ESC ] Ps ; Pt BEL' */
/* INTPROTO */
void
process_xterm_seq(void)
{
    unsigned char   ch, string[STRING_MAX];
    int             arg;

    ch = cmd_getc();
    for (arg = 0; isdigit(ch); ch = cmd_getc())
	arg = arg * 10 + (ch - '0');

    if (ch == ';') {
	int             n = 0;

	while ((ch = cmd_getc()) != 007) {
	    if (ch) {
		if (ch == '\t')
		    ch = ' ';	/* translate '\t' to space */
		else if (ch < ' ')
		    return;	/* control character - exit */

		if (n < sizeof(string) - 1)
		    string[n++] = ch;
	    }
	}
	string[n] = '\0';
    /*
     * menubar_dispatch() violates the constness of the string,
     * so do it here
     */
	if (arg == XTerm_Menu)
	    menubar_dispatch((char *)string);
	else
	    xterm_seq(arg, (char *)string);
    }
}

/*}}} */

/*{{{ process DEC private mode sequences `ESC [ ? Ps mode' */
/*
 * mode can only have the following values:
 *      'l' = low
 *      'h' = high
 *      's' = save
 *      'r' = restore
 *      't' = toggle
 * so no need for fancy checking
 */
/* INTOPROTO */
int
privcases(int mode, unsigned long bit)
{
    int             state;

    if (mode == 's') {
	SavedModes |= (PrivateModes & bit);
	return -1;
    } else {
	if (mode == 'r')
	    state = (SavedModes & bit) ? 1 : 0;	/* no overlapping */
	else
	    state = (mode == 't') ? !(PrivateModes & bit) : mode;
	PrivMode(state, bit);
    }
    return state;
}
/* INTPROTO */
void
process_terminal_mode(int mode, int priv, unsigned int nargs, const int *arg)
{
    unsigned int    i;
    int             state;

    if (nargs == 0)
	return;

/* make lo/hi boolean */
    if (mode == 'l')
	mode = 0;
    else if (mode == 'h')
	mode = 1;

    for (i = 0; i < nargs; i++)
	switch (arg[i]) {
	case 1:		/* application cursor keys */
	    privcases(mode, PrivMode_aplCUR);
	    break;

	/* case 2:   - reset charsets to USASCII */

	case 3:		/* 80/132 */
	    if ((state = privcases(mode, PrivMode_132)) != -1)
		if (PrivateModes & PrivMode_132OK)
		    set_widthheight((state ? 132 : 80) * TermWin.fwidth,
				    TermWin.height);
	    break;

	/* case 4:   - smooth scrolling */

	case 5:		/* reverse video */
	    if ((state = privcases(mode, PrivMode_rVideo)) != -1)
		scr_rvideo_mode(state);
	    break;

	case 6:		/* relative/absolute origins  */
	    if ((state = privcases(mode, PrivMode_relOrigin)) != -1)
		scr_relative_origin(state);
	    break;

	case 7:		/* autowrap */
	    if ((state = privcases(mode, PrivMode_Autowrap)) != -1)
		scr_autowrap(state);
	    break;

	/* case 8:   - auto repeat, can't do on a per window basis */

	case 9:		/* X10 mouse reporting */
	    if (privcases(mode, PrivMode_MouseX10) != -1)
	/* orthogonal */
		if (PrivateModes & PrivMode_MouseX10)
		    PrivateModes &= ~(PrivMode_MouseX11);
	    break;
#ifdef menuBar_esc
	case menuBar_esc:
	    if ((state = privcases(mode, PrivMode_menuBar)) != -1)
		map_menuBar(state);
	    break;
#endif
#ifdef scrollBar_esc
	case scrollBar_esc:
	    if ((state = privcases(mode, PrivMode_scrollBar)) != -1)
		map_scrollBar(state);
	    break;
#endif
	case 25:		/* visible/invisible cursor */
	    if ((state = privcases(mode, PrivMode_VisibleCursor)) != -1)
		scr_cursor_visible(state);
	    break;

	case 35:
	    privcases(mode, PrivMode_ShiftKeys);
	    break;

	case 40:		/* 80 <--> 132 mode */
	    privcases(mode, PrivMode_132OK);
	    break;

	case 47:		/* secondary screen */
	    if ((state = privcases(mode, PrivMode_Screen)) != -1)
		scr_change_screen(state);
	    break;

	case 66:		/* application key pad */
	    privcases(mode, PrivMode_aplKP);
	    break;

	case 67:
#ifndef NO_BACKSPACE_KEY
	    if (PrivateModes & PrivMode_HaveBackSpace)
		privcases(mode, PrivMode_BackSpace);
#endif
	    break;

	case 1000:		/* X11 mouse reporting */
	    if (privcases(mode, PrivMode_MouseX11) != -1)
	/* orthogonal */
		if (PrivateModes & PrivMode_MouseX11)
	            PrivateModes &= ~(PrivMode_MouseX10);
	    break;
#if 0
	case 1001:
	    break;		/* X11 mouse highlighting */
#endif
	case 1010:		/* scroll to bottom on TTY output inhibit */
	    if (privcases(mode, PrivMode_TtyOutputInh) != -1) {
		if (PrivateModes & PrivMode_TtyOutputInh)
		    Options &= ~Opt_scrollTtyOutput;
	        else
		    Options |= Opt_scrollTtyOutput;
	    }
	    break;

	case 1011:		/* scroll to bottom on key press */
	    if (privcases(mode, PrivMode_Keypress) != -1) {
		if (PrivateModes & PrivMode_Keypress)
		    Options |= Opt_scrollKeypress;
		else
	            Options &= ~Opt_scrollKeypress;
	    }
	/* FALLTHROUGH */
	default:
	    break;
	}
}
/*}}} */

/*{{{ process sgr sequences */
/* INTPROTO */
void
process_sgr_mode(unsigned int nargs, const int *arg)
{
    unsigned int    i;

    if (nargs == 0) {
	scr_rendition(0, ~RS_None);
	return;
    }
    for (i = 0; i < nargs; i++)
	switch (arg[i]) {
	case 0:
	    scr_rendition(0, ~RS_None);
	    break;
	case 1:
	    scr_rendition(1, RS_Bold);
	    break;
	case 4:
	    scr_rendition(1, RS_Uline);
	    break;
	case 5:
	    scr_rendition(1, RS_Blink);
	    break;
	case 7:
	    scr_rendition(1, RS_RVid);
	    break;
	case 22:
	    scr_rendition(0, RS_Bold);
	    break;
	case 24:
	    scr_rendition(0, RS_Uline);
	    break;
	case 25:
	    scr_rendition(0, RS_Blink);
	    break;
	case 27:
	    scr_rendition(0, RS_RVid);
	    break;

	case 30:
	case 31:		/* set fg color */
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	    scr_color(minCOLOR + (arg[i] - 30), RS_Bold);
	    break;
	case 39:		/* default fg */
	    scr_color(restoreFG, RS_Bold);
	    break;

	case 40:
	case 41:		/* set bg color */
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	    scr_color(minCOLOR + (arg[i] - 40), RS_Blink);
	    break;
	case 49:		/* default bg */
	    scr_color(restoreBG, RS_Blink);
	    break;
	}
}
/*}}} */

/*{{{ process Rob Nation's own graphics mode sequences */
/* INTPROTO */
void
process_graphics(void)
{
    unsigned char   ch, cmd = cmd_getc();

#ifndef RXVT_GRAPHICS
    if (cmd == 'Q') {		/* query graphics */
	tt_printf("\033G0\n");	/* no graphics */
	return;
    }
/* swallow other graphics sequences until terminating ':' */
    do
	ch = cmd_getc();
    while (ch != ':');
#else
    int             nargs;
    int             args[NGRX_PTS];
    unsigned char  *text = NULL;

    if (cmd == 'Q') {		/* query graphics */
	tt_printf("\033G1\n");	/* yes, graphics (color) */
	return;
    }
    for (nargs = 0; nargs < (sizeof(args) / sizeof(args[0])) - 1;) {
	int             neg;

	ch = cmd_getc();
	neg = (ch == '-');
	if (neg || ch == '+')
	    ch = cmd_getc();

	for (args[nargs] = 0; isdigit(ch); ch = cmd_getc())
	    args[nargs] = args[nargs] * 10 + (ch - '0');
	if (neg)
	    args[nargs] = -args[nargs];

	nargs++;
	args[nargs] = 0;
	if (ch != ';')
	    break;
    }

    if ((cmd == 'T') && (nargs >= 5)) {
	int             i, len = args[4];

	text = MALLOC((len + 1) * sizeof(char));

	if (text != NULL) {
	    for (i = 0; i < len; i++)
		text[i] = cmd_getc();
	    text[len] = '\0';
	}
    }
    Gr_do_graphics(cmd, nargs, args, text);
#endif
}
/*}}} */

/* ------------------------------------------------------------------------- */
/*
 * A simple queue to hold ConfigureNotify events we've generated so we can
 * bypass them when they come in.  Don't bother keeping a pointer to the tail
 * since we don't expect masses of CNs at any one time.
 */
/* EXTPROTO */
void
AddToCNQueue(int width, int height)
{
    XCNQueue_t     *rq, *nrq;

    nrq = (XCNQueue_t *) MALLOC(sizeof(XCNQueue_t));
    assert(nrq);
    nrq->next = NULL;
    nrq->width = width;
    nrq->height = height;
    if (XCNQueue == NULL)
	XCNQueue = nrq;
    else {
	for (rq = XCNQueue; rq->next; rq = rq->next)
	/* nothing */ ;
	rq->next = nrq;
    }
}

/* INTPROTO */
int
RemoveFromCNQueue(int width, int height)
{
    XCNQueue_t     *rq, *prq;

/*
 * If things are working properly we should only need to check the first one
 */
    for (rq = XCNQueue, prq = NULL; rq; rq = rq->next) {
	if (rq->width == width && rq->height == height) {
	/* unlink rq */
	    if (prq)
		prq->next = rq->next;
	    else
		XCNQueue = rq->next;
	    FREE(rq);
	    return 1;
	}
	prq = rq;
    }
    return 0;
}
/* ------------------------------------------------------------------------- */

/*{{{ Read and process output from the application */
/* EXTPROTO */
void
main_loop(void)
{
    int             nlines;
    unsigned char   ch, *str;

    for (;;) {
	while ((ch = cmd_getc()) == 0) ;	/* wait for something */

	if (ch >= ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
	/* Read a text string from the input buffer */
	/*
	 * point `str' to the start of the string,
	 * decrement first since it was post incremented in cmd_getc()
	 */
	    for (str = --cmdbuf_ptr, nlines = 0; cmdbuf_ptr < cmdbuf_endp;) {
		ch = *cmdbuf_ptr++;
		if (ch == '\n') {
		    nlines++;
		    if (++refresh_count >= (refresh_limit * (TermWin.nrow - 1)))
			break;
		} else if (ch < ' ' && ch != '\t' && ch != '\r') {
		/* unprintable */
		    cmdbuf_ptr--;
		    break;
		}
	    }
	    scr_add_lines(str, nlines, (cmdbuf_ptr - str));
	} else
	    switch (ch) {
	    case 005:		/* terminal Status */
		tt_printf(VT100_ANS);
		break;
	    case 007:		/* bell */
		scr_bell();
		break;
	    case '\b':		/* backspace */
		scr_backspace();
		break;
	    case 013:		/* vertical tab, form feed */
	    case 014:
		scr_index(UP);
		break;
	    case 016:		/* shift out - acs */
		scr_charset_choose(1);
		break;
	    case 017:		/* shift in - acs */
		scr_charset_choose(0);
		break;
	    case 033:		/* escape char */
		process_escape_seq();
		break;
	    }
    }
/* NOTREACHED */
}

/* ---------------------------------------------------------------------- */
/* Addresses pasting large amounts of data and rxvt hang
 * code pinched from xterm (v_write()) and applied originally to
 * rxvt-2.18 - Hops
 * Write data to the pty as typed by the user, pasted with the mouse,
 * or generated by us in response to a query ESC sequence.
 */
/* EXTPROTO */
void
tt_write(const unsigned char *d, int len)
{
    int             riten, p;

    if (v_bufstr == NULL && len > 0) {
	v_buffer = v_bufstr = v_bufptr = MALLOC(len);
	v_bufend = v_buffer + len;
    }
/*
 * Append to the block we already have.  Always doing this simplifies the
 * code, and isn't too bad, either.  If this is a short block, it isn't
 * too expensive, and if this is a long block, we won't be able to write
 * it all anyway.
 */
    if (len > 0) {
	if (v_bufend < v_bufptr + len) {	/* we've run out of room */
	    if (v_bufstr != v_buffer) {
	    /* there is unused space, move everything down */
	    /* possibly overlapping bcopy here */
	    /* bcopy(v_bufstr, v_buffer, v_bufptr - v_bufstr); */
		MEMCPY(v_buffer, v_bufstr, v_bufptr - v_bufstr);
		v_bufptr -= v_bufstr - v_buffer;
		v_bufstr = v_buffer;
	    }
	    if (v_bufend < v_bufptr + len) {
	    /* still won't fit: get more space */
	    /* Don't use XtRealloc because an error is not fatal. */
		int             size = v_bufptr - v_buffer;

	    /* save across realloc */
		v_buffer = REALLOC(v_buffer, size + len);
		if (v_buffer) {
		    v_bufstr = v_buffer;
		    v_bufptr = v_buffer + size;
		    v_bufend = v_bufptr + len;
		} else {
		/* no memory: ignore entire write request */
		    print_error("cannot allocate buffer space");
		    v_buffer = v_bufstr;	/* restore clobbered pointer */
		}
	    }
	}
	if (v_bufend >= v_bufptr + len) {	/* new stuff will fit */
	    MEMCPY(v_bufptr, d, len);	/* bcopy(d, v_bufptr, len); */
	    v_bufptr += len;
	}
    }
/*
 * Write out as much of the buffer as we can.
 * Be careful not to overflow the pty's input silo.
 * We are conservative here and only write a small amount at a time.
 *
 * If we can't push all the data into the pty yet, we expect write
 * to return a non-negative number less than the length requested
 * (if some data written) or -1 and set errno to EAGAIN,
 * EWOULDBLOCK, or EINTR (if no data written).
 *
 * (Not all systems do this, sigh, so the code is actually
 * a little more forgiving.)
 */

#define MAX_PTY_WRITE 128	/* 1/2 POSIX minimum MAX_INPUT */

    if ((p = v_bufptr - v_bufstr) > 0) {
	riten = write(cmd_fd, v_bufstr, p < MAX_PTY_WRITE ? p : MAX_PTY_WRITE);
	if (riten < 0)
	    riten = 0;
	v_bufstr += riten;
	if (v_bufstr >= v_bufptr)	/* we wrote it all */
	    v_bufstr = v_bufptr = v_buffer;
    }
/*
 * If we have lots of unused memory allocated, return it
 */
    if (v_bufend - v_bufptr > 1024) {	/* arbitrary hysteresis */
    /* save pointers across realloc */
	int             start = v_bufstr - v_buffer;
	int             size = v_bufptr - v_buffer;
	int             allocsize = size ? size : 1;

	v_buffer = REALLOC(v_buffer, allocsize);
	if (v_buffer) {
	    v_bufstr = v_buffer + start;
	    v_bufptr = v_buffer + size;
	    v_bufend = v_buffer + allocsize;
	} else {
	/* should we print a warning if couldn't return memory? */
	    v_buffer = v_bufstr - start;	/* restore clobbered pointer */
	}
    }
}

#ifdef USE_XIM
/* INTPROTO */
void
setSize(XRectangle * size)
{
    size->x = TermWin.int_bwidth;
    size->y = TermWin.int_bwidth;
    size->width = Width2Pixel(TermWin.ncol);
    size->height = Height2Pixel(TermWin.nrow);
}

/* INTPROTO */
void
setColor(unsigned long *fg, unsigned long *bg)
{
    *fg = PixColors[Color_fg];
    *bg = PixColors[Color_bg];
}

/* Checking whether input method is running. */
/* INTPROTO */
Bool
IMisRunning(void)
{
    char           *p;
    Atom            atom;
    Window          win;
    char            server[IMBUFSIZ];

    /* get current locale modifier */
    if ((p = XSetLocaleModifiers(NULL)) != NULL) {
	STRCPY(server, "@server=");
	STRNCAT(server, &(p[4]), IMBUFSIZ - 9);	/* skip "@im=" */

	atom = XInternAtom(Xdisplay, server, False);
	win = XGetSelectionOwner(Xdisplay, atom);
	if (win != None)
	    return True;
    }
    return False;
}

/* INTPROTO */
void
IMSendSpot(void)
{
    XPoint          spot;
    XVaNestedList   preedit_attr;

    if (Input_Context == NULL 
	|| !TermWin.focus
	|| !(input_style & XIMPreeditPosition)
	|| !(event_type == KeyPress
	     || event_type == SelectionNotify
	     || event_type == ButtonRelease
	     || event_type == FocusIn)
	|| !IMisRunning())
        return;

    setPosition(&spot);

    preedit_attr = XVaCreateNestedList(0, XNSpotLocation, &spot, NULL);
    XSetICValues(Input_Context, XNPreeditAttributes, preedit_attr, NULL);
    XFree(preedit_attr);
}

/* INTPROTO */
void
setTermFontSet(void)
{
    char           *string;
    long            length, i;

    D_CMD((stderr, "setTermFontSet()"));
    if (TermWin.fontset != NULL) {
	XFreeFontSet(Xdisplay, TermWin.fontset);
	TermWin.fontset = NULL;
    }
    length = 0;
    for (i = 0; i < NFONTS; i++) {
	if (rs.font[i])
	    length += STRLEN(rs.font[i]) + 1;
# ifdef MULTICHAR_SET
	if (rs.mfont[i])
	    length += STRLEN(rs.mfont[i]) + 1;
# endif
    }
    if (length == 0
	|| (string = MALLOC(length + 1)) == NULL)
	TermWin.fontset = NULL;
    else {
	int             missing_charsetcount;
	char          **missing_charsetlist, *def_string;

	string[0] = '\0';
	for (i = 0; i < NFONTS; i++) {
	    if (rs.font[i]) {
		STRCAT(string, rs.font[i]);
		STRCAT(string, ",");
	    }
# ifdef MULTICHAR_SET
	    if (rs.mfont[i]) {
		STRCAT(string, rs.mfont[i]);
		STRCAT(string, ",");
	    }
# endif
	}
	string[STRLEN(string) - 1] = '\0';
	TermWin.fontset = XCreateFontSet(Xdisplay, string,
					 &missing_charsetlist,
					 &missing_charsetcount,
					 &def_string);
	FREE(string);
    }
}

/* INTPROTO */
void
setPreeditArea(XRectangle * preedit_rect, XRectangle * status_rect, XRectangle * needed_rect)
{
    preedit_rect->x = needed_rect->width
		      + (scrollbar_visible() && !(Options & Opt_scrollBar_right)
			 ? (SB_WIDTH + sb_shadow * 2) : 0);
    preedit_rect->y = Height2Pixel(TermWin.nrow - 1)
		      + ((menuBar.state == 1) ? menuBar_TotalHeight() : 0);

    preedit_rect->width = Width2Pixel(TermWin.ncol + 1) - needed_rect->width
			  + (!(Options & Opt_scrollBar_right)
			     ? (SB_WIDTH + sb_shadow * 2) : 0);
    preedit_rect->height = Height2Pixel(1);

    status_rect->x = (scrollbar_visible() && !(Options & Opt_scrollBar_right))
		      ? (SB_WIDTH + sb_shadow * 2) : 0;
    status_rect->y = Height2Pixel(TermWin.nrow - 1)
		     + ((menuBar.state == 1) ? menuBar_TotalHeight() : 0);

    status_rect->width = needed_rect->width ? needed_rect->width
					    : Width2Pixel(TermWin.ncol + 1);
    status_rect->height = Height2Pixel(1);
}

/* INTPROTO */
void
IMDestroyCallback(XIM xim, XPointer client_data, XPointer call_data)
{
    Input_Context = NULL;
    XRegisterIMInstantiateCallback(Xdisplay, NULL, NULL, NULL,
				   IMInstantiateCallback, NULL);
}

/* INTPROTO */
void
IMInstantiateCallback(Display * display, XPointer client_data, XPointer call_data)
{
    char           *p, *s, buf[IMBUFSIZ], tmp[1024];
    char           *end, *next_s;
    XIM             xim = NULL;
    XIMStyles      *xim_styles = NULL;
    int             found;
    XPoint          spot;
    XRectangle      rect, status_rect, needed_rect;
    unsigned long   fg, bg;
    XVaNestedList   preedit_attr = NULL;
    XVaNestedList   status_attr = NULL;
    XIMCallback     ximcallback;

    if (Input_Context)
	return;

    ximcallback.callback = IMDestroyCallback;
    ximcallback.client_data = NULL;

    if (rs.inputMethod && *(rs.inputMethod)) {
	STRNCPY(tmp, rs.inputMethod, sizeof(tmp) - 1);
	for (s = tmp; *s; s = next_s + 1) {
	    for (; *s && isspace(*s); s++) ;
	    if (!*s)
		break;
	    for (end = s; (*end && (*end != ',')); end++) ;
	    for (next_s = end--; ((end >= s) && isspace(*end)); end--) ;
	    *(end + 1) = '\0';

	    if (*s) {
		STRCPY(buf, "@im=");
		STRNCAT(buf, s, IMBUFSIZ - 5);
		if ((p = XSetLocaleModifiers(buf)) != NULL && *p
		    && (xim = XOpenIM(Xdisplay, NULL, NULL, NULL)) != NULL)
		    break;
	    }
	    if (!*next_s)
		break;
	}
    }
/* try with XMODIFIERS env. var. */
    if (xim == NULL && (p = XSetLocaleModifiers("")) != NULL && *p)
	xim = XOpenIM(Xdisplay, NULL, NULL, NULL);

/* try with no modifiers base */
    if (xim == NULL && (p = XSetLocaleModifiers("@im=none")) != NULL && *p)
	xim = XOpenIM(Xdisplay, NULL, NULL, NULL);

    if (xim == NULL)
	return;
    XSetIMValues(xim, XNDestroyCallback, &ximcallback, NULL);

    if (XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL)
	|| !xim_styles) {
	print_error("input method doesn't support any style");
	XCloseIM(xim);
	return;
    }
    STRNCPY(tmp, (rs.preeditType ? rs.preeditType
				 : "OverTheSpot,OffTheSpot,Root"),
	    sizeof(tmp) - 1);
    for (found = 0, s = tmp; *s && !found; s = next_s + 1) {
	unsigned short  i;

	for (; *s && isspace(*s); s++) ;
	if (!*s)
	    break;
	for (end = s; (*end && (*end != ',')); end++) ;
	for (next_s = end--; ((end >= s) && isspace(*end)); end--) ;
	*(end + 1) = '\0';

	if (!STRCMP(s, "OverTheSpot"))
	    input_style = (XIMPreeditPosition | XIMStatusNothing);
	else if (!STRCMP(s, "OffTheSpot"))
	    input_style = (XIMPreeditArea | XIMStatusArea);
	else if (!STRCMP(s, "Root"))
	    input_style = (XIMPreeditNothing | XIMStatusNothing);

	for (i = 0; i < xim_styles->count_styles; i++)
	    if (input_style == xim_styles->supported_styles[i]) {
		found = 1;
		break;
	    }
    }
    XFree(xim_styles);

    if (found == 0) {
	print_error("input method doesn't support my preedit type");
	XCloseIM(xim);
	return;
    }
    if ((input_style != (XIMPreeditNothing | XIMStatusNothing))
	&& (input_style != (XIMPreeditArea | XIMStatusArea))
	&& (input_style != (XIMPreeditPosition | XIMStatusNothing))) {
	print_error("This program does not support the preedit type");
	XCloseIM(xim);
	return;
    }
    if (input_style & XIMPreeditPosition) {
	setSize(&rect);
	setPosition(&spot);
	setColor(&fg, &bg);

	preedit_attr = XVaCreateNestedList(0, XNArea, &rect,
					   XNSpotLocation, &spot,
					   XNForeground, fg,
					   XNBackground, bg,
					   XNFontSet, TermWin.fontset,
					   NULL);
    } else if (input_style & XIMPreeditArea) {
	setColor(&fg, &bg);

    /*
     * The necessary width of preedit area is unknown
     * until create input context.
     */
	needed_rect.width = 0;

	setPreeditArea(&rect, &status_rect, &needed_rect);

	preedit_attr = XVaCreateNestedList(0, XNArea, &rect,
					   XNForeground, fg,
					   XNBackground, bg,
					   XNFontSet, TermWin.fontset,
					   NULL);
	status_attr = XVaCreateNestedList(0, XNArea, &status_rect,
					  XNForeground, fg,
					  XNBackground, bg,
					  XNFontSet, TermWin.fontset,
					  NULL);
    }
    Input_Context = XCreateIC(xim, XNInputStyle, input_style,
			      XNClientWindow, TermWin.parent[0],
			      XNFocusWindow, TermWin.parent[0],
			      XNDestroyCallback, &ximcallback,
			      preedit_attr ? XNPreeditAttributes : NULL,
			      preedit_attr,
			      status_attr ? XNStatusAttributes : NULL,
			      status_attr,
			      NULL);
    XFree(preedit_attr);
    XFree(status_attr);
    if (Input_Context == NULL) {
	print_error("Failed to create input context");
	XCloseIM(xim);
    }
    if (input_style & XIMPreeditArea)
	IMSetStatusPosition();
}

/* EXTPROTO */
void
IMSetStatusPosition(void)
{
    XRectangle      preedit_rect, status_rect, *needed_rect;
    XVaNestedList   preedit_attr, status_attr;

    if (Input_Context == NULL 
	|| !TermWin.focus
	|| !(input_style & XIMPreeditArea)
	|| !IMisRunning())
        return;

    /* Getting the necessary width of preedit area */
    status_attr = XVaCreateNestedList(0, XNAreaNeeded, &needed_rect, NULL);
    XGetICValues(Input_Context, XNStatusAttributes, status_attr, NULL);
    XFree(status_attr);

    setPreeditArea(&preedit_rect, &status_rect, needed_rect);

    preedit_attr = XVaCreateNestedList(0, XNArea, &preedit_rect, NULL);
    status_attr = XVaCreateNestedList(0, XNArea, &status_rect, NULL);

    XSetICValues(Input_Context,
		 XNPreeditAttributes, preedit_attr,
		 XNStatusAttributes, status_attr, NULL);

    XFree(preedit_attr);
    XFree(status_attr);
}
#endif				/* USE_XIM */

/* INTPROTO */
void
XProcessEvent(Display *display)
{
    XEvent          xev;

    XNextEvent(display, &xev);
#ifdef USE_XIM
    if (!XFilterEvent(&xev, xev.xany.window))
	process_x_event(&xev);
    event_type = xev.type;
#else
    process_x_event(&xev);
#endif
    return;
}

/*}}} */
/*----------------------- end-of-file (C source) -----------------------*/
