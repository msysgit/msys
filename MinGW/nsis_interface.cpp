/** \file nsis_interface.cpp
 *
 * Created: JohnE, 2008-07-26
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#include "nsis_interface.hpp"

#include "ref.hpp"


int NSIS::s_stringsize = 0;
char* NSIS::s_variables = 0;
stack_t** NSIS::s_stacktop = 0;
extra_parameters* NSIS::s_extra = 0;


static void CharArrayDeleter(char* a)
{
	delete[] a;
}


static int myatoi(const char *s)
{
  int v=0;
  if (*s == '0' && (s[1] == 'x' || s[1] == 'X'))
  {
    s++;
    for (;;)
    {
      int c=*(++s);
      if (c >= '0' && c <= '9') c-='0';
      else if (c >= 'a' && c <= 'f') c-='a'-10;
      else if (c >= 'A' && c <= 'F') c-='A'-10;
      else break;
      v<<=4;
      v+=c;
    }
  }
  else if (*s == '0' && s[1] <= '7' && s[1] >= '0')
  {
    for (;;)
    {
      int c=*(++s);
      if (c >= '0' && c <= '7') c-='0';
      else break;
      v<<=3;
      v+=c;
    }
  }
  else
  {
    int sign=0;
    if (*s == '-') sign++; else s--;
    for (;;)
    {
      int c=*(++s) - '0';
      if (c < 0 || c > 9) break;
      v*=10;
      v+=c;
    }
    if (sign) v = -v;
  }

  // Support for simple ORed expressions
  if (*s == '|') 
  {
      v |= myatoi(s+1);
  }

  return v;
}


void NSIS::UpdateParams
 (int string_size,
  char* variables,
  stack_t** stacktop,
  extra_parameters* extra)
{
	s_stringsize = string_size;
	s_variables = variables;
	s_stacktop = stacktop;
	s_extra = extra;
}


StringType NSIS::popstring()
{
  stack_t* th;
  if (!s_stacktop || !*s_stacktop)
	return StringType();
  th = (*s_stacktop);
  RefType< char >::Ref str(new char[s_stringsize], CharArrayDeleter);
  lstrcpyn(str.get(), th->text, s_stringsize);
  *s_stacktop = th->next;
  GlobalFree((HGLOBAL)th);
  return StringType(str.get());
}


int NSIS::popint()
{
  return myatoi(popstring().c_str());
}


void NSIS::pushstring(const char* str)
{
  stack_t* th;
  if (!s_stacktop)
	return;
  th = (stack_t*)GlobalAlloc(GPTR, sizeof(stack_t) + s_stringsize);
  lstrcpyn(th->text, str, s_stringsize);
  th->next = *s_stacktop;
  *s_stacktop = th;
}


void NSIS::pushint(int value)
{
	char buffer[1024];
	wsprintf(buffer, "%d", value);
	pushstring(buffer);
}


void NSIS::ExecuteCallback(nsFunction func)
{
	s_extra->ExecuteCodeSegment(func - 1, 0);
}

