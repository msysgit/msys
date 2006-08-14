## Process this file with automake to produce Makefile.in.
EXTRA_DIST =
EXTRA_DIST += Makefile.am.in
EXTRA_DIST += alloca.m4
EXTRA_DIST += c-bs-a.m4
EXTRA_DIST += c-stack.m4
EXTRA_DIST += check-decl.m4
EXTRA_DIST += codeset.m4
EXTRA_DIST += dirname.m4
EXTRA_DIST += dos.m4
EXTRA_DIST += error.m4
EXTRA_DIST += exclude.m4
EXTRA_DIST += exitfail.m4
EXTRA_DIST += extensions.m4
EXTRA_DIST += file-type.m4
EXTRA_DIST += fnmatch.m4
EXTRA_DIST += getopt.m4
EXTRA_DIST += gettext.m4
EXTRA_DIST += gettimeofday.m4
EXTRA_DIST += glibc21.m4
EXTRA_DIST += gnu-source.m4
EXTRA_DIST += gnulib.m4
EXTRA_DIST += hard-locale.m4
EXTRA_DIST += iconv.m4
EXTRA_DIST += intdiv0.m4
EXTRA_DIST += intmax.m4
EXTRA_DIST += intmax_t.m4
EXTRA_DIST += inttostr.m4
EXTRA_DIST += inttypes-pri.m4
EXTRA_DIST += inttypes.m4
EXTRA_DIST += inttypes_h.m4
EXTRA_DIST += isc-posix.m4
EXTRA_DIST += jm-glibc-io.m4
EXTRA_DIST += lcmessage.m4
EXTRA_DIST += lib-ld.m4
EXTRA_DIST += lib-link.m4
EXTRA_DIST += lib-prefix.m4
EXTRA_DIST += longdouble.m4
EXTRA_DIST += longlong.m4
EXTRA_DIST += malloc.m4
EXTRA_DIST += mbrtowc.m4
EXTRA_DIST += mbstate_t.m4
EXTRA_DIST += mkstemp.m4
EXTRA_DIST += nls.m4
EXTRA_DIST += onceonly.m4
EXTRA_DIST += po.m4
EXTRA_DIST += posixver.m4
EXTRA_DIST += prereq.m4
EXTRA_DIST += printf-posix.m4
EXTRA_DIST += progtest.m4
EXTRA_DIST += realloc.m4
EXTRA_DIST += regex.m4
EXTRA_DIST += restrict.m4
EXTRA_DIST += setmode.m4
EXTRA_DIST += signed.m4
EXTRA_DIST += size_max.m4
EXTRA_DIST += st_mtim.m4
EXTRA_DIST += stdbool.m4
EXTRA_DIST += stdint_h.m4
EXTRA_DIST += strcase.m4
EXTRA_DIST += strerror_r.m4
EXTRA_DIST += strftime.m4
EXTRA_DIST += strtoimax.m4
EXTRA_DIST += strtol.m4
EXTRA_DIST += strtoll.m4
EXTRA_DIST += strtoul.m4
EXTRA_DIST += strtoull.m4
EXTRA_DIST += strtoumax.m4
EXTRA_DIST += time_r.m4
EXTRA_DIST += tm_gmtoff.m4
EXTRA_DIST += tzset.m4
EXTRA_DIST += uintmax_t.m4
EXTRA_DIST += ulonglong.m4
EXTRA_DIST += unlocked-io.m4
EXTRA_DIST += vararrays.m4
EXTRA_DIST += wchar_t.m4
EXTRA_DIST += wint_t.m4
EXTRA_DIST += xalloc.m4
EXTRA_DIST += xsize.m4
EXTRA_DIST += xstrtol.m4
EXTRA_DIST += xstrtoumax.m4

Makefile.am: Makefile.am.in
	rm -f $@ $@t
	(echo "## Process this file with automake to produce Makefile.in."; \
	 echo EXTRA_DIST =;				\
	 echo "EXTRA_DIST += Makefile.am.in";		\
	 ls -1 *.m4 | sed 's/^/EXTRA_DIST += /'; echo )	\
	  >> $@t
	sed -n '/^Makefile\.am:/,$$p' $@.in >> $@t
	chmod a-w $@t
	mv $@t $@
