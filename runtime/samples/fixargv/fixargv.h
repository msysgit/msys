/*
 * fixargv.h
 *
 * Prototypes of utility functions for 'properly' escaping argv vectors.
 *
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 * $Revision: 1.3 $
 * $Author: infidel $
 * $Date: 2002-05-26 10:05:59 $
 *
 */

#ifndef _FIXARGV_H_
#define _FIXARGV_H_

char* fix_arg (const char* szArg);
char* const* fix_argv (int argc, char* const* szaArgv);
void free_fixed_argv (char* const* szaFixed, char* const* szaOld);

#endif
