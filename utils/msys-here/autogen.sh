#! /bin/sh

export WANT_AUTOMAKE_1_6=1

aclocal \
&& autoheader \
&& automake --gnu --add-missing \
&& autoconf \
&& ./configure --build=mingw32 --prefix=/mingw --enable-maintainer-mode

