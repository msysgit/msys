## Process this file with automake to produce Makefile.in -*-Makefile-*-

AUTOMAKE_OPTIONS = ../src/ansi2knr

noinst_LIBRARIES = libtu.a

INCLUDES = -I.. -I$(srcdir) -I../intl

libtu_a_SOURCES = argmatch.c closeout.c diacrit.c full-write.c getopt.c \
  getopt1.c hard-locale.c human.c \
  linebuffer.c long-options.c md5.c memcasecmp.c memcoll.c \
  obstack.c quotearg.c readtokens.c safe-read.c \
  version-etc.c xmalloc.c xstrdup.c xstrtod.c xstrtol.c xstrtoul.c xstrtoumax.c

libtu_a_LIBADD = @LIBOBJS@ @ALLOCA@
libtu_a_DEPENDENCIES = $(libtu_a_LIBADD)

noinst_HEADERS = argmatch.h bumpalloc.h closeout.h diacrit.h error.h \
  fnmatch.h \
  getline.h getopt.h hard-locale.h human.h \
  lchown.h linebuffer.h long-options.h md5.h \
  memcasecmp.h memcoll.h obstack.h quotearg.h \
  readtokens.h regex.h safe-read.h \
  version-etc.h xalloc.h xstrtod.h xstrtol.h

BUILT_SOURCES = lstat.c stat.c

EXTRA_DIST = xstat.in
lstat.c: xstat.in
	sed -e '/@IGNORE@/d' -e 's/@xstat@/lstat/g' $(srcdir)/xstat.in > $@-t
	mv $@-t $@

stat.c: xstat.in
	sed -e '/@IGNORE@/d' -e 's/@xstat@/stat/g' $(srcdir)/xstat.in > $@-t
	mv $@-t $@
