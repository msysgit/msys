/*
FUNCTION
	<<strchr>>---search for character in string

INDEX
	strchr

ANSI_SYNOPSIS
	#include <string.h>
	char * strchr(const char *<[string]>, int <[c]>);

TRAD_SYNOPSIS
	#include <string.h>
	char * strchr(<[string]>, <[c]>);
	char *<[string]>;
	int *<[c]>;

DESCRIPTION
	This function finds the first occurence of <[c]> (converted to
	a char) in the string pointed to by <[string]> (including the
	terminating null character).

RETURNS
	Returns a pointer to the located character, or a null pointer
	if <[c]> does not occur in <[string]>.

PORTABILITY
<<strchr>> is ANSI C.

<<strchr>> requires no supporting OS subroutines.

QUICKREF
	strchr ansi pure
*/

#include <string.h>
#include <bitblock.h>

char *
_DEFUN (strchr, (s1, i),
	_CONST char *s1 _AND
	int i)
{
  _CONST unsigned char *s = (_CONST unsigned char *)s1;
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  unsigned char c = (unsigned int)i;

  while (*s && *s != c)
    {
      s++;
    }

  if (*s != c)
    {
      s = NULL;
    }

  return (char *) s;
#else
  unsigned char c = (unsigned char)i;
  unsigned long mask,j;
  unsigned long *aligned_addr;

  if (!UNALIGNED (s))
    {
      mask = 0;
      for (j = 0; j < LBLOCKSIZE; j++)
        mask = (mask << 8) | c;

      aligned_addr = (unsigned long*)s;
      while (!DETECTNULL (*aligned_addr) && !DETECTCHAR (*aligned_addr, mask))
        aligned_addr++;

      /* The block of bytes currently pointed to by aligned_addr
         contains either a null or the target char, or both.  We
         catch it using the bytewise search.  */

      s = (unsigned char*)aligned_addr;
    }

  while (*s && *s != c)
      s++;
  if (*s == c)
    return (char *)s;
  return NULL;
#endif /* not PREFER_SIZE_OVER_SPEED */
}
