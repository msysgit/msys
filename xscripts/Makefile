# Makefile for x86-mingw32-build.sh script package
# $Id: Makefile,v 1.2 2009-03-15 09:36:45 keithmarshall Exp $
#
# Automates updates of generated files from the x86-mingw32-build CVS
# (xscripts), and creates distribution tarballs.
#
# Copyright (C) 2006, 2009, MinGW Project
# Written by Keith Marshall <keithmarshall@users.sourceforge.net>
# 
# This is the primary script for the x86-mingw32-build package.
#
# x86-mingw32-build is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version.
# 
# x86-mingw32-build is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for further details.
# 
# You should have received a copy of the GNU General Public License along
# with x86-mingw32-build; see the file COPYING.  If not, write to the Free
# Software Foundation, 51 Franklin St - Fifth Floor, Boston, MA 02110-1301,
# USA.

PACKAGE_NAME = x86-mingw32-build
PACKAGE_SCRIPTS = $(PACKAGE_NAME).sh*
SHOW_VERSION = $(SHOW_VERSION_CMD) --echo $(VERSION_FLAGS)
SHOW_VERSION_CMD = $(SHELL) ${srcdir}/x86-mingw32-build.sh.version
UPDATE_VERSION = sed s",<version>,`$(SHOW_VERSION)`,"
NROFF = nroff -Tascii

GENERATED_FILES = README AUTHORS LICENSE
DISTFILES = ${srcdir}/COPYING $(GENERATED_FILES) ${srcdir}/$(PACKAGE_SCRIPTS)
DEVELOPER_DISTFILES = ${srcdir}/*.in ${srcdir}/*.tmac ${srcdir}/Makefile

all: $(GENERATED_FILES)

# GNU `make' predefines `RM', but some others may not.

RM = rm -f

# We could use `ln -s', but not all hosts support it.
# If we used `autoconf', it could determine if it is safe, or not,
# but the package requirements are so modest, that `autoconf' seems
# like overkill; for now we'll just force it's fallback case, but
# in a fashion which will facilitate future `autoconfiscation'.

LN_S = cp -p

# Using `srcdir' also helps to keep us `autoconf ready';
# for now we'll just accommodate `in-place' builds, with no `VPATH'.

srcdir = .

%: %.in
	$(NROFF) $(NROFF_FLAGS) $< > $@

README: README.in x86-mingw32-build.sh.version
	$(UPDATE_VERSION) ${srcdir}/README.in | $(NROFF) $(NROFF_FLAGS) > $@

clean:
	$(RM) $(GENERATED_FILES) *~

distclean: clean
	$(RM) -r $(PACKAGE_NAME)-* tmp
	$(RM) *.gz *.bz2 *.log

# We create distributions comprising two tarballs;
# the `basic-dist' target encapsulates the scripts needed by end users;
# the `devel-dist' target provides the extra files needed by developers.
#
# By default, we create these tarballs with bzip2 compression, but we
# allow the user to override this, by specifying "ZIPMODE=gz", so that
# gzip compression is used instead.

TARBALL = $(PACKAGE_NAME)-`$(SHOW_VERSION) SNAPSHOT=$(SNAPSHOT)`
ZIPMODE = .bz2

%.tar.gz: %.tar; gzip -c $< > $@
%.tar.bz2: %.tar; bzip2 -c $< > $@

%-dev.tar: $(DEVELOPER_DISTFILES)
	$(RM) -r $*; mkdir $*
	$(LN_S) $(DEVELOPER_DISTFILES) $*
	tar chf $@ $*
	$(RM) -r $*

%-sh.tar: $(DISTFILES)
	$(RM) README; $(MAKE) README
	$(RM) -r $(PACKAGE_NAME)-* $* tmp; mkdir tmp
	$(LN_S) $(DISTFILES) tmp; mv tmp $*
	cd $*; $(RM) $(PACKAGE_NAME).log
	tar chf $@ $*
	$(RM) -r $*

dist: basic-dist devel-dist
basic-dist: ; $(MAKE) $(TARBALL)-sh.tar$(ZIPMODE)
devel-dist: ; $(MAKE) $(TARBALL)-dev.tar$(ZIPMODE)

# One day, we might have this package install some scripts,
# but for now, there is nothing to install.

install: install-scripts
install-scripts:

# $RCSfile: Makefile,v $Revision: 1.2 $: end of file
