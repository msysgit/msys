# top-level Makefile for the entire tree.
%:
	@for i in $(filter-out CVS/,$(wildcard */)) ; do \
		test ! -e $$i/Makefile || $(MAKE) -C $$i $* ; \
	done

paranoid-%:
	@for i in $(filter-out CVS/,$(wildcard */)) ; do \
		test ! -e $$i/Makefile || $(MAKE) -C $$i $@ || exit 2; \
	done

export BUILDLOG ?= $(shell pwd)/buildlog.txt

report-%:
	@echo "$@ started at $$(date)" >> $(BUILDLOG)
	@for i in $(filter-out CVS/,$(wildcard */)) ; do \
		test ! -e $$i/Makefile || $(MAKE) -C $$i $@ || echo "	make $@ in category $$i failed" >> $(BUILDLOG); \
	done
	@echo "$@ completed at $$(date)" >> $(BUILDLOG)
