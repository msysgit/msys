#include <stdarg.h>
#include <stdlib.h>

#include <reimp.h>

char *program_name;

#define swapb(a,b) a ^= b; b ^= a; a ^= b

/* swap little <=> big endian */
uint32
swap_endian (uint32 u)
{
#define swapb(a,b) a ^= b; b ^= a; a ^= b
  unsigned char *p = (unsigned char *) &u;
  swapb (p[0], p[3]);
  swapb (p[1], p[2]);
  return *((uint32 *) p);
}


/* report warning and terminate */
void
warning (int perr, char *s, ...)
{
  va_list args;

  fprintf (stderr, "%s: ", program_name);
  if (!perr)
    {
      va_start (args, s);
      vfprintf (stderr, s, args);
      va_end (args);
    }
  else
    perror (s);
}

/* report error and terminate */
void
error (int perr, char *s, ...)
{
  va_list args;

  fprintf (stderr, "%s: ", program_name);
  if (!perr)
    {
      va_start (args, s);
      vfprintf (stderr, s, args);
      va_end (args);
    }
  else
    perror (s);
  exit (1);
}

/* allocate memory or die! */
void *
xmalloc (size_t size)
{
  void *p = malloc (size);
  if (!p)
    error (0, "out of memory\n");
  return p;
}

/* check if a string begins with with */
int
begins (char *s, char *with)
{
  return strncmp (s, with, strlen (with)) == 0;
}

#if !defined(_WIN32) && !defined(__MSDOS__)

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

int
spawnvp (int mode, char *path, char **argv)
{
  int pid;
  int status;

  switch ((pid = fork ()))
    {
    case -1:
      return -1;
    case 0:
      execvp (path, argv);

      _exit (EXIT_FAILURE);
      break;
    default:
      if (mode == P_WAIT)
        {
          if (wait (&status) == -1)
            return -1;
          return status;
        }
      else
        return pid;
    }
}
#endif
