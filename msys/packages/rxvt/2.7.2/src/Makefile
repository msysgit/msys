# Generated automatically from Makefile.in by configure.
# $Id: Makefile,v 1.2 2004-03-15 15:03:09 earnie Exp $

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
exec_prefix = ${prefix}
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
XLIB =   -L ../W11/lib -lX11 

# End of common section of the Makefile
#-------------------------------------------------------------------------

srcdir =	.
.PATH:		.

basedir = ..
thisdir = src

# for developers: the following debug options may be used
#	-DDEBUG_CMD -DDEBUG_MENU -DDEBUG_MENUARROWS -DDEBUG_MENUBAR_STACKING
#	-DDEBUG_MENU_LAYOUT -DDEBUG_RESOURCES -DDEBUG_SCREEN
#	-DDEBUG_SEARCH_PATH -DDEBUG_SELECT -DDEBUG_TTYMODE -DDEBUG_X
DEBUG=-DDEBUG_STRICT 

first_rule: all
dummy:

SRCS =	command.c graphics.c grkelot.c logging.c main.c menubar.c \
	misc.c netdisp.c screen.c scrollbar.c strings.c xdefaults.c xpm.c

HDRS =	command.h grkelot.h menubar.h protos.h rxvtgrx.h version.h \
	feature.h logging.h netdisp.h rxvt.h screen.h

# it was ``be kind to gmake week''.  Now it's ``autoconf week''
#OBJS =	$(SRCS:S,.c,.o,)
OBJS =	command.o    main.o  misc.o netdisp.o screen.o scrollbar.o  xdefaults.o 


# it's ``be kind to gmake week''
#EXTPROS = $(SRCS:S,.c,.extpro,)
EXTPROS = command.extpro graphics.extpro grkelot.extpro logging.extpro \
	main.extpro menubar.extpro misc.extpro netdisp.extpro \
	screen.extpro scrollbar.extpro strings.extpro xdefaults.extpro \
	xpm.extpro

# it's ``be kind to gmake week''
#INTPROS = $(SRCS:S,.c,.intpro,)
INTPROS = command.intpro graphics.intpro grkelot.intpro logging.intpro \
	main.intpro menubar.intpro misc.intpro netdisp.intpro \
	screen.intpro scrollbar.intpro strings.intpro xdefaults.intpro \
	xpm.intpro

DEPS =  rxvt.h ${basedir}/config.h feature.h rsizes.h

#
# Distribution variables
#

DIST = $(HDRS) $(SRCS) Makefile.in rsizes.h.in gcc-Wall .indent.pro \
	makeintprotos-awk makeextprotos-awk $(INTPROS) $(EXTPROS)

.SUFFIXES:	.c .o .intpro
.PHONY:		protos
.DONTCARE:	protos

# inference rules
.c.o:
	$(CC) $(DEFS) $(DEBUG) -c $(CPPFLAGS) $(XINC)  -I../W11 -I. -I$(basedir) -I$(srcdir) $(DINCLUDE) $(CFLAGS) $<
#-------------------------------------------------------------------------
all: protos rxvt

rxvt: version.h $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(XLIB) $(DLIB)

protos:
	@for I in $(EXTPROS); do \
	    $(RMF) tmpproto; \
	    J=`echo $$I | $(SED) 's,.extpro,.c,'`; \
	    $(AWK) -f $(srcdir)/makeextprotos-awk $(srcdir)/$$J > tmpproto; \
	    if $(CMP) -s $$I tmpproto ; then : ; else $(ECHO) "Updating $$I"; $(CP) tmpproto $$I; fi; \
	    $(RMF) tmpproto; \
	done

.c.intpro:
	@$(RMF) tmpproto-$@
	@$(AWK) -f $(srcdir)/makeintprotos-awk $< > tmpproto-$@
	@if $(CMP) -s $@ tmpproto-$@ ; then : ; else $(ECHO) "Updating $@"; $(CP) tmpproto-$@ $@; fi
	@$(RMF) tmpproto-$@

tags: $(SRCS)
	ctags $(SRCS)

allbin: rxvt

alldoc:

clean:
	$(RMF) rxvt core a.out *.o *.bak *~ *.intpro *.extpro

realclean: clean
	$(RMF) tags

distclean: realclean
	if test $(srcdir) = .; then $(MAKE) realclean; fi
	(cd $(srcdir); $(RMF) Makefile)

install: allbin alldoc
	$(INSTALL_PROGRAM) rxvt $(DESTDIR)$(bindir)/$(binprefix)rxvt

uninstall:
	(cd $(bindir); $(RMF) $(binprefix)rxvt)

distdirs:
	mkdir $(basedir)/../$(VERNAME)/$(thisdir)

distcopy: protos $(INTPROS)
	$(CP) -p $(DIST) $(basedir)/../$(VERNAME)/$(thisdir)

# Semi-automatic generation of dependencies:
# Use gcc -MM because X11 `makedepend' doesn't work on all systems
# and it also includes system headers.
# `semi'-automatic since dependencies are generated at distribution time.

#distdepend:
#	mv Makefile.in Makefile.in~
#	sed "/^# DO NOT DELETE:/,$$ d" Makefile.in~ > Makefile.in
#	echo "# DO NOT DELETE: ugly dependency list follows" >> Makefile.in
#	gcc -MM $(CPPFLAGS) $(XINC) -I. -I$(basedir) -I$(srcdir) $(SRCS) >> Makefile.in

# -----------------------------------------------------------------------
# DO NOT DELETE: nice dependency list follows
command.o:   command.c   $(DEPS) command.h version.h			\
	command.intpro graphics.extpro grkelot.extpro logging.extpro	\
	main.extpro menubar.extpro misc.extpro netdisp.extpro		\
	screen.extpro scrollbar.extpro strings.extpro xdefaults.extpro	\
	xpm.extpro
graphics.o:  graphics.c  $(DEPS)					\
	command.extpro graphics.intpro grkelot.extpro logging.extpro	\
	main.extpro menubar.extpro misc.extpro netdisp.extpro		\
	screen.extpro scrollbar.extpro strings.extpro xdefaults.extpro	\
	xpm.extpro
grkelot.o:   grkelot.c   $(DEPS) grkelot.h				\
	command.extpro graphics.extpro grkelot.intpro logging.extpro	\
	main.extpro menubar.extpro misc.extpro netdisp.extpro		\
	screen.extpro scrollbar.extpro strings.extpro xdefaults.extpro	\
	xpm.extpro
logging.o:   logging.c   $(DEPS) logging.h				\
	command.extpro graphics.extpro grkelot.extpro logging.intpro	\
	main.extpro menubar.extpro misc.extpro netdisp.extpro		\
	screen.extpro scrollbar.extpro strings.extpro xdefaults.extpro	\
	xpm.extpro
main.o:      main.c      $(DEPS) 					\
	command.extpro graphics.extpro grkelot.extpro logging.extpro	\
	main.intpro menubar.extpro misc.extpro netdisp.extpro		\
	screen.extpro scrollbar.extpro strings.extpro xdefaults.extpro	\
	xpm.extpro
menubar.o:   menubar.c   $(DEPS) menubar.h version.h			\
	command.extpro graphics.extpro grkelot.extpro logging.extpro	\
	main.extpro menubar.intpro misc.extpro netdisp.extpro		\
	screen.extpro scrollbar.extpro strings.extpro xdefaults.extpro	\
	xpm.extpro
misc.o:      misc.c      $(DEPS)					\
	command.extpro graphics.extpro grkelot.extpro logging.extpro	\
	main.extpro menubar.extpro misc.intpro netdisp.extpro		\
	screen.extpro scrollbar.extpro strings.extpro xdefaults.extpro	\
	xpm.extpro
netdisp.o:   netdisp.c   $(DEPS) netdisp.h				\
	command.extpro graphics.extpro grkelot.extpro logging.extpro	\
	main.extpro menubar.extpro misc.extpro netdisp.intpro		\
	screen.extpro scrollbar.extpro strings.extpro xdefaults.extpro	\
	xpm.extpro
screen.o:    screen.c    $(DEPS) screen.h				\
	command.extpro graphics.extpro grkelot.extpro logging.extpro	\
	main.extpro menubar.extpro misc.extpro netdisp.extpro		\
	screen.intpro scrollbar.extpro strings.extpro xdefaults.extpro	\
	xpm.extpro
scrollbar.o: scrollbar.c $(DEPS)					\
	command.extpro graphics.extpro grkelot.extpro logging.extpro	\
	main.extpro menubar.extpro misc.extpro netdisp.extpro		\
	screen.extpro scrollbar.intpro strings.extpro xdefaults.extpro	\
	xpm.extpro
strings.o:   strings.c   $(DEPS) 					\
	command.extpro graphics.extpro grkelot.extpro logging.extpro	\
	main.extpro menubar.extpro misc.extpro netdisp.extpro		\
	screen.extpro scrollbar.extpro strings.intpro xdefaults.extpro	\
	xpm.extpro
xdefaults.o: xdefaults.c $(DEPS) version.h				\
	command.extpro graphics.extpro grkelot.extpro logging.extpro	\
	main.extpro menubar.extpro misc.extpro netdisp.extpro		\
	screen.extpro scrollbar.extpro strings.extpro xdefaults.intpro	\
	xpm.extpro
xpm.o:       xpm.c       $(DEPS) 					\
	command.extpro graphics.extpro grkelot.extpro logging.extpro	\
	main.extpro menubar.extpro misc.extpro netdisp.extpro		\
	screen.extpro scrollbar.extpro strings.extpro xdefaults.extpro	\
	xpm.intpro
