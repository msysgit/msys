TOP_DIR = ../..

VERSION = 2.12.90.0.7
BOOKS = as.texi bfd.texinfo bfdint.texi binutils.texi ld.texinfo ldint.texinfo libiberty.texi standards.texi

TEXI2HTML_FLAGS = --expand=tex

all: gasver.texi ldver.texi

include $(TOP_DIR)/rules.mak

gasver.texi ldver.texi:
	echo "@set VERSION $(VERSION)" > $@

clean:
	rm -f gasver.texi ldver.texi
