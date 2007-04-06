#!/bin/awk -f
# $Id: gendefs.awk,v 1.1 2007-04-06 22:34:55 keithmarshall Exp $
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
#                   must all on the same line as the `#define' token.
#
#
# Written by Keith Marshall  <keithmarshall@users.sourceforge.net>
# Last modification: 05-Mar-2007
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
  BEGIN { MSGTAG = "MSG_[A-Z_a-z0-9]+" }
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
# $RCSfile: gendefs.awk,v $Revision: end of file
