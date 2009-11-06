proc say {k v} {set ::say($k) $v}
proc category {k} {}
source locale/english.tcl
puts "#N canvas 0 0 560 480 10 ;"
set y 50
foreach k [lsort [array names ::say *]] {
	set v $::say($k)
	if {$k == "#"} {set k "# +"}
	if {$k == "#fold"} {set k "#fold +"}
	if {$k == "#scan"} {set k "#scan +"}
	if {$k == "#outer"} {set k "#outer +"}
	if {$k == "#cast"} {set k "#cast i"}
	if {$k == "#for"} {set k "#for 0 4 1"}
	if {$k == "#redim"} {set k "#redim ()"}
	if {$k == "receives"} {set k "receives \$0-"}
	if {$k == "send39"} {set k "send39 \$0-patchname"}
	set w [string length $k]
	regsub "\\$" $k "\\$" k
	if {$w<3} {set w 3}
	set w [expr {$w*6+2}]
	if {$k == "#color"} {set w 156}
	puts "#X obj [expr 160-$w] $y $k;"
	regsub "," $v " \\, " v
	regsub ";" $v " \\; " v
	regsub "\\$" $v "\\$" v
	puts "#X text 180 $y $v;"
	if {$k == "#color"} {incr y 40}
	incr y 32
}
