/*--------------------------------*-C-*---------------------------------*
 * File:	logging.c
 *----------------------------------------------------------------------*
 * $Id: logging.c,v 1.1 2002/12/06 23:08:02 earnie Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (C) 1992      John Bovey <jdb@ukc.ac.uk>
 *				- original version
 * Copyright (C) 1993      lipka
 * Copyright (C) 1993      Brian Stempien <stempien@cs.wmich.edu>
 * Copyright (C) 1995      Raul Garcia Garcia <rgg@tid.es>
 * Copyright (C) 1995      Piet W. Plomp <piet@idefix.icce.rug.nl>
 * Copyright (C) 1997      Raul Garcia Garcia <rgg@tid.es>
 * Copyright (C) 1997,1999 Geoff Wing <gcw@pobox.com>
 * Copyright (C) 1999      D J Hawkey Jr <hawkeyd@visi.com>
 *				- lastlog support
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
 * Public:
 *	extern void cleanutent (void);
 *	extern void makeutent (const char * pty, const char * hostname);
 *
 * Private:
 *	rxvt_update_wtmp ();
 *----------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "logging.h"
#ifdef UTMP_SUPPORT

/*
 * HAVE_SETUTENT corresponds to SYSV-style utmp support.
 * Without it corresponds to using BSD utmp support.
 * Exception: QNX has setutent(), yet is a mix of BSD and SYSV.
 * SYSV-style utmp support is further divided in normal utmp support
 * and utmpx support (Solaris 2.x) by RXVT_UTMP_AS_UTMPX
 */

/*
 * BSD style utmp entry
 *      ut_line, ut_name, ut_host, ut_time
 * SYSV style utmp entry
 *      ut_user, ut_id, ut_line, ut_pid, ut_type, ut_exit, ut_time
 */
static UTMP     ut;
static int      utmp_pos = 0;

#ifdef USE_SYSV_UTMP
static char     ut_id[5];
#endif

/*
 * update wtmp entries
 */
/* ------------------------------------------------------------------------- */
#if defined(WTMP_SUPPORT) && !defined(RXVT_UTMP_AS_UTMPX)
/* INTPROTO */
void
rxvt_update_wtmp(const char *fname, const UTMP *putmp)
{
    int             fd, gotlock, retry;
    struct flock    lck;	/* fcntl locking scheme */
    struct stat     sbuf;

    if ((fd = open(fname, O_WRONLY | O_APPEND, 0)) < 0)
	return;

    lck.l_whence = SEEK_END;	/* start lock at current eof */
    lck.l_len = 0;		/* end at ``largest possible eof'' */
    lck.l_start = 0;
    lck.l_type = F_WRLCK;	/* we want a write lock */

    /* attempt lock with F_SETLK; F_SETLKW would cause a deadlock! */
    for (retry = 10, gotlock = 0; retry--;)
	if (fcntl(fd, F_SETLK, &lck) != -1) {
	    gotlock = 1;
	    break;
	} else if (errno != EAGAIN && errno != EACCES)
	    break;
    if (!gotlock) {		/* give it up */
	close(fd);
	return;
    }
    if (fstat(fd, &sbuf) == 0)
	if (write(fd, putmp, sizeof(UTMP)) != sizeof(UTMP))
	    ftruncate(fd, sbuf.st_size);	/* remove bad writes */

    lck.l_type = F_UNLCK;	/* unlocking the file */
    fcntl(fd, F_SETLK, &lck);
    close(fd);
}
#endif
/* ------------------------------------------------------------------------- */

#ifndef USE_SYSV_UTMP
/* INTPROTO */
int
write_bsd_utmp(UTMP wu)
{
    int             fd;

    if (utmp_pos > 0 && (fd = open(RXVT_REAL_UTMP_FILE, O_WRONLY)) != -1) {
	lseek(fd, (off_t) (utmp_pos * sizeof(UTMP)), SEEK_SET);
	write(fd, &wu, sizeof(UTMP));
	close(fd);
	return 1;
    }
    return 0;
}
#endif
/* ------------------------------------------------------------------------- */

#ifdef LASTLOG_SUPPORT
/* INTPROTO */
void
rxvt_update_lastlog(const char *fname, const char *pty, const char *host)
{
# ifdef HAVE_STRUCT_LASTLOG
    int             fd;
    struct passwd  *pwent;
    struct lastlog  ll;
#  ifdef LASTLOG_IS_DIR
    char            lastlogfile[256];
#  endif

    pwent = getpwuid(getuid());
    if (!pwent) {
	print_error("no entry in password file");
	return;
    }
    MEMSET(&ll, 0, sizeof(ll));
    ll.ll_time = time(NULL);
    STRNCPY(ll.ll_line, pty, sizeof(ll.ll_line));
    STRNCPY(ll.ll_host, host, sizeof(ll.ll_host));
#  ifdef LASTLOG_IS_DIR
    sprintf(lastlogfile, "%.*s/%.*s",
	    sizeof(lastlogfile) - sizeof(pwent->pw_name) - 2, fname,
	    sizeof(pwent->pw_name),
	    (!pwent->pw_name || pwent->pw_name[0] == '\0') ? "unknown"
							   : pwent->pw_name);
    if ((fd = open(lastlogfile, O_WRONLY | O_CREAT, 0644)) >= 0) {
	write(fd, &ll, sizeof(ll));
	close(fd);
    }
#  else
    if ((fd = open(fname, O_RDWR)) != -1) {
	lseek(fd, (off_t) ((long)pwent->pw_uid * sizeof(ll)), SEEK_SET);
	write(fd, &ll, sizeof(ll));
	close(fd);
    }
#  endif			/* LASTLOG_IS_DIR */
# endif				/* HAVE_STRUCT_LASTLOG */
}
#endif				/* LASTLOG_SUPPORT */
/* ------------------------------------------------------------------------- */

/*
 * make a utmp entry
 */
/* EXTPROTO */
void
makeutent(const char *pty, const char *hostname)
#ifndef USE_SYSV_UTMP
/* ------------------------------ BSD ------------------------------  */
{
    FILE           *fd0;
    struct passwd  *pwent = getpwuid(getuid());
    char            buf[256], name[256];
#ifdef __QNX__
    UTMP            u2;
    struct utsname  un;
#endif

/* BSD naming is of the form /dev/tty?? or /dev/pty?? */
    MEMSET(&ut, 0, sizeof(UTMP));

    if (!STRNCMP(pty, "/dev/", 5))
	pty += 5;		/* skip /dev/ prefix */
    if (STRNCMP(pty, "pty", 3) && STRNCMP(pty, "tty", 3)) {
	print_error("can't parse tty name \"%s\"", pty);
	return;
    }
#ifdef __QNX__
    uname(&un);
    STRNCPY(ut.id, un.nodename, sizeof(ut.ut_line));
    ut.ut_pid = getpid();
    ut.ut_type = USER_PROCESS;
#endif
    STRNCPY(ut.ut_line, pty, sizeof(ut.ut_line));
    STRNCPY(ut.ut_name, (pwent && pwent->pw_name) ? pwent->pw_name : "?",
	    sizeof(ut.ut_name));
    STRNCPY(ut.ut_host, hostname, sizeof(ut.ut_host));
    ut.ut_time = time(NULL);

    buf[sizeof(buf) - 1] = '\0';
#ifdef __QNX__
    for (;;) {
	utmp_pos = fseek(fd0, 0L, 1);
	if (fread(&u2, sizeof(UTMP), 1, fd0) != 1)
	    break;
	if ((u2.ut_type == LOGIN_PROCESS || u2.ut_type == USER_PROCESS)
	    && (STRNCMP(u2.ut_line, pty, sizeof(u2.ut_line)) == 0))
	    break;
    }
#else
    if ((fd0 = fopen(TTYTAB_FILENAME, "r")) != NULL) {
	int             i;

	for (i = 1; (fgets(buf, sizeof(buf) - 1, fd0) != NULL);) {
	    if (*buf == '#' || sscanf(buf, "%s", name) != 1)
		continue;
	    if (!STRCMP(ut.ut_line, name)) {
		utmp_pos = i;
		fclose(fd0);
		break;
	    }
	    i++;
	}
	fclose(fd0);
    }
#endif
    if (!write_bsd_utmp(ut))
	utmp_pos = 0;
#ifdef WTMP_SUPPORT
# ifdef WTMP_ONLY_ON_LOGIN
    if (Options & Opt_loginShell)
# endif
	update_wtmp(RXVT_WTMP_FILE, &ut);
#endif
#ifdef LASTLOG_SUPPORT
    if (Options & Opt_loginShell)
	rxvt_update_lastlog(RXVT_LASTLOG_FILE, pty, hostname);
#endif
}
#else
/* ------------------------------ SYSV ------------------------------  */
{
    int             i;
    char           *colon;
    struct passwd  *pwent = getpwuid(getuid());

    MEMSET(&ut, 0, sizeof(UTMP));

    if (!STRNCMP(pty, "/dev/", 5))
	pty += 5;		/* skip /dev/ prefix */
    if (!STRNCMP(pty, "pty", 3) || !STRNCMP(pty, "tty", 3))
	STRNCPY(ut.ut_id, (pty + 3), sizeof(ut.ut_id));
    else if (sscanf(pty, "pts/%d", &i) == 1)
	sprintf(ut.ut_id, "vt%02x", (i & 0xff));	/* sysv naming */
    else {
	print_error("can't parse tty name \"%s\"", pty);
	return;
    }

#if 0
    /* XXX: most likely unnecessary.  could be harmful */
    utmpname(RXVT_REAL_UTMP_FILE);
#endif
    STRNCPY(ut_id, ut.ut_id, sizeof(ut_id));

    setutent();			/* XXX: should be unnecessaray */

    ut.ut_type = DEAD_PROCESS;
    getutid(&ut);		/* position to entry in utmp file */

/* set up the new entry */
    ut.ut_type = USER_PROCESS;
#ifndef linux
    ut.ut_exit.e_exit = 2;
#endif
    STRNCPY(ut.ut_user, (pwent && pwent->pw_name) ? pwent->pw_name : "?",
	    sizeof(ut.ut_user));
/* ut_name is normally the same as ut_user, but .... */
    STRNCPY(ut.ut_name, (pwent && pwent->pw_name) ? pwent->pw_name : "?",
	    sizeof(ut.ut_name));
    STRNCPY(ut.ut_id, ut_id, sizeof(ut.ut_id));
    STRNCPY(ut.ut_line, pty, sizeof(ut.ut_line));

#if (defined(HAVE_UTMP_HOST) && ! defined(RXVT_UTMP_AS_UTMPX)) || (defined(HAVE_UTMPX_HOST) && defined(RXVT_UTMP_AS_UTMPX))
    STRNCPY(ut.ut_host, hostname, sizeof(ut.ut_host));
# ifndef linux
    if ((colon = STRRCHR(ut.ut_host, ':')) != NULL)
	*colon = '\0';
# endif
#endif

    ut.ut_pid = getpid();

#ifdef RXVT_UTMP_AS_UTMPX
    ut.ut_session = getsid(0);
    ut.ut_tv.tv_sec = time(NULL);
    ut.ut_tv.tv_usec = 0;
#else
    ut.ut_time = time(NULL);
#endif				/* HAVE_UTMPX_H */
    pututline(&ut);
    utmp_pos = 1;

#ifdef WTMP_SUPPORT
# ifdef WTMP_ONLY_ON_LOGIN
    if (Options & Opt_loginShell)
# endif
	update_wtmp(RXVT_WTMP_FILE, &ut);
#endif
#ifdef LASTLOG_SUPPORT
    if (Options & Opt_loginShell)
	rxvt_update_lastlog(RXVT_LASTLOG_FILE, pty, hostname);
#endif
    endutent();			/* close the file */
}
#endif				/* !USE_SYSV_UTMP */

/* ------------------------------------------------------------------------- */
/*
 * remove a utmp entry
 */
/* EXTPROTO */
void
cleanutent(void)
#ifndef USE_SYSV_UTMP
/* ------------------------------ BSD ------------------------------  */
{
#ifdef __QNX__
    UTMP            u2;

    MEMCPY(u2, ut, sizeof(UTMP));
#endif
#ifdef WTMP_SUPPORT
# ifdef WTMP_ONLY_ON_LOGIN
    if (Options & Opt_loginShell)
# endif
    {
	MEMSET(&ut.ut_name, 0, sizeof(ut.ut_name));
	MEMSET(&ut.ut_host, 0, sizeof(ut.ut_host));
	ut.ut_time = time(NULL);
	update_wtmp(RXVT_WTMP_FILE, &ut);
    }
#endif
    if (!utmp_pos)
	return;
#ifdef __QNX__
    u2.ut_type = DEAD_PROCESS;
    write_bsd_utmp(u2);
#else
    MEMSET(&ut, 0, sizeof(UTMP));
    write_bsd_utmp(ut);
#endif
}
#else				/* USE_SYSV_UTMP */
/* ------------------------------ SYSV ------------------------------  */
{
    UTMP           *putmp;

    if (!utmp_pos)
	return;

#if 0
    /* XXX: most likely unnecessary.  could be harmful */
    utmpname(RXVT_REAL_UTMP_FILE);
#endif
    MEMSET(&ut, 0, sizeof(UTMP));
    STRNCPY(ut.ut_id, ut_id, sizeof(ut.ut_id));
    ut.ut_type = USER_PROCESS;

    setutent();			/* XXX: should be unnecessaray */

    putmp = getutid(&ut);
    if (!putmp || putmp->ut_pid != getpid())
	return;

    putmp->ut_type = DEAD_PROCESS;

#ifdef RXVT_UTMP_AS_UTMPX
    putmp->ut_session = getsid(0);
    putmp->ut_tv.tv_sec = time(NULL);
    putmp->ut_tv.tv_usec = 0;
#else				/* HAVE_UTMPX_H */
    putmp->ut_time = time(NULL);
#endif				/* HAVE_UTMPX_H */
    pututline(putmp);

#ifdef WTMP_SUPPORT
# ifdef WTMP_ONLY_ON_LOGIN
    if (Options & Opt_loginShell)
# endif
	update_wtmp(RXVT_WTMP_FILE, putmp);
#endif

    endutent();
}
#endif				/* !USE_SYSV_UTMP */
#endif				/* UTMP_SUPPORT */
