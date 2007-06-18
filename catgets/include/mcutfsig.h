#ifndef _MCUTFSIG_H_
/*
 * mcutfsig.h
 *
 * $Id: mcutfsig.h,v 1.1 2007-06-18 22:36:08 keithmarshall Exp $
 *
 * Copyright (C) 2007, Keith Marshall
 *
 * Header file defining the `mc_utf_signature' function API, and a set
 * of supporting macros, used for obtaining and manipulating an encoding
 * `signature' for UTF-8, UTF-16 and UTF-32 encoded input files.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last Revision: 30-May-2007
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
#define _MCUTFSIG_H_

/*
 * Flags used to designate the `endianness' of the input encoding,
 * and to record presence or absence of a byte order mark.
 *
 */
#define UTF_BIG_ENDIAN            0100
#define UTF_LITTLE_ENDIAN         0200
#define UTF_WITH_BYTE_ORDER_MARK  0400

/*
 * Mask used to isolate a bit-field representing the number of bytes
 * per encoding unit in the input stream.
 *
 */
#define UTF_CODE_UNIT_SIZE_MASK   0007

/*
 * Macros used to disambiguate the codeset name, wrt byte order.
 *
 */
#define UTF_IS_MB(FLAGS)          (UTF_CODE_SIZE(FLAGS) > 1)
#define UTF_CODE_SIZE(FLAGS)      ((FLAGS) & UTF_CODE_UNIT_SIZE_MASK)
#define UTF_BYTE_ORDER(FLAGS)     (UTF_IS_MB(FLAGS) ? UTF_SUFFIX(FLAGS) : '\0')
#define UTF_SUFFIX(FLAGS)         (((FLAGS) & UTF_LITTLE_ENDIAN) ? 'L' : 'B')

/*
 * Function prototypes.
 *
 */
unsigned short mc_ucs_signature( unsigned char *stream );

#endif /* !defined(_MCUTFSIG_H_): $RCSfile: mcutfsig.h,v $Revision: 1.1 $: end of file */
