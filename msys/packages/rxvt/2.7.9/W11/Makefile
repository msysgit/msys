

# autoconf/Make.common.in	 -*- Makefile -*-
# release date (man), LSM date, version number/name, current maintainer
DATE=04 OCTOBER 2002
LSMDATE=04OCT02
VERSION=2.7.9
VERNAME=rxvt-$(VERSION)#
MAINT=Geoff Wing#
MAINTEMAIL=<gcw@rxvt.org>#
WEBMAINT=Oezguer Kesim#
WEBMAINTEMAIL=<oec@rxvt.org>#
WEBPAGE=<http://www.rxvt.org/>#
FTPSITENAME=ftp.rxvt.org#
FTPSITEDIR=/pub/rxvt#
#-------------------------------------------------------------------------
RXVTNAME=rxvt

SHELL = /bin/sh

# This variable makes it possible to move the installation root to another
# directory. This is useful when you're creating a binary distribution
# If empty, normal root will be used.
# You can run eg. 'make install DESTDIR=/packages/rxvt-xx' to accomplish
# that.
# DESTDIR = /usr/local/X11/$(VERNAME)

# Installation target directories & other installation stuff
prefix = /usr
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include
mandir = ${prefix}/man/man1
manext = 1

# Tools & program stuff
CC = gcc
CPP = gcc -E
MV = /usr/bin/mv
RM = /usr/bin/rm
RMF = /usr/bin/rm -f
CP = /usr/bin/cp
LN = /usr/bin/ln
SED = /usr/bin/sed
AWK = gawk
ECHO = /usr/bin/echo
CMP = /usr/bin/cmp
TBL = /usr/bin/tbl
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = /usr/bin/install -c -m 755
INSTALL_DATA = /usr/bin/install -c -m 644

# Flags & libs
# add -DBINDIR=\""$(bindir)/"\" to CPPFLAGS, if we need to spawn a program

CFLAGS = -g -O2 
CPPFLAGS =  -DHAVE_LIBXPM
LDFLAGS = 
DEFS = -DHAVE_CONFIG_H
LIBS = 
DINCLUDE = 
DLIB = 

# X Include directory
XINC =  -I../W11 -IW11 -I../W11

# extra libraries needed by X on some systems, X library location
XLIB =  -L../W11/lib -mwindows ../W11/wrap/rxvt_res.o -Wl,--subsystem,console -L../W11/lib -lXpm -lX11 

LIBTOOL = $(SHELL) $(top_builddir)/libtool
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(DEBUG) $(DINCLUDE) $(XINC) -I$(basedir) -I$(srcdir) -I.
LINK = $(CC) $(CFLAGS) $(LDFLAGS)

# End of common section of the Makefile
#-------------------------------------------------------------------------

srcdir =	.

.PATH:		.

top_builddir = ..
basedir = ..
thisdir = W11
XPMDIR=xpm-3.4k
MKDIR = ../autoconf/mkinstalldirs

first_rule: all

all allbin alldoc:
	-mkdir lib
	cd wrap; make $@
	cd w32; make $@
	if test -d $(XPMDIR); then make -f Makefile.xpm; fi

tags clean:
	cd wrap; make $@
	cd w32; make $@
	if test -d $(XPMDIR); then make -f Makefile.xpm $@; fi

realclean: clean

cleandir: realclean

install:
	$(INSTALL_PROGRAM) lib/libW11.dll $(bindir)/libW11.dll

#
# Distribution variables
#

DIST = Makefile.in Makefile.xpm
DISTX11 = X11/Intrinsic.h X11/Xatom.h X11/Xlib.h X11/Xlocale.h X11/Xmd.h \
	X11/Xutil.h X11/cursorfont.h X11/keysym.h X11/keysymdef.h
DISTw32 = w32/Makefile w32/event.c w32/ntdef.h w32/ntutil.c w32/xlib.c
DISTwrap = wrap/Makefile wrap/mkproxy.pl wrap/rxtv.ico wrap/rxvt.ico \
	wrap/rxvt.rc wrap/wrap.c wrap/wrap.h wrap/xwrappers.gen

distclean: realclean
	if test $(srcdir) = .; then $(MAKE) realclean; fi
	(cd $(srcdir); $(RMF) Makefile)

distdirs:
	mkdir $(basedir)/../$(VERNAME)/$(thisdir)
	mkdir $(basedir)/../$(VERNAME)/$(thisdir)/X11
	mkdir $(basedir)/../$(VERNAME)/$(thisdir)/w32
	mkdir $(basedir)/../$(VERNAME)/$(thisdir)/wrap

distcopy:
	$(CP) -p $(DIST) $(basedir)/../$(VERNAME)/$(thisdir)
	$(CP) -p $(DISTX11) $(basedir)/../$(VERNAME)/$(thisdir)/X11
	$(CP) -p $(DISTw32) $(basedir)/../$(VERNAME)/$(thisdir)/w32
	$(CP) -p $(DISTwrap) $(basedir)/../$(VERNAME)/$(thisdir)/wrap
