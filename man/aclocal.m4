# aclocal.m4 for `man'
# ====================
#
# Copyright (C) 2005 by Keith Marshall <keithmarshall@users.sourceforge.net>
#
# This file is part of the man package.
#
# man is free software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2, or (at your option) any later version.
#
# man is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along
# with man; see the file COPYING.  If not, write to the Free Software
# Foundation, 51 Franklin St - Fifth Floor, Boston, MA 02110-1301, USA.
#
# Process this file with autoconf to produce a configure script.


## =========================================================== ##
## Path Name Fixup Macros for the Win32/MSYS Build Environment ##
## =========================================================== ##
#
# When building in the MSYS environment, on Win32, these ensure
# that POSIX syntax path names are transformed to their Win32
# native equivalents.
#
# These supply safe transforms;  in any build environment other
# than MSYS, they return the POSIX absolute equivalent of any
# specified relative path name, or the original POSIX path name
# unchanged, if it was already absolute.

# MSYS_AC_PREFIX_DEFAULT( PREFIX )
# --------------------------------
# Like standard AC_PREFIX_DEFAULT( PREFIX ), but resolves the assigned
# path using the MSYS_AC_CANONICAL_PATH macro, (defined below).
#
# (Ideally, autoconf should always define "ac_default_prefix" this way,
#  but, for all autoconf versions to date, we need to force it).
#
AC_DEFUN([MSYS_AC_PREFIX_DEFAULT],
[MSYS_AC_CANONICAL_PATH([ac_default_prefix],[$1])dnl
])

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
   s?/*$[]??'`
])


## ================================================================== ##
## Configuration File Identification and Standards Compliance Options ##
## ================================================================== ##
#
# MAN_FHS_ENABLE
# --------------
# Allow "--enable-fhs" configure option
# to request an FHS standards compliant installation.
#
# Note: this option is cached; use "--disable-fhs" to disable it,
# if a previous caching run of configure had "--enable-fhs".
#
AC_DEFUN([MAN_FHS_ENABLE],
[MAN_STANDARD_ENABLE([FHS], [fhs])dnl
])

# MAN_FSSTND_ENABLE
# -----------------
# Allow "--enable-fsstnd" configure option
# to request an FSSTND standards compliant installation.
#
# Note: "--enable-fhs" will override and disable this option.
# Otherwise, the option is cached; use "--disable-fsstnd" to disable it,
# if a previous caching run of configure had "--enable-fsstnd".
#
AC_DEFUN([MAN_FSSTND_ENABLE],
[MAN_STANDARD_ENABLE([FSSTND], [fsstnd], [FHS])dnl
])

# MAN_STANDARD_ENABLE( VARIABLE, STANDARD, [OVERRIDE] )
# -----------------------------------------------------
# Internal macro called by MAN_FHS_ENABLE and MAN_FSSTND_ENABLE.
# Sets VARIABLE according to the status of the "--enable-STANDARD" option.
# Forces "--disable-STANDARD" if "--enable-OVERRIDE" is active.
#
AC_DEFUN([MAN_STANDARD_ENABLE],
[AC_MSG_CHECKING([whether to adopt the $1 standard])
 AC_ARG_ENABLE([$2],
   AC_HELP_STRING([--enable-$2],
     [implement $1 standard installation paths]),
   [test x${enableval} = xno && man_cv_$1=no || man_cv_$1=yes],
   [AC_CACHE_VAL([man_cv_$1], [man_cv_$1=no])])
 m4_ifvaln([$3], [test $man_cv_$3 = no || man_cv_$1=no])dnl
 [test $man_cv_$1 = no && $1="__undef__($1)"]
 AC_SUBST([$1], [${$1-"$1"}])
 AC_MSG_RESULT([$man_cv_$1])dnl
])

# MAN_CONFIG_FILE_DEFAULT( FILENAME )
# -----------------------------------
# Set the default name for the man configuration file, (normally man.conf).
#
AC_DEFUN([MAN_CONFIG_FILE_DEFAULT], [man_config_file_default=$1])

# MAN_CONFDIR_DEFAULT( DIRNAME )
# ------------------------------
# Set the default location, where the configuration file should be installed.
#
AC_DEFUN([MAN_CONFDIR_DEFAULT], [man_confdir_default=$1/])

# MAN_CONFDIR
# -----------
# Interpret the "--with-confdir=PATH" option,
# to establish an alternative location for installation of the
# configuration file.
#
AC_DEFUN([MAN_CONFDIR],
[AC_ARG_WITH([confdir],
  AS_HELP_STRING([--with-confdir],
   [directory where the configuration file is installed [[PREFIX/lib]]]),
  [man_confdir=${withval}/],
  [man_confdir=${man_confdir_default-'${prefix}/lib/'}])
])

# MAN_CONFIG_FILE
# ---------------
# Interpret the "--with-config=FILENAME" option,
# to specify an alternative name for the configuration file.
# Alternatively, accept the "--with-config=PATHNAME" syntax for this option,
# to specify the fully qualified path name for the installed configuration file.
#
AC_DEFUN([MAN_CONFIG_FILE],
[AC_REQUIRE([MAN_CONFDIR])dnl
 AC_ARG_WITH([config],
  AS_HELP_STRING([--with-config=NAME],
   [name to use for the configuration file [[man.conf]]]),
  [man_config_file=${withval}],
  [man_config_file=${man_config_file_default-'man.conf'}])
 AC_MSG_CHECKING([where to install '${man_config_file}'])
 if test `AS_DIRNAME([${man_config_file}])` = "."
 then
  AC_SUBST([man_config_file],[${man_confdir}${man_config_file}])
 else
  AC_SUBST([man_config_file],[${man_config_file}])
 fi
 case ${man_config_file} in
  '${prefix}'*) ;;
  *) MSYS_AC_CANONICAL_PATH([man_config_file],[${man_config_file}]) ;;
 esac
 AC_MSG_RESULT([${man_config_file}])
])


## =================================================================== ##
## MANSECT Search Order, Default MANPATH and MANPATH_MAP Configuration ##
## =================================================================== ##
#
# MANSECT_SEARCH_ORDER( LIST )
# ----------------------------
# Define a colon separated LIST of manual sections,
# which will be searched in the order specified, when resolving
# references for any requested manpage.
#
AC_DEFUN([MANSECT_SEARCH_ORDER],
[AC_ARG_WITH([sections],
 AS_HELP_STRING([--with-sections=LIST],
  [colon separated ordered LIST of MANPAGE sections to search [[$1]]]),
 [sections=${withval}], [sections=$1])
 AC_SUBST([sections])dnl
])

# MANPATH_DEFAULT_INCLUDE( PATHNAME )
# -----------------------------------
# Specify a POSIX format PATHNAME,
# which is to be included in the system default MANPATH.
#
AC_DEFUN([MANPATH_DEFAULT_INCLUDE],
[MANPATH_DEFAULT_SUBST([path_]m4_translit([$1], [-/.], [___]), [$1])dnl
])

# MANPATH_DEFAULT_SUBST( VARNAME, PATHNAME )
# ------------------------------------------
# Internal macro called only by MANPATH_DEFAULT_INCLUDE,
# to assign the canonical representation of PATHNAME to the substitution
# variable VARNAME, for incorporation into man.conf by configure;
# mark as "__undef__(PATHNAME)" if PATHNAME does not exist.
# (DO NOT invoke this macro directly).
#
AC_DEFUN([MANPATH_DEFAULT_SUBST],
[AC_REQUIRE([WIN32_AC_NULLDEV])dnl
 AC_CACHE_CHECK([canonical MANPATH form for $2],
  [man_cv_$1], [MSYS_AC_CANONICAL_PATH([man_cv_$1], [$2])
  [(exec >${NULLDEV} 2>&1; cd $2) || man_cv_$1="__undef__(${man_cv_$1})"]dnl
 ])
 AC_SUBST([$1], [${man_cv_$1}])dnl
])

# MANPATH_MAP_DEFAULT( PATHDIR, MANDIR )
# --------------------------------------
# Specify a default MANPATH_MAP entry,
# such that when PATHDIR is present in the system PATH,
# then MANDIR will be added to the MANPATH.
#
AC_DEFUN([MANPATH_MAP_DEFAULT],
[MANPATH_MAP_CANONICAL([path_]m4_translit([$1], [-/.], [___]), [$1], [$2])dnl
])

# MANPATH_MAP_ALIAS( STANDARD, PRIMARY, ALIAS )
# ---------------------------------------------
# Create a remapped MANDIR alias,
# such that when STANDARD (either FHS or FSSTND) is in effect,
# then any MANPATH_MAP entry with a default MANDIR = PRIMARY mapping,
# will instead refer to MANDIR = ALIAS.
#
AC_DEFUN([MANPATH_MAP_ALIAS],
[MANPATH_REMAP([$1], m4_translit([$2], [-/.], [___]), [$3])dnl
])

# MANPATH_REMAP( STANDARD, VARNAME, ALIAS )
# -----------------------------------------
# Internal macro called by MANPATH_MAP_ALIAS, to remap the MANPATH_MAP
# substitution VARNAME to ALIAS, when STANDARD is in effect.
# (DO NOT invoke this macro directly).
#
AC_DEFUN([MANPATH_REMAP],
[AC_REQUIRE([MAN_$1_ENABLE])dnl
 test -z "$man_alias_$2" && test x$man_cv_$1 != xno && man_alias_$2="$3"
])

# MANPATH_MAP_CANONICAL( VARNAME, PATHDIR, MANDIR )
# -------------------------------------------------
# Internal macro called by MANPATH_MAP_DEFAULT,
# to assign the substitution VARNAME associated with PATHDIR,
# and to establish the appropriate MANDIR mapping.
# (DO NOT invoke this macro directly).
#
AC_DEFUN([MANPATH_MAP_CANONICAL],
[AC_CACHE_CHECK([canonical form for $2],
  [man_cv_$1], [MSYS_AC_CANONICAL_PATH([man_cv_$1], [$2])])
 MANPATH_MAP_DEFINE([man_cv_$1], m4_translit([$3], [-/.], [___]), [$3])
 AC_SUBST([$1], [${man_cv_$1}])dnl
])

# MANPATH_MAP_DEFINE( PATHDIR, VARNAME, MAPDIR )
# ----------------------------------------------
# Internal macro called only by MANPATH_MAP_CANONICAL,
# to establish the MAPDIR reference to associate with PATHDIR,
# and assign it to appropriate VARNAME substitution variables.
# (DO NOT invoke this macro directly).
#
AC_DEFUN([MANPATH_MAP_DEFINE],
[AC_MSG_CHECKING([canonical MANPATH_MAP for $$1])
 test -z "$man_alias_$2" && man_alias_$2="$3"
 if test "$man_alias_$2" = "$man_cv_alias_$2"
 then
   AC_CACHE_VAL([man_cv_map_to_$2],
     MANPATH_MAP_CACHE_ASSIGN([man_cv_map_to_$2], [man_alias_$2]))
 else
   MANPATH_MAP_CACHE_ASSIGN([man_cv_map_to_$2], [man_alias_$2])
   man_cv_alias_$2="$man_alias_$2"
 fi
 AC_SUBST([map_to_$2], [$man_cv_map_to_$2])
 AC_MSG_RESULT([${map_to_$2}])dnl
])

# MANPATH_MAP_CACHE_ASSIGN( VARNAME, MAPDIR )
# -------------------------------------------
# Internal macro called only by MANPATH_MAP_DEFINE,
# to assign the canonical representation of MAPDIR to the cached VARNAME,
# marking it as "__undef__(MAPDIR)", if MAPDIR does not exist.
# (DO NOT invoke this macro directly).
#
AC_DEFUN([MANPATH_MAP_CACHE_ASSIGN],
[AC_REQUIRE([WIN32_AC_NULLDEV])dnl
 MSYS_AC_CANONICAL_PATH([$1], [$$2])
 (exec >${NULLDEV} 2>&1; cd $$2) || $1="__undef__($$1)"dnl
])


## ================================================ ##
## Miscellaneous Package Portability Considerations ##
## ================================================ ##
#
# WIN32_AC_NULLDEV
# ----------------
# Select appropriate name for null device: Win32 = nul; else /dev/null.
# (Use the compiler, so we can set the correct value for the target platform,
#  even when we are cross compiling).
#
AC_DEFUN([WIN32_AC_NULLDEV],
[AC_MSG_CHECKING([name for NULL device])
 AC_LANG_PUSH([C])
 AC_COMPILE_IFELSE(
 [[
   #if defined(_WIN32) || defined(__CYGWIN32__)
    choke me
   #endif
 ]],
 [NULLDEV=/dev/null],
 [NULLDEV=nul])
 AC_LANG_POP([C])
 AC_MSG_RESULT([$NULLDEV])
 AC_SUBST([NULLDEV])
])  

# MAN_GREP_SILENT
# ---------------
# Identify how to make grep discard its output.
# Safest is to redirect to the null device, but IF we are sure we are
# NOT cross compiling, then try `grep -q' or `grep -s' as alternatives.
#
AC_DEFUN([MAN_GREP_SILENT],
[AC_REQUIRE([WIN32_AC_NULLDEV])dnl
 AC_MSG_CHECKING([how to run grep silently])
 man_grepsilent=">$NULLDEV 2>&1"
 if test x$cross_compiling = xno; then
   test x`echo testing | grep -q testing` = x && man_grepsilent=-q
   test x`echo testing | grep -s testing` = x && man_grepsilent=-s
 fi
 AC_MSG_RESULT([grep $man_grepsilent])
 AC_SUBST([man_grepsilent])
])

# EOF -- vim: ft=config
