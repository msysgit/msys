bin_PROGRAMS = res2coff

res2coff_SOURCES = objimage.c protos.h res2coff.c res2coff.h resimage.c

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(prefix)/doc/res2coff
	$(INSTALL_DATA) $(srcdir)/README $(DESTDIR)$(prefix)/doc/res2coff/README
