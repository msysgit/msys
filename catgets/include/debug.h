#ifndef DEBUG_H
/*
 * debug.h
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Last modification: 02-Apr-2007
 *
 * THIS SOFTWARE IS NOT COPYRIGHTED
 *
 * This source code is offered for use in the public domain. You may
 * use, modify or distribute it freely.
 *
 * This code is distributed in the hope that it will be useful but
 * WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 * DISCLAIMED. This includes but is not limited to warranties of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#define DEBUG_H

#ifdef DEBUG
/*
 * We need some macros to facilitate printing debug messages...
 */
# define dinvoke(x)  x
# define dfprintf(x) fprintf x
# define dfputc(x)   fputc x
# define DCODEFMT    "<escape: %#4.4x>"
#else
/*
 * ...or, to do nothing, if not building for debugging
 */
# define dinvoke(x)
# define dfprintf(x)
# define dfputc(x)
#endif

#endif /* !defined( DEBUG_H ): $RCSfile: debug.h,v $Revision: 1.1 $: end of file */
