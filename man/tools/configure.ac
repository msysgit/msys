# -*- Autoconf -*-  tools/configure.ac for man.
#
  AC_INIT
  AC_PREREQ([2.59])
#
# Copyright (C) 2005 by Keith Marshall <keithmarshall@users.sourceforge.net>
#
# This file is part of the `man' package; process it with `autoconf' to
# create a `configure' script.
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
# with man; see the file COPYING, in the top source directory.  If not,
# write to the Free Software Foundation, 51 Franklin St - Fifth Floor,
# Boston, MA 02110-1301, USA.
#
#
# The `configure' script, generated from this template, provides a mechanism
# for configuring the `tools' directory for building native code tools, which
# may be required in a cross-compiling environment.  While it may be invoked
# `stand-alone', it is normally invoked indirectly, by the `configure' script
# at the top level;  in this case, the following settings are passed down
# from the top level configure:--
#
#    top level BUILD_CC is propagated to `tools' CC
#    top level BUILD_CFLAGS is propagated to `tools' CFLAGS
#    top level BUILD_LDFLAGS is propagated to `tools' LDFLAGS
#    top level BUILD_CPPFLAGS is propagated to `tools' CPPFLAGS
#
  AC_CONFIG_SRCDIR([makemsg.c])
#
# Identify the C compiler...
# Since the top level `configure' doesn't propagate its `build_alias' or
# `host_alias' settings, this should be a native code compiler, even when the
# top level `configure' specifies cross-compiling.
#
  AC_PROG_CC
#
# Check how to make this C compiler support the `const' keyword...
#
  AC_C_CONST
#
# Check for optionally required headers and functions...
#
  AC_HEADER_STDC
  AC_CHECK_HEADERS([stdlib.h string.h unistd.h])
  AC_CHECK_FUNCS([strchr strrchr])
#
# Create `Makefile'...
#
  AC_CONFIG_FILES([Makefile])
  AC_OUTPUT
#
# EOF
