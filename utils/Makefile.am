SUBDIRS = redir reimp res2coff scripts msys-here

doc_DATA = COPYING README LICENSE NEWS AUTHORS ChangeLog TODO
EXTRA_DIST = LICENSE

instdir = /tmp/$(PACKAGE)-$(VERSION)
PKGREL = 1
SUBSYSTEM = mingw32
LIC_CONTENTS=$(docdir)/LICENSE $(docdir)/COPYING $(docdir)/msys-here/COPYING

sdist: dist
	mv $(PACKAGE)-$(VERSION).tar.lzma $(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-src.tar.lzma

staging:
	mkdir $(instdir)
	make prefix=$(instdir) install

.PHONY: lic.list
lic.list:
	rm -f lic.list
	for F in $(LIC_CONTENTS); do\
		echo $$F | sed -e 's@$(prefix)[/]@@g' -e 's@$(instdir)[/]@@g' >> lic.list;\
	done

bdist: staging lic.list
	cd $(instdir) && $(AMTAR) chof - bin | xz --format=lzma -c > \
		/tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-bin.tar.lzma
	mv /tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-bin.tar.lzma .
	cd $(instdir) && $(AMTAR) chof - -T $(abs_builddir)/lic.list |\
		xz --format=lzma -c > \
		/tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-lic.tar.lzma
	mv /tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-lic.tar.lzma .
	cd $(instdir) && $(AMTAR) chof - -X $(abs_builddir)/lic.list share |\
		xz --format=lzma -c > \
		/tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-doc.tar.lzma
	mv /tmp/$(PACKAGE)-$(VERSION)-$(PKGREL)-$(SUBSYSTEM)-doc.tar.lzma .
	rm -rf $(instdir)

# Ensure AC_CONFIG_MACRO_DIR is distributed, even if empty
dist-hook:
	$(MKDIR_P) $(top_distdir)/m4

