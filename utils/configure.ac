# Process this file with autoconf 2.53 or better to produce a configure script.

AC_INIT(mingw-utils,0.2)
AC_PREREQ(2.53)
AM_INIT_AUTOMAKE(1.6 no-define)
AC_CONFIG_SRCDIR(Makefile.am)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_AWK
AC_PROG_RANLIB

AM_PROG_LEX
AC_PROG_YACC

AC_ARG_VAR(WINDRES, [Windows Resource compiler tool path])
AC_PATH_TOOL(WINDRES,windres,
	[AC_MSG_WARN(Could not find a windres tool in your PATH. Will not be able to compile drmingw.)]
)
AC_SUBST(WINDRES)

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

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS(mkstemp mktemp, break)
AC_FUNC_UTIME_NULL
AC_CHECK_FUNCS(utime)

AC_CONFIG_FILES([
Makefile
dos2unix/Makefile
drmingw/Makefile
pexports/Makefile
redir/Makefile
reimp/Makefile
res2coff/Makefile
scripts/Makefile
scripts/a2dll
scripts/dsw2mak
unix2dos/Makefile
])
AC_OUTPUT
