#ifndef GCMSGS_H
/*
 * gcmsgs.h
 *
 * (`* !$' leaders, in the following, *must* be preserved).
 *
 * !$ Message definitions for MinGW implementation of `gencat'.
 * !$
 * !$ $Id: gcmsgs.h,v 1.5 2008-01-08 19:51:58 keithmarshall Exp $
 * !$ Copyright (C) 2006, 2007, 2008, MinGW Project
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
 * !$ When translating these message definitions, please take care
 * !$ to preserve the number and order of `printf' format tags; (i.e.
 * !$ tags of the form `%c', where `c' is any alphabetic character).
 * !$ Also please note the usage of `\n' to insert a line break into
 * !$ any message definition, and the use of a `\` following the
 * !$ closing quote, on all but the last line of a multiline message
 * !$ definition; there must be no white space between the quote and
 * !$ the following `\', nor following the `\' itself.
 * !
 * !
 * !$ Message set #1 is reserved for the program banner, which is
 * !$ displayed when either the `-help' or the `-version' option is
 * !$ invoked; it is discretional whether a translation of this, in
 * !$ the national language, is provided.  If not, then set #1 may be
 * !$ excluded from the message catalogue, or the banner template
 * !$ may be left untranslated, in which case the banner will be
 * !$ displayed in its original English language form.
 * !
 */
#define MSG_PROGRAM_BANNER   1, 1, \
"%s (MinGW catgets SubSystem for MS-Windows), Version %s\n"\
"Written by %s\n"
/* 
 * !
 * !$ Messages in set #2 are reserved for use by the `-version' option,
 * !$ and are displayed *both* in English *and* in the national language.
 * !$ While it is not normally necessary to provide an English language
 * !$ message catalogue, if one is provided, it should *exclude* all of
 * !$ the messages defined in set #2, to avoid displaying identical
 * !$ copies of these messages twice.
 * !
 */
#define MSG_COPYRIGHT_NOTICE 2, 3, "\nCopyright (C) %s, %s.\n"\
"This is free software; see the source for copying conditions.  There is NO\n"\
"WARRANTY OF ANY KIND, not even an IMPLIED WARRANTY OF MERCHANTABILITY, nor\n"\
"of FITNESS FOR ANY PARTICULAR PURPOSE.\n"
/*
 * !
 * !$ Set #3 is reserved for the synopsis displayed by the `-help' option;
 * !$ in general, a national language version of this should be provided.
 * !
 */
#define MSG_GENCAT_SYNOPSIS  3, 2, \
"\nUsage: %s catalogue-name input-file ...\n"\
"\nDescription\n"\
"  Generate, or update, the national language message catalogue specified\n"\
"  by `catalogue-name', (which is a binary file, in the format required by\n"\
"  the MinGW implementation of the POSIX `catgets' facility), reading the\n"\
"  message definitions from each specified `input-file' in turn.\n"\
"\nOptions\n"\
"  In normal use, there are no options.  However, for compatibility with\n"\
"  GNU standard utilities, the following are accepted:--\n\n"\
"    -help, --help\n"\
"      Display this program synopsis, and exit.\n\n"\
"    -version, --version\n"\
"      Display program identification, and copyright notice,\n"\
"      then exit.\n\n"
/*
 * !
 * !$ Set #4 specifies messages relating to `gencat' program invocation,
 * !$ or to runtime problems; they should always be translated to the
 * !$ applicable national language.
 * !
 */
#define MSG_MISSING_ARGS     4, 1, "%s: incorrect number of arguments\n"
#define MSG_GENCAT_USAGE     4, 2, "usage: %s catalogue-name input-file ...\n"
#define MSG_OUTPUT_NOFILE    4, 3, "%s: %s: cannot create temporary output file\n"
#define MSG_OUT_OF_MEMORY    4, 4, "%s: out of memory\n"
#define MSG_INTERNAL_ERROR   4, 5, "%s: internal error: %s\n"
/*
 * !
 * !$ Set #5 defines messages relating to problems in interpretation of
 * !$ the content, or format, of the input files, or any pre-existing
 * !$ version of the message catalogue which is being updated; national
 * !$ language translations should be provided.
 * !
 */
#define MSG_BAD_CATALOGUE    5, 1, "%s: %s: file is not a valid message catalogue\n"
#define MSG_UNKNOWN_CODESET  5, 2, "%s: %s: unknown codeset descriptor\n"
#define MSG_CODESET_CLASH    5, 3, "%s:%u: codeset `%s' conflicts with prior declaration\n"
#define MSG_HAD_CODESET      5, 4, "%s:%u: codeset `%s' previously declared here\n"
#define MSG_SETNUM_NOT_INCR  5, 5, "invalid set number: expecting > %d; got %d\n"
#define MSG_MSGNUM_NOT_INCR  5, 6, "invalid message number: expecting > %d; got %d\n"
#define MSG_REDEFINED        5, 7, "%s: %s:%u: redefinition of message %u in set %u\n"
#define MSG_PREVIOUS_HERE    5, 8, "%s: %s:%u: previous definition was here\n"
/*
 * !
 * !$ Set #6 defines messages relating to abnormal conditions occuring
 * !$ when the end of an input file is encountered; again, national
 * !$ language translations should be provided.
 * !
 */
#define MSG_EOF_IN_QUOTES    6, 2, "%s:%u: unexpected EOF encountered before closing quote\n"
#define MSG_TEXT_DISCARDED   6, 3, "%s:%u: incomplete message marked for deletion\n"
#define MSG_MISSING_NEWLINE  6, 4, "%s:%u: missing newline at end of file\n"
#define MSG_BAD_INDEX        6, 5, "invalid reference in message index"
/*
 * !
 * !$ Set #7 defines messages relating to malformed Unicode input streams;
 * !$ once again, national language translations should be provided.
 * !
 */
#define MSG_UTF_UNKNOWN      7, 1, "%s:unrecognisable encoding format\n"
#define MSG_UTF_SIZE_ERROR   7, 2, "%s:invalid byte count per code point; value was %d\n"
#define MSG_UTF_FRAME_ERROR  7, 3, "%s:%u:UTF-%u%cE input framing error\n"
#define MSG_UTF_CODESET      7, 4, "%s:input codeset identified as %s; conflicts with ...\n"
/* !
 * !$ end of file
 */
#endif /* !defined( GCMSGS_H ): $RCSfile: gcmsgs.h,v $Revision: 1.5 $: end of file */
