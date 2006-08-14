VER=1.0.3
SO_VER=1.0
DLLVER=1

# Where you want it installed when you do 'make install'
PREFIX=/usr
prefix=$(PREFIX)
exec_prefix=$(prefix)
bindir=$(prefix)/bin
libdir=$(prefix)/lib
incdir=$(prefix)/include
mandir=$(prefix)/share/man
man1dir=$(mandir)/man1
docdir=$(prefix)/share/doc/bzip2-$(VER)

SHELL=/bin/sh

# To assist in cross-compiling
CC=gcc
AR=ar
RANLIB=ranlib
LDFLAGS=

BIGFILES=-D_FILE_OFFSET_BITS=64
CFLAGS=-Wall -Winline -O -g $(BIGFILES)

STRIP=strip
LN=cp
INSTALL=install
LDSHARED=gcc -shared -Wl,--export-all
LDEXTRA=-Wl,--out-implib=$(IMPLIB) 

DOCS=CHANGES LICENSE README Y2K_INFO manual.ps manual.pdf manual.html \
	README.XML.STUFF README.COMPILATION.PROBLEMS 
 
.SUFFIXES:
.SUFFIXES: .c .o

%.o : %.c
	$(CC) -c $(CFLAGS) -I. -o $@ $<


OBJS= blocksort.o  \
      huffman.o    \
      crctable.o   \
      randtable.o  \
      compress.o   \
      decompress.o \
      bzlib.o

SHAREDLIB=cygbz2-${DLLVER}.dll
STATLIB=libbz2.a
IMPLIB=libbz2.dll.a
SHAREDDEF=libbz2.def
EXE=.exe

all: ${STATLIB} ${SHAREDLIB} bzip2$(EXE) bzip2-stat$(EXE) \
	bzip2recover$(EXE) bzip2recover-stat$(EXE) \
	test

bzip2$(EXE): bzip2.o $(SHAREDLIB)
	$(CC) $(CFLAGS) -o $@ $< -L. -lbz2
bzip2-stat$(EXE): bzip2.o $(STATLIB)
	$(CC) -static $(CFLAGS) -o $@ $< -L. -lbz2

bzip2recover$(EXE): bzip2recover.o
	$(CC) $(CFLAGS) -o $@ $<
bzip2recover-stat$(EXE): bzip2recover.o
	$(CC) -static $(CFLAGS) -o $@ $<

$(STATLIB): $(OBJS)
	rm -f $(STATLIB)
	$(AR) cq $(STATLIB) $(OBJS)
	ranlib $(STATLIB)

$(SHAREDLIB): $(OBJS)
	rm -f $(SHAREDLIB) $(IMPLIB)
	$(LDSHARED) -o $@ $(LDEXTRA) $^
  

check: test
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

install: bzip2-stat$(EXE) bzip2recover-stat$(EXE) $(INFO)
	if ( test ! -d $(bindir) ) ; then mkdir -p $(bindir) ; fi
	if ( test ! -d $(libdir) ) ; then mkdir -p $(libdir) ; fi
	if ( test ! -d $(mandir) ) ; then mkdir -p $(mandir) ; fi
	if ( test ! -d $(man1dir) ) ; then mkdir -p $(man1dir) ; fi
	if ( test ! -d $(incdir) ) ; then mkdir -p $(incdir) ; fi
	if ( test ! -d $(docdir) ) ; then mkdir -p $(docdir) ; fi
	install -m 755 -s bzip2-stat$(EXE) $(bindir)/bzip2$(EXE)
	install -m 755 -s bzip2-stat$(EXE) $(bindir)/bunzip2$(EXE)
	install -m 755 -s bzip2-stat$(EXE) $(bindir)/bzcat$(EXE)
	install -m 755 -s bzip2recover-stat$(EXE) $(bindir)/bzip2recover$(EXE)
	install -m 755 bzgrep $(bindir)/bzgrep
	install -m 755 bzmore $(bindir)/bzmore
	install -m 755 bzdiff $(bindir)/bzdiff
	(cd $(bindir) ;\
	rm -f bzegrep bzfgrep bzless bzcmp ;\
	ln -s bzgrep bzegrep ;\
	ln -s bzgrep bzfgrep ;\
	ln -s bzmore bzless ;\
	ln -s bzdiff bzcmp )
	install -m 644 bzip2.1 bzgrep.1 bzmore.1 bzdiff.1 $(man1dir)
	echo ".so man1/bzgrep.1" > $(man1dir)/bzegrep.1
	echo ".so man1/bzgrep.1" > $(man1dir)/bzfgrep.1
	echo ".so man1/bzmore.1" > $(man1dir)/bzless.1
	echo ".so man1/bzdiff.1" > $(man1dir)/bzcmp.1
	install -m 644 bzlib.h $(incdir)
	install -m 644 $(STATLIB) $(libdir)
	install -m 644 $(IMPLIB) $(libdir)
	install -m 755 -s $(SHAREDLIB) $(bindir)
	install -m 644 $(DOCS) $(docdir)


clean: 
	rm -f *.o $(STATLIB) $(SHAREDLIB) $(IMPLIB) \
	bzip2-stat$(EXE) bzip2recover-stat$(EXE) \
	bzip2$(EXE) bzip2recover$(EXE) \
	sample1.rb2 sample2.rb2 sample3.rb2 \
	sample1.tst sample2.tst sample3.tst \
	$(INFO)

blocksort.o: blocksort.c
	@cat words0
	$(CC) $(CFLAGS) -c -I. -o $@ $<


distclean: clean
	rm -f manual.ps manual.html manual.pdf

DISTNAME=bzip2-1.0.3
dist: check manual
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
	   $(DISTNAME)/manual.html \
	   $(DISTNAME)/manual.pdf \
	   $(DISTNAME)/manual.ps \
	   $(DISTNAME)/README \
	   $(DISTNAME)/README.COMPILATION.PROBLEMS \
	   $(DISTNAME)/README.XML.STUFF \
	   $(DISTNAME)/CHANGES \
	   $(DISTNAME)/libbz2.def \
	   $(DISTNAME)/libbz2.dsp \
	   $(DISTNAME)/dlltest.dsp \
	   $(DISTNAME)/makefile.msc \
	   $(DISTNAME)/Y2K_INFO \
	   $(DISTNAME)/unzcrash.c \
	   $(DISTNAME)/spewG.c \
	   $(DISTNAME)/mk251.c \
	   $(DISTNAME)/bzdiff \
	   $(DISTNAME)/bzdiff.1 \
	   $(DISTNAME)/bzmore \
	   $(DISTNAME)/bzmore.1 \
	   $(DISTNAME)/bzgrep \
	   $(DISTNAME)/bzgrep.1 \
	   $(DISTNAME)/Makefile-libbz2_so \
	   $(DISTNAME)/bz-common.xsl \
	   $(DISTNAME)/bz-fo.xsl \
	   $(DISTNAME)/bz-html.xsl \
	   $(DISTNAME)/bzip.css \
	   $(DISTNAME)/entities.xml \
	   $(DISTNAME)/manual.xml \
	   $(DISTNAME)/format.pl \
	   $(DISTNAME)/xmlproc.sh
	gzip -v $(DISTNAME).tar

# For rebuilding the manual from sources on my SuSE 9.1 box

MANUAL_SRCS= 	bz-common.xsl bz-fo.xsl bz-html.xsl bzip.css \
		entities.xml manual.xml 

manual: manual.html manual.ps manual.pdf

manual.ps: $(MANUAL_SRCS)
	./xmlproc.sh -ps manual.xml

manual.pdf: $(MANUAL_SRCS)
	./xmlproc.sh -pdf manual.xml

manual.html: $(MANUAL_SRCS)
	./xmlproc.sh -html manual.xml
