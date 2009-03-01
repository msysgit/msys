#!/bin/sh
# x86-mingw32-build.sh -*- sh -*- vim: filetype=sh
# $Id: x86-mingw32-build.sh,v 1.9 2009-03-01 16:20:09 keithmarshall Exp $
#
# Script to guide the user through the build of a GNU/Linux hosted
# MinGW cross-compiler for Win32.
#
# Copyright (C) 2006, 2009, MinGW Project
# Written by Keith Marshall <keithmarshall@users.sourceforge.net>
# 
# This is the primary script for the x86-mingw32-build package.
#
# x86-mingw32-build is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version.
# 
# x86-mingw32-build is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for further details.
# 
# You should have received a copy of the GNU General Public License along
# with x86-mingw32-build; see the file COPYING.  If not, write to the Free
# Software Foundation, 51 Franklin St - Fifth Floor, Boston, MA 02110-1301,
# USA.

CURDIR=`pwd` PATH="$PATH:$CURDIR"
test -r $0.sh.conf && script=$0.sh || script=$0
. $script.functions
. $script.conf
. $script.getopts

# Specify the TARGET identification prefix, to be assigned to the cross
# compiler tool suite, and the operating system element, which may be used
# to qualify source package names.
#
TARGET=${1-${TARGET-${TARGET_CPU-"i386"}-${TARGET_OS="mingw32"}}}
TARGET_OS=${TARGET_OS-"`echo $TARGET | sed 's,^.*-mingw,mingw,'`"}

# Check how the user wants to progress the build, (default is `interactive'),
# then hand off package selection and set-up to the appropriate helper script.
#
assume BUILD_METHOD interactive
test "$BUILD_METHOD" = interactive && BUILD_METHOD=dialogue || BUILD_METHOD=batch
test -r $script.$BUILD_METHOD && . $script.$BUILD_METHOD

# Check that all required packages are either available locally,
# or can be downloaded from a source repository identified on set-up.
#
echo "
$script: checking package availability ..."
setbuilddir $PACKAGE_DIR .
for FILE in $DOWNLOAD
do
  prompt " $FILE ... "
  if test -f $FILE
  then
    echo ok
  elif isyes $ALLOW_DOWNLOADS
  then
    echo downloading ...
    $RUN wget $DOWNLOAD_HOST/$FILE || die $? "$script: download failed"
  else
    die 2 "missing ...
$script: unable to continue"
  fi
done

# Prepare for building all selected packages and components.
#
prompt "
$script: preparing the build tree... "
eval $RUN $CLEAN_SLATE_AT_START
setbuilddir "$WORKING_DIR" .
echo "done."

MAKE=${MAKE-"make"}
PATH=$INSTALL_DIR/bin:$PATH
unrecoverable="$script: unrecoverable error"

# The following pair of variables are used when `making' the GCC components;
# we initialise them here, to restrict the STAGE 1 build to creation of only
# the C language compiler; they are subsequently reset, after completion of
# that STAGE 1 build, to enable the building of any other selected language
# components in STAGE 2.
#
ALL_GCC=all-gcc INSTALL_GCC=install-gcc

# Progress the build, in two stages.
#
for STAGE in 1 2
do for COMPONENT in $BUILD_COMPONENTS
  do echo "
$script: stage $STAGE: build $COMPONENT ..."
  case $COMPONENT in

    binutils)
      if test -r binutils*/build/Makefile
      then
        cd binutils*/build
      else
	$RUN prepare binutils-$BINUTILS_VERSION
	$RUN setbuilddir binutils*
	$RUN ../configure --prefix="$INSTALL_DIR" --target="$TARGET" \
	  $GLOBAL_BASE_OPTIONS $BINUTILS_BASE_OPTIONS \
	  --with-sysroot="${INSTALL_DIR}" || die $? \
          "$unrecoverable configuring binutils"
      fi
      $RUN $MAKE CFLAGS="`echo $CFLAGS_FOR_BINUTILS`" \
        LDFLAGS="`echo $LDFLAGS_FOR_BINUTILS`" || die $? \
	"$unrecoverable building binutils"
      $RUN $MAKE install || die $? \
	"$unrecoverable installing binutils"
      cd "$WORKING_DIR"; test $LEAN_BUILD && $RUN rm -rf binutils*
      ;;

    gcc)
      if ! test -r gcc-*/configure
      then
	$RUN prepare gcc-core-$GCC_VERSION
        for FILE in $GCC_LANGUAGE_OPTIONS
	do
	  case $GCC_LANGUAGE_SET in *$FILE*) ;; *) FILE=no ;; esac
	  case $FILE in 'c++') FILE='g++' ;; f77) FILE=g77 ;; esac
	  test $FILE = no || $RUN prepare gcc-$FILE-$GCC_VERSION
	done
      fi
      $RUN setbuilddir build-gcc .
      if ! test -r ./config.status
      then
	$RUN ../gcc-*/configure --prefix="$INSTALL_DIR" --target="$TARGET" \
	  $GLOBAL_BASE_OPTIONS $GCC_BASE_OPTIONS --with-sysroot="$INSTALL_DIR" \
	  --enable-languages=$GCC_LANGUAGE_SET || die $? \
	  "$unrecoverable configuring gcc"
      fi
      $RUN $MAKE CFLAGS="$CFLAGS_FOR_GCC" \
        LDFLAGS="$LDFLAGS_FOR_GCC" $ALL_GCC || die $? \
       	"$unrecoverable building gcc"
      $RUN $MAKE $INSTALL_GCC || die $? \
        "$unrecoverable installing gcc"
      cd "$WORKING_DIR"; test $LEAN_BUILD && rm -rf build-gcc
      ALL_GCC="" INSTALL_GCC=""
      ;;

    headers | mingw-runtime | w32api)
      MINGWRT=`tarname mingw-runtime \
        $RUNTIME_VERSION src.tar.gz $PACKAGE_DIR | sed s',-[0-9].*,,'`
      test -r $MINGWRT-*/configure || $RUN prepare $MINGWRT-$RUNTIME_VERSION
      test -r w32api-*/configure || $RUN prepare "w32api-$W32API_VERSION"
      case $COMPONENT in
	headers)
	  $RUN mkdir -p "$INSTALL_DIR/include"
          test -e "$INSTALL_DIR/usr" || (
	    $RUN cd "$INSTALL_DIR" && $RUN ln -s . usr )
          test -e "$INSTALL_DIR/usr/local" || (
	    $RUN cd "$INSTALL_DIR/usr" && $RUN ln -s . local )
	  $RUN cp -r $MINGWRT-*/include "$INSTALL_DIR" || die $? \
            "$unrecoverable installing mingw-runtime headers"
	  $RUN cp -r w32api-*/include "$INSTALL_DIR" || die $? \
            "$unrecoverable installing w32api headers"
	  ;;
	mingw-runtime)
	  COMPONENT=$MINGWRT
          test -e w32api || $RUN ln -s w32api-* w32api
	  ;;
      esac
      case $COMPONENT in mingw-runtime | mingwrt | w32api)
	$RUN setbuilddir ${COMPONENT}-*
	$RUN ../configure --prefix="$INSTALL_DIR" --host="$TARGET" \
          --build=${BUILD_PLATFORM="`../config.guess`"} || die $? \
          "$unrecoverable configuring $COMPONENT"
	$RUN $MAKE CFLAGS="$CFLAGS_FOR_RUNTIME" \
          LDFLAGS="$LDFLAGS_FOR_RUNTIME" || die $? \
          "$unrecoverable building $COMPONENT"
        $RUN $MAKE install || die $? \
          "$unrecoverable installing $COMPONENT"
        ;;
      esac
      ;;

  esac; done
  cd "$WORKING_DIR"
  test $LEAN_BUILD && $RUN rm -rf mingw-runtime-* mingwrt-* w32api-*
  BUILD_COMPONENTS=`case $BUILD_COMPONENTS in *gcc*) echo gcc ;; esac`
done

# Clean up when done.
#
prompt "
$script: cleaning up... "
cd "$WORKING_DIR/.."; eval $RUN $CLEAN_SLATE_ON_EXIT
echo "done."
exit 0

# $RCSfile: x86-mingw32-build.sh,v $Revision: 1.9 $: end of file
