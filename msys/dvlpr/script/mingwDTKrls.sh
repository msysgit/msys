#! /bin/sh
#############################################################################
# mingwDTKrls.sh - Create an mingwDTK release                          	    #
# Copyright (C) 2002  Earnie Boyd  <earnie@users.sf.net>                    #
#                                                                           #
# This file is part of mingwDVLPR                                           #
#   http://www.mingw.org/mingwDVLPR.shtml                                   #
#                                                                           #
#############################################################################

#FIXME: Need to use getopts to control PRODUCTION release, SNAPDATE, SUBVERSION,
#       etc.

# User changeable values section.
HOST=mingw32
PACKAGE=mingwDTK
MAJORVER=1
MINORVER=0
PATCHVER=0
STOREROOT=/store/${HOST}
RLSROOT=/release/${HOST}/${PACKAGE}
SNAPDATE=\-`date +%Y.%m.%d`
#SNAPDATE=\-rc
#SNAPDATE=
SUBVERSION=\-1
#SUBVERSION=
#END User changeable values section.

VERSION=${MAJORVER}.${MINORVER}.${PATCHVER}
SHORTVER=${MAJORVER}.${MINORVER}
RLSDEPOT=/depot/binary/${PACKAGE}/${SHORTVER}
RLSOUTPUTDIR="`p2w ${RLSROOT}/${VERSION}`"

if [ -z "$SNAPDATE" ]
then
    RELEASE="Production Release${SUBVERSION}"
elif [ "x${SNAPDATE}" == "x-rc" ]
then
    RELEASE="Release Candidate${SUBVERSION}"
else
    RELEASE="Snapshot${SNAPDATE}${SUBVERSION}"
fi

istore=${STOREROOT}/pkg
noarchstore=${STOREROOT}/noarch
datastore=${STOREROOT}/dtk

INFOBEFOREFILE="`p2w ${RLSDEPOT}/doc/mingw/${PACKAGE}-${VERSION}-changes.rtf`"
INFOAFTERFILE="`p2w ${RLSDEPOT}/doc/mingw/mingwDTK.rtf`"
LICENSEFILE="`p2w ${RLSDEPOT}/doc/mingw/MINGW_LICENSE.rtf`"
RLSSOURCEDIR="`p2w $RLSDEPOT`"

bin_LIST="`cat ${datastore}/exe.dat`"
etc_LIST="`cat ${datastore}/etc.dat`"
include_LIST="`cat ${datastore}/include.dat`"
lib_LIST="`cat ${datastore}/lib.dat`"
share_LIST="`cat ${datastore}/share.dat`"
doc_LIST="`cat ${datastore}/doc.dat` ${PACKAGE}-${VERSION}-changes.rtf"

if [ ! -d ${RLSDEPOT} ]
then
  mkdir -p ${RLSDEPOT}
fi

echo Removing old depot
rm -rf ${RLSDEPOT}/*

echo Creating needed depot directories
DIRLIST="bin doc/mingw etc include lib share"

for DIR in $DIRLIST
do
    if [ ! -d ${RLSDEPOT}/${DIR} ]
    then
	mkdir -p ${RLSDEPOT}/${DIR}
    fi
done

if [ ! -f ${RLSOUTPUTDIR} ]
then
  mkdir -p ${RLSOUTPUTDIR}
fi

echo Registering store items to the depot
for I in ${exe_LIST}
do
  cp ${istore}/bin/${I} ${RLSDEPOT}/bin/
done

for I in ${dll_LIST}
do
  cp ${istore}/bin/${I} ${RLSDEPOT}/bin/
done

for I in ${lib_LIST}
do
  cp -a ${istore}/lib/${I} ${RLSDEPOT}/lib/
done

for I in ${share_LIST}
do
  cp -a ${istore}/share/${I} ${RLSDEPOT}/share/
done

for I in ${doc_LIST}
do
  cat ${noarchstore}/doc/mingw/${I} | sed -c -e "s/@VERSION@/$VERSION/g" -e "s/@RELEASE@/$RELEASE/g" > ${RLSDEPOT}/doc/mingw/$I
done

echo Creating INNO script.
cat ${noarchstore}/mingwDTK.iss.in | \
  sed -c \
      -e "s/@PACKAGE@/$PACKAGE/g" \
      -e "s/@VERSION@/$VERSION/g" \
      -e "s/@ARC@/$ARC/g" \
      -e "s/@SNAPDATE@/$SNAPDATE/g" \
      -e "s/@SUBVERSION@/$SUBVERSION/g" \
      -e "s%@LICENSEFILE@%${LICENSEFILE}%g" \
      -e "s%@INFOBEFOREFILE@%${INFOBEFOREFILE}%g" \
      -e "s%@INFOAFTERFILE@%${INFOAFTERFILE}%g" \
      -e "s%@RLSSOURCEDIR@%${RLSSOURCEDIR}%g" \
      -e "s%@RLSOUTPUTDIR@%${RLSOUTPUTDIR}%g" \
  > /tmp/mingwDTK$$.iss

echo Executing INNO script.
/c/InnoSetup2/iscc "/tmp/mingwDTK$$.iss"

echo Removing INNO script.
rm -f /tmp/mingwDTK$$.iss
