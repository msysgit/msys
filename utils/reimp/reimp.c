/* reimp.c -- convert MS import libraries to GNU imports.
   Copyright (C) 1999 Anders Norlander

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>

#include <reimp.h>

#if defined(_WIN32) || defined(__MSDOS__)
#include <process.h>
#endif

#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER)
#define spawnvp _spawnvp
#endif

#ifndef P_WAIT
#define P_WAIT _P_WAIT
#endif

#ifndef PATH_MAX
#define PATH_MAX 260
#endif

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

#ifndef DLLTOOL
#define DLLTOOL "dlltool"
#endif

#ifndef AS
#define AS "as"
#endif

char *dlltool_program = DLLTOOL;
char *as_program = AS;

/* specifies whether to use quick method */
int only_symbols = 0;

/* specifies whether to only create .def files */
int only_defs = 0;

/* specifies whether to convert lib names to lower case or not */
int keep_case = 0;

/* based on the name of a symbol determine whether it is an imported
   symbol */
static int
is_import (unsigned char *name)
{
  if (begins (name, "__imp_")) /* don't reimport symbols */
    return 0;
  else if (begins (name, "__IMPORT_DESCRIPTOR_"))
    return 0;
  else if (strcmp (name, "__NULL_IMPORT_DESCRIPTOR") == 0)
    return 0;
  else if (*name == 0x7f) /* thunk */
    return 0;
  return 1;
}

static int do_import (char *, uint32, FILE *);

/* read second linker member and process all archive members */
static int
read_link_member (struct ar_hdr *hdr, FILE *f)
{
  uint32 n_memb;
  uint32 n_syms;
  uint32 *ofs;
  uint16 *idx;
  char *buf;
  char *sym;
  uint32 n;
  uint16 i;

  /* FIXME: are offsets and sizes in host endianness?  Currently we
     assume so. */

  /* get number of archive members */
  if (fread (&n_memb, 4, 1, f) != 1)
    return 0;

  if (only_symbols)
    {
      /* if we only dump symbols there is no need to read the offset
       * and index tables */
      if (fseek (f, 4 * n_memb, SEEK_CUR) != 0)
        return 0;
      if (fread (&n_syms, 4, 1, f) != 1)
        return 0;
      if (fseek (f, 2 * n_syms, SEEK_CUR) != 0)
        return 0;
      ofs = NULL;
      idx = NULL;
    }
  else
    {
      /* suck in everything from the archive */
      ofs = xmalloc (4 * n_memb);
      if (fread (ofs, 4, n_memb, f) != n_memb)
        {
          free (ofs);
          return 0;
        }
      if (fread (&n_syms, 4, 1, f) != 1)
        {
          free (ofs);
          return 0;
        }
      idx = xmalloc (2 * n_syms);
      if (fread (idx, 2, n_syms, f) != n_syms)
        {
          free (ofs);
          free (idx);
          return 0;
        }
    }


  /* calculate size of symbol string table */
  n = strtol (hdr->ar_size, NULL, 10);
  n = n - (4 + 4 + 2 * n_syms + 4 * n_memb);

  /* allocate memory for string table */
  buf = xmalloc (n);

  /* read symbol string table */
  if (fread (buf, n, 1, f) != 1)
    {
      free (ofs);
      free (idx);
      return 0;
    }

  /* look for long-names member */
  ar_read_header (hdr, f);

  /* read symbols */
  for (n = 0, sym = buf; n < n_syms; n++)
    {
      if (is_import (sym))
        {
          if (only_symbols)
            puts (sym);
          else
            {
              i = idx[n];
              do_import (sym, ofs[i-1], f);
            }
        }
      sym += strlen (sym) + 1;
    }

  free (buf);
  free (ofs);
  free (idx);

  return 1;
}

struct def_file {
  char *dllname;
  FILE *f;
  struct def_file *left, *right;
};

/* tree of def files */
struct def_file *def_files = NULL;

/* convert dll filename to .def filename */
static char *
dll_to_def_name (char *def, char *dll)
{
  char *p;
  strcpy (def, dll);
  p = strrchr (def, '.');
  if (p)
    strcpy (p, ".def");
  else
    strcat (def, ".def");
  return def;
}

/* find node in def_file tree */
static struct def_file*
find_def_file (struct def_file *def, char *dll)
{
  int r;
  while (def && (r = strcasecmp (dll, def->dllname)) != 0)
    {
      if (r < 0)
        def = def->left;
      else
        def = def->right;
    }
  return def;
}

/* allocate a def_file node */
static struct def_file *
alloc_def_file (char *dll)
{
  char *p;
  struct def_file *df = xmalloc (sizeof (*df));

  df->left = df->right = NULL;

  /* open .def file */
  p = xmalloc (strlen (dll) + 1);
  dll_to_def_name (p, dll);
  df->f = fopen (p, "w");
  if (!df->f)
    error (1, p);

  /* print header */
  fprintf (df->f, "LIBRARY %s\nEXPORTS\n", dll);

  /* set dllname */
  df->dllname = p;
  strcpy (df->dllname, dll);
  return df;
}

/* create a new def_file node in tree */
static struct def_file *
create_def_file (struct def_file **head, char *dll)
{
  int r;
  if (!*head)
    return (*head = alloc_def_file (dll));

  r = strcasecmp (dll, (*head)->dllname);
  if (r < 0)
    return create_def_file (&(*head)->left, dll);
  else
    return create_def_file (&(*head)->right, dll);
}

static void
for_def_files (struct def_file *head, void (*fn)(struct def_file *, void *),
               void *arg)
{
  if (head)
    {
      (*fn) (head, arg);
      for_def_files (head->left, fn, arg);
      for_def_files (head->right, fn, arg);
    }
}

static void
free_def_file (struct def_file *head)
{
  if (head)
    {
      free_def_file (head->left);
      free_def_file (head->right);
      free (head->dllname);
      free (head);
    }
}

/* write a .def entry */
static void
write_def (char *dll, char *def, int ord, char *flags)
{
  struct def_file *df;

  /* find or create .def file */
  df = find_def_file (def_files, dll);
  if (!df)
    df = create_def_file (&def_files, dll);

  /* write entry */
  if (flags && ord != -1)
    fprintf (df->f, "%s\t@%d %s\n", def, ord, flags);
  else if (flags)
    fprintf (df->f, "%s\t%s\n", def, flags);
  else if (ord != -1)
    fprintf (df->f, "%s\t@%d\n", def, ord);
  else
    fprintf (df->f, "%s\n", def);
}

void
extract_member (char *name, uint32 size, FILE *f)
{
  static int non_imports = 0;
  char buf[2048];
  FILE *fo;
  int to_read;
  int r;
  char *p, *q;

  strncpy (buf, name, sizeof(buf));

  /* remove trailing slash */
  p = buf + strlen(buf) - 1;
  if(*p == '/')
      *p = '\0';

#if 0
  /* make parent directories */
  q = buf;
  while((p = strchr (q, '/')) || (p = strchr (q, '\\')))
    {
      *p = '\0';
#ifdef _WIN32
      mkdir (buf);
#else
      mkdir (buf, S_IRWXUGO);
#endif
        printf("%s\n", buf);
      *p = '/';
      q = p + 1;
    }
#else
  /* strip base directory */
  q = buf;
  while((p = strchr (q, '/')) || (p = strchr (q, '\\')))
    {
      q = p + 1;
    }
  strncpy (buf, q, sizeof(buf));
#endif

  fo = fopen (buf, "wb");

  if (!fo)
  {
    warning (1, name);
    return;
  }

  do
    {
      if (size < sizeof (buf))
        to_read = size;
      else
        to_read = sizeof (buf);
      if (fread (buf, to_read, 1, f) != 1)
      {
        warning (0, "unexpected end-of-file\n");
        break;
      }
      r = fwrite (buf, to_read, 1, fo);
      if (r != 1)
      {
        warning (0, "error writing to file\n");
        break;
      }
      size -= to_read;
    }
  while (size);
  fclose (fo);
}

/* process an import */
static int
do_import (char *name, uint32 offset, FILE *f)
{
  struct ar_hdr ar_hdr;
  struct imp_hdr imp_hdr;
  char imp_flags[16];
  char *buf;
  char *sym;
  char *dll;
  int ord;
  long pos;

  if (fseek (f, offset, SEEK_SET) != 0)
    return 0;

  if (!ar_read_header (&ar_hdr, f))
    return 0;

  pos = ftell (f);
  if (fread (&imp_hdr, sizeof (imp_hdr), 1, f) != 1)
    return 1;

  /* check if this is an import or not */
  if (imp_hdr.sig1 != IMAGE_FILE_MACHINE_UNKNOWN || imp_hdr.sig2 != SIG2)
    {
      if (!only_defs)
        {
          /* rewind to start of member */
          fseek (f, pos, SEEK_SET);
          extract_member (get_ar_name (&ar_hdr),
                          strtol (ar_hdr.ar_size, NULL, 10), f);
        }
      return;
    }

  sym = buf = xmalloc (imp_hdr.size);
  if (fread (buf, imp_hdr.size, 1, f) != 1)
    {
      free (buf);
      return 0;
    }
  dll = sym + strlen (sym) + 1;

  if (TEST_IMPNT(imp_hdr.type, IMPORT_ORDINAL))
    ord = imp_hdr.ord_or_hint;
  else
    ord = -1;

  if (TEST_IMPNT(imp_hdr.type, IMPORT_NAME_NOPREFIX))
    {
      if (*sym == '_')
        sym++;
      else while (*sym == '?' || *sym == '@')
        sym++;
    }
  else if (TEST_IMPNT(imp_hdr.type, IMPORT_NAME_UNDECORATE))
    {
      if (*sym == '_')
        sym++;
      else while (*sym == '?' || *sym == '@')
        sym++;
    }

  imp_flags[0] = '\0';
  if (imp_hdr.type & IMPORT_DATA)
    strcpy (imp_flags, "DATA ");
  if (imp_hdr.type & IMPORT_CONST)
    strcat (imp_flags, "CONST");

  write_def (dll, sym, ord, imp_flags[0] ? imp_flags : NULL);

  free (buf);

  return 1;
}

/* run dlltool on .def file to create import library */
static void
create_implib (struct def_file *def, void *arg)
{
  char *def_name;
  char *lib_name;
  char *p;
  int r;
  static char *argv[] = {NULL,
                         "--as", NULL,
                         "--output-lib", NULL,
                         "--def", NULL,
                         "--dllname", NULL,
                         "-k",
                         NULL};
  fclose (def->f);
  def_name = xmalloc (strlen (def->dllname) + 5);
  lib_name = xmalloc (strlen (def->dllname) + 5);
  dll_to_def_name (def_name, def->dllname);

  p = strrchr (def->dllname, '.');
  if (p)
    {
      *p = '\0';
      sprintf (lib_name, "lib%s.a", def->dllname);
      *p = '.';
    }
  else
    sprintf (lib_name, "lib%s.a", def->dllname);

  if (!keep_case)
    for (p = lib_name + 3; *p; p++)
      *p = tolower (*p);

  argv[0] = dlltool_program;
  argv[2] = as_program;
  argv[4] = lib_name;
  argv[6] = def_name;
  argv[8] = def->dllname;
  r = spawnvp (P_WAIT, argv[0], argv);
  if (r == -1)
    error (1, argv[0]);

  free (lib_name);
  free (def_name);
}

void
usage ()
{
    printf ("Usage: %s [options] IMPLIB\n", program_name);
    printf ("  -s, --dump-symbols      dump symbols to stdout\n");
    printf ("  -d, --only-def          only create .def files\n");
    printf ("  -c, --keep-case         keep case in lib*.a file names\n");
    printf ("  --dlltool <name>        use <name> for dlltool\n");
    printf ("  --as <name>             use <name> for assembler\n");
    exit (1);
}

static int
is_opt (char *opt, char *opt_short, char *opt_long)
{
  if (*opt == '-' && opt[1] != '-' &&
      opt_short && !strcmp (opt_short, opt+1))
    {
      return 1;
    }
  else if (*opt == '-' && opt[1] == '-' &&
           opt_long && !strcmp (opt_long, opt+2))
    {
      return 1;
    }
  else
    return 0;
}

int
main (int argc, char *argv[])
{
  FILE *ar;
  struct ar_hdr ar_hdr;
  long ofs;
  int i;

  char sign[SARMAG+1];
  char *ar_name;

  program_name = argv[0];

  if (argc < 2)
    usage ();

  for (i = 1; i < argc; i++)
    {
      if (is_opt(argv[i], "s", "dump-symbols"))
        only_symbols = 1;
      else if (is_opt (argv[i], "d", "only-def"))
        only_defs = 1;
      else if (is_opt (argv[i], "c", "keep-case"))
        keep_case = 1;
      else if (is_opt (argv[i], NULL, "dlltool"))
        {
          if (argv[++i])
            dlltool_program = argv[i];
          else
            usage ();
        }
      else if (is_opt (argv[i], NULL, "as"))
        {
          if (argv[++i])
            as_program = argv[i];
          else
            usage ();
        }
      else if (*argv[i] == '-')
        {
          fprintf (stderr, "%s: invalid option: %s\n",
                   program_name, argv[i]);
          usage ();
        }
      else
        break;
    }
  ar_name = argv[i];
  if (!ar_name)
    usage ();

  /* open archive */
  ar = fopen (ar_name, "rb");
  if (!ar)
    {
      error (1, argv[1]);
      return 1;
    }

  if (fread (sign, SARMAG, 1, ar) != 1)
invalid_ar:
      error (0, "%s: invalid or corrupt import library\n", ar_name);

  sign[SARMAG] = '\0';
  if (strcmp (sign, ARMAG) != 0)
    goto invalid_ar;

  /* skip first linker member */
  if (!ar_read_header (&ar_hdr, ar))
    goto invalid_ar;

  if (ar_hdr.ar_name[0] != '/')
    goto invalid_ar;

  ofs = strtoul (ar_hdr.ar_size, NULL, 10);
  ofs = (ofs + 1) & ~1; // fix alignment here
  if (fseek (ar, ofs, SEEK_CUR) != 0)
    error (1, ar_name);

  if (!ar_read_header (&ar_hdr, ar))
    goto invalid_ar;

  /* make sure we have a second linker member */
  if (ar_hdr.ar_name[0] != '/' && ar_hdr.ar_name[1] == ' ')
    goto invalid_ar;

  /* read second linker member */
  read_link_member (&ar_hdr, ar);
  fclose (ar);

  if (only_symbols)
    exit (0);

  if (!only_defs)
    for_def_files (def_files, create_implib, NULL);
  free_def_file (def_files);

  free (long_names);

  exit (0);
}
