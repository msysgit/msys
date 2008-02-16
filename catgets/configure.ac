# configure.ac  -*- Autoconf -*-
# Process this file with autoconf, to generate a configure script.
#
# $Id: configure.ac,v 1.7 2008-02-16 12:57:19 keithmarshall Exp $
#
# Copyright (C) 2006, 2007, 2008, MinGW Project
# Written by Keith Marshall <keithmarshall@users.sourceforge.net>
#
# Package identification.
#
# This is configure.ac for the MinGW `catgets' package.
# BASENAME, VERSION_MAJOR and VERSION_MINOR are required tags;
# complete `Value' fields as appropriate.
#
#                            Tag              Value
#                            ---------------  ---------
  MINGW_AC_DEFINE_PACKAGE_ID([BASENAME],      [catgets])
  MINGW_AC_DEFINE_PACKAGE_ID([VERSION_MAJOR], [1])
  MINGW_AC_DEFINE_PACKAGE_ID([VERSION_MINOR], [0])
#
# PATCHLEVEL is optional; comment/uncomment and adjust as required.
#
  MINGW_AC_DEFINE_PACKAGE_ID([PATCHLEVEL],    [1])
#
# DLL_VERSION is required; installed DLLs will be versioned, by
# appending a hyphen, the specified tag value, and then the `.dll'
# file name extension, to the base name of each generated DLL.
#
  MINGW_AC_DEFINE_PACKAGE_ID([DLL_VERSION],   [1])
#
#
# This is free software.  It is provided AS IS, in the hope that it may
# be useful, but WITHOUT WARRANTY OF ANY KIND, not even an IMPLIED WARRANTY
# of MERCHANTABILITY, nor of FITNESS FOR ANY PARTICULAR PURPOSE.
#
# Permission is granted to redistribute this software, either "as is" or
# in modified form, under the terms of the GNU General Public License, as
# published by the Free Software Foundation; either version 2, or (at your
# option) any later version.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to the
# Free Software Foundation, 51 Franklin St - Fifth Floor, Boston,
# MA 02110-1301, USA.

# Autoconf initialisation.
#
  AC_PREREQ([2.59])
  AC_INIT(__MINGW_AC_PACKAGE_IDENTIFICATION__)

# Initialise for possible building as a cross-hosted tool kit.
#
  CATGETS_AC_CONFIG_TARGET

# Set up to use `config.h', with included package version tags.
#
  AC_CONFIG_HEADERS([config.h])
  CATGETS_AC_CONFIG_VERSION

# Compiler and build tool checks.
#
  AC_PROG_CC
  AC_PROG_LN_S
  AC_CHECK_TOOL([AR], [ar], [ar])
  AC_PATH_PROG([AWK_INTERPRETER], [awk], [/bin/awk])
  AC_PROG_INSTALL

# Check if Win32 DLLs should be built,
# and set the release version for any which are.
#
  MINGW_AC_HOST_CONFIG_DLL([__MINGW_AC_PACKAGE_DLL_VERSION__])

# We need to identify the canonical path prefix, and the appropriate
# PATH separator character, to be used in the default NLSPATH definition.
#
  MINGW_AC_HOST_CANONICAL_PREFIX
  MINGW_AC_HOST_PATH_SEPARATOR([NLSPATH])

# Schedule an automatic header update, if we find a `unistd.h',
# with the missing `SEEK_SET' definition bug.
#
  MINGW_AC_BROKEN_UNISTD_H([REPLACEMENT_HEADERS])

# To build a cross-hosted i586-mingw32-gencat, we need a local
# substitute for MinGW's `_mingw.h'; note that this is simply a
# minimal stub -- we never install it.
#
  MINGW_AC_CHECK_HEADER([LOCAL_HEADERS], [_mingw.h])

# Unless we have a *very* recent MinGW installation, it is likely
# to be missing some headers we need; we provide copies of these,
# *and* install them if required.
#
  MINGW_AC_CHECK_HEADER([REPLACEMENT_HEADERS], [libgen.h])
  MINGW_AC_CHECK_HEADER([REPLACEMENT_HEADERS], [nl_types.h])
  MINGW_AC_CHECK_HEADER([REPLACEMENT_HEADERS], [langinfo.h])

# Building for a native Win32 host, we need the `windows.h' header,
# (which is not required for a cross-hosted tool chain build), and
# also the MinGW locale extensions.
#
  AC_CHECK_HEADERS([windows.h])
  MINGW_AC_LC_EXTENSIONS([LOCAL_HEADERS])

# Object modules required to build the `gencat' executable.
# We need `mkstemp', `nl_langinfo', `getopt_long_only' and `basename';
# if these are already available in system libraries, just use the
# provided implementations, otherwise we must supply them.
#
  GENCAT_AC_FUNC_MKSTEMP
  GENCAT_AC_FUNC_NL_LANGINFO
  GENCAT_AC_FUNC_GETOPT_LONG_ONLY
  GENCAT_AC_FUNC_BASENAME
#
# The remainder are always provided by this package.
#
  GENCAT_AC_OBJECTS_ADD([mcload])
  GENCAT_AC_OBJECTS_ADD([mciconv])
  GENCAT_AC_OBJECTS_ADD([mcsource])
  GENCAT_AC_OBJECTS_ADD([mcmerge])
  GENCAT_AC_OBJECTS_ADD([mcutfsig])

# We also require `iconv', always expecting it to be pre-installed; we
# need this to check if we are using a standard `libc' implementation,
# or if we need to adapt to use GNU libiconv.
#
  LIBICONV_AC_FUNC_ICONV

  AC_CONFIG_FILES([Makefile])
  AC_OUTPUT
#
# $RCSfile: configure.ac,v $Revision: 1.7 $: end of file
