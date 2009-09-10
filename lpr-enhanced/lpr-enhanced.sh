#! /bin/bash
#-------------------------------------------------------------------------------
# lpr.sh v1.0.1, Copyright (C) 2002, 2007 by Keith Marshall
#-------------------------------------------------------------------------------
#
# Shell script for use under MSYS or Cygwin, providing a daemonless emulation
# of the UNIX System V "lp", or BSD "lpr", line printing utility.
#
# Author: Keith Marshall <keithmarshall@users.sourceforge.net>
#
# Description:
#
# This script is typically installed as "/usr/bin/lp", with support files in
# "/usr/spool/lp".  All printer specific configuration is established in the
# file  "/usr/spool/lp/config/.active";  this may  typically  be  linked  to
# "/etc/printcap";  (symlinked for Cygwin;  hard linked for MSYS on NTFS; if
# preferred, just use /etc/printcap, especially with MSYS on FAT-32).
#
# A sample configuration is provided in "/usr/spool/lp/config/example".
#
# When  invoked  as  "lpr",   this  implementation  currently  supports  the
# following subset of the standard capabilities of BSD "lpr" ...
#
#    lpr [-P PRINTER] [-cdfgntv] file ...
#
#    where ...
#
#       -P PRINTER      specifies the printer to be used; if not supplied
#                       the values of the environment variables LDPEST or
#                       else PRINTER are substituted, in that order; if a
#                       printer still has not been identified,  then  the
#                       default set in "/usr/spool/lp/config/.active"  is
#                       used.
#
#       -c              selects a filter for printing "cifplot" formatted
#                       output  data,  when a filter for this purpose  is
#                       defined in the printer configuration.
#
#       -d              selects  a  filter  for  printing  "tex"   device
#                       independent output data,  when a filter for  this
#                       purpose is defined in the printer configuration.
#
#       -f              selects  a filter for printing output  data  with
#                       FORTRAN  carriage control formatting  attributes,
#                       when a filter for this purpose is defined in  the 
#                       printer configuration.
#
#       -g              selects a filter for printing plot format  output
#                       data,  when a filter for this purpose is  defined
#                       in the printer configuration.
#
#       -n              selects a filter for printing "ditroff" formatted
#                       output  data,  when a filter for this purpose  is
#                       defined in the printer configuration.
#
#       -t              selects a filter for printing  "troff"  formatted
#                       output  data,  when a filter for this purpose  is
#                       defined in the printer configuration.
#
#       -v              selects a filter for printing raster image output
#                       data,  when a filter for this purpose is  defined
#                       in the printer configuration.
#
#    other "lpr" options,  as described on the relevant "man" page,  may  be
#    specified; they will be silently ignored.
#
# When invoked as "lp", this implementation currently supports the following
# subset of the normal capabilities of UNIX System V "lp" ...
#
#    lp [-d PRINTER] [-o OPTIONS] [-s] file ...
#
#    where ...
#
#       -d PRINTER      specifies the printer to be used; if not supplied
#                       the values of the environment variables LDPEST or
#                       else PRINTER are substituted, in that order; if a
#                       printer still has not been identified,  then  the
#                       default set in "/usr/spool/lp/config/.active"  is
#                       used.
#
#       -o OPTIONS      specifies options to be passed to the filter used
#                       to format files for output;  these are passed  as
#                       typed, without validation.
#
#       -s              suppress informational messages.
#
#    other  "lp" options,  as described on the relevant "man" page,  may  be
#    specified; they will be silently ignored.
#
#-------------------------------------------------------------------------------
#
# first identify the base path ...
# for the spool file system used by "lp" ...
#
  SPOOL=/usr/spool/lp
#
# and also the location of the printer configuration file ...
# (default is $SPOOL/config/.active, with fall back to /etc/printcap) ...
#
  LP_CONFIG_FILE=$SPOOL/config/.active
  test -r $LP_CONFIG_FILE || LP_CONFIG_FILE=/etc/printcap
#
# establish the default printer ...
# inheriting any "$PRINTER" setting from the environment ...
# otherwise, using the default established in the "$LP_CONFIG_FILE" ...
#
  : ${PRINTER:="<default>"}
#
# if specified in the environment ...
# the System V compatible "$LPDEST" overrides "$PRINTER" assignment ...
#
  : ${LPDEST:=$PRINTER}
#
# make sure we don't inherit any exported settings ...
# for parameters we want to conditionally establish locally ...
#
  unset LP_SERVER LP_DEVICE LP_STREAM LP_FILTER LP_DRIVER
  unset LPSTAT LPOPTS LM MI PL PW FF FQ
#
# we can now parse the "lp" command line options ...
# ( this may override the default "$LPDEST" ) ...
#
  if test "${CMD:=$(basename $0)}" = "lpr"
  then
#
#   when invoked as "lpr" ...
#   then we must honour the BSD option set ...
#
    while getopts 1:2:3:4:"#":cC:dfghi:J:lmnpP:rstT:U:vw: opt
    do
      case $opt in
	1) ;;
	2) ;;
	3) ;;
	4) ;;
	#) ;;
	c) IF=cf ;;
	C) ;;
	d) IF=df ;;
	f) IF=rf ;;
	g) IF=gf ;;
	h) ;;
	i) LM=$OPTARG
	   echo $LM | grep -q "[^0-9]" && LM=8 OPTIND=$(($OPTIND-1)) ;;
	J) ;;
	l) ;;
	m) ;;
	n) IF=nf ;;
	p) ;;
	P) LPDEST=$OPTARG ;;
	r) ;;
	s) ;;
	t) IF=tf ;;
	T) ;;
	U) ;;
	v) IF=vf ;;
	w) PW=$OPTARG ;;
	*) exit 1 ;;
      esac
    done
#
  else
#
#   when invoked as "lp" ...
#   then we honour the UNIX System V "lp" option set ...
#
    while getopts cd:f:H:imn:o:pP:q:rsS:t:T:wy: opt
    do
      case $opt in
	c) ;;
	d) LPDEST=$OPTARG ;;
	f) ;;
	H) ;;
	i) ;;
	m) ;;
	n) ;;
	o) LPOPTS=${LPOPTS:+"$LPOPTS "}$OPTARG ;;
	p) ;;
	P) ;;
	q) ;;
	r) ;;
	s) LPSTAT=/dev/null ;;
	S) ;;
	t) ;;
	T) ;;
	w) ;;
	y) ;;
	*) exit 1 ;;
      esac
    done
  fi
#
# with any supplied options safely interpreted ...
# we can now delete them from the command line ...
# leaving just the list of files to print ...
#
  shift $(( $OPTIND - 1 ))
#
# now we must extract the configuration record for the target printer ...
# from the printer configuration file ...
#
  UNCOMMENT="-e /^[`echo -en \\011\\040`]*#/d"
  GET_CONFIG="-e s/|/:/g -e /$LPDEST:/,/[^\\]\$/p"
  LP_CONFIG=`sed -n $UNCOMMENT $GET_CONFIG $LP_CONFIG_FILE | tr -d '\012'`
  unset UNCOMMENT GET_CONFIG
#
# if we are using the default printer ...
# then we should resolve it back to its primary system name ...
#
  test "$LPDEST" = "<default>" && LPDEST=$(echo "$LP_CONFIG" | cut -d: -f1)
#
# now we can use the following functions ...
#
  function extract()
  {
#   to identify printer specific options ...
#   from the configuration field identified by the "$1" function argument ...
#   provided such a field is defined for the current printer ...
#
    if echo $2 | grep -q "^.*:$1[=#]"
    then
      echo $2 | sed -e s/"^.*:$1[=#]"// -e s/':.*$'//
    fi
  }
#
# and ...
#
  function asserted()
  {
#   to test if a specified printcap flag ...
#   is asserted or revoked for the current printer ...
#
    DEFAULT_YES=0 DEFAULT_NO=1
#
    echo $2 | grep -q ":$1:" && return $DEFAULT_YES
    echo $2 | grep -q ":$1@:" && return $DEFAULT_NO
    return ${3:-$DEFAULT_NO}
  }
#
# now we attempt to identify the printer output stream ...
# initially assuming it is configured by an "lp" (local printer) record ...
#
  : ${LP_SERVER:=${LP_DEVICE:=${LP_STREAM:=$(extract lp "$LP_CONFIG")}}}
#
# if we didn't find a local printer definition ...
# then we MUST have an "rp" (remote printer) configuration ...
#
  : ${LP_DEVICE:=$(extract rp "$LP_CONFIG")}
  : ${LP_DEVICE:?"printer '$LPDEST' not configured"}
#
# a remote printer definition ...
# MUST be supported by an "rm" (remote machine) server configuration ...
#
  : ${LP_STREAM:=//${LP_SERVER:=$(extract rm "$LP_CONFIG")}/$LP_DEVICE}
  : ${LP_SERVER:?"remote printer '$LPDEST' has no known server"}
#
# once we have established "LP_STREAM" ...
# we can safely forget about "LP_SERVER" and "LP_DEVICE" ...
#
  unset LP_SERVER LP_DEVICE
#
# request a unique print job identifier ...
# by evaluation of the "lpd job index" registry entry ...
#
# (The following three commented commands work only for Cygwin) ...
#
# LP_REGKEY="/user/Software/Cygnus Solutions/Cygwin/Program Options/lpd"
# regtool -q check "$LP_REGKEY" || regtool -q add "$LP_REGKEY"
# LP_JOBKEY=$(regtool -q get "$LP_REGKEY/job index")
#
# On MSYS, we don't have the `regtool' command, so we track the index in
# a data file instead; (this works on Cygwin too, in place of the above
# use of `regtool'; Cygwin users may comment these, and uncomment above,
# if use of the registry is preferred) ...
#
  test -d ${LP_REGKEY="/usr/spool/lp/data"} || mkdir -p $LP_REGKEY
  test -f $LP_REGKEY/.job.id && LP_JOBKEY=$( cat $LP_REGKEY/.job.id )
  LP_JOBKEY=$(( ${LP_JOBKEY:-0} + 1 ))
#
# we generate a serial number for the current print job ...
# by incrementing the index returned from the registry lookup ...
# wrapping around to one, when we exceed 32767 ...
#
  test $LP_JOBKEY -ge 32767 && LP_JOBKEY=1
#
# we must now update the job index in the registry ...
# before we forget the key name ...
#
# regtool -q set "$LP_REGKEY/job index" $LP_JOBKEY
  echo $LP_JOBKEY > $LP_REGKEY/.job.id
  unset LP_REGKEY
#
# we can now format the print job identifier ...
# from the target printer name and job serial number ...
#
  LP_JOBKEY=$(printf "%s-%04d" "$LPDEST" $LP_JOBKEY)
#
# process a single file named "-" ...
# as if no file list was specified ...
# 
  test "$*" = "-" && set --
#
# unless suppressed by "-s" command line option ...
# log a "request id" message ...
#
  test $# -gt 0 || FILES="1 file (standard input)"
  eval echo '"request id is $LP_JOBKEY; ${FILES:-$# file(s)}" >'${LPSTAT:-"&2"}
  unset FILES
#
# define a function ...
# to handle the actual printing operation ...
#
  function spool()
  {
#
#   set up a temporary spool file ...
#   and initialise it, by writing any specified job leader string ...
#
    LP_SPOOL_FILE=$(extract sd "$LP_CONFIG")
    LP_SPOOL_FILE=${LP_SPOOL_FILE:-"$SPOOL/tmp"}/$LP_JOBKEY
    echo -en $(extract ld "$LP_CONFIG") > $LP_SPOOL_FILE
#
#   set up form feed controls ...
#
    : ${FF:=$(extract ff "$LP_CONFIG")}
    : ${FQ:=${FF:='\014'}}
#
#   output an initial form feed, if required ...
#
    asserted fo "$LP_CONFIG" $DEFAULT_NO && echo -en "$FF" >> $LP_SPOOL_FILE
#
#   activate form feed suppression, if required ...
#
    asserted sf "$LP_CONFIG" $DEFAULT_NO && unset FF
#
#   identify the formatting and output filters ...
#   applicable for this print job ...
#
    : ${LP_DRIVER:=$(extract ${OF:-of} "$LP_CONFIG")}
    : ${LP_FILTER:=$(extract ${IF:-if} "$LP_CONFIG")}
#
#   if "$LP_CONFIG" doesn't specify a formatting filter ...
#   then force the margin indicator character to "o" ...
#   and default to using "/bin/pr" with tab expansion ...
#
    : ${LP_FILTER:-${MI:="o"}}
    : ${LP_FILTER:='/bin/pr -e -h$FILE'}
#
#   establish the printer's default page size ...
#
    : ${PW:=$(extract pw "$LP_CONFIG")}
    : ${PL:=$(extract pl "$LP_CONFIG")}
#
#   establish the width for the optional left margin ...
#
    : ${MI:=$(extract mi "$LP_CONFIG")}
    : ${LM:=$(extract lm "$LP_CONFIG")}
#
#   pass the page size and margin settings to the formatting filter ...
#   by prepending to the "$LPOPTS" string ...
#
    LPOPTS=${PW:+"-w$PW "}${PL:+"-l$PL "}${LM:+"-${MI:-i}$LM "}$LPOPTS
#
#   establish a destination for any error messages ...
#
    LP_LOG_FILE=$(extract lf "$LP_CONFIG")
    exec 2>> ${LP_LOG_FILE:-"/dev/tty"}
#
#   loop over all specified files to be printed ...
#
    test $# -gt 0 || set -- -
    for FILE
    do
#
#     when the specified file is at least readable ...
#
      if test "$FILE" = "-" -o -r $FILE
      then
#
#       pass it through the formatting filter ...
#       and append it to the spool file, followed by a page eject ...
#
        test "$FILE" = "-" && FILE=" "
	/bin/cat $FILE | eval $LP_FILTER $LPOPTS >> $LP_SPOOL_FILE
	echo -en "$FF" >> $LP_SPOOL_FILE
#
      else
#
#	when the file is NOT readable ...
#	write an appropriate message to the error log ...
#
	echo $0: $LPDEST: $FILE: cannot access file >&2
#
      fi
    done
#
#   after all files have been processed ...
#   output a terminal form feed, if required ...
#   and append the configured job termination string (if any) ...
#
    asserted fq "$LP_CONFIG" $DEFAULT_NO && echo -en "$FQ" >> $LP_SPOOL_FILE
    echo -en $(extract tr "$LP_CONFIG") >> $LP_SPOOL_FILE
#
#   and finally ...
#   push the spool file to the printer output stream ...
#   through any applicable output filter ...
#   and then delete the spool file ...
#
    eval /bin/cat $LP_SPOOL_FILE ${LP_DRIVER:+"| $LP_DRIVER"} > $LP_STREAM
    /bin/rm -f $LP_SPOOL_FILE
#
  }
#
# and finally ...
# invoke the printing function ...
#
  if asserted bg "$LP_CONFIG" $DEFAULT_YES
  then
#
#   forking a background process to complete it ...
#   when background printing is supported by the printer configuration ...
#
    spool "$@" &
#
  else
#
#   otherwise ...
#   we must keep the print job in the foreground ...
#
    spool "$@"
  fi
#
#-------------------------------------------------------------------------------
# lpr: end of file
