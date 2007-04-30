# aclocal.m4 -*- Autoconf -*- vim: filetype=config
# File included by autoconf, when generating a configure script.
#
# $Id: aclocal.m4,v 1.1 2007-04-30 22:46:36 keithmarshall Exp $
#
# Copyright (C) 2007, MinGW Project
# Written by Keith Marshall <keithmarshall@users.sourceforge.net>
#
#
# This is free software.  It is provided AS IS, in the hope that it may
# be useful, but WITHOUT WARRANTY OF ANY KIND, not even an IMPLIED WARRANTY
# of MERCHANTABILITY, nor of FITNESS FOR ANY PARTICULAR PURPOSE.
#
# Permission is granted to redistribute this software, either "as is" or
# in modified form, under the terms of the GNU Lesser General Public License,
# as published by the Free Software Foundation; either version 2.1, or (at
# your option) any later version.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this software; see the file COPYING.LIB.  If not, write to the
# Free Software Foundation, 51 Franklin St - Fifth Floor, Boston,
# MA 02110-1301, USA.

m4_include([pkgid.m4])

# MINGW_AC_PROG_CC_OPTIONS( VARNAME, CLASS, OPTION ... )
# ------------------------------------------------------
# For each specified OPTION, prefix the CLASS flag, then check if the
# C compiler will accept the resulting CLASSOPTION flag as a valid argument,
# and if so, add it to the space separated list specified in VARNAME.
#
AC_DEFUN([MINGW_AC_PROG_CC_OPTIONS],
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
])# MINGW_AC_PROG_CC_OPTIONS

# MINGW_AC_DISTRIBUTION_TYPE( PREF )
# ----------------------------------
# Set the preferred packaging method to PREF, (either `tar' or `zip'),
# by setting the ZIPCMD and ZIPEXT substitution variables appropriately.
#
AC_DEFUN([MINGW_AC_DISTRIBUTION_TYPE],
[AC_SUBST([ZIPCMD], ['tar czhf'])
 AC_SUBST([ZIPEXT], ['.tar.gz'])
 test "x$1" = xzip && ZIPCMD='zip -r' ZIPEXT='.zip'
 AC_ARG_ENABLE([dist],
  AS_HELP_STRING([--enable-dist=TYPE],
  [package distribution as TYPE tar or zip [[TYPE=$1]]]),
  [case $enableval in
    tar) ;; zip) ZIPCMD='zip -r' ZIPEXT='.zip' ;;
    *) AC_MSG_WARN([$enableval:unsupported distribution format])
       AC_MSG_WARN([reverting to $1 format]) ;;
   esac
  ])dnl
])# MINGW_AC_DISTRIBUTION_TYPE

# MINGW_AC_DEV_INSTALL_OPTION
# ---------------------------
# Allow the user to optionally disable the installation of the
# development libraries, when invoking `make install'
#
AC_DEFUN([MINGW_AC_DEV_INSTALL_OPTION],
[AC_SUBST([install_dev], [install-dev])
 AC_ARG_ENABLE([dev-install],
  AS_HELP_STRING([--disable-dev-install],
  [omit development libraries with `make install']),
  [test "x$enableval" = xno && install_dev=""])dnl
])# MINGW_AC_DEV_INSTALL_OPTION
# $RCSfile: aclocal.m4,v $Revision: 1.1 $: end of file
