dnl aclocal.m4 generated automatically by aclocal 1.4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

#serial 3

dnl From Paul Eggert.

AC_DEFUN(AC_C_BACKSLASH_A,
[
  AC_CACHE_CHECK([whether backslash-a works in strings], ac_cv_c_backslash_a,
   [AC_TRY_COMPILE([],
     [
#if '\a' == 'a'
      syntax error;
#endif
      char buf['\a' == 'a' ? -1 : 1];
      buf[0] = '\a';
      return buf[0] != "\a"[0];
     ],
     ac_cv_c_backslash_a=yes,
     ac_cv_c_backslash_a=no)])
  if test $ac_cv_c_backslash_a = yes; then
    AC_DEFINE(HAVE_C_BACKSLASH_A, 1,
      [Define if backslash-a works in C strings.])
  fi
])
# The following is taken from automake 1.4,
# except that it prefers the compiler option -Ae to "-Aa -D_HPUX_SOURCE"
# because only the former supports 64-bit integral types on HP-UX 10.20.


# serial 2

# @defmac AC_PROG_CC_STDC
# @maindex PROG_CC_STDC
# @ovindex CC
# If the C compiler in not in ANSI C mode by default, try to add an option
# to output variable @code{CC} to make it so.  This macro tries various
# options that select ANSI C on some system or another.  It considers the
# compiler to be in ANSI C mode if it handles function prototypes correctly.
#
# If you use this macro, you should check after calling it whether the C
# compiler has been set to accept ANSI C; if not, the shell variable
# @code{am_cv_prog_cc_stdc} is set to @samp{no}.  If you wrote your source
# code in ANSI C, you can make an un-ANSIfied copy of it by using the
# program @code{ansi2knr}, which comes with Ghostscript.
# @end defmac

AC_DEFUN(AM_PROG_CC_STDC,
[AC_REQUIRE([AC_PROG_CC])
AC_BEFORE([$0], [AC_C_INLINE])
AC_BEFORE([$0], [AC_C_CONST])
dnl Force this before AC_PROG_CPP.  Some cpp's, eg on HPUX, require
dnl a magic option to avoid problems with ANSI preprocessor commands
dnl like #elif.
dnl FIXME: can't do this because then AC_AIX won't work due to a
dnl circular dependency.
dnl AC_BEFORE([$0], [AC_PROG_CPP])
AC_MSG_CHECKING(for ${CC-cc} option to accept ANSI C)
AC_CACHE_VAL(am_cv_prog_cc_stdc,
[am_cv_prog_cc_stdc=no
ac_save_CC="$CC"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc -D__EXTENSIONS__
for ac_arg in "" -qlanglvl=ansi -std1 -Ae "-Aa -D_HPUX_SOURCE" "-Xc -D__EXTENSIONS__"
do
  CC="$ac_save_CC $ac_arg"
  AC_TRY_COMPILE(
[#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
/* Most of the following tests are stolen from RCS 5.7's src/conf.sh.  */
struct buf { int x; };
FILE * (*rcsopen) (struct buf *, struct stat *, int);
static char *e (p, i)
     char **p;
     int i;
{
  return p[i];
}
static char *f (char * (*g) (char **, int), char **p, ...)
{
  char *s;
  va_list v;
  va_start (v,p);
  s = g (p, va_arg (v,int));
  va_end (v);
  return s;
}
int test (int i, double x);
struct s1 {int (*f) (int a);};
struct s2 {int (*f) (double a);};
int pairnames (int, char **, FILE *(*)(struct buf *, struct stat *, int), int, int);
int argc;
char **argv;
], [
return f (e, argv, 0) != argv[0]  ||  f (e, argv, 1) != argv[1];
],
[am_cv_prog_cc_stdc="$ac_arg"; break])
done
CC="$ac_save_CC"
])
if test -z "$am_cv_prog_cc_stdc"; then
  AC_MSG_RESULT([none needed])
else
  AC_MSG_RESULT($am_cv_prog_cc_stdc)
fi
case "x$am_cv_prog_cc_stdc" in
  x|xno) ;;
  *) CC="$CC $am_cv_prog_cc_stdc" ;;
esac
])
#serial 3

dnl From Jim Meyering.
dnl
dnl Check whether struct dirent has a member named d_ino.
dnl

AC_DEFUN(jm_CHECK_TYPE_STRUCT_DIRENT_D_INO,
  [AC_REQUIRE([AC_HEADER_DIRENT])dnl
   AC_CACHE_CHECK([for d_ino member in directory struct],
		  jm_cv_struct_dirent_d_ino,
     [AC_TRY_LINK(dnl
       [
#include <sys/types.h>
#ifdef HAVE_DIRENT_H
# include <dirent.h>
#else /* not HAVE_DIRENT_H */
# define dirent direct
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif /* HAVE_SYS_NDIR_H */
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif /* HAVE_SYS_DIR_H */
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif /* HAVE_NDIR_H */
#endif /* HAVE_DIRENT_H */
       ],
       [struct dirent dp; dp.d_ino = 0;],

       jm_cv_struct_dirent_d_ino=yes,
       jm_cv_struct_dirent_d_ino=no)
     ]
   )
   if test $jm_cv_struct_dirent_d_ino = yes; then
     AC_DEFINE(D_INO_IN_DIRENT, 1,
  [Define if there is a member named d_ino in the struct describing
   directory headers.])
   fi
  ]
)
#serial 5

AC_DEFUN(jm_CHECK_DECLARATION,
[
  AC_MSG_CHECKING([whether $1 is declared])
  AC_CACHE_VAL(jm_cv_func_decl_$1,
    [AC_TRY_COMPILE([$2],
      [
#ifndef $1
char *(*pfn) = (char *(*)) $1
#endif
      ],
      eval "jm_cv_func_decl_$1=yes",
      eval "jm_cv_func_decl_$1=no")])

  if eval "test \"`echo '$jm_cv_func_decl_'$1`\" = yes"; then
    AC_MSG_RESULT(yes)
    ifelse([$3], , :, [$3])
  else
    AC_MSG_RESULT(no)
    ifelse([$4], , , [$4
])dnl
  fi
])dnl

dnl jm_CHECK_DECLARATIONS(INCLUDES, FUNCTION... [, ACTION-IF-DECLARED
dnl                       [, ACTION-IF-NOT-DECLARED]])
AC_DEFUN(jm_CHECK_DECLARATIONS,
[
  for jm_func in $2
  do
    jm_CHECK_DECLARATION($jm_func, [$1],
    [
      jm_tr_func=HAVE_DECL_`echo $jm_func | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ`
      AC_DEFINE_UNQUOTED($jm_tr_func) $3], [$4])dnl
  done
])
#serial 1

dnl FIXME: put these prerequisite-only *.m4 files in a separate
dnl directory -- otherwise, they'll conflict with existing files.

dnl These are the prerequisite macros for GNU's error.c file.
AC_DEFUN(jm_PREREQ_ERROR,
[
  AC_CHECK_FUNCS(strerror strerror_r vprintf doprnt)
  AC_HEADER_STDC
])
dnl This is a copy of autoconf 2.13, except we also check that
dnl FNM_FILE_NAME | FNM_LEADING_DIR works.
dnl
undefine([AC_FUNC_FNMATCH])
AC_DEFUN(AC_FUNC_FNMATCH,
[AC_CACHE_CHECK(for working GNU-style fnmatch, ac_cv_func_fnmatch_works,
# Some versions of Solaris, SCO, and the GNU C Library
# have a broken or incompatible fnmatch.
# So we run a test program.  If we are cross-compiling, take no chance.
# Thanks to John Oleynick, Franc,ois Pinard, and Paul Eggert for this test.
[AC_TRY_RUN([#include <fnmatch.h>
main() {
  exit (fnmatch ("a*", "abc", 0) != 0
	|| fnmatch("d*/*1", "d/s/1", FNM_FILE_NAME) != FNM_NOMATCH
	|| fnmatch("*", "x", FNM_FILE_NAME | FNM_LEADING_DIR) != 0
	|| fnmatch("x*", "x/y/z", FNM_FILE_NAME | FNM_LEADING_DIR) != 0
	|| fnmatch("*c*", "c/x", FNM_FILE_NAME | FNM_LEADING_DIR) != 0);
}],
ac_cv_func_fnmatch_works=yes, ac_cv_func_fnmatch_works=no,
ac_cv_func_fnmatch_works=no)])
if test $ac_cv_func_fnmatch_works = yes; then
  AC_DEFINE(HAVE_FNMATCH)
fi
])
#serial 3

dnl See if there's a working, system-supplied version of the getline function.
dnl We can't just do AC_REPLACE_FUNCS(getline) because some systems
dnl have a function by that name in -linet that doesn't have anything
dnl to do with the function we need.
AC_DEFUN(AM_FUNC_GETLINE,
[dnl
  am_getline_needs_run_time_check=no
  AC_CHECK_FUNC(getline,
		dnl Found it in some library.  Verify that it works.
		am_getline_needs_run_time_check=yes,
		am_cv_func_working_getline=no)
  if test $am_getline_needs_run_time_check = yes; then
    AC_CHECK_HEADERS(string.h)
    AC_CACHE_CHECK([for working getline function], am_cv_func_working_getline,
    [echo fooN |tr -d '\012'|tr N '\012' > conftest.data
    AC_TRY_RUN([
#    include <stdio.h>
#    include <sys/types.h>
#    if HAVE_STRING_H
#     include <string.h>
#    endif
    int main ()
    { /* Based on a test program from Karl Heuer.  */
      char *line = NULL;
      size_t siz = 0;
      int len;
      FILE *in = fopen ("./conftest.data", "r");
      if (!in)
	return 1;
      len = getline (&line, &siz, in);
      exit ((len == 4 && line && strcmp (line, "foo\n") == 0) ? 0 : 1);
    }
    ], am_cv_func_working_getline=yes dnl The library version works.
    , am_cv_func_working_getline=no dnl The library version does NOT work.
    , am_cv_func_working_getline=no dnl We're cross compiling.
    )])
  fi

  if test $am_cv_func_working_getline = no; then
    LIBOBJS="$LIBOBJS getline.$ac_objext"
    AC_SUBST(LIBOBJS)dnl
  fi
])
# Macro to add for using GNU gettext.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 108

AC_PREREQ(2.13)               dnl Minimum Autoconf version required.

AC_DEFUN(AM_WITH_NLS,
  [AC_MSG_CHECKING([whether NLS is requested])
    dnl Default is enabled NLS
    AC_ARG_ENABLE(nls,
      [  --disable-nls           do not use Native Language Support],
      USE_NLS=$enableval, USE_NLS=yes)
    AC_MSG_RESULT($USE_NLS)
    AC_SUBST(USE_NLS)

    USE_INCLUDED_LIBINTL=no

    dnl If we use NLS figure out what method
    if test "$USE_NLS" = "yes"; then
      AC_DEFINE(ENABLE_NLS, 1, [Define to 1 if NLS is requested.])
      AC_MSG_CHECKING([whether included gettext is requested])
      AC_ARG_WITH(included-gettext,
        [  --with-included-gettext use the GNU gettext library included here],
        nls_cv_force_use_gnu_gettext=$withval,
        nls_cv_force_use_gnu_gettext=no)
      AC_MSG_RESULT($nls_cv_force_use_gnu_gettext)

      nls_cv_use_gnu_gettext="$nls_cv_force_use_gnu_gettext"
      if test "$nls_cv_force_use_gnu_gettext" != "yes"; then
        dnl User does not insist on using GNU NLS library.  Figure out what
        dnl to use.  If gettext or catgets are available (in this order) we
        dnl use this.  Else we have to fall back to GNU NLS library.
	dnl catgets is only used if permitted by option --with-catgets.
	nls_cv_header_intl=
	nls_cv_header_libgt=
	CATOBJEXT=NONE

	AC_CHECK_HEADER(libintl.h,
	  [AC_CACHE_CHECK([for gettext in libc], gt_cv_func_gettext_libc,
	    [AC_TRY_LINK([#include <libintl.h>], [return (int) gettext ("")],
	       gt_cv_func_gettext_libc=yes, gt_cv_func_gettext_libc=no)])

	   if test "$gt_cv_func_gettext_libc" != "yes"; then
	     AC_CHECK_LIB(intl, bindtextdomain,
	       [AC_CHECK_LIB(intl, gettext)])
	   fi

	   if test "$gt_cv_func_gettext_libc" = "yes" \
	      || test "$ac_cv_lib_intl_gettext" = "yes"; then
	      AC_DEFINE(HAVE_GETTEXT, 1,
	  [Define to 1 if you have gettext and don't want to use GNU gettext.])
	      AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
		[test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)dnl
	      if test "$MSGFMT" != "no"; then
		AC_CHECK_FUNCS(dcgettext)
		AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
		AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		AC_TRY_LINK(, [extern int _nl_msg_cat_cntr;
			       return _nl_msg_cat_cntr],
		  [CATOBJEXT=.gmo
		   DATADIRNAME=share],
		  [CATOBJEXT=.mo
		   DATADIRNAME=lib])
		INSTOBJEXT=.mo
	      fi
	    fi
	])

        if test "$CATOBJEXT" = "NONE"; then
	  AC_MSG_CHECKING([whether catgets can be used])
	  AC_ARG_WITH(catgets,
	    [  --with-catgets          use catgets functions if available],
	    nls_cv_use_catgets=$withval, nls_cv_use_catgets=no)
	  AC_MSG_RESULT($nls_cv_use_catgets)

	  if test "$nls_cv_use_catgets" = "yes"; then
	    dnl No gettext in C library.  Try catgets next.
	    AC_CHECK_LIB(i, main)
	    AC_CHECK_FUNC(catgets,
	      [AC_DEFINE(HAVE_CATGETS, 1,
			 [Define as 1 if you have catgets and don't want to use GNU gettext.])
	       INTLOBJS="\$(CATOBJS)"
	       AC_PATH_PROG(GENCAT, gencat, no)dnl
	       if test "$GENCAT" != "no"; then
		 AC_PATH_PROG(GMSGFMT, gmsgfmt, no)
		 if test "$GMSGFMT" = "no"; then
		   AM_PATH_PROG_WITH_TEST(GMSGFMT, msgfmt,
		    [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)
		 fi
		 AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		   [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		 USE_INCLUDED_LIBINTL=yes
		 CATOBJEXT=.cat
		 INSTOBJEXT=.cat
		 DATADIRNAME=lib
		 INTLDEPS='$(top_builddir)/intl/libintl.a'
		 INTLLIBS=$INTLDEPS
		 LIBS=`echo $LIBS | sed -e 's/-lintl//'`
		 nls_cv_header_intl=intl/libintl.h
		 nls_cv_header_libgt=intl/libgettext.h
	       fi])
	  fi
        fi

        if test "$CATOBJEXT" = "NONE"; then
	  dnl Neither gettext nor catgets in included in the C library.
	  dnl Fall back on GNU gettext library.
	  nls_cv_use_gnu_gettext=yes
        fi
      fi

      if test "$nls_cv_use_gnu_gettext" = "yes"; then
        dnl Mark actions used to generate GNU NLS library.
        INTLOBJS="\$(GETTOBJS)"
        AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], msgfmt)
        AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
        AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
        AC_SUBST(MSGFMT)
	USE_INCLUDED_LIBINTL=yes
        CATOBJEXT=.gmo
        INSTOBJEXT=.mo
        DATADIRNAME=share
	INTLDEPS='$(top_builddir)/intl/libintl.a'
	INTLLIBS=$INTLDEPS
	LIBS=`echo $LIBS | sed -e 's/-lintl//'`
        nls_cv_header_intl=intl/libintl.h
        nls_cv_header_libgt=intl/libgettext.h
      fi

      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null 2> /dev/null; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext program is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi

      # We need to process the po/ directory.
      POSUB=po
    else
      DATADIRNAME=share
      nls_cv_header_intl=intl/libintl.h
      nls_cv_header_libgt=intl/libgettext.h
    fi
    if test -z "$nls_cv_header_intl"; then
      # Clean out junk possibly left behind by a previous configuration.
      rm -f intl/libintl.h
    fi
    AC_LINK_FILES($nls_cv_header_libgt, $nls_cv_header_intl)
    AC_OUTPUT_COMMANDS(
     [case "$CONFIG_FILES" in *po/Makefile.in*)
        sed -e "/POTFILES =/r po/POTFILES" po/Makefile.in > po/Makefile
      esac])


    # If this is used in GNU gettext we have to set USE_NLS to `yes'
    # because some of the sources are only built for this goal.
    if test "$PACKAGE" = gettext; then
      USE_NLS=yes
      USE_INCLUDED_LIBINTL=yes
    fi

    dnl These rules are solely for the distribution goal.  While doing this
    dnl we only have to keep exactly one list of the available catalogs
    dnl in configure.in.
    for lang in $ALL_LINGUAS; do
      GMOFILES="$GMOFILES $lang.gmo"
      POFILES="$POFILES $lang.po"
    done

    dnl Make all variables we use known to autoconf.
    AC_SUBST(USE_INCLUDED_LIBINTL)
    AC_SUBST(CATALOGS)
    AC_SUBST(CATOBJEXT)
    AC_SUBST(DATADIRNAME)
    AC_SUBST(GMOFILES)
    AC_SUBST(INSTOBJEXT)
    AC_SUBST(INTLDEPS)
    AC_SUBST(INTLLIBS)
    AC_SUBST(INTLOBJS)
    AC_SUBST(POFILES)
    AC_SUBST(POSUB)
  ])

AC_DEFUN(AM_GNU_GETTEXT,
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_PROG_CC])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_ISC_POSIX])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_CONST])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl

   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h string.h \
unistd.h sys/param.h])
   AC_CHECK_FUNCS([getcwd munmap putenv setenv setlocale strchr strcasecmp \
strdup __argz_count __argz_stringify __argz_next])

   if test "${ac_cv_func_stpcpy+set}" != "set"; then
     AC_CHECK_FUNCS(stpcpy)
   fi
   if test "${ac_cv_func_stpcpy}" = "yes"; then
     AC_DEFINE(HAVE_STPCPY, 1, [Define to 1 if you have the stpcpy function.])
   fi

   AM_LC_MESSAGES
   AM_WITH_NLS

   if test "x$CATOBJEXT" != "x"; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       NEW_LINGUAS=
       for lang in ${LINGUAS=$ALL_LINGUAS}; do
         case "$ALL_LINGUAS" in
          *$lang*) NEW_LINGUAS="$NEW_LINGUAS $lang" ;;
         esac
       done
       LINGUAS=$NEW_LINGUAS
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

   dnl The reference to <locale.h> in the installed <libintl.h> file
   dnl must be resolved because we cannot expect the users of this
   dnl to define HAVE_LOCALE_H.
   if test $ac_cv_header_locale_h = yes; then
     INCLUDE_LOCALE_H="#include <locale.h>"
   else
     INCLUDE_LOCALE_H="\
/* The system does not provide the header <locale.h>.  Take care yourself.  */"
   fi
   AC_SUBST(INCLUDE_LOCALE_H)

   dnl Determine which catalog format we have (if any is needed)
   dnl For now we know about two different formats:
   dnl   Linux libc-5 and the normal X/Open format
   test -d intl || mkdir intl
   if test "$CATOBJEXT" = ".cat"; then
     AC_CHECK_HEADER(linux/version.h, msgformat=linux, msgformat=xopen)

     dnl Transform the SED scripts while copying because some dumb SEDs
     dnl cannot handle comments.
     sed -e '/^#/d' $srcdir/intl/$msgformat-msg.sed > intl/po2msg.sed
   fi
   dnl po2tbl.sed is always needed.
   sed -e '/^#.*[^\\]$/d' -e '/^#$/d' \
     $srcdir/intl/po2tbl.sed.in > intl/po2tbl.sed

   dnl In the intl/Makefile.in we have a special dependency which makes
   dnl only sense for gettext.  We comment this out for non-gettext
   dnl packages.
   if test "$PACKAGE" = "gettext"; then
     GT_NO="#NO#"
     GT_YES=
   else
     GT_NO=
     GT_YES="#YES#"
   fi
   AC_SUBST(GT_NO)
   AC_SUBST(GT_YES)

   dnl If the AC_CONFIG_AUX_DIR macro for autoconf is used we possibly
   dnl find the mkinstalldirs script in another subdir but ($top_srcdir).
   dnl Try to locate is.
   MKINSTALLDIRS=
   if test -n "$ac_aux_dir"; then
     MKINSTALLDIRS="$ac_aux_dir/mkinstalldirs"
   fi
   if test -z "$MKINSTALLDIRS"; then
     MKINSTALLDIRS="\$(top_srcdir)/mkinstalldirs"
   fi
   AC_SUBST(MKINSTALLDIRS)

   dnl *** For now the libtool support in intl/Makefile is not for real.
   l=
   AC_SUBST(l)

   dnl Generate list of files to be processed by xgettext which will
   dnl be included in po/Makefile.
   test -d po || mkdir po
   case "$srcdir" in
   .)
     posrcprefix="../" ;;
   /* | [[A-Za-z]]:*)
     posrcprefix="$srcdir/" ;;
   *)
     posrcprefix="../$srcdir/" ;;
   esac
   rm -f po/POTFILES
   sed -e "/^#/d" -e "/^\$/d" -e "s,.*,	$posrcprefix& \\\\," -e "\$s/\(.*\) \\\\/\1/" \
	< $srcdir/po/POTFILES.in > po/POTFILES
  ])
#serial 3

dnl From Paul Eggert.

# Define HAVE_INTTYPES_H if <inttypes.h> exists,
# doesn't clash with <sys/types.h>, and declares uintmax_t.

AC_DEFUN(jm_AC_HEADER_INTTYPES_H,
[
  AC_CACHE_CHECK([for inttypes.h], jm_ac_cv_header_inttypes_h,
  [AC_TRY_COMPILE(
    [#include <sys/types.h>
#include <inttypes.h>],
    [uintmax_t i = (uintmax_t) -1;],
    jm_ac_cv_header_inttypes_h=yes,
    jm_ac_cv_header_inttypes_h=no)])
  if test $jm_ac_cv_header_inttypes_h = yes; then
    AC_DEFINE_UNQUOTED(HAVE_INTTYPES_H, 1,
[Define if <inttypes.h> exists, doesn't clash with <sys/types.h>,
   and declares uintmax_t. ])
  fi
])
#serial 1
dnl This test replaces the one in autoconf.
dnl Currently this macro should have the same name as the autoconf macro
dnl because gettext's gettext.m4 (distributed in the automake package)
dnl still uses it.  Otherwise, the use in gettext.m4 makes autoheader
dnl give these diagnostics:
dnl   configure.in:556: AC_TRY_COMPILE was called before AC_ISC_POSIX
dnl   configure.in:556: AC_TRY_RUN was called before AC_ISC_POSIX

undefine([AC_ISC_POSIX])
AC_DEFUN(AC_ISC_POSIX,
  [
    dnl This test replaces the obsolescent AC_ISC_POSIX kludge.
    AC_CHECK_LIB(cposix, strerror, [LIBS="$LIBS -lcposix"])
  ]
)
#serial 7

dnl From Jim Meyering.
dnl A wrapper around AC_FUNC_MKTIME.

AC_DEFUN(jm_FUNC_MKTIME,
[AC_REQUIRE([AC_FUNC_MKTIME])dnl

 dnl mktime.c uses localtime_r if it exists.  Check for it.
 AC_CHECK_FUNCS(localtime_r)

 if test $ac_cv_func_working_mktime = no; then
   AC_DEFINE_UNQUOTED(mktime, rpl_mktime,
    [Define to rpl_mktime if the replacement function should be used.])
 fi
])
#serial 19

dnl By default, many hosts won't let programs access large files;
dnl one must use special compiler options to get large-file access to work.
dnl For more details about this brain damage please see:
dnl http://www.sas.com/standards/large.file/x_open.20Mar96.html

dnl Written by Paul Eggert <eggert@twinsun.com>.

dnl Internal subroutine of AC_SYS_LARGEFILE.
dnl AC_SYS_LARGEFILE_TEST_INCLUDES
AC_DEFUN(AC_SYS_LARGEFILE_TEST_INCLUDES,
  [[#include <sys/types.h>
    /* Check that off_t can represent 2**63 - 1 correctly.
       We can't simply "#define LARGE_OFF_T 9223372036854775807",
       since some C++ compilers masquerading as C compilers
       incorrectly reject 9223372036854775807.  */
#   define LARGE_OFF_T (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
    int off_t_is_large[(LARGE_OFF_T % 2147483629 == 721
			&& LARGE_OFF_T % 2147483647 == 1)
		       ? 1 : -1];
  ]])

dnl Internal subroutine of AC_SYS_LARGEFILE.
dnl AC_SYS_LARGEFILE_MACRO_VALUE(C-MACRO, VALUE, CACHE-VAR, COMMENT, INCLUDES, FUNCTION-BODY)
AC_DEFUN(AC_SYS_LARGEFILE_MACRO_VALUE,
  [AC_CACHE_CHECK([for $1 value needed for large files], $3,
     [$3=no
      AC_TRY_COMPILE([$5],
	[$6], 
	,
	[AC_TRY_COMPILE([#define $1 $2]
[$5]
	   ,
	   [$6],
	   [$3=$2])])])
   if test "[$]$3" != no; then
     AC_DEFINE_UNQUOTED([$1], [$]$3, [$4])
   fi])

AC_DEFUN(AC_SYS_LARGEFILE,
  [AC_REQUIRE([AC_PROG_CC])
   AC_ARG_ENABLE(largefile,
     [  --disable-largefile     omit support for large files])
   if test "$enable_largefile" != no; then

     AC_CACHE_CHECK([for special C compiler options needed for large files],
       ac_cv_sys_largefile_CC,
       [ac_cv_sys_largefile_CC=no
        if test "$GCC" != yes; then
	  # IRIX 6.2 and later do not support large files by default,
	  # so use the C compiler's -n32 option if that helps.
	  AC_TRY_COMPILE(AC_SYS_LARGEFILE_TEST_INCLUDES, , ,
	    [ac_save_CC="$CC"
	     CC="$CC -n32"
	     AC_TRY_COMPILE(AC_SYS_LARGEFILE_TEST_INCLUDES, ,
	       ac_cv_sys_largefile_CC=' -n32')
	     CC="$ac_save_CC"])
        fi])
     if test "$ac_cv_sys_largefile_CC" != no; then
       CC="$CC$ac_cv_sys_largefile_CC"
     fi

     AC_SYS_LARGEFILE_MACRO_VALUE(_FILE_OFFSET_BITS, 64,
       ac_cv_sys_file_offset_bits,
       [Number of bits in a file offset, on hosts where this is settable.],
       AC_SYS_LARGEFILE_TEST_INCLUDES)
     AC_SYS_LARGEFILE_MACRO_VALUE(_LARGE_FILES, 1,
       ac_cv_sys_large_files,
       [Define for large files, on AIX-style hosts.],
       AC_SYS_LARGEFILE_TEST_INCLUDES)
   fi
  ])

AC_DEFUN(AC_FUNC_FSEEKO,
  [AC_SYS_LARGEFILE_MACRO_VALUE(_LARGEFILE_SOURCE, 1,
     ac_cv_sys_largefile_source,
     [Define to make fseeko visible on some hosts (e.g. glibc 2.2).],
     [#include <stdio.h>], [return !fseeko;])
   # We used to try defining _XOPEN_SOURCE=500 too, to work around a bug
   # in glibc 2.1.3, but that breaks too many other things.
   # If you want fseeko and ftello with glibc, upgrade to a fixed glibc.

   AC_CACHE_CHECK([for fseeko], ac_cv_func_fseeko,
     [ac_cv_func_fseeko=no
      AC_TRY_LINK([#include <stdio.h>],
        [return fseeko && fseeko (stdin, 0, 0);],
	[ac_cv_func_fseeko=yes])])
   if test $ac_cv_func_fseeko != no; then
     AC_DEFINE(HAVE_FSEEKO, 1,
       [Define if fseeko (and presumably ftello) exists and is declared.])
   fi])
# Check whether LC_MESSAGES is available in <locale.h>.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 2

AC_PREREQ(2.13)               dnl Minimum Autoconf version required.

AC_DEFUN(AM_LC_MESSAGES,
  [if test $ac_cv_header_locale_h = yes; then
    AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
      [AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
       am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
    if test $am_cv_val_LC_MESSAGES = yes; then
      AC_DEFINE(HAVE_LC_MESSAGES, 1,
		[Define if your locale.h file contains LC_MESSAGES.])
    fi
  fi])
#serial 3

dnl From Jim Meyering.
dnl Determine whether malloc accepts 0 as its argument.
dnl If it doesn't, arrange to use the replacement function.
dnl

AC_DEFUN(jm_FUNC_MALLOC,
[
 dnl xmalloc.c requires that this symbol be defined so it doesn't
 dnl mistakenly use a broken malloc -- as it might if this test were omitted.
 AC_DEFINE_UNQUOTED(HAVE_DONE_WORKING_MALLOC_CHECK, 1,
                    [Define if the malloc check has been performed. ])

 AC_CACHE_CHECK([for working malloc], jm_cv_func_working_malloc,
  [AC_TRY_RUN([
    char *malloc ();
    int
    main ()
    {
      exit (malloc (0) ? 0 : 1);
    }
	  ],
	 jm_cv_func_working_malloc=yes,
	 jm_cv_func_working_malloc=no,
	 dnl When crosscompiling, assume malloc is broken.
	 jm_cv_func_working_malloc=no)
  ])
  if test $jm_cv_func_working_malloc = no; then
    AC_SUBST(LIBOBJS)
    LIBOBJS="$LIBOBJS malloc.$ac_objext"
    AC_DEFINE_UNQUOTED(malloc, rpl_malloc,
      [Define to rpl_malloc if the replacement function should be used.])
  fi
])
# serial 8

# From Paul Eggert.

# BeOS 5 has <wchar.h> but does not define mbstate_t,
# so you can't declare an object of that type.
# Check for this incompatibility with Standard C.

# Include stdlib.h first, because otherwise this test would fail on Linux
# (at least glibc-2.1.3) because the "_XOPEN_SOURCE 500" definition elicits
# a syntax error in wchar.h due to the use of undefined __int32_t.

AC_DEFUN(AC_MBSTATE_T,
  [
   AC_CHECK_HEADERS(stdlib.h)

   AC_CACHE_CHECK([for mbstate_t], ac_cv_type_mbstate_t,
    [AC_TRY_COMPILE([
#if HAVE_STDLIB_H
# include <stdlib.h>
#endif
#include <wchar.h>],
      [mbstate_t x; return sizeof x;],
      ac_cv_type_mbstate_t=yes,
      ac_cv_type_mbstate_t=no)])
   if test $ac_cv_type_mbstate_t = no; then
     AC_DEFINE(mbstate_t, int,
	       [Define to a type if <wchar.h> does not define.])
   fi])
#serial 1000

dnl From Paul Eggert
dnl Check for a working mktime.
dnl This is a preview of what should appear in the next public autoconf release.

dnl Override any existing definition.
undefine([AC_FUNC_MKTIME])

AC_DEFUN(AC_FUNC_MKTIME,
[AC_REQUIRE([AC_HEADER_TIME])dnl
AC_CHECK_HEADERS(sys/time.h unistd.h)
AC_CHECK_FUNCS(alarm)
AC_CACHE_CHECK([for working mktime], ac_cv_func_working_mktime,
[AC_TRY_RUN(
changequote(<<, >>)dnl
<</* Test program from Paul Eggert (eggert@twinsun.com)
   and Tony Leneis (tony@plaza.ds.adp.com).  */
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if !HAVE_ALARM
# define alarm(X) /* empty */
#endif

/* Work around redefinition to rpl_putenv by other config tests.  */
#undef putenv

static time_t time_t_max;

/* Values we'll use to set the TZ environment variable.  */
static const char *const tz_strings[] = {
  (const char *) 0, "TZ=GMT0", "TZ=JST-9",
  "TZ=EST+3EDT+2,M10.1.0/00:00:00,M2.3.0/00:00:00"
};
#define N_STRINGS (sizeof (tz_strings) / sizeof (tz_strings[0]))

/* Fail if mktime fails to convert a date in the spring-forward gap.
   Based on a problem report from Andreas Jaeger.  */
static void
spring_forward_gap ()
{
  /* glibc (up to about 1998-10-07) failed this test) */
  struct tm tm;

  /* Use the portable POSIX.1 specification "TZ=PST8PDT,M4.1.0,M10.5.0"
     instead of "TZ=America/Vancouver" in order to detect the bug even
     on systems that don't support the Olson extension, or don't have the
     full zoneinfo tables installed.  */
  putenv ("TZ=PST8PDT,M4.1.0,M10.5.0");

  tm.tm_year = 98;
  tm.tm_mon = 3;
  tm.tm_mday = 5;
  tm.tm_hour = 2;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  if (mktime (&tm) == (time_t)-1)
    exit (1);
}

static void
mktime_test (now)
     time_t now;
{
  struct tm *lt;
  if ((lt = localtime (&now)) && mktime (lt) != now)
    exit (1);
  now = time_t_max - now;
  if ((lt = localtime (&now)) && mktime (lt) != now)
    exit (1);
}

static void
irix_6_4_bug ()
{
  /* Based on code from Ariel Faigon.  */
  struct tm tm;
  tm.tm_year = 96;
  tm.tm_mon = 3;
  tm.tm_mday = 0;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  mktime (&tm);
  if (tm.tm_mon != 2 || tm.tm_mday != 31)
    exit (1);
}

static void
bigtime_test (j)
     int j;
{
  struct tm tm;
  time_t now;
  tm.tm_year = tm.tm_mon = tm.tm_mday = tm.tm_hour = tm.tm_min = tm.tm_sec = j;
  now = mktime (&tm);
  if (now != (time_t) -1)
    {
      struct tm *lt = localtime (&now);
      if (! (lt
	     && lt->tm_year == tm.tm_year
	     && lt->tm_mon == tm.tm_mon
	     && lt->tm_mday == tm.tm_mday
	     && lt->tm_hour == tm.tm_hour
	     && lt->tm_min == tm.tm_min
	     && lt->tm_sec == tm.tm_sec
	     && lt->tm_yday == tm.tm_yday
	     && lt->tm_wday == tm.tm_wday
	     && ((lt->tm_isdst < 0 ? -1 : 0 < lt->tm_isdst)
		  == (tm.tm_isdst < 0 ? -1 : 0 < tm.tm_isdst))))
	exit (1);
    }
}

int
main ()
{
  time_t t, delta;
  int i, j;

  /* This test makes some buggy mktime implementations loop.
     Give up after 60 seconds; a mktime slower than that
     isn't worth using anyway.  */
  alarm (60);

  for (time_t_max = 1; 0 < time_t_max; time_t_max *= 2)
    continue;
  time_t_max--;
  delta = time_t_max / 997; /* a suitable prime number */
  for (i = 0; i < N_STRINGS; i++)
    {
      if (tz_strings[i])
	putenv (tz_strings[i]);

      for (t = 0; t <= time_t_max - delta; t += delta)
	mktime_test (t);
      mktime_test ((time_t) 60 * 60);
      mktime_test ((time_t) 60 * 60 * 24);

      for (j = 1; 0 < j; j *= 2)
        bigtime_test (j);
      bigtime_test (j - 1);
    }
  irix_6_4_bug ();
  spring_forward_gap ();
  exit (0);
}
>>,
changequote([, ])dnl
ac_cv_func_working_mktime=yes, ac_cv_func_working_mktime=no,
ac_cv_func_working_mktime=no)])
if test $ac_cv_func_working_mktime = no; then
  LIBOBJS="$LIBOBJS mktime.${ac_objext}"
fi
])
#serial 13

dnl These are the prerequisite macros for files in the lib/
dnl directories of the fileutils, sh-utils, and textutils packages.

AC_DEFUN(jm_PREREQ,
[
  jm_PREREQ_ADDEXT
  jm_PREREQ_CANON_HOST
  jm_PREREQ_DIRNAME
  jm_PREREQ_ERROR
  jm_PREREQ_GETPAGESIZE
  jm_PREREQ_HUMAN
  jm_PREREQ_MBSWIDTH
  jm_PREREQ_MEMCHR
  jm_PREREQ_QUOTEARG
  jm_PREREQ_READUTMP
  jm_PREREQ_REGEX
])

AC_DEFUN(jm_PREREQ_ADDEXT,
[
  dnl For addext.c.
  AC_SYS_LONG_FILE_NAMES
  AC_CHECK_FUNCS(pathconf)
  AC_CHECK_HEADERS(limits.h string.h unistd.h)
])

AC_DEFUN(jm_PREREQ_CANON_HOST,
[
  dnl Add any libraries as early as possible.
  dnl In particular, inet_ntoa needs -lnsl at least on Solaris5.5.1,
  dnl so we have to add -lnsl to LIBS before checking for that function.
  AC_SEARCH_LIBS(gethostbyname, [inet nsl])

  dnl These come from -lnsl on Solaris5.5.1.
  AC_CHECK_FUNCS(gethostbyname gethostbyaddr inet_ntoa)

  AC_CHECK_FUNCS(gethostbyname gethostbyaddr inet_ntoa)
  AC_CHECK_HEADERS(unistd.h string.h netdb.h sys/socket.h \
                   netinet/in.h arpa/inet.h)
])

AC_DEFUN(jm_PREREQ_DIRNAME,
[
  AC_HEADER_STDC
  AC_CHECK_HEADERS(string.h)
])

AC_DEFUN(jm_PREREQ_GETPAGESIZE,
[
  AC_CHECK_FUNCS(getpagesize)
  AC_CHECK_HEADERS(OS.h unistd.h)
])

# If you use human.c, you need the following files:
# uintmax_t.m4 inttypes_h.m4 ulonglong.m4
AC_DEFUN(jm_PREREQ_HUMAN,
[
  AC_CHECK_HEADERS(limits.h stdlib.h string.h)
  AC_CHECK_DECLS([getenv])
  AC_REQUIRE([jm_AC_HEADER_INTTYPES_H])
  AC_REQUIRE([jm_AC_TYPE_UINTMAX_T])
])

AC_DEFUN(jm_PREREQ_MEMCHR,
[
  AC_CHECK_HEADERS(limits.h stdlib.h bp-sym.h)
])

AC_DEFUN(jm_PREREQ_QUOTEARG,
[
  AC_CHECK_FUNCS(isascii iswprint mbrtowc)
  AC_CHECK_HEADERS(limits.h stdlib.h string.h wchar.h wctype.h)
  AC_HEADER_STDC
  AC_C_BACKSLASH_A
  AC_MBSTATE_T
  AM_C_PROTOTYPES
])

AC_DEFUN(jm_PREREQ_READUTMP,
[
  AC_HEADER_STDC
  AC_CHECK_HEADERS(string.h utmp.h utmpx.h sys/param.h)
  AC_CHECK_FUNCS(utmpname)
  AC_CHECK_FUNCS(utmpxname)
  AM_C_PROTOTYPES

  if test $ac_cv_header_utmp_h = yes || test $ac_cv_header_utmpx_h = yes; then
    utmp_includes="\
$ac_includes_default
#ifdef HAVE_UTMPX_H
# include <utmpx.h>
#else
# include <utmp.h>
#endif
"
    AC_CHECK_MEMBERS([struct utmpx.ut_user],,,[$utmp_includes])
    AC_CHECK_MEMBERS([struct utmp.ut_user],,,[$utmp_includes])
    AC_CHECK_MEMBERS([struct utmpx.ut_name],,,[$utmp_includes])
    AC_CHECK_MEMBERS([struct utmp.ut_name],,,[$utmp_includes])
    AC_LIBOBJ(readutmp)
  fi
])

AC_DEFUN(jm_PREREQ_REGEX,
[
  dnl FIXME: Maybe provide a btowc replacement someday: solaris-2.5.1 lacks it.
  dnl FIXME: Check for wctype and iswctype, and and add -lw if necessary
  dnl to get them.
  AC_CHECK_FUNCS(bzero bcopy isascii btowc)
  AC_CHECK_HEADERS(alloca.h libintl.h wctype.h wchar.h)
  AC_HEADER_STDC
  AC_FUNC_ALLOCA
])
# Search path for a program which passes the given test.
# Ulrich Drepper <drepper@cygnus.com>, 1996.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 1

dnl AM_PATH_PROG_WITH_TEST(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN(AM_PATH_PROG_WITH_TEST,
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test -n "[$]$1"; then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])
#serial 3

dnl From Jim Meyering.
dnl Determine whether realloc works when both arguments are 0.
dnl If it doesn't, arrange to use the replacement function.
dnl

AC_DEFUN(jm_FUNC_REALLOC,
[
 dnl xmalloc.c requires that this symbol be defined so it doesn't
 dnl mistakenly use a broken realloc -- as it might if this test were omitted.
 AC_DEFINE_UNQUOTED(HAVE_DONE_WORKING_REALLOC_CHECK, 1,
                    [Define if the realloc check has been performed. ])

 AC_CACHE_CHECK([for working realloc], jm_cv_func_working_realloc,
  [AC_TRY_RUN([
    char *realloc ();
    int
    main ()
    {
      exit (realloc (0, 0) ? 0 : 1);
    }
	  ],
	 jm_cv_func_working_realloc=yes,
	 jm_cv_func_working_realloc=no,
	 dnl When crosscompiling, assume realloc is broken.
	 jm_cv_func_working_realloc=no)
  ])
  if test $jm_cv_func_working_realloc = no; then
    AC_SUBST(LIBOBJS)
    LIBOBJS="$LIBOBJS realloc.$ac_objext"
    AC_DEFINE_UNQUOTED(realloc, rpl_realloc,
      [Define to rpl_realloc if the replacement function should be used.])
  fi
])
#serial 1002, but ported to autoconf 2.13 by eggert
# Experimental replacement for the function in the latest CVS autoconf.
# If the compile-test says strerror_r doesn't work, then resort to a
# `run'-test that works on BeOS and segfaults on DEC Unix.
# Use with the error.c file in ../lib.

undefine([AC_FUNC_STRERROR_R])

# AC_FUNC_STRERROR_R
# ------------------
AC_DEFUN([AC_FUNC_STRERROR_R],
[jm_CHECK_DECLARATION(strerror_r, [#include <string.h>])
test $jm_cv_func_decl_strerror_r != yes
AC_DEFINE_UNQUOTED(HAVE_DECL_STRERROR_R, $?,
  [Define to 1 if strerror_r is declared.])
AC_CHECK_FUNCS([strerror_r])
if test $ac_cv_func_strerror_r = yes; then
  AC_CHECK_HEADERS(string.h)
  AC_CACHE_CHECK([for working strerror_r],
		 ac_cv_func_strerror_r_works,
   [
    AC_TRY_COMPILE(
     [
#       include <stdio.h>
#       if HAVE_STRING_H
#        include <string.h>
#       endif
     ],
     [
       char buf[100];
       char x = *strerror_r (0, buf, sizeof buf);
     ],
     ac_cv_func_strerror_r_works=yes,
     ac_cv_func_strerror_r_works=no
    )
    if test $ac_cv_func_strerror_r_works = no; then
      # strerror_r seems not to work, but now we have to choose between
      # systems that have relatively inaccessible declarations for the
      # function.  BeOS and DEC UNIX 4.0 fall in this category, but the
      # former has a strerror_r that returns char*, while the latter
      # has a strerror_r that returns int.
      # This test should segfault on the DEC system.
      AC_TRY_RUN(
       [
#       include <stdio.h>
#       include <string.h>
#       include <ctype.h>

	extern char *strerror_r ();

	int
	main ()
	{
	  char buf[100];
	  char x = *strerror_r (0, buf, sizeof buf);
	  exit (!isalpha (x));
	}
       ],
       ac_cv_func_strerror_r_works=yes,
       ac_cv_func_strerror_r_works=no,
       ac_cv_func_strerror_r_works=no)
    fi
  ])
  if test $ac_cv_func_strerror_r_works = yes; then
    AC_DEFINE_UNQUOTED(HAVE_WORKING_STRERROR_R, 1,
      [Define to 1 if strerror_r returns a string.])
  fi
fi
])# AC_FUNC_STRERROR_R
#serial 5

dnl From Paul Eggert.

AC_PREREQ(2.13)

# Define uintmax_t to `unsigned long' or `unsigned long long'
# if <inttypes.h> does not exist.

AC_DEFUN(jm_AC_TYPE_UINTMAX_T,
[
  AC_REQUIRE([jm_AC_HEADER_INTTYPES_H])
  if test $jm_ac_cv_header_inttypes_h = no; then
    AC_REQUIRE([jm_AC_TYPE_UNSIGNED_LONG_LONG])
    test $ac_cv_type_unsigned_long_long = yes \
      && ac_type='unsigned long long' \
      || ac_type='unsigned long'
    AC_DEFINE_UNQUOTED(uintmax_t, $ac_type,
[  Define to unsigned long or unsigned long long
   if <inttypes.h> doesn't define.])
  fi
])
#serial 2

dnl From Paul Eggert.

AC_DEFUN(jm_AC_TYPE_UNSIGNED_LONG_LONG,
[
  AC_CACHE_CHECK([for unsigned long long], ac_cv_type_unsigned_long_long,
  [AC_TRY_LINK([unsigned long long ull = 1; int i = 63;],
    [unsigned long long ullmax = (unsigned long long) -1;
     return ull << i | ull >> i | ullmax / ull | ullmax % ull;],
    ac_cv_type_unsigned_long_long=yes,
    ac_cv_type_unsigned_long_long=no)])
  if test $ac_cv_type_unsigned_long_long = yes; then
    AC_DEFINE(HAVE_UNSIGNED_LONG_LONG, 1,
      [Define if you have the unsigned long long type.])
  fi
])
#serial 3

dnl From Jim Meyering

dnl Define HAVE_STRUCT_UTIMBUF if `struct utimbuf' is declared --
dnl usually in <utime.h>.
dnl Some systems have utime.h but don't declare the struct anywhere.

AC_DEFUN(jm_CHECK_TYPE_STRUCT_UTIMBUF,
[
  AC_CHECK_HEADERS(utime.h)
  AC_REQUIRE([AC_HEADER_TIME])
  AC_CACHE_CHECK([for struct utimbuf], fu_cv_sys_struct_utimbuf,
    [AC_TRY_COMPILE(
      [
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_UTIME_H
# include <utime.h>
#endif
      ],
      [static struct utimbuf x; x.actime = x.modtime;],
      fu_cv_sys_struct_utimbuf=yes,
      fu_cv_sys_struct_utimbuf=no)
    ])

  if test $fu_cv_sys_struct_utimbuf = yes; then
    AC_DEFINE_UNQUOTED(HAVE_STRUCT_UTIMBUF, 1,
[Define if struct utimbuf is declared -- usually in <utime.h>.
   Some systems have utime.h but don't declare the struct anywhere. ])
  fi
])
#serial 2

dnl From Jim Meyering
dnl Replace the utime function on systems that need it.

dnl FIXME

AC_DEFUN(jm_FUNC_UTIME,
[
  AC_CHECK_HEADERS(utime.h)
  AC_REQUIRE([jm_CHECK_TYPE_STRUCT_UTIMBUF])
  AC_REQUIRE([AC_FUNC_UTIME_NULL])

  if test $ac_cv_func_utime_null = no; then
    jm_FUNC_UTIMES_NULL
    AC_REPLACE_FUNCS(utime)
  fi
])
#serial 3

dnl Shamelessly cloned from acspecific.m4's AC_FUNC_UTIME_NULL,
dnl then do case-insensitive s/utime/utimes/.

AC_DEFUN(jm_FUNC_UTIMES_NULL,
[AC_CACHE_CHECK(whether utimes accepts a null argument, ac_cv_func_utimes_null,
[rm -f conftestdata; > conftestdata
AC_TRY_RUN([
/* In case stat has been defined to rpl_stat, undef it here.  */
#undef stat
#include <sys/types.h>
#include <sys/stat.h>
main() {
struct stat s, t;
exit(!(stat ("conftestdata", &s) == 0 && utimes("conftestdata", (long *)0) == 0
&& stat("conftestdata", &t) == 0 && t.st_mtime >= s.st_mtime
&& t.st_mtime - s.st_mtime < 120));
}],
  ac_cv_func_utimes_null=yes,
  ac_cv_func_utimes_null=no,
  ac_cv_func_utimes_null=no)
rm -f core core.* *.core])

    if test $ac_cv_func_utimes_null = yes; then
      AC_DEFINE_UNQUOTED(HAVE_UTIMES_NULL, 1,
			 [Define if utimes accepts a null argument])
    fi
  ]
)
#serial 2

# autoconf tests required for use of xstrtoumax.c

AC_DEFUN(jm_AC_PREREQ_XSTRTOUMAX,
[
  AC_REQUIRE([jm_AC_TYPE_UINTMAX_T])
  AC_REQUIRE([jm_AC_HEADER_INTTYPES_H])
  AC_REQUIRE([jm_AC_TYPE_UNSIGNED_LONG_LONG])
  AC_CHECK_HEADERS(stdlib.h)

  AC_CACHE_CHECK([whether <inttypes.h> defines strtoumax as a macro],
    jm_cv_func_strtoumax_macro,
    AC_EGREP_CPP([inttypes_h_defines_strtoumax], [#include <inttypes.h>
#ifdef strtoumax
 inttypes_h_defines_strtoumax
#endif],
      jm_cv_func_strtoumax_macro=yes,
      jm_cv_func_strtoumax_macro=no))

  if test "$jm_cv_func_strtoumax_macro" != yes; then
    AC_REPLACE_FUNCS(strtoumax)
  fi

  dnl We don't need (and can't compile) the replacement strtoull
  dnl unless the type `unsigned long long' exists.
  dnl Also, only the replacement strtoumax invokes strtoull,
  dnl so we need the replacement strtoull only if strtoumax does not exist.
  case "$ac_cv_type_unsigned_long_long,$jm_cv_func_strtoumax_macro,$ac_cv_func_strtoumax" in
    yes,no,no)
      AC_REPLACE_FUNCS(strtoull)
      ;;
  esac

])


# serial 1

AC_DEFUN(AM_C_PROTOTYPES,
[AC_REQUIRE([AM_PROG_CC_STDC])
AC_REQUIRE([AC_PROG_CPP])
AC_MSG_CHECKING([for function prototypes])
if test "$am_cv_prog_cc_stdc" != no; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(PROTOTYPES,1,[Define if compiler has function prototypes])
  U= ANSI2KNR=
else
  AC_MSG_RESULT(no)
  U=_ ANSI2KNR=./ansi2knr
  # Ensure some checks needed by ansi2knr itself.
  AC_HEADER_STDC
  AC_CHECK_HEADERS(string.h)
fi
AC_SUBST(U)dnl
AC_SUBST(ANSI2KNR)dnl
])

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN(AM_CONFIG_HEADER,
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

