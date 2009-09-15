EXTRA_DIST = include/CFile.h \
	include/CList.h \
	include/winui.h \
	winui.prj \
	src/makefile \
	ChangeLog

noinst_LIBRARIES = libwinui.a
libwinui_a_SOURCES = src/CFile.cpp src/CList.cpp src/winui.cpp
libwinui_a_CFLAGS = -Wall -pedantic -fvtable-thunks -fno-exceptions -fno-rtti
libwinui_a_CPPFLAGS = -I$(top_srcdir)/winui/include

install-data-hook:
	$(MKDIR_P) $(DESTDIR)$(docdir)
	$(INSTALL_DATA) $(srcdir)/ChangeLog $(DESTDIR)$(docdir)/libwinui.ChangeLog
