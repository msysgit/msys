# Rules to generate chm books

# Hooks to the main targets

BOOKS_CHM = $(patsubst %.sgml,%.chm,$(patsubst %.texinfo,%.chm,$(patsubst %.texi,%.chm,$(BOOKS))))

all: all_chm

.PHONY: all_chm
all_chm:  $(BOOKS_CHM)

clean: clean_chm

.PHONY: clean_chm
clean_chm:
	-rm -f *.html
	-rm -f *.{hhp,hhc,hhk,chm}


# Common

HHC = -wine "C:/Program Files/HTML Help Workshop/hhc.exe"

%.chm: %.hhp
	$(HHC) $<


# Texinfo

TEXI2CHM = perl $(TOP_DIR)/scripts/texi2chm

%.hhp: %.texi
	$(TEXI2CHM) $(TEXI2HTML_FLAGS) $<

%.hhp: %.texinfo
	$(TEXI2CHM) $(TEXI2HTML_FLAGS) $<


# DocBook

CHM_DSL = $(TOP_DIR)/stylesheets/htmlhelp.dsl

CATALOG = /etc/sgml/catalog 

ifdef STYLESHEET
DSSSL=htmlhelp.dsl

$(DSSSL): $(CHM_DSL)
	sed -e "s/docbook\.dsl/$(STYLESHEET)/" $< > $@

clean: clean_dsssl

.PHONY: clean_dsssl
clean_dsssl:
	-rm -f $(DSSSL)
else
DSSSL=$(CHM_DSL)
endif

%.chm: %.sgml $(DSSSL)
	jade -t sgml -i html -c $(CATALOG) -d $(DSSSL) $<

