## Process this file with automake to produce Makefile.in -*-Makefile-*-
AUTOMAKE_OPTIONS = gnits 1.2h

SUBDIRS = lib intl src doc man m4 po tests djgpp

# Remove this file here (it is created via configure), not from within intl.
DISTCLEANFILES = intl/libintl.h

EXTRA_DIST = GNUmakefile Makefile.maint

ACLOCAL_AMFLAGS = -I m4

distcheck-hook:
	$(MAKE) my-distcheck
