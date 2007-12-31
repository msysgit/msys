# aclocal.m4 -*- Autoconf -*- vim: filetype=config
#
m4_include([m4/pkgid.m4])
m4_include([m4/iconv.m4])

# MINGW_AC_WIN32_NATIVE_HOST
# --------------------------
# Check if the runtime platform is a native Win32 host.
#
AC_DEFUN([MINGW_AC_WIN32_NATIVE_HOST],
[AC_CACHE_CHECK([whether we are building for a Win32 host], [mingw_cv_win32_host],
 AC_COMPILE_IFELSE([[
#ifdef _WIN32
 choke me
#endif]], [mingw_cv_win32_host=no], [mingw_cv_win32_host=yes]))dnl
])# MINGW_AC_WIN32_NATIVE_HOST

# MINGW_AC_HOST_CANONICAL_PREFIX
# ------------------------------
# Set the AC_SUBST variable `canonical_prefix' to the canonical form
# of `prefix', as applicable for a mingw32 host.
#
AC_DEFUN([MINGW_AC_HOST_CANONICAL_PREFIX],
[AC_SUBST([canonical_prefix])dnl
 ac_val=$prefix; test "x$ac_val" = xNONE && ac_val=$ac_default_prefix
 MSYS_AC_CANONICAL_PATH([canonical_prefix],[$ac_val])dnl
])# MINGW_AC_HOST_CANONICAL_PREFIX

# MSYS_AC_CANONICAL_PATH( VAR, PATHNAME )
# ---------------------------------------
# Set VAR to the canonically resolved absolute equivalent of PATHNAME,
# (which may be a relative path, and need not refer to any existing entity).
#
# On Win32-MSYS build hosts, the returned path is resolved to its true
# native Win32 path name, (but with slashes, not backslashes).
#
# On any other system, it is simply the result which would be obtained
# if PATHNAME represented an existing directory, and the pwd command was
# executed in that directory.
#
AC_DEFUN([MSYS_AC_CANONICAL_PATH],
[ac_dir="$2"
 pwd -W >/dev/null 2>&1 && ac_pwd_w="pwd -W" || ac_pwd_w=pwd
 until ac_val=`exec 2>/dev/null; cd "$ac_dir" && $ac_pwd_w`
 do
   ac_dir=`AS_DIRNAME(["$ac_dir"])`
 done
 ac_dir=`echo "$ac_dir" | sed 's?^[[./]]*??'`
 ac_val=`echo "$ac_val" | sed 's?/*$[]??'`
 $1=`echo "$2" | sed "s?^[[./]]*$ac_dir/*?$ac_val/?"'
   s?/*$[]??'`dnl
])# MSYS_AC_CANONICAL_PATH

# MINGW_AC_CHECK_HEADER( LISTVAR, HEADER )
# ----------------------------------------
# Invoke AC_CHECK_HEADER, to check availability of HEADER;
# if it is not found, append HEADER to the list of locally provided
# replacement headers, specified in LISTVAR.
#
AC_DEFUN([MINGW_AC_CHECK_HEADER],
[AC_CHECK_HEADER([$2],[],[$1="${$1} $2"])
 AC_SUBST([$1])dnl
])# MINGW_AC_CHECK_HEADER

# MINGW_AC_BROKEN_UNISTD_H( LISTVAR, [UPDATES = unistd.h] )
# ---------------------------------------------------------
# Check for the broken MinGW `unistd.h' which fails to define `SEEK_SET';
# if detected, add UPDATES to the list of headers scheduled for replacement
# by locally provided counterparts, by appending to LISTVAR.
# 
AC_DEFUN([MINGW_AC_BROKEN_UNISTD_H],
[AC_BEFORE([AC_CHECK_HEADER])dnl
 AC_CACHE_VAL([mingw_cv_unistd_updates],
 [MINGW_AC_CHECK_DEFINED([SEEK_SET], [unistd.h])
  test x${mingw_cv_SEEK_SET_defined} = xno && mingw_cv_unistd_updates="$2" \
   || mingw_cv_unistd_updates=none
  test "x${mingw_cv_unistd_updates}" = x && mingw_cv_unistd_updates=unistd.h
 ])dnl
 AC_MSG_CHECKING([updated headers required by broken unistd.h])
 test "x${mingw_cv_unistd_updates}" = xnone || $1="${$1} ${mingw_cv_unistd_updates}"
 AC_MSG_RESULT([${mingw_cv_unistd_updates}])
 AC_SUBST([$1])dnl
])# MINGW_AC_BROKEN_UNISTD_H

# MINGW_AC_CHECK_DEFINED( NAME, HEADER )
# --------------------------------------
# Check if HEADER #defines NAME.  This is similar to AC_CHECK_DECL,
# except that it isn't misled by any definition of NAME, in any of
# the default headers.
#
AC_DEFUN([MINGW_AC_CHECK_DEFINED],
[AC_CACHE_CHECK([whether $2 defines $1], [mingw_cv_$1_defined],
 AC_COMPILE_IFELSE([[
#include <$2>
#ifndef $1
 choke me
#endif]],[mingw_cv_$1_defined=yes],[mingw_cv_$1_defined=no]))dnl
])# MINGW_AC_CHECK_DEFINED

# MINGW_AC_HOST_PATH_SEPARATOR( VARNAME )
# ---------------------------------------
# Determine the separator character used by the runtime host,
# as it must appear in the VARNAME path compiled into applications,
# and assign it to the AC_SUBSTed variable, HOST_PATH_SEPARATOR.
# (This is definitively `;' on MS-DOS/Win32, or `:' otherwise).
#
AC_DEFUN([MINGW_AC_HOST_PATH_SEPARATOR],
[AC_CACHE_CHECK([$1 separator character used at runtime],
[mingw_cv_host_path_separator],
 AC_COMPILE_IFELSE([[
#if defined _WIN32 || defined __MS_DOS__
 choke me
#endif]], [mingw_cv_host_path_separator=':'], [mingw_cv_host_path_separator=';']))
 AC_SUBST([HOST_PATH_SEPARATOR], [${mingw_cv_host_path_separator}])dnl
])# MINGW_AC_HOST_PATH_SEPARATOR

# MINGW_AC_HOST_CONFIG_DLL( VERSION )
# -----------------------------------
# Configure for building DLLs, when the host is Win32, and assign
# VERSION, as the DLL release identifier.
#
AC_DEFUN([MINGW_AC_HOST_CONFIG_DLL],
[AC_REQUIRE([MINGW_AC_WIN32_NATIVE_HOST])dnl
 AC_SUBST([MAKE_DLL], [`test ${mingw_cv_win32_host} = yes && echo all-dll`])
 AC_MSG_CHECKING([release version for mingw32 DLLs])
 AC_SUBST([DLLVERSION],[-][$1])
 AC_MSG_RESULT([${DLLVERSION}])dnl
])# MINGW_AC_HOST_CONFIG_DLL

# MINGW_AC_LC_EXTENSIONS( LISTVAR )
# ---------------------------------
# Check if the system's `locale.h' supports `MINGW32_LC_EXTENSIONS';
# if not, and if compiling for a Win32 host, substitute a local replacement,
# (but do not install it), for use in the build process.
#
AC_DEFUN([MINGW_AC_LC_EXTENSIONS],
[AC_REQUIRE([MINGW_AC_WIN32_NATIVE_HOST])dnl
 test x$mingw_cv_win32_host = xyes || mingw_cv_lc_extensions="not required"
 AC_CHECK_HEADER([locale.h],[],[mingw_cv_lc_extensions=${mingw_cv_lc_extensions-no}])
 AC_CACHE_CHECK([locale.h for MINGW32_LC_EXTENSIONS],[mingw_cv_lc_extensions],
 [AC_COMPILE_IFELSE([[
#include <locale.h>
#ifndef MINGW32_LC_EXTENSIONS
 choke me
#endif]],[mingw_cv_lc_extensions=yes],[mingw_cv_lc_extensions=no])
 ])
 if test "$mingw_cv_lc_extensions" = "not required"; then
   AC_MSG_CHECKING([for _mingw_setlocale])
   AC_MSG_RESULT([not required])
 else
   AC_CHECK_FUNCS([_mingw_setlocale],[],[GENCAT_AC_OBJECTS_ADD([setlocale])])
 fi
 test "x$mingw_cv_lc_extensions" = xno && $1="${$1} locale.h"
 AC_SUBST([$1])dnl
])# MINGW_AC_LC_EXTENSIONS

# CATGETS_AC_CONFIG_TARGET
# ------------------------
# Check if building a locally hosted tool chain, targetting a foreign
# Win32 runtime host; if this is the case, configure a cross-hosted build
# in directory `cross-build', where the target libraries will be built.
#
AC_DEFUN([CATGETS_AC_CONFIG_TARGET],
[AC_SUBST([MAKE_TARGETS], [all-native])
 if test -n "$target_alias" && test x"$target_alias" != x"$host_alias"
 then (
  case $srcdir in
   .*) ac_command="$SHELL ../$srcdir/$as_me" ;;
    *) ac_command="$SHELL `cd $srcdir && pwd`/$as_me" ;;
  esac
  test -d cross-build || mkdir cross-build; cd cross-build; ac_dir=`pwd`
  AC_MSG_NOTICE([entering directory `$ac_dir'])
  AC_MSG_NOTICE([configuring component build for target `$target_alias'])
  if test -z "$build_alias"
  then
   build_alias=`uname -m 2>/dev/null || echo unknown`
   build_alias=$build_alias-`uname -s 2>/dev/null || echo unknown`
   build_alias=`echo "$build_alias" | tr $as_cr_LETTERS $as_cr_letters`
  fi
  eval $ac_command `eval set x $ac_configure_args; shift
   ac_opt="" ac_dashdash=""
   for ac_arg
   do
    ac_optarg_needed=no
    case $ac_dashdash$ac_opt$ac_arg in
     --) ac_dashdash=yes
      ;;
     -build | --build | --buil | --bui | --bu |\
     -target | --target | --targe | --targ | --tar | --ta | --t |\
     -host | --host | --hos | --ho )
      ac_optarg_needed=yes
      ;;
     build_alias=* | host_alias=* | target_alias=* |\
     -build=* | --build=* | --buil=* | --bui=* | --bu=* |\
     -host=* | --host=* | --hos=* | --ho=* )
      ;;
     -target=* | --target=* |\
     --targe=* | --targ=* | --tar=* | --ta=* | --t=* )
      echo "'--build=$build_alias' '--host=$target_alias'"
      ;;
     *) echo "'$ac_arg'"
      ;;
    esac
    test $ac_optarg_needed = yes && ac_opt="$ac_arg=" || ac_opt=""
   done`
  AC_MSG_NOTICE([leaving directory `$ac_dir']) )
  MAKE_TARGETS=all-cross-hosted
 fi[]dnl
])# CATGETS_AC_CONFIG_TARGET

AC_DEFUN([GENCAT_AC_OBJECTS_INIT],
[GENCAT_OBJECTS=${GENCAT_OBJECTS-'gencat.$(OBJEXT)'}
 AC_SUBST([GENCAT_OBJECTS])dnl
])# GENCAT_AC_OBJECTS_INIT

AC_DEFUN([GENCAT_AC_OBJECTS_ADD],
[AC_REQUIRE([GENCAT_AC_OBJECTS_INIT])dnl
 GENCAT_OBJECTS=${GENCAT_OBJECTS}' $1.$(OBJEXT)'dnl
])# GENCAT_AC_OBJECTS_ADD

AC_DEFUN([GENCAT_AC_FUNC_BASENAME],
[AC_REQUIRE([GENCAT_AC_OBJECTS_INIT])dnl
 AC_CHECK_FUNCS([basename],[],GENCAT_AC_OBJECTS_ADD([basename]))dnl
])# GENCAT_AC_FUNC_BASENAME

AC_DEFUN([GENCAT_AC_FUNC_MKSTEMP],
[AC_REQUIRE([GENCAT_AC_OBJECTS_INIT])dnl
 AC_CHECK_FUNCS([mkstemp],[],GENCAT_AC_OBJECTS_ADD([mkstemp]))dnl
])# GENCAT_AC_FUNC_MKSTEMP

AC_DEFUN([GENCAT_AC_FUNC_NL_LANGINFO],
[AC_REQUIRE([GENCAT_AC_OBJECTS_INIT])dnl
 AC_CHECK_FUNCS([nl_langinfo],[],GENCAT_AC_OBJECTS_ADD([langinfo]))dnl
])# GENCAT_AC_FUNC_NL_LANGINFO

AC_DEFUN([CATGETS_AC_CONFIG_VERSION],
[CATGETS_AC_CONFIG_VERSION_DEFINE([CATGETS_VERSION_MAJOR],[major],[2])dnl
 CATGETS_AC_CONFIG_VERSION_DEFINE([CATGETS_VERSION_MINOR],[minor],[3])dnl
])# CATGETS_AC_CONFIG_VERSION

AC_DEFUN([CATGETS_AC_CONFIG_VERSION_DEFINE],
[AC_DEFINE_UNQUOTED([$1],[`IFS=.;set x $PACKAGE_VERSION;echo ${$3}`],
 [Define numerically to the catgets $2 version number])dnl
])# CATGETS_AC_CONFIG_VERSION_DEFINE

# $RCSfile: aclocal.m4,v $Revision: 1.3 $: end of file
