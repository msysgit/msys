GARNAME = gcc
GARVERSION = 3.2.3
MASTER_SITES = \
	ftp://gcc.gnu.org/pub/$(GARNAME)/releases/$(GARNAME)-$(GARVERSION)/ \
	ftp://sources.redhat.com/pub/$(GARNAME)/releases/$(GARNAME)-$(GARVERSION)/
DISTFILES = $(GARNAME)-$(GARVERSION).tar.bz2

BOOKS = \
	$(WORKSRC)/gcc/doc/cpp.texi \
	$(WORKSRC)/gcc/doc/cppinternals.texi \
	$(WORKSRC)/gcc/doc/gcc.texi \
	$(WORKSRC)/gcc/doc/gccint.texi \
	$(WORKSRC)/gcc/f/g77.texi \
	$(WORKSRC)/gcc/java/gcj.texi \
	$(WORKSRC)/gcc/ada/gnat_ug.texi \
	$(WORKSRC)/gcc/ada/gnat_rm.texi \
	$(WORKSRC)/gcc/ada/gnat-style.texi

NODIST_BOOKS = \
	$(WORKSRC)/libiberty/libiberty.texi

include ../../gar.mk

MAKEINFO_FLAGS += --force
MAKEINFO_FLAGS += -I ../doc/include

# NOTE: The libstdc++ book can't be generated under libstcd++-v3 directory
# because WINE or HHC has problems opening files under there.
$(WORKSRC)/libstdc++-v3/docs/doxygen/user.cfg: $(WORKSRC)/libstdc++-v3/docs/doxygen/user.cfg.in
	sed \
		-e "s=@outdir@=../..=" \
		-e "s=@srcdir@=.=" \
		-e "s=@html_output_dir@=html_user=" \
		-e "s=@enabled_sections@==" \
		-e "s=@do_html@=YES=" \
		-e "s=@do_man@=NO=" \
		-e "/GENERATE_HTMLHELP/s=NO=YES=" \
		-e "/CHM_FILE/s=$$=libstdc++-$(GARVERSION).chm=" \
		$< > $@

$(WORKDIR)/html_user/index.hhp: $(WORKSRC)/libstdc++-v3/docs/doxygen/user.cfg
	cd $(WORKSRC)/libstdc++-v3 && doxygen docs/doxygen/user.cfg

$(WORKDIR)/html_user/libstdc++-$(GARVERSION).chm: $(WORKDIR)/html_user/index.hhp
	cd $(<D) && $(HHC) $(<F)

post-htmlhelp: htmlhelp-here

htmlhelp-here: $(WORKDIR)/html_user/libstdc++-$(GARVERSION).chm

post-install: install-here

install-here:
	[ -f $(WORKDIR)/html_user/libstdc++-$(GARVERSION).chm ] && cp -a $(WORKDIR)/html_user/libstdc++-$(GARVERSION).chm $(DESTDIR)/chm
