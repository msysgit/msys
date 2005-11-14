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


## ================================================ ##
## Autoconf Extensions to Support Cross Compilation ##
## ================================================ ##
#
# MAN_AC_PROG_CC_VARS
# -------------------
# Define a custom set of C compiler control variables,
# supporting independent settings for a native code compiler,
# and for the primary compiler, if cross-compiling.
#
AC_DEFUN([MAN_AC_PROG_CC_VARS],dnl
[AC_ARG_VAR([CC],dnl
   [the C compiler for package deliverables])dnl
 AC_ARG_VAR([CFLAGS],dnl
   [C compiler flags to use with the CC compiler])dnl
 AC_ARG_VAR([LDFLAGS],dnl
   [linker flags for package deliverables, e.g. -L<lib_dir> if]dnl
   [you have libraries in a non-standard directory <lib_dir>])dnl
 AC_ARG_VAR([CPPFLAGS],dnl
   [C/C++ preprocessor flags for package deliverables,]dnl
   [e.g. -I<include_dir> if you have header files in a]dnl
   [non-standard directory <include_dir>])dnl
 AC_ARG_VAR([BUILD_CC],dnl
   [the C compiler for native code executables,]dnl
   [if any are needed when cross compiling])dnl
 AC_ARG_VAR([BUILD_CFLAGS],dnl
   [C compiler flags to use with the BUILD_CC compiler])dnl
 AC_ARG_VAR([BUILD_LDFLAGS],dnl
   [linker flags for native code executables])dnl
 AC_ARG_VAR([BUILD_CPPFLAGS],dnl
   [C/C++ preprocessor flags for native code])dnl
])

# MAN_AC_CONFIG_NATIVE( SUBDIR )
# ------------------------------
# Configure a subdirectory, SUBDIR, in which all executables
# will be compiled with a native code compiler, even if the primary
# compiler for the build is a cross-compiler.
#
# Propagates:
#   BUILD_CC        to  CC
#   BUILD_CFLAGS    to  CFLAGS
#   BUILD_CPPFLAGS  to  CPPFLAGS
#   BUILD_LDFLAGS   to  LDFLAGS
#
# for use by the native code compiler.
#
# Does NOT propagate any other command line settings, especially those
# of the `prefix' family, as it is assumed there is no need to install
# any locally compiled native tool, nor any of the `host_alias' family,
# as these would compromise detection of the native code compiler;
# however, it WILL use its own local `config.cache' file, if the
# parent `configure' is invoked with caching active.
#
AC_DEFUN([MAN_AC_CONFIG_NATIVE],dnl
[AC_REQUIRE([MAN_AC_PROG_CC_VARS])dnl
 ac_dir=`pwd`
 AS_MKDIR_P([$1])
 AC_MSG_NOTICE([entering directory \SQ([$ac_dir/$1])...])
 AC_MSG_NOTICE([executables built here need a native code compiler,])
 AC_MSG_NOTICE([even when we are cross-compiling.])
 ac_config_cmd=`cd $srcdir/$1; pwd`/configure
 test -r ${ac_config_cmd}.gnu && ac_config_cmd=${ac_config_cmd}.gnu
 test "$cache_file" = "/dev/null" || ac_config_cmd="$ac_config_cmd -C"
 AC_FOREACH([VAR], [CC CFLAGS LDFLAGS CPPFLAGS],
   [MAN_AC_ARG_VAR_PROPAGATE([VAR], [BUILD_]VAR)])
 ( cd $1; $CONFIG_SHELL $ac_config_cmd ) && ac_fail=false || ac_fail=true
 AC_MSG_NOTICE([leaving directory \SQ($ac_dir/$1)])
 if $ac_fail
 then
   AC_MSG_FAILURE([invalid configuration in \SQ($1) subdirectory])
 fi[]dnl
])

# MAN_AC_ARG_VAR_PROPAGATE( FROMVAR, TOVAR )
# ------------------------------------------
# Helper macro used only by MAN_AC_CONFIG_NATIVE...
# Add TOVAR to the subdirectory configuration command's argument list,
# setting equal to the value of FROMVAR, in the parent configuration,
# for propagation to the subdirectory configuration.
#
AC_DEFUN([MAN_AC_ARG_VAR_PROPAGATE],dnl
[test x"$cross_compiling" = xno && ac_val=$$1 || ac_val=""
 ac_config_cmd="$ac_config_cmd $1=$m4_if([$2], [], [ac_val], [$2])"
])


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
  AS_HELP_STRING([--with-confdir=DIR],
   [install configuration file in DIR [[PREFIX/lib]]]),
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
   [NAME of configuration file [[man.conf]]]),
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

# MANSECT_FILENAME_EXT( SECTION, SUBSTNAME, [EXTENSION] )
# -------------------------------------------------------
# Use EXTENSION, (SECTION if EXTENSION is undefined), as the
# filename extension for manpage files in SECTION; pass the definition
# to configuration output files in @SUBSTNAME@.
#
AC_DEFUN([MANSECT_FILENAME_EXT],
[MAN_SECTION_FILEXT([$1], [$2], m4_if([$3], [], [$1], [$3]))dnl
])

# MAN_SECTION_FILEXT( SECTION, SUBSTNAME, EXTENSION )
# ---------------------------------------------------
# Helper macro used only by MANSECT_FILENAME_EXT.
# Use EXTENSION as the filename extension for manpage files in SECTION;
# pass the definition to configuration output files in @SUBSTNAME@.
#
m4_define([MAN_SECTION_FILEXT],
[AC_MSG_CHECKING([file name extension for section $1 manpages])
 AC_ARG_WITH([$2],
  [AS_HELP_STRING([--with-$2=EXT],
    [use EXT]) MAN_AS_HELP_DEFAULT([$3])[ as extension for section $1 manpage files]],
  [AC_SUBST([$2], [$withval])],
  [AC_SUBST([$2], [$3])dnl
 ])
 AC_MSG_RESULT([$$2])dnl
])

# MANPATH_DEFAULT_INCLUDE( PATHNAME )
# -----------------------------------
# Specify a POSIX format PATHNAME,
# which is to be included in the system default MANPATH.
#
AC_DEFUN([MANPATH_DEFAULT_INCLUDE],
[MANPATH_DEFAULT_SUBST([path_]man_path_transform([$1]), [$1])dnl
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

# man_path_transform( PATHNAME )
# ------------------------------
# Helper macro, used by MANPATH_DEFAULT_INCLUDE and its kin,
# to transform PATHNAME to a legal VARNAME for use in AC_SUBST.
# PATHNAMEs with `*' globbing are supported, but note that they will be
# marked as `__undef__(PATHNAME)' by MANPATH_DEFAULT_SUBST, and also
# when specified as a MANPATH_MAP_DEFAULT target.
# (DO NOT invoke this macro directly).
#
m4_define([man_path_transform],
[m4_translit(m4_bpatsubst([$1], [\*], [__STAR__]), [-/.], [___])])

# MANPATH_MAP_DEFAULT( PATHDIR, MANDIR )
# --------------------------------------
# Specify a default MANPATH_MAP entry,
# such that when PATHDIR is present in the system PATH,
# then MANDIR will be added to the MANPATH.
#
AC_DEFUN([MANPATH_MAP_DEFAULT],
[MANPATH_MAP_CANONICAL([path_]man_path_transform([$1]), [$1], [$2])dnl
])

# MANPATH_MAP_ALIAS( STANDARD, PRIMARY, ALIAS )
# ---------------------------------------------
# Create a remapped MANDIR alias,
# such that when STANDARD (either FHS or FSSTND) is in effect,
# then any MANPATH_MAP entry with a default MANDIR = PRIMARY mapping,
# will instead refer to MANDIR = ALIAS.
#
AC_DEFUN([MANPATH_MAP_ALIAS],
[MANPATH_REMAP([$1], man_path_transform([$2]), [$3])dnl
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
 MANPATH_MAP_DEFINE([man_cv_$1], man_path_transform([$3]), [$3])
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
## National Language Support and Language Selection ##
## ================================================ ##
#
# MAN_NLS_HEADERS
# ---------------
# Confirm that all header files needed to support NLS are present.
#
AC_DEFUN([MAN_NLS_HEADERS],
[AC_CHECK_HEADERS([langinfo.h locale.h nl_types.h])dnl
 [man_nls_headers=$ac_cv_header_langinfo_h]
 [test x$ac_cv_header_locale_h = xno && man_nls_headers=no]
 [test x$ac_cv_header_nl_types_h = xno && man_nls_headers=no]dnl
])

# MAN_NLS_FUNCTIONS
# -----------------
# Confirm that all functions needed to support NLS are available.
#
AC_DEFUN([MAN_NLS_FUNCTIONS],
[AC_CHECK_FUNCS([nl_langinfo setlocale])
 [man_nls_funcs=$ac_cv_func_nl_langinfo]
 [test x$ac_cv_func_setlocale = xno && man_nls_funcs=no]dnl
])

# MAN_NLS_PREREQUISITES
# ---------------------
# Check if the system provides prerequisite support for NLS;
# the user can override the requirement by specifying `--disable-nls'.
#
AC_DEFUN([MAN_NLS_PREREQUISITES],
[AC_REQUIRE([MAN_NLS_HEADERS])dnl
 AC_REQUIRE([MAN_NLS_FUNCTIONS])dnl
 AC_MSG_CHECKING([whether NLS support is requested])
 AC_ARG_ENABLE([nls],
   AS_HELP_STRING([--disable-nls],
     [don't include National Language Support]),
   [man_enable_nls=$enableval],
   [man_enable_nls=yes])
 AC_MSG_RESULT([$man_enable_nls])
 if test x$man_enable_nls = xyes
 then
   AC_MSG_CHECKING([whether NLS support is available])
   man_enable_nls=$man_nls_headers
   test x$man_nls_funcs = xno && man_enable_nls=no
   AC_MSG_RESULT([$man_enable_nls])
 fi[]dnl
])

# MAN_NLS_LOCALE_DIRECTORY( PREFERRED, [FALLBACK] )
# -------------------------------------------------
# Identify a directory in which to install NLS message catalogues;
# it uses PREFERRED, if available, otherwise the first available from
# the colon separated list specified in the NLSPATH environment variable,
# followed by the supplementary colon separated FALLBACK list.
#
# The PREFERRED directory is named in `configure.ac', but may be overriden
# by the user, through the `--with-localedir=DIR' option; when this option
# is specified, the user is assumed to know best, and his specified DIR is
# adopted, without checking for its existence.
#
# If this search for an existing candidate directory is unsuccessful,
# and no directory is named with the `--with-localedir=DIR' option,
# then the directory named as PREFERRED is assumed anyway.
#
AC_DEFUN([MAN_NLS_LOCALE_DIRECTORY],
[AC_REQUIRE([MAN_NLS_PREREQUISITES])dnl
 AC_ARG_WITH([localedir],
   AS_HELP_STRING([--with-localedir=DIR], [message catalogue DIR [[$1]]]),
   MAN_NLS_LOCALE_CACHE_OVERRIDE([$withval]),
   MAN_NLS_LOCALE_CACHE_INIT([$1], [$2]))
 AC_SUBST([locale], [$man_cv_locale])dnl
])

# MAN_NLS_LOCALE_CACHE_INIT( PREFERRED, [FALLBACK] )
# --------------------------------------------------
# Internal macro called by MAN_NLS_LOCALE_DIRECTORY,
# to initialise the `config.cache' entry for the locale directory,
# based on the PREFERRED and FALLBACK specifications, which are
# passed down, with identical meaning, from the caller.
#
# (Note that this macro attempts to handle the common Win32 scenario
#  where directories named in $NLSPATH may have spaces embedded in their
#  names, and the path separator character is `;' rather than `:';
#  however, it cannot handle mixed POSIX/Win32 semantics).
#
AC_DEFUN([MAN_NLS_LOCALE_CACHE_INIT],
[if test x$man_enable_nls = xyes
 then
   AC_CACHE_CHECK([where to install message catalogues], [man_cv_locale],
   [[ac_dir="$1$PATH_SEPARATOR$NLSPATH"
     m4_ifvaln([$2],
       [ac_dir=$ac_dir$PATH_SEPARATOR`echo $2 | sed s",:,$PATH_SEPARATOR,"g`])dnl
     ac_dir="$ac_dir$PATH_SEPARATOR$1"
     for locale in `echo $ac_dir | sed -e s', ,%20,'g -e s",$PATH_SEPARATOR, ,"g`
     do
       test -d `echo $locale | sed -e s'?/%[NL].*??' -e s'?%20? ?'g` && break
     done
     locale=`echo $locale | sed s'?%20? ?'g`
     MSYS_AC_CANONICAL_PATH([man_cv_locale], [$locale])dnl
   ]])
 fi[]dnl
])

# MAN_NLS_LOCALE_CACHE_OVERRIDE( DIR )
# ------------------------------------
# Internal macro called by MAN_NLS_LOCALE_DIRECTORY,
# when configuring with the `--with-localedir=DIR' option specified;
# it bypasses the normal NLSPATH search, and overrides any previously
# cached setting, to impose the user's choice of directory where
# message catalogues are to be installed.
#
AC_DEFUN([MAN_NLS_LOCALE_CACHE_OVERRIDE],
[AC_MSG_CHECKING([where to install message catalogues])
 MSYS_AC_CANONICAL_PATH([man_cv_locale], [$1])
 AC_MSG_RESULT([$man_cv_locale])dnl
])

# MAN_LANGUAGES_AVAILABLE
# -----------------------
# Inspect the `manpage' source repository,
# to identify which national language `manpage' sets are available.
#
AC_DEFUN([MAN_LANGUAGES_AVAILABLE],
[m4_esyscmd([cd man; echo ?? | tr " " ,])dnl
])

# MAN_NLS_LANGUAGE_SELECTION( AVAILABLE )
# ---------------------------------------
# ENABLE/DISABLE National Language Support,
# and select the language pack(s) to install, from those AVAILABLE.
#
AC_DEFUN([MAN_NLS_LANGUAGE_SELECTION],
[AC_REQUIRE([WIN32_AC_NULLDEV])dnl
 AC_REQUIRE([MAN_NLS_PREREQUISITES])dnl
 AC_MSG_CHECKING([which national language manpages are required])
 AC_ARG_WITH([languages],
   MAN_LANGUAGE_HELP_STRING(MAN_LANGUAGE_LIST($1)),
   [test x$withval = xall && languages=MAN_LANGUAGE_LIST($1) || languages=$withval],
   [languages=en])
 test x$languages = xnone && man_enable_nls=no languages=en
 if test x$man_enable_nls = xno
 then
   AC_DEFINE([NONLS], [1], [Define to 1 if you DON'T want National Language Support.])
 fi
 AC_MSG_RESULT([$languages])
 for lang in `IFS=,; echo $languages`
 do
   langname=`cat $srcdir/man/$lang.txt 2>$NULLDEV`
   test x$langname = x && langname=$lang || langname="$lang ($langname)"
   AC_MSG_CHECKING([whether $langname manpages are available])
   ac_val=`exec 2>$NULLDEV; cd $srcdir/man/$lang && echo *.man || echo '*.man'`
   if test "$ac_val" = "*.man"
   then
     ac_val=no
   else
     test x$man_languages != x && man_languages="$man_languages,"
     man_languages=${man_languages}${lang}
     ac_val=yes
   fi
   AC_MSG_RESULT([$ac_val])
 done
 AC_MSG_CHECKING([which national language manpages to install])
 test x$man_languages = x && man_languages=en
 AC_SUBST([languages], [`IFS="$IFS,"; eval echo $man_languages`])
 AC_MSG_RESULT([$man_languages])dnl
])

# MAN_LANGUAGE_LIST( LANGUAGES,... )
# ----------------------------------
# A wrapper macro called by MAN_NLS_LANGUAGE_SELECTION,
# to remove the trailing newline character from the LANGUAGES list
# which is generated by MAN_LANGUAGES_AVAILABLE.
#
AC_DEFUN([MAN_LANGUAGE_LIST], [m4_normalize([$*])])

# MAN_LANGUAGE_HELP_STRING( LANGUAGES,... )
# -----------------------------------------
# A helper macro called by MAN_NLS_LANGUAGE_SELECTION,
# to emit the list of available LANGUAGES in `configure --help' output.
#
AC_DEFUN([MAN_LANGUAGE_HELP_STRING],
[AS_HELP_STRING([--with-languages=LIST],
[LIST of language packs to install] MAN_AS_HELP_DEFAULT(SQ(en) only)[,])
MAN_AS_HELP_APPEND([where LIST is SQ(all), any comma separated subset of SQ($*),])
MAN_AS_HELP_APPEND([or SQ(none), to disable NLS and fall back to])[ SQ(en) only]dnl
])

# MAN_AS_HELP_DEFAULT( TEXT,... )
# -------------------------------
# Emit TEXT, enclosed in brackets, for use in an AS_HELP_STRING.
#
m4_define([MAN_AS_HELP_DEFAULT], [@<:@$*@:>@])

# MAN_AS_HELP_APPEND( DESCRIPTION )
# ---------------------------------
# Append an extra line of DESCRIPTION to an AS_HELP_STRING.
#
m4_define([MAN_AS_HELP_APPEND], [AS_HELP_STRING([], [$*])])

# SQ( TEXT )
# ----------
# Emit TEXT, enclosed in single quotation marks.
#
m4_define([SQ], [`$*'])dnl`

## ================================================ ##
## Specify Filter Programs for Formatting man Pages ##
## ================================================ ##
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
  AS_HELP_STRING([--with-$2=COMMAND],
   [invoke $1 using COMMAND]) MAN_AS_HELP_DEFAULT([$3]),
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


## ======================================================== ##
## Specify Filters for Handling of Compressed man/cat Pages ##
## ======================================================== ##
#
# MAN_COMPRESS_WITH( PREFERENCE, COMMAND, EXT )
# ---------------------------------------------
# Invoke the PREFERENCE macro, which should be MAN_FILTER_PREFERRED
# on first reference, and MAN_FILTER_ALTERNATE on any subsequent reference,
# to specify COMMAND as the COMPRESS filter, if available, and specify
# EXT as the corresponding file name extension to identify
# a compressed cat file.
#
AC_DEFUN([MAN_COMPRESS_WITH],
[$1([COMPRESS], [$2])
 test -n "$compress" && compress_ext=${compress_ext-".$3"}
 AC_SUBST([compress_ext])dnl
])

# MAN_SET_DEFAULT_DECOMPRESSION_FILTER
# ------------------------------------
# Set the DO_COMPRESS symbol, to reflect availability of compression,
# and identify the decompression filter for the chosen compression format.
#
AC_DEFUN([MAN_SET_DEFAULT_DECOMPRESSION_FILTER],
[AC_MSG_CHECKING([whether compressed cat pages can be supported])
 if test -n "$compress_ext"
 then
   AC_MSG_RESULT([yes])
   AC_DEFINE([DO_COMPRESS], [1], [Define to 1 if compressed cat pages are required.])
   AC_MSG_CHECKING([default decompression filter])
   case $compress_ext in
     [.gz)  decompress=${gunzip-"none"}  ;;]
     [.bz2) decompress=${bzip2-"none"}   ;;]
     [.z)   decompress=${pcat-"none"}    ;;]
     [.Z)   decompress=${zcat-"none"}    ;;]
     [.F)   decompress=${fcat-"none"}    ;;]
     [.Y)   decompress=${unyabba-"none"} ;;]
   esac
   AC_MSG_RESULT([${decompress-"none"}])
 else
   AC_MSG_RESULT([no])
 fi
 test "$decompress" = none && decompress=""
 AC_SUBST([decompress])dnl
])


## ================================================ ##
## Miscellaneous Package Portability Considerations ##
## ================================================ ##
#
# MAN_AC_PROG_CC_WARN( OPTION ... )
# ---------------------------------
# For each specified OPTION, prefix -W, check if the C compiler accepts
# the resultant -WOPTION as a valid argument, and if so, add it to the
# list of warning flags specified in the CWARN substitution variable.
# 
AC_DEFUN([MAN_AC_PROG_CC_WARN], [MAN_AC_PROG_CC_OPTIONS([CWARN], [-W], [$1])])

# MAN_AC_PROG_CC_OPTIONS_INITIALISE( VARNAME, FROMVAR )
# -----------------------------------------------------
# If FROMVAR is defined, and is non-NULL, then initialise VARNAME
# to the same value.
#
AC_DEFUN([MAN_AC_PROG_CC_OPTIONS_INITIALISE],
[test "x$$2" = "x" || $1="$$2"])

# MAN_AC_PROG_CC_OPTIONS( VARNAME, CLASS, OPTION ... )
# ----------------------------------------------------
# For each specified OPTION, prefix the CLASS flag, then check if the
# C compiler will accept the resulting CLASSOPTION flag as a valid argument,
# and if so, add it to the space separated list specified in VARNAME.
#
AC_DEFUN([MAN_AC_PROG_CC_OPTIONS],
[AC_LANG_PUSH(C)
 popCFLAGS=$CFLAGS
 echo 'int main(void){return 0;}' > conftest.$ac_ext
 AC_FOREACH([OPTION], [$3],
  [AC_MSG_CHECKING([whether $CC accepts the $2[]m4_normalize(OPTION) option])
   CFLAGS="$popCFLAGS $$1 $2[]m4_normalize(OPTION)"
   if (eval $ac_compile) 2>&5; then
     AC_MSG_RESULT([yes])
     $1=${$1+"$$1 "}"$2[]m4_normalize(OPTION)"
   else
     AC_MSG_RESULT([no])
     echo 'failed program was:' >&5
     sed 's/^/| /' conftest.$ac_ext >&5
   fi
  ])dnl
 rm -f conftest*
 AC_LANG_POP([C])
 CFLAGS=$popCFLAGS
 AC_SUBST([$1])dnl
])

# WIN32_AC_NATIVE_HOST
# --------------------
# Check if we are compiling code for a Win32 native host,
# so that we can include Win32 support libraries in the build.
#
AC_DEFUN([WIN32_AC_NATIVE_HOST],
[AC_MSG_CHECKING([whether Win32 support libraries must be built])
 AC_LANG_PUSH(C)
 AC_COMPILE_IFELSE(dnl
   [[
#if defined(_WIN32) || defined(_MSC_VER)
 choke me
#endif
   ]],
   [win32_cv_native_support_required=no],
   [win32_cv_native_support_required=yes])
 AC_LANG_POP([C])
 AC_MSG_RESULT([$win32_cv_native_support_required])dnl
])

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

# WIN32_AC_NEED_LIBS( LIBNAME ... )
# ---------------------------------
# Add LIBNAME to ${WIN32LIBS}, when building for a Win32 host.
#
AC_DEFUN([WIN32_AC_NEED_LIBS],
[AC_REQUIRE([WIN32_AC_NATIVE_HOST])dnl
 test x$win32_cv_native_support_required = xyes && WIN32LIBS="$WIN32LIBS $1"
 AC_SUBST([WIN32LIBS])
])

# MAN_PATH_SEPARATOR
# ------------------
# Determine if `man --path', or `man -w', will return a MANPATH
# with individual directory paths separated by colons or semicolons,
# as it will be built according to the logic in `src/compat.h'.
#
AC_DEFUN([MAN_PATH_SEPARATOR],
[AC_REQUIRE([AC_PROG_LN_S])dnl
 AC_MSG_CHECKING([separator character in \SQ([man --path]) output])
 ${LN_S} ${srcdir}/src/compat.h conftest.h
 AC_LANG_PUSH(C)
 AC_COMPILE_IFELSE(dnl
   [AC_LANG_PROGRAM(dnl
     [[
#include "conftest.h"
     ]], [[
#if PATH_SEPARATOR_CHAR == ';'
 choke me
#endif
     ]])],
   [path_separator_char=':'],
   [path_separator_char=';'])
 AC_LANG_POP([C])
 AC_SUBST([path_separator_char])
 AC_MSG_RESULT([$path_separator_char])dnl
])

# MAN_GETOPT_LONG
# ---------------
# Check if we are building a `man' which can handle long options,
# and set the `manpathoption' for `apropos' and `whatis' accordingly.
#
AC_DEFUN([MAN_GETOPT_LONG],
[AC_CHECK_HEADERS([getopt.h])
 AC_CHECK_FUNCS([getopt_long],
   [manpathoption="--path"], [manpathoption="-w"])
 AC_MSG_CHECKING([how to get the active MANPATH])
 AC_SUBST([manpathoption])
 AC_MSG_RESULT([man $manpathoption])dnl
])

# MAN_PROG_AWK
# ------------
# Identify the AWK interpreter to be used by `makewhatis',
# allowing the user to specify his choice, using `--with-awk=PROG',
# since the automatic choice may not suit when cross-compiling.
#
# In the event of the user specifying `--without-awk', then
# `makewhatis' will be configured with a generic default of `awk',
# so that it may find a generic `awk' in the PATH, when run.
#
AC_DEFUN([MAN_PROG_AWK],
[AC_ARG_WITH([awk],
  [AS_HELP_STRING([--with-awk=PROG],
    [use PROG as the AWK interpreter] MAN_AS_HELP_DEFAULT([auto detect]))],
  [ac_cv_path_awk=$withval])
 test x$ac_cv_path_awk = xno && ac_cv_path_awk=awk
 AC_PATH_PROGS([awk], [gawk nawk mawk awk])dnl
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
#  explicitly configures with the `--enable-sgr' option).
#
AC_DEFUN([MAN_NROFF_SGR_CHECK],
[man_sgr_check=indeterminate
 test x$cross_compiling = xno && man_sgr_check=yes
 test x$1 = xno || man_sgr_check=no[]dnl
])

# MAN_DISABLE_COL_WITH_GROFF
# --------------------------
# When "groff" is used to provide the TROFF filter,
# post-filtering with "col" may damage the formatted output,
# so inhibit the use of "col" in this configuration.
#
# (This macro is also responsible for establishing the proper
#  relationship between the "col" and "pcol" substitution variables).
#
AC_DEFUN([MAN_DISABLE_COL_WITH_GROFF],
[AC_SUBST([pcol], ["$col"])
 AC_MSG_CHECKING([whether the COL filter should be deployed])
 case $troff in
   [*groff*) pcol="" col="__undef__($col)" ac_val="no (using groff)" ;;]
   [*)       ac_val=yes ;;]
 esac
 AC_MSG_RESULT([$ac_val])dnl
])

# EOF -- vim: ft=config
