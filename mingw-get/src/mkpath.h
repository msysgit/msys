#ifndef MKPATH_H
/*
 * mkpath.h
 *
 * $Id: mkpath.h,v 1.1 2009-11-16 21:54:30 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Prototype declarations for the path name constructor functions,
 * and the directory hierarchy and file creation functions, which are
 * implemented in mkpath.c
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
#define MKPATH_H  1

#ifndef EXTERN_C
# ifdef __cplusplus
#  define EXTERN_C extern "C"
# else
#  define EXTERN_C
# endif
#endif

EXTERN_C int mkdir_recursive( const char *, int );
EXTERN_C int set_output_stream( const char *, int );
EXTERN_C int mkpath( char *, const char *, const char *, const char * );

#endif /* MKPATH_H: $RCSfile: mkpath.h,v $: end of file */
