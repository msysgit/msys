/* regmagic.h

   Copyright 1996, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/*	$Id: regmagic.h,v 1.1 1997/10/07 21:54:10 noer Exp $ */

/*
 * The first byte of the regexp internal "program" is actually this magic
 * number; the start node begins in the second byte.
 */
#define	MAGIC	0234
