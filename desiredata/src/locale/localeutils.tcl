#!/usr/bin/env tclsh
# to check the difference between locale files
# arguments:
# -list view the list of supported locales
# -lang <locale> compare <locale> to english
# no option: summary  of all locales

proc lwithout {a b} {
	set r {}
	foreach x $b {set c($x) {}}
	foreach x $a {if {![info exists c($x)]} {lappend r $x}}
	return $r
}

proc lintersection {a b} {
	set r {}
	foreach x $b {set c($x) {}}
	foreach x $a {if {[info exists c($x)]} {lappend r $x}}
	return $r
}

proc say {k args} {
	global text
	if {[llength $args]} {
		set ${::name}($k) [lindex $args 0]
	} else {
		if {[info exist text($k)]} {
			puts "------------"
			return $text($k)
		} else {return "{{$k}}"}
	}
}

proc say_namespace {k code} {uplevel 1 $code}
proc say_category  {text} {}

set ::name ::index
array set ::index {}
source index.tcl

foreach lang [array names ::index] {
	set ::name ::$lang
	source $lang.tcl
}

proc summary {} {
	foreach lang [lsort [lwithout [array names ::index] english]] {
		puts "#-----------------------------8<-----CUT------8<-----------------------------#"
		key_compare $lang; value_compare $lang
	}
}

proc compare {lang} {
	key_compare $lang
	value_compare $lang
}

proc value_compare {lang} {
	puts "\nFollowing entries have the same value as English:"
	set values {}
	set english2 [array names ::english]
	set locale [array names ::$lang]
	set intersect [lintersection $english2 $locale]
	foreach item [lsort $intersect] {
		set val1 $::english($item)
		set val2 [set ::${lang}($item)]
		if {$val1 == $val2} {
			lappend values $item
		}
	}
	foreach item $values {puts "\t $item"}
}


proc key_compare {lang} {
	set english2 [array names ::english]
	set locale [array names ::$lang]
	set diff_missing [lwithout $english2 $locale]
	set diff_extra [lwithout $locale $english2]
	set percent_missing [expr int(([llength $diff_missing] / [llength $english2].0)*100)]
	set percent_extra [expr int(([llength $diff_extra] / [llength $english2].0)*100)]
	puts "\ncompare $lang to english --> ${percent_missing}% difference"
	foreach item [lsort $diff_missing] {
		puts "    -    $item"
	}
	puts "\ncompare english to $lang --> ${percent_extra}% difference"
	foreach item [lsort $diff_extra] {
		puts "    +    $item"
	}
}


if {![llength $argv]} {
	summary
} else {
	switch -regexp -- [lindex $argv 0] {
		^-list\$ {puts [lwithout [array names ::index] english]}
		^-lang\$ {
			set lang [lindex $argv 1]
			if {[lsearch [array names ::index] $lang] > 0} {compare $lang} {puts "Unknown locale '$lang'"}
		}
		^-h\$ {
			puts "-list             view the list of supported locales"
			puts "-lang <locale>    only report about <locale> instead of about all locales"
		}
		default {puts "Unknown option '[lindex $argv 0]'. try -h for help"}
	}
}

