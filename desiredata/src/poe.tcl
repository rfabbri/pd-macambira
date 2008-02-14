# $Id: poe.tcl,v 1.1.2.2.2.27 2007-10-15 15:58:13 chunlee Exp $
#----------------------------------------------------------------#
# POETCL
#
#   Copyright (c) 2005,2006,2007,2008 by Mathieu Bouchard
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# See file ../COPYING.desire-client.txt for further informations on licensing terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# Note that this is not under the same license as the rest of PureData.
# Even the DesireData server-side modifications stay on the same license
# as the rest of PureData.
#
#-----------------------------------------------------------------------------------#

# (please distinguish between what this is and what dataflow is)
# note, the toplevel class is called "thing".

package provide poe 0.1

set nextid 0
set _(Class:_class) Class
set _(Class:_super) {Thing}
set have_expand [expr ![catch {set a {foo bar}; list {expand}$a}]]
proc proc* {name args body} {
	set argl {}
	foreach arg $args {set arg [lindex $arg 0]; lappend argl "$arg=\$$arg"}
	if {[regexp {_unknown$} $name]} {
		proc $name $args "upvar 1 selector ___; puts \"\[VTgreen\]CALL TO PROC $name selector=\$___ [join $argl " "]\[VTgrey\]\"; $body"
	} else {
		if {![regexp "return" $body]} {
			set body  "time {$body}"
			proc $name $args "puts \"\[VTgreen\]CALL TO PROC $name [join $argl " "], \[VTred\]\[lrange \[split \[$body\] \] 0 1\] \[VTgrey\]\""
		} {
			proc $name $args "puts \"\[VTgreen\]CALL TO PROC $name [join $argl " "]\[VTgrey\]\"; $body"
		}
	}
}

#proc Class_def {self selector args body} {
#	global _; if {![info exists _($self:_class)]} {error "unknown class '$self'"}
#	proc  ${self}_$selector "self $args" "global _; [regsub -all @(\[\\w\\?\]+) $body _(\$self:\\1)]"
#}
#proc def  {class selector args body}  {$class def  $selector $args $body}

proc expand_macros {body} {
	return [regsub -all @(\\\$?\[\\w\\?\]+) $body _(\$self:\\1)]
}

proc def {self selector argnames body} {
	global _ __trace __args
	if {![info exists _($self:_class)]} {error "unknown class '$self'"}
	set name ${self}_$selector
	#if {$name == "Canvas_motion_wrap"} {set body "puts \[time {$body}\]"}
	set argnames [concat [list self] $argnames]
	if {([info exists __trace($self:$selector)] || [info exists __trace(*:$selector)]
         ||  [info exists __trace($self:*)]         || [info exists __trace(*:*)])
         && ![info exists __trace($self:!$selector)]
         && ![info exists __trace(*:!$selector)]
    } {
		proc* $name $argnames "global _; [expand_macros $body]"
	} {
		proc  $name $argnames "global _; [expand_macros $body]"
	}
	set __args($name) $argnames
	#trace add execution ${self}_$selector enter dedebug
}

proc class_new {self {super {Thing}}} {
	global _
	set _($self:_class) Class
	set _($self:_super) $super
	set _($self:subclasses) {}
	foreach sup $super {lappend _($sup:subclasses) $self}
	proc ${self}_new {args} "global _
		set self \[format o%07x \$::nextid\]
		incr ::nextid
		set _(\$self:_class) $self
		setup_dispatcher \$self
		eval [concat \[list \$self init\] \$args]
		return \$self
	"
	proc ${self}_new_as {self args} "global _
		if {\[info exists _(\$self:_class)\]} {error \"object '\\$self' already exists\" }
		set _(\$self:_class) $self
		setup_dispatcher \$self
		eval [concat \[list \$self init\] \$args]
		return \$self
	"
	setup_dispatcher $self
}

# TODO: remove duplicates in lookup
proc lookup_method {class selector methodsv ancestorsv} {
	global _
	upvar $methodsv   methods
	upvar $ancestorsv ancestors
	set name ${class}_$selector
	if {[llength [info procs $name]]} {lappend methods $name}
	lappend ancestors $class
	foreach super $_($class:_super) {lookup_method $super $selector methods ancestors}
}

proc cache_method {class selector} {
	global _ __
	set methods {}; set ancestors {}
	lookup_method $class $selector methods ancestors
	if {![llength $methods]} {set methods [cache_method $class unknown]}
	set __($class:$selector) $methods
	return $methods
}

if {$have_expand} {
	set dispatch {
		set i 0; set class $::_($self:_class)
		if {[catch {set methods $::__($class:$selector)}]} {set methods [cache_method $class $selector]}
		[lindex $methods 0] $self {expand}$args
	}
} else {
	set dispatch {
		set i 0; set class $::_($self:_class)
		if {[catch {set methods $::__($class:$selector)}]} {set methods [cache_method $class $selector]}
		[lindex $methods 0] $self {*}$args
	}
}
proc setup_dispatcher {self} {
	if {[llength [info commands $self]]} {rename $self old_$self}
	proc $self {selector args} [regsub -all {\$self} $::dispatch $self]
}

set super {
	upvar 1 self self
	upvar 2 methods methods i oi
	set i [expr {1+$oi}]
	if {[llength $methods] < $i} {error "no more supermethods"}
}
if {$have_expand} {
	append super {[lindex $methods $i] $self {expand}$args}
} else {
	append super {eval [concat [list [lindex $methods $i] $self] $args]}
}
proc super {args} $super

class_new Thing {}
#set _(Thing:_super) {}
def Thing init {} {}
def Thing == {other} {return [expr ![string compare $self $other]]}

# virtual destructor
def Thing delete {} {
	foreach elem [array names _ $self:*] {array unset _ $elem}
	rename $self ""
}

def Thing vars {} {
	set n [string length $self:]
	set ks [list]
	foreach k [array names _] {
		if {0==[string compare -length $n $self: $k]} {lappend ks [string range $k $n end]}
	}
	return $ks
}

def Thing inspect {} {
	set t [list "#<$self: "]
	foreach k [lsort [$self vars]] {lappend t "$k=[list $@$k] "}
	lappend t ">"
	return [join $t ""]
}

def Thing class {} {return $@_class}

def Thing unknown {args} {
	upvar 1 selector selector class class
	error "no such method '$selector' for object '$self'\nwith ancestors {[Class_ancestors $class]}"
}

class_new Class

# those return only the direct neighbours in the hierarchy
def Class superclasses {} {return $@_super}
def Class subclasses {} {return $@subclasses}

# those look recursively.
def Class ancestors {} {
	#if {[info exists @ancestors]} {}
	set r [list $self]
	foreach super $@_super {eval [concat [list lappend r] [$super ancestors]]}
	return $r
}
def Class <= {class} {return [expr [lsearch [$self ancestors] $class]>=0]}

# note: [luniq] is actually defined in desire.tk
def Class methods {} {
	set methods {}
	set anc [$self ancestors]
	foreach class $anc {
		foreach name [info procs ${class}_*] {
			lappend methods [join [lrange [split $name _] 1 end] _]
		}
	}
	return [luniq [lsort $methods]]
}

# those are static methods, and poe.tcl doesn't distinguish them yet.
def Class new    {   args} {eval [concat [list ${self}_new       ] $args]}
def Class new_as {id args} {eval [concat [list ${self}_new_as $id] $args]}

#-----------------------------------------------------------------------------------#

# this makes me think of Maximus-CBCS...
proc VTgrey    {} {return "\x1b\[0m"}
proc VTred     {} {return "\x1b\[0;1;31m"}
proc VTgreen   {} {return "\x1b\[0;1;32m"}
proc VTyellow  {} {return "\x1b\[0;1;33m"}
proc VTblue    {} {return "\x1b\[0;1;34m"}
proc VTmagenta {} {return "\x1b\[0;1;35m"}
proc VTcyan    {} {return "\x1b\[0;1;36m"}
proc VTwhite   {} {return "\x1b\[0;1;37m"}

proc error_text {} {
	set e $::errorInfo
	regsub -all "    invoked from within\n" $e "" e
	regsub -all "\n    \\(" $e " (" e
	regsub -all {\n[^\n]*procedure \"(::unknown|super)\"[^\n]*\n} $e "\n" e
	regsub -all {\n\"\[lindex \$methods 0\][^\n]*\n} $e "\n" e
	regsub -all {\s*while executing\s*\n} $e "\n" e
	#regsub {\n$} $e "" e
	return $e
}
set suicidal 0
proc error_dump {} {
	puts "[VTred]Exception:[VTgrey] [error_text]"
	if {$::suicidal} {exit 1}
}

proc tracedef {class method {when enter}} {
	global __trace
	set __trace($class:$method) $when
}

proc yell {var key args} {
	global $key
	puts "[VTyellow]HEY! at [info level -1] set $key [list $::_($key)][VTgrey]"
}

proc object_table {} {
	set n 0
	puts "poe.tcl object_table: {"
	foreach o [lsort [array names ::_ *:_class]] {
		set oo [lindex [split $o :] 0]
		set class $::_($o)
		incr by_class($class)
		puts "  $oo is a $class"
		incr n
	}
	puts "} ($n objects)"
	set n 0
	puts "poe.tcl class_stats: {"
	foreach o [array names by_class] {
		puts "  class $o has $by_class($o) objects"
		incr n
	}
	puts "} ($n classes)"
}

proc object_exists {self} {info exists ::_($self:_class)}
