# aclocal.m4 for MinGW portmaker package.
# Created: 2006-06-16 by Keith Marshall <keithmarshall@users.sf.net>

# Interpret version.m4, to set up AC_INIT parameters.
#
m4_include([version.m4])
m4_define([PM_PACKAGE_VERSION],[PM_VERSION_MAJOR][.][PM_VERSION_MINOR]dnl
m4_ifval(PM_VERSION_PATCH,[.][PM_VERSION_PATCH]))

# $RCSfile: aclocal.m4,v $: end of file
