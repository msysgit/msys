# This file contains configuration variables that are global to
# the GAR system.  Users wishing to make a change on a
# per-package basis should edit the category/package/Makefile, or
# specify environment variables on the make command-line.

# Setting this variable will cause the results of your builds to
# be cleaned out after being installed.  Uncomment only if you
# desire this behavior!

# export BUILD_CLEAN = true

DESTDIR ?= $(GARDIR)/../books

GARCHIVEDIR = $(GARDIR)/../garchive/$(DISTNAME)/

# prepend the local file listing
FILE_SITES = file://$(FILEDIR)/ file://$(GARCHIVEDIR)

# Extra libs to include with gar.mk
#EXTRA_LIBS = devhelp.lib.mk
EXTRA_LIBS = htmlhelp.lib.mk
