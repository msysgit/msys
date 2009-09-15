# Process this file with autoconf 2.63 or better to produce a configure script.
AC_PREREQ(2.63)

AC_INIT(mingw-utils,0.4)
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([1.11 silent-rules no-define no-dist-gzip dist-lzma])
AM_MAINTAINER_MODE

# Subpackages
AC_CONFIG_SUBDIRS([msys-here])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_AWK
AC_PROG_RANLIB

AC_PROG_LEX
AC_PROG_YACC

# Checks for header files.
AC_HEADER_STDC
where_toGet_utime_h='not found!'
# this part is scrambled by autoconf. phooey. so we just cannot tell the user what we are doing.
# AC_MSG_CHECKING([for whether utime.h is found as "utime.h" or "sys/utime.h"])
AC_CHECK_HEADER(utime.h,
	[AC_DEFINE(HAVE_UTIME_H,1, ["utime.h" is present instead of "sys/utime.h"])]
	where_toGet_utime_h='utime.h',
	[AC_CHECK_HEADER(sys/utime.h,
		[AC_DEFINE(HAVE_SYS_UTIME_H,1, ["sys/utime.h" is present instead of "utime.h"])]
		where_toGet_utime_h='sys/utime.h',
		[AC_MSG_WARN(Could find neither "utime.h" nor "sys/utime.h")]
	)]
)
# AC_MSG_RESULT($where_toGet_utime_h)

# Checks for library functions.
AC_CHECK_FUNCS(mkstemp mktemp, break)
AC_CHECK_FUNCS(utime)

AC_CONFIG_FILES([
Makefile
redir/Makefile
reimp/Makefile
res2coff/Makefile
scripts/Makefile
])
AC_CONFIG_FILES([scripts/a2dll], [chmod +x scripts/a2dll])
AC_CONFIG_FILES([scripts/dsw2mak], [chmod +x scripts/dsw2mak])

AC_OUTPUT
