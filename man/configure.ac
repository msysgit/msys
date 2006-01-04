# -*- Autoconf -*-  configure.ac for man.

  AC_PREREQ(2.59)
  AC_INIT(man, 1.5m3)

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

# Configure a native build subsystem, for native code tools
# which will be required when cross-compiling.
#
  MAN_AC_CONFIG_NATIVE([tools])

# Establish whether man should be installed suid, sgid, or neither,
# and set the install program flags appropriately.
#
# Note: MAN_SUID_ENABLE and MAN_SGID_ENABLE define the DEFAULT user
# and group names to be assigned, if `--enable-suid' or `--enable-sgid'
# are specified without any specific name argument;  they do NOT cause
# these options to be activated, unless the user specifies them.
#
  MAN_SUID_ENABLE([man])
  MAN_SGID_ENABLE([man])
  MAN_INSTALL_FLAGS

# Checks for programs.
  MAN_PROG_AWK
  AC_PROG_CC
  AC_PROG_INSTALL
  AC_PROG_LN_S
  AC_PROG_MAKE_SET

# Set warning options for C compiler.
  MAN_AC_PROG_CC_OPTIONS([CWARNNP], [-W], [all])
  MAN_AC_PROG_CC_OPTIONS_INITIALISE([CWARN], [CWARNNP])
  MAN_AC_PROG_CC_OPTIONS([CWARN], [-W], [strict-prototypes missing-prototypes])

# Checks for libraries.
  WIN32_AC_NEED_LIBS([win32/libexec.a])

# Checks for header files.
  AC_HEADER_STDC
  AC_FUNC_ALLOCA
  AC_HEADER_DIRENT
  AC_CHECK_HEADERS([string.h strings.h], break)
  AC_CHECK_HEADERS([fcntl.h sys/file.h sys/ioctl.h])
  AC_CHECK_HEADERS([memory.h stdlib.h sys/param.h unistd.h])
  AC_CHECK_HEADERS([process.h])
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
  MAN_GETOPT_LONG

# Establish the name and location for the configuration file.
#
# (The defaults are as defined here; if you wish to change these,
#  uncomment the next two macro lines, and modify them accordingly.
#  Note the use of single quotes to defer expansion of ${prefix}
#  until "make" time).
#
# MAN_CONFIG_FILE_DEFAULT([man.conf])
# MAN_CONFDIR_DEFAULT(['${prefix}/lib'])
#
# The default for MAN_CONFDIR may need to be modified,
# if conformance with FHS or FSSTND standards is requested.
#
  MAN_FHS_ENABLE
  MAN_FSSTND_ENABLE
#
# Subject to established defaults,
# identify the appropriate configuration file name and location.
#
  MAN_CONFIG_FILE

# Specify which manual sections will be supported,
# and the order in which they will be searched.
#
  MANSECT_SEARCH_ORDER([1:1p:8:2:3:3p:4:5:6:7:9:0p:tcl:n:l:p:o])

# For manpages installed by the man package itself,
# define the MANSECT specific filename extensions to be used.
#
  MANSECT_FILENAME_EXT([1], [man1ext], [1])
  MANSECT_FILENAME_EXT([5], [man5ext], [5])
  MANSECT_FILENAME_EXT([8], [man8ext], [8])

# Identify the character which will be used to separate individual
# directory path names, in the MANPATH reported by `man --path'.
#
  MAN_PATH_SEPARATOR

# Set up the default MANPATH entries, in the configuration file.
# (First, the standard set, which are normally included)...
#
  MANPATH_DEFAULT_INCLUDE([/usr/man])
  MANPATH_DEFAULT_INCLUDE([/usr/share/man])
  MANPATH_DEFAULT_INCLUDE([/usr/local/man])
  MANPATH_DEFAULT_INCLUDE([/usr/local/share/man])
  MANPATH_DEFAULT_INCLUDE([/usr/X11R6/man])
#
# (followed by the optional set, which are normally commented out)...
# (we use MANPATH_DEFAULT_INCLUDE, but OCCLUSION is specified by
#  a leading `#' in src/man.conf.in).
#
  MANPATH_DEFAULT_INCLUDE([/opt/*/man])
  MANPATH_DEFAULT_INCLUDE([/usr/lib/*/man])
  MANPATH_DEFAULT_INCLUDE([/usr/share/*/man])
  MANPATH_DEFAULT_INCLUDE([/usr/kerberos/man])

# Specify standard MANPATH_MAP aliases,
# for conformance with FHS or FSSTND standards, when in use.
# (This MUST be set up BEFORE the MANPATH_MAP is defined,
#  to ensure that the aliases are resolved in the map).
#
  MANPATH_MAP_ALIAS([FHS], [/usr/man],       [/usr/share/man])
  MANPATH_MAP_ALIAS([FHS], [/usr/local/man], [/usr/local/share/man])

# Now, define the default MANPATH_MAP.
#
  MANPATH_MAP_DEFAULT([/bin],                [/usr/man])
  MANPATH_MAP_DEFAULT([/sbin],               [/usr/man])
  MANPATH_MAP_DEFAULT([/usr/bin],            [/usr/man])
  MANPATH_MAP_DEFAULT([/usr/sbin],           [/usr/man])
  MANPATH_MAP_DEFAULT([/usr/local/bin],      [/usr/local/man])
  MANPATH_MAP_DEFAULT([/usr/local/sbin],     [/usr/local/man])
  MANPATH_MAP_DEFAULT([/usr/X11R6/bin],      [/usr/X11R6/man])
  MANPATH_MAP_DEFAULT([/usr/bin/X11],        [/usr/X11R6/man])
  MANPATH_MAP_DEFAULT([/usr/bin/mh],         [/usr/man])

# Check if National Language Support is available,
# specify which language packs are available for installation,
# and allow the user to select those which will be installed.
#
  MAN_NLS_LANGUAGE_SELECTION([MAN_LANGUAGES_AVAILABLE])
  MAN_NLS_LOCALE_DIRECTORY([/usr/lib/locale/%N/%L], [/usr/share/locale/%L/%N])

# Identify the PAGER, BROWSER and HTMLPAGER programs to be used.
#
  MAN_FILTER_PREFERRED([PAGER],     [less -is])
  MAN_FILTER_ALTERNATE([PAGER],     [more])
#
  MAN_FILTER_PREFERRED([BROWSER],   [lynx])
  MAN_FILTER_ALTERNATE([BROWSER],   [less -is])
  MAN_FILTER_ALTERNATE([BROWSER],   [more])
#
  MAN_FILTER_PREFERRED([HTMLPAGER], [lynx -dump])
  MAN_FILTER_ALTERNATE([HTMLPAGER], [cat])

# We need to work out how to invoke `grep' silently,
# i.e. just check for presence of an expression match, with no output,
# and without displaying any error messages.
#
  MAN_GREP_SILENT

# Select the filter programs to be used
# for TROFF formatting of man page source documents.
#
  MAN_FILTER_PREFERRED([CAT],    [cat])
#
  MAN_FILTER_PREFERRED([TROFF],  [groff -Tps -mandoc])
  MAN_FILTER_ALTERNATE([TROFF],  [troff -mandoc])
#
  MAN_FILTER_PREFERRED([NROFF],  [nroff -Tlatin1 -mandoc])
  MAN_DISABLE_NROFF_SGR
#
  MAN_FILTER_PREFERRED([JNROFF], [groff -Tnippon -mandocj])
#
  MAN_FILTER_PREFERRED([COL],    [col])
  MAN_DISABLE_COL_WITH_GROFF
#
  MAN_FILTER_PREFERRED([EQN],    [geqn -Tps])
  MAN_FILTER_ALTERNATE([EQN],    [eqn -Tps])
#
  MAN_FILTER_PREFERRED([NEQN],   [neqn -Tlatin1])
#
  MAN_FILTER_PREFERRED([JNEQN],  [neqn -Tlatin1])
#
  MAN_FILTER_PREFERRED([TBL],    [gtbl])
  MAN_FILTER_ALTERNATE([TBL],    [tbl])
#
  MAN_FILTER_PREFERRED([VGRIND], [vgrind])
#
  MAN_FILTER_PREFERRED([REFER],  [grefer])
  MAN_FILTER_ALTERNATE([REFER],  [refer])
#
  MAN_FILTER_PREFERRED([GRAP],   [grap])
#
  MAN_FILTER_PREFERRED([PIC],    [gpic])
  MAN_FILTER_ALTERNATE([PIC],    [pic])

# For `catpage' support, `cmp' is also required,
# to determine when obsolete cat pages must be updated.
#
  MAN_FILTER_PREFERRED([CMP],    [cmp -s])

# Select a default compression filter,
# which will be used when preformatted `cat pages' are saved.
#
  MAN_COMPRESS_WITH([MAN_FILTER_PREFERRED], [bzip2 -c], [bz2])
  MAN_COMPRESS_WITH([MAN_FILTER_ALTERNATE], [gzip -c],  [gz])
  MAN_COMPRESS_WITH([MAN_FILTER_ALTERNATE], [pack],     [z])
  MAN_COMPRESS_WITH([MAN_FILTER_ALTERNATE], [compress], [Z])
  MAN_COMPRESS_WITH([MAN_FILTER_ALTERNATE], [freeze],   [F])
  MAN_COMPRESS_WITH([MAN_FILTER_ALTERNATE], [yabba],    [Y])

# Also identify the decompression filters which are available,
# so that we can directly support compressed `man' or `cat' pages,
# in a variety of compression formats.
#
  MAN_FILTER_PREFERRED([GUNZIP],  [gzip -c -d])
  MAN_FILTER_PREFERRED([BZIP2],   [bzip2 -c -d])
  MAN_FILTER_PREFERRED([PCAT],    [pcat])
  MAN_FILTER_PREFERRED([ZCAT],    [zcat])
  MAN_FILTER_PREFERRED([FCAT],    [fcat])
  MAN_FILTER_PREFERRED([UNYABBA], [unyabba])

# Identify which of these available decompression filters
# to associate with the selected default compression filter.
#
  MAN_SET_DEFAULT_DECOMPRESSION_FILTER

  AC_CONFIG_FILES([Makefile])
  AC_CONFIG_FILES([src/Makefile src/win32/Makefile src/paths.h src/man.conf])
  AC_CONFIG_FILES([man/Makefile msgs/Makefile man2html/Makefile])
  AC_OUTPUT
#
# EOF: vim: ft=config
