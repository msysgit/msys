/* sum -- checksum and count the blocks in a file
   Copyright (C) 86, 89, 91, 1995-1999 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* Like BSD sum or SysV sum -r, except like SysV sum if -s option is given. */

/* Written by Kayvan Aghaiepour and David MacKenzie. */

#include <config.h>

#include <stdio.h>
#include <sys/types.h>
#include <getopt.h>
#include "system.h"
#include "error.h"
#include "safe-read.h"

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "sum"

#define AUTHORS "Kayvan Aghaiepour and David MacKenzie"

/* The name this program was run with. */
char *program_name;

/* Nonzero if any of the files read were the standard input. */
static int have_read_stdin;

/* Right-rotate 32-bit integer variable C. */
#define ROTATE_RIGHT(c) if ((c) & 01) (c) = ((c) >>1) + 0x8000; else (c) >>= 1;

static struct option const longopts[] =
{
  {"sysv", no_argument, NULL, 's'},
  {GETOPT_HELP_OPTION_DECL},
  {GETOPT_VERSION_OPTION_DECL},
  {NULL, 0, NULL, 0}
};

void
usage (int status)
{
  if (status != 0)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      printf (_("\
Usage: %s [OPTION]... [FILE]...\n\
"),
	      program_name);
      printf (_("\
Print checksum and block counts for each FILE.\n\
\n\
  -r              defeat -s, use BSD sum algorithm, use 1K blocks\n\
  -s, --sysv      use System V sum algorithm, use 512 bytes blocks\n\
      --help      display this help and exit\n\
      --version   output version information and exit\n\
\n\
With no FILE, or when FILE is -, read standard input.\n\
"));
      puts (_("\nReport bugs to <bug-textutils@gnu.org>."));
    }
  exit (status == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

/* Calculate and print the rotated checksum and the size in 1K blocks
   of file FILE, or of the standard input if FILE is "-".
   If PRINT_NAME is >1, print FILE next to the checksum and size.
   The checksum varies depending on sizeof(int).
   Return 0 if successful, -1 if an error occurs. */

static int
bsd_sum_file (const char *file, int print_name)
{
  register FILE *fp;
  register unsigned long checksum = 0; /* The checksum mod 2^16. */
  register long total_bytes = 0; /* The number of bytes. */
  register int ch;		/* Each character read. */

  if (STREQ (file, "-"))
    {
      fp = stdin;
      have_read_stdin = 1;
    }
  else
    {
      fp = fopen (file, "r");
      if (fp == NULL)
	{
	  error (0, errno, "%s", file);
	  return -1;
	}
    }
  /* Need binary I/O, or else byte counts and checksums are incorrect.  */
  SET_BINARY (fileno(fp));

  while ((ch = getc (fp)) != EOF)
    {
      total_bytes++;
      ROTATE_RIGHT (checksum);
      checksum += ch;
      checksum &= 0xffff;	/* Keep it within bounds. */
    }

  if (ferror (fp))
    {
      error (0, errno, "%s", file);
      if (!STREQ (file, "-"))
	fclose (fp);
      return -1;
    }

  if (!STREQ (file, "-") && fclose (fp) == EOF)
    {
      error (0, errno, "%s", file);
      return -1;
    }

  printf ("%05lu %5ld", checksum, (total_bytes + 1024 - 1) / 1024);
  if (print_name > 1)
    printf (" %s", file);
  putchar ('\n');

  return 0;
}

/* Calculate and print the checksum and the size in 512-byte blocks
   of file FILE, or of the standard input if FILE is "-".
   If PRINT_NAME is >0, print FILE next to the checksum and size.
   Return 0 if successful, -1 if an error occurs. */

static int
sysv_sum_file (const char *file, int print_name)
{
  int fd;
  unsigned char buf[8192];
  register int bytes_read;
  register unsigned long checksum = 0;
  long total_bytes = 0;

  if (STREQ (file, "-"))
    {
      fd = 0;
      have_read_stdin = 1;
    }
  else
    {
      fd = open (file, O_RDONLY);
      if (fd == -1)
	{
	  error (0, errno, "%s", file);
	  return -1;
	}
    }
  /* Need binary I/O, or else byte counts and checksums are incorrect.  */
  SET_BINARY (fd);

  while ((bytes_read = safe_read (fd, buf, sizeof buf)) > 0)
    {
      register int i;

      for (i = 0; i < bytes_read; i++)
	checksum += buf[i];
      total_bytes += bytes_read;
    }

  if (bytes_read < 0)
    {
      error (0, errno, "%s", file);
      if (!STREQ (file, "-"))
	close (fd);
      return -1;
    }

  if (!STREQ (file, "-") && close (fd) == -1)
    {
      error (0, errno, "%s", file);
      return -1;
    }

  printf ("%lu %ld", checksum % 0xffff, (total_bytes + 512 - 1) / 512);
  if (print_name)
    printf (" %s", file);
  putchar ('\n');

  return 0;
}

int
main (int argc, char **argv)
{
  int errors = 0;
  int optc;
  int files_given;
  int (*sum_func) () = bsd_sum_file;

  program_name = argv[0];
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  have_read_stdin = 0;

  while ((optc = getopt_long (argc, argv, "rs", longopts, NULL)) != -1)
    {
      switch (optc)
	{
	case 0:
	  break;

	case 'r':		/* For SysV compatibility. */
	  sum_func = bsd_sum_file;
	  break;

	case 's':
	  sum_func = sysv_sum_file;
	  break;

	case_GETOPT_HELP_CHAR;

	case_GETOPT_VERSION_CHAR (PROGRAM_NAME, AUTHORS);

	default:
	  usage (1);
	}
    }

  files_given = argc - optind;
  if (files_given == 0)
    {
      if ((*sum_func) ("-", files_given) < 0)
	errors = 1;
    }
  else
    for (; optind < argc; optind++)
      if ((*sum_func) (argv[optind], files_given) < 0)
	errors = 1;

  if (have_read_stdin && fclose (stdin) == EOF)
    error (EXIT_FAILURE, errno, "-");
  exit (errors == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
