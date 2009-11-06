# (this proc is taken from desiredata)
# split at message boundaries.
# \n is wiped, then that character is reused temporarily to mean a quoted semicolon.
proc pd_mess_split {e} {
	set r {}
	regsub -all "\n" $e " " y
	regsub -all {\\;} $y "\n" z
	foreach mess [split $z ";"] {
		regsub -all "\n" $mess "\\;" mess
		set mess [string trimleft $mess]
		if {$mess != ""} {lappend r $mess}
	}
	return $r
}

proc pd_read_file {filename} {
  set f [open $filename]
  set r [pd_mess_split [read $f]]
  close $f
  return $r
}

proc pd_pickle {l} {
	set i 0
	set t ""
	set n [llength $l]
	foreach e $l {
		incr n -1
		#regsub -all "," $e "\\," e
		append t $e
		incr i [string length $e]
		if {$i>65} {set i 0; append t "\n"} elseif {$n>0} {incr i; append t " "}
	}
	append t ";"
	return $t
}

