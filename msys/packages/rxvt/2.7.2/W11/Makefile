# Generated automatically from Makefile.in by configure.


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

first_rule: all

all allbin alldoc:
	cd xpm-3.4k/lib; make -f Makefile.noX all install
	cd wrap; make $@
	cd w32; make $@

tags clean:
	cd xpm-3.4k/lib; make -f Makefile.noX $@
	cd wrap; make $@
	cd w32; make $@

install:
	$(INSTALL_PROGRAM) lib/libW11.dll $(bindir)/libW11.dll
	$(INSTALL) ../CYGWIN-PATCHES/rxvt-2.7.2.README $(prefix)/doc/Cygwin
