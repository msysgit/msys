#! /bin/sh
#############################################################################
# msysbld.sh - Drive an automated build of msysCORE                         #
# Copyright (C) 2007, 2009  Cesar Strauss  <cestrauss@gmail.com>            #
#                                                                           #
# This file is part of msysDVLPR                                            #
#   http://www.mingw.org/wiki/HOWTO_Create_an_MSYS_Build_Environment        #
#                                                                           #
#############################################################################

fail()
{
  echo
  echo "=================="
  echo "MSYS Build failed."
  echo "=================="
  exit 1
}

succeed()
{
  echo
  echo "====================="
  echo "MSYS Build succeeded."
  echo "====================="
}

export msysinstalldir=$PWD/store/pkg

for D in ${PWD}/src/*; do
  cd "$D"
  ./msysrlsbld || fail
done

succeed
