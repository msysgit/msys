#-*- mode: Fundamental; tab-width: 4; -*-
# ex:ts=4
# $Id: gar.mk,v 1.1 2003-05-12 13:08:25 jrfonseca Exp $

# Copyright (C) 2001 Nick Moffitt
# 
# Redistribution and/or use, with or without modification, is
# permitted.  This software is without warranty of any kind.  The
# author(s) shall not be liable in the event that use of the
# software causes damage.


# Comment this out to make much verbosity
#.SILENT:


#ifeq ($(origin GARDIR), undefined)
#GARDIR := $(CURDIR)/../..
#endif

GARDIR ?= ../..
FILEDIR ?= files
DOWNLOADDIR ?= download
PARTIALDIR ?= $(DOWNLOADDIR)/partial
COOKIEDIR ?= cookies
WORKDIR ?= work
WORKSRC ?= $(WORKDIR)/$(DISTNAME)
EXTRACTDIR ?= $(WORKDIR)
SCRATCHDIR ?= tmp
CHECKSUM_FILE ?= checksums
MANIFEST_FILE ?= manifest

DIRSTODOTS = $(subst . /,./,$(patsubst %,/..,$(subst /, ,/$(1))))
ROOTFROMDEST = $(call DIRSTODOTS,$(DESTDIR))

ifdef BUILD_CLEAN
DO_BUILD_CLEAN = buildclean
else
DO_BUILD_CLEAN =
endif

PARALLELMFLAGS ?= $(MFLAGS)
export PARALLELMFLAGS

DISTNAME ?= $(GARNAME)-$(GARVERSION)


ALLFILES ?= $(DISTFILES) $(PATCHFILES)

INSTALL_DIRS = 

# For rules that do nothing, display what dependencies they
# successfully completed
#DONADA = @echo "	[$@] complete.  Finished rules: $+"
DONADA = @touch $(COOKIEDIR)/$@; echo "	[$@] complete for $(GARNAME)."

# TODO: write a stub rule to print out the name of a rule when it
# *does* do something, and handle indentation intelligently.

# Default sequence for "all" is:  fetch checksum extract patch configure build
all: build
	$(DONADA)


# include the configuration file to override any of these variables
include $(GARDIR)/gar.conf.mk
include $(GARDIR)/gar.lib.mk
include $(GARDIR)/gar.bugs.mk

#################### DIRECTORY MAKERS ####################

# This is to make dirs as needed by the base rules
$(sort $(DOWNLOADDIR) $(PARTIALDIR) $(COOKIEDIR) $(WORKSRC) $(WORKDIR) $(EXTRACTDIR) $(FILEDIR) $(SCRATCHDIR) $(INSTALL_DIRS) $(GARCHIVEDIR)) $(COOKIEDIR)/%:
	@if test -d $@; then : ; else \
		install -d $@; \
		echo "install -d $@"; \
	fi

# These stubs are wildcarded, so that the port maintainer can
# define something like "pre-configure" and it won't conflict,
# while the configure target can call "pre-configure" safely even
# if the port maintainer hasn't defined it.
# 
# in addition to the pre-<target> rules, the maintainer may wish
# to set a "pre-everything" rule, which runs before the first
# actual target.
pre-%:
	@true

post-%:
	@true

# Call any arbitrary rule recursively
deep-%: %
	@for i in $(LIBDEPS) $(DEPENDS) $(BUILDDEPS); do \
		$(MAKE) -C ../../$$i $@; \
	done

# ========================= MAIN RULES ========================= 
# The main rules are the ones that the user can specify as a
# target on the "make" command-line.  Currently, they are:
#	fetch-list fetch checksum makesum extract checkpatch patch
#	build install reinstall uninstall package
# (some may not be complete yet).
#
# Each of these rules has dependencies that run in the following
# order:
# 	- run the previous main rule in the chain (e.g., install
# 	  depends on build)
#	- run the pre- rule for the target (e.g., configure would
#	  then run pre-configure)
#	- generate a set of files to depend on.  These are typically
#	  cookie files in $(COOKIEDIR), but in the case of fetch are
#	  actual downloaded files in $(DOWNLOADDIR)
# 	- run the post- rule for the target
# 
# The main rules also run the $(DONADA) code, which prints out
# what just happened when all the dependencies are finished.

announce:
	@echo "[===== NOW BUILDING:	$(DISTNAME)	=====]"

# fetch-list	- Show list of files that would be retrieved by fetch.
# NOTE: DOES NOT RUN pre-everything!
fetch-list:
	@echo "Distribution files: "
	@for i in $(DISTFILES); do echo "	$$i"; done
	@echo "Patch files: "
	@for i in $(PATCHFILES); do echo "	$$i"; done

# showdeps		- Show dependencies in a tree-structure
showdeps:
	@for i in $(LIBDEPS) $(BUILDDEPS); do \
		echo -e "$(TABLEVEL)$$i";\
		$(MAKE) -s -C $(GARDIR)/$$i TABLEVEL="$(TABLEVEL)\t" showdeps;\
	done

# fetch			- Retrieves $(DISTFILES) (and $(PATCHFILES) if defined)
#				  into $(DOWNLOADDIR) as necessary.
FETCH_TARGETS =  $(addprefix $(DOWNLOADDIR)/,$(ALLFILES))

fetch: announce pre-everything $(DOWNLOADDIR) $(PARTIALDIR) $(addprefix dep-$(GARDIR)/,$(FETCHDEPS)) pre-fetch $(FETCH_TARGETS) post-fetch 
	$(DONADA)

# returns true if fetch has completed successfully, false
# otherwise
fetch-p:
	@$(foreach COOKIEFILE,$(FETCH_TARGETS), test -e $(COOKIEFILE) ;)

# checksum		- Use $(CHECKSUMFILE) to ensure that your
# 				  distfiles are valid.
CHECKSUM_TARGETS = $(addprefix checksum-,$(filter-out $(NOCHECKSUM),$(ALLFILES)))

checksum: fetch $(COOKIEDIR) pre-checksum $(CHECKSUM_TARGETS) post-checksum
	$(DONADA)

# returns true if checksum has completed successfully, false
# otherwise
checksum-p:
	@$(foreach COOKIEFILE,$(CHECKSUM_TARGETS), test -e $(COOKIEDIR)/$(COOKIEFILE) ;)

# makesum		- Generate distinfo (only do this for your own ports!).
MAKESUM_TARGETS =  $(addprefix $(DOWNLOADDIR)/,$(filter-out $(NOCHECKSUM),$(ALLFILES))) 

makesum: fetch $(MAKESUM_TARGETS)
	@if test "x$(MAKESUM_TARGETS)" != "x "; then \
		md5sum $(MAKESUM_TARGETS) > $(CHECKSUM_FILE) ; \
		echo "Checksums made for $(MAKESUM_TARGETS)" ; \
		cat $(CHECKSUM_FILE) ; \
	fi

# I am always typing this by mistake
makesums: makesum

GARCHIVE_TARGETS =  $(addprefix $(GARCHIVEDIR)/,$(ALLFILES))

garchive: checksum $(GARCHIVE_TARGETS) 
	$(DONADA)

# extract		- Unpacks $(DISTFILES) into $(EXTRACTDIR) (patches are "zcatted" into the patch program)
EXTRACT_TARGETS = $(addprefix extract-,$(filter-out $(NOEXTRACT),$(DISTFILES)))

extract: checksum $(EXTRACTDIR) $(COOKIEDIR) $(addprefix dep-$(GARDIR)/,$(EXTRACTDEPS)) pre-extract $(EXTRACT_TARGETS) post-extract
	$(DONADA)

# returns true if extract has completed successfully, false
# otherwise
extract-p:
	@$(foreach COOKIEFILE,$(EXTRACT_TARGETS), test -e $(COOKIEDIR)/$(COOKIEFILE) ;)

# checkpatch	- Do a "patch -C" instead of a "patch".  Note
# 				  that it may give incorrect results if multiple
# 				  patches deal with the same file.
# TODO: actually write it!
checkpatch: extract
	@echo "$@ NOT IMPLEMENTED YET"

# patch			- Apply any provided patches to the source.
PATCH_TARGETS = $(addprefix patch-,$(PATCHFILES))

patch: extract $(WORKSRC) pre-patch $(PATCH_TARGETS) post-patch
	$(DONADA)

# returns true if patch has completed successfully, false
# otherwise
patch-p:
	@$(foreach COOKIEFILE,$(PATCH_TARGETS), test -e $(COOKIEDIR)/$(COOKIEFILE) ;)

# makepatch		- Grab the upstream source and diff against $(WORKSRC).  Since
# 				  diff returns 1 if there are differences, we remove the patch
# 				  file on "success".  Goofy diff.
makepatch: $(SCRATCHDIR) $(FILEDIR) $(FILEDIR)/gar-base.diff
	$(DONADA)

# this takes the changes you've made to a working directory,
# distills them to a patch, updates the checksum file, and tries
# out the build (assuming you've listed the gar-base.diff in your
# PATCHFILES).  This is way undocumented.  -NickM
beaujolais: makepatch makesum clean build
	$(DONADA)

# configure		- Runs either GNU configure, one or more local
# 				  configure scripts or nothing, depending on
# 				  what's available.
CONFIGURE_TARGETS = $(addprefix configure-,$(CONFIGURE_SCRIPTS))
LIBDEPS += $(DEPENDS)

configure: patch $(addprefix builddep-$(GARDIR)/,$(BUILDDEPS)) $(addprefix dep-$(GARDIR)/,$(LIBDEPS)) $(addprefix srcdep-$(GARDIR)/,$(SOURCEDEPS)) pre-configure $(CONFIGURE_TARGETS) post-configure
	$(DONADA)

# returns true if configure has completed successfully, false
# otherwise
configure-p:
	@$(foreach COOKIEFILE,$(CONFIGURE_TARGETS), test -e $(COOKIEDIR)/$(COOKIEFILE) ;)

# build			- Actually compile the sources.
BUILD_TARGETS = $(addprefix build-,$(BUILD_SCRIPTS))

build: configure pre-build $(BUILD_TARGETS) post-build
	$(DONADA)

# returns true if build has completed successfully, false
# otherwise
build-p:
	@$(foreach COOKIEFILE,$(BUILD_TARGETS), test -e $(COOKIEDIR)/$(COOKIEFILE) ;)

# strip			- Strip binaries
strip: build pre-strip $(addprefix strip-,$(STRIP_SCRIPTS)) post-strip
	@echo "$@ NOT IMPLEMENTED YET"

# install		- Install the results of a build.
INSTALL_TARGETS = $(addprefix install-,$(INSTALL_SCRIPTS))

install: build $(addprefix dep-$(GARDIR)/,$(INSTALLDEPS)) $(INSTALL_DIRS) pre-install $(INSTALL_TARGETS) post-install $(DO_BUILD_CLEAN)
	$(DONADA)

# returns true if install has completed successfully, false
# otherwise
install-p:
	@$(foreach COOKIEFILE,$(INSTALL_TARGETS), test -e $(COOKIEDIR)/$(COOKIEFILE) ;)

# installstrip		- Install the results of a build, stripping first.
installstrip: strip pre-install $(INSTALL_TARGETS) post-install
	$(DONADA)

# reinstall		- Install the results of a build, ignoring
# 				  "already installed" flag.
reinstall: build
	rm -rf $(COOKIEDIR)/*install*
	$(MAKE) install

# uninstall		- Remove the installation.
# TODO: actually write it!
uninstall: build
	@echo "$@ NOT IMPLEMENTED YET"
	

# package		- Create a package from an _installed_ port.
# TODO: actually write it!
package: build
	@echo "$@ NOT IMPLEMENTED YET"

# tarball		- Make a tarball from an install of the package into a scratch dir
tarball: build
	rm -rf $(COOKIEDIR)/install*
	$(MAKE) DESTDIR=$(CURDIR)/$(SCRATCHDIR) BUILD_PREFIX=$(call DIRSTODOTS,$(CURDIR)/$(SCRATCHDIR))/$(BUILD_PREFIX) install
	find $(SCRATCHDIR) -depth -type d | while read i; do rmdir $$i > /dev/null 2>&1 || true; done
	tar czvf $(CURDIR)/$(WORKDIR)/$(DISTNAME)-install.tar.gz -C $(SCRATCHDIR) .
	$(MAKECOOKIE)


# The clean rule.  It must be run if you want to re-download a
# file after a successful checksum (or just remove the checksum
# cookie, but that would be lame and unportable).
clean:
	@rm -rf $(DOWNLOADDIR) $(COOKIEDIR) $(COOKIEDIR)-* $(WORKSRC) $(WORKDIR) $(EXTRACTDIR) $(SCRATCHDIR) $(SCRATCHDIR)-$(COOKIEDIR) $(SCRATCHDIR)-build *~

buildclean:
	@rm -rf $(WORKSRC) $(WORKDIR) $(EXTRACTDIR) $(SCRATCHDIR) $(SCRATCHDIR)-$(COOKIEDIR) $(SCRATCHDIR)-build *~

# calls clean, but then also cleans up dependencies!
deep-clean: clean
	@for i in $(LIBDEPS) $(DEPENDS) $(BUILDDEPS); do \
		$(MAKE) -C ../../$$i deep-clean; \
	done

# these targets do not have actual corresponding files
.PHONY: all fetch-list beaujolais fetch-p checksum-p extract-p patch-p configure-p build-p install-p

# apparently this makes all previous rules non-parallelizable,
# but the actual builds of the packages will be, according to
# jdub.
.NOTPARALLEL:
