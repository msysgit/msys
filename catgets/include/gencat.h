#ifndef GENCAT_H
/*
 * gencat.h
 *
 * $Id: gencat.h,v 1.2 2007-05-12 16:54:36 keithmarshall Exp $
 *
 * Copyright (C) 2006, 2007, Keith Marshall
 *
 * This file defines the extended message catalogue dictionary structures,
 * used internally by the `gencat' message catalogue compiler, and declares
 * the prototypes for the functions used to implement `gencat'.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last modification: 12-May-2007
 *
 *
 * This is free software.  It is provided AS IS, in the hope that it may
 * be useful, but WITHOUT WARRANTY OF ANY KIND, not even an IMPLIED WARRANTY
 * of MERCHANTABILITY, nor of FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 * Permission is granted to redistribute this software, either "as is" or
 * in modified form, under the terms of the GNU General Public License, as
 * published by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to the
 * Free Software Foundation, 51 Franklin St - Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */
#define GENCAT_H

#include <sys/types.h>
#include <nl_types.h>

extern char *   progname;
extern nl_catd  gencat_messages;
extern int      gencat_errno;

struct msgdict
{
  /* Internal structure used by gencat, to keep track of messages as
   * they are added to the list for compilation into the catalogue.
   */
  const char         *src;      /* name of the gencat input file        */
  char               *base;     /* base address of the message buffer   */
  unsigned            lineno;   /* line number of message in input file */
  union
  {
    unsigned long     key;      /* composite set/message number         */
    struct 
    {
      unsigned short  msg;      /* message number component of `key'    */
      unsigned short  set;      /* set number for this message          */
    };
  };
  off_t               loc;      /* offset from `base' for this message  */
  unsigned int        len;      /* storage size of this message         */
  struct msgdict     *link;     /* pointer to next message in this list */
};

extern struct msgdict *mc_load( const char * );
extern struct msgdict *mc_source( const char * );
extern struct msgdict *mc_merge( struct msgdict *, struct msgdict * );

/*
 * To permit gencat to compile message catalogues for multibyte character
 * locales, which may not necessarily be installed on the host, `iconv' is
 * used to emulate the behaviour of `mbtowc' and `wctomb'.
 *
 */
#include <iconv.h>

/*
 * MSVCRT appears to define MB_LEN_MAX as two, but the character codesets
 * supported by `libiconv' would seem to expect a larger value; to be safe,
 * we use this alternative definition, derived from GNU/Linux `limits.h'.
 * 
 */
#define ICONV_MB_LEN_MAX     16

#define iconv_mode(N)        (N), iconv_map[(N)]
#define iconv_char(S)        (S), ICONV_MB_LEN_MAX
#define iconv_wchar(W)       (char *)(W), sizeof( wchar_t )
#define iconv_mbtowc(W,S,L)  iconv_wrap( iconv_mode(0), (S), (L), iconv_wchar(W) )
#define iconv_wctomb(S,W)    iconv_wrap( iconv_mode(1), iconv_wchar(&(W)), iconv_char(S) )

extern char *map_codeset( iconv_t *, char *, char * );
extern size_t iconv_wrap( int, iconv_t, char *, size_t, char *, size_t );

#define msgarg(MSG) catgets( gencat_messages, MSG )
#define errmsg(MSG) stderr, catgets( gencat_messages, MSG )
#define FATAL(MSG) input, linenum, catgets( gencat_messages, MSG )

#define GENCAT_MSG_SRC(REF)  progname, (REF)->src, (REF)->lineno
#define GENCAT_MSG_INPUT     GENCAT_MSG_SRC( input )

#endif /* !defined( GENCAT_H ): $RCSfile: gencat.h,v $Revision: 1.2 $: end of file */
