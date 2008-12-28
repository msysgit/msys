/** \file error.c
 *
 * Created: JohnE, 2008-10-12
 */


#include "error.hh"

#include <cstdarg.>
#include <cstdio>
#include <vector>
#include <list>
#include <string>
#include "ref.hpp"


static std::vector< const char* > g_raw_errors(1, static_cast< const char* >(0));
static std::list< std::string > g_str_errors;


extern "C" void MGError(const char* fmt, ...)
{
	char msg[2048];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, 2048, fmt, ap);
	va_end(ap);
	g_str_errors.push_back(msg);
	g_raw_errors.pop_back();
	g_raw_errors.push_back(g_str_errors.back().c_str());
	g_raw_errors.push_back(0);
}


extern "C" const char* const* MGGetErrors()
{
	return &g_raw_errors.front();
}


extern "C" void MGClearErrors()
{
	g_raw_errors.clear();
	g_raw_errors.push_back(0);
	g_str_errors.clear();
}
