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
PATCHVER=7
STOREROOT=/prjz/msys/nstl
RLSROOT=/prjz/rls
ARC=$1
SNAPDATE=\-`date +%Y.%m.%d`
#SNAPDATE=\-prerelease
#SNAPDATE=
SUBVERSION=\-1
#SUBVERSION=
#END User changeable values section.

VERSION=${MAJORVER}.${MINORVER}.${PATCHVER}
SHORTVER=${MAJORVER}.${MINORVER}
INFOBEFOREFILE="`p2w ${STOREROOT}/noarch/doc/msys/MSYS-${VERSION}-changes.rtf`"
INFOAFTERFILE="`p2w ${STOREROOT}/noarch/doc/msys//MSYS_WELCOME.rtf`"
LICENSEFILE="`p2w ${STOREROOT}/noarch/doc/msys/MSYS_LICENSE.rtf`"
RLSOUTPUTDIR="`p2w ${RLSROOT}/${PACKAGE}/${VERSION}`"

istore=${STOREROOT}/${ARC}
noarchstore=${STOREROOT}/noarch
miscstore=${STOREROOT}/misc
datastore=${STOREROOT}/var

rlsdepot=${RLSROOT}/${PACKAGE}/depot/${PACKAGE}/${SHORTVER}
RLSSOURCEDIR="`p2w $rlsdepot`"

exe_LIST="`cat ${datastore}/exe.dat`"
etc_LIST="`cat ${datastore}/etc.dat`"
dll_LIST="`cat ${datastore}/dll.dat`"
doc_LIST="`cat ${datastore}/doc.dat` MSYS-${VERSION}-changes.rtf"
misc_LIST="`cat ${datastore}/misc.dat`"
script_LIST="`cat ${datastore}/script.dat`"

if [ ! -f ${rlsdepot} ]
then
  mkdir -p ${rlsdepot}
fi

rm -rf ${rlsdepot}/*

if [ ! -f ${rlsdepot}/bin ]
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

if [ ! -f ${rlsdepot}/doc/msys ]
then
  mkdir -p ${rlsdepot}/doc/msys
fi

for I in ${doc_LIST}
do
  cp ${noarchstore}/doc/msys/${I} ${rlsdepot}/doc/msys/
done

if [ ! -f ${rlsdepot}/etc ]
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
