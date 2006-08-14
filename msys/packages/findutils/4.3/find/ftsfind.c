/* find -- search for files in a directory hierarchy (fts version)
   Copyright (C) 1990, 91, 92, 93, 94, 2000, 
                 2003, 2004, 2005 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.*/

/* GNU find was written by Eric Decker <cire@cisco.com>,
   with enhancements by David MacKenzie <djm@gnu.org>,
   Jay Plett <jay@silence.princeton.nj.us>,
   and Tim Wood <axolotl!tim@toad.com>.
   The idea for -print0 and xargs -0 came from
   Dan Bernstein <brnstnd@kramden.acf.nyu.edu>.  
   Improvements have been made by James Youngman <jay@gnu.org>.
*/


#include "defs.h"


#define USE_SAFE_CHDIR 1
#undef  STAT_MOUNTPOINTS


#include <errno.h>
#include <assert.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#else
#include <sys/file.h>
#endif


#include "../gnulib/lib/xalloc.h"
#include "closeout.h"
#include <modetype.h>
#include "quotearg.h"
#include "quote.h"
#include "fts_.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# define _(Text) Text
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)
#endif
#ifdef gettext_noop
# define N_(String) gettext_noop (String)
#else
/* See locate.c for explanation as to why not use (String) */
# define N_(String) String
#endif


#ifdef STAT_MOUNTPOINTS
static void init_mounted_dev_list(void);
#endif

/* We have encountered an error which shoudl affect the exit status.
 * This is normally used to change the exit status from 0 to 1.
 * However, if the exit status is already 2 for example, we don't want to 
 * reduce it to 1.
 */
static void
error_severity(int level)
{
  if (state.exit_status < level)
    state.exit_status = level;
}

#ifdef DEBUG
#define STRINGIFY(X) #X
#define HANDLECASE(N) case N: return #N;

static char *
get_fts_info_name(int info)
{
  static char buf[10];
  switch (info)
    {
      HANDLECASE(FTS_D);
      HANDLECASE(FTS_DC);
      HANDLECASE(FTS_DEFAULT);
      HANDLECASE(FTS_DNR);
      HANDLECASE(FTS_DOT);
      HANDLECASE(FTS_DP);
      HANDLECASE(FTS_ERR);
      HANDLECASE(FTS_F);
      HANDLECASE(FTS_INIT);
      HANDLECASE(FTS_NS);
      HANDLECASE(FTS_NSOK);
      HANDLECASE(FTS_SL);
      HANDLECASE(FTS_SLNONE);
      HANDLECASE(FTS_W);
    default:
      sprintf(buf, "[%d]", info);
      return buf;
    }
}

#endif

static void
visit(FTS *p, FTSENT *ent, struct stat *pstat)
{
  state.curdepth = ent->fts_level;
  state.have_stat = (ent->fts_info != FTS_NS) && (ent->fts_info != FTS_NSOK);
  state.rel_pathname = ent->fts_accpath;

  /* Apply the predicates to this path. */
  (*(eval_tree)->pred_func)(ent->fts_path, pstat, eval_tree);

  /* Deal with any side effects of applying the predicates. */
  if (state.stop_at_current_level)
    {
      fts_set(p, ent, FTS_SKIP);
    }
}

static const char*
partial_quotearg_n(int n, char *s, size_t len, enum quoting_style style)
{
  if (0 == len)
    {
      return quotearg_n_style(n, style, "");
    }
  else
    {
      char saved;
      const char *result;
      
      saved = s[len];
      s[len] = 0;
      result = quotearg_n_style(n, style, s);
      s[len] = saved;
      return result;
    }
}


/* We've detected a filesystem loop.   This is caused by one of 
 * two things:
 *
 * 1. Option -L is in effect and we've hit a symbolic link that 
 *    points to an ancestor.  This is harmless.  We won't traverse the 
 *    symbolic link.
 *
 * 2. We have hit a real cycle in the directory hierarchy.  In this 
 *    case, we issue a diagnostic message (POSIX requires this) and we
 *    skip that directory entry.
 */
static void
issue_loop_warning(FTSENT * ent)
{
  if (S_ISLNK(ent->fts_statp->st_mode))
    {
      error(0, 0,
	    _("Symbolic link %s is part of a loop in the directory hierarchy; we have already visited the directory to which it points."),
	    quotearg_n_style(0, locale_quoting_style, ent->fts_path));
    }
  else
    {
      /* We have found an infinite loop.  POSIX requires us to
       * issue a diagnostic.  Usually we won't get to here
       * because when the leaf optimisation is on, it will cause
       * the subdirectory to be skipped.  If /a/b/c/d is a hard
       * link to /a/b, then the link count of /a/b/c is 2,
       * because the ".." entry of /b/b/c/d points to /a, not
       * to /a/b/c.
       */
      error(0, 0,
	    _("Filesystem loop detected; "
	      "%s is part of the same filesystem loop as %s."),
	    quotearg_n_style(0, locale_quoting_style, ent->fts_path),
	    partial_quotearg_n(1,
			       ent->fts_cycle->fts_path,
			       ent->fts_cycle->fts_pathlen,
			       locale_quoting_style));
    }
}

/* 
 * Return true if NAME corresponds to a file which forms part of a 
 * symbolic link loop.  The command 
 *      rm -f a b; ln -s a b; ln -s b a 
 * produces such a loop.
 */
static boolean 
symlink_loop(const char *name)
{
  struct stat stbuf;
  int rv;
  if (following_links())
    rv = stat(name, &stbuf);
  else
    rv = lstat(name, &stbuf);
  return (0 != rv) && (ELOOP == errno);
}


static void
consider_visiting(FTS *p, FTSENT *ent)
{
  struct stat statbuf;
  mode_t mode;

#ifdef DEBUG
  fprintf(stderr,
	  "consider_visiting: end->fts_info=%s, ent->fts_path=%s\n",
	  get_fts_info_name(ent->fts_info),
	  quotearg_n(0, ent->fts_path, locale_quoting_style));
#endif

  /* Cope with various error conditions. */
  if (ent->fts_info == FTS_ERR
      || ent->fts_info == FTS_NS
      || ent->fts_info == FTS_DNR)
    {
      error(0, ent->fts_errno, ent->fts_path);
      error_severity(1);
      return;
    }
  else if (ent->fts_info == FTS_DC)
    {
      issue_loop_warning(ent);
      error_severity(1);
      return;
    }
  else if (ent->fts_info == FTS_SLNONE)
    {
      /* fts_read() claims that ent->fts_accpath is a broken symbolic
       * link.  That would be fine, but if this is part of a symbolic
       * link loop, we diagnose the problem and also ensure that the
       * eventual return value is nonzero.   Note that while the path 
       * we stat is local (fts_accpath), we print the fill path name 
       * of the file (fts_path) in the error message.
       */
      if (symlink_loop(ent->fts_accpath))
	{
	  error(0, ELOOP, ent->fts_path);
	  error_severity(1);
	  return;
	}
    }
  
  /* Not an error, cope with the usual cases. */
  if (ent->fts_info == FTS_NSOK)
    {
      state.have_stat = false;
      mode = 0;
    }
  else
    {
      state.have_stat = true;
      statbuf = *(ent->fts_statp);
      mode = statbuf.st_mode;
    }

  if (0 == ent->fts_level && (0u == state.starting_path_length))
    state.starting_path_length = ent->fts_pathlen;
	      
  if (0 != digest_mode(mode, ent->fts_path, ent->fts_name, &statbuf, 0))
    {
      /* examine this item. */
      int ignore = 0;
	      
      if (S_ISDIR(statbuf.st_mode) && (ent->fts_info == FTS_NSOK))
	{
	  /* This is a directory, but fts did not stat it, so
	   * presumably would not be planning to search its
	   * children.  Force a stat of the file so that the
	   * children can be checked.
	   */
	  fts_set(p, ent, FTS_AGAIN);
	  return;
	}

      if (options.maxdepth >= 0 && (ent->fts_level > options.maxdepth))
	{
	  ignore = 1;
	  fts_set(p, ent, FTS_SKIP);
	}
      else if ( (ent->fts_info == FTS_D) && !options.do_dir_first )
	{
	  /* this is the preorder visit, but user said -depth */ 
	  ignore = 1;
	}
      else if ( (ent->fts_info == FTS_DP) && options.do_dir_first )
	{
	  /* this is the postorder visit, but user didn't say -depth */ 
	  ignore = 1;
	}
      else if (ent->fts_level < options.mindepth)
	{
	  ignore = 1;
	}

      if (!ignore)
	{
	  visit(p, ent, &statbuf);
	}
	      
	      
      if (ent->fts_info == FTS_DP)
	{
	  /* we're leaving a directory. */
	  state.stop_at_current_level = false;
	  complete_pending_execdirs(eval_tree);
	}
    }
}


static void
find(char *arg)
{
  char * arglist[2];
  int ftsoptions;
  FTS *p;
  FTSENT *ent;
  

  arglist[0] = arg;
  arglist[1] = NULL;
  
  ftsoptions = FTS_NOSTAT;
  switch (options.symlink_handling)
    {
    case SYMLINK_ALWAYS_DEREF:
      ftsoptions |= FTS_COMFOLLOW|FTS_LOGICAL;
      break;
	  
    case SYMLINK_DEREF_ARGSONLY:
      ftsoptions |= FTS_COMFOLLOW;
      break;
	  
    case SYMLINK_NEVER_DEREF:
      ftsoptions |= FTS_PHYSICAL;
      break;
    }

  if (options.stay_on_filesystem)
    ftsoptions |= FTS_XDEV;
      
  p = fts_open(arglist, ftsoptions, NULL);
  if (NULL == p)
    {
      error (0, errno,
	     _("cannot search %s"),
	     quotearg_n_style(0, locale_quoting_style, arg));
    }
  else
    {
      while ( (ent=fts_read(p)) != NULL )
	{
	  consider_visiting(p, ent);
	}
      fts_close(p);
      p = NULL;
    }
}


static void 
process_all_startpoints(int argc, char *argv[])
{
  int i;

  /* figure out how many start points there are */
  for (i = 0; i < argc && strchr ("-!(),", argv[i][0]) == NULL; i++)
    {
      find(argv[i]);
    }
  
  if (i == 0)
    {
      /* 
       * We use a temporary variable here because some actions modify 
       * the path temporarily.  Hence if we use a string constant, 
       * we get a coredump.  The best example of this is if we say 
       * "find -printf %H" (note, not "find . -printf %H").
       */
      char defaultpath[2] = ".";
      find(defaultpath);
    }
}




int
main (int argc, char **argv)
{
  int i;
  const struct parser_table *parse_entry; /* Pointer to the parsing table entry for this expression. */
  struct predicate *cur_pred;
  char *predicate_name;		/* Name of predicate being parsed. */
  int end_of_leading_options = 0; /* First arg after any -H/-L etc. */
  program_name = argv[0];
  const struct parser_table *entry_close, *entry_print, *entry_open;


  /* We call check_nofollow() before setlocale() because the numbers 
   * for which we check (in the results of uname) definitiely have "."
   * as the decimal point indicator even under locales for which that 
   * is not normally true.   Hence atof() would do the wrong thing 
   * if we call it after setlocale().
   */
#ifdef O_NOFOLLOW
  options.open_nofollow_available = check_nofollow();
#else
  options.open_nofollow_available = false;
#endif

  options.regex_options = RE_SYNTAX_EMACS;
  
#ifdef HAVE_SETLOCALE
  setlocale (LC_ALL, "");
#endif

  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
  atexit (close_stdout);

  
  if (isatty(0))
    {
      options.warnings = true;
      options.literal_control_chars = false;
    }
  else
    {
      options.warnings = false;
      options.literal_control_chars = false; /* may change */
    }
  
  
  predicates = NULL;
  last_pred = NULL;
  options.do_dir_first = true;
  options.maxdepth = options.mindepth = -1;
  options.start_time = time (NULL);
  options.cur_day_start = options.start_time - DAYSECS;
  options.full_days = false;
  options.stay_on_filesystem = false;
  options.ignore_readdir_race = false;

  state.exit_status = 0;

#if defined(DEBUG_STAT)
  options.xstat = debug_stat;
#endif /* !DEBUG_STAT */

  if (getenv("POSIXLY_CORRECT"))
    options.output_block_size = 512;
  else
    options.output_block_size = 1024;

  if (getenv("FIND_BLOCK_SIZE"))
    {
      error (1, 0, _("The environment variable FIND_BLOCK_SIZE is not supported, the only thing that affects the block size is the POSIXLY_CORRECT environment variable"));
    }

#if LEAF_OPTIMISATION
  /* The leaf optimisation is enabled. */
  options.no_leaf_check = false;
#else
  /* The leaf optimisation is disabled. */
  options.no_leaf_check = true;
#endif

  set_follow_state(SYMLINK_NEVER_DEREF); /* The default is equivalent to -P. */

#ifdef DEBUG
  fprintf (stderr, "cur_day_start = %s", ctime (&options.cur_day_start));
#endif /* DEBUG */

  /* Check for -P, -H or -L options. */
  for (i=1; (end_of_leading_options = i) < argc; ++i)
    {
      if (0 == strcmp("-H", argv[i]))
	{
	  /* Meaning: dereference symbolic links on command line, but nowhere else. */
	  set_follow_state(SYMLINK_DEREF_ARGSONLY);
	}
      else if (0 == strcmp("-L", argv[i]))
	{
	  /* Meaning: dereference all symbolic links. */
	  set_follow_state(SYMLINK_ALWAYS_DEREF);
	}
      else if (0 == strcmp("-P", argv[i]))
	{
	  /* Meaning: never dereference symbolic links (default). */
	  set_follow_state(SYMLINK_NEVER_DEREF);
	}
      else if (0 == strcmp("--", argv[i]))
	{
	  /* -- signifies the end of options. */
	  end_of_leading_options = i+1;	/* Next time start with the next option */
	  break;
	}
      else
	{
	  /* Hmm, must be one of 
	   * (a) A path name
	   * (b) A predicate
	   */
	  end_of_leading_options = i; /* Next time start with this option */
	  break;
	}
    }

  /* We are now processing the part of the "find" command line 
   * after the -H/-L options (if any).
   */

  /* fprintf(stderr, "rest: optind=%ld\n", (long)optind); */
  
  /* Find where in ARGV the predicates begin. */
  for (i = end_of_leading_options; i < argc && strchr ("-!(),", argv[i][0]) == NULL; i++)
    {
      /* fprintf(stderr, "Looks like %s is not a predicate\n", argv[i]); */
      /* Do nothing. */ ;
    }
  
  /* Enclose the expression in `( ... )' so a default -print will
     apply to the whole expression. */
  entry_open  = find_parser("(");
  entry_close = find_parser(")");
  entry_print = find_parser("print");
  assert(entry_open  != NULL);
  assert(entry_close != NULL);
  assert(entry_print != NULL);
  
  parse_open (entry_open, argv, &argc);
  parse_begin_user_args(argv, argc, last_pred, predicates);
  pred_sanity_check(last_pred);
  
  /* Build the input order list. */
  while (i < argc)
    {
      if (strchr ("-!(),", argv[i][0]) == NULL)
	usage (_("paths must precede expression"));
      predicate_name = argv[i];
      parse_entry = find_parser (predicate_name);
      if (parse_entry == NULL)
	{
	  /* Command line option not recognized */
	  error (1, 0, _("invalid predicate `%s'"), predicate_name);
	}
      
      i++;
      if (!(*(parse_entry->parser_func)) (parse_entry, argv, &i))
	{
	  if (argv[i] == NULL)
	    /* Command line option requires an argument */
	    error (1, 0, _("missing argument to `%s'"), predicate_name);
	  else
	    error (1, 0, _("invalid argument `%s' to `%s'"),
		   argv[i], predicate_name);
	}

      pred_sanity_check(last_pred);
      pred_sanity_check(predicates); /* XXX: expensive */
    }
  parse_end_user_args(argv, argc, last_pred, predicates);
  
  if (predicates->pred_next == NULL)
    {
      /* No predicates that do something other than set a global variable
	 were given; remove the unneeded initial `(' and add `-print'. */
      cur_pred = predicates;
      predicates = last_pred = predicates->pred_next;
      free ((char *) cur_pred);
      parse_print (entry_print, argv, &argc);
      pred_sanity_check(last_pred); 
      pred_sanity_check(predicates); /* XXX: expensive */
    }
  else if (!default_prints (predicates->pred_next))
    {
      /* One or more predicates that produce output were given;
	 remove the unneeded initial `('. */
      cur_pred = predicates;
      predicates = predicates->pred_next;
      pred_sanity_check(predicates); /* XXX: expensive */
      free ((char *) cur_pred);
    }
  else
    {
      /* `( user-supplied-expression ) -print'. */
      parse_close (entry_close, argv, &argc);
      pred_sanity_check(last_pred);
      parse_print (entry_print, argv, &argc);
      pred_sanity_check(last_pred);
      pred_sanity_check(predicates); /* XXX: expensive */
    }

#ifdef	DEBUG
  fprintf (stderr, "Predicate List:\n");
  print_list (stderr, predicates);
#endif /* DEBUG */

  /* do a sanity check */
  pred_sanity_check(predicates);
  
  /* Done parsing the predicates.  Build the evaluation tree. */
  cur_pred = predicates;
  eval_tree = get_expr (&cur_pred, NO_PREC);

  /* Check if we have any left-over predicates (this fixes
   * Debian bug #185202).
   */
  if (cur_pred != NULL)
    {
      error (1, 0, _("unexpected extra predicate"));
    }
  
#ifdef	DEBUG
  fprintf (stderr, "Eval Tree:\n");
  print_tree (stderr, eval_tree, 0);
#endif /* DEBUG */

  /* Rearrange the eval tree in optimal-predicate order. */
  opt_expr (&eval_tree);

  /* Determine the point, if any, at which to stat the file. */
  mark_stat (eval_tree);
  /* Determine the point, if any, at which to determine file type. */
  mark_type (eval_tree);

#ifdef DEBUG
  fprintf (stderr, "Optimized Eval Tree:\n");
  print_tree (stderr, eval_tree, 0);
  fprintf (stderr, "Optimized command line:\n");
  print_optlist(stderr, eval_tree);
  fprintf(stderr, "\n");
#endif /* DEBUG */

  /* safely_chdir() needs to check that it has ended up in the right place. 
   * To avoid bailing out when something gets automounted, it checks if 
   * the target directory appears to have had a directory mounted on it as
   * we chdir()ed.  The problem with this is that in order to notice that 
   * a filesystem was mounted, we would need to lstat() all the mount points.
   * That strategy loses if our machine is a client of a dead NFS server.
   *
   * Hence if safely_chdir() and wd_sanity_check() can manage without needing 
   * to know the mounted device list, we do that.  
   */
  if (!options.open_nofollow_available)
    {
#ifdef STAT_MOUNTPOINTS
      init_mounted_dev_list();
#endif
    }
  

  starting_desc = open (".", O_RDONLY);
  if (0 <= starting_desc && fchdir (starting_desc) != 0)
    {
      close (starting_desc);
      starting_desc = -1;
    }
  if (starting_desc < 0)
    {
      starting_dir = xgetcwd ();
      if (! starting_dir)
	error (1, errno, _("cannot get current directory"));
    }


  process_all_startpoints(argc-end_of_leading_options, argv+end_of_leading_options);
  
  /* If "-exec ... {} +" has been used, there may be some 
   * partially-full command lines which have been built, 
   * but which are not yet complete.   Execute those now.
   */
  cleanup();
  return state.exit_status;
}

boolean is_fts_enabled()
{
  /* this version of find (i.e. this main()) uses fts. */
  return true;
}
