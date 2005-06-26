libdir = $(PREFIX)/lib/$(PACKAGE)-$(VERSION)

libmingwPORT_SOURCES = mingwPORT.functions

all: stamp-lib

stamp-lib:
	touch stamp-lib

install: $(libmingwPORT_SOURCES)
	mkdir -p $(libdir)
	cp -p $^ $(libdir)

clean:
	rm -f stamp-lib
