SUBDIRS = dos2unix drmingw pexports redir reimp res2coff scripts unix2dos

instdir = /tmp/$(PACKAGE)-$(VERSION)

sdist: dist
	mv $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE)-$(VERSION)-src.tar.gz

bdist:
	mkdir $(instdir)
	make prefix=$(instdir) install
	cd $(instdir) && $(AMTAR) chof - * | GZIP=$(GZIP_ENV) gzip -c > /tmp/$(PACKAGE)-$(VERSION).tar.gz
	mv /tmp/$(PACKAGE)-$(VERSION).tar.gz .
	rm -rf $(instdir)
