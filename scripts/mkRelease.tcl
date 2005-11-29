#!/bin/sh
# mkRelease
# ---------
#
# Synchronise file release data on mingw.org download page
# with actual file releases published on SourceForge.
#
# Copyright (C) 2003, 2005
#   Earnie Boyd     <earnie@users.sourceforge.net>
#   Keith Marshall  <keithmarshall@users.sourceforge.net>
#
# $Id: mkRelease.tcl,v 1.5 2005-11-29 19:37:50 keithmarshall Exp $
#\
exec tclsh "$0" "$@"


# Global Variables
# ================
#
# Debugging options
# -----------------
# Set `DebugMode' to 1, to enable debugging mode, or zero to disable.
#
set DebugMode 0

# Regular expressions to match HTML tagged page elements
# ------------------------------------------------------
# Any of these entries may span multiple input lines, it the SF project files
# HTML page description file.
#
set Record_Begin_Tag  {<[ \t]*tr[ \t]+}                   ;# start of a file release table entry
set Record_End_Tag    {</tr>}                             ;# end of a file release table entry
#
set Package_ID_Tag    {<[ \t]*tr[ \t]+class="package"}    ;# start of package name entry
set Release_ID_Tag    {<[ \t]*tr[ \t]+class="release.*"}  ;# start of file release name entry


# User Defined Procedures
# =======================
#
# debugging
# ---------
# Facilitates access to the global `DebugMode' flag,
# allowing the use of debugging constructs like `if {[debugging]} {command ...}'
#
proc debugging {} {

  global DebugMode
  return [set DebugMode]
}

# get_current_files_page
# ----------------------
# Make local working copies of the SourceForge project file release data,
# as identified in the actual web page HTML source, viz:--
#
# -  mingw.files     the project package/file list
# -  mingwftp.files  the list of files available for download
#
# Inputs: none
# Returns: nothing
#
proc get_current_files_page {} {
exec wget http://sourceforge.net/project/showfiles.php?group_id=2435 -Omingw.files -nv -a mingw.files.log
exec wget http://prdownloads.sf.net/mingw/ -Omingwftp.files -nv -a mingw.files.log
}

# seek_ftp_files_list { mingwftp_files input_buffer }
# ---------------------------------------------------
# Position the FTP files list to the first downloadable file name record,
# assuming that this is the first input line containing an image tag matching ".*\.gif.*",
# following the parent directory link, which has an image tag matching ".*back.gif.*".
#
# Inputs:
#   `mingwftp_files' = file descriptor for the opened "mingwftp.files" file.
#   `input_buffer' = caller's name for the variable used as the input line buffer.
#
# Returns:
#   a true/false value, indicating presence of at least one file name record,
#   `input_buffer' filled with the first such file name record read
#
proc seek_ftp_files_list {mingwftp_files buffer} {

  upvar $buffer ftpline

  set start_of_ftpfiles 0
  while {[gets $mingwftp_files ftpline] >= 0} {
    set start_of_ftpfiles [regexp -- ".*back.gif.*" "$ftpline"]
    if {$start_of_ftpfiles != 0} {
      while {[gets $mingwftp_files ftpline] >= 0} {
	if {[regexp -- ".*\.gif.*" "$ftpline"]} {
	  break
	}
      }
      break
    }
  }
  return $start_of_ftpfiles
}

# catalogue_all_files { ref_array }
# ---------------------------------
# Construct a catalogue of all downloadable files named on the FTP download page.
#
# Inputs:
#   `ref_array' = name of array, in caller, where catalogue will be constructed.
#
# Returns:
#   `ref_array' filled with file name, date and size entries for each file.
#
proc catalogue_all_files {ref_array} {

  upvar $ref_array FtpFiles

  set ftpfilesset 0
  set mingwftp_files [open mingwftp.files r]
  set process_files [expr [seek_ftp_files_list $mingwftp_files ftpline] != 0 ? 1 : 0]

  while {$process_files} {
    if {[regexp -- ".*\.gif.*" "$ftpline"]} {
      set ftpfile [lindex [split [lindex [split $ftpline "<"] 3] ">"] 1]
    }
    if {[regexp -- ".*label-size.*" "$ftpline"]} {
      set ftpfilesize [lindex [split [lindex [split $ftpline ">"] 1] "<"] 0]
    }
    if {[regexp -- ".*label-date.*" "$ftpline"]} {
      set ftpfiledate [lindex [split [lindex [split $ftpline ">"] 1] "<"] 0]
      set ftpfilesset 1
    }
    if {$ftpfilesset} {
      set FtpFiles($ftpfile,size) $ftpfilesize
      set FtpFiles($ftpfile,date) $ftpfiledate
      set ftpfilesset 0
    }
    set process_files [expr [gets $mingwftp_files ftpline] >= 0 ? 1 : 0]
  }
  close $mingwftp_files
}

# get_HTML_tagged_record { mingw_files buffer tag [depth] }
# ---------------------------------------------------------
# Concatenate consecutive input lines into a single line record buffer,
# until a specified closing tag is encountered.
#
# Inputs:
#   `mingw_files' = file descriptor for the opened `mingw.files' file.
#   `buffer' = caller's variable name for the record accumulation buffer.
#   `tag' = a regular expression which will match the required closing tag.
#   `depth' = `buffer' location in TCL call stack, (optional, default=1).
#
# Returns:
#   a true/false return value indicating that the required tag was found,
#   and `buffer' filled in with the trimmed content of the input record.
#
proc get_HTML_tagged_record {mingw_files buffer tag {depth 1}} {

  upvar $depth $buffer record

  set record [string trim $record]
  if {[regexp -- "$tag" "$record"]} {
    return 1
  }
  while {[gets $mingw_files line] >=0} {
    append record [string trim $line]
    if {[regexp -- "$tag" "$line"]} {
      return 1
    }
  }
  return 0
}

# get_first_package_list_entry { mingw_files buffer }
# ---------------------------------------------------
# Locate the start of the package list, within the file release table
# section of the SF project files page.
#
# Inputs:
#   `mingw_files' = file descriptor for the opened `mingw.files' file.
#   `buffer' = caller's variable name for the HTML file input buffer.
#
# Returns:
#   a true/false return value indicating if the package list was found,
#   and `buffer' filled in with the complete table row definition record.
#
proc get_first_package_list_entry {mingw_files buffer} {

  upvar $buffer input_line

  global Package_ID_Tag
  global Record_End_Tag

  set lineno 0
  while {[gets $mingw_files input_line] >= 0} {
    incr lineno
    if {[regexp -- "$Package_ID_Tag" "$input_line"]} {
      if {[debugging]} {puts stderr "$lineno: found file release table"}
      return [get_HTML_tagged_record $mingw_files $buffer "$Record_End_Tag" 2]
    }
  }
  return 0
}

# get_next_package_list_entry { mingw_files buffer }
# --------------------------------------------------
# After an initial call to `get_first_package_list_entry',
# retrieve one subsequent package list entry record, from the file release
# section of the SF project files page, on each successive call.
#
# Inputs:
#   `mingw_files' = file descriptor for the opened `mingw.files' file.
#   `buffer' = caller's variable name for the HTML file input buffer.
#
# Returns:
#   a true/false return value indicating if a new package list entry was found,
#   and `buffer' filled in with the complete table row definition record.
#
proc get_next_package_list_entry {mingw_files buffer} {

  upvar $buffer input_line

  global Record_Begin_Tag
  global Record_End_Tag

  while {[gets $mingw_files input_line] >= 0} {
    if {[regexp -- "$Record_Begin_Tag" "$input_line"]} {
      return [get_HTML_tagged_record $mingw_files $buffer "$Record_End_Tag" 2]
    }
  }
  return 0
}

# get_package_list_entry_data { source }
# --------------------------------------
# Extract all data fields from a fully constructed `<tr>...</tr>' input record,
# and place them in a TCL list, with each element representing one field.
#
# Inputs:
#   `source' = a copy of the original `<tr>...</tr>' input record.
#
# Returns:
#   the constructed TCL list of all data fields.
#
proc get_package_list_entry_data {source} {

  while {[regexp -- {>([^<]+)} "$source" match field]} {
    lappend fieldlist "$field"
    regsub -- {>[^<]+<} "$source" {><} source
  }
  return $fieldlist
}

# get_released_file_type { type_description }
# -------------------------------------------
# Establish the file type identification code, to be displayed in the
# file release table on mingw.org, based on the SF type description.
#
# Inputs:
#   `type_description' = file type, as described on SF project files page.
#
# Returns:
#   File type description to display in mingw.org file release table.
#
proc get_released_file_type {type_description} {

  switch -regexp -- $type_description {
    {Source Patch/Diff}  {set file_type diff}
    {Source.*}           {set file_type src}
    {\.gz}               {set file_type bin}
    {\.bz2}              {set file_type bin}
    {\.zip}              {set file_type bin}
    {\.exe.*}            {set file_type bin}
    default              {set file_type other}
  }
  return $file_type
}

# catalogue_file_releases { ref_array }
# -------------------------------------
# Parse the HTML source for the SF project files page,
# and construct a reference list of file statistics, grouped by SF package name,
# and file release identity.
#
# Inputs:
#   `ref_array' = name of caller's array variable, where the return data is assembled.
#
# Returns:
#   `ref_array' variable filled out with appropriate data.
#
proc catalogue_file_releases {ref_array} {

  upvar $ref_array files

  global Package_ID_Tag
  global Release_ID_Tag

  set file_id 0
  set mingw_files [open mingw.files r]
  if {[get_first_package_list_entry $mingw_files line]} {
    set category_name [lindex [get_package_list_entry_data $line] 0]
    while {[get_next_package_list_entry $mingw_files line]} {
      switch -regexp -- "$line" \
        "$Package_ID_Tag" {
          set category_name [lindex [get_package_list_entry_data $line] 0]
          if {[debugging]} {puts stderr "New Category: $category_name"}
        }\
        "$Release_ID_Tag" {
          set release_name [lindex [get_package_list_entry_data $line] 1]
          if {[debugging]} {puts stderr "Release: $category_name $release_name"}
        }\
        default {
          incr file_id
          set file_info [get_package_list_entry_data $line]
          set file_name [lindex $file_info 1]
          set file_size [lindex $file_info 2]
          set downloads [lindex $file_info 3]
          set file_arch [lindex $file_info 4]
          set file_type [get_released_file_type [lindex $file_info 5]]
          if {[debugging]} {puts stderr "File: [format %5s: $file_type] $file_name"}
          set files(${category_name},${release_name},${file_id}) "$file_type $file_name"
        }
    }
  }
  close $mingw_files
}

# construct_release_table { mingw_files ftp_files }
# -------------------------------------------------
# Emit HTML code to define the mingw.org file release table,
# (on `stdout' if `DebugMode' is set, or to file `release.html' for production use),
# based on the content of file catalogues previously created by `catalogue_all_files',
# and `catalogue_file_releases'.
#
# Inputs:
#   `mingw_files' = caller's name for the released files catalogue array.
#   `ftp_files' = caller's name for the catalogue array of all files.
#
# Returns: nothing.
#
proc construct_release_table {mingw_files ftp_files} {

  upvar $mingw_files files
  upvar $ftp_files FtpFiles

  if {[debugging]} {
    set release_html stdout
  } else {
    set release_html [open release.html w]
  }

  puts $release_html {<table border="6" width="100%">}
  set prelease ""
  set prelcat ""
  foreach I [lsort [array names files]] {
      set lrelease [split $I ","]
      set trelease [lindex $lrelease 0]
      set trelcat "$trelease,[lindex $lrelease 1]"
      if {$prelease != $trelease} {
	if {$prelease != ""} {
	  puts $release_html {		    </table></td></tr>}
	  puts $release_html {	</table></td></tr>}
	  set prelcat ""
	}
	set prelease $trelease
	puts -nonewline $release_html {<tr><td width="8%" valign="top"><h4>}
	puts -nonewline $release_html $trelease
	puts $release_html {</h4></td>}
	puts -nonewline $release_html {    <td valign="top"><table border="4" width="100%"> <tr> }
      }
      if {$prelcat != $trelcat} {
	if {$prelcat != ""} {
	  puts $release_html {		    </table></td></tr>}
	  puts -nonewline $release_html {		<tr>}
	}
	puts -nonewline $release_html {<td width="15%" valign="top"><h5>}
	puts -nonewline $release_html [lindex $lrelease 1]
	set prelcat $trelcat
	puts $release_html {</h5></td>}
	puts -nonewline $release_html {		    <td width="85%" valign="top"><table border="2" width="100%"> <tr> <td width="5%" valign="top">}
      } else {
	puts -nonewline $release_html {		   		<tr> <td width="5%" valign="top">}
      }
      set lfile $files($I)
      puts -nonewline $release_html [lindex $lfile 0]
      puts $release_html {</td>}
      puts -nonewline $release_html {				     <td width="64%" valign="top">}
      puts -nonewline $release_html {<a href="http://prdownloads.sf.net/mingw/}
      puts -nonewline $release_html [lindex $lfile 1]
      puts $release_html {?download" target="_nw">}
      puts -nonewline $release_html [lindex $lfile 1]
      puts $release_html {</a></td>}
      puts -nonewline $release_html {				     <td width="11%" nowrap valign="top">}
      puts -nonewline $release_html $FtpFiles([lindex $lfile 1],size)
      puts $release_html {</td>}
      puts -nonewline $release_html {				     <td width="15%" nowrap valign="top">}
      set ldate [split "$FtpFiles([lindex $lfile 1],date)"]
      set date "[lindex $ldate 0] [lindex $ldate 1] [lindex $ldate 2]"
      set time "[lindex $ldate 3]"
      puts -nonewline $release_html $date
      puts $release_html {</td>}
      puts -nonewline $release_html {				     <td width="5%" nowrap valign="top">}
      puts -nonewline $release_html $time
      puts $release_html {</td>}
      puts $release_html {				</tr>}
  }
  puts $release_html {		    </table></td></tr>}
  puts $release_html {	</table></td></tr>}
  puts $release_html {</table>}
  if {![debugging]} {close $release_html}
}

# process_current_files_page
# --------------------------
# Parse working files, to create catalogues of all, and released files,
# and construct the HTML table definition, based on these catalogues.
#
# Inputs: none.
# Returns: nothing.
#
proc process_current_files_page {} {
  catalogue_all_files FtpFiles
  catalogue_file_releases files
  construct_release_table files FtpFiles
}

# move_release_html
# -----------------
# Make the generated HTML table definition available to the mingw.org web server.
#
# Inputs: none.
# Returns: nothing.
#
proc move_release_html {} {
  exec cp -f release.html /home/groups/m/mi/mingw/htdocs/trial/htdocs/ssi/
  exec cp -f /home/groups/m/mi/mingw/htdocs/trial/htdocs/ssi/release.html /home/groups/m/mi/mingw/htdocs/ssi/
  exec rm -f release.html
}

# delete_working_files
# --------------------
# Clean up the working directory, by removing temporary working files.
#
# Inputs: none.
# Returns: nothing.
#
proc delete_working_files {} {
  exec rm -f mingw.files
  exec rm -f mingwftp.files
}


# Main Program
# ============
#
if {[debugging]} {
#
# for debugging...
# assume working files already exist, and
# restrict activity to parsing of the SF project files listing,
# and formatting of the mingw.org HTML table source...
#
  process_current_files_page

} else {
#
# while for production...
# update the working files, format the mingw.org HTML source,
# make it active, and clean up the working files.
#
  get_current_files_page
  process_current_files_page
  move_release_html
  delete_working_files
}

# $RCSfile: mkRelease.tcl,v $: end of file
