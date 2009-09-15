# Process this file with autoconf 2.53 or better to produce a configure script.
AC_PREREQ(2.63)

AC_INIT(msys-here,0.2)
AC_CONFIG_SRCDIR([Makefile.am])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([1.11 silent-rules foreign no-define no-dist-gzip dist-lzma])
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

AC_ARG_VAR(WINDRES, [Windows Resource compiler tool path])
AC_PATH_TOOL(WINDRES,windres,
	[AC_MSG_WARN(Could not find a windres tool in your PATH. Will not be able to compile msys-here.)]
)
AC_SUBST(WINDRES)

sub_docdir=
AC_MSG_CHECKING([if docs should be installed in a subdirectory])
AC_ARG_ENABLE([subproject-docdir],
	AC_HELP_STRING([--enable-subproject-docdir[=SUBDIR]],
		[Install documents underneath $docdir/$arg]),
        [], enable_subproject_docdir=no)
if test "x$enable_subproject_docdir" = xyes; then
        AC_MSG_RESULT([msys-here])
	sub_docdir=/msys-here
elif test "x$enable_subproject_docdir" = xno; then
        AC_MSG_RESULT([no])
elif test "x$enable_subproject_docdir" = x; then
        AC_MSG_RESULT([no])
else
        AC_MSG_RESULT([$enable_subproject_docdir])
	sub_docdir="/$enable_subproject_docdir"
fi

docdir=${docdir}${sub_docdir}

AC_CONFIG_FILES([
Makefile
lib/Makefile
src/Makefile
winui/Makefile
])
AC_CONFIG_FILES([setup/msys-here-config], [chmod +x setup/msys-here-config])

AC_OUTPUT
