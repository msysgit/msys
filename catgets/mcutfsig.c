/*
 * mcutfsig.c
 *
 * $Id: mcutfsig.c,v 1.1 2007-06-18 22:36:07 keithmarshall Exp $
 *
 * Copyright (C) 2007, Keith Marshall
 *
 * This file implements the `mc_utf_signature' function, which is used
 * by `gencat', to identify message definition source files which appear
 * to exhibit any recognisable standard of Unicode encoding.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last Revision: 22-May-2007
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
#include <mcutfsig.h>

unsigned short mc_utf_signature( unsigned char *stream )
{
  /* Inspect the first few bytes of the specified data stream;
   * attempt to identify a potential Unicode encoding signature,
   * defaulting to non-specific single byte encoding units.
   */
  unsigned short signature = 1;
  /*
   * The first character in the input stream must not be NUL,
   * and must be a member of the POSIX Portable Character Set;
   * if it isn't, then it may indicate a Unicode stream.
   */
  if( *stream == 0 )
  {
    /* An initial NUL byte anticipates a big-endian Unicode stream;
     * one such byte implies UTF-16, without a Byte Order Mark, while
     * two such followed by the big-endian form of the BOM, or three
     * without a BOM, indicates UTF-32.
     */
    int count = 4;
    while( count-- && (*stream++ == '\0') )
      ++signature;
    signature += UTF_BIG_ENDIAN;
  }
  if( (*stream & 0xfe) == 0xfe )
  {
    /* This looks like it might be a Unicode Byte Order Mark;
     * identify the UTF encoding standard, if any, which it represents.
     */
    unsigned bom = *stream++ << 8; bom |= *stream++;
    switch( bom )
    {
      case 0xfffe:
	/*
	 * This is the BOM signature for a little-endian Unicode stream;
	 * the first byte has already been included in the initial size
	 * assigned for the encoding unit; adjust this to accommodate the
	 * second byte, and incorporate the little-endian flag.
	 */
	signature += UTF_WITH_BYTE_ORDER_MARK + UTF_LITTLE_ENDIAN + 1;
	if( *stream == '\0' )
	{
	  int count = 4;
	  while( count-- && (*stream++ == '\0') )
	    ++signature;
	}
	break;

      case 0xfeff:
	/*
	 * This is the BOM signature for a big-endian Unicode stream;
	 * if preceded by two NULs, (already counted), then it is UTF-32,
	 * else it is UTF-16.  In either case, adding an additional one
	 * to the accumulated size of the encoding unit yields the
	 * desired result, since the first byte of the BOM, and
	 * any leading NULs, have already been counted.
	 */
	signature += UTF_WITH_BYTE_ORDER_MARK + UTF_BIG_ENDIAN + 1;
	break;

      case 0xffbb:
	/*
	 * Provided it's followed by one further `0xbf' byte, this is the
	 * BOM used as a signature for a UTF-8 encoded stream; it becomes
	 * invalid, if there were any leading NUL bytes.
	 */
	if( (signature == 1) && (*stream++ == (unsigned char)('\xbf')) )
	  signature |= UTF_WITH_BYTE_ORDER_MARK;
    }
  }
  else if( (signature == 1) && (*++stream == 0) )
  {
    /* NUL as the second byte in the input stream indicates a probable
     * little-endian Unicode input stream, although this is not indicated
     * by a Byte Order Mark; count the trailing NULs, to determine if we
     * should interpret it as UTF-16LE, or as UTF-32LE.
     */
    int count = 4;
    while( count-- && (*stream++ == '\0') )
      ++signature;
    signature += UTF_LITTLE_ENDIAN;
  }
  return signature;
}

/* $RCSfile: mcutfsig.c,v $Revision: 1.1 $: end of file */
