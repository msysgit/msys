bin_PROGRAMS = dos2unix

dos2unix_SOURCES = dos2unix.c dos2unix.h

noinst_man_MANS = dos2unix.1

docdir = $(prefix)/doc/dos2unix

doc_DATA = COPYING dos2unix.html

dos2unix.html: dos2unix.1
	man2html $< > $@
