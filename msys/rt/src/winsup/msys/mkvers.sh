#!/bin/sh
# mkvers.sh - Make version information for cygwin DLL
#
#   Copyright 1998, 1999, 2000, 2001 Red Hat, Inc.
#
# This file is part of Cygwin.
#
# This software is a copyrighted work licensed under the terms of the
# Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
# details.

exec 9> version.cc

#
# Arg 1 is the name of the version include file
#
incfile="$1"
rcfile="$2"
windres="$3"

[ -r $incfile ] || {
    echo "**** Couldn't open file '$incfile'.  Aborting."
}

#
# Load the current date so we can work on individual fields
#
build_date=`date`
set -$- $build_date
#
# Translate the month into a number
#
case "$2" in
    Jan) m=01 ;;
    Feb) m=02 ;;
    Mar) m=03 ;;
    Apr) m=04 ;;
    May) m=05 ;;
    Jun) m=06 ;;
    Jul) m=07 ;;
    Aug) m=08 ;;
    Sep) m=09 ;;
    Oct) m=10 ;;
    Nov) m=11 ;;
    Dec) m=12 ;;
esac

if [ "$3" -le 10 ]; then
    d=0$3
else
    d=$3
fi
hhmm="`echo $4 | sed 's/:..$//'`"
#
# Set date into YYYY-MM-DD HH:MM:SS format
#
builddate="${6-$5}-$m-$d $hhmm"

set -$- ''

#
# Output the initial part of version.cc
#
cat <<EOF 1>&9
#include "config.h"
#include "cygwin_version.h"

#define strval(x) #x
#define str(x) strval(x)
#define shared_data_version str(SHARED_DATA_VERSION)

const char *cygwin_version_strings =
  "BEGIN_CYGWIN_VERSION_INFO\n"
EOF

#
# Split version file into dir and filename components
#
dir=`dirname $incfile`
fn=`basename $incfile`

#
# Look in the include file CVS directory for a CVS Tag file.  This file,
# if it exists, will contain the name of the sticky tag associated with
# the current build.  Save that for output later.
#
cvs_tag="`sed 's%^.\(.*\)%\1%' $dir/CVS/Tag 2>/dev/null`"

wv_cvs_tag="$cvs_tag"
[ -n "$cvs_tag" ] && cvs_tag=" CVS tag"'
'"$cvs_tag"

#
# Look in the source directory containing the include/msys/version.h
# and set dir accordingly.
dir=`echo $dir | sed -e 's%/include/cygwin.*$%%' -e 's%include/cygwin.*$%.%'`

# Look in $dir for a a ".snapshot-date" file.  If one is found then this
# information will be saved for output to the DLL.
#
if [ -r "$dir/.snapshot-date" ]; then
    read snapshotdate < "$dir/.snapshot-date"
    snapshot="snapshot date
$snapshotdate"
fi

#
# Scan the version.h file for strings that begin with CYGWIN_INFO or
# DLL_VERSION.  Perform crude parsing on the lines to get the values
# associated with these values and then pipe it into a while loop which
# outputs these values in C palatable format for inclusion in the DLL
# with a '%% ' identifier that will introduce "interesting" strings.
# These strings are strictly for use by a user to scan the DLL for
# interesting information.
#
(sed -n -e 's%#define CYGWIN_INFO_\([A-Z_]*\)[ 	][ 	]*\([a-zA-Z0-9"][^/]*\).*%_\1\
\2%p' -e 's%#define DLL_VERSION_\([A-Z_]*\)[ 	][ 	]*\([a-zA-Z0-9"][^/]*\).*%DLL \1\
\2%p' -e 's%#define API_VERSION_\([A-Z_]*\)[ 	][ 	]*\([a-zA-Z0-9"][^/]*\).*%API \1\
\2%p' $incfile | sed -e 's/["\\]//g'  -e '/^_/y/ABCDEFGHIJKLMNOPQRSTUVWXYZ_/abcdefghijklmnopqrstuvwxyz /';
echo ' build date'; echo $build_date; [ -n "$cvs_tag" ] && echo "$cvs_tag";\
[ -n "$snapshot" ] && echo "$snapshot"
) | while read var; do
    read val
cat <<EOF
  "%%% MSYS $var: $val\n"
EOF
done | tee /tmp/mkvers.$$ 1>&9

trap "rm -f /tmp/mkvers.$$" 0 1 2 15

if [ -n "$snapshotdate" ]; then
  usedate="`echo $snapshotdate | sed 's/-\\(..:..[^-]*\\).*$/ \1SNP/'`"
else
  usedate="$builddate"
fi

#
# Finally, output the shared ID and set up the cygwin_version structure
# for use by Cygwin itself.
#
cat <<EOF 1>&9
#ifdef DEBUGGING
  "%%% MSYS shared id: " DLL_VERSION_IDENTIFIER "S" shared_data_version "-$builddate\n"
#else
  "%%% MSYS shared id: " DLL_VERSION_IDENTIFIER "S" shared_data_version "\n"
#endif
  "END_CYGWIN_VERSION_INFO\n\0";
cygwin_version_info cygwin_version =
{
  API_VERSION_MAJOR, API_VERSION_MINOR,
  DLL_VERSION_MAJOR, DLL_VERSION_MINOR,
  SHARED_DATA_VERSION,
  CYGWIN_VERSION_MOUNT_REGISTRY,
  "$usedate",
#ifdef DEBUGGING
  DLL_VERSION_IDENTIFIER "S" shared_data_version "-$builddate"
#else
  DLL_VERSION_IDENTIFIER "S" shared_data_version
#endif
};
EOF

#
# Generate winver.o using msys/version.h information.
# Turn the cygwin major number from some large number to something like 1.1.0.
#
eval `sed -n 's/^.*DLL \(M[AI][JN]OR\): \([0-9]*\)[^0-9]*$/\1=\2/p' /tmp/mkvers.$$`
cygverhigh=`expr $MAJOR / 1000`
cygverlow=`expr $MAJOR % 1000`
cygwin_ver="$cygverhigh.$cygverlow.$MINOR"
if [ -n "$cvs_tag" ]; then
    cvs_tag="`echo $wv_cvs_tag | sed -e 's/-branch.*//'`"
    cygwin_ver="$cygwin_ver-$cvs_tag"
fi

echo "Version $cygwin_ver"
set -$- $builddate
$windres --include-dir $dir/../w32api/include --include-dir $dir/include --define DLL_BUILD_DATE="$1" --define DLL_BUILD_TIME="$2" --define MSYS_VERSION='"'"$cygwin_ver"'"' $rcfile winver.o
