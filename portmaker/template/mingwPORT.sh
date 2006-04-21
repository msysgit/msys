#! /bin/sh
# mingwPORT.sh
# Required.  Used to execute commands to build the ported package.

CURDIR=`pwd`
mingwPORT="${CURDIR}/mingwPORT"

# Load mingwPORT function library and message catalogue,
# and parse any options specified on the command line.

. "$mingwPORT.functions"
. "$mingwPORT.messages"
. "$mingwPORT.getopts"

# Initialise the target package configuration.

pref "$mingwPORT.ini"

# Apply any `mingwPORT.site' customisations, as specified
# in any of the standard site configuration directories.

pref "/etc/mingwPORT/mingwPORT.site"
pref "/usr/lib/mingwPORT/mingwPORT.site"
pref "/usr/local/lib/mingwPORT/mingwPORT.site"
pref "$HOME/.mingwPORT/mingwPORT.site"

perform action question

cd ${CURDIR}

pref "$mingwPORT.exports"

[ -z "$SRCDIR" ] && SRCDIR=`pwd` || eval SRCDIR=\"$SRCDIR\"
ABSSRCDIR=`cd $SRCDIR >/dev/null 2>&1; pwd`

if [ -f $mingwPORT.patch ]
then
  pref $mingwPORT.beforepatch

  eval sed \"${PATCHFILTER-"s/x/x/"}\" $mingwPORT.patch \
  | patch -t -N $PATCHFLAGS 

  pref $mingwPORT.afterpatch
fi

[ -z "$BUILDDIR" ] && BUILDDIR=bld

if [ ! -d ${BUILDDIR} ]
then
  mkdir ${BUILDDIR}
  RMDIR=${BUILDDIR}
else
  RMDIR='NORMDIR'
fi

cd ${BUILDDIR}
ABSBUILDDIR=`pwd`

require action configure
require action make
require action install

cd ${CURDIR}

eval ${CLEANUP_ON_EXIT}

# $RCSfile: mingwPORT.sh,v $: end of file: vim: ft=sh
