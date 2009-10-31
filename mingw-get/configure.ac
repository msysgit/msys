# configure.ac -*- autoconf -*- vim: filetype=config
#
# $Id: configure.ac,v 1.1 2009-10-31 21:08:41 keithmarshall Exp $
#
# Written by Keith Marshall <keithmarshall@users.sourceforge.net>
# Copyright (C) 2009, MinGW Project
#
#
# Configuration script for mingw-get
#
#
# This is free software.  Permission is granted to copy, modify and
# redistribute this software, under the provisions of the GNU General
# Public License, Version 3, (or, at your option, any later version),
# as published by the Free Software Foundation; see the file COPYING
# for licensing details.
#
# Note, in particular, that this software is provided "as is", in the
# hope that it may prove useful, but WITHOUT WARRANTY OF ANY KIND; not
# even an implied WARRANTY OF MERCHANTABILITY, nor of FITNESS FOR ANY
# PARTICULAR PURPOSE.  Under no circumstances will the author, or the
# MinGW Project, accept liability for any damages, however caused,
# arising from the use of this software.
#
  AC_INIT([mingw-get],[0.0],[http://mingw.org/reporting_bugs])

# We need both C and C++ compilers; check how to invoke them
#
  AC_PROG_CC
  AC_PROG_CXX

# We also need a lexical analyser generator
#
  AC_PROG_LEX

# Set up the archive librarian, to match our compiler settings
#
  AC_CHECK_TOOL([AR],[ar],[ar])
  AC_SUBST([ARFLAGS],[${ARFLAGS-rcs}])

# Create a makefile
#
  AC_CONFIG_FILES([Makefile])
  AC_OUTPUT
#
# $RCSfile: configure.ac,v $: end of file
