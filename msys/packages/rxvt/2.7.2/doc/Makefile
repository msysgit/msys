# Generated automatically from Makefile.in by configure.
# doc/Makefile.in	-*- Makefile -*-
# $Id: Makefile,v 1.1 2002/12/06 23:08:01 earnie Exp $

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
thisdir = doc

first_rule: all
dummy:

#
# Distribution variables
#

YODL_RXVT = yodl/rxvt-colours.yo yodl/rxvt-environment.yo\
	    yodl/rxvt-files.yo yodl/rxvt-fonts.yo yodl/rxvt-login.yo\
	    yodl/rxvt-mousereporting.yo yodl/rxvt-options.yo\
	    yodl/rxvt-resources.yo yodl/rxvt-scrollbar.yo\
	    yodl/rxvt-textselection.yo
YODL_RREF = yodl/rxvtRef-csi.yo yodl/rxvtRef-definitions.yo\
	    yodl/rxvtRef-graphics.yo yodl/rxvtRef-keycodes.yo\
	    yodl/rxvtRef-menubar.yo yodl/rxvtRef-mouse.yo\
	    yodl/rxvtRef-privatemodes.yo yodl/rxvtRef-sequences.yo\
	    yodl/rxvtRef-values.yo yodl/rxvtRef-xpm.yo yodl/rxvtRef-xterm.yo

DIST_ETC  = etc/rxvt.termcap etc/rxvt.terminfo etc/XTerm.ad
DIST_YODL = yodl/masonyodl.yo yodl/versioninfo.yo yodl/versioninfo.yo.in\
	    $(YODL_RXVT) yodl/rxvt.yo $(YODL_RREF) yodl/rxvtRef.yo
DIST_MENU = menu/menu menu/example.menu menu/rxvt.menu menu/terminal.menu\
	    menu/jedmenu.sl
DIST	  = Makefile.in README.greek README.menu README.xvt BUGS FAQ TODO\
	    changes.txt xterm.seq LSM.in rxvt.1 rxvt.tbl rxvtRef.txt\
	    rxvtRef.html rxvtRef-frame.html rxvtRef-toc.html rxvt.html 

#-------------------------------------------------------------------------

all:	rxvt.1

rxvt.1: rxvt.tbl Makefile
	@if test x$(TBL) = x; then : ; else echo "Making rxvt.1"; $(TBL) $(srcdir)/rxvt.tbl > rxvt.1 ; fi

SEDREPLACE = -e 's%@RXVT_VERSION@%$(VERSION)%g;'\
	     -e 's%@RXVT_LSMDATE@%$(LSMDATE)%g;'\
	     -e 's%@RXVT_DATE@%$(DATE)%g;'\
	     -e 's%@RXVT_MAINTEMAIL@%$(MAINTEMAIL)%g;'\
	     -e 's%@RXVT_MAINT@%$(MAINT)%g;'\
	     -e 's%@RXVT_WEBPAGE@%$(WEBPAGE)%g;'\
	     -e 's%@RXVT_WEBMAINTEMAIL@%$(WEBMAINTEMAIL)%g;'\
	     -e 's%@RXVT_WEBMAINT@%$(WEBMAINT)%g;'\
	     -e 's%@RXVT_FTPSITENAME@%$(FTPSITENAME)%g;'\
	     -e 's%@RXVT_FTPSITEDIR@%$(FTPSITEDIR)%g;'

tags allbin:

alldoc:	$(basedir)/$(VERNAME).lsm rxvt.1 rxvt.html rxvtRef.html rxvtRef.txt

yodl/versioninfo.yo: yodl/versioninfo.yo.in ../src/version.h
	$(SED) $(SEDREPLACE) < $(srcdir)/yodl/versioninfo.yo.in > $@

$(basedir)/$(VERNAME).lsm: LSM.in $(srcdir)/../autoconf/Make.common.in $(srcdir)/../src/version.h
	$(SED) $(SEDREPLACE) < $(srcdir)/LSM.in > $@

rxvt.tbl: yodl/rxvt.yo $(YODL_RXVT) yodl/versioninfo.yo yodl/masonyodl.yo
	(cd $(srcdir)/yodl; yodl2man -o../rxvt.tbl rxvt.yo)

rxvt.html: yodl/rxvt.yo $(YODL_RXVT) yodl/versioninfo.yo yodl/masonyodl.yo
	(cd $(srcdir)/yodl; yodl2html -o../rxvt.html rxvt.yo)

rxvtRef.html: $(YODL_RREF) yodl/rxvtRef.yo yodl/masonyodl.yo
	(cd $(srcdir)/yodl; yodl2html -o../rxvtRef.html rxvtRef.yo)

rxvtRef.txt: $(YODL_RREF) yodl/rxvtRef.yo yodl/masonyodl.yo
	(cd $(srcdir)/yodl; yodl2txt -o../rxvtRef.txt rxvtRef.yo)

clean:
	$(RMF) rxvt.man *~ yodl/*~

realclean: clean
	$(RMF) tags rxvt.1 rxvt.html rxvtRef.html rxvtRef.txt

distclean:

install:
	$(INSTALL_DATA) rxvt.1 $(DESTDIR)$(mandir)/$(manprefix)rxvt.$(manext)

uninstall:
	-cd $(mandir); $(RMF) $(manprefix)rxvt.$(manext)

distdirs:
	mkdir $(basedir)/../$(VERNAME)/$(thisdir)
	mkdir $(basedir)/../$(VERNAME)/$(thisdir)/etc
	mkdir $(basedir)/../$(VERNAME)/$(thisdir)/menu
	mkdir $(basedir)/../$(VERNAME)/$(thisdir)/yodl

distcopy: distdepend
	$(CP) -p $(basedir)/$(VERNAME).lsm $(basedir)/../$(VERNAME);
	$(CP) -p $(DIST)      $(basedir)/../$(VERNAME)/$(thisdir)
	$(CP) -p $(DIST_ETC)  $(basedir)/../$(VERNAME)/$(thisdir)/etc
	$(CP) -p $(DIST_MENU) $(basedir)/../$(VERNAME)/$(thisdir)/menu
	$(CP) -p $(DIST_YODL) $(basedir)/../$(VERNAME)/$(thisdir)/yodl

distdepend: alldoc

# ------------------------------------------------------------------------
# DO NOT DELETE: ugly dependency list follows
