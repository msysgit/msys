## Process this file with automake to produce Makefile.in -*-Makefile-*-.

EXTRA_DIST = README Makefile.am.in .env-warn mk-script Fetish.pm
SUBDIRS = cut head join md5sum pr sort tac tail tr uniq wc

check-recursive: envvar-check

# Warn when `make check' is run with POSIXLY_CORRECT or CDPATH set.
envvar-check:
	test "$${POSIXLY_CORRECT+set}" = set \
	  && sed s/%%/POSIXLY_CORRECT/ $(srcdir)/.env-warn || :
	test "$${CDPATH+set}" = set \
	  && sed s/%%/CDPATH/ $(srcdir)/.env-warn || :
