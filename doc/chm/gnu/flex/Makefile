GARNAME = flex
GARVERSION = 2.5.4a
MASTER_SITES = ftp://ftp.gnu.org/pub/non-gnu/$(GARNAME)/
DISTFILES = $(GARNAME)-$(GARVERSION).tar.gz

BOOKS = $(WORKSRC)/MISC/texinfo/flex.texi

WORKSRC = work/flex-2.5.4

include ../../gar.mk

$(WORKDIR)/flex.texi: $(WORKSRC)/MISC/texinfo/flex.texi
	sed -e "s/@section/@chapter/g" $< > $@
