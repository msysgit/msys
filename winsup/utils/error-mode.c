/*
******************************************************************************

   error-mode, a program that sets the error mode and execs.

   Copyright (C) 2010 Peter Rosin  [peda@lysator.liu.se]

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************
*/

#define PROGRAM "error-mode"
#define VERSION "0.3"

#include <process.h>
#include <stdio.h>
#include <getopt.h>
#include <windows.h>

void
help (const char *prg, FILE *f)
{
  fprintf (f,
"Usage: " PROGRAM " [options] [program]\n"
"   or: " PROGRAM " [options] -- program [program options]\n"
"  -c, --no-critical-fail       No critical failure error box\n"
"  -g, --no-general-protection  No general fault error box\n"
"  -f, --no-file-open           No open file error box\n"
"  -m MODE, --mode=MODE         Desired SetErrorMode argument (a number)\n"
"  -d, --debug                  Debug output\n"
"  -h, --help                   Print this message and exit\n"
"  -v, --version                Print the version number and exit\n"
"\n"
"Example:\n"
"  %s -cgf C:/mingw/msys/1.0/msys.bat\n",
           prg);
}

int
main (int argc, char *argv[])
{
  int c;
  UINT old_mode;
  UINT new_mode;
  UINT error_mode = 0;
  int debug = 0;
  struct option opts[] = {
    { "no-critical-fail",      0, NULL, 'c' },
    { "no-general-protection", 0, NULL, 'g' },
    { "no-file-open",          0, NULL, 'f' },
    { "mode",                  1, NULL, 'm' },
    { "debug",                 0, NULL, 'd' },
    { "help",                  0, NULL, 'h' },
    { "version",               0, NULL, 'v' },
    { NULL,                    0, NULL, 0 }
  };
  intptr_t status;
  
  while ((c = getopt_long_only (argc, argv, ":cgfm:dhv", opts, NULL)) != -1) {
    switch (c) {
    case 'c':
      error_mode |= SEM_FAILCRITICALERRORS;
      break;
    case 'g':
      error_mode |= SEM_NOGPFAULTERRORBOX;
      break;
    case 'f':
      error_mode |= SEM_NOOPENFILEERRORBOX;
      break;
    case 'm':
      error_mode = strtoul (optarg, NULL, 0);
      break;
    case 'd':
      ++debug;
      break;
    case 'h':
      help (argv[0], stdout);
      return 0;
    case 'v':
      printf (PROGRAM " " VERSION "\n");
      return 0;
    case ':':
    case '?':
      help (argv[0], stderr);
      return 1;
    }
  }

  old_mode = SetErrorMode (error_mode);

  if (optind >= argc) {
    printf ("error mode: 0x%x\n", old_mode);
    return 0;
  }

  new_mode = SetErrorMode (error_mode);

  if (debug) {
    fprintf (stderr, "old error mode: 0x%x\n", old_mode);
    fprintf (stderr, "new error mode: 0x%x\n", new_mode);
    fflush (stderr);
  }

  status = spawnvp (P_WAIT, argv[optind], (const char * const *)argv + optind);

  if (status == -1) {
    fprintf (stderr, "Failed to spawn inferior\n");
    return 1;
  }
  return status;
}
