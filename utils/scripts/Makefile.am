bin_SCRIPTS = a2dll dsw2mak

EXTRA_DIST = a2dll.html static2dll_howto.txt

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(prefix)/doc/a2dll
	$(INSTALL_DATA) $(srcdir)/a2dll.html $(DESTDIR)$(prefix)/doc/a2dll/a2dll.html
	$(INSTALL_DATA) $(srcdir)/static2dll_howto.txt $(DESTDIR)$(prefix)/doc/a2dll/static2dll_howto.txt
