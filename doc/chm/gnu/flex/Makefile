TOP_DIR = ../..

VERSION = 2.5.4
BOOKS = flex.texi

include $(TOP_DIR)/rules.mak

flex.texi: orig/flex.texi
	sed -e "s/@section/@chapter/g" $< > $@

clean:
	rm -f flex.texi
