#!/bin/sh

VERINFO_SRC=bugInfo.cpp
BUGREPORT=bugReport.cpp
FILEEXISTS=0
W32API_VER=`echo -e "#include <w32api.h>\n__W32API_MAJOR_VERSION.__W32API_MINOR_VERSION" | gcc -E - | grep -v "#"`
MINGW_VER=`echo -e "#include <_mingw.h>\n__MINGW32_VERSION" | gcc -E - | grep -v "#"`


#
# dumpInfo: generates C style comments in the bug report.
#           the param 1 is a comment description, while param 2 is an executable
#           command that will provide version information.
#
function dumpInfo {

  echo "/* $1" >> $BUGREPORT

  PROGOUT=`$2 2>&1`
  echo $PROGOUT >> $BUGREPORT
  
  echo "*/" >> $BUGREPORT
  echo >> $BUGREPORT
  
}


#
# generateReport: clears a previous bug report and writes the version
#                  information, problem desc, and test case to a c or cpp file
#
function generateReport {

  echo > $BUGREPORT    # clear any previous report
 
  dumpInfo "MinGW Bug Report" "date"
	dumpInfo "MinGW Version:" "echo $MINGW_VER"
	dumpInfo "W32API Version:" "echo $W32API_VER"
  dumpInfo "gcc Version:" "gcc -v"
  dumpInfo "ld Version:" "ld -v"
  dumpInfo "MSYS Version:" "uname -a"
  dumpInfo "Problem Description:" "cat $PROBDESC"
  
  echo "/* ---------------- BEGIN TESTCASE ------------------- */" >> $BUGREPORT
  cat $TSTCASE >> $BUGREPORT
  echo "/* ---------------- END TESTCASE --------------------- */" >> $BUGREPORT

  echo "Report generated as: " $BUGREPORT

}


#
#  checkFile: checks if the file exists returns 1 if exists, else 0.
#
function checkFile {

  if [ -f $1 ];
  then
    return 1
  else
    echo "ERR: File not found!"
    return 0
  fi

}


#
#  getFiles: prompts the user to enter the filenames and checks if they exist
#
function getFiles {

  while [ "$FILEEXISTS" != "1" ]; do

    echo "Please enter the problem description filename: "
    read PROBDESC
    checkFile $PROBDESC
    FILEEXISTS=$?

		#if [ "$FILEEXISTS" != "1" ];
		#then
		#	echo "Do you wish to generate a file named $PROBDESC now(y/n)?"
		#	read CREATEFILE
		#	echo $CREATEFILE
			#yeah I know should check for uppercase Y too
		#	if [ "$CREATEFILE" == "y" ];
		#	then
		#		exec "vi $PROBDESC"	
		#	fi
		#fi
  done

  FILEEXISTS=0

  while [ "$FILEEXISTS" != "1" ]; do

    echo "Please enter the testcase source code filename: "
    read TSTCASE
    checkFile $TSTCASE
    FILEEXISTS=$?
          
  done

}


#
#  usage: will be used later when command line parameters are enabled
#
function usage {
  
  echo "usage: $0 problem_description.txt testcase.c[pp]"
  exit

}


#---------------- BEGIN main ---------------------


getFiles

generateReport

# bugs.sh END
