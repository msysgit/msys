VER=1.0.1
SO_VER=1.0

PREFIX=/usr
prefix=$(PREFIX)
exec_prefix=$(prefix)
bindir=$(prefix)/bin
libdir=$(prefix)/lib
incdir=$(prefix)/include
mandir=$(prefix)/man
man1dir=$(mandir)/man1
infodir=$(prefix)/info
docdir=$(prefix)/doc/bzip2-$(VER)
cygdocdir=$(prefix)/doc/Cygwin

SHELL=/bin/sh
CC=gcc
STRIP=strip
LN=cp
BIGFILES=-D_FILE_OFFSET_BITS=64
CFLAGS=-Wall -Winline -O2 -fomit-frame-pointer -fno-strength-reduce $(BIGFILES)
INSTALL=install
LDSHARED=gcc -shared -Wl,--enable-auto-image-base
LDEXTRA=-Wl,--out-implib=$(IMPLIB) $(SHAREDDEF)

DOCS=CHANGES LICENSE README Y2K_INFO manual.ps manual.texi manual_1.html \
     manual_2.html manual_3.html manual_4.html manual_toc.html
INFO=bzip2.info bzip2.info-1 bzip2.info-2 bzip2.info-3

.SUFFIXES:
.SUFFIXES: .c .o .pic.o .dyn.o

%.o : %.c
	$(CC) -c $(CFLAGS) -I. -DBZLIB_STATIC -o $@ $<

%.pic.o : %.c
	$(CC) -c $(CFLAGS) -I. -DBZLIB_DLL -o $@ $<

%.dyn.o : %.c
	$(CC) -c $(CFLAGS) -I. -o $@ $<

OBJS= blocksort.o  \
      huffman.o    \
      crctable.o   \
      randtable.o  \
      compress.o   \
      decompress.o \
      bzlib.o

OBJS_DLL= ${OBJS:.o=.pic.o}

SHAREDLIB=cygbz2${SO_VER}.dll
STATLIB=libbz2.a
IMPLIB=libbz2.dll.a
SHAREDDEF=libbz2.def
EXE=.exe

all: ${STATLIB} ${SHAREDLIB} bzip2$(EXE) bzip2-stat$(EXE) \
	bzip2recover$(EXE) bzip2recover-stat$(EXE) \
	$(INFO) test

bzip2$(EXE): bzip2.dyn.o $(STATLIB)
	$(CC) $(CFLAGS) -o $@ $< -L. -lbz2
bzip2-stat$(EXE): bzip2.o $(STATLIB)
	$(CC) -static $(CFLAGS) -o $@ $< -L. -lbz2

bzip2recover$(EXE): bzip2recover.dyn.o
	$(CC) $(CFLAGS) -o $@ $<
bzip2recover-stat$(EXE): bzip2recover.o
	$(CC) -static $(CFLAGS) -o $@ $<

$(STATLIB): $(OBJS)
	rm -f $(STATLIB)
	ar cq $(STATLIB) $(OBJS)
	ranlib $(STATLIB)

$(SHAREDLIB): $(OBJS_DLL)
	rm -f $(SHAREDLIB) $(IMPLIB)
	$(LDSHARED) -o $@ $(LDEXTRA) $(OBJS_DLL)

test: test-dyn test-stat

test-dyn: bzip2$(EXE)
	@cat words1
	./bzip2 -1  < sample1.ref > sample1.rb2
	./bzip2 -2  < sample2.ref > sample2.rb2
	./bzip2 -3  < sample3.ref > sample3.rb2
	./bzip2 -d  < sample1.bz2 > sample1.tst
	./bzip2 -d  < sample2.bz2 > sample2.tst
	./bzip2 -ds < sample3.bz2 > sample3.tst
	cmp sample1.bz2 sample1.rb2 
	cmp sample2.bz2 sample2.rb2
	cmp sample3.bz2 sample3.rb2
	cmp sample1.tst sample1.ref
	cmp sample2.tst sample2.ref
	cmp sample3.tst sample3.ref
	@cat words3

test-stat: bzip2-stat$(EXE)
	@cat words1
	./bzip2-stat -1  < sample1.ref > sample1.rb2
	./bzip2-stat -2  < sample2.ref > sample2.rb2
	./bzip2-stat -3  < sample3.ref > sample3.rb2
	./bzip2-stat -d  < sample1.bz2 > sample1.tst
	./bzip2-stat -d  < sample2.bz2 > sample2.tst
	./bzip2-stat -ds < sample3.bz2 > sample3.tst
	cmp sample1.bz2 sample1.rb2 
	cmp sample2.bz2 sample2.rb2
	cmp sample3.bz2 sample3.rb2
	cmp sample1.tst sample1.ref
	cmp sample2.tst sample2.ref
	cmp sample3.tst sample3.ref
	@cat words3

$(INFO): manual.texi
	makeinfo manual.texi

install: bzip2-stat$(EXE) bzip2recover-stat$(EXE) $(INFO)
	if ( test ! -d $(bindir) ) ; then mkdir -p $(bindir) ; fi
	if ( test ! -d $(libdir) ) ; then mkdir -p $(libdir) ; fi
	if ( test ! -d $(mandir) ) ; then mkdir -p $(mandir) ; fi
	if ( test ! -d $(man1dir) ) ; then mkdir -p $(man1dir) ; fi
	if ( test ! -d $(incdir) ) ; then mkdir -p $(incdir) ; fi
	if ( test ! -d $(docdir) ) ; then mkdir -p $(docdir) ; fi
	if ( test ! -d $(cygdocdir) ) ; then mkdir -p $(cygdocdir) ; fi
	if ( test ! -d $(infodir) ) ; then mkdir -p $(infodir) ; fi
	install -m 755 -s bzip2-stat$(EXE) $(bindir)/bzip2$(EXE)
	install -m 755 -s bzip2-stat$(EXE) $(bindir)/bunzip2$(EXE)
	install -m 755 -s bzip2-stat$(EXE) $(bindir)/bzcat$(EXE)
	install -m 755 -s bzip2recover-stat$(EXE) $(bindir)/bzip2recover$(EXE)
	install -m 644 bzip2.1 $(man1dir)
	install -m 644 bzlib.h $(incdir)
	install -m 644 $(STATLIB) $(libdir)
	install -m 644 $(IMPLIB) $(libdir)
	install -m 644 -s $(SHAREDLIB) $(bindir)
	install -m 644 CYGWIN-PATCHES/bzip2-$(VER).README $(cygdocdir)
	install -m 644 $(DOCS) $(docdir)
	install -m 644 $(INFO) $(infodir)
	install-info --info-dir=$(infodir) --info-file=$(infodir)/bzip2.info

clean: 
	rm -f *.o $(STATLIB) $(SHAREDLIB) $(IMPLIB) \
	bzip2-stat$(EXE) bzip2recover-stat$(EXE) \
	bzip2$(EXE) bzip2recover$(EXE) \
	sample1.rb2 sample2.rb2 sample3.rb2 \
	sample1.tst sample2.tst sample3.tst \
	$(INFO)

blocksort.o: blocksort.c
	@cat words0
	$(CC) -c $(CFLAGS) -I. -DBZLIB_STATIC -o $@ $<
blocksort.pic.o: blocksort.c
	@cat words0
	$(CC) -c $(CFLAGS) -I. -DBZLIB_DLL -o $@ $<

DISTNAME=bzip2-1.0.1
tarfile:
	rm -f $(DISTNAME)
	ln -sf . $(DISTNAME)
	tar cvf $(DISTNAME).tar \
	   $(DISTNAME)/blocksort.c \
	   $(DISTNAME)/huffman.c \
	   $(DISTNAME)/crctable.c \
	   $(DISTNAME)/randtable.c \
	   $(DISTNAME)/compress.c \
	   $(DISTNAME)/decompress.c \
	   $(DISTNAME)/bzlib.c \
	   $(DISTNAME)/bzip2.c \
	   $(DISTNAME)/bzip2recover.c \
	   $(DISTNAME)/bzlib.h \
	   $(DISTNAME)/bzlib_private.h \
	   $(DISTNAME)/Makefile \
	   $(DISTNAME)/manual.texi \
	   $(DISTNAME)/manual.ps \
	   $(DISTNAME)/LICENSE \
	   $(DISTNAME)/bzip2.1 \
	   $(DISTNAME)/bzip2.1.preformatted \
	   $(DISTNAME)/bzip2.txt \
	   $(DISTNAME)/words0 \
	   $(DISTNAME)/words1 \
	   $(DISTNAME)/words2 \
	   $(DISTNAME)/words3 \
	   $(DISTNAME)/sample1.ref \
	   $(DISTNAME)/sample2.ref \
	   $(DISTNAME)/sample3.ref \
	   $(DISTNAME)/sample1.bz2 \
	   $(DISTNAME)/sample2.bz2 \
	   $(DISTNAME)/sample3.bz2 \
	   $(DISTNAME)/dlltest.c \
	   $(DISTNAME)/*.html \
	   $(DISTNAME)/README \
	   $(DISTNAME)/README.COMPILATION.PROBLEMS \
	   $(DISTNAME)/CHANGES \
	   $(DISTNAME)/libbz2.def \
	   $(DISTNAME)/libbz2.dsp \
	   $(DISTNAME)/dlltest.dsp \
	   $(DISTNAME)/makefile.msc \
	   $(DISTNAME)/Y2K_INFO \
	   $(DISTNAME)/unzcrash.c \
	   $(DISTNAME)/spewG.c \
	   $(DISTNAME)/Makefile-libbz2_so
