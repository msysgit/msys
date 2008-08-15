# Makefile for TDMInstall
# Created: JohnE, 2008-07-28
#
# Tested under MinGW GCC 3.4.5 and (unofficial) 4.3.1


SETUPEXEFULL = output/tdm-mingw-full.exe
SETUPEXEWEBDL = output/tdm-mingw-webdl.exe
TDMINSTDLL = plugins/tdminstall.dll


CC = mingw32-gcc
CFLAGS = -Wall -Os -I. -Ibzip2 -Izlib
CXX = mingw32-g++
CXXFLAGS = -Wall -Os -I. -Ibzip2 -Izlib
LD = mingw32-g++
LDFLAGS = -s


.PHONY: all

all: $(SETUPEXEFULL) $(SETUPEXEWEBDL)

$(SETUPEXEFULL): setup_full.nsi main.nsh ctemplate.exe $(TDMINSTDLL)
	makensis /V2 setup_full.nsi

$(SETUPEXEWEBDL): setup_webdl.nsi main.nsh ctemplate.exe $(TDMINSTDLL)
	makensis /V2 setup_webdl.nsi

TINYXMLSOURCES = \
  tinyxml/tinystr.o \
  tinyxml/tinyxml.o \
  tinyxml/tinyxmlerror.o \
  tinyxml/tinyxmlparser.o

ctemplate.exe: ctemplate.o $(TINYXMLSOURCES)
	$(LD) $(LDFLAGS) -o ctemplate.exe ctemplate.o $(TINYXMLSOURCES)

TDMINSTDLLSOURCES = \
  $(TINYXMLSOURCES) \
  7z/Archive/7z/7zAlloc.o \
  7z/Archive/7z/7zBuffer.o \
  7z/Archive/7z/7zDecode.o \
  7z/Archive/7z/7zExtract.o \
  7z/Archive/7z/7zHeader.o \
  7z/Archive/7z/7zIn.o \
  7z/Archive/7z/7zItem.o \
  7z/Compress/Branch/BranchX86.o \
  7z/Compress/Branch/BranchX86_2.o \
  7z/Compress/Lzma/LzmaDecode.o \
  7z/7zCrc.o \
  7z/Alloc.o \
  bzip2/bzlib.o \
  bzip2/crctable.o \
  bzip2/decompress.o \
  bzip2/huffman.o \
  bzip2/randtable.o \
  zlib/contrib/minizip/ioapi.o \
  zlib/contrib/minizip/unzip.o \
  zlib/adler32.o \
  zlib/crc32.o \
  zlib/deflate.o \
  zlib/gzio.o \
  zlib/inffast.o \
  zlib/inflate.o \
  zlib/inftrees.o \
  zlib/trees.o \
  zlib/zutil.o \
  archive.o \
  archive_7z.o \
  archive_base.o \
  archive_zip.o \
  componentstree.o \
  install_manifest.o \
  multiread.o \
  nsis_interface.o \
  tdminst_res.o \
  tdminstall.o \
  untgz.o

$(TDMINSTDLL): $(TDMINSTDLLSOURCES)
	$(LD) -shared -Wl,--dll $(LDFLAGS) -o $(TDMINSTDLL) $(TDMINSTDLLSOURCES) \
	 -lgdi32 -lcomctl32

tdminst_res.o: tdminst_res.rc tdminst_res.h
	windres tdminst_res.rc tdminst_res.o
