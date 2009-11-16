#ifndef PKGTASK_H
/*
 * pkgtask.h
 *
 * $Id: pkgtask.h,v 1.1 2009-11-16 21:54:30 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * This header provides manifest definitions for the action codes,
 * which are used by the installer engine's task scheduler.
 *
 *
 * This is free software.  Permission is granted to copy, modify and
 * redistribute this software, under the provisions of the GNU General
 * Public License, Version 3, (or, at your option, any later version),
 * as published by the Free Software Foundation; see the file COPYING
 * for licensing details.
 *
 * Note, in particular, that this software is provided "as is", in the
 * hope that it may prove useful, but WITHOUT WARRANTY OF ANY KIND; not
 * even an implied WARRANTY OF MERCHANTABILITY, nor of FITNESS FOR ANY
 * PARTICULAR PURPOSE.  Under no circumstances will the author, or the
 * MinGW Project, accept liability for any damages, however caused,
 * arising from the use of this software.
 *
 */
#define PKGTASK_H  1

enum
{
  action_none = 0,
  action_remove,
  action_install,
  action_upgrade,

  end_of_actions
};

#define ACTION_MASK	0x0F

#define ACTION_NONE     (unsigned long)(action_none)
#define ACTION_REMOVE   (unsigned long)(action_remove)
#define ACTION_INSTALL  (unsigned long)(action_install)
#define ACTION_UPGRADE  (unsigned long)(action_upgrade)

#define STRICTLY_GT	ACTION_MASK + 1
#define STRICTLY_LT	STRICTLY_GT << 1

#ifndef EXTERN_C
# ifdef __cplusplus
#  define EXTERN_C extern "C"
# else
#  define EXTERN_C
# endif
#endif

EXTERN_C const char *action_name( unsigned long );
EXTERN_C int action_code( const char* );

#endif /* PKGTASK_H: $RCSfile: pkgtask.h,v $: end of file */
