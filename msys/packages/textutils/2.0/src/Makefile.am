## Process this file with automake to produce Makefile.in -*-Makefile-*-
AUTOMAKE_OPTIONS = ansi2knr

bin_PROGRAMS = cat cksum comm csplit cut expand fmt fold head join md5sum \
nl od paste pr ptx sort split sum tac tail tr tsort unexpand uniq wc

noinst_HEADERS = sys2.h system.h
EXTRA_DIST = tac-pipe.c

datadir = $(prefix)/@DATADIRNAME@
localedir = $(datadir)/locale

INCLUDES = -I.. -I$(srcdir) -I$(top_srcdir)/lib -I../intl
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
LDADD = @INTLLIBS@ ../lib/libtu.a

$(PROGRAMS): $(LDADD)

## If necessary, add -lm to resolve use of pow in lib/strtod.c.
sort_LDADD = $(LDADD) @POW_LIBM@
sort_DEPENDENCIES = $(LDADD)
