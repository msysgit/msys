bin_PROGRAMS = unix2dos

unix2dos_SOURCES = unix2dos.c unix2dos.h

noinst_man_MANS = unix2dos.1

docdir = $(prefix)/doc/unix2dos

doc_DATA = COPYING unix2dos.html

unix2dos.html: unix2dos.1
	man2html $< > $@
