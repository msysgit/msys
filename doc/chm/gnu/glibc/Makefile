GARNAME = glibc
GARVERSION = 2.3.2
MASTER_SITES = ftp://ftp.gnu.org/pub/gnu/$(GARNAME)/
DISTFILES = $(GARNAME)-$(GARVERSION).tar.gz $(GARNAME)-linuxthreads-$(GARVERSION).tar.gz
#DISTFILES = $(GARNAME)-$(GARVERSION).tar.bz2 $(GARNAME)-linuxthreads-$(GARVERSION).tar.bz2

CONFIGURE_SCRIPTS = \
	$(WORKSRC)/configure
BUILD_SCRIPTS = \
	$(WORKSRC)/manual/Makefile
	
BOOKS = $(WORKSRC)/manual/libc.texinfo

include ../../gar.mk

MAKEINFO_FLAGS += --force

extract-$(GARNAME)-linuxthreads-%: EXTRACTDIR = $(WORKDIR)/$(GARNAME)-$(GARVERSION)
configure-$(WORKSRC)/configure: CONFIGURE_ARGS = --enable-add-ons=linuxthreads --disable-sanity-checks
build-$(WORKSRC)/manual/Makefile: BUILD_ARGS = info
