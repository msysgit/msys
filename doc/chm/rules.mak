# Common rules

.PHONY: all all_rules
all: all_rules

.PHONY: all_rules
all_rules:
ifdef SUB_DIRS
	for i in $(SUB_DIRS) ; do $(MAKE) -C $$i all ; done
endif

.PHONY: clean
clean: clean_rules

.PHONY: clean_rules
clean_rules:
ifdef SUB_DIRS
	for i in $(SUB_DIRS) ; do $(MAKE) -C $$i clean ; done
endif

.PHONY: install
install: install_rules

.PHONY: install_rules
install_rules:
ifdef SUB_DIRS
	for i in $(SUB_DIRS) ; do $(MAKE) -C $$i install ; done
endif


# Include backend rules
include $(TOP_DIR)/htmlhelp.mak
