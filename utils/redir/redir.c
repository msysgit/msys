/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*

   Redir 2.0 Copyright (C) 1995-1998 DJ Delorie (dj@delorie.com)
   Modified 1999 by Mumit Khan <khan@xraylith.wisc.edu>

   Redir is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Redir is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

*/
   
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <process.h>
#include <stdio.h>
#include <string.h>

/* Here's the deal.  We need to pass the command-line arguments to the
   child program *exactly* as we got them.  This means we cannot allow
   any wildcard expansion, we need to retain any quote characters, and
   we need to disable response files processing.  That's why we must
   link with CRT_noglob.o!  */

extern void *xmalloc(size_t);
extern void *xrealloc(void *, size_t);

int display_exit_code=0;
int std_err_fid;
FILE *std_err;
int rv;

static void
usage(void)
{
  /*               ----+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8 */
  fprintf(stderr, "Redir 2.0 Copyright (C) 1995 - 1998 DJ Delorie (dj@delorie.com)\n");
  fprintf(stderr, "Modified 1999 for Mingw32 by Mumit Khan <khan@xraylith.wisc.edu>\n");
  fprintf(stderr, "Distribute freely.  There is NO WARRANTY.\n");
  fprintf(stderr, "This program is protected by the GNU General Public License.\n\n");
  fprintf(stderr, "Usage: redir [-i file] [-o file] [-oa file] [-e file] [-ea file]\n");
  fprintf(stderr, "                [-eo] [-oe] [-x] command [args . . .]\n\n");
  fprintf(stderr, "  -i file   redirect stdandard input from file\n");
  fprintf(stderr, "  -o file   redirect standard output to file\n");
  fprintf(stderr, "  -oa file  append standard output to file\n");
  fprintf(stderr, "  -e file   redirect standard error to file\n");
  fprintf(stderr, "  -ea file  append standard error to file\n");
  fprintf(stderr, "  -eo       redirect standard error to standard output\n");
  fprintf(stderr, "  -oe       redirect standard output to standard error\n");
  fprintf(stderr, "  -x        print exit code\n");
  fprintf(stderr, "  command   the program you want to run, with arguments\n\n");
  fprintf(stderr, "Options are processed in the order they are encountered.\n\n");
  exit(1);
}

static void
fatal(const char *msg, const char *fn)
{
  fprintf(std_err, msg, fn);
  fprintf(std_err, "\nThe error was: %s\n", strerror(errno));
  exit(1);
}

static void
unquote(const char *src, char *dst)
{
  int quote=0;

  while ((quote || !isspace(*src)) && *src)
  {
    if (quote && *src == quote)
    {
      quote = 0;
      src++;
    }
    else if (!quote && (*src == '\'' || *src == '"'))
    {
      quote = *src;
      src++;
    }
    else if (*src == '\\' && strchr("'\"", src[1]) && src[1])
    {
      src++;
      *dst++ = *src++;
    }
    else
    {
      *dst++ = *src++;
    }
  }
  *dst = '\0';
}

static char *
unquoted_argv(int argc, char *argv[], char *reuse)
{
  char *new_arg;

  if (reuse)
    new_arg = (char *)xrealloc(reuse, strlen(argv[argc]) + 1);
  else
    new_arg = (char *)xmalloc(strlen(argv[argc]) + 1);
  unquote(argv[argc], new_arg);
  return new_arg;
}

static int
run_program(int argc, char *argv[])
{
  return spawnvp (P_WAIT, argv[1], argv+1);
}

int
main(int argc, char **argv)
{
  char *arg1 = NULL, *arg2 = NULL;

  if (argc < 2)
    usage();

  std_err_fid = dup(1);
  std_err = fdopen(std_err_fid, "w");

  /* We requested that the startup code retains any quote characters
     in the argv[] elements.  So we need to unquote those that we
     process as we go.  */
  while (argc > 1 && (arg1 = unquoted_argv(1, argv, arg2))[0] == '-')
  {
    int temp;
    if (strcmp(arg1, "-i")==0 && argc > 2)
    {
      if ((temp = open(arg2 = unquoted_argv(2, argv, arg1),
		       O_RDONLY, 0666)) < 0
	  || dup2(temp, 0) == -1)
	fatal("redir: attempt to redirect stdin from %s failed", arg2);
      close(temp);
      argc--;
      argv++;
    }
    else if (strcmp(arg1, "-o")==0 && argc > 2)
    {
      if ((temp = open(arg2 = unquoted_argv(2, argv, arg1),
		       O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0
	  || dup2(temp, 1) == -1)
	fatal("redir: attempt to redirect stdout to %s failed", arg2);
      close(temp);
      argc--;
      argv++;
    }
    else if (strcmp(arg1, "-oa")==0 && argc > 2)
    {
      if ((temp = open(arg2 = unquoted_argv(2, argv, arg1),
		       O_WRONLY|O_APPEND|O_CREAT, 0666)) < 0
	  || dup2(temp, 1) == -1)
	fatal("redir: attempt to append stdout to %s failed", arg2);
      close(temp);
      argc--;
      argv++;
    }
    else if (strcmp(arg1, "-e")==0 && argc > 2)
    {
      if ((temp = open(arg2 = unquoted_argv(2, argv, arg1),
		       O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0
	  || dup2(temp, 2) == -1)
	fatal("redir: attempt to redirect stderr to %s failed", arg2);
      close(temp);
      argc--;
      argv++;
    }
    else if (strcmp(arg1, "-ea")==0 && argc > 2)
    {
      if ((temp = open(arg2 = unquoted_argv(2, argv, arg1),
		       O_WRONLY|O_APPEND|O_CREAT, 0666)) < 0
	  || dup2(temp, 2) == -1)
	fatal("redir: attempt to append stderr to %s failed", arg2);
      close(temp);
      argc--;
      argv++;
    }
    else if (strcmp(arg1, "-eo")==0)
    {
      if (dup2(1,2) == -1)
	fatal("redir: attempt to redirect stderr to stdout failed", 0);
    }
    else if (strcmp(arg1, "-oe")==0)
    {
      if (dup2(2,1) == -1)
	fatal("redir: attempt to redirect stdout to stderr failed", 0);
    }
    else if (strcmp(arg1, "-x")==0)
    {
      display_exit_code = 1;
    }
    else
      usage();
    argc--;
    argv++;
  }

  if (argc <= 1)
  {
    errno = EINVAL;
    fatal("Missing program name; aborting", "");
  }

  rv = run_program(argc, argv);

  if (rv < 0)
    fatal("Error attempting to run program %s", argv[1]);

  if (display_exit_code)
  {
    fprintf(std_err, "Exit code: %d\n", rv);
  }

  return rv;
}
