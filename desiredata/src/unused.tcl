if {$tk} {
	set main [Client new]
	set window_list [list $main]
} else {
	set cmdline(console) 0
	#foreach dir $auto_path {
	#	set file $dir/libtclreadline[info sharedlibextension]
	#	puts "trying $file"
	#	if {![catch {load $file}]} {puts "found tclreadline !"}
	#}
	package require tclreadline
	proc ::tclreadline::prompt1 {} {return "desire> "}
	::tclreadline::Loop
	#while {1} {
	#	#set line [::tclreadline::readline read]
	#	puts -nonewline "desire> "
	#	flush stdout
	#	set line [gets stdin]
	#	if {[catch {puts [eval $line]}]} {
	#		puts "error: $::errorInfo"
	#	}
	#}
	#vwait foo
}


#lappend ::auto_path /usr/local/lib/graphviz
catch {package require Tcldot}
def Canvas graphviz_sort {} {
	error "this code has to be rewritten to use the new containers"
	set nodes {}
	set gwidth 0; set gh 0
	#toplevel .graph -height 600 -width 800
	#set c [canvas .graph.c -height 600 -width 800]
	#pack $c
	set g [dotnew digraph]
	$g setnodeattribute style filled color white
	foreach child $@children {
		lappend nodes [$g addnode $child label "[$child text]" shape "record" height "0.1"]
		lappend nodes $child
	}
	puts "$nodes"
	foreach wire $@wires {
		mset {from outlet to inlet}  [$wire report]
		set n1  [lindex $nodes [expr [lsearch $nodes $from]-1]]
		set n2  [lindex $nodes [expr [lsearch $nodes   $to]-1]]
		$n1 addedge $n2
	}
	#$g layout
        ;# see what render produces
	#if {$debug} {puts [$g render]}
	#eval [$g render]
	set f {}
	set fd [open graph.txt w]
	$g write $fd plain
	close $fd

	set fd [open graph.txt r]
	set contents [read $fd]
	close $fd
	exec rm graph.txt
	mset {x1 y1 x2 y2} [[$self widget] bbox all]
	set width [expr $x2 - $x1]
	set height [expr $y2 - $y1]
	foreach line [split $contents "\n"] {
		switch [lindex $line 0] {
			graph {set gw [lindex $line 2]; set gh [lindex $line 3]}
			node {
				set w [expr $width/$gw]
				set h [expr $height/$gh]
				set id [lindex $line 1]
				set x [lindex $line 2]; set y [lindex $line 3]
				$id moveto [expr $x*$w] [expr ($gh-$y)*$h]
			}
			edge {break}
		}
	}
}
