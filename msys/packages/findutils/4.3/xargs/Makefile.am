AUTOMAKE_OPTIONS = std-options
localedir = $(datadir)/locale
bin_PROGRAMS = xargs
man_MANS = xargs.1
INCLUDES = -I.. -I../gnulib/lib -I$(top_srcdir)/gnulib/lib -I$(top_srcdir)/lib -I../intl -DLOCALEDIR=\"$(localedir)\"
LDADD = ../find/version.o ../lib/libfind.a ../gnulib/lib/libgnulib.a @INTLLIBS@
EXTRA_DIST = $(man_MANS)
SUBDIRS = testsuite

$(PROGRAMS): ../find/version.o ../lib/libfind.a ../gnulib/lib/libgnulib.a

