# Main Makefile for GNU tar.
# Copyright 1994, 1995, 1996, 1997, 1999, 2000 Free Software Foundation, Inc.

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.

## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

AUTOMAKE_OPTIONS = gnits dist-shar
EXTRA_DIST = ChangeLog.1 PORTS
SUBDIRS = doc lib intl m4 src scripts po tests
DISTCLEANFILES = intl/libintl.h

# This is a bit of a hack; there must be a better way.
ACLOCAL_AMFLAGS =
M4DIR = $(srcdir)/m4
ACINCLUDE_INPUTS = $(M4DIR)/c-bs-a.m4 $(M4DIR)/ccstdc.m4 $(M4DIR)/d-ino.m4 \
  $(M4DIR)/decl.m4 $(M4DIR)/error.m4 \
  $(M4DIR)/fnmatch.m4 $(M4DIR)/getline.m4 $(M4DIR)/gettext.m4 \
  $(M4DIR)/inttypes_h.m4 $(M4DIR)/isc-posix.m4 $(M4DIR)/jm-mktime.m4 \
  $(M4DIR)/largefile.m4 $(M4DIR)/lcmessage.m4 $(M4DIR)/malloc.m4 \
  $(M4DIR)/mbstate_t.m4 $(M4DIR)/mktime.m4 \
  $(M4DIR)/prereq.m4 $(M4DIR)/progtest.m4 $(M4DIR)/realloc.m4 \
  $(M4DIR)/strerror_r.m4 \
  $(M4DIR)/uintmax_t.m4 $(M4DIR)/ulonglong.m4 $(M4DIR)/utimbuf.m4 \
  $(M4DIR)/utime.m4 $(M4DIR)/utimes.m4 $(M4DIR)/xstrtoumax.m4

$(srcdir)/acinclude.m4 : $(ACINCLUDE_INPUTS)
	cat $(ACINCLUDE_INPUTS) >$(srcdir)/acinclude.m4
