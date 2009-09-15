SUBDIRS = redir reimp res2coff scripts msys-here

doc_DATA = COPYING README LICENSE NEWS AUTHORS ChangeLog TODO

instdir = /tmp/$(PACKAGE)-$(VERSION)
PKGREL = 1
SUBSYSTEM = mingw32
LIC_CONTENTS=$(docdir)/LICENSE $(docdir)/COPYING

srcdist: dist
	mv $(PACKAGE)-$(VERSION).tar.lzma $(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-src.tar.lzma

staging:
	mkdir $(instdir)
	make prefix=$(instdir) install

lic.list:
	rm -f lic.list
	echo $(LIC_CONTENTS) | while read F ; do\
		echo $F | sed -e "s@$(instdir)[/]@@g" >> lic.list;
	done

bindist: staging lic.list
	mkdir $(instdir)
	make prefix=$(instdir) install
	cd $(instdir) && $(AMTAR) chof - bin | xz --format=lzma -c > \
		/tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-bin.tar.lzma
	mv /tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-bin.tar.lzma .
	cd $(instdir) && $(AMTAR) chof - -T lic.list | xz --format=lzma -c > \
		/tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-lic.tar.lzma
	mv /tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-lic.tar.lzma .
	cd $(instdir) && $(AMTAR) chof - -X lic.list share | xz --format=lzma -c > \ 
		/tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-doc.tar.lzma
	mv /tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-doc.tar.lzma .
	rm -rf $(instdir)

