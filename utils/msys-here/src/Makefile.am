bin_PROGRAMS = msys-here
msys_here_SOURCES = main.cpp  main.h  rsrc.h  rsrc.rc
msys_here_CPPFLAGS = -I$(top_srcdir)/winui/include -I$(top_srcdir)/lib -I$(top_srcdir)/src
msys_here_LDADD = $(top_builddir)/winui/libwinui.a \
	$(top_builddir)/lib/libgetopt.a \
	-lcomctl32 -lcomdlg32 -lgdi32
msys_here_LDFLAGS = -static-libgcc

msys_here_icons_list = m.ico

EXTRA_DIST = $(msys_here_icons_list)

# hacks for RC support
RCFLAGS = -I$(srcdir)
rc_verbose = $(rc_verbose_$(V))
rc_verbose_ = $(rc_verbose_$(AM_DEFAULT_VERBOSITY))
rc_verbose_0 = @echo "  CCRC  " $@;

RCCOMPILE = $(WINDRES) \
	`echo $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(RCFLAGS) |\
	sed -e 's/-I/--include-dir /g;s/-D/--define /g'`

.rc.o:
	$(rc_verbose)$(RCCOMPILE) -i $< -o $@

## Extra dependencies for rsrc.lo
rsrc.o: rsrc.rc $(msys_here_icons_list) rsrc.h

