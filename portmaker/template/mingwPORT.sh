#! /bin/sh
# mingwPORT.template
# Required but name changed to reflect package ported.
# Used to execute commands to build the ported package.

CURDIR=`pwd`

. ${CURDIR}/mingwPORT.functions

if [ -f ${CURDIR}/mingwPORT.ini ]
then
  . ${CURDIR}/mingwPORT.ini
fi

if [ -f ${CURDIR}/mingwPORT.question ]
then
  if [ -f ${CURDIR}/mingwPORT.beforequestion ]
  then
    . ${CURDIR}/mingwPORT.beforequestion
  fi

  . ${CURDIR}/mingwPORT.question

  if [ -f ${CURDIR}/mingwPORT.afterquestion ]
  then
    . ${CURDIR}/mingwPORT.afterquestion
  fi
fi

if [ -f ${CURDIR}/mingwPORT.exports ]
then
  . ${CURDIR}/mingwPORT.exports
fi

if [ -f ${CURDIR}/mingwPORT.patch ]
then
  if [ -f ${CURDIR}/mingwPORT.beforepatch ]
  then
   . ${CURDIR}/mingwPORT.beforepatch ]
  fi

  patch -t $PATCHFLAGS < ${CURDIR}/mingwPORT.patch

  if [ -f ${CURDIR}/mingwPORT.afterpatch ]
  then
   . ${CURDIR}/mingwPORT.afterpatch ]
  fi
fi

if [ -f ${CURDIR}/mingwPORT.beforeconfigure ]
then
    . ${CURDIR}/mingwPORT.beforeconfigure
fi
if [ -z "$BUILDDIR" ]
then
  BUILDDIR=bld
fi

if [ -z "$SRCDIR" ]
then
  SRCDIR=`pwd`
fi
cd $SRCDIR
ABSSRCDIR=`pwd`
cd $CURDIR

if [ ! -d ${BUILDDIR} ]
then
  mkdir ${BUILDDIR}
  RMDIR=${BUILDDIR}
else
  RMDIR='NORMDIR'
fi

cd ${BUILDDIR}
ABSBUILDDIR=`pwd`

. ${CURDIR}/mingwPORT.configure

if [ -f ${CURDIR}/mingwPORT.afterconfigure ]
then
    . ${CURDIR}/mingwPORT.afterconfigure
fi

if [ -f ${CURDIR}/mingwPORT.beforemake ]
then
  . ${CURDIR}/mingwPORT.beforemake
fi

. ${CURDIR}/mingwPORT.make

if [ -f ${CURDIR}/mingwPORT.aftermake ]
then
  . ${CURDIR}/mingwPORT.aftermake
fi

if [ -f ${CURDIR}/mingwPORT.beforeinsall ]
then
  . ${CURDIR}/mingwPORT.beforeinstall
fi

. ${CURDIR}/mingwPORT.install

if [ -f ${CURDIR}/mingwPORT.afterinstall ]
then
  . ${CURDIR}/mingwPORT.afterinstall
fi

cd ${CURDIR}

. ${CURDIR}/mingwPORT.cleanup

# end of port
