# Html Help books generation

all: htmlhelp

# htmlhelp	- Generate Html Help books.
ifdef GARVERSION
TAG = -$(GARVERSION)
else
TAG =
endif

HTMLHELP_TARGETS = $(addsuffix $(TAG).chm,$(basename $(filter %.texi %.texinfo %.txi %.xml,$(BOOKS))))

htmlhelp: build pre-htmlhelp $(HTMLHELP_TARGETS) post-htmlhelp
	$(DONADA)

# returns true if Html Help books have completed successfully, false otherwise
htmlhelp-p:
	@$(foreach COOKIEFILE,$(HTMLHELP_TARGETS), test -e $(COOKIEDIR)/$(COOKIEFILE) ;)

post-install:
	$(foreach FILE,$(HTMLHELP_TARGETS), cp -a $(FILE) $(DESTDIR)/chm ;)


#################### XML RULES ####################
# Rules to generate DocBook XML books


# Texinfo

# NOTE: This uses a local version of texinfo from CVS where I try to fix some
# of the bugs of makeinfo DocBook XML output until they're accepted upstream.
#MAKEINFO = makeinfo 
MAKEINFO = /home/jfonseca/projects/htmlhelp/texinfo/makeinfo/makeinfo
MAKEINFO_FLAGS = --docbook --ifinfo

%.xml: %.txi
	cd $(@D) && $(MAKEINFO) $(MAKEINFO_FLAGS) -o $(@F) $(<F)
	
%.xml: %.texi
	cd $(@D) && $(MAKEINFO) $(MAKEINFO_FLAGS) -o $(@F) $(<F)

%.xml: %.texinfo
	cd $(@D) && $(MAKEINFO) $(MAKEINFO_FLAGS) -o $(@F) $(<F)


#################### HTMLHELP RULES ####################

# Compilation

ifdef WIN32
HHC = "C:/Program Files/HTML Help Workshop/hhc.exe"
else
HHC = wine "C:/Program Files/HTML Help Workshop/hhc.exe"

.PRECIOUS: %.hhp %.hhc %.hhk
endif

%$(TAG).chm: %.hhp %.hhc %.hhk
	cd $(<D) && $(HHC) $(<F)

# DocBook XML (using XSL)

XSLTPROC = xsltproc 
XSLTPROC_FLAGS = \
	--docbook \
	--stringparam "base.dir" "$(*F)/" \
	--stringparam "generate.toc" "" \
	--stringparam "htmlhelp.chm" "$(*F)$(TAG).chm" \
	--stringparam "htmlhelp.hhp" "$(*F).hhp" \
	--stringparam "htmlhelp.hhc" "$(*F).hhc" \
	--stringparam "htmlhelp.hhk" "$(*F).hhk" \
	--param htmlhelp.use.hhk 1 \
	--param htmlhelp.autolabel 1 \
	--param chapter.autolabel 1 \
	--param appendix.autolabel 1 \
	--param section.autolabel 1 \
	--param section.label.includes.component.label 1 \
	--param chunk.first.sections 1

HTMLHELP_XSL = /usr/share/sgml/docbook/stylesheet/xsl/nwalsh/htmlhelp/htmlhelp.xsl

%.hhp %.hhc %.hhk: %.xml
	rm -rf $*
	cd $(*D) && $(XSLTPROC) $(XSLTPROC_FLAGS) $(HTMLHELP_XSL) $(<F)
ifdef FIGURES
	cp -r $(FIGURES) $*
endif
