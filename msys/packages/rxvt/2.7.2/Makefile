# Generated automatically from Makefile.in by configure.
# ./Makefile.in			-*- Makefile -*-
# $Id: Makefile,v 1.2 2004-03-15 15:03:08 earnie Exp $

# autoconf/Make.common.in	 -*- Makefile -*-
# release date (man), LSM date, version number/name, current maintainer
DATE=21 DECEMBER 1999
LSMDATE=21DEC99
VERSION=2.7.2
VERNAME=rxvt-$(VERSION)#
MAINT=Geoff Wing#
MAINTEMAIL=<gcw@rxvt.org>#
WEBMAINT=Oezguer Kesim#
WEBMAINTEMAIL=<oec@rxvt.org>#
WEBPAGE=<http://www.rxvt.org/>#
FTPSITENAME=ftp.rxvt.org#
FTPSITEDIR=/pub/rxvt#
#-------------------------------------------------------------------------

SHELL = /bin/sh

# This variable makes it possible to move the installation root to another
# directory. This is useful when you're creating a binary distribution
# If empty, normal root will be used.
# You can run eg. 'make install DESTDIR=/packages/rxvt-xx' to accomplish
# that.
# DESTDIR = /usr/local/X11/$(VERNAME)

# Installation target directories & other installation stuff
prefix = /usr
exec_prefix = /usr
binprefix =
manprefix =
bindir = ${exec_prefix}/bin
mandir = ${prefix}/man/man1
manext = 1

# Tools & program stuff
CC = gcc
CPP = gcc -E
MV = /usr/bin/mv
RM = /usr/bin/rm
RMF = /usr/bin/rm -f
CP = /usr/bin/cp
SED = /usr/bin/sed
AWK = /usr/bin/awk
ECHO = /usr/bin/echo
CMP = /usr/bin/cmp
TBL = tbl
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = /usr/bin/install -c -s -m 755
INSTALL_DATA = /usr/bin/install -c -m 644

# Flags & libs
# add -DBINDIR=\""$(bindir)/"\" to CPPFLAGS, if we need to spawn a program

CFLAGS = -O3 -s 
CPPFLAGS =  
LDFLAGS = -O
DEFS = -DHAVE_CONFIG_H
LIBS = 
DINCLUDE = 
DLIB = 

# X Include directory
XINC =  

# extra libraries needed by X on some systems, X library location
XLIB =   -lX11 

# End of common section of the Makefile
#-------------------------------------------------------------------------

srcdir =	.
.PATH:		.

first_rule: all
dummy:

subdirs = W11 src doc src/graphics src/test

DIST =	INSTALL README.configure configure Makefile Makefile.in ChangeLog

DIST_CFG = autoconf/aclocal.m4 autoconf/configure.in \
	autoconf/config.h.in  autoconf/acconfig.h  autoconf/Make.common.in \
	autoconf/install-sh autoconf/mkinstalldirs

MKDIR = $(srcdir)/autoconf/mkinstalldirs

#-------------------------------------------------------------------------

all allbin alldoc tags clean:
	@for I in ${subdirs}; do (cd $$I; ${MAKE} $@ || exit 1); done

#
# entry points for other programs
#
rxvt:
	(cd src; ${MAKE})

clock:
	(cd rclock; ${MAKE})

graphics qplot:
	(cd src/graphics; ${MAKE} qplot)

#-------------------------------------------------------------------------
configure: autoconf/configure.in autoconf/aclocal.m4 autoconf/acconfig.h autoconf/config.h.in
	cd $(srcdir);
	autoconf --localdir=$(srcdir)/autoconf \
	autoconf/configure.in > configure
	chmod 755 configure

config.status:
	if test -x config.status; then config.status --recheck; \
	else $(SHELL) configure; fi

autoconf/config.h.in: autoconf/configure.in autoconf/acconfig.h
	cd $(srcdir);
	autoheader --localdir=$(srcdir)/autoconf \
	autoconf/configure.in > autoconf/config.h.in
	chmod 644 autoconf/config.h.in

installdirs:
	$(MKDIR) $(DESTDIR)$(bindir)
	$(MKDIR) $(DESTDIR)$(mandir)

install: installdirs
	@for I in $(subdirs); do (cd $$I; $(MAKE) DESTDIR=$(DESTDIR) $@ || exit 1); done

Makefiles:
	$(SHELL) config.status

realclean:
	$(RMF) *~ config.cache
	@for I in ${subdirs}; do (cd $$I; ${MAKE} $@ || exit 1); done

# distclean goal is for making a clean source tree, but if you have run
# configure from a different directory, then doesn't destroy all your
# hardly compiled and linked stuff. That's why there is always $(srcdir)/
# In that case most of those commands do nothing, except cleaning *~
# and cleaning source links.
distclean:
	(cd $(srcdir); $(RMF) *~ config.cache config.h config.log config.status)
	@for I in $(subdirs); do (cd $$I; $(MAKE) $@ || exit 1); done
	(cd $(srcdir); $(RMF) Makefile autoconf/Make.common)

distdirs:
	mkdir ../$(VERNAME);
	mkdir ../$(VERNAME)/autoconf;
	@for I in $(subdirs); do (cd $$I; $(MAKE) $@ || (echo "Failed to make distclean in $$I"; exit 0) ); done

distcopy:
	$(CP) -p $(DIST) ../$(VERNAME);
	$(CP) -p $(DIST_CFG) ../$(VERNAME)/autoconf;
	@for I in $(subdirs); do (cd $$I; $(MAKE) $@ || exit 1); done

distrib: configure autoconf/config.h.in distdirs distcopy

tar.gz:  ../$(VERNAME).tar.gz
../$(VERNAME).tar.gz:
	(cd ..; tar cvf - $(VERNAME) | gzip -f9 > $(VERNAME).tar.gz)

tar.Z: ../$(VERNAME).tar.Z
../$(VERNAME).tar.Z:
	(cd ..; tar cvf - $(VERNAME) | compress > $(VERNAME).tar.Z)

tar.bz2: ../$(VERNAME).tar.bz2
../$(VERNAME).tar.bz2:
	(cd ..; tar cvf - $(VERNAME) | bzip2 -f9 > $(VERNAME).tar.bz2)

uuencode: tar.gz
	uuencode ../$(VERNAME).tar.gz $(VERNAME).tar.gz > ../$(VERNAME).tgz.uu

# ------------------------------------------------------------------------
