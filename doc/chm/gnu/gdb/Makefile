GARNAME = gdb
GARVERSION = 5.3
MASTER_SITES = \
	ftp://gcc.gnu.org/pub/$(GARNAME)/releases/ \
	ftp://sources.redhat.com/pub/$(GARNAME)/releases/$(GARNAME)-$(GARVERSION)/
DISTFILES = $(GARNAME)-$(GARVERSION).tar.bz2

CONFIGURE_SCRIPTS = $(WORKSRC)/gdb/configure
BUILD_SCRIPTS = $(WORKSRC)/gdb/doc/Makefile
	
BOOKS = \
	$(WORKSRC)/gdb/doc/gdb.texinfo \
	$(WORKSRC)/gdb/doc/gdbint.texinfo

NOINST_BOOKS = \
	$(WORKSRC)/gdb/doc/stabs.texinfo

include ../../gar.mk

MAKEINFO_FLAGS += -I ../mi -I ../../readline/doc

build-$(WORKSRC)/gdb/doc/Makefile: BUILD_ARGS = clean gdb.info
