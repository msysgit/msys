#ifndef PLATFORM_H
/*
 * platform.h
 *
 * $Id: platform.h,v 1.1 2007-04-06 22:34:56 keithmarshall Exp $
 *
 * Copyright (C) 2006, 2007, Keith Marshall
 *
 * This file defines a number of platform specific entities,
 * which are required by the MinGW specific `catgets' implementation,
 * supporting use of its `gencat' tool in a cross-hosted environment.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last modification: 27-Mar-2007
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
#define PLATFORM_H

#ifndef O_BINARY
/*
 * On any native MinGW platform, this will be defined already,
 * but if we are compiling a cross tool chain, so we can invoke
 * gencat on a non-MS platform, to create MinGW message catalogues,
 * then it probably isn't; we need to define it, so we can be sure
 * that the message catalogue is properly opened as a binary file
 * on every platform, regardless of any text/binary distinction.
 */
# define O_BINARY   0
#endif

#if defined( LC_CTYPE ) && !defined( LC_MESSAGES )
/*
 * On Win32 hosts...
 * If `locale.h' didn't recognise `MINGW32_LC_EXTENSIONS',
 * then we probably don't know how to handle `LC_MESSAGES',
 * so use `LC_CTYPE' as fall back.
 */
# define LC_MESSAGES LC_CTYPE
#endif

#ifdef _WIN32
/*
 * Aarrrgh!  Microsoft's malloc/realloc implementation doesn't bother
 * to set `errno' on failure; we will use this wrapper, to fix it.
 *
 */
# define mc_malloc( SIZE ) mc_realloc( NULL, (SIZE) )

/* With this define, a single wrapper function serves for both `malloc'
 * and `realloc'.
 *
 */
extern void *mc_realloc( void *, size_t );

#else
/*
 * On non-Microsoft platforms, or even on MS-Windows with Cygwin,
 * just assume that `malloc' and `realloc' do the `Right Thing(TM)'.
 *
 */
# define mc_malloc malloc
# define mc_realloc realloc
#endif

#endif  /* !defined( PLATFORM_H ): $RCSfile: platform.h,v $Revision: 1.1 $: end of file */
