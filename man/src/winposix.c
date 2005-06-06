#include "compat.h"

/* win32path_transform(): fix up Win32 path names, so that they
 * conform to the POSIX convention of using "/" as the directory separator,
 * (but leave "D:" as a drive designator, if it is present).
 */

const char *win32path_transform (const char *pathname)
{
  char *p = (char *)pathname;

  if ( p )
    do if (*p == '\\') *p = '/';
      while (*p++);
  return pathname;
}

/* win32path_is_absolute(): check if a specified Win32 path,
 * which may, or may not have been transformed by win32path_transform(),
 * represents an absolute reference to a file system location.
 */

int win32path_is_absolute (const char *pathname)
{
  pathname = win32path_transform (pathname);
  return (*pathname == '/') || ((pathname[1] == ':') && (pathname[2] == '/'));
}
