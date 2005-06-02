#include "compat.h"

/* win32posix(): fix up Win32 path names, so that they
 * conform to the POSIX convention of using "/" as the directory separator,
 * (but leave "D:" as a drive designator, if it is present).
 */

const char *win32posix (const char *pathname)
{
  char *p = (char *)pathname;

  if ( p )
    do if (*p == '\\') *p = '/';
      while (*p++);
  return pathname;
}
