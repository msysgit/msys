docdir=$(PREFIX)/doc/$(PACKAGE)-$(VERSION)
doc_SOURCES = README

all: stamp-doc

stamp-doc:
	@touch stamp-doc

install: $(doc_SOURCES)
	mkdir -p $(docdir)
	cp -pf $^ $(docdir)

clean:
	rm -f stamp-doc
