TOP_DIR = ../..

VERSION = 2.2.4
BOOKS = libc.texinfo

include $(TOP_DIR)/rules.mak

libc.texinfo: orig/libc.texinfo fix-linewrapping.sed
	for i in $(wildcard orig/*.texi*); do sed -f fix-linewrapping.sed $$i > `basename $$i`; done

clean:
	rm -f *.texi*
