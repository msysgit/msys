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
PACKAGE=msys
MAJORVER=1
MINORVER=0
PATCHVER=8
STOREROOT=/prjz/msys/nstl
RLSROOT=/prjz/rls
ARC=$1
SNAPDATE=\-`date +%Y.%m.%d`
#SNAPDATE=\-rc
#SNAPDATE=
SUBVERSION=\-1
#SUBVERSION=
#END User changeable values section.

VERSION=${MAJORVER}.${MINORVER}.${PATCHVER}
SHORTVER=${MAJORVER}.${MINORVER}
RLSOUTPUTDIR="`p2w ${RLSROOT}/${PACKAGE}/${VERSION}`"

if [ -z "$SNAPDATE" ]
then
    RELEASE="Production Release${SUBVERSION}"
elif [ "x${SNAPDATE}" == "x-rc" ]
then
    RELEASE="Release Candidate${SUBVERSION}"
else
    RELEASE="Snapshot${SNAPDATE}${SUBVERSION}"
fi

istore=${STOREROOT}/${ARC}
noarchstore=${STOREROOT}/noarch
miscstore=${STOREROOT}/misc
datastore=${STOREROOT}/var

rlsdepot=${RLSROOT}/${PACKAGE}/depot/${PACKAGE}/${SHORTVER}
INFOBEFOREFILE="`p2w ${rlsdepot}/doc/msys/MSYS-${VERSION}-changes.rtf`"
INFOAFTERFILE="`p2w ${rlsdepot}/doc/msys//MSYS_WELCOME.rtf`"
LICENSEFILE="`p2w ${rlsdepot}/doc/msys/MSYS_LICENSE.rtf`"
RLSSOURCEDIR="`p2w $rlsdepot`"

exe_LIST="`cat ${datastore}/exe.dat`"
etc_LIST="`cat ${datastore}/etc.dat`"
dll_LIST="`cat ${datastore}/dll.dat`"
doc_LIST="`cat ${datastore}/doc.dat` MSYS-${VERSION}-changes.rtf"
misc_LIST="`cat ${datastore}/misc.dat`"
pi_LIST="`cat ${datastore}/pi.dat`"
script_LIST="`cat ${datastore}/script.dat`"

if [ ! -d ${rlsdepot} ]
then
  mkdir -p ${rlsdepot}
fi

rm -rf ${rlsdepot}/*

if [ ! -d ${rlsdepot}/bin ]
then
  mkdir ${rlsdepot}/bin
fi

for I in ${exe_LIST}
do
  cp ${istore}/bin/${I} ${rlsdepot}/bin/
done

for I in ${dll_LIST}
do
  cp ${istore}/bin/${I} ${rlsdepot}/bin/
done

if [ ! -d ${rlsdepot}/doc/msys ]
then
  mkdir -p ${rlsdepot}/doc/msys
fi

for I in ${doc_LIST}
do
  cat ${noarchstore}/doc/msys/${I} | sed -c -e "s/@VERSION@/$VERSION/g" -e "s/@RELEASE@/$RELEASE/g" > ${rlsdepot}/doc/msys/$I
done

if [ ! -d ${rlsdepot}/etc ]
then
  mkdir ${rlsdepot}/etc
fi

for I in ${etc_LIST}
do
  cp ${noarchstore}/etc/${I} ${rlsdepot}/etc/
done

for I in ${script_LIST}
do
  cp ${noarchstore}/bin/${I} ${rlsdepot}/bin/
done

for I in ${misc_LIST}
do
  case $I in
  msys.bat)
    cp ${noarchstore}/bin/${I} ${rlsdepot}
    ;;
  msys.ico | m.ico)
    cp ${noarchstore}/${I} ${rlsdepot}
    ;;
  esac
done

if [ ! -d ${rlsdepot}/postinstall ]
then
  mkdir ${rlsdepot}/postinstall
fi

for I in ${pi_LIST}
do
  cp ${noarchstore}/pi/${I} ${rlsdepot}/postinstall/
done

if [ ! -f ${RLSOUTPUTDIR} ]
then
  mkdir -p ${RLSOUTPUTDIR}
fi

cat msys.iss.in | \
  sed -c \
      -e "s/@VERSION@/$VERSION/g" \
      -e "s/@ARC@/$ARC/g" \
      -e "s/@SNAPDATE@/$SNAPDATE/g" \
      -e "s/@SUBVERSION@/$SUBVERSION/g" \
      -e "s%@LICENSEFILE@%${LICENSEFILE}%g" \
      -e "s%@INFOBEFOREFILE@%${INFOBEFOREFILE}%g" \
      -e "s%@INFOAFTERFILE@%${INFOAFTERFILE}%g" \
      -e "s%@RLSSOURCEDIR@%${RLSSOURCEDIR}%g" \
      -e "s%@RLSOUTPUTDIR@%${RLSOUTPUTDIR}%g" \
  > msys.iss

/c/InnoSetup2/iscc "msys.iss"
rm msys.iss
