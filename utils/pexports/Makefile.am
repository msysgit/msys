bin_PROGRAMS = pexports

pexports_SOURCES = hlex.l hparse.h hparse.y pe.h pexports.c pexports.h str_tree.c str_tree.h

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(prefix)/doc/pexports
	$(INSTALL_DATA) $(srcdir)/README $(DESTDIR)$(prefix)/doc/pexports/README
