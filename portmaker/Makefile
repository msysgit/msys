include config.mak
SUBDIRS = src lib template doc

all: $(SUBDIRS)
	$(MAKE) $^ REDIRECT=$@

install: $(SUBDIRS)
	$(MAKE) $^ REDIRECT=$@

clean: $(SUBDIRS)
	$(MAKE) $^ REDIRECT=$@

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ -f Makefile $(REDIRECT) VERSION=$(VERSION) PREFIX=$(PREFIX) PACKAGE=$(PACKAGE)
