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

# Set indicator to show we are "on track" for successful completion;
# (failing operations may reset it, either directly or by raising SIGUSR1,
#  to suppress further pointless processing, and provide a graceful exit).

ONTRACK=true
trap "ONTRACK=false" SIGUSR1

# Get user's configuration preferences.

perform action question

cd ${CURDIR}
$ONTRACK && pref "$mingwPORT.exports"

if $ONTRACK && [ -f $mingwPORT.patch ]
then
  pref $mingwPORT.beforepatch

  PATCHFLAGS=${PATCHFLAGS-"-p0"}
  PATCHFILTER=${PATCHFILTER-"s,x,x,"}
  sed ${PATCHFILTER} $mingwPORT.patch | ( cd "${SRCDIR}" &&
    patch -t -N $PATCHFLAGS )

  pref $mingwPORT.afterpatch
fi

BUILDDIR=`abspath "${BUILDDIR:-bld}"`

if [ ! -d ${BUILDDIR} ]
then
  mkdir -p ${BUILDDIR} && RMDIR=`echo $RMDIR; pathenc "${BUILDDIR}"`
fi

cd ${BUILDDIR}

$ONTRACK && require action configure
$ONTRACK && require action make
$ONTRACK && require action install

cd ${CURDIR}

eval ${CLEANUP_ON_EXIT}

# $RCSfile: mingwPORT.sh,v $: end of file: vim: ft=sh
