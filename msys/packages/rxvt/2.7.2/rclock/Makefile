# Generated automatically from Makefile.in by configure.
# rclock/Makefile.in	-*- Makefile -*-

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

basedir = ..
thisdir = rclock

first_rule: clock
dummy:

SRCS =	rclock.c
OBJS =	rclock.o
HDRS =	feature.h

#
# Distribution variables
#

DIST = $(HDRS) $(SRCS) Makefile.in rclock.1.in

# inference rules
.c.o:
	$(CC) -c $(CPPFLAGS) $(XINC) -I. -I$(basedir) -I$(srcdir) $(DEFS) $(CFLAGS) $<
#-------------------------------------------------------------------------
all:
clock: alldoc allbin

rclock: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(XLIB)

rclock.1: $(srcdir)/rclock.1.in
	cat $(srcdir)/rclock.1.in\
	|sed -e 's%@RXVT_VERSION@%$(VERSION)%g;'\
	|sed -e 's%@RXVT_DATE@%$(DATE)%g;'\
	|sed -e 's%@RXVT_MAINT@%$(MAINT)%g;'\
	> $@

rclock.html: rclock.1
	rman -f HTML -S -r off rclock.1\
	|sed -e 's%&lt\;file:]*\(.*\)&gt\;%<A HREF="\1">\1</A>%g;'\
	|sed -e 's%&lt\;\([fht]*tp://\)\([^/]*\)\(.*\)&gt\;%<A HREF="\1\2\3">\2</A>%g;'\
	|sed -e 's%&lt\;\([^ /]*\)&gt\;%<A HREF="mailto:\1">\1</A>%g;'\
	> $@

tags: $(SRCS)
	ctags $(SRCS)

allbin: rclock

alldoc:	rclock.1

clean:
	$(RMF) rclock rclock.1 core a.out *.o *.bak *~

realclean: clean
	$(RMF) tags

distclean:
	(cd $(srcdir); $(RMF) rclock rclock.1 *~ *.o core a.out)
	if test $(srcdir) = .; then $(MAKE) realclean; fi
	(cd $(srcdir); $(RMF) Makefile)

install: allbin alldoc
	$(INSTALL_PROGRAM) rclock $(DESTDIR)$(bindir)/$(binprefix)rclock
	$(INSTALL_DATA) rclock.1 $(DESTDIR)$(mandir)/$(manprefix)rclock.$(manext)

uninstall:
	cd $(bindir); $(RMF) $(binprefix)rclock
	cd $(mandir); $(RMF) $(manprefix)rclock.$(manext)

distdirs:
	mkdir $(basedir)/../$(VERNAME)/$(thisdir)

distcopy:  rclock.1
	$(CP) -p $(DIST) $(basedir)/../$(VERNAME)/$(thisdir)
	if test -f rclock.html; then \
	$(CP) -p rclock.html $(basedir)/../$(VERNAME)/$(thisdir); \
	fi

# -----------------------------------------------------------------------
# DO NOT DELETE: ugly dependency list follows
rclock.o: rclock.c feature.h
