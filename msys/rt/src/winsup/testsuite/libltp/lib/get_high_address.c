/* $Header: /cvs/src/src/winsup/testsuite/libltp/lib/get_high_address.c,v 1.1 2000/09/03 03:52:30 cgf Exp $ */

/*
 *	(C) COPYRIGHT CRAY RESEARCH, INC.
 *	UNPUBLISHED PROPRIETARY INFORMATION.
 *	ALL RIGHTS RESERVED.
 */

#include <unistd.h> 

char *
get_high_address()
{
       return (char *)sbrk(0) + 16384;
}
