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


## ======================================================== ##
## Support Program Detection and Command Line Configuration ##
## ======================================================== ##
#
# MAN_FILTER_PREFERRED( CLASS, COMMAND )
# --------------------------------------
# Specify the preferred COMMAND to be used
# to invoke the filter program for the specified filter CLASS.
#
AC_DEFUN([MAN_FILTER_PREFERRED],
[MAN_FILTER_CONFIGURE_PREFERRED([$1], m4_translit([$1], [A-Z], [a-z]), [$2])dnl
])

# MAN_FILTER_CONFIGURE_PREFERRED( CLASS, VARNAME, COMMAND )
# ---------------------------------------------------------
# Internal macro, invoked only by MAN_FILTER_PREFERRED,
# to identify the absolute path name for the program required by COMMAND,
# and assign the full path qualified invocation command line to VARNAME,
# (a lower case transform of CLASS).
#
AC_DEFUN([MAN_FILTER_CONFIGURE_PREFERRED],
[AC_ARG_WITH([$2],
  AS_HELP_STRING([--with-$2=COMMAND], [use COMMAND @<:@$3@:>@ to invoke $1]),
  MAN_AC_PATH_COMMAND_OVERRIDE_CACHE([$2], [$withval]),
  MAN_AC_PATH_COMMAND([$2], [$3]))dnl
])

# MAN_FILTER_ALTERNATE( CLASS, COMMAND )
# --------------------------------------
# Specify an alternative COMMAND which may be used
# to invoke an alternative filter program for the specified filter CLASS,
# if the program required by any earlier COMMAND preference for this CLASS
# cannot be located, when configure is executed.
#
AC_DEFUN([MAN_FILTER_ALTERNATE],
[MAN_FILTER_CONFIGURE_ALTERNATE([$1], m4_translit([$1], [A-Z], [a-z]), [$2])dnl
])

# MAN_FILTER_CONFIGURE_ALTERNATE( CLASS, VARNAME, COMMAND )
# ---------------------------------------------------------
# Internal macro, invoked only by MAN_FILTER_ALTERNATE,
# to identify the absolute path name for the program required by COMMAND,
# and assign the full path qualified invocation command line to VARNAME,
# (a lower case transform of CLASS); does NOTHING, if VARNAME has been
# previously assigned a non-empty value.
#
AC_DEFUN([MAN_FILTER_CONFIGURE_ALTERNATE],
[AS_IF([test -z "$$2"], MAN_AC_PATH_COMMAND([$2], [$3]))dnl
])

# MAN_AC_PATH_COMMAND( VARNAME, COMMAND, [DEFAULT], [PATH] )
# ----------------------------------------------------------
# A modular reimplementation of autoconf's standard AC_PATH_PROG macro,
# with enhanced cache handling, and modified to preserve COMMAND arguments.
# (Handling for the DEFAULT argument is currently unimplemented).
#
AC_DEFUN([MAN_AC_PATH_COMMAND],
[MAN_AC_MSG_PATH_PROG_CHECKING([ac_word], [$2])
 AC_CACHE_VAL([ac_cv_path_$1],
   [MAN_AC_PATH_COMMAND_RESOLVE([ac_cv_path_$1], [$ac_word], [$4])])
 MAN_AC_MSG_PATH_PROG_RESULT([$1], [ac_cv_path_$1])dnl
])

# MAN_AC_PATH_COMMAND_OVERRIDE_CACHE( VARNAME, COMMAND, [DEFAULT], [PATH] )
# -------------------------------------------------------------------------
# An alternative reimplementation of autoconf's AC_PATH_PROG macro...
# This version ALWAYS overrides any prior result in config.cache!
#
AC_DEFUN([MAN_AC_PATH_COMMAND_OVERRIDE_CACHE],
[MAN_AC_MSG_PATH_PROG_CHECKING([ac_word], [$2])
 MAN_AC_PATH_COMMAND_RESOLVE([ac_cv_path_$1], [$ac_word], [$4])
 MAN_AC_MSG_PATH_PROG_RESULT([$1], [ac_cv_path_$1])dnl
])

# MAN_AC_MSG_PATH_PROG_CHECKING( VARNAME, COMMAND )
# -------------------------------------------------
# A component of the modular reimplementation of AC_PATH_PROG...
# This component macro displays the "checking for ..." message.
#
AC_DEFUN([MAN_AC_MSG_PATH_PROG_CHECKING],
[# Extract the first word of "$2", so it can be a program name with args.
  set dummy $2; $1=$[2]
  AC_MSG_CHECKING([for $$1])dnl
])

# MAN_AC_PATH_COMMAND_RESOLVE( VARNAME, PROGNAME, [PATH] )
# --------------------------------------------------------
# Resolve PROGNAME to its absolute canonical path name,
# by searching PATH, (default as defined in the environment).
# Append any arguments captured by MAN_AC_MSG_PATH_PROG_CHECKING,
# and assign the resolved command to VARNAME.
#
AC_DEFUN([MAN_AC_PATH_COMMAND_RESOLVE],
[MAN_AC_PATH_RESOLVE([$1], [$2], [$3])
[shift 2
case ${#} in
  0) ;;
  *) test -n "$$1" && $1="$$1 ${@}" ;;
esac]dnl
])

# MAN_AC_PATH_RESOLVE( VARNAME, PROGNAME, [PATH] )
# ------------------------------------------------
# A component of the modular reimplementation of AC_PATH_PROG...
# This internal macro, based on the PATH search algorithm in AC_PATH_PROG,
# walks PATH (default as defined in the environment), searching for PROGNAME.
# If found, it sets VARNAME to the canonical pathname for the program.
#
## !!! WARNING !!! ##
#
# This macro employs undocumented m4sugar internals.
# Behaviour is correct for autoconf version 2.59, but may require
# review, if these m4sugar macros are modified for some future
# version of autoconf.
#
AC_DEFUN([MAN_AC_PATH_RESOLVE],
[_AS_PATH_WALK([$3],
[for ac_exec_ext in '' $ac_executable_extensions; do
  if AS_EXECUTABLE_P(["$as_dir/$2$ac_exec_ext"])
  then
    MSYS_AC_CANONICAL_PATH([$1], ["$as_dir/$2$ac_exec_ext"])
    echo "$as_me:$LINENO: found $$1" >&AS_MESSAGE_LOG_FD
    break 2
  fi
done])dnl
])

# MAN_AC_MSG_PATH_PROG_RESULT( CACHENAME, VARNAME )
# -------------------------------------------------
# A component of the modular reimplementation of AC_PATH_PROG...
# This component macro appends the result to the "checking for ..." message.
#
AC_DEFUN([MAN_AC_MSG_PATH_PROG_RESULT],
[AC_SUBST([$1], [$$2])
 test -n "$$1" && ac_word="$$1" || ac_word=no
 AC_MSG_RESULT([$ac_word])dnl
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

# MAN_DISABLE_NROFF_SGR
# ---------------------
# If the host's nroff implementation is based on groff >= 1.18,
# then it may be necessary to disable its default SGR output mode;
# modify the "nroff" substitution variable as required.
#
AC_DEFUN([MAN_DISABLE_NROFF_SGR],
[AC_REQUIRE([WIN32_AC_NULLDEV])dnl
 if test "x$nroff" != xno
 then
   AC_MSG_CHECKING([whether nroff requires suppression of SGR output])
   AC_ARG_ENABLE([sgr],
     AS_HELP_STRING([--enable-sgr],
       [use nroff's SGR output mode, if available]),
     MAN_NROFF_SGR_CHECK([$enableval]),dnl
     MAN_NROFF_SGR_CHECK([no]))dnl
   AC_MSG_RESULT([$man_sgr_check])
   if test x$man_sgr_check = xyes
   then
     AC_MSG_CHECKING([whether installed nroff supports SGR output])
     man_sgr_check=`(echo .TH; echo .SH TEST) | $nroff 2>$NULLDEV`
     case $man_sgr_check in
       [*0m*) man_sgr_check=yes ;;]
       [*)    man_sgr_check=no  ;;]
     esac
     AC_MSG_RESULT([$man_sgr_check])
   fi
   if test x$man_sgr_check = xyes
   then
     AC_MSG_CHECKING([how to suppress nroff's SGR output])
     man_sgr_check=unknown
     for ac_val in -P-c -c
     do
       if test "$man_sgr_check" = unknown
       then
	 case `(echo .TH; echo .SH TEST) | $nroff $ac_val 2>$NULLDEV \
	 || echo 0m` in
	   [*0m*) ;;]
	   [*) man_sgr_check="$nroff $ac_val" ;;]
	 esac
       fi
     done
     test "$man_sgr_check" = unknown || nroff=$man_sgr_check
     AC_MSG_RESULT([$man_sgr_check])
   fi
 fi[]dnl
])

# MAN_NROFF_SGR_CHECK
# -------------------
# Internal macro, called only by MAN_DISABLE_NROFF_SGR.
# Check if we are able to verify the availability of nroff SGR output;
# (we can't when we are cross compiling, and we won't bother if the user
#  explicitly configures with the '--enable-sgr' option).
#
AC_DEFUN([MAN_NROFF_SGR_CHECK],
[man_sgr_check=indeterminate
 test x$cross_compiling = xno && man_sgr_check=yes
 test x$1 = xno || man_sgr_check=no[]dnl
])

# EOF -- vim: ft=config
