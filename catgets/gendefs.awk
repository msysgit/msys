#!/bin/awk -f
# $Id: gendefs.awk,v 1.2 2008-01-08 19:51:57 keithmarshall Exp $
#
# A simple script for generating a message catalogue definitions file
# suitable for input to `gencat', from a `C' header file which contains
# message definition records of the form:--
#
#   #define MSG_ID_TOKEN  <setnum>, <msgnum>, "<message text>"
#
# where:--
#
#   MSG_ID_TOKEN    is any valid `C' token, used to represent the
#                   manifest declaration of the message.  A unique
#                   token must be used for each individual message
#                   definition, and each must match a specified
#                   regular expression.
#
#   <setnum>        is a numeric token, specifying the identification
#                   number of the message set, to which the specified
#                   message is to be assigned; all messages in a given
#                   set must be grouped together within the `C' header
#                   file, and all message sets must be presented in
#                   strictly ascending order of set number.
#
#   <msgnum>        is a numeric token, specifying the identification
#                   number of the message, within its designated set;
#                   message definitions must be presented in strictly
#                   ascending order of message number, within each
#                   message set.
#
#   <message text>  is the fallback text defined for the message; it
#                   must all appear on the same line as the `#define'
#                   token, or that line must terminate with a reverse
#                   solidus, so escaping the newline, to indicate that
#                   the following line is to be subsumed as a logical
#                   continuation of the `#define' line; continuation,
#                   in this manner, may be extended over any number
#                   of consecutive input lines, by similarly escaping
#                   the terminal newline on all but the last physical
#                   input line of any such single logical line.
#
#
# Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
# Last modification: 02-Jan-2008
#
#
# This is free software.  It is provided AS IS, in the hope that it may
# be useful, but WITHOUT WARRANTY OF ANY KIND, not even an IMPLIED WARRANTY
# of MERCHANTABILITY, nor of FITNESS FOR ANY PARTICULAR PURPOSE.
#
# Permission is granted to redistribute this software, either "as is" or
# in modified form, under the terms of the GNU General Public License, as
# published by the Free Software Foundation; either version 2, or (at your
# option) any later version.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to the
# Free Software Foundation, 51 Franklin St - Fifth Floor, Boston,
# MA 02110-1301, USA.
#

# Establish the default convention for identifying message definitions.
#
  BEGIN { MSGTAG = "MSG_[A-Z_a-z0-9]+"; MULTILINE = 0 }
#
# If we enabled MULTILINE mode, on the preceding input line, then we treat
# the current input line as a logical continuation of the preceding line.
# We must handle this case *before* any other condition, because such line
# continuation overrides any other line type identification checks.
#
  MULTILINE == 1 {
#
#   If this continuation line is not itself continued,
#   then we turn off MULTILINE, effective from the *next* input line.
#
    if( $0 !~ /\\$/ ) MULTILINE = 0;
#
#   We then emit the continuation line, and immediately proceed to
#   the next input line.
#
    print
    next
  }
#
# If we are *not* in MULTILINE mode, then we proceed to the normal input
# line interpretation strategy...
#
# Allow the user to substitute an alternative identification convention,
# by assigning a new MSGTAG value in the input stream.
#
  ( $1 == "/*" || $1 == "*" ) && match( $2$3, "MSGTAG=" ) == 1 {
    sub( "^[^=]*=[ 	]*", "" ); MSGTAG = $0
    next
  }
#
# Identify `boilerplate' text in the header file comments,
# which is to be replicated into the message catalogue source file.
#
  ( $1 == "/*" || $1 == "*" ) && match( $2, "!" ) == 1 {
    sub( "^[^!]*!", "" ); print
    next
  }
#
# Normalise whitespace, between an initial `#' and a following `define' keyword,
# then select those records which define messages.
#
  { sub( "^#[ 	]*define[ 	]+", "# define " ) }
  $1$2 == "#define" && match( $3, MSGTAG ) == 1 && RLENGTH == length( $3 ){
#
#   Only lines, such as this, which begin with a `#define' token, are permitted
#   to initiate MULTILINE continuation mode; when such a line is terminated by
#   a reverse solidus escaped newline, then initiate MULTILINE mode.
#
    if( $0 ~ /\\$/ ) MULTILINE = 1
#
#   Extract the set number from the message definition;
#   if it differs from that of the set identified by CURSET,
#   then start a new set.
#
    SETNUM = $4
    sub( ",.*", "", SETNUM )
    if( SETNUM != CURSET ){
      CURSET = SETNUM
      print "$set", SETNUM
    }
#
#   Capture the message identification comment string,
#   and write it out to the generated message catalogue source file,
#   immediately preceding the message definition.
#
    print "$", $2, $3
#
#   Extract the message number for the current message definition.
#
    sub( "[^,]*,[ 	]*", "" )
    MSGNUM = $1; sub( ",.*", "", MSGNUM )
    sub( "[^,]*,[ 	]*", "" )
#
#   And finally, append the message number, and its associated definition,
#   to the generated message catalogue source file.
#
    print MSGNUM, $0
  }
#
# $RCSfile: gendefs.awk,v $Revision: 1.2 $: end of file
