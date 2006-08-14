# The default database to build and search.
AUTOMAKE_OPTIONS = std-options
LOCATE_DB = $(localstatedir)/locatedb
localedir = $(datadir)/locale

AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = frcode code bigram
bin_PROGRAMS = locate 
libexec_PROGRAMS = frcode code bigram
bin_SCRIPTS = updatedb
man_MANS = locate.1 updatedb.1 locatedb.5

EXTRA_DIST = locatedb.h updatedb.sh $(man_MANS)
CLEANFILES = updatedb

INCLUDES = -I$(top_srcdir)/lib -I../gnulib/lib -I$(top_srcdir)/gnulib/lib -I../intl -DLOCATE_DB=\"$(LOCATE_DB)\" -DLOCALEDIR=\"$(localedir)\"

LDADD = ../find/version.o ../lib/libfind.a ../gnulib/lib/libgnulib.a @INTLLIBS@

$(PROGRAMS) $(LIBPROGRAMS): ../find/version.o ../lib/libfind.a ../gnulib/lib/libgnulib.a

updatedb: updatedb.sh
	rm -f $@
	find=`echo find|sed '$(transform)'`; \
	frcode=`echo frcode|sed '$(transform)'`; \
	bigram=`echo bigram|sed '$(transform)'`; \
	code=`echo code|sed '$(transform)'`; \
	sed \
	-e "s,@""bindir""@,$(bindir)," \
	-e "s,@""libexecdir""@,$(libexecdir)," \
	-e "s,@""LOCATE_DB""@,$(LOCATE_DB)," \
	-e "s,@""VERSION""@,@VERSION@," \
	-e "s,@""find""@,$${find}," \
	-e "s,@""frcode""@,$${frcode}," \
	-e "s,@""bigram""@,$${bigram}," \
	-e "s,@""code""@,$${code}," \
	-e "s,@""SORT""@,$(SORT)," \
	-e "s,@""SORT_SUPPORTS_Z""@,$(SORT_SUPPORTS_Z)," \
	$(srcdir)/updatedb.sh > $@
	chmod +x $@

install-data-hook:
	$(top_srcdir)/mkinstalldirs $(DESTDIR)$(localstatedir)

SUBDIRS = testsuite
