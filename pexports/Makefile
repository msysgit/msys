CC=gcc -Wall
LEX=flex
YACC=bison
PACKAGE_VERSION=0.45
PACKAGE_NAME=pexports
EXEEXT=.exe
O=o

DISTFILES=AUTHORS README COPYING ChangeLog \
	Makefile hlex.l hparse.y pexports.h \
	pexports.c str_tree.c str_tree.h

OBJS=hlex.$(O) hparse.$(O) pexports.$(O) str_tree.$(O)

all: pexports$(EXEEXT)

pexports$(EXEEXT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $(OBJS)

pexports.$(O): pexports.c pexports.h str_tree.h
str_tree.$(O): str_tree.c str_tree.h
hlex.$(O): hlex.c hparse.h
hlex.c: hlex.l 
hparse.$(O): hparse.c str_tree.h
hparse.h: hparse.c
hparse.c: hparse.y
	bison -d $< -o $@

dist: $(DISTFILES)
	mkdir -p /tmp/$(PACKAGE_NAME)-$(PACKAGE_VERSION)
	cp -p $(DISTFILES) /tmp/$(PACKAGE_NAME)-$(PACKAGE_VERSION)/
	cd /tmp && tar -chof - $(PACKAGE_NAME)-$(PACKAGE_VERSION) | xz --format=lzma >\
		$(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.lzma
	mv /tmp/$(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.lzma .
	rm -rf /tmp/$(PACKAGE_NAME)-$(PACKAGE_VERSION)

clean:
	$(RM) -f *.o pexports$(EXEEXT) hlex.c hparse.c hparse.h 

realclean: clean
	$(RM) -f $(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.lzma

