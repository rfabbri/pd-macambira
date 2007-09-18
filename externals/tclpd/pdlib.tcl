# TCL objectized library for PD api
# by Federico Ferri <mescalinum@gmail.com> - 2007

package provide pdlib 0.1

package require Tcl 8.5

set verbose 0

namespace eval ::pd {

    proc add_inlet {self sel} {
        if $::verbose {post [info level 0]}
        variable _
        switch -- $sel {
            float {
                set ptr [new_t_float]
                lappend _($self:p_inlet) $ptr
                lappend _($self:x_inlet) [floatinlet_new [tclpd_get_object $self] $ptr]
            }
            symbol {
                set ptr [new_t_symbol]
                lappend _($self:p_inlet) $ptr
                lappend _($self:x_inlet) [symbolinlet_new [tclpd_get_object $self] $ptr]
            }
            default {
                post "inlet creation error: unsupported selector: $sel"
                return {}
            }
        }
        return [lindex $_($self:x_inlet) end]
    }

    proc inlet {self n} {
        if {$::verbose} {post [info level 0]}
        if {$n <= 0} {return {}}
        if {![info exists _($self:p_inlet)] ||
            $n >= [llength $_($self:p_inlet)]} {
            return -code error "pdlib: error: no such inlet: $n"
        }
        variable _
        return [[lindex $_($self:p_inlet) [expr $n-1]] value]
    }

    proc add_outlet {self sel} {
        if $::verbose {post [info level 0]}
        variable _
        switch -- $sel {
            float {
                lappend _($self:x_outlet) \
                    [outlet_new [tclpd_get_object $self] [gensym "float"]]
            }
            symbol {
                lappend _($self:x_outlet) \
                    [outlet_new [tclpd_get_object $self] [gensym "symbol"]]
            }
            list {
                lappend _($self:x_outlet) \
                    [outlet_new [tclpd_get_object $self] [gensym "list"]]
            }
            default {
                return -code error \
                "pdlib: outlet creation error: unsupported selector: $sel"
            }
        }
        return [lindex $_($self:x_outlet) end]
    }

    proc outlet {self n sel args} {
        if $::verbose {post [info level 0]}
        variable _
        set outlet [lindex $_($self:x_outlet) $n]
        switch -- $sel {
            float {
                set v [lindex $args 0]
                outlet_float $outlet $v
            }
            symbol {
                set v [lindex $args 0]
                outlet_symbol $outlet $v
            }
            list {
                set v [lindex $args 0]
                set sz [llength $v]
                set aa [new_atom_array $sz]
                for {set i 0} {$i < $sz} {incr i} {
                    set_atom_array $aa $i [lindex $v $i]
                }
                outlet_list $outlet [gensym "list"] $sz $aa
                delete_atom_array $aa $sz
            }
            bang {
                outlet_bang $outlet
            }
            default {
                return -code error "pdlib: outlet: unknown selector: $sel"
            }
        }
    }

    proc create_iolets {cn self} {
        if $::verbose {post [info level 0]}
        variable class_db
        variable _
        set _($self:p_inlet) {}
        set _($self:x_inlet) {}
        set _($self:x_outlet) {}
        for {set i 0} {$i < [llength $class_db($cn:d_inlet)]} {incr i} {
            add_inlet $self [lindex $class_db($cn:d_inlet) $i]
        }
        for {set i 0} {$i < [llength $class_db($cn:d_outlet)]} {incr i} {
            add_outlet $self [lindex $class_db($cn:d_outlet) $i]
        }
    }

    proc call_classmethod {classname self sel args} {
        if $::verbose {post [info level 0]}
        set m "${classname}_${sel}"
        if {[llength [info commands "::$m"]] > 0} {
            return [$m $self {*}$args]
        }
    }

    proc class {classname def} {
        variable class_db
        array set class_db {}
        set class_db($classname:d_inlet) {}
        set class_db($classname:d_outlet) {}
        set def2 [regsub -all -line {#.*$} $def {}]
        foreach {id arg} $def2 {
            switch -- $id {
                inlet {
                    lappend class_db($classname:d_inlet) $arg
                }
                outlet {
                    lappend class_db($classname:d_outlet) $arg
                }
                default {
                    proc ::${classname}_${id} {self args} \
                        "global _; [expand_macros $arg]"
                }
            }
        }

        proc ::$classname {self args} "
            ::pd::create_iolets $classname \$self
            ::pd::call_classmethod $classname \$self constructor {*}\$args
            proc ::\$self {selector args} \"
             ::pd::call_classmethod $classname \$self \\\$selector {*}\\\$args
            \"
            return \$self
        "

        tclpd_class_new $classname 3
    }

    proc expand_macros {body} {
        # from poe.tcl by Mathieu Bouchard
        return [regsub -all @(\\\$?\[\\w\\?\]+) $body _(\$self:\\1)]
    }

    proc post {args} {
        poststring2 [concat {*}$args]
    }

    proc assert= {a b} {
        if {$a != $b} {
            post "ASSERTION FAILED: \"$a\" == \"$b\""
            return 0
        }
        return 1
    }

    proc args {} {
        return [uplevel 1 "llength \$args"]
    }

    proc arg_float {n} {
        set v [uplevel 1 "lindex \$args $n"]
        foreach {selector value} $v {break}
        assert= $selector "float"
        return $value
    }

    proc arg_int {n} {
        set v [uplevel 1 "lindex \$args $n"]
        foreach {selector value} $v {break}
        assert= $selector "float"
        return [expr {int($value)}]
    }

    proc arg_symbol {n} {
        set v [uplevel 1 "lindex \$args $n"]
        foreach {selector value} $v {break}
        assert= $selector "symbol"
        return $value
    }

}

