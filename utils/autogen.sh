#! /bin/bash -e

export WANT_AUTOMAKE=1.11
usage="Usage: autogen.sh [-c]"
opt_c=
while getopts ":c" options; do
  case $options in
    c  ) opt_c=-c;;
    \? ) echo $usage; exit 0;;
    *  ) echo $usage 1>&2; exit 1;;
  esac
done
shift $(($OPTIND - 1))

set -x
aclocal
autoheader
automake --gnu --add-missing
autoconf

(cd msys-here && ./autogen.sh)

if [ "x${opt_c}" != "x" ]
then
  ./configure --host=mingw32 --prefix=/mingw \
	--enable-maintainer-mode --enable-silent-rules "$@"
fi

