bindir=$(PREFIX)/bin
bin_PROGRAMS = portmaker

portmaker_SOURCES = \
	portmaker.sh \
	version.sh

all: $(bin_PROGRAMS)

install: $(bin_PROGRAMS)
	mkdir -p $(bindir)
	cp -pf $^ $(bindir)

clean: $(bin_PROGRAMS)
	rm -f $^

portmaker: portmaker.sh version.sh
	$(SHELL) -c '. version.sh && sed -e "s/@VERSION@/$$VERSION/g" -e "s~@PREFIX@~$$PREFIX~g" $^ > $@'
	chmod 755 $@
