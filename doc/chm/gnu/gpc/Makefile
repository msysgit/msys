GARNAME = gpc
GARVERSION = 2.1
MASTER_SITES = http://www.gnu-pascal.de/current/
DISTFILES = $(GARNAME)-$(GARVERSION).tar.gz

WORKSRC = $(WORKDIR)/$(GARNAME)-20020510/p

BOOKS = $(WORKSRC)/doc/en/gpc.texi

FIGURES = $(WORKSRC)/doc/images/*

include ../../gar.mk

MAKEINFO_FLAGS += -I .. -I ../generated
