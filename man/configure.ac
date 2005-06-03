# -*- Autoconf -*-  configure.ac for man.

AC_PREREQ(2.59)
AC_INIT(man,1.5m3)

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
 
AC_CONFIG_SRCDIR([src/man.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_CHECK_HEADERS([string.h strings.h], break)
AC_CHECK_HEADERS([fcntl.h sys/file.h sys/ioctl.h])
AC_CHECK_HEADERS([memory.h stdlib.h sys/param.h unistd.h])
AC_CHECK_HEADERS([langinfo.h locale.h nl_types.h])
AC_HEADER_SYS_WAIT

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([strchr strdup strrchr strstr])
AC_CHECK_FUNCS([bzero getcwd memset putenv setenv])
AC_CHECK_FUNCS([nl_langinfo setlocale])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile src/paths.h src/man.conf])
AC_CONFIG_FILES([man/Makefile gencat/Makefile msgs/Makefile])
AC_CONFIG_FILES([man2html/Makefile])
AC_OUTPUT
#
# EOF: vim: ft=autoconf
