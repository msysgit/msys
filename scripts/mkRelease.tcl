#!/bin/sh
#\
exec tclsh "$0" "$@"

proc get_current_files_page {} {
exec wget http://sourceforge.net/project/showfiles.php?group_id=2435 -Omingw.files -nv -a mingw.files.log
exec wget http://prdownloads.sf.net/mingw/ -Omingwftp.files -nv -a mingw.files.log
}

proc process_current_files_page {} {

  set mingwftp_files [open mingwftp.files r]

  set start_of_ftpfiles 0
  set lcnt 0
  set ftpfilesset 0
  while {[gets $mingwftp_files ftpline] >= 0} {
    set start_of_ftpfiles [regexp -- ".*back.gif.*" "$ftpline"]
    incr lcnt
    if {$start_of_ftpfiles != 0} {
      while {[gets $mingwftp_files ftpline] >= 0} {
	if {[regexp -- ".*\.gif.*" "$ftpline"]} {
	  break
	}
      }
      break
    }
  }

  set process_files [expr $start_of_ftpfiles != 0 ? 1 : 0]

  while {$process_files} {
    if {[regexp -- ".*\.gif.*" "$ftpline"]} {
      set ftpfile [lindex [split [lindex [split $ftpline "<"] 3] ">"] 1]
    }
    if {[regexp -- ".*label-size.*" "$ftpline"]} {
      set ftpfilesize [lindex [split [lindex [split $ftpline ">"] 1] "<"] 0]
    }
    if {[regexp -- ".*label-date.*" $ftpline"]} {
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

  set mingw_files [open mingw.files r]

  set category_name "unknown"
  set file_id 0
  while {[gets $mingw_files line] >= 0} {
    set lline [split "$line" ">"]
    if {[set category_colnum [lsearch "$lline" "<h3"]] >= 0} {
      set category_name [lindex [split [lindex $lline [expr $category_colnum + 1]] "<"] 0]
    }
    if {[set release_colnum [lsearch -regexp "$lline" ".*release_id.*"]] >= 0} {
      set release_id [lindex [split [lindex [split [lindex $lline $release_colnum] "="] 2] "\""] 0]
      set release_name [lindex [split [lindex $lline [expr $release_colnum + 2]] "<"] 0]
    }
    if {[set download_colnum [lsearch -regexp $lline ".*\?download\".*"]] >= 0} {
      set download_type [lindex [split [lindex $lline [expr ${download_colnum} + 10]] "<"] 0]
      switch -regexp ${download_type} {
	{Source Patch/Diff} {set file_type diff}
	{Source.*} {set file_type src}
	{\.gz} {set file_type bin}
	{\.bz2} {set file_type bin}
	{\.exe.*} {set file_type bin}
	default {set file_type other}
      }
      set file_name [lindex [split [lindex $lline [expr $download_colnum + 1]] "<"] 0]
      incr file_id
      set files(${category_name},${release_name},${file_id}) "$file_type $file_name"
    }
  }

  close $mingw_files

  set release_html [open release.html w]

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
	puts -nonewline $release_html {<tr><td width="8%">}
	puts -nonewline $release_html $trelease
	puts $release_html {</td>}
	puts -nonewline $release_html {    <td><table border="4" width="100%"> <tr> }
      }
      if {$prelcat != $trelcat} {
	if {$prelcat != ""} {
	  puts $release_html {		    </table></td></tr>}
	  puts -nonewline $release_html {		<tr>}
	}
	puts -nonewline $release_html {<td width="15%">}
	puts -nonewline $release_html [lindex $lrelease 1]
	set prelcat $trelcat
	puts $release_html {</td>}
	puts -nonewline $release_html {		    <td width="85%"><table border="2" width="100%"> <tr> <td width="5%">}
      } else {
	puts -nonewline $release_html {		   		<tr> <td width="5%">}
      }
      set lfile $files($I)
      puts -nonewline $release_html [lindex $lfile 0]
      puts $release_html {</td>}
      puts -nonewline $release_html {				     <td width="64%">}
      puts -nonewline $release_html {<a href="http://prdownloads.sf.net/mingw/}
      puts -nonewline $release_html [lindex $lfile 1]
      puts $release_html {?download" target="_nw">}
      puts -nonewline $release_html [lindex $lfile 1]
      puts $release_html {</a></td>}
      puts -nonewline $release_html {				     <td width="11%" nowrap>}
      puts -nonewline $release_html $FtpFiles([lindex $lfile 1],size)
      puts $release_html {</td>}
      puts -nonewline $release_html {				     <td width="15%" nowrap>}
      set ldate [split "$FtpFiles([lindex $lfile 1],date)"]
      set date "[lindex $ldate 0] [lindex $ldate 1] [lindex $ldate 2]"
      set time "[lindex $ldate 3]"
      puts -nonewline $release_html $date
      puts $release_html {</td>}
      puts -nonewline $release_html {				     <td width="5%" nowrap>}
      puts -nonewline $release_html $time
      puts $release_html {</td>}
      puts $release_html {				</tr>}
  }
  puts $release_html {		    </table></td></tr>}
  puts $release_html {	</table></td></tr>}
  puts $release_html {</table>}
  close $release_html
}

proc move_release_html {} {
  exec cp -f release.html /home/groups/m/mi/mingw/htdocs/trial/htdocs/ssi/
  exec cp -f /home/groups/m/mi/mingw/htdocs/trial/htdocs/ssi/release.html /home/groups/m/mi/mingw/htdocs/ssi/
  exec rm -f release.html
}

proc delete_work_file {} {
  exec rm -f mingw.files
  exec rm -f mingwftp.files
}

proc main {} {
  get_current_files_page
  process_current_files_page
  move_release_html
  delete_work_file
}

main
