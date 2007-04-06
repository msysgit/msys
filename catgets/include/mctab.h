#ifndef _MCTAB_H_
/*
 * mctab.h
 *
 * $Id: mctab.h,v 1.1 2007-04-06 22:34:56 keithmarshall Exp $
 *
 * Copyright (C) 2006, Keith Marshall
 *
 * This file defines the data structures which implement the internal
 * message catalogue descriptor table, as used by the MinGW `catgets'
 * function, and declares prototypes for the functions which are used
 * to access and manipulate it.
 *
 * Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
 * Last modification: 30-Dec-2006
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
#define _MCTAB_H_

#include <stdarg.h>
#include <nl_types.h>
#include <msgcat.h>

/* All implementation defined functions, which require access to
 * the message catalogue descriptor table, obtain this by invoking
 * the `_mctab_' function.  To achieve this, the accessor function
 * defines a call-back helper function, and passes this, with any
 * appropriate arguments to `_mctab_'; `_mctab_' then invokes the
 * call-back function, passing it a pointer to the descriptor
 * table, in addition to the specified arguments.
 *
 *
 * The message catalogue descriptor table itself, is implemented as
 * a dynamically allocated array of `mc_descriptor' structures, each
 * relating to a single message catalogue, and specifying:-
 *
 */
struct mc_descriptor
{
  int                   fd;     /* the catalogue's input file descriptor */
  MSGCAT               *data;   /* its `in memory' data representation   */
};

/* The `_mctab_' function provides catalogue descriptor table access,
 * for implementation defined accessor functions, by invoking an accessor
 * specified helper function, passing a pointer to an `mc_tab' structure
 * as the first argument.
 *
 */
struct mc_tab
{
  int                   curr_size;   /* number of `mc_descriptor' slots  */
  int                   grow_size;   /* slots to add, when expanding ... */
  struct mc_descriptor *tab;         /* the allocated descriptor table   */
};

extern void *_mctab_( void *function( struct mc_tab *, va_list ), ... );

extern MSGCAT * _mc_select_( nl_catd );
extern int      _mc_free_( struct mc_descriptor * );

#endif /* _MCTAB_H_: $RCSfile: mctab.h,v $Revision: 1.1 $: end of file */
