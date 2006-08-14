/* util.c -- functions for initializing new tree elements, and other things.
   Copyright (C) 1990, 91, 92, 93, 94, 2000, 2003, 2004 Free Software Foundation, Inc.

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
   USA.
*/

#include "defs.h"
#include "xalloc.h"


#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif


#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# define _(Text) Text
#endif
#ifdef gettext_noop
# define N_(String) gettext_noop (String)
#else
/* See locate.c for explanation as to why not use (String) */
# define N_(String) String
#endif

#include <assert.h>


/* Return a pointer to a new predicate structure, which has been
   linked in as the last one in the predicates list.

   Set `predicates' to point to the start of the predicates list.
   Set `last_pred' to point to the new last predicate in the list.
   
   Set all cells in the new structure to the default values. */

struct predicate *
get_new_pred (const struct parser_table *entry)
{
  register struct predicate *new_pred;
  (void) entry;

  /* Options should not be turned into predicates. */
  assert(entry->type != ARG_OPTION);
  assert(entry->type != ARG_POSITIONAL_OPTION);
  
  if (predicates == NULL)
    {
      predicates = (struct predicate *)
	xmalloc (sizeof (struct predicate));
      last_pred = predicates;
    }
  else
    {
      new_pred = (struct predicate *) xmalloc (sizeof (struct predicate));
      last_pred->pred_next = new_pred;
      last_pred = new_pred;
    }
  last_pred->parser_entry = entry;
  last_pred->pred_func = NULL;
#ifdef	DEBUG
  last_pred->p_name = NULL;
#endif	/* DEBUG */
  last_pred->p_type = NO_TYPE;
  last_pred->p_prec = NO_PREC;
  last_pred->side_effects = false;
  last_pred->no_default_print = false;
  last_pred->need_stat = true;
  last_pred->need_type = true;
  last_pred->args.str = NULL;
  last_pred->pred_next = NULL;
  last_pred->pred_left = NULL;
  last_pred->pred_right = NULL;
  last_pred->literal_control_chars = options.literal_control_chars;
  return last_pred;
}

/* Return a pointer to a new predicate, with operator check.
   Like get_new_pred, but it checks to make sure that the previous
   predicate is an operator.  If it isn't, the AND operator is inserted. */

struct predicate *
get_new_pred_chk_op (const struct parser_table *entry)
{
  struct predicate *new_pred;
  static const struct parser_table *entry_and = NULL;

  /* Locate the entry in the parser table for the "and" operator */
  if (NULL == entry_and)
    entry_and = find_parser("and");

  /* Check that it's actually there. If not, that is a bug.*/
  assert(entry_and != NULL);	

  if (last_pred)
    switch (last_pred->p_type)
      {
      case NO_TYPE:
	error (1, 0, _("oops -- invalid default insertion of and!"));
	break;

      case PRIMARY_TYPE:
      case CLOSE_PAREN:
	/* We need to interpose the and operator. */
	new_pred = get_new_pred (entry_and);
	new_pred->pred_func = pred_and;
#ifdef	DEBUG
	new_pred->p_name = find_pred_name (pred_and);
#endif	/* DEBUG */
	new_pred->p_type = BI_OP;
	new_pred->p_prec = AND_PREC;
	new_pred->need_stat = false;
	new_pred->need_type = false;
	new_pred->args.str = NULL;
	new_pred->side_effects = false;
	new_pred->no_default_print = false;
	break;

      default:
	break;
      }
  
  new_pred = get_new_pred (entry);
  new_pred->parser_entry = entry;
  return new_pred;
}

/* Add a primary of predicate type PRED_FUNC (described by ENTRY) to the predicate input list.

   Return a pointer to the predicate node just inserted.

   Fills in the following cells of the new predicate node:

   pred_func	    PRED_FUNC
   args(.str)	    NULL
   p_type	    PRIMARY_TYPE
   p_prec	    NO_PREC

   Other cells that need to be filled in are defaulted by
   get_new_pred_chk_op, which is used to insure that the prior node is
   either not there at all (we are the very first node) or is an
   operator. */

struct predicate *
insert_primary_withpred (const struct parser_table *entry, PRED_FUNC pred_func)
{
  struct predicate *new_pred;

  new_pred = get_new_pred_chk_op (entry);
  new_pred->pred_func = pred_func;
#ifdef	DEBUG
  new_pred->p_name = entry->parser_name;
#endif	/* DEBUG */
  new_pred->args.str = NULL;
  new_pred->p_type = PRIMARY_TYPE;
  new_pred->p_prec = NO_PREC;
  return new_pred;
}

/* Add a primary described by ENTRY to the predicate input list.

   Return a pointer to the predicate node just inserted.

   Fills in the following cells of the new predicate node:

   pred_func	    PRED_FUNC
   args(.str)	    NULL
   p_type	    PRIMARY_TYPE
   p_prec	    NO_PREC

   Other cells that need to be filled in are defaulted by
   get_new_pred_chk_op, which is used to insure that the prior node is
   either not there at all (we are the very first node) or is an
   operator. */
struct predicate *
insert_primary (const struct parser_table *entry)
{
  assert(entry->pred_func != NULL);
  return insert_primary_withpred(entry, entry->pred_func);
}



void
usage (char *msg)
{
  if (msg)
    fprintf (stderr, "%s: %s\n", program_name, msg);
  fprintf (stderr, _("\
Usage: %s [-H] [-L] [-P] [path...] [expression]\n"), program_name);
  exit (1);
}


/* Get the stat information for a file, if it is 
 * not already known. 
 */
int
get_statinfo (const char *pathname, const char *name, struct stat *p)
{
  if (!state.have_stat && (*options.xstat) (name, p) != 0)
    {
      if (!options.ignore_readdir_race || (errno != ENOENT) )
	{
	  error (0, errno, "%s", pathname);
	  state.exit_status = 1;
	}
      return -1;
    }
  state.have_stat = true;
  state.have_type = true;
  state.type = p->st_mode;
  return 0;
}


/* Get the stat/type information for a file, if it is 
 * not already known. 
 */
int
get_info (const char *pathname,
	  const char *name,
	  struct stat *p,
	  struct predicate *pred_ptr)
{
  /* If we need the full stat info, or we need the type info but don't 
   * already have it, stat the file now.
   */
  (void) name;
  if (pred_ptr->need_stat)
    {
      return get_statinfo(pathname, state.rel_pathname, p);
    }
  if ((pred_ptr->need_type && (0 == state.have_type)))
    {
      return get_statinfo(pathname, state.rel_pathname, p);
    }
  return 0;
}

/* Determine if we can use O_NOFOLLOW.
 */
#if defined(O_NOFOLLOW)
boolean 
check_nofollow(void)
{
  struct utsname uts;
  float  release;

  if (0 == uname(&uts))
    {
      /* POSIX requires that atof() ignore "unrecognised suffixes". */
      release = atof(uts.release);
      
      if (0 == strcmp("Linux", uts.sysname))
	{
	  /* Linux kernels 2.1.126 and earlier ignore the O_NOFOLLOW flag. */
	  return release >= 2.2; /* close enough */
	}
      else if (0 == strcmp("FreeBSD", uts.sysname)) 
	{
	  /* FreeBSD 3.0-CURRENT and later support it */
	  return release >= 3.1;
	}
    }

  /* Well, O_NOFOLLOW was defined, so we'll try to use it. */
  return true;
}
#endif



/* Examine the predicate list for instances of -execdir or -okdir
 * which have been terminated with '+' (build argument list) rather
 * than ';' (singles only).  If there are any, run them (this will
 * have no effect if there are no arguments waiting).
 */
void
complete_pending_execdirs(struct predicate *p)
{
#if defined(NEW_EXEC)
  if (NULL == p)
    return;
  
  complete_pending_execdirs(p->pred_left);
  
  if (p->pred_func == pred_execdir || p->pred_func == pred_okdir)
    {
      /* It's an exec-family predicate.  p->args.exec_val is valid. */
      if (p->args.exec_vec.multiple)
	{
	  struct exec_val *execp = &p->args.exec_vec;
	  
	  /* This one was terminated by '+' and so might have some
	   * left... Run it if necessary.
	   */
	  if (execp->state.todo)
	    {
	      /* There are not-yet-executed arguments. */
	      launch (&execp->ctl, &execp->state);
	    }
	}
    }

  complete_pending_execdirs(p->pred_right);
#else
  /* nothing to do. */
  return;
#endif
}


/* Examine the predicate list for instances of -exec which have been
 * terminated with '+' (build argument list) rather than ';' (singles
 * only).  If there are any, run them (this will have no effect if
 * there are no arguments waiting).
 */
void
complete_pending_execs(struct predicate *p)
{
#if defined(NEW_EXEC)
  if (NULL == p)
    return;
  
  complete_pending_execs(p->pred_left);
  
  /* It's an exec-family predicate then p->args.exec_val is valid
   * and we can check it. 
   */
  if (p->pred_func == pred_exec && p->args.exec_vec.multiple)
    {
      struct exec_val *execp = &p->args.exec_vec;
      
      /* This one was terminated by '+' and so might have some
       * left... Run it if necessary.  Set state.exit_status if
       * there are any problems.
       */
      if (execp->state.todo)
	{
	  /* There are not-yet-executed arguments. */
	  launch (&execp->ctl, &execp->state);
	}
    }

  complete_pending_execs(p->pred_right);
#else
  /* nothing to do. */
  return;
#endif
}


/* Complete any outstanding commands.
 */
void 
cleanup(void)
{
  if (eval_tree)
    {
      complete_pending_execs(eval_tree);
      complete_pending_execdirs(eval_tree);
    }
}


static int
fallback_stat(const char *name, struct stat *p, int prev_rv)
{
  /* Our original stat() call failed.  Perhaps we can't follow a
   * symbolic link.  If that might be the problem, lstat() the link. 
   * Otherwise, admit defeat. 
   */
  switch (errno)
    {
    case ENOENT:
    case ENOTDIR:
#ifdef DEBUG_STAT
      fprintf(stderr, "fallback_stat(): stat(%s) failed; falling back on lstat()\n", name);
#endif
      return lstat(name, p);

    case EACCES:
    case EIO:
    case ELOOP:
    case ENAMETOOLONG:
#ifdef EOVERFLOW
    case EOVERFLOW:	    /* EOVERFLOW is not #defined on UNICOS. */
#endif
    default:
      return prev_rv;	       
    }
}


/* optionh_stat() implements the stat operation when the -H option is
 * in effect.
 * 
 * If the item to be examined is a command-line argument, we follow
 * symbolic links.  If the stat() call fails on the command-line item,
 * we fall back on the properties of the symbolic link.
 *
 * If the item to be examined is not a command-line argument, we
 * examine the link itself.
 */
int 
optionh_stat(const char *name, struct stat *p)
{
  if (0 == state.curdepth) 
    {
      /* This file is from the command line; deference the link (if it
       * is a link).  
       */
      int rv = stat(name, p);
      if (0 == rv)
	return 0;		/* success */
      else
	return fallback_stat(name, p, rv);
    }
  else
    {
      /* Not a file on the command line; do not dereference the link.
       */
      return lstat(name, p);
    }
}

/* optionl_stat() implements the stat operation when the -L option is
 * in effect.  That option makes us examine the thing the symbolic
 * link points to, not the symbolic link itself.
 */
int 
optionl_stat(const char *name, struct stat *p)
{
  int rv = stat(name, p);
  if (0 == rv)
    return 0;			/* normal case. */
  else
    return fallback_stat(name, p, rv);
}

/* optionp_stat() implements the stat operation when the -P option is
 * in effect (this is also the default).  That option makes us examine
 * the symbolic link itself, not the thing it points to.
 */
int 
optionp_stat(const char *name, struct stat *p)
{
  return lstat(name, p);
}

#ifdef DEBUG_STAT
static uintmax_t stat_count = 0u;

int
debug_stat (const char *file, struct stat *bufp)
{
  ++stat_count;
  fprintf (stderr, "debug_stat (%s)\n", file);
  switch (options.symlink_handling)
    {
    case SYMLINK_ALWAYS_DEREF:
      return optionl_stat(file, bufp);
    case SYMLINK_DEREF_ARGSONLY:
      return optionh_stat(file, bufp);
    case SYMLINK_NEVER_DEREF:
      return optionp_stat(file, bufp);
    }
}
#endif /* DEBUG_STAT */


int
following_links(void)
{
  switch (options.symlink_handling)
    {
    case SYMLINK_ALWAYS_DEREF:
      return 1;
    case SYMLINK_DEREF_ARGSONLY:
      return (state.curdepth == 0);
    case SYMLINK_NEVER_DEREF:
    default:
      return 0;
    }
}


/* Take a "mode" indicator and fill in the files of 'state'.
 */
int
digest_mode(mode_t mode,
	    const char *pathname,
	    const char *name,
	    struct stat *pstat,
	    boolean leaf)
{
  /* If we know the type of the directory entry, and it is not a
   * symbolic link, we may be able to avoid a stat() or lstat() call.
   */
  if (mode)
    {
      if (S_ISLNK(mode) && following_links())
	{
	  /* mode is wrong because we should have followed the symlink. */
	  if (get_statinfo(pathname, name, pstat) != 0)
	    return 0;
	  mode = state.type = pstat->st_mode;
	  state.have_type = true;
	}
      else
	{
	  state.have_type = true;
	  pstat->st_mode = state.type = mode;
	}
    }
  else
    {
      /* Mode is not yet known; may have to stat the file unless we 
       * can deduce that it is not a directory (which is all we need to 
       * know at this stage)
       */
      if (leaf)
	{
	  state.have_stat = false;
	  state.have_type = false;;
	  state.type = 0;
	}
      else
	{
	  if (get_statinfo(pathname, name, pstat) != 0)
	    return 0;
	  
	  /* If -L is in effect and we are dealing with a symlink,
	   * st_mode is the mode of the pointed-to file, while mode is
	   * the mode of the directory entry (S_IFLNK).  Hence now
	   * that we have the stat information, override "mode".
	   */
	  state.type = pstat->st_mode;
	  state.have_type = true;
	}
    }

  /* success. */
  return 1;
}


/* Return true if there are no predicates with no_default_print in
   predicate list PRED, false if there are any.
   Returns true if default print should be performed */

boolean
default_prints (struct predicate *pred)
{
  while (pred != NULL)
    {
      if (pred->no_default_print)
	return (false);
      pred = pred->pred_next;
    }
  return (true);
}
