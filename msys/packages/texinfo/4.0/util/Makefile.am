## Makefile.am for texinfo/util.
## $Id: Makefile.am,v 1.1 2002-05-13 22:15:59 earnie Exp $
## Run automake in .. to produce Makefile.in from this.

bin_PROGRAMS = install-info texindex
bin_SCRIPTS = texi2dvi

localedir = $(datadir)/locale
INCLUDES = -I$(top_srcdir)/lib -I../intl -DLOCALEDIR=\"$(localedir)\"
LDADD = ../lib/libtxi.a @INTLLIBS@

# Most of these are just for fun.  The only one that's really supported
# is texi2dvi.
EXTRA_DIST = README deref.c fix-info-dir fixfonts fixref.gawk gen-dir-node \
  outline.gawk prepinfo.awk tex3patch texi-docstring-magic.el texi2dvi
