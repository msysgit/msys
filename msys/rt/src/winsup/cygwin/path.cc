/* path.cc: path support.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* This module's job is to
   - convert between POSIX and Win32 style filenames,
   - support the `mount' functionality,
   - support symlinks for files and directories

   Pathnames are handled as follows:

   - A \ or : in a path denotes a pure windows spec.
   - Paths beginning with // (or \\) are not translated (i.e. looked
     up in the mount table) and are assumed to be UNC path names.

   The goal in the above set of rules is to allow both POSIX and Win32
   flavors of pathnames without either interfering.  The rules are
   intended to be as close to a superset of both as possible.

   Note that you can have more than one path to a file.  The mount
   table is always prefered when translating Win32 paths to POSIX
   paths.  Win32 paths in mount table entries may be UNC paths or
   standard Win32 paths starting with <drive-letter>:

   Text vs Binary issues are not considered here in path style
   decisions, although the appropriate flags are retrieved and
   stored in various structures.

   Removing mounted filesystem support would simplify things greatly,
   but having it gives us a mechanism of treating disk that lives on a
   UNIX machine as having UNIX semantics [it allows one to edit a text
   file on that disk and not have cr's magically appear and perhaps
   break apps running on UNIX boxes].  It also useful to be able to
   layout a hierarchy without changing the underlying directories.

   The semantics of mounting file systems is not intended to precisely
   follow normal UNIX systems.

   Each DOS drive is defined to have a current directory.  Supporting
   this would complicate things so for now things are defined so that
   c: means c:\.  FIXME: Is this still true?
*/

#ifndef NEW_PATH_METHOD
# define NEW_PATH_METHOD 1
#endif

#ifndef NO_SYMLINK
# define NO_SYMLINK 1
#endif

#include "winsup.h"
#include "msys.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <mntent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <winioctl.h>
#include <wingdi.h>
#include <winuser.h>
#include <winnls.h>
#include <winnetwk.h>
#include <sys/cygwin.h>
#include <cygwin/version.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "registry.h"
#include <assert.h>
#include "shortcut.h"
#include "msys.h"

#ifdef _MT_SAFE
#define iteration _reent_winsup ()->_iteration
#define available_drives _reent_winsup ()->available_drives
#else
static int iteration;
static DWORD available_drives;
#endif

static int normalize_win32_path (const char *src, char *dst);
static void slashify (const char *src, char *dst, int trailing_slash_p);
static void backslashify (const char *src, char *dst, int trailing_slash_p);

struct symlink_info
{
  char contents[MAX_PATH + 4];
  char *ext_here;
  int extn;
  unsigned pflags;
  DWORD fileattr;
  int is_symlink;
  bool ext_tacked_on;
  int error;
  BOOL case_clash;
  int check (char *path, const suffix_info *suffixes, unsigned opt);
  BOOL case_check (char *path);
};

int pcheck_case = PCHECK_RELAXED; /* Determines the case check behaviour. */

/* Determine if path prefix matches current cygdrive */
#define iscygdrive(path) \
  (path_prefix_p (mount_table->cygdrive, (path), mount_table->cygdrive_len))

#define iscygdrive_device(path) \
  (iscygdrive(path) && isalpha(path[mount_table->cygdrive_len]) && \
   (isdirsep(path[mount_table->cygdrive_len + 1]) || \
    !path[mount_table->cygdrive_len + 1]))

/* Return non-zero if PATH1 is a prefix of PATH2.
   Both are assumed to be of the same path style and / vs \ usage.
   Neither may be "".
   LEN1 = strlen (PATH1).  It's passed because often it's already known.

   Examples:
   /foo/ is a prefix of /foo  <-- may seem odd, but desired
   /foo is a prefix of /foo/
   / is a prefix of /foo/bar
   / is not a prefix of foo/bar
   foo/ is a prefix foo/bar
   /foo is not a prefix of /foobar
*/

int
path_prefix_p (const char *path1, const char *path2, int len1)
{
  TRACE_IN;
  /* Handle case where PATH1 has trailing '/' and when it doesn't.  */
  if (len1 > 0 && SLASH_P (path1[len1 - 1]))
    len1--;

  if (len1 == 0)
    return SLASH_P (path2[0]) && !SLASH_P (path2[1]);

  if (!pathnmatch (path1, path2, len1))
    return 0;

  return SLASH_P (path2[len1]) || path2[len1] == 0 || path1[len1 - 1] == ':';
}

/* Return non-zero if paths match in first len chars. */
int
pathnmatch (const char *path1, const char *path2, int len)
{
  TRACE_IN;
  debug_printf("pathnmatch(%s, %s, %d))", path1, path2, len);
  // Paths of just dots can't be matched so don't say they are.
  if (path1[0] == '.' && path2[0] == '.')
    {
      if (len > 1 && path1[1] && path2[1])
	{
	  if (path1[1] == '.' && path2[1] == '.')
	      return 0;
	}
      else
	  return 0;
    }
  return (strncasecmp (path1, path2, len) ? 0 : 1);
}

/* Return non-zero if paths match. */
int
pathmatch (const char *path1, const char *path2)
{
  TRACE_IN;
  debug_printf("pathmatch(path1=%s, path2=%s))", path1, path2);
  // Paths of just dots can't be matched so don't say they are.
  if (! path1 || ! path2 || !*path1 || !*path2)
    {
      debug_printf("Path length 0 or not initialized");
      return 0;
    }
  if (strlen (path1) > MAX_PATH || strlen (path2) > MAX_PATH)
    {
      debug_printf("Maximum path exceeded");
      return 0;
    }
  if (path1[0] == '.' && path2[0] == '.')
    {
      if (path1[1] && path2[1])
	{
	  if (path1[1] == '.' && path2[1] == '.')
	      return 0;
	}
      else
	  return 0;
    }
  return (strcasecmp (path1, path2) ? 0 : 1);
}

/* Normalize a POSIX path.
   \'s are converted to /'s in the process.
   All duplicate /'s, except for 2 leading /'s, are deleted.
   The result is 0 for success, or an errno error value.  */

#define isslash(c) ((c) == '/')

int
normalize_posix_path (const char *src, char *dst)
{
  TRACE_IN;
  const char *src_start = src;
  char *dst_start = dst;
  static char last_src[MAX_PATH] = "\0";
  static char last_dst[MAX_PATH] = "\0";
  if (pathmatch(src, last_src))
    {
      strcpy (dst, last_dst);
      return 0;
    }
  strncpy (last_src, src, MAX_PATH);

  syscall_printf ("src %s", src);
  syscall_printf ("dst %s", dst);
  if (!strcmp (src, "//"))
      src_start = ++src;
  if (isdrive (src) || strpbrk (src, "\\:") || 
      (isslash (src[0]) && isslash(src[1])))
    {
      cygwin_conv_to_full_posix_path (src, dst);
      strcpy (last_dst, dst_start);
      return 0;
    }
  if (!isslash (src[0]))
    {
      if (!cygheap->cwd.get (dst))
	return get_errno ();
      syscall_printf("src %s", src);
      syscall_printf("dst %s", dst);
      dst = strchr (dst, '\0');
      if (*src == '.')
	{
	  if (dst == dst_start + 1 && *dst_start == '/')
	     --dst;
	  goto sawdot;
	}
      if (dst > dst_start && !isslash (dst[-1]))
	*dst++ = '/';
      syscall_printf("dst %s", dst);
    }
  /* Two leading /'s?  If so, preserve them.  */
  else if (isslash (src[1]))
    {
      *dst++ = '/';
      *dst++ = '/';
      src += 2;
      if (isslash (*src))
	{ /* Starts with three or more slashes - reset. */
	  dst = dst_start;
	  *dst++ = '/';
	  src = src_start + 1;
	}
    }
  else
    *dst = '\0';

  while (*src)
    {
      /* Strip runs of /'s.  */
      if (!isslash (*src))
	*dst++ = *src++;
      else
	{
	  while (*++src)
	    {
	      if (isslash (*src))
		continue;

	      if (*src != '.')
		break;

	    sawdot:
	      if (src[1] != '.')
		{
		  if (!src[1])
		    {
		      if (dst == dst_start)
			*dst++ = '/';
		      goto done;
		    }
		  if (!isslash (src[1]))
		    break;
		}
	      else if (src[2] && !isslash (src[2]))
		break;
	      else
		{
		  while (dst > dst_start && !isslash (*--dst))
		    continue;
		  src++;
		}
	    }

	  *dst++ = '/';
	}
	if ((dst - dst_start) >= MAX_PATH)
	  {
	    debug_printf ("ENAMETOOLONG = normalize_posix_path (%s)", src);
	    return ENAMETOOLONG;
	  }
    }

done:
  *dst = '\0';
  if (--dst > dst_start && isslash (*dst))
    *dst = '\0';

  debug_printf ("%s = normalize_posix_path (%s)", dst_start, src_start);
  strcpy (last_dst, dst_start);
  return 0;
}

inline void
path_conv::add_ext_from_sym (symlink_info &sym)
{
  TRACE_IN;
  if (sym.ext_here && *sym.ext_here)
    {
      known_suffix = path + sym.extn;
      if (sym.ext_tacked_on)
	strcpy (known_suffix, sym.ext_here);
    }
}

static void __stdcall mkrelpath (char *dst) __attribute__ ((regparm (2)));
static void __stdcall
mkrelpath (char *path)
{
  TRACE_IN;
  char cwd_win32[MAX_PATH];
  debug_printf("entr %s", path);
  if (!cygheap->cwd.get (cwd_win32, 0))
    return;

  unsigned cwdlen = strlen (cwd_win32);
  if (!path_prefix_p (cwd_win32, path, cwdlen))
    return;

  size_t n = strlen (path);
  if (n < cwdlen)
    return;

  char *tail = path;
  if (n == cwdlen)
    tail += cwdlen;
  else
    tail += isdirsep (cwd_win32[cwdlen - 1]) ? cwdlen : cwdlen + 1;

  memmove (path, tail, strlen (tail) + 1);
  if (!*path)
    strcpy (path, ".");
  debug_printf("exit %s", path);
}

void
path_conv::update_fs_info (const char* win32_path)
{
  TRACE_IN;
  char tmp_buf [MAX_PATH];
  strncpy (tmp_buf, win32_path, MAX_PATH);

  if (!rootdir (tmp_buf))
    {
      debug_printf ("Cannot get root component of path %s", win32_path);
      root_dir [0] = fs_name [0] = '\0';
      fs_flags = fs_serial = 0;
      sym_opt = 0;
      return;
    }

  if (strcmp (tmp_buf, root_dir) != 0)
    {
      strncpy (root_dir, tmp_buf, MAX_PATH);
      drive_type = GetDriveType (root_dir);
      if (drive_type == DRIVE_REMOTE || (drive_type == DRIVE_UNKNOWN && (root_dir[0] == '\\' && root_dir[1] == '\\')))
	is_remote_drive = 1;
      else
	is_remote_drive = 0;

      if (!GetVolumeInformation (root_dir, NULL, 0, &fs_serial, NULL, &fs_flags,
				     fs_name, sizeof (fs_name)))
	{
	  debug_printf ("Cannot get volume information (%s), %E", root_dir);
	  fs_name [0] = '\0';
	  fs_flags = fs_serial = 0;
	  sym_opt = 0;
	}
      else
	{
	  /* FIXME: Samba by default returns "NTFS" in file system name, but
	   * doesn't support Extended Attributes. If there's some fast way to
	   * distinguish between samba and real ntfs, it should be implemented
	   * here.
	   */
	  sym_opt = (!is_remote_drive && strcmp (fs_name, "NTFS") == 0) ? PC_CHECK_EA : 0;
	}
    }
}

/* Convert an arbitrary path SRC to a pure Win32 path, suitable for
   passing to Win32 API routines.

   If an error occurs, `error' is set to the errno value.
   Otherwise it is set to 0.

   follow_mode values:
	SYMLINK_FOLLOW	    - convert to PATH symlink points to
	SYMLINK_NOFOLLOW    - convert to PATH of symlink itself
	SYMLINK_IGNORE	    - do not check PATH for symlinks
	SYMLINK_CONTENTS    - just return symlink contents
*/

void
path_conv::check (const char *src, unsigned opt,
		  const suffix_info *suffixes)
{
  TRACE_IN;
  /* ******************************************************************
   * So what do we need to check, or what the &*(# is this doing?
   * 
   * Symlinks are out but the coding for all path checking
   * appears to be embedded with symlink checking so we have
   * misleading names below.
   * 
   * A class suffix_scan is used in symlink_info::check to find the
   * extension based on the suffix_info passed to this function.
   * symlink_info is both a class and a struct.
   *
   * Now, to answer the question:
   * First, let's remember that we're a member of the path_conv class.
   * Second, let's remember that we're passed the path to check, A
   *   bit mask set of options, and a list of suffixes.
   * So, let's check what the valid options are to perform on *src.
   * 1) PC_NULLEMPTY: Appears to cause an error to be set if the src
   *    is 0 or *src = '\0'
   * 2) PC_SYM_IGNORE: Doesn't do symlink checking including the path
   *    extention.
   * 3) PC_SYM_CONTENTS: Then return the contents of the symlink;
   *    I.E.: the file pointed to.
   * ******************************************************************
   * pcheck_case == PCHECK_RELAXED used with opt & PC_SYM_IGNORE.
   * pcheck_case == PCHECK_STRICT used with sym.case_clase;
   * sym.case_clash is set via the previous sym.check call and sym is
   * an object of the symlink_info class.
   * pcheck_case == PCHECK_ADJUST allows a path creator to maintain
   * the current case of a file if the file exists and the case for
   * the new name differs.
   * ******************************************************************
   * FIXME: All we really need path_conv::check to do is:
   * 1) Does the value of src exists in the file system?
   *    Y) Does case of file system name differ with the value of src?
   *       Y) Report File Not Found.
   *       N) Report File Found
   *    N) Does the value of src contain a '.'?
   *       Y) Report File Not Found.
   *       N) Do 2).
   * 2) Loop through values of suffix_info appending the suffix
   *    to the value of src and recursive call path_conv::check.
   * 3) Successful path_conv::check?
   *    Y) Report File Found.
   *    N) Report File Not Found.
   * 4) FUTURE ENHANCEMENT: Create a hash value for the value of src
   *    and check for that value first for existing files.
   * ******************************************************************
   * Further analysis needs to be done for the use of the symlink_info
   * struct to see if that needs to have values filled.
   * *****************************************************************/

// This isn't working.  Giving much to do with symlink_info.  I've modified
// other methods to remove some symlink checking.  This needs a complete
// rework including all calls.

// This is also used in the path_conv constructor for initialization!!!!

#if 0 // #if NEW_PATH_METHOD
    FIXME: Please!!!
    Using the above analysis, rewrite this function.
#else
  /* This array is used when expanding symlinks.  It is MAX_PATH * 2
     in length so that we can hold the expanded symlink plus a
     trailer.  */
  char tmp_buf[2 * MAX_PATH + 3];
  char path_copy[MAX_PATH + 3];
  symlink_info sym;
  bool need_directory = 0;
  bool saw_symlinks = 0;
  int is_relpath;
  sigframe thisframe (mainthread);

  int loop = 0;
  path_flags = 0;
  known_suffix = NULL;
  fileattr = (DWORD) -1;
  case_clash = FALSE;
  devn = unit = 0;
  root_dir[0] = '\0';
  fs_name[0] = '\0';
  fs_flags = fs_serial = 0;
  sym_opt = 0;
  drive_type = 0;
  is_remote_drive = 0;

  //MSYS - See if this works
  //FIXME: If it does work then what can we remove from this function
  //
  //opt |= PC_SYM_IGNORE;
  //OK, it doesn't work and the reason is that foo is treated as a symlink for
  //foo.exe.  So, now maybe we can go clean this up.

  if (!(opt & PC_NULLEMPTY))
    error = 0;
  else if ((error = check_null_empty_str (src)))
    return;

  /* This loop handles symlink expansion.  */
  for (;;)
    {
      //What's this macro do?
      MALLOC_CHECK;
      assert (src);

      char *p = strrchr (src, '\0');
      /* Detect if the user was looking for a directory.  We have to strip the
	 trailing slash initially and add it back on at the end due to Windows
	 brain damage. */
      if (--p > src)
	{
	  if (isdirsep (*p))
	    need_directory = 1;
	  else if (--p  > src && p[1] == '.' && isdirsep (*p))
	    need_directory = 1;
	}

      is_relpath = !isabspath (src);
      error = normalize_posix_path (src, path_copy);
      if (error)
	return;

      char *tail = strchr (path_copy, '\0');   // Point to end of copy
      char *path_end = tail;
      tail[1] = '\0';  //FIXME: Ain't this dangerous?!?!?

      /* Scan path_copy from right to left looking either for a symlink
	 or an actual existing file.  If an existing file is found, just
	 return.  If a symlink is found exit the for loop.
	 Also: be careful to preserve the errno returned from
	 symlink.check as the caller may need it. */
      /* FIXME: Do we have to worry about multiple \'s here? */
      int component = 0;		// Number of translated components
      sym.contents[0] = '\0';

      for (;;)
	{
	  const suffix_info *suff;
	  char pathbuf[MAX_PATH];
	  char *full_path;

	  /* Don't allow symlink.check to set anything in the path_conv
	     class if we're working on an inner component of the path */
	  if (component)
	    {
	      suff = NULL;
	      sym.pflags = 0;
	      full_path = pathbuf;
	    }
	  else
	    {
	      suff = suffixes;
	      sym.pflags = path_flags;
	      full_path = this->path;
	    }

	  /* Convert to native path spec sans symbolic link info. */
	  error = mount_table->conv_to_win32_path (path_copy, full_path, devn,
						   unit, &sym.pflags, 1);

	  if (error)
	    return;

	  update_fs_info (full_path);

	  /* devn should not be a device.  If it is, then stop parsing now. */
	  if (devn != FH_BAD)
	    {
	      fileattr = 0;
	      goto out;		/* Found a device.  Stop parsing. */
	    }

	  /* Eat trailing slashes */
	  char *dostail = strchr (full_path, '\0');

	  /* 
	   * If path is only a drivename, 
	   * Windows interprets it as the current working directory on 
	   * this drive instead of the root dir which is not what we want. 
	   * So we need the trailing backslash in this case. 
	   */
	  while (dostail > full_path + 3 && (*--dostail == '\\'))
	    *tail = '\0';

	  if (full_path[0] && full_path[1] == ':' && full_path[2] == '\0')
	    {
	      full_path[2] = '\\';
	      full_path[3] = '\0';
	    }

	  if ((opt & PC_SYM_IGNORE) && pcheck_case == PCHECK_RELAXED)
	    {
	      fileattr = GetFileAttributesA (full_path);
	      goto out;
	    }

	  int len = sym.check (full_path, suff, opt | sym_opt);

	  if (sym.case_clash)
	    {
	      if (pcheck_case == PCHECK_STRICT)
		{
		  case_clash = TRUE;
		  error = ENOENT;
		  goto out;
		}
	      /* If pcheck_case==PCHECK_ADJUST the case_clash is remembered
		 if the last component is concerned. This allows functions
		 which shall create files to avoid overriding already existing
		 files with another case. */
	      if (!component)
		case_clash = TRUE;
	    }

	  // since I don't care about symlinks I can get rid of this, right?
	  // No!! We can't find any thing on PATH if we don't do this.?!?!
	  if (!(opt & PC_SYM_IGNORE))
	    {
	      if (!component)
		path_flags = sym.pflags;

	      /* If symlink.check found an existing non-symlink file, then
		 it sets the appropriate flag.  It also sets any suffix found
		 into `ext_here'. */
	      if (!sym.is_symlink && sym.fileattr != (DWORD) -1)
		{
		  error = sym.error;
		  if (component == 0)
		    {
		      fileattr = sym.fileattr;
		      add_ext_from_sym (sym);
		    }
		  if (pcheck_case == PCHECK_RELAXED)
		    goto out;	// file found
		  /* Avoid further symlink evaluation. Only case checks are
		     done now. */
		  opt |= PC_SYM_IGNORE;
		}
	      /* Found a symlink if len > 0.  If component == 0, then the
		 src path itself was a symlink.  If !follow_mode then
		 we're done.  Otherwise we have to insert the path found
		 into the full path that we are building and perform all of
		 these operations again on the newly derived path. */
	      else if (len > 0)
		{
		  saw_symlinks = 1;
		  if (component == 0 && !need_directory && !(opt & PC_SYM_FOLLOW))
		    {
		      set_symlink (); // last component of path is a symlink.
		      fileattr = sym.fileattr;
		      if (opt & PC_SYM_CONTENTS)
			{
			  strcpy (path, sym.contents);
			  goto out;
			}
		      add_ext_from_sym (sym);
		      if (pcheck_case == PCHECK_RELAXED)
			goto out;
		      /* Avoid further symlink evaluation. Only case checks are
			 done now. */
		      opt |= PC_SYM_IGNORE;
		    }
		  else
		    break;
		}
	      /* No existing file found. */
	    }

	  /* Find the "tail" of the path, e.g. in '/for/bar/baz',
	     /baz is the tail. */
	  char *newtail = strrchr (path_copy, '/');
	  if (tail != path_end)
	    *tail = '/';

	  /* Exit loop if there is no tail or we are at the
	     beginning of a UNC path */
	  if (!newtail || newtail == path_copy || (newtail == path_copy + 1 && newtail[-1] == '/'))
	    goto out;	// all done

	  tail = newtail;

	  /* Haven't found an existing pathname component yet.
	     Pinch off the tail and try again. */
	  *tail = '\0';
	  component++;
	}

      /* Arrive here if above loop detected a symlink. */
      if (++loop > MAX_LINK_DEPTH)
	{
	  error = ELOOP;   // Eep.
	  return;
	}

      MALLOC_CHECK;

      /* The tail is pointing at a null pointer.  Increment it and get the length.
	 If the tail was empty then this increment will end up pointing to the extra
	 \0 added to path_copy above. */
      int taillen = strlen (++tail);
      int buflen = strlen (sym.contents);
      if (buflen + taillen > MAX_PATH)
	  {
	    error = ENAMETOOLONG;
	    strcpy (path, "::ENAMETOOLONG::");
	    return;
	  }

      /* Strip off current directory component since this is the part that refers
	 to the symbolic link. */
      if ((p = strrchr (path_copy, '/')) == NULL)
	p = path_copy;
      else if (p == path_copy)
	p++;
      *p = '\0';

      char *headptr;
      if (isabspath (sym.contents))
	headptr = tmp_buf;	/* absolute path */
      else
	{
	  /* Copy the first part of the path and point to the end. */
	  strcpy (tmp_buf, path_copy);
	  headptr = strchr (tmp_buf, '\0');
	}

      /* See if we need to separate first part + symlink contents with a / */
      if (headptr > tmp_buf && headptr[-1] != '/')
	*headptr++ = '/';

      /* Copy the symlink contents to the end of tmp_buf.
	 Convert slashes.  FIXME? */
      for (p = sym.contents; *p; p++)
	*headptr++ = *p == '\\' ? '/' : *p;

      /* Copy any tail component */
      if (tail >= path_end)
	*headptr = '\0';
      else
	{
	  *headptr++ = '/';
	  strcpy (headptr, tail);
	}

      /* Now evaluate everything all over again. */
      src = tmp_buf;
    }

  if (!(opt & PC_SYM_CONTENTS))
    add_ext_from_sym (sym);

out:
  /* Deal with Windows stupidity which considers filename\. to be valid
     even when "filename" is not a directory. */
  if (!need_directory || error)
    /* nothing to do */;
  else if (fileattr & FILE_ATTRIBUTE_DIRECTORY)
    path_flags &= ~PATH_SYMLINK;
  else
    {
      debug_printf ("%s is a non-directory", path);
      error = ENOTDIR;
      return;
    }

  update_fs_info (path);
  if (!fs_name[0])
    {
      set_has_acls (FALSE);
      set_has_buggy_open (FALSE);
    }
  else
    {
      set_isdisk ();
      debug_printf ("root_dir(%s), this->path(%s), set_has_acls(%d)",
		    root_dir, this->path, fs_flags & FS_PERSISTENT_ACLS);
      if (!allow_smbntsec && is_remote_drive)
	set_has_acls (FALSE);
      else
	set_has_acls (fs_flags & FS_PERSISTENT_ACLS);
      /* Known file systems with buggy open calls. Further explanation
	 in fhandler.cc (fhandler_disk_file::open). */
      set_has_buggy_open (strcmp (fs_name, "SUNWNFS") == 0);
    }

  if (!(opt & PC_FULL))
    {
      if (is_relpath)
	mkrelpath (this->path);
      if (need_directory)
	{
	  char n = strlen (this->path);
	  /* Do not add trailing \ to UNC device names like \\.\a: */
	  if (this->path[n - 1] != '\\' &&
	      (strncmp (this->path, "\\\\.\\", 4) != 0 ||
	       !strncasematch (this->path + 4, "unc\\", 4)))
	    {
	      this->path[n] = '\\';
	      this->path[n + 1] = '\0';
	    }
	}
    }

  if (saw_symlinks)
    set_has_symlinks ();

  if (!error && !(path_flags & (PATH_ALL_EXEC | PATH_NOTEXEC)))
    {
      const char *p = strchr (path, '\0') - 4;
      if (p >= path &&
	  (strcasematch (".exe", p) ||
	   strcasematch (".bat", p) ||
	   strcasematch (".com", p)))
	path_flags |= PATH_EXEC;
    }

#endif
}

#define deveq(s) (strcasematch (name, (s)))
#define deveqn(s, n) (strncasematch (name, (s), (n)))

static __inline int
digits (const char *name)
{
  TRACE_IN;
  char *p;
  int n = strtol(name, &p, 10);

  return p > name && !*p ? n : -1;
}

const char *windows_device_names[] NO_COPY =
{
  NULL,
  "\\dev\\console",
  "conin",
  "conout",
  "\\dev\\ttym",
  "\\dev\\tty%d",
  "\\dev\\ptym",
  "\\\\.\\com%d",
  "\\dev\\pipe",
  "\\dev\\piper",
  "\\dev\\pipew",
  "\\dev\\socket",
  "\\dev\\windows",

  NULL, NULL, NULL,

  "\\dev\\disk",
  "\\dev\\fd%d",
  "\\dev\\st%d",
  "nul",
  "\\dev\\zero",
  "\\dev\\%srandom",
  "\\dev\\mem",
  "\\dev\\clipboard",
  "\\dev\\dsp"
};

static int
get_raw_device_number (const char *uxname, const char *w32path, int &unit)
{
  TRACE_IN;
  DWORD devn = FH_BAD;

  if (strncasematch (w32path, "\\\\.\\tape", 8))
    {
      devn = FH_TAPE;
      unit = digits (w32path + 8);
      // norewind tape devices have leading n in name
      if (strncasematch (uxname, "/dev/n", 6))
	unit += 128;
    }
  else if (isdrive (w32path + 4))
    {
      devn = FH_FLOPPY;
      unit = cyg_tolower (w32path[4]) - 'a';
    }
  else if (strncasematch (w32path, "\\\\.\\physicaldrive", 17))
    {
      devn = FH_FLOPPY;
      unit = digits (w32path + 17) + 128;
    }
  return devn;
}

int __stdcall
get_device_number (const char *name, int &unit, BOOL from_conv)
{
  TRACE_IN;
  DWORD devn = FH_BAD;
  unit = 0;

  if ((*name == '/' && deveqn ("/dev/", 5)) ||
      (*name == '\\' && deveqn ("\\dev\\", 5)))
    {
      name += 5;
      if (deveq ("tty"))
	{
	  if (real_tty_attached (myself))
	    {
	      unit = myself->ctty;
	      devn = FH_TTYS;
	    }
	  else if (myself->ctty > 0)
	    devn = FH_CONSOLE;
	}
      else if (deveqn ("tty", 3) && (unit = digits (name + 3)) >= 0)
	devn = FH_TTYS;
      else if (deveq ("ttym"))
	devn = FH_TTYM;
      else if (deveq ("ptmx"))
	devn = FH_PTYM;
      else if (deveq ("windows"))
	devn = FH_WINDOWS;
      else if (deveq ("dsp"))
	devn = FH_OSS_DSP;
      else if (deveq ("conin"))
	devn = FH_CONIN;
      else if (deveq ("conout"))
	devn = FH_CONOUT;
      else if (deveq ("null"))
	devn = FH_NULL;
      else if (deveq ("zero"))
	devn = FH_ZERO;
      else if (deveq ("random") || deveq ("urandom"))
	{
	  devn = FH_RANDOM;
	  unit = 8 + (deveqn ("u", 1) ? 1 : 0); /* Keep unit Linux conformant */
	}
      else if (deveq ("mem"))
	{
	  devn = FH_MEM;
	  unit = 1;
	}
      else if (deveq ("clipboard"))
	devn = FH_CLIPBOARD;
      else if (deveq ("port"))
	{
	  devn = FH_MEM;
	  unit = 4;
	}
      else if (deveqn ("com", 3) && (unit = digits (name + 3)) >= 0)
	devn = FH_SERIAL;
      else if (deveqn ("ttyS", 4) && (unit = digits (name + 4)) >= 0)
	devn = FH_SERIAL;
      else if (deveq ("pipe") || deveq ("piper") || deveq ("pipew"))
	devn = FH_PIPE;
      else if (deveq ("tcp") || deveq ("udp") || deveq ("streamsocket")
	       || deveq ("dgsocket"))
	devn = FH_SOCKET;
      else if (!from_conv)
	devn = get_raw_device_number (name - 5,
				      path_conv (name - 5,
						 PC_SYM_IGNORE).get_win32 (),
				      unit);
    }
  else if (deveqn ("com", 3) && (unit = digits (name + 3)) >= 0)
    devn = FH_SERIAL;
  else if (deveqn ("ttyS", 4) && (unit = digits (name + 4)) >= 0)
    devn = FH_SERIAL;

  return devn;
}

/* Return TRUE if src_path is a Win32 device name, filling out the device
   name in win32_path */

static BOOL
win32_device_name (const char *src_path, char *win32_path,
		   DWORD &devn, int &unit)
{
  TRACE_IN;
  const char *devfmt;

  devn = get_device_number (src_path, unit, TRUE);

  if (devn == FH_BAD)
    return FALSE;

  if ((devfmt = windows_device_names[FHDEVN (devn)]) == NULL)
    return FALSE;
  if (devn == FH_RANDOM)
    __small_sprintf (win32_path, devfmt, unit == 8 ? "" : "u");
  else
    __small_sprintf (win32_path, devfmt, unit);
  return TRUE;
}

/* Normalize a Win32 path.
   /'s are converted to \'s in the process.
   All duplicate \'s, except for 2 leading \'s, are deleted.

   The result is 0 for success, or an errno error value.
   FIXME: A lot of this should be mergeable with the POSIX critter.  */
static int
normalize_win32_path (const char *src, char *dst)
{
  TRACE_IN;
  const char *src_start = src;
  char *dst_start = dst;
  char *dst_root_start = dst;
  bool beg_src_slash = isdirsep (src[0]);

  if (beg_src_slash && isdirsep (src[1]))
    {
      *dst++ = '\\';
      ++src;
    }
  else if (strchr (src, ':') == NULL && *src != '/')
    {
      if (!cygheap->cwd.get (dst, 0))
	return get_errno ();
      if (beg_src_slash)
	{
	  if (dst[1] == ':')
	    dst[2] = '\0';
	  else if (slash_unc_prefix_p (dst))
	    {
	      char *p = strpbrk (dst + 2, "\\/");
	      if (p && (p = strpbrk (p + 1, "\\/")))
		  *p = '\0';
	    }
	}
      if (strlen (dst) + 1 + strlen (src) >= MAX_PATH)
	{
	  debug_printf ("ENAMETOOLONG = normalize_win32_path (%s)", src);
	  return ENAMETOOLONG;
	}
      dst += strlen (dst);
      if (!beg_src_slash)
	*dst++ = '\\';
    }

  while (*src)
    {
      /* Strip duplicate /'s.  */
      if (SLASH_P (src[0]) && SLASH_P (src[1]))
	src++;
      /* Ignore "./".  */
      else if (src[0] == '.' && SLASH_P (src[1])
	       && (src == src_start || SLASH_P (src[-1])))
	src += 2;

      /* Backup if "..".  */
      else if (src[0] == '.' && src[1] == '.'
	       /* dst must be greater than dst_start */
	       && dst[-1] == '\\'
	       && (SLASH_P (src[2]) || src[2] == 0))
	{
	  /* Back up over /, but not if it's the first one.  */
	  if (dst > dst_root_start + 1)
	    dst--;
	  /* Now back up to the next /.  */
	  while (dst > dst_root_start + 1 && dst[-1] != '\\' && dst[-2] != ':')
	    dst--;
	  src += 2;
	  if (SLASH_P (*src))
	    src++;
	}
      /* Otherwise, add char to result.  */
      else
	{
	  if (*src == '/')
	    *dst++ = '\\';
	  else
	    *dst++ = *src;
	  ++src;
	}
      if ((dst - dst_start) >= MAX_PATH)
	return ENAMETOOLONG;
    }
  *dst = 0;
  debug_printf ("%s = normalize_win32_path (%s)", dst_start, src_start);
  return 0;
}

/* Various utilities.  */

/* slashify: Convert all back slashes in src path to forward slashes
   in dst path.  Add a trailing slash to dst when trailing_slash_p arg
   is set to 1. */

static void
slashify (const char *src, char *dst, int trailing_slash_p)
{
  TRACE_IN;
  const char *start = src;

  while (*src)
    {
      if (*src == '\\')
	*dst++ = '/';
      else
	*dst++ = *src;
      ++src;
    }
  if (trailing_slash_p
      && src > start
      && !isdirsep (src[-1]))
    *dst++ = '/';
  *dst++ = 0;
}

/* backslashify: Convert all forward slashes in src path to back slashes
   in dst path.  Add a trailing slash to dst when trailing_slash_p arg
   is set to 1. */

static void
backslashify (const char *src, char *dst, int trailing_slash_p)
{
  TRACE_IN;
  const char *start = src;

  while (*src)
    {
      if (*src == '/')
	*dst++ = '\\';
      else
	*dst++ = *src;
      ++src;
    }
  if (trailing_slash_p
      && src > start
      && !isdirsep (src[-1]))
    *dst++ = '\\';
  *dst++ = 0;
}

/* nofinalslash: Remove trailing / and \ from SRC (except for the
   first one).  It is ok for src == dst.  */

void __stdcall
nofinalslash (const char *src, char *dst)
{
  TRACE_IN;
  int len = strlen (src);
  if (src != dst)
    memcpy (dst, src, len + 1);
  while (len > 1 && SLASH_P (dst[--len]))
    dst[len] = '\0';
}

/* slash_unc_prefix_p: Return non-zero if PATH begins with //UNC/SHARE */

int __stdcall
slash_unc_prefix_p (const char *path)
{
  TRACE_IN;
  char *p = NULL;
  int ret = (isdirsep (path[0])
	     && isdirsep (path[1])
	     && isalpha (path[2])
	     && path[3] != 0
	     && !isdirsep (path[3])
	     && ((p = strpbrk(path + 3, "\\/")) != NULL));
  if (!ret || p == NULL)
    return ret;
  return ret && isalnum (p[1]);
}

/* conv_path_list: Convert a list of path names to/from Win32/POSIX.

   SRC is not a const char * because we temporarily modify it to ease
   the implementation.

   I believe Win32 always has '.' in $PATH.   POSIX obviously doesn't.
   We certainly don't want to handle that here, but it is something for
   the caller to think about.  */

static void
conv_path_list (const char *src, char *dst, int to_posix_p)
{
  TRACE_IN;
  char *s;
  char *d = dst;
  char src_delim = to_posix_p ? ';' : ':';
  char dst_delim = to_posix_p ? ':' : ';';
  int (*conv_fn) (const char *, char *) = (to_posix_p
					   ? cygwin_conv_to_posix_path
					   : cygwin_conv_to_win32_path);

  do
    {
      s = strchr (src, src_delim);
      if (s)
	{
	  *s = 0;
	  (*conv_fn) (src[0] != 0 ? src : ".", d);
	  d += strlen (d);
	  *d++ = dst_delim;
	  *s = src_delim;
	  src = s + 1;
	}
      else
	{
	  /* Last one.  */
	  (*conv_fn) (src[0] != 0 ? src : ".", d);
	}
    }
  while (s != NULL);
}

/* init: Initialize the mount table.  */

void
mount_info::init ()
{
  TRACE_IN;
  nmounts = 0;
  had_to_create_mount_areas = 0;

  /* Fetch the mount table and cygdrive-related information from
     the registry.  */
  from_registry ();
}

/* conv_to_win32_path: Ensure src_path is a pure Win32 path and store
   the result in win32_path.

   If win32_path != NULL, the relative path, if possible to keep, is
   stored in win32_path.  If the relative path isn't possible to keep,
   the full path is stored.

   If full_win32_path != NULL, the full path is stored there.

   The result is zero for success, or an errno value.

   {,full_}win32_path must have sufficient space (i.e. MAX_PATH bytes).  */

int
mount_info::conv_to_win32_path (const char *src_path, char *dst,
				DWORD &devn, int &unit, unsigned *flags,
				bool no_normalize)
{
  TRACE_IN;
  static char last_src_path[MAX_PATH];
  static char last_dst[MAX_PATH];
  static int last_rc;
  static DWORD last_devn;
  static int last_unit;
  static unsigned last_flags;
  if (!strcmp (src_path, last_src_path))
    {
      strcpy (dst, last_dst);
      devn = last_devn;
      unit = last_unit;
      *flags = last_flags;
      return 0;
    }
  strcpy (last_src_path, src_path);
  while (sys_mount_table_counter < cygwin_shared->sys_mount_table_counter)
    {
      init ();
      sys_mount_table_counter++;
    }
  int src_path_len = strlen (src_path);
  MALLOC_CHECK;
  unsigned dummy_flags;
  int chroot_ok = !cygheap->root.exists ();

  devn = FH_BAD;
  unit = 0;

  if (!flags)
    flags = &dummy_flags;

  *flags = 0;
  debug_printf ("conv_to_win32_path (%s)", src_path);


  if (src_path_len >= MAX_PATH)
    {
      debug_printf ("ENAMETOOLONG = conv_to_win32_path (%s)", src_path);
      last_src_path[0] = '\0';
      last_rc = ENAMETOOLONG;
      last_devn = devn;
      last_unit = unit;
      last_flags = *flags;
      return ENAMETOOLONG;
    }

  int i, rc;
  mount_item *mi = NULL;	/* initialized to avoid compiler warning */
  char pathbuf[MAX_PATH];

  if (dst == NULL)
    goto out;		/* Sanity check. */

  /* An MS-DOS spec has either a : or a \.  If this is found, short
     circuit most of the rest of this function. */
  if (strpbrk (src_path, ":\\") != NULL || slash_unc_prefix_p (src_path))
    {
      debug_printf ("%s already win32", src_path);
      rc = normalize_win32_path (src_path, dst);
      if (rc)
	{
	  debug_printf ("normalize_win32_path failed, rc %d", rc);
	  last_dst[0] = '\0';
	  last_rc = rc;
	  last_devn = devn;
	  last_unit = unit;
	  last_flags = *flags;
	  return rc;
	}

      *flags = set_flags_from_win32_path (dst);
      goto out;
    }

  /* Normalize the path, taking out ../../ stuff, we need to do this
     so that we can move from one mounted directory to another with relative
     stuff.

     eg mounting c:/foo /foo
     d:/bar /bar

     cd /bar
     ls ../foo

     should look in c:/foo, not d:/foo.

     We do this by first getting an absolute UNIX-style path and then
     converting it to a DOS-style path, looking up the appropriate drive
     in the mount table.  */

  if (no_normalize)
    strcpy (pathbuf, src_path);
  else
    {
      rc = normalize_posix_path (src_path, pathbuf);

      if (rc)
	{
	  debug_printf ("%d = conv_to_win32_path (%s)", rc, src_path);
	  *flags = 0;
	  last_dst[0] = '\0';
	  last_rc = rc;
	  last_devn = devn;
	  last_unit = unit;
	  last_flags = *flags;
	  return rc;
	}
    }

  /* See if this is a cygwin "device" */
  if (win32_device_name (pathbuf, dst, devn, unit))
    {
      *flags = MOUNT_BINARY;	/* FIXME: Is this a sensible default for devices? */
      rc = 0;
      goto out_no_chroot_check;
    }

  /* Check if the cygdrive prefix was specified.  If so, just strip
     off the prefix and transform it into an MS-DOS path. */
  MALLOC_CHECK;
#if ! NEW_PATH_METHOD
  if (iscygdrive_device (pathbuf))
    {
      if (!cygdrive_win32_path (pathbuf, dst, 0))
	return ENOENT;
      *flags = cygdrive_flags;
      goto out;
    }
#endif

  int chrooted_path_len;
  chrooted_path_len = 0;
  /* Check the mount table for prefix matches. */
  for (i = 0; i < nmounts; i++)
    {
      const char *path;
      int len;

      mi = mount + posix_sorted[i];
      if (!cygheap->root.exists ()
	  || (mi->posix_pathlen == 1 && mi->posix_path[0] == '/'))
	{
	  path = mi->posix_path;
	  len = mi->posix_pathlen;
	}
      else if (cygheap->root.posix_ok (mi->posix_path))
	{
	  path = cygheap->root.unchroot (mi->posix_path);
	  chrooted_path_len = len = strlen (path);
	}
      else
	{
	  chrooted_path_len = 0;
	  continue;
	}

      if (path_prefix_p (path, pathbuf, len))
	break;
    }

  if (i >= nmounts)
    {
      backslashify (pathbuf, dst, 0);	/* just convert */
      *flags = 0;
    }
  else
    {
      int n;
      const char *native_path;
      int posix_pathlen;
      if (chroot_ok || chrooted_path_len || mi->posix_pathlen != 1
	  || mi->posix_path[0] != '/')
	{
	  n = mi->native_pathlen;
	  native_path = mi->native_path;
	  posix_pathlen = chrooted_path_len ?: mi->posix_pathlen;
	  chroot_ok = 1;
	}
      else
	{
	  n = cygheap->root.native_length ();
	  native_path = cygheap->root.native_path ();
	  posix_pathlen = mi->posix_pathlen;
	  chroot_ok = 1;
	}
      memcpy (dst, native_path, n + 1);
      const char *p = pathbuf + posix_pathlen;
      if (*p == '/')
	/* nothing */;
      else if ((isdrive (dst) && !dst[2]) || *p)
	dst[n++] = '\\';
      strcpy (dst + n, p);
      backslashify (dst, dst, 0);
      *flags = mi->flags;
    }

 out:
  MALLOC_CHECK;
  if (chroot_ok || cygheap->root.ischroot_native (dst))
    rc = 0;
  else
    {
      debug_printf ("attempt to access outside of chroot '%s = %s'",
		    cygheap->root.posix_path (), cygheap->root.native_path ());
      rc = ENOENT;
    }

 out_no_chroot_check:
  debug_printf ("src_path %s, dst %s, flags %p, rc %d", src_path, dst, *flags, rc);
  strcpy(last_dst, dst);
  last_rc = rc;
  last_devn = devn;
  last_unit = unit;
  last_flags = *flags;
  return rc;
}

/* cygdrive_posix_path: Build POSIX path used as the
   mount point for cygdrives created when there is no other way to
   obtain a POSIX path from a Win32 one. */

void
mount_info::cygdrive_posix_path (const char *src, char *dst, int trailing_slash_p)
{
  TRACE_IN;
  int len = cygdrive_len;

  memcpy (dst, cygdrive, len + 1);

  /* Now finish the path off with the drive letter to be used.
     The cygdrive prefix always ends with a trailing slash so
     the drive letter is added after the path. */
  dst[len++] = cyg_tolower (src[0]);
  if (!src[2] || (SLASH_P (src[2]) && !src[3]))
    dst[len++] = '\000';
  else
    {
      int n;
      dst[len++] = '/';
      if (SLASH_P (src[2]))
	n = 3;
      else
	n = 2;
      strcpy (dst + len, src + n);
    }
  slashify (dst, dst, trailing_slash_p);
}

int
mount_info::cygdrive_win32_path (const char *src, char *dst, int trailing_slash_p)
{
  TRACE_IN;
  const char *p = src + cygdrive_len;
  if (!isalpha (*p) || (!isdirsep (p[1]) && p[1]))
    return 0;
  dst[0] = *p;
  dst[1] = ':';
  strcpy (dst + 2, p + 1);
  backslashify (dst, dst, trailing_slash_p || !dst[2]);
  debug_printf ("src '%s', dst '%s'", src, dst);
  return 1;
}

/* conv_to_posix_path: Ensure src_path is a POSIX path.

   The result is zero for success, or an errno value.
   posix_path must have sufficient space (i.e. MAX_PATH bytes).
   If keep_rel_p is non-zero, relative paths stay that way.  */

int
mount_info::conv_to_posix_path (const char *src_path, char *posix_path,
				int keep_rel_p)
{
  TRACE_IN;
  int src_path_len = strlen (src_path);
  int relative_path_p = !isabspath (src_path);
  int trailing_slash_p;

  if (src_path_len <= 1)
    trailing_slash_p = 0;
  else
    {
      const char *lastchar = src_path + src_path_len - 1;
      trailing_slash_p = SLASH_P (*lastchar) && lastchar[-1] != ':';
    }

  debug_printf ("conv_to_posix_path (%s, %s, %s)", src_path,
		keep_rel_p ? "keep-rel" : "no-keep-rel",
		trailing_slash_p ? "add-slash" : "no-add-slash");
  MALLOC_CHECK;

  if (src_path_len >= MAX_PATH)
    {
      debug_printf ("ENAMETOOLONG");
      return ENAMETOOLONG;
    }

  /* FIXME: For now, if the path is relative and it's supposed to stay
     that way, skip mount table processing. */

  if (keep_rel_p && relative_path_p)
    {
      slashify (src_path, posix_path, 0);
      debug_printf ("%s = conv_to_posix_path (%s)", posix_path, src_path);
      return 0;
    }

  char pathbuf[MAX_PATH];
  int rc = normalize_win32_path (src_path, pathbuf);
  if (rc != 0)
    {
      debug_printf ("%d = conv_to_posix_path (%s)", rc, src_path);
      return rc;
    }

  int pathbuflen = strlen (pathbuf);
  for (int i = 0; i < nmounts; ++i)
    {
      mount_item &mi = mount[native_sorted[i]];
      if (!path_prefix_p (mi.native_path, pathbuf, mi.native_pathlen))
	continue;

      if (cygheap->root.exists () && !cygheap->root.posix_ok (mi.posix_path))
	continue;

      /* SRC_PATH is in the mount table. */
      int nextchar;
      const char *p = pathbuf + mi.native_pathlen;

      if (!*p || !p[1])
	nextchar = 0;
      else if (isdirsep (*p))
	nextchar = -1;
      else
	nextchar = 1;

      int addslash = nextchar > 0 ? 1 : 0;
      if ((mi.posix_pathlen + (pathbuflen - mi.native_pathlen) + addslash) >= MAX_PATH)
	return ENAMETOOLONG;
      strcpy (posix_path, mi.posix_path);
      if (addslash)
	strcat (posix_path, "/");
      if (nextchar)
	slashify (p,
		  posix_path + addslash + (mi.posix_pathlen == 1 ? 0 : mi.posix_pathlen),
		  trailing_slash_p);

      if (cygheap->root.exists ())
	{
	  const char *p = cygheap->root.unchroot (posix_path);
	  memmove (posix_path, p, strlen (p) + 1);
	}
      goto out;
    }

  if (!cygheap->root.exists ())
    /* nothing */;
  else if (cygheap->root.ischroot_native (pathbuf))
    {
      const char *p = pathbuf + cygheap->root.native_length ();
      if (*p)
	slashify (p, posix_path, trailing_slash_p);
      else
	{
	  posix_path[0] = '/';
	  posix_path[1] = '\0';
	}
    }
  else
    return ENOENT;

  /* Not in the database.  This should [theoretically] only happen if either
     the path begins with //, or / isn't mounted, or the path has a drive
     letter not covered by the mount table.  If it's a relative path then the
     caller must want an absolute path (otherwise we would have returned
     above).  So we always return an absolute path at this point. */
  if (isdrive (pathbuf))
    cygdrive_posix_path (pathbuf, posix_path, trailing_slash_p);
  else
    {
      /* The use of src_path and not pathbuf here is intentional.
	 We couldn't translate the path, so just ensure no \'s are present. */
      slashify (src_path, posix_path, trailing_slash_p);
    }

out:
  debug_printf ("%s = conv_to_posix_path (%s)", posix_path, src_path);
  MALLOC_CHECK;
  return 0;
}

/* Return flags associated with a mount point given the win32 path. */

unsigned
mount_info::set_flags_from_win32_path (const char *p)
{
  TRACE_IN;
  for (int i = 0; i < nmounts; i++)
    {
      mount_item &mi = mount[native_sorted[i]];
      if (path_prefix_p (mi.native_path, p, mi.native_pathlen))
	return mi.flags;
    }
  return 0;
}

DWORD WINAPI
mount_info::read_mounts_thread (LPVOID thrdParam)
{
    mount_info *info = (mount_info *)thrdParam;
    char etcPath [MAX_PATH+1];
    strcpy (etcPath, info->RootPath);
    strcat (etcPath, "/etc");
    HANDLE ffcnH = FindFirstChangeNotification (etcPath, true,
	      FILE_NOTIFY_CHANGE_FILE_NAME
	    | FILE_NOTIFY_CHANGE_DIR_NAME
	    | FILE_NOTIFY_CHANGE_SIZE
	    | FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (ffcnH == INVALID_HANDLE_VALUE)
      {
	debug_printf("/etc path change notification failure, %s\n", etcPath);
	ExitProcess (1);
      }
    do {
	FindNextChangeNotification (ffcnH);
	if (WaitForSingleObject (ffcnH, INFINITE) == WAIT_OBJECT_0)
	  {
	    FIXME; // We need only do this if /etc/fstab actually changed and
	    // not any change to the /etc directory.
	    auto_lock mounts_lock(info->lock);
	    info->nmounts = 0;
	    info->read_mounts2 ();
	  }
    } while (true);
    return 0;
}

void
mount_info::read_mounts (reg_key& r)
{
    InitializeCriticalSection(&lock);
    read_mounts2();
    threadH = CreateThread (NULL, 0, mount_info::read_mounts_thread, this, 0, threadID);
}

/* read_mounts: Given a specific regkey, read mounts from under its
   key. */

void
mount_info::read_mounts2 (void)
{
  TRACE_IN;
  char native_path[4];
  char posix_path[MAX_PATH];
  DWORD mask = 1, drive = 'a';
  available_drives = GetLogicalDrives ();
  int mount_flags = 0;
  int res;
  char DllPath[MAX_PATH+1];

  mount_flags |= MOUNT_AUTO;
  mount_flags |= MOUNT_BINARY;

  AbsDllPath ("msys-1.0.dll", DllPath, sizeof (DllPath));
  {
    char *ptr;
    strcpy(RootPath, DllPath);
    ptr = strrchr (RootPath, '\\');
    *ptr = '\0';
  }
  res = mount_table->add_item (RootPath, "/", mount_flags, FALSE);

  //Allow /etc/fstab to override the default mounts as well as add to the
  //list of mounted items.  This will allow me the pleasure of doing 
  //business with my already layed out scheme and thusly sharing commonly 
  //one /prj directory with cygwin.
  //
  //Process fstab first so that automounted takes precedence to avoid user
  //complaint problems.
  //
  FILE *fp;
  char fstab[MAX_PATH*3];
  if ((fp = fopen("/etc/fstab", "r")) == NULL)
    {
      debug_printf("open /etc/fstab failed");
    }
  else
    {
      debug_printf("open /etc/fstab succeeded");
      while (fgets (fstab, sizeof (fstab), fp) != NULL)
	{
	  char *pfstab = fstab;
	  char *st;
	  char *sst;
	  debug_printf("fstab = %s", fstab);
	  if (*pfstab == '#')
	      continue;
	  st = strchr(pfstab, '\n');
	  if (st)
	    {
	      *st = '\0';
	      // Strip carriage return.
	      if (st > pfstab && *--st == '\r')
		  *st = '\0';
	    }
	  if (! strlen (pfstab))
	      continue;
	  // Replace tabs with spaces.
	  while ((st = strchr (pfstab, '\t')))
	    {
	      *st = ' ';
	    }
	  // Strip spaces at end of line.
	  sst = strchr(pfstab, '\0');
	  while (sst > pfstab && *--sst == ' ')
	      *sst = '\0';
	  // Find space separator.
	  st = strchr (pfstab, ' ');
	  if (!st)
	      continue;
	  *st++ = '\0';
	  while (*st == ' ')
	      st++;
	  debug_printf("fstab: native - %s, posix - %s", pfstab, st);
	  res = mount_table->add_item (pfstab, st, MOUNT_BINARY, FALSE);
	}
      fclose (fp);
    }
  // Automount all of the important mountpoints.
  while (available_drives)
    {
      for (/* nothing */; drive <= 'z'; mask <<= 1, drive++)
	  if (available_drives & mask)
	      break;

      __small_sprintf (native_path, "%c:\\", drive);
      __small_sprintf (posix_path, "/%c", drive);

      res = mount_table->add_item (native_path, posix_path, mount_flags, FALSE);

      available_drives &= ~mask;
      if (res && get_errno () == EMFILE)
	  break;

    }

  // Add "/" again to prevent user corruption.
  res = mount_table->add_item (RootPath, "/", mount_flags, FALSE);
  res = mount_table->add_item (RootPath, "/usr", mount_flags, FALSE);
  //FIXME-1.0:
  //	    In order to pass Win32 paths to Win32 programs and POSIX paths to
  //	    MSYS programs we must know if we have MSYS programs.  The flag
  //	    MOUNT_CYGWIN_EXEC is being used for this purpose.  Once iscygexec
  //	    is fixed to know that we have an MSYS program based on the msys dll
  //	    being present then we can remove this.
  //	    NOTE: I added the /usr/bin mount point simply to mark it as
  //		  containing MSYS programs.  It can be removed once fixed.
  res = mount_table->add_item (DllPath, "/bin", MOUNT_CYGWIN_EXEC | mount_flags, FALSE);
  res = mount_table->add_item (DllPath, "/usr/bin", MOUNT_CYGWIN_EXEC | mount_flags, FALSE);

  {
    char buf[MAX_PATH];
    GetEnvironmentVariable ("TMP", buf, sizeof (buf));
    res = mount_table->add_item (buf, "/tmp", mount_flags, FALSE);
  }  
}

/* from_registry: Build the entire mount table from the registry.  Also,
   read in cygdrive-related information from its registry location. */

void
mount_info::from_registry ()
{
  TRACE_IN;
    reg_key r;
    read_mounts (r);
    return;
}

/* add_reg_mount: Add mount item to registry.  Return zero on success,
   non-zero on failure. */
/* FIXME: Need a mutex to avoid collisions with other tasks. */

int
mount_info::add_reg_mount (const char * native_path, const char * posix_path, unsigned mountflags)
{
  TRACE_IN;
  int res = 0;

  /* Add the mount to the right registry location, depending on
     whether MOUNT_SYSTEM is set in the mount flags. */
  if (!(mountflags & MOUNT_SYSTEM)) /* current_user mount */
    {
      /* reg_key for user mounts in HKEY_CURRENT_USER. */
      reg_key reg_user;

      /* Start by deleting existing mount if one exists. */
      res = reg_user.kill (posix_path);
      if (res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND)
	goto err;

      /* Create the new mount. */
      reg_key subkey = reg_key (reg_user.get_key (),
				KEY_ALL_ACCESS,
				posix_path, NULL);
      res = subkey.set_string ("native", native_path);
      if (res != ERROR_SUCCESS)
	goto err;
      res = subkey.set_int ("flags", mountflags);
    }

  return 0; /* Success */
 err:
  __seterrno_from_win_error (res);
  return -1;
}

/* del_reg_mount: delete mount item from registry indicated in flags.
   Return zero on success, non-zero on failure.*/
/* FIXME: Need a mutex to avoid collisions with other tasks. */

int
mount_info::del_reg_mount (const char * posix_path, unsigned flags)
{
  TRACE_IN;
  set_errno(ENOSYS);
  return -1;
}

/* read_cygdrive_info_from_registry: Read the default prefix and flags
   to use when creating cygdrives from the special user registry
   location used to store cygdrive information. */

void
mount_info::read_cygdrive_info_from_registry ()
{
  TRACE_IN;
  set_errno(ENOSYS);
}

/* write_cygdrive_info_to_registry: Write the default prefix and flags
   to use when creating cygdrives to the special user registry
   location used to store cygdrive information. */

int
mount_info::write_cygdrive_info_to_registry (const char *cygdrive_prefix, unsigned flags)
{
  TRACE_IN;
  set_errno(ENOSYS);
  return -1;
}

int
mount_info::remove_cygdrive_info_from_registry (const char *cygdrive_prefix, unsigned flags)
{
  TRACE_IN;
  set_errno(ENOSYS);
  return -1;
}

int
mount_info::get_cygdrive_info (char *user, char *system, char* user_flags,
			       char* system_flags)
{
  TRACE_IN;
  set_errno(ENOSYS);
  return -1;
}

static mount_item *mounts_for_sort;

/* sort_by_posix_name: qsort callback to sort the mount entries.  Sort
   user mounts ahead of system mounts to the same POSIX path. */
/* FIXME: should the user should be able to choose whether to
   prefer user or system mounts??? */
static int
sort_by_posix_name (const void *a, const void *b)
{
  TRACE_IN;
  mount_item *ap = mounts_for_sort + (*((int*) a));
  mount_item *bp = mounts_for_sort + (*((int*) b));

  /* Base weighting on longest posix path first so that the most
     obvious path will be chosen. */
  size_t alen = strlen (ap->posix_path);
  size_t blen = strlen (bp->posix_path);

  int res = blen - alen;

  if (res)
    return res;		/* Path lengths differed */

  /* The two paths were the same length, so just determine normal
     lexical sorted order. */
  res = strcmp (ap->posix_path, bp->posix_path);

  if (res == 0)
   {
     /* need to select between user and system mount to same POSIX path */
     if (!(bp->flags & MOUNT_SYSTEM))	/* user mount */
      return 1;
     else
      return -1;
   }

  return res;
}

/* sort_by_native_name: qsort callback to sort the mount entries.  Sort
   user mounts ahead of system mounts to the same POSIX path. */
/* FIXME: should the user should be able to choose whether to
   prefer user or system mounts??? */
static int
sort_by_native_name (const void *a, const void *b)
{
  TRACE_IN;
  mount_item *ap = mounts_for_sort + (*((int*) a));
  mount_item *bp = mounts_for_sort + (*((int*) b));

  /* Base weighting on longest win32 path first so that the most
     obvious path will be chosen. */
  size_t alen = strlen (ap->native_path);
  size_t blen = strlen (bp->native_path);

  int res = blen - alen;

  if (res)
    return res;		/* Path lengths differed */

  /* The two paths were the same length, so just determine normal
     lexical sorted order. */
  res = strcmp (ap->native_path, bp->native_path);

  if (res == 0)
   {
     /* need to select between user and system mount to same POSIX path */
     if (!(bp->flags & MOUNT_SYSTEM))	/* user mount */
      return 1;
     else
      return -1;
   }

  return res;
}

void
mount_info::sort ()
{
  TRACE_IN;
  for (int i = 0; i < nmounts; i++)
    native_sorted[i] = posix_sorted[i] = i;
  /* Sort them into reverse length order, otherwise we won't
     be able to look for /foo in /.  */
  mounts_for_sort = mount;	/* ouch. */
  qsort (posix_sorted, nmounts, sizeof (posix_sorted[0]), sort_by_posix_name);
  qsort (native_sorted, nmounts, sizeof (native_sorted[0]), sort_by_native_name);
}

/* Add an entry to the mount table.
   Returns 0 on success, -1 on failure and errno is set.

   This is where all argument validation is done.  It may not make sense to
   do this when called internally, but it's cleaner to keep it all here.  */

int
mount_info::add_item (const char *native, const char *posix, unsigned mountflags, int reg_p)
{
  TRACE_IN;
  auto_lock mounts_lock(lock); 
  /* Something's wrong if either path is NULL or empty, or if it's
     not a UNC or absolute path. */

  if ((native == NULL) || (*native == 0) ||
      (posix == NULL) || (*posix == 0) ||
      !isabspath (native) || !isabspath (posix) ||
      slash_unc_prefix_p (posix) || isdrive (posix))
    {
      set_errno (EINVAL);
      return -1;
    }

  /* Make sure both paths do not end in /. */
  char nativetmp[MAX_PATH];
  char posixtmp[MAX_PATH];

  backslashify (native, nativetmp, 0);
  nofinalslash (nativetmp, nativetmp);

  slashify (posix, posixtmp, 0);
  nofinalslash (posixtmp, posixtmp);

  debug_printf ("%s[%s], %s[%s], %p",
		native, nativetmp, posix, posixtmp, mountflags);

  /* Duplicate /'s in path are an error. */
  for (char *p = posixtmp + 1; *p; ++p)
    {
      if (p[-1] == '/' && p[0] == '/')
	{
	  set_errno (EINVAL);
	  return -1;
	}
    }

  /* Write over an existing mount item with the same POSIX path if
     it exists and is from the same registry area. */
  int i;
  for (i = 0; i < nmounts; i++)
    {
      if (strcasematch (mount[i].posix_path, posixtmp) &&
	  (mount[i].flags & MOUNT_SYSTEM) == (mountflags & MOUNT_SYSTEM))
	break;
    }

  if (i == nmounts && nmounts == MAX_MOUNTS)
    {
      set_errno (EMFILE);
      return -1;
    }

  if (i == nmounts)
    nmounts++;
  mount[i].init (nativetmp, posixtmp, mountflags);
  sort ();

  return 0;
}

/* Delete a mount table entry where path is either a Win32 or POSIX
   path. Since the mount table is really just a table of aliases,
   deleting / is ok (although running without a slash mount is
   strongly discouraged because some programs may run erratically
   without one).  If MOUNT_SYSTEM is set in flags, remove from system
   registry, otherwise remove the user registry mount.
*/

int
mount_info::del_item (const char *path, unsigned flags, int reg_p)
{
  TRACE_IN;
  auto_lock mounts_lock(lock); 
  char pathtmp[MAX_PATH];
  int posix_path_p = FALSE;

  /* Something's wrong if path is NULL or empty. */
  if (path == NULL || *path == 0 || !isabspath (path))
    {
      set_errno (EINVAL);
      return -1;
    }

  if (slash_unc_prefix_p (path) || strpbrk (path, ":\\"))
    backslashify (path, pathtmp, 0);
  else
    {
      slashify (path, pathtmp, 0);
      posix_path_p = TRUE;
    }
  nofinalslash (pathtmp, pathtmp);

  if (reg_p && posix_path_p &&
      del_reg_mount (pathtmp, flags) &&
      del_reg_mount (path, flags)) /* for old irregular entries */
    return -1;

  for (int i = 0; i < nmounts; i++)
    {
      int ent = native_sorted[i]; /* in the same order as getmntent() */
      if (((posix_path_p)
	   ? strcasematch (mount[ent].posix_path, pathtmp)
	   : strcasematch (mount[ent].native_path, pathtmp)) &&
	  (mount[ent].flags & MOUNT_SYSTEM) == (flags & MOUNT_SYSTEM))
	{
	  if (!posix_path_p &&
	      reg_p && del_reg_mount (mount[ent].posix_path, flags))
	    return -1;

	  nmounts--; /* One less mount table entry */
	  /* Fill in the hole if not at the end of the table */
	  if (ent < nmounts)
	    memmove (mount + ent, mount + ent + 1,
		     sizeof (mount[ent]) * (nmounts - ent));
	  sort (); /* Resort the table */
	  return 0;
	}
    }
  set_errno (EINVAL);
  return -1;
}

/************************* mount_item class ****************************/

static mntent *
fillout_mntent (const char *native_path, const char *posix_path, unsigned flags)
{
  TRACE_IN;
#ifdef _MT_SAFE
  struct mntent &ret=_reent_winsup()->mntbuf;
#else
  static NO_COPY struct mntent ret;
#endif

  /* Remove drivenum from list if we see a x: style path */
  if (strlen (native_path) == 2 && native_path[1] == ':')
    {
      int drivenum = tolower (native_path[0]) - 'a';
      if (drivenum >= 0 && drivenum <= 31)
	available_drives &= ~(1 << drivenum);
    }

  /* Pass back pointers to mount_table strings reserved for use by
     getmntent rather than pointers to strings in the internal mount
     table because the mount table might change, causing weird effects
     from the getmntent user's point of view. */

  strcpy (_reent_winsup ()->mnt_fsname, native_path);
  ret.mnt_fsname = _reent_winsup ()->mnt_fsname;
  strcpy (_reent_winsup ()->mnt_dir, posix_path);
  ret.mnt_dir = _reent_winsup ()->mnt_dir;

  if (!(flags & MOUNT_SYSTEM))		/* user mount */
    strcpy (_reent_winsup ()->mnt_type, (char *) "user");
  else					/* system mount */
    strcpy (_reent_winsup ()->mnt_type, (char *) "system");

  ret.mnt_type = _reent_winsup ()->mnt_type;

  /* mnt_opts is a string that details mount params such as
     binary or textmode, or exec.  We don't print
     `silent' here; it's a magic internal thing. */

  if (!(flags & MOUNT_BINARY))
    strcpy (_reent_winsup ()->mnt_opts, (char *) "textmode");
  else
    strcpy (_reent_winsup ()->mnt_opts, (char *) "binmode");

  if (flags & MOUNT_CYGWIN_EXEC)
    strcat (_reent_winsup ()->mnt_opts, (char *) ",cygexec");
  else if (flags & MOUNT_EXEC)
    strcat (_reent_winsup ()->mnt_opts, (char *) ",exec");

  if ((flags & MOUNT_AUTO))		/* cygdrive */
    strcat (_reent_winsup ()->mnt_opts, (char *) ",noumount");

  ret.mnt_opts = _reent_winsup ()->mnt_opts;

  ret.mnt_freq = 1;
  ret.mnt_passno = 1;
  return &ret;
}

struct mntent *
mount_item::getmntent ()
{
  TRACE_IN;
  return fillout_mntent (native_path, posix_path, flags);
}

static struct mntent *
cygdrive_getmntent ()
{
  TRACE_IN;
  char native_path[4];
  char posix_path[MAX_PATH];
  DWORD mask = 1, drive = 'a';
  struct mntent *ret = NULL;

  while (available_drives)
    {
      for (/* nothing */; drive <= 'z'; mask <<= 1, drive++)
	if (available_drives & mask)
	  break;

      __small_sprintf (native_path, "%c:\\", drive);
      if (GetDriveType (native_path) == DRIVE_REMOVABLE ||
	  GetFileAttributes (native_path) == (DWORD) -1)
	{
	  available_drives &= ~mask;
	  continue;
	}
      native_path[2] = '\0';
      __small_sprintf (posix_path, "%s%c", mount_table->cygdrive, drive);
      ret = fillout_mntent (native_path, posix_path, mount_table->cygdrive_flags);
      break;
    }

  return ret;
}

struct mntent *
mount_info::getmntent (int x)
{
  TRACE_IN;
  if (x < 0 || x >= nmounts)
    return cygdrive_getmntent ();

  return mount[native_sorted[x]].getmntent ();
}

/* Fill in the fields of a mount table entry.  */

void
mount_item::init (const char *native, const char *posix, unsigned mountflags)
{
  TRACE_IN;
  strcpy ((char *) native_path, native);
  strcpy ((char *) posix_path, posix);

  native_pathlen = strlen (native_path);
  posix_pathlen = strlen (posix_path);

  flags = mountflags;
}

/********************** Mount System Calls **************************/

/* Mount table system calls.
   Note that these are exported to the application.  */

/* mount: Add a mount to the mount table in memory and to the registry
   that will cause paths under win32_path to be translated to paths
   under posix_path. */

extern "C"
int
mount (const char *win32_path, const char *posix_path, unsigned flags)
{
  TRACE_IN;
  set_errno(ENOSYS);
  return -1;
}

/* umount: The standard umount call only has a path parameter.  Since
   it is not possible for this call to specify whether to remove the
   mount from the user or global mount registry table, assume the user
   table. */

extern "C"
int
umount (const char *path)
{
  TRACE_IN;
  set_errno(ENOSYS);
  return -1;
}

/* cygwin_umount: This is like umount but takes an additional flags
   parameter that specifies whether to umount from the user or system-wide
   registry area. */

extern "C"
int
cygwin_umount (const char *path, unsigned flags)
{
  TRACE_IN;
  int res = -1;

  if (flags & MOUNT_AUTO)
    {
      /* When flags include MOUNT_AUTO, take this to mean that we actually want
	 to remove the cygdrive prefix and flags without actually unmounting
	 anything. */
      res = mount_table->remove_cygdrive_info_from_registry (path, flags);
    }
  else
    {
      res = mount_table->del_item (path, flags, TRUE);
    }

  syscall_printf ("%d = cygwin_umount (%s, %d)", res,  path, flags);
  return res;
}

extern "C"
FILE *
setmntent (const char *filep, const char *)
{
  TRACE_IN;
  iteration = 0;
  available_drives = GetLogicalDrives ();
  return (FILE *) filep;
}

extern "C"
struct mntent *
getmntent (FILE *)
{
  TRACE_IN;
  return mount_table->getmntent (iteration++);
}

extern "C"
int
endmntent (FILE *)
{
  TRACE_IN;
  return 1;
}

/********************** Symbolic Link Support **************************/

/* Read symlink from Extended Attribute */
int
get_symlink_ea (const char* frompath, char* buf, int buf_size)
{
  TRACE_IN;
  int res = NTReadEA (frompath, SYMLINK_EA_NAME, buf, buf_size);
  if (res == 0)
    debug_printf ("Cannot read symlink from EA");
  return (res - 1);
}

/* Save symlink to Extended Attribute */
BOOL
set_symlink_ea (const char* frompath, const char* topath)
{
  TRACE_IN;
  if (!NTWriteEA (frompath, SYMLINK_EA_NAME, topath, strlen (topath) + 1))
    {
      debug_printf ("Cannot save symlink in EA");
      return FALSE;
    }
  return TRUE;
}

/* Create a symlink from FROMPATH to TOPATH. */

/* If TRUE create symlinks as Windows shortcuts, if FALSE create symlinks
   as normal files with magic number and system bit set. */
int allow_winsymlinks = TRUE;

extern "C"
int
symlink (const char *topath, const char *frompath)
{
  TRACE_IN;
#if NO_SYMLINK
    int res;
    debug_printf("symlink (%s, %s)", topath, frompath);
    res = msys_symlink (frompath, topath);
    return res;
#else
  HANDLE h;
  int res = -1;
  path_conv win32_path, win32_topath;
  char from[MAX_PATH + 5];
  char cwd[MAX_PATH + 1], *cp = NULL, c = 0;
  char w32topath[MAX_PATH + 1];
  DWORD written;
  SECURITY_ATTRIBUTES sa = sec_none_nih;

  win32_path.check (frompath, PC_SYM_NOFOLLOW);
  if (allow_winsymlinks && !win32_path.error)
    {
      strcpy (from, frompath);
      strcat (from, ".lnk");
      win32_path.check (from, PC_SYM_NOFOLLOW);
    }

  if (win32_path.error)
    {
      set_errno (win32_path.case_clash ? ECASECLASH : win32_path.error);
      goto done;
    }

  syscall_printf ("symlink (%s, %s)", topath, win32_path.get_win32 ());

  if (topath[0] == 0)
    {
      set_errno (EINVAL);
      goto done;
    }
  if (strlen (topath) >= MAX_PATH)
    {
      set_errno (ENAMETOOLONG);
      goto done;
    }

  if (win32_path.is_device () ||
      win32_path.file_attributes () != (DWORD) -1)
    {
      set_errno (EEXIST);
      goto done;
    }

  if (allow_winsymlinks)
    {
      if (!isabspath (topath))
	{
	  getcwd (cwd, MAX_PATH + 1);
	  if ((cp = strrchr (from, '/')) || (cp = strrchr (from, '\\')))
	    {
	      c = *cp;
	      *cp = '\0';
	      debug_printf("chdir(from=%s)", from);
	      chdir (from);
	    }
	  backslashify (topath, w32topath, 0);
	}
      if (!cp || GetFileAttributes (w32topath) == (DWORD)-1)
	{
	  win32_topath.check (topath, PC_SYM_NOFOLLOW);
	  if (!cp || win32_topath.error != ENOENT)
	    strcpy (w32topath, win32_topath);
	}
      if (cp)
	{
	  *cp = c;
	  debug_printf("chrdir(cwd=%s)", cwd);
	  chdir (cwd);
	}
    }

  if (allow_ntsec && win32_path.has_acls ())
    {
      set_security_attribute (S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO,
			    &sa, 
			    alloca (4096),
			    4096);
    }

  h = CreateFileA(win32_path, GENERIC_WRITE, 0, &sa,
		  CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
  if (h == INVALID_HANDLE_VALUE)
      __seterrno ();
  else
    {
      BOOL success;

      if (allow_winsymlinks)
	{
	  create_shortcut_header ();
	  /* Don't change the datatypes of `len' and `win_len' since
	     their sizeof is used when writing. */
	  unsigned short len = strlen (topath);
	  unsigned short win_len = strlen (w32topath);
	  success = WriteFile (h, shortcut_header, SHORTCUT_HDR_SIZE,
			       &written, NULL)
		    && written == SHORTCUT_HDR_SIZE
		    && WriteFile (h, &len, sizeof len, &written, NULL)
		    && written == sizeof len
		    && WriteFile (h, topath, len, &written, NULL)
		    && written == len
		    && WriteFile (h, &win_len, sizeof win_len, &written, NULL)
		    && written == sizeof win_len
		    && WriteFile (h, w32topath, win_len, &written, NULL)
		    && written == win_len;
	}
      else
	{
	  /* This is the old technique creating a symlink. */
	  char buf[sizeof (SYMLINK_COOKIE) + MAX_PATH + 10];

	  __small_sprintf (buf, "%s%s", SYMLINK_COOKIE, topath);
	  DWORD len = strlen (buf) + 1;

	  /* Note that the terminating nul is written.  */
	  success = WriteFile (h, buf, len, &written, NULL)
		    || written != len;

	}
      if (success)
	{
	  CloseHandle (h);
	  if (!allow_ntsec && allow_ntea)
	    set_file_attribute (win32_path.has_acls (),
				win32_path.get_win32 (),
				S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO);
	  SetFileAttributesA (win32_path.get_win32 (),
			      allow_winsymlinks ? FILE_ATTRIBUTE_READONLY
						: FILE_ATTRIBUTE_SYSTEM);
	  if (win32_path.fs_fast_ea ())
	    set_symlink_ea (win32_path, topath);
	  res = 0;
	}
      else
	{
	  __seterrno ();
	  CloseHandle (h);
	  DeleteFileA (win32_path.get_win32 ());
	}
    }

done:
  if (h != INVALID_HANDLE_VALUE)
      CloseHandle (h);
  syscall_printf ("%d = symlink (%s, %s)", res, topath, frompath);
  return res;
#endif //NO_SYMLINK
}

char *
suffix_scan::has (const char *in_path, const suffix_info *in_suffixes)
{
  TRACE_IN;
  nextstate = SCAN_BEG;
  suffixes = suffixes_start = in_suffixes;

  char *ext_here = strrchr (in_path, '.');
  path = in_path;
  eopath = strchr (path, '\0');

  if (!ext_here)
    goto noext;

  if (suffixes)
    {
      /* Check if the extension matches a known extension */
      for (const suffix_info *ex = in_suffixes; ex->name != NULL; ex++)
	if (strcasematch (ext_here, ex->name))
	  {
	    nextstate = SCAN_JUSTCHECK;
	    suffixes = NULL;	/* Has an extension so don't scan for one. */
	    goto done;
	  }
    }

#if ! NEW_PATH_METHOD
  /* Didn't match.  Use last resort -- .lnk. */
  if (strcasematch (ext_here, ".lnk"))
    {
      nextstate = SCAN_HASLNK;
      suffixes = NULL;
    }
#endif /* ! NEW_PATH_METHOD */

 noext:
  ext_here = eopath;

 done:
  return ext_here;
}

int
suffix_scan::next ()
{
  TRACE_IN;
  if (suffixes)
    {
      while (suffixes && suffixes->name)
	if (!suffixes->addon)
	  suffixes++;
	else
	  {
	    strcpy (eopath, suffixes->name);
#if ! NEW_PATH_METHOD
	    if (nextstate == SCAN_EXTRALNK)
	      strcat (eopath, ".lnk");
#endif
	    suffixes++;
	    return 1;
	  }
      suffixes = NULL;
    }

  switch (nextstate)
    {
    case SCAN_BEG:
      suffixes = suffixes_start;
#if ! NEW_PATH_METHOD
      if (!suffixes)
	nextstate = SCAN_LNK;
      else
	{
	  if (!*suffixes->name)
	    suffixes++;
	  nextstate = SCAN_EXTRALNK;
	}
#else
      if (!suffixes)
	nextstate = SCAN_JUSTCHECK;
      else
	{
	  if (!*suffixes->name)
	    suffixes++;
	  nextstate = SCAN_DONE;
	}
#endif // ! NEW_PATH_METHOD
      return 1;
#if ! NEW_PATH_METHOD
    case SCAN_HASLNK:
      nextstate = SCAN_EXTRALNK;	/* Skip SCAN_BEG */
      return 1;
    case SCAN_LNK:
    case SCAN_EXTRALNK:
      strcpy (eopath, ".lnk");
      nextstate = SCAN_DONE;
      return 1;
#endif
    case SCAN_JUSTCHECK:
#if ! NEW_PATH_METHOD
      nextstate = SCAN_APPENDLNK;
#else
      nextstate = SCAN_DONE;
#endif
      return 1;
#if ! NEW_PATH_METHOD
    case SCAN_APPENDLNK:
      strcat (eopath, ".lnk");
      nextstate = SCAN_DONE;
      return 1;
#endif
    default:
      *eopath = '\0';
      return 0;
    }
}

/* Check if PATH is a symlink.  PATH must be a valid Win32 path name.

   If PATH is a symlink, put the value of the symlink--the file to
   which it points--into BUF.  The value stored in BUF is not
   necessarily null terminated.  BUFLEN is the length of BUF; only up
   to BUFLEN characters will be stored in BUF.  BUF may be NULL, in
   which case nothing will be stored.

   Set *SYML if PATH is a symlink.

   Set *EXEC if PATH appears to be executable.  This is an efficiency
   hack because we sometimes have to open the file anyhow.  *EXEC will
   not be set for every executable file.

   Return -1 on error, 0 if PATH is not a symlink, or the length
   stored into BUF if PATH is a symlink.  */

int
symlink_info::check (char *path, const suffix_info *suffixes, unsigned opt)
{
  TRACE_IN;
  HANDLE h = (HANDLE)NULL;
  int res = 0;
  suffix_scan suffix;
  contents[0] = '\0';

  debug_printf("path: %s", path);

  is_symlink = TRUE;
  ext_here = suffix.has (path, suffixes);
  extn = ext_here - path;

  pflags &= ~PATH_SYMLINK;

  case_clash = FALSE;

  while (suffix.next ())
    {
      error = 0;
      fileattr = GetFileAttributesA (suffix.path);
      if (fileattr == (DWORD) -1)
	{
	  /* The GetFileAttributesA call can fail for reasons that don't
	     matter, so we just return 0.  For example, getting the
	     attributes of \\HOST will typically fail.  */
	  debug_printf ("GetFileAttributesA (%s) failed", suffix.path);
	  error = geterrno_from_win_error (GetLastError (), EACCES);
	  continue;
	}


      ext_tacked_on = !!*ext_here;

      is_symlink = FALSE;
      syscall_printf ("not a symlink");
      res = 0;
      break;
    }

  if (h != INVALID_HANDLE_VALUE)
      CloseHandle(h);
  syscall_printf ("%d = symlink.check (%s, %p) (%p)",
		  res, suffix.path, contents, pflags);
  return res;
}

/* Check the correct case of the last path component (given in DOS style).
   Adjust the case in this->path if pcheck_case == PCHECK_ADJUST or return
   FALSE if pcheck_case == PCHECK_STRICT.
   Dont't call if pcheck_case == PCHECK_RELAXED.
*/

BOOL
symlink_info::case_check (char *path)
{
  TRACE_IN;
  WIN32_FIND_DATA data;
  HANDLE h;
  char *c;

  /* Set a pointer to the beginning of the last component. */
  if (!(c = strrchr (path, '\\')))
    c = path;
  else
    ++c;

  if ((h = FindFirstFile (path, &data))
      != INVALID_HANDLE_VALUE)
    {
      FindClose (h);

      /* If that part of the component exists, check the case. */
      if (strcmp (c, data.cFileName))
	{
	  case_clash = TRUE;

	  /* If check is set to STRICT, a wrong case results
	     in returning a ENOENT. */
	  if (pcheck_case == PCHECK_STRICT)
	    return FALSE;

	  /* PCHECK_ADJUST adjusts the case in the incoming
	     path which points to the path in *this. */
	  strcpy (c, data.cFileName);
	}
    }
  return TRUE;
}

/* readlink system call */

extern "C"
int
readlink (const char *path, char *buf, int buflen)
{
  TRACE_IN;
  extern suffix_info stat_suffixes[];

  if (buflen < 0)
    {
      set_errno (ENAMETOOLONG);
      return -1;
    }

  path_conv pathbuf (path, PC_SYM_CONTENTS, stat_suffixes);

  if (pathbuf.error)
    {
      set_errno (pathbuf.error);
      syscall_printf ("-1 = readlink (%s, %p, %d)", path, buf, buflen);
      return -1;
    }

  if (pathbuf.file_attributes () == (DWORD) -1)
    {
      set_errno (ENOENT);
      return -1;
    }

  if (!pathbuf.issymlink ())
    {
      if (pathbuf.fileattr != (DWORD) -1)
	set_errno (EINVAL);
      return -1;
    }

  int len = min (buflen, (int) strlen (pathbuf.get_win32 ()));
  memcpy (buf, pathbuf.get_win32 (), len);

  /* errno set by symlink.check if error */
  return len;
}

/* Some programs rely on st_dev/st_ino being unique for each file.
   Hash the path name and hope for the best.  The hash arg is not
   always initialized to zero since readdir needs to compute the
   dirent ino_t based on a combination of the hash of the directory
   done during the opendir call and the hash or the filename within
   the directory.  FIXME: Not bullet-proof. */
/* Cygwin internal */

unsigned long __stdcall
hash_path_name (unsigned long hash, const char *name)
{
  TRACE_IN;
  if (!*name)
    return hash;

  /* Perform some initial permutations on the pathname if this is
     not "seeded" */
  if (!hash)
    {
      /* Simplistic handling of drives.  If there is a drive specified,
	 make sure that the initial letter is upper case.  If there is
	 no \ after the ':' assume access through the root directory
	 of that drive.
	 FIXME:  Should really honor MS-Windows convention of using
	 the environment to track current directory on various drives. */
      if (name[1] == ':')
	{
	  char *nn, *newname = (char *) alloca (strlen (name) + 2);
	  nn = newname;
	  *nn = isupper (*name) ? cyg_tolower (*name) : *name;
	  *++nn = ':';
	  name += 2;
	  if (*name != '\\')
	    *++nn = '\\';
	  strcpy (++nn, name);
	  name = newname;
	  goto hashit;
	}

      /* Fill out the hashed path name with the current working directory if
	 this is not an absolute path and there is no pre-specified hash value.
	 Otherwise the inodes same will differ depending on whether a file is
	 referenced with an absolute value or relatively. */

      if (!hash && !isabspath (name))
	{
	  hash = cygheap->cwd.get_hash ();
	  if (name[0] == '.' && name[1] == '\0')
	    return hash;
	  hash += hash_path_name (hash, "\\");
	}
    }

hashit:
  /* Build up hash.  Ignore single trailing slash or \a\b\ != \a\b or
     \a\b\.  but allow a single \ if that's all there is. */
  do
    {
      int ch = cyg_tolower(*name);
      hash += ch + (ch << 17);
      hash ^= hash >> 2;
    }
  while (*++name != '\0' &&
	 !(*name == '\\' && (!name[1] || (name[1] == '.' && !name[2]))));
  return hash;
}

char *
getcwd (char *buf, size_t ulen)
{
  TRACE_IN;
  // This cygheap->cwd.get function is a part of struct cwdstuff.
  // The following are the parameters.
  //   Pointer to buffer to hold the return data.
  //   Flag to return posix vs win32. (0 == win32, 1 == posix)
  //   Flag to indicate chroot honor (0 == false, 1 == true)
  //   Length of the buffer for the return data.
  // The return value is the value set into buf.
  return cygheap->cwd.get (buf, 1, 1, ulen);
}

/* getwd: standards? */
extern "C"
char *
getwd (char *buf)
{
  TRACE_IN;
  return getcwd (buf, MAX_PATH);
}

/* chdir: POSIX 5.2.1.1 */
extern "C" int
chdir (const char *in_dir)
{
  TRACE_IN;
  if (check_null_empty_str_errno (in_dir))
    return -1;

  syscall_printf ("dir '%s'", in_dir);

  char *s;
  char dir[strlen (in_dir) + 1];
  strcpy (dir, in_dir);
  /* Incredibly. Windows allows you to specify a path with trailing
     whitespace to SetCurrentDirectory.  This doesn't work too well
     with other parts of the API, though, apparently.  So nuke trailing
     white space. */
  for (s = strchr (dir, '\0'); --s >= dir && isspace ((unsigned int) (*s & 0xff)); )
    *s = '\0';

  if (!*s)
    {
      set_errno (ENOENT);
      return -1;
    }

  /* Convert path.  First argument ensures that we don't check for NULL/empty/invalid
     again. */
  path_conv path (PC_NONULLEMPTY, dir, PC_FULL | PC_SYM_FOLLOW);
  if (path.error)
    {
      set_errno (path.error);
      syscall_printf ("-1 = chdir (%s)", dir);
      return -1;
    }


  /* Look for trailing path component consisting entirely of dots.  This
     is needed only in case of chdir since Windows simply ignores count
     of dots > 2 here instead of returning an error code.  Counts of dots
     <= 2 are already eliminated by normalize_posix_path. */
  const char *p = strrchr (dir, '/');
  if (!p)
    p = dir;
  else
    p++;

  size_t len = strlen (p);
  if (len > 2 && strspn (p, ".") == len)
    {
      set_errno (ENOENT);
      return -1;
    }

  char *native_dir = path.get_win32 ();

  /* Check to see if path translates to something like C:.
     If it does, append a \ to the native directory specification to
     defeat the Windows 95 (i.e. MS-DOS) tendency of returning to
     the last directory visited on the given drive. */
  if (isdrive (native_dir) && !native_dir[2])
    {
      native_dir[2] = '\\';
      native_dir[3] = '\0';
    }
  int res = SetCurrentDirectoryA (native_dir) ? 0 : -1;

  /* If res < 0, we didn't change to a new directory.
     Otherwise, set the current windows and posix directory cache from input.
     If the specified directory is a MS-DOS style directory or if the directory
     was symlinked, convert the MS-DOS path back to posix style.  Otherwise just
     store the given directory.  This allows things like "find", which traverse
     directory trees, to work correctly with Cygwin mounted directories.
     FIXME: Is just storing the posixized windows directory the correct thing to
     do when we detect a symlink?  Should we instead rebuild the posix path from
     the input by traversing links?  This would be an expensive operation but
     we'll see if Cygwin mailing list users whine about the current behavior. */
  if (res == -1)
    __seterrno ();
  else if (!path.has_symlinks () && strpbrk (dir, ":\\") == NULL
	   && pcheck_case == PCHECK_RELAXED)
    cygheap->cwd.set (path, dir);
  else
    cygheap->cwd.set (path, NULL);

  /* Note that we're accessing cwd.posix without a lock here.  I didn't think
     it was worth locking just for strace. */
  syscall_printf ("%d = chdir() cygheap->cwd.posix '%s' native '%s'", res,
		  cygheap->cwd.posix, native_dir);
  MALLOC_CHECK;
  return res;
}

extern "C"
int
fchdir (int fd)
{
  TRACE_IN;
  sigframe thisframe (mainthread);

  if (cygheap->fdtab.not_open (fd))
    {
      syscall_printf ("-1 = fchdir (%d)", fd);
      set_errno (EBADF);
      return -1;
    }
  debug_printf("chdir (%s)", cygheap->fdtab[fd]->get_name ());
  int ret = chdir (cygheap->fdtab[fd]->get_name ());
  if (!ret)
    {
      /* The name in the fhandler is explicitely overwritten with the full path.
	 Otherwise fchmod() to a path originally given as a relative path could
	 end up in a completely different directory. Imagine:

	   fd = open ("..");
	   fchmod(fd);
	   fchmod(fd);

	 The 2nd fchmod should chdir to the same dir as the first call, not
	 to it's parent dir. */
      char path[MAX_PATH];
      char posix_path[MAX_PATH];
      mount_table->conv_to_posix_path (cygheap->cwd.get (path, 0, 1),
				       posix_path, 0);
      cygheap->fdtab[fd]->set_name (path, posix_path);
    }

  syscall_printf ("%d = fchdir (%d)", ret, fd);
  return ret;
}

#if 0
static bool
QuotedRelativePath (const char *Path)
{
    if (Path[0] == '"' || Path[0] == '\'')
      {
	if (Path[1] == '/')
	  {
	    return false;
	  }
	else
	  {
	    return true;
	  }
      }
    else
      {
	return false;
      }
}
#endif

static bool
IsAbsWin32Path (const char * path)
{
  int plen = strlen (path);
  bool p0alpha = isalpha (path[0]) != 0;
  bool p1colon = (plen > 1 && path[1] == ':');
  bool rval = 
         (   ((plen == 2) && p0alpha && p1colon)
          || (  (plen > 2) 
	      && p0alpha 
	      && p1colon 
	      && (strchr (&path[2], ':') == (char *)NULL)
	     )
	  || (   plen > 3 
	      && path[0] == '\\' 
	      && path[1] == '\\' 
	      && path[3] == '\\'
	     )
	 );
    return rval;
}

static char *
ScrubRetpath (char * const retpath)
{ 
  char * sspath = (char *)retpath;
  //
  // Check for null path because Win32 doesn't like them.
  // I.E.:  Path lists of c:/foo;;c:/bar need changed to 
  // c:/foo;c:/bar.
  //
  // This need be executed only if we actually converted the path.
  //
  while (*sspath)
    {
      if (*sspath == ';' && sspath[1] == ';')
	  for (char *i = sspath; *i; i++)
	      *i = *(i + 1);
      else
	sspath++;
    }
  if (*(sspath - 1) == ';')
    *(sspath - 1) = '\0';

  //
  // If we modified the path then convert all / to \ if we have a path list
  // else convert all \ to /.
  // 
  if ((strchr (retpath, ';')))
  {
    backslashify (retpath, retpath, 0);
  } else
  {
    slashify (retpath, retpath, 0);
  }
  return retpath;
}

/******************** Exported Path Routines *********************/

/* Cover functions to the path conversion routines.
   These are exported to the world as cygwin_foo by cygwin.din.  */

extern "C"
char *
msys_p2w (char const * const path)
{
  TRACE_IN;

  int pathlen = (path ? strlen (path): 0);
  
  if (pathlen == 0)
  {
    char *retpath = (char *)malloc (sizeof (char));
    memset (retpath, 0, sizeof (char));
    return retpath;
  }

  debug_printf("msys_p2w (%s)", path);

  char *spath = (char *)alloca (pathlen + 1);
  memcpy (spath, path, pathlen + 1);
  char * sspath;
  // retpath will be what sets win32_path before exiting.
  char *retpath = (char *)malloc(((MAX_PATH - pathlen) > 0) ? 
      MAX_PATH : pathlen + MAX_PATH);
  memset (retpath, 0, MAX_PATH);
  int retpath_len = 0;
  int retpath_buflen = MAX_PATH;
    
#define retpathcat(retstr) \
  retpath_len += strlen(retstr); \
  if (retpath_buflen <= retpath_len) \
    { \
      retpath_buflen = ((retpath_buflen * 2 <= retpath_len) ? \
	  retpath_len + 1 : retpath_buflen * 2); \
      retpath = (char *)realloc (retpath, retpath_buflen); \
    } \
  strcat (retpath, retstr);

#define retpathcpy(retstr) \
  retpath_len = strlen (retstr); \
  *retpath = '\0'; \
  if (retpath_buflen <= retpath_len ) \
    { \
      retpath_buflen = ((retpath_buflen * 2 <= retpath_len) ? \
	  retpath_len + 1 : retpath_buflen * 2); \
      retpath = (char *)realloc (retpath, retpath_buflen); \
    } \
  strcpy (retpath, retstr);

  //
  // Just return win32 paths and path lists.
  //
  if (IsAbsWin32Path (path) 
      || (strchr (path, ';') > 0)
      )
    {
      return ((char *)path);
    }
  //
  // Multiple forward slashes are treated special,
  // Remove one and return for the form of //foo or ///bar
  // but just return for the form of //server/share.
  //
  else if (path[0] == '/' && path[1] == '/')
    {
      int tidx = 2;
      while (spath[tidx] && spath[tidx] == '/')
	  tidx++;
      if (strchr (&spath[tidx], '/'))
	{
	  retpathcpy (spath);
	}
      else
	{
	  retpathcpy (&spath[1]);
	}
      return ScrubRetpath (retpath);
    }
  //
  // special case confusion elimination
  // Translate a path that looks similar to /c: to c:/.
  //
  else if (path[0] == '/' && IsAbsWin32Path (path + 1))
    {
      retpathcpy (&path[1]);
      return ScrubRetpath (retpath);
    }
  //
  // Check for variable set.
  //
  else if ((sspath = strchr(spath, '=')) && isalpha (spath[0]))
    {
      if (IsAbsWin32Path (sspath + 1))
	return (char *)path;
      char *swin32_path = msys_p2w(sspath + 1);
      if (swin32_path == (sspath + 1))
	return (char *)path;
      *sspath = '\0';
      retpathcpy (spath);
      retpathcat ("=");
      retpathcat (swin32_path);
      free (swin32_path);
      return ScrubRetpath (retpath);
    }
  //
  // Check for POSIX path lists.
  // But we have to allow processing of quoted strings and switches first
  // which uses recursion so this code will be seen again.
  //
  else 
    {
      sspath = strchr (spath, ':');
      //
      // Prevent http://some.string/ from being modified.
      // 
      if ((sspath > 0 && strlen (sspath) > 2)
	  && (sspath[1] == '/')
	  && (sspath[2] == '/')
	  )
	{
	  return ((char *)path);
	}
      else
      if ((sspath > 0)
	   && (strchr (spath, '/') > 0)
	   // 
	   // Prevent strings beginning with -, ", or ' from being processed,
	   // remember that this is a recursive routine.
	   // 
	   && (strchr ("-\"\'", spath[0]) == 0)
	   // 
	   // Prevent ``foo:echo /bar/baz'' from being considered a path list.
	   // 
	   && (strlen (sspath) > 1 && strchr (":./", sspath[1]) > 0)
	   )
    {
      //
      // Yes, convert to Win32 path list.
      //
      while (sspath)
	{
	  *sspath = '\0';
	  char *swin32_path = msys_p2w (spath);
	  //
	  // Just ignore sret; swin32_path has the value we need.
	  //
	  retpathcat (swin32_path);
	  if (swin32_path != spath)
	    free (swin32_path);
	  spath = sspath + 1;
	  sspath = strchr (spath, ':');
	  retpathcat (";");
	  //
	  // Handle the last path in the list.
	  //
	  if (!sspath)
	    {
	      char *swin32_path = msys_p2w (spath);
	      retpathcat (swin32_path);
	      if (swin32_path != spath)
		free (swin32_path);
	    }
	}
      return ScrubRetpath (retpath);
    }
  else
    {
      switch (spath[0])
	{
	case '/':
	  //
	  // Just a normal POSIX path.
	  //
	  {
	    sspath = strchr (spath, '.');
	    if (sspath && *(sspath - 1) == '/' && *(sspath + 1) == '.')
	      {
		*(sspath - 1) = '\0';
		char *swin32_path = msys_p2w (spath);
		if (swin32_path == spath)
		  {
		    return ((char *)path);
		  }
		retpathcpy (swin32_path);
		retpathcat ("/");
		retpathcat (sspath);
		free (swin32_path);
		return ScrubRetpath (retpath);
	      }
	    path_conv p (spath, 0);
	    if (p.error)
	      {
		set_errno(p.error);
		return ((char *)path);
	      }
	    retpathcpy (p.get_win32 ());
	    return ScrubRetpath (retpath);
	  }
	case '-':
	  //
	  // here we check for POSIX paths as attributes to a POSIX switch.
	  //
	  sspath = strchr (spath, '=');
	  if (sspath)
	    {
	      //
	      // just use recursion if we find a set variable token.
	      //
	      *sspath = '\0';
	      if (IsAbsWin32Path (sspath + 1))
		return (char *)path;
	      char *swin32_path = msys_p2w(sspath + 1);
	      if (swin32_path == sspath + 1)
		{
		  return ((char *)path);
		}
	      retpathcpy (spath);
	      retpathcat ("=");
	      retpathcat (swin32_path);
	      free (swin32_path);
	      return ScrubRetpath (retpath);
	    }
	  else
	    {
	      sspath = (char *)spath;
	      sspath++;
	      sspath++;
	      if (*sspath == '/')
		{
#if DEBUGGING
		  debug_printf("spath = %s", spath);
#endif
		  char *swin32_path = msys_p2w (sspath);
		  if (swin32_path == sspath)
		    {
		      return ((char *)path);
		    }
		  sspath = (char *)spath;
		  sspath++;
		  sspath++;
		  *sspath = '\0';
		  retpathcpy (spath);
		  *sspath = '/';
		  retpathcat (swin32_path);
		  free (swin32_path);
		  return ScrubRetpath (retpath);
		}
	      else
		{
		  return ((char *)path);
		}
	    }
	  break;
	case '"':
	  //
	  // Handle a double quote case.
	  //
	  if (spath[1] == '/')
	    {
	      retpathcpy ("\"");
	      char *swin32_path = msys_p2w (&spath[1]);
	      if (swin32_path == &spath[1])
		{
		  return ((char *)path);
		}
	      retpathcat (swin32_path);
	      free (swin32_path);
	      return ScrubRetpath (retpath);
	    }
	  return ((char *)path);
	case '\'':
	  //
	  // Handle a single quote case.
	  //
	  if (spath[1] == '/')
	    {
	      retpathcpy ("'");
	      char *swin32_path = msys_p2w (&spath[1]);
	      if (swin32_path == &spath[1])
		{
		  return ((char *)path);
		}
	      retpathcat (swin32_path);
	      free (swin32_path);
	      return ScrubRetpath (retpath);
	    }
	  return ((char *)path);
	default:
	  //
	  // This takes care of variable_foo=/bar/baz
	  //
	  if ((sspath = strchr(spath, '=')) && (sspath[1] == '/'))
	    {
	      sspath[1] = '\0';
	      retpathcpy (spath);
	      sspath[1] = '/';
	      char *swin32_path = msys_p2w (&sspath[1]);
	      if (swin32_path == &sspath[1])
		{
		  return ((char *)path);
		}
	      retpathcat (swin32_path);
	      free (swin32_path);
	      return ScrubRetpath (retpath);
	    }
	  //
	  // Oh well, nothing special found, set win32_path same as path.
	  //
	  return ((char *)path);
	}
      }
    }
  // I should not get to this point.
  assert (false);
  return ScrubRetpath (retpath);
}

extern "C"
int
cygwin_conv_to_win32_path (const char *path, char *win32_path)
{
  TRACE_IN;
  char *tptr = msys_p2w(path);
  int rval = 0;
  if (tptr == path) {
    rval = -1;
    strcpy(win32_path, path);
  } else {
    strcpy(win32_path, tptr);
    free (tptr);
  }
  return rval;
}

extern "C"
int
cygwin_conv_to_full_win32_path (const char *path, char *win32_path)
{
  TRACE_IN;
  path_conv p (path, PC_SYM_FOLLOW | PC_FULL);
  if (p.error)
    {
      set_errno (p.error);
      return -1;
    }

  strcpy (win32_path, p.get_win32 ());
  return 0;
}

/* This is exported to the world as cygwin_foo by cygwin.din.  */

extern "C"
int
cygwin_conv_to_posix_path (const char *path, char *posix_path)
{
  TRACE_IN;
  if (check_null_empty_str_errno (path))
    return -1;
  mount_table->conv_to_posix_path (path, posix_path, 1);
  return 0;
}

extern "C"
int
cygwin_conv_to_full_posix_path (const char *path, char *posix_path)
{
  TRACE_IN;
  if (check_null_empty_str_errno (path))
    return -1;
  mount_table->conv_to_posix_path (path, posix_path, 0);
  return 0;
}

/* The realpath function is supported on some UNIX systems.  */

extern "C"
char *
realpath (const char *path, char *resolved)
{
  TRACE_IN;
  int err;

  path_conv real_path (path, PC_SYM_FOLLOW | PC_FULL);

  if (real_path.error)
    err = real_path.error;
  else
    {
      err = mount_table->conv_to_posix_path (real_path.get_win32 (), resolved, 0);
      if (err == 0)
	return resolved;
    }

  /* FIXME: on error, we are supposed to put the name of the path
     component which could not be resolved into RESOLVED.  */
  resolved[0] = '\0';

  set_errno (err);
  return NULL;
}

/* Return non-zero if path is a POSIX path list.
   This is exported to the world as cygwin_foo by cygwin.din.

DOCTOOL-START
<sect1 id="add-func-cygwin-posix-path-list-p">
  <para>Rather than use a mode to say what the "proper" path list
  format is, we allow any, and give apps the tools they need to
  convert between the two.  If a ';' is present in the path list it's
  a Win32 path list.  Otherwise, if the first path begins with
  [letter]: (in which case it can be the only element since if it
  wasn't a ';' would be present) it's a Win32 path list.  Otherwise,
  it's a POSIX path list.</para>
</sect1>
DOCTOOL-END
  */

extern "C"
int
cygwin_posix_path_list_p (const char *path)
{
  TRACE_IN;
  int posix_p = !(strchr (path, ';') || isdrive (path));
  return posix_p;
}

/* These are used for apps that need to convert env vars like PATH back and
   forth.  The conversion is a two step process.  First, an upper bound on the
   size of the buffer needed is computed.  Then the conversion is done.  This
   allows the caller to use alloca if it wants.  */

int
mount_info::conv_path_list_buf_size (const char *path_list, int to_posix_p)
{
  TRACE_IN;
  int i, num_elms, max_mount_path_len, size;
  const char *p;

  /* The theory is that an upper bound is
     current_size + (num_elms * max_mount_path_len)  */

  char delim = to_posix_p ? ';' : ':';
  p = path_list;
  for (num_elms = 1; (p = strchr (p, delim)) != NULL; ++num_elms)
    ++p;

  /* 7: strlen ("//c") + slop, a conservative initial value */
  for (max_mount_path_len = 7, i = 0; i < nmounts; ++i)
    {
      int mount_len = (to_posix_p
		       ? mount[i].posix_pathlen
		       : mount[i].native_pathlen);
      if (max_mount_path_len < mount_len)
	max_mount_path_len = mount_len;
    }

  /* 100: slop */
  size = strlen (path_list) + (num_elms * max_mount_path_len) + 100;
  return size;
}

extern "C"
int
cygwin_win32_to_posix_path_list_buf_size (const char *path_list)
{
  TRACE_IN;
  return mount_table->conv_path_list_buf_size (path_list, 1);
}

extern "C"
int
cygwin_posix_to_win32_path_list_buf_size (const char *path_list)
{
  TRACE_IN;
  return mount_table->conv_path_list_buf_size (path_list, 0);
}

extern "C"
int
cygwin_win32_to_posix_path_list (const char *win32, char *posix)
{
  TRACE_IN;
  conv_path_list (win32, posix, 1);
  return 0;
}

extern "C"
int
cygwin_posix_to_win32_path_list (const char *posix, char *win32)
{
  TRACE_IN;
  conv_path_list (posix, win32, 0);
  return 0;
}

/* cygwin_split_path: Split a path into directory and file name parts.
   Buffers DIR and FILE are assumed to be big enough.

   Examples (path -> `dir' / `file'):
   / -> `/' / `'
   "" -> `.' / `'
   . -> `.' / `.' (FIXME: should this be `.' / `'?)
   .. -> `.' / `..' (FIXME: should this be `..' / `'?)
   foo -> `.' / `foo'
   foo/bar -> `foo' / `bar'
   foo/bar/ -> `foo' / `bar'
   /foo -> `/' / `foo'
   /foo/bar -> `/foo' / `bar'
   c: -> `c:/' / `'
   c:/ -> `c:/' / `'
   c:foo -> `c:/' / `foo'
   c:/foo -> `c:/' / `foo'
 */

extern "C"
void
cygwin_split_path (const char *path, char *dir, char *file)
{
  TRACE_IN;
  int dir_started_p = 0;

  /* Deal with drives.
     Remember that c:foo <==> c:/foo.  */
  if (isdrive (path))
    {
      *dir++ = *path++;
      *dir++ = *path++;
      *dir++ = '/';
      if (!*path)
	{
	  *dir = 0;
	  *file = 0;
	  return;
	}
      if (SLASH_P (*path))
	++path;
      dir_started_p = 1;
    }

  /* Determine if there are trailing slashes and "delete" them if present.
     We pretend as if they don't exist.  */
  const char *end = path + strlen (path);
  /* path + 1: keep leading slash.  */
  while (end > path + 1 && SLASH_P (end[-1]))
    --end;

  /* At this point, END points to one beyond the last character
     (with trailing slashes "deleted").  */

  /* Point LAST_SLASH at the last slash (duh...).  */
  const char *last_slash;
  for (last_slash = end - 1; last_slash >= path; --last_slash)
    if (SLASH_P (*last_slash))
      break;

  if (last_slash == path)
    {
      *dir++ = '/';
      *dir = 0;
    }
  else if (last_slash > path)
    {
      memcpy (dir, path, last_slash - path);
      dir[last_slash - path] = 0;
    }
  else
    {
      if (dir_started_p)
	; /* nothing to do */
      else
	*dir++ = '.';
      *dir = 0;
    }

  memcpy (file, last_slash + 1, end - last_slash - 1);
  file[end - last_slash - 1] = 0;
}

/*****************************************************************************/

/* Return the hash value for the current win32 value.
   This is used when constructing inodes. */
DWORD
cwdstuff::get_hash ()
{
  TRACE_IN;
  DWORD hashnow;
  lock->acquire ();
  hashnow = hash;
  lock->release ();
  return hashnow;
}

/* Initialize cygcwd 'muto' for serializing access to cwd info. */
void
cwdstuff::init ()
{
  TRACE_IN;
  lock = new_muto (FALSE, "cwd");
}

/* Get initial cwd.  Should only be called once in a
   process tree. */
bool
cwdstuff::get_initial ()
{
  TRACE_IN;
  lock->acquire ();

  if (win32)
    return 1;

  int i;
  DWORD len, dlen;
  for (i = 0, dlen = MAX_PATH, len = 0; i < 3; dlen *= 2, i++)
    {
      win32 = (char *) crealloc (win32, dlen + 2);
      if ((len = GetCurrentDirectoryA (dlen, win32)) < dlen)
	break;
    }

  if (len == 0)
    {
      __seterrno ();
      lock->release ();
#if DEBUGGING
      debug_printf ("get_initial_cwd failed, %E");
#endif
      lock->release ();
      return 0;
    }
  set (NULL);
  return 1;	/* Leaves cwd lock unreleased */
}

/* Fill out the elements of a cwdstuff struct.
   It is assumed that the lock for the cwd is acquired if
   win32_cwd == NULL. */
void
cwdstuff::set (const char *win32_cwd, const char *posix_cwd)
{
  TRACE_IN;
  char pathbuf[MAX_PATH];

  if (win32_cwd)
    {
      lock->acquire ();
      win32 = (char *) crealloc (win32, strlen (win32_cwd) + 1);
      strcpy (win32, win32_cwd);
    }

  if (!posix_cwd)
    mount_table->conv_to_posix_path (win32, pathbuf, 0);
  else
    (void) normalize_posix_path (posix_cwd, pathbuf);

  posix = (char *) crealloc (posix, strlen (pathbuf) + 1);
  strcpy (posix, pathbuf);

  hash = hash_path_name (0, win32);

  if (win32_cwd)
    lock->release ();

  return;
}

/* Copy the value for either the posix or the win32 cwd into a buffer. */
char *
cwdstuff::get (char *buf, int need_posix, int with_chroot, unsigned ulen)
{
  TRACE_IN;
  MALLOC_CHECK;

  if (ulen)
    /* nothing */;
  else if (buf == NULL)
    ulen = (unsigned) -1;
  else
    {
      set_errno (EINVAL);
      goto out;
    }

  if (!get_initial ())	/* Get initial cwd and set cwd lock */
    return NULL;

  char *tocopy;
  if (!need_posix)
    tocopy = win32;
  else
    tocopy = posix;

  // Make sure that we have forward slashes always.
  char *pstr;
  pstr = strchr(tocopy, '\\');
  while (pstr)
    {
      *pstr = '/';
      pstr = strchr(pstr, '\\');
    }

  debug_printf("posix %s", posix);
  if (strlen (tocopy) >= ulen)
    {
      set_errno (ERANGE);
      buf = NULL;
    }
  else
    {
      if (!buf)
	buf = (char *) malloc (strlen (tocopy) + 1);
      strcpy (buf, tocopy);
      if (!buf[0])	/* Should only happen when chroot */
	strcpy (buf, "/");
    }

  lock->release ();

out:
  syscall_printf ("(%s) = cwdstuff::get (%p, %d, %d, %d), errno %d",
		  buf, buf, ulen, need_posix, with_chroot, errno);
  MALLOC_CHECK;
  return buf;
}
