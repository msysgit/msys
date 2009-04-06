#! /bin/sh
#############################################################################
# msysrls.sh - Create an MSYS release                             .	    #
# Copyright (C) 2002  Earnie Boyd  <earnie@users.sf.net>                    #
#                                                                           #
# This file is part of msysDVLPR                                            #
#   http://www.mingw.org/msysDVLPR.shtml                                    #
#                                                                           #
#############################################################################

#FIXME: Need to use getopts to control PRODUCTION release, SNAPDATE, SUBVERSION,
#       etc.

# User changeable values section.
HOST=msys
PACKAGE=msysCORE
MAJORVER=1
MINORVER=0
PATCHVER=11
STOREROOT=${PWD}/store
RLSROOT=${PWD}/release
DEPOTROOT=${PWD}/depot
SNAPDATE=\-`date +%Y%m%d`
#SNAPDATE=\-rc
#SNAPDATE=
SUBVERSION=\-1
#SUBVERSION=

if [ -f /etc/msysrlsbld.pref ]
then
  source /etc/msysrlsbld.pref
fi

if [ -f ${HOME}/msysrlsbld.pref ]
then
  source ${HOME}/msysrlsbld.pref
fi

#END User changeable values section.

VERSION=${MAJORVER}.${MINORVER}.${PATCHVER}
SHORTVER=${MAJORVER}.${MINORVER}
RLSDEPOT=${DEPOTROOT}/binary/${PACKAGE}/${SHORTVER}
RLSOUTPUTDIR="${RLSROOT}/${VERSION}"
PACKAGE_NAME=${PACKAGE}-${VERSION}${SNAPDATE}${SUBVERSION}.tar.gz

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
miscstore=${STOREROOT}/misc
datastore=${STOREROOT}/var

exe_LIST="`cat ${datastore}/exe.dat`"
etc_LIST="`cat ${datastore}/etc.dat`"
dll_LIST="`cat ${datastore}/dll.dat`"
doc_LIST="`cat ${datastore}/doc.dat` MSYS-${VERSION}-changes.rtf"
misc_LIST="`cat ${datastore}/misc.dat`"
pi_LIST="`cat ${datastore}/pi.dat`"
script_LIST="`cat ${datastore}/script.dat`"
share_LIST="`cat ${datastore}/share.dat`"

if [ ! -d ${RLSDEPOT} ]
then
  mkdir -p ${RLSDEPOT}
fi

rm -rf ${RLSDEPOT}/*

if [ ! -d ${RLSDEPOT}/bin ]
then
  mkdir ${RLSDEPOT}/bin
fi

for I in ${exe_LIST}
do
  cp ${istore}/bin/${I} ${RLSDEPOT}/bin/
done

for I in ${dll_LIST}
do
  cp ${istore}/bin/${I} ${RLSDEPOT}/bin/
done

if [ ! -d ${RLSDEPOT}/share ]
then
  mkdir ${RLSDEPOT}/share
fi

for I in ${share_LIST}
do
  cp -r ${istore}/share/${I} ${RLSDEPOT}/share/
done

if [ ! -d ${RLSDEPOT}/doc/msys ]
then
  mkdir -p ${RLSDEPOT}/doc/msys
fi

for I in ${doc_LIST}
do
  cat ${noarchstore}/doc/msys/${I} | sed -c -e "s/@VERSION@/$VERSION/g" -e "s/@RELEASE@/$RELEASE/g" > ${RLSDEPOT}/doc/msys/$I
done

if [ ! -d ${RLSDEPOT}/etc ]
then
  mkdir ${RLSDEPOT}/etc
fi

for I in ${etc_LIST}
do
  cp ${noarchstore}/etc/${I} ${RLSDEPOT}/etc/
done

for I in ${script_LIST}
do
  cp ${noarchstore}/bin/${I} ${RLSDEPOT}/bin/
done

for I in ${misc_LIST}
do
  case $I in
  msys.bat)
    cp ${noarchstore}/bin/${I} ${RLSDEPOT}
    ;;
  msys.ico | m.ico)
    cp ${noarchstore}/${I} ${RLSDEPOT}
    ;;
  esac
done

if [ ! -d ${RLSDEPOT}/postinstall ]
then
  mkdir ${RLSDEPOT}/postinstall
fi

for I in ${pi_LIST}
do
  cp ${noarchstore}/pi/${I} ${RLSDEPOT}/postinstall/
done

if [ ! -f ${RLSOUTPUTDIR} ]
then
  mkdir -p ${RLSOUTPUTDIR}
fi

(cd ${RLSDEPOT} && tar -zcf ${RLSOUTPUTDIR}/${PACKAGE_NAME} *)
