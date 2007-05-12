#ifndef GCMSGS_H
/*
 * gcmsgs.h
 *
 * (`* !$' leaders, in the following, *must* be preserved).
 *
 * !$ Message definitions for MinGW implementation of `gencat'.
 * !$
 * !$ $Id: gcmsgs.h,v 1.3 2007-05-12 22:51:10 keithmarshall Exp $
 * !$ Copyright (C) 2006, 2007, MinGW Project
 * !$ Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * !$
 * !$ This file has been automatically generated from `gcmsgs.h',
 * !$ by `gendefs.awk'; edit by hand, only to provide a translation.
 * !$
 * !$ This is free software.  It is distributed AS IS, in the hope that
 * !$ it may be useful, but WITHOUT WARRANTY OF ANY KIND, not even an
 * !$ implied warranty of merchantibility, nor fitness for any purpose.
 * !$ It may be redistributed as is, or in modified form, under the
 * !$ terms of the GNU General Public License, as published by the
 * !$ Free Software Foundation; either Version 2, or at your option,
 * !$ any later version.
 * !$
 * !$ You should have received a copy of the GNU General Public License
 * !$ with this software; see the file `COPYING'.  If you did not, please
 * !$ write to the Free Software Foundation, 51 Franklin St - Fifth Floor,
 * !$ Boston, MA 02110-1301, USA.
 */
#define GCMSGS_H
/*
 * Here is some boiler plate code, to emit to the gencat source stream,
 * so that this message definition stream will be correctly interpreted.
 * !$
 * !$ codeset=ASCII
 * !$
 * !$quote "
 * !
 */
#define MSG_MISSING_ARGS     1, 1, "%s: incorrect number of arguments\n"
#define MSG_GENCAT_USAGE     1, 2, "usage: %s catalogue-name input-file ...\n"
#define MSG_OUTPUT_NOFILE    1, 3, "%s: %s: cannot create temporary output file\n"
#define MSG_OUT_OF_MEMORY    1, 4, "%s: out of memory\n"
#define MSG_INTERNAL_ERROR   1, 5, "%s: internal error: %s\n"
#define MSG_BAD_CATALOGUE    2, 1, "%s: %s: file is not a valid message catalogue\n"
#define MSG_UNKNOWN_CODESET  2, 2, "%s: %s: unknown codeset descriptor\n"
#define MSG_CODESET_CLASH    2, 3, "%s:%u: codeset `%s' conflicts with prior declaration\n"
#define MSG_HAD_CODESET      2, 4, "%s:%u: codeset `%s' was previously declared here\n"
#define MSG_SETNUM_NOT_INCR  2, 5, "invalid set number: expecting > %d; got %d\n"
#define MSG_MSGNUM_NOT_INCR  2, 6, "invalid message number: expecting > %d; got %d\n"
#define MSG_REDEFINED        2, 7, "%s: %s:%u: redefinition of message %u in set %u\n"
#define MSG_PREVIOUS_HERE    2, 8, "%s: %s:%u: previous definition was here\n"
#define MSG_EOF_IN_QUOTES    3, 2, "%s:%u: unexpected EOF encountered before closing quote\n"
#define MSG_TEXT_DISCARDED   3, 3, "%s:%u: incomplete message marked for deletion\n"
#define MSG_MISSING_NEWLINE  3, 4, "%s:%u: missing newline at end of file\n"
#define MSG_BAD_INDEX        3, 5, "invalid reference in message index"
/* !
 * !$ end of file
 */
#endif /* !defined( GCMSGS_H ): $RCSfile: gcmsgs.h,v $Revision: 1.3 $: end of file */
