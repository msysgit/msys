#-*- mode: Fundamental; tab-width: 4; -*-
# ex:ts=4
# $Id: gar.lib.mk,v 1.1 2003-05-12 13:08:25 jrfonseca Exp $

# Copyright (C) 2001 Nick Moffitt
# 
# Redistribution and/or use, with or without modification, is
# permitted.  This software is without warranty of any kind.  The
# author(s) shall not be liable in the event that use of the
# software causes damage.

# cookies go here, so we have to be able to find them for
# dependency checking.
VPATH += $(COOKIEDIR)

# So these targets are all loaded into bbc.port.mk at the end,
# and provide actions that would be written often, such as
# running configure, automake, makemaker, etc.  
#
# The do- targets depend on these, and they can be overridden by
# a port maintainer, since they'e pattern-based.  Thus:
#
# extract-foo.tar.gz:
#	(special stuff to unpack non-standard tarball, such as one
#	accidentally named .gz when it's really bzip2 or something)
#
# and this will override the extract-%.tar.gz rule.

# convenience variable to make the cookie.
MAKECOOKIE = mkdir -p $(COOKIEDIR)/$(@D) && date >> $(COOKIEDIR)/$@
#################### FETCH RULES ####################

URLS = $(subst ://,//,$(foreach SITE,$(FILE_SITES) $(MASTER_SITES),$(addprefix $(SITE),$(DISTFILES))) $(foreach SITE,$(FILE_SITES) $(PATCH_SITES) $(MASTER_SITES),$(addprefix $(SITE),$(PATCHFILES))))


# Download the file if and only if it doesn't have a preexisting
# checksum file.  Loop through available URLs and stop when you
# get one that doesn't return an error code.
$(DOWNLOADDIR)/%:  
	@if test -f $(COOKIEDIR)/checksum-$*; then : ; else \
		echo " ==> Grabbing $@"; \
		for i in $(filter %/$*,$(URLS)); do  \
			echo " 	==> Trying $$i"; \
			$(MAKE) -s $$i || continue; \
			mv $(PARTIALDIR)/$* $@; \
			break; \
		done; \
		if test -r $@ ; then : ; else \
			echo '*** GAR GAR GAR!  Failed to download $@!  GAR GAR GAR! ***' 1>&2; \
			false; \
		fi; \
	fi

# download an http URL (colons omitted)
http//%: 
	@wget -c -P $(PARTIALDIR) http://$*

# download an ftp URL (colons omitted)
ftp//%: 
	@wget -c --passive-ftp -P $(PARTIALDIR) ftp://$*

# link to a local copy of the file
# (absolute path)
file///%: 
	@if test -f /$*; then \
		ln -sf /$* $(PARTIALDIR)/$(notdir $*); \
	else \
		false; \
	fi

# link to a local copy of the file
# (relative path)
file//%: 
	@if test -f $*; then \
		ln -sf "$(CURDIR)/$*" $(PARTIALDIR)/$(notdir $*); \
	else \
		false; \
	fi

# Using Jeff Waugh's rsync rule.
# DOES NOT PRESERVE SYMLINKS!
rsync//%: 
	@rsync -azvLP rsync://$* $(PARTIALDIR)/

# Using Jeff Waugh's scp rule
scp//%:
	@scp -C $* $(PARTIALDIR)/

#################### CHECKSUM RULES ####################

# check a given file's checksum against $(CHECKSUM_FILE) and
# error out if it mentions the file without an "OK".
checksum-%: $(CHECKSUM_FILE) 
	@echo " ==> Running checksum on $*"
	@if grep -- '$*' $(CHECKSUM_FILE); then \
		if LC_ALL="C" LANG="C" md5sum -c $(CHECKSUM_FILE) 2>&1 | grep -- '$*' | grep -v ':[ ]\+OK'; then \
			echo '*** GAR GAR GAR!  $* failed checksum test!  GAR GAR GAR! ***' 1>&2; \
			false; \
		else \
			echo 'file $* passes checksum test!'; \
			$(MAKECOOKIE); \
		fi \
	else \
		echo '*** GAR GAR GAR!  $* not in $(CHECKSUM_FILE) file!  GAR GAR GAR! ***' 1>&2; \
		false; \
	fi
		

#################### GARCHIVE RULES ####################

# while we're here, let's just handle how to back up our
# checksummed files

$(GARCHIVEDIR)/%: $(GARCHIVEDIR)
	cp -Lr $(DOWNLOADDIR)/$* $@ 


#################### EXTRACT RULES ####################

# rule to extract uncompressed tarballs
tar-extract-%:
	@echo " ==> Extracting $(DOWNLOADDIR)/$*"
	@tar -xf $(DOWNLOADDIR)/$* -C $(EXTRACTDIR)
	@$(MAKECOOKIE)

# rule to extract files with tar xzf
tar-gz-extract-%:
	@echo " ==> Extracting $(DOWNLOADDIR)/$*"
	@gzip -dc $(DOWNLOADDIR)/$* | tar -xf - -C $(EXTRACTDIR)
	@$(MAKECOOKIE)

# rule to extract files with tar and bzip
tar-bz-extract-%:
	@echo " ==> Extracting $(DOWNLOADDIR)/$*"
	@bzip2 -dc $(DOWNLOADDIR)/$* | tar -xf - -C $(EXTRACTDIR)
	@$(MAKECOOKIE)

# rule to extract files with unzip
zip-extract-%:
	@echo " ==> Extracting $(DOWNLOADDIR)/$*"
	@unzip $(DOWNLOADDIR)/$* -d $(EXTRACTDIR)
	@$(MAKECOOKIE)

# this is a null extract rule for files which are constant and
# unchanged (not archives)
cp-extract-%:
	@echo " ==> Copying $(DOWNLOADDIR)/$*"
	@cp $(DOWNLOADDIR)/$* $(WORKDIR)/
	@$(MAKECOOKIE)

### EXTRACT FILE TYPE MAPPINGS ###
# These rules specify which of the above extract action rules to use for a
# given file extension.  Often support for a given extract type can be handled
# by simply adding a rule here.

extract-%.tar: tar-extract-%.tar
	@$(MAKECOOKIE)

extract-%.tar.gz: tar-gz-extract-%.tar.gz
	@$(MAKECOOKIE)

extract-%.tar.Z: tar-gz-extract-%.tar.Z
	@$(MAKECOOKIE)

extract-%.tgz: tar-gz-extract-%.tgz
	@$(MAKECOOKIE)

extract-%.taz: tar-gz-extract-%.taz
	@$(MAKECOOKIE)

extract-%.tar.bz: tar-bz-extract-%.tar.bz
	@$(MAKECOOKIE)

extract-%.tar.bz2: tar-bz-extract-%.tar.bz2
	@$(MAKECOOKIE)

extract-%.tbz: tar-bz-extract-%.tbz
	@$(MAKECOOKIE)

extract-%.zip: zip-extract-%.zip
	@$(MAKECOOKIE)

extract-%.ZIP: zip-extract-%.ZIP
	@$(MAKECOOKIE)

# anything we don't know about, we just assume is already
# uncompressed and unarchived in plain format
extract-%: cp-extract-%
	@$(MAKECOOKIE)


#################### PATCH RULES ####################

PATCHDIR ?= $(WORKDIR)
PATCHDIRLEVEL ?= 1
GARPATCH = patch -d$(PATCHDIR) -p$(PATCHDIRLEVEL)

# apply bzipped patches
bz-patch-%:
	@echo " ==> Applying patch $(DOWNLOADDIR)/$*"
	@bzip2 -dc $(DOWNLOADDIR)/$* |$(GARPATCH)
	@$(MAKECOOKIE)

# apply gzipped patches
gz-patch-%:
	@echo " ==> Applying patch $(DOWNLOADDIR)/$*"
	@gzip -dc $(DOWNLOADDIR)/$* |$(GARPATCH)
	@$(MAKECOOKIE)

# apply normal patches
normal-patch-%:
	@echo " ==> Applying patch $(DOWNLOADDIR)/$*"
	@$(GARPATCH) <$(DOWNLOADDIR)/$*
	@$(MAKECOOKIE)

# This is used by makepatch
%/gar-base.diff:
	@echo " ==> Creating patch $@"
	@EXTRACTDIR=$(SCRATCHDIR) COOKIEDIR=$(SCRATCHDIR)-$(COOKIEDIR) $(MAKE) extract
	@if diff --speed-large-files --minimal -Nru $(SCRATCHDIR) $(WORKDIR) > $@; then \
		rm $@; \
	fi

### PATCH FILE TYPE MAPPINGS ###
# These rules specify which of the above patch action rules to use for a given
# file extension.  Often support for a given patch format can be handled by
# simply adding a rule here.

patch-%.bz: bz-patch-%.bz
	@$(MAKECOOKIE)

patch-%.bz2: bz-patch-%.bz2
	@$(MAKECOOKIE)

patch-%.gz: gz-patch-%.gz
	@$(MAKECOOKIE)

patch-%.Z: gz-patch-%.Z
	@$(MAKECOOKIE)

patch-%.diff: normal-patch-%.diff
	@$(MAKECOOKIE)

patch-%.patch: normal-patch-%.patch
	@$(MAKECOOKIE)

#################### CONFIGURE RULES ####################

TMP_DIRPATHS = --prefix=$(prefix) --exec_prefix=$(exec_prefix) --bindir=$(bindir) --sbindir=$(sbindir) --libexecdir=$(libexecdir) --datadir=$(datadir) --sysconfdir=$(sysconfdir) --sharedstatedir=$(sharedstatedir) --localstatedir=$(localstatedir) --libdir=$(libdir) --infodir=$(infodir) --lispdir=$(lispdir) --includedir=$(includedir) --mandir=$(mandir)

NODIRPATHS += --lispdir

DIRPATHS = $(filter-out $(addsuffix %,$(NODIRPATHS)), $(TMP_DIRPATHS))

# configure a package that has an autoconf-style configure
# script.
configure-%/configure: 
	@echo " ==> Running configure in $*"
	@cd $* && $(CONFIGURE_ENV) ./configure $(CONFIGURE_ARGS)
	@$(MAKECOOKIE)

# configure a package that uses imake
# FIXME: untested and likely not the right way to handle the
# arguments
configure-%/Imakefile: 
	@echo " ==> Running imake in $*"
	@cd $* && $(CONFIGURE_ENV) imake -DUseInstalled -DBOOTSTRAPCFLAGS="$(CFLAGS)" -I$(DESTDIR)$(includedir)/X11/config $(CONFIGURE_ARGS)
	@$(MAKECOOKIE)

#################### BUILD RULES ####################

# build from a standard gnu-style makefile's default rule.
build-%/Makefile:
	@echo " ==> Running make in $*"
	@$(BUILD_ENV) $(MAKE) $(PARALLELMFLAGS) $(foreach TTT,$(BUILD_OVERRIDE_DIRS),$(TTT)="$($(TTT))") -C $* $(BUILD_ARGS)
	@$(MAKECOOKIE)

build-%/makefile:
	@echo " ==> Running make in $*"
	@$(BUILD_ENV) $(MAKE) $(PARALLELMFLAGS) $(foreach TTT,$(BUILD_OVERRIDE_DIRS),$(TTT)="$($(TTT))") -C $* $(BUILD_ARGS)
	@$(MAKECOOKIE)

build-%/GNUmakefile:
	@echo " ==> Running make in $*"
	@$(BUILD_ENV) $(MAKE) $(PARALLELMFLAGS) $(foreach TTT,$(BUILD_OVERRIDE_DIRS),$(TTT)="$($(TTT))") -C $* $(BUILD_ARGS)
	@$(MAKECOOKIE)

#################### STRIP RULES ####################
# The strip rule should probably strip uninstalled binaries.
# TODO: Seth, what was the exact parameter set to strip that you
# used to gain maximal space on the LNX-BBC?

# Strip all binaries listed in the manifest file
# TODO: actually write it!
#  This will likely become almost as hairy as the actual
#  installation code.
strip-$(MANIFEST_FILE):
	@echo "Not finished"

# The Makefile must have a "make strip" rule for this to work.
strip-%/Makefile:
	@echo " ==> Running make strip in $*"
	@$(BUILD_ENV) $(MAKE) -C $* $(BUILD_ARGS) strip
	@$(MAKECOOKIE)

#################### INSTALL RULES ####################

# just run make install and hope for the best.
install-%/Makefile:
	@echo " ==> Running make install in $*"
	@$(INSTALL_ENV) $(MAKE) DESTDIR=$(DESTDIR) $(foreach TTT,$(INSTALL_OVERRIDE_DIRS),$(TTT)="$(DESTDIR)$($(TTT))") -C $* $(INSTALL_ARGS) install
	@$(MAKECOOKIE)

install-%/makefile:
	@echo " ==> Running make install in $*"
	@$(INSTALL_ENV) $(MAKE) DESTDIR=$(DESTDIR) $(foreach TTT,$(INSTALL_OVERRIDE_DIRS),$(TTT)="$(DESTDIR)$($(TTT))") -C $* $(INSTALL_ARGS) install
	@$(MAKECOOKIE)

install-%/GNUmakefile:
	@echo " ==> Running make install in $*"
	@$(INSTALL_ENV) $(MAKE) DESTDIR=$(DESTDIR) $(foreach TTT,$(INSTALL_OVERRIDE_DIRS),$(TTT)="$(DESTDIR)$($(TTT))") -C $* $(INSTALL_ARGS) install
	@$(MAKECOOKIE)

######################################
# Use a manifest file of the format:
# src:dest[:mode[:owner[:group]]]
#   as in...
# ${WORKSRC}/nwall:${bindir}/nwall:2755:root:tty
# ${WORKSRC}/src/foo:${sharedstatedir}/foo
# ${WORKSRC}/yoink:${sysconfdir}/yoink:0600

# Okay, so for the benefit of future generations, this is how it
# works:
# 
# First of all, we have this file with colon-separated lines.
# The $(shell cat foo) routine turns it into a space-separated
# list of words.  The foreach iterates over this list, putting a
# colon-separated record in $(ZORCH) on each pass through.
# 
# Next, we have the macro $(MANIFEST_LINE), which splits a record
# into a space-separated list, and $(MANIFEST_SIZE), which
# determines how many elements are in such a list.  These are
# purely for convenience, and could be inserted inline if need
# be.
MANIFEST_LINE = $(subst :, ,$(ZORCH)) 
MANIFEST_SIZE = $(words $(MANIFEST_LINE))

# So the install command takes a variable number of parameters,
# and our records have from two to five elements.  Gmake can't do
# any sort of arithmetic, so we can't do any really intelligent
# indexing into the list of parameters.
# 
# Since the last three elements of the $(MANIFEST_LINE) are what
# we're interested in, we make a parallel list with the parameter
# switch text (note the dummy elements at the beginning):
MANIFEST_FLAGS = notused notused --mode= --owner= --group=

# The following environment variables are set before the
# installation boogaloo begins.  This ensures that WORKSRC is
# available to the manifest and that all of the location
# variables are suitable for *installation* (that is, using
# DESTDIR)

MANIFEST_ENV += WORKSRC=$(WORKSRC)
# This was part of the "implicit DESTDIR" regime.  However:
# http://gar.lnx-bbc.org/wiki/ImplicitDestdirConsideredHarmful
#MANIFEST_ENV += $(foreach TTT,prefix exec_prefix bindir sbindir libexecdir datadir sysconfdir sharedstatedir localstatedir libdir infodir lispdir includedir mandir,$(TTT)=$(DESTDIR)$($(TTT)))

# ...and then we join a slice of it with the corresponding slice
# of the $(MANIFEST_LINE), starting at 3 and going to
# $(MANIFEST_SIZE).  That's where all the real magic happens,
# right there!
# 
# following that, we just splat elements one and two of
# $(MANIFEST_LINE) on the end, since they're the ones that are
# always there.  Slap a semicolon on the end, and you've got a
# completed iteration through the foreach!  Beaujolais!

# FIXME: using -D may not be the right thing to do!
install-$(MANIFEST_FILE):
	@echo " ==> Installing from $(MANIFEST_FILE)"
	$(MANIFEST_ENV) ; $(foreach ZORCH,$(shell cat $(MANIFEST_FILE)), install -Dc $(join $(wordlist 3,$(MANIFEST_SIZE),$(MANIFEST_FLAGS)),$(wordlist 3,$(MANIFEST_SIZE),$(MANIFEST_LINE))) $(word 1,$(MANIFEST_LINE)) $(word 2,$(MANIFEST_LINE)) ;)
	@$(MAKECOOKIE)


#################### DEPENDENCY RULES ####################
# builddeps need to have everything put in $(BUILD_PREFIX)
# (unless they've been installed already, in which case they're
# already in the install dir)
# it checks the standard cookie dir first, then a special
# -builddep cookie dir, then if those fail, it does the builddep
# build with the -builddep cookie dir.  This should do The Right
# Thing.
builddep-$(GARDIR)/%:
	@echo ' ==> Building $* as a build dep'
	@$(MAKE) -C $(GARDIR)/$* install-p COOKIEDIR=cookies-builddep > /dev/null 2>&1 || \
	$(MAKE) -C $(GARDIR)/$* install	 COOKIEDIR=cookies-builddep prefix=$(BUILD_PREFIX) exec_prefix=$(BUILD_PREFIX)

# Standard deps install into the standard install dir.  For the
# BBC, we set the includedir to the build tree and the libdir to
# the install tree.  Most dependencies work this way.
# XXX: use a secondary variable to store the canonical cookiedir
# somehow.
dep-$(GARDIR)/%:
	@echo ' ==> Building $* as a dependency'
	@$(MAKE) -C $(GARDIR)/$* install-p COOKIEDIR=cookies > /dev/null 2>&1 || \
	 $(MAKE) -C $(GARDIR)/$* install

# Source Deps grab the source code for another package
srcdep-$(GARDIR)/%:
	@echo ' ==> Grabbing source for $* as a dependency'
	@$(MAKE) -C $(GARDIR)/$* patch-p extract-p > /dev/null 2>&1 || \
	 $(MAKE) -C $(GARDIR)/$* patch

# Igor's info and man gzipper rule
gzip-info-man: gzip-info gzip-man

gzip-info:
	find $(DESTDIR) -type f -iname *.info* -not -iname *.gz | \
        xargs -r gzip --force

gzip-man:
	find $(DESTDIR) -type f -iname *.[1-8] -size +2 -print | \
        xargs -r gzip --force

include $(addprefix $(GARDIR)/,$(EXTRA_LIBS))

# Mmm, yesssss.  cookies my preciousssss!  Mmm, yes downloads it
# is!  We mustn't have nasty little gmakeses deleting our
# precious cookieses now must we?
.PRECIOUS: $(DOWNLOADDIR)/% $(COOKIEDIR)/% $(FILEDIR)/%
