#! /bin/sh
#############################################################################
# mingwrls.sh - Create an MinGW release                           .	    #
# Copyright (C) 2002  Earnie Boyd  <earnie@users.sf.net>                    #
#                                                                           #
# This file is part of msysDVLPR                                            #
#   http://www.mingw.org/msysDVLPR.shtml                                    #
#                                                                           #
#############################################################################

#FIXME: Need to use getopts to control PRODUCTION release, SNAPDATE, SUBVERSION,
#       etc.

# User changeable values section.
HOST=mingw32
PACKAGE=MinGW
MAJORVER=2
MINORVER=0
PATCHVER=0
STOREROOT=/store/${HOST}
RLSROOT=/release/${HOST}/${PACKAGE}
#SNAPDATE=\-`date +%Y.%m.%d`
#SNAPDATE=\-rc
SNAPDATE=
if [ x${SUBVERSION} == x ]
then
  SUBVERSION=\-1
fi
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
datastore=${STOREROOT}/var

INFOBEFOREFILE="`p2w ${RLSDEPOT}/doc/mingw/MinGW_PACKAGES.rtf`"
INFOAFTERFILE="`p2w ${RLSDEPOT}/doc/mingw/MinGW_WELCOME.rtf`"
LICENSEFILE="`p2w ${RLSDEPOT}/doc/mingw/MinGW_LICENSE.rtf`"
RLSSOURCEDIR="`p2w $RLSDEPOT`"

exe_LIST="`cat ${datastore}/exe.dat`"
lib_LIST="`cat ${datastore}/lib.dat`"
include_LIST="`cat ${datastore}/include.dat`"
doc_LIST="`cat ${datastore}/doc.dat`"
mingw32_LIST="`cat ${datastore}/mingw32.dat`"

if [ ! -d ${RLSDEPOT} ]
then
  mkdir -p ${RLSDEPOT}
fi

echo Removing old depot
rm -rf ${RLSDEPOT}/*

echo Creating needed depot directories
if [ ! -d ${RLSDEPOT}/bin ]
then
  mkdir -p ${RLSDEPOT}/bin
fi

if [ ! -d ${RLSDEPOT}/include ]
then
  mkdir -p ${RLSDEPOT}/include
fi

if [ ! -d ${RLSDEPOT}/lib ]
then
  mkdir -p ${RLSDEPOT}/lib
fi

if [ ! -d ${RLSDEPOT}/mingw32 ]
then
  mkdir -p ${RLSDEPOT}/mingw32
fi

if [ ! -d ${RLSDEPOT}/doc/MinGW ]
then
  mkdir -p ${RLSDEPOT}/doc/MinGW
fi

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

subdir=""
for I in ${lib_LIST}
do
  if [ -d ${istore}/lib/${I} ]
  then
    subdir=${I}
    if [ ! -d ${RLSDEPOT}/lib/${subdir} ]
    then
      mkdir -p ${RLSDEPOT}/lib/${subdir}
    fi
  else
    cp -a ${istore}/lib/${subdir}/${I} ${RLSDEPOT}/lib/${subdir}
  fi
done

subdir=""
for I in ${include_LIST}
do
  if [ -d ${istore}/include/${I} ]
  then
    subdir=${I}
    if [ ! -d ${RLSDEPOT}/include/${subdir} ]
    then
      mkdir -p ${RLSDEPOT}/include/${subdir}
    fi
  else
    cp -a ${istore}/include/${subdir}/${I} ${RLSDEPOT}/include/${subdir}
  fi
done

subdir=""
for I in ${mingw32_LIST}
do
  if [ -d ${istore}/mingw32/${I} ]
  then
    subdir=${I}
    if [ ! -d ${RLSDEPOT}/mingw32/${subdir} ]
    then
      mkdir -p ${RLSDEPOT}/mingw32/${subdir}
    fi
  else
    cp -a ${istore}/mingw32/${subdir}/${I} ${RLSDEPOT}/mingw32/${subdir}
  fi
done

for I in ${doc_LIST}
do
  cat ${noarchstore}/doc/MinGW/${I} | sed -c -e "s/@VERSION@/$VERSION/g" -e "s/@RELEASE@/$RELEASE/g" > ${RLSDEPOT}/doc/MinGW/$I
done

echo Creating INNO script.
cat ${noarchstore}/mingw.iss.in | \
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
  > /tmp/MinGW$$.iss

echo Executing INNO script.
/c/InnoSetup2/iscc "/tmp/MinGW$$.iss"

echo Removing INNO script.
rm -f /tmp/MinGW$$.iss
