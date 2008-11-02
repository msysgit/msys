/** \file error.c
 *
 * Created: JohnE, 2008-10-12
 */


#include "error.hh"

#include <stdarg.h>
#include <stdio.h>


char mg_last_error[2048] = {'\0'};


void MGSetError(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(mg_last_error, 2048, fmt, ap);
	va_end(ap);
}
