# texi2html has problems if tags wrap around a line
# this sed script joins those lines

/@[a-z]\+{[^}]*$/ {
	N
	s/\
//
}
