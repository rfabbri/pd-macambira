# TCL objectized library for PD api
# by Federico Ferri <mescalinum@gmail.com> - 2007

package provide pdlib 0.1

package require Tcl 8.5

set verbose 1

namespace eval ::pd {
    proc error_msg {m} {
        return "pdlib: [uplevel {lindex [info level 0] 0}]: error: $m"
    }

    # create additional inlets with this
    proc add_inlet {self sel} {
        if $::verbose {post [info level 0]}
        variable _
        switch -- $sel {
            float {
                set ptr [new_t_float]
                lappend _($self:p_inlet) $ptr
                lappend _($self:t_inlet) "float"
                #lappend _($self:x_inlet) [floatinlet_new [tclpd_get_object $self] $ptr]
            }
            symbol {
                set ptr [new_t_symbol]
                lappend _($self:p_inlet) $ptr
                lappend _($self:t_inlet) "symbol"
                #lappend _($self:x_inlet) [symbolinlet_new [tclpd_get_object $self] $ptr]
            }
            list {
                set ptr [new_t_proxyinlet]
                proxyinlet_init $ptr
                #proxyinlet_list $ptr [gensym list] 2 {{symbol foo} {symbol bar}}
                #puts "(t_proxyinlet) $ptr cget -pd = "
                #puts "                               [$ptr cget -pd]"
                #inlet_new [tclpd_get_object $self] [$ptr cget -pd] 0 {}
                # I HATE SWIG
                tclpd_add_proxyinlet [tclpd_get_object $self] $ptr
                lappend _($self:p_inlet) $ptr
                lappend _($self:t_inlet) "list"
            }
            DISABLED__pointer {
            ## need to think more about this
            #    set ptr [new_t_pointer]
            #    lappend _($self:p_inlet) $ptr
            ##    lappend _($self:x_inlet) [pointerinlet_new [tclpd_get_object $self] $ptr]
            }
            default {
                return -code error [error_msg "unsupported selector: $sel"]
            }
        }
        #return [lindex $_($self:x_inlet) end]
    }

    # get the value of a given inlet (inlets numbered starting from 1)
    proc inlet {self n} {
        if {$::verbose} {post [info level 0]}
        if {$n <= 0} {return {}}
        if {![info exists _($self:p_inlet)] ||
            $n >= [llength $_($self:p_inlet)]} {
            return -code error [error_msg "no such inlet: $n"]
        }
        variable _
        set p_inlet [lindex $_($self:p_inlet) [expr $n-1]]
        if {$_($self:t_inlet) == {list}} {
            return [$p_inlet argv]
        } else {
            return [$p_inlet value]
        }
    }

    # used in object constructor for adding inlets
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
                return -code error [error_msg "unsupported selector: $sel"]
            }
        }
        return [lindex $_($self:x_outlet) end]
    }

    # used inside class for outputting some value
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
                return -code error [error_msg "unknown selector: $sel"]
            }
        }
    }

    # used in object constructor to create inlets (internal method)
    proc create_iolets {cn self} {
        if $::verbose {post [info level 0]}
        variable class_db
        variable _
        set _($self:p_inlet) {}
        #set _($self:x_inlet) {}
        set _($self:t_inlet) {}
        set _($self:x_outlet) {}
        for {set i 0} {$i < [llength $class_db($cn:d_inlet)]} {incr i} {
            add_inlet $self [lindex $class_db($cn:d_inlet) $i]
        }
        for {set i 0} {$i < [llength $class_db($cn:d_outlet)]} {incr i} {
            add_outlet $self [lindex $class_db($cn:d_outlet) $i]
        }
    }

    # add a class method (that is: a proc named <class>_<sel>)
    proc call_classmethod {classname self sel args} {
        if $::verbose {post [info level 0]}
        set m "${classname}_${sel}"
        if {[llength [info commands "::$m"]] > 0} {
            return [$m $self {*}$args]
        }
    }

    # this handles the pd::class definition
    proc class {classname def} {
        if $::verbose {post [lrange [info level 0] 0 end-1]}
        variable class_db
        array set class_db {}
        set class_db($classname:d_inlet) {}
        set class_db($classname:d_outlet) {}
        # strip comments:
        set def2 [regsub -all -line {#.*$} $def {}]
        set patchable_flag 1
        set noinlet_flag 0
        foreach {id arg} $def2 {
            switch -- $id {
                inlet {
                    lappend class_db($classname:d_inlet) $arg
                }
                outlet {
                    lappend class_db($classname:d_outlet) $arg
                }
                patchable {
                    if {$arg != 0 && $arg != 1} {
                        return -code error [error_msg "patchable must be 0/1"]
                    }
                    set patchable_flag $arg
                }
                noinlet {
                    if {$arg != 0 && $arg != 1} {
                        return -code error [error_msg "noinlet must be 0/1"]
                    }
                    set noinlet_flag $arg
                }
                default {
                    proc ::${classname}_${id} {self args} [concat "global _;" [regsub -all @(\\\$?\[\\w\\?\]+) $arg _(\$self:\\1)]]
                }
            }
        }

        # class level dispatcher (sort of class constructor)
        proc ::$classname {self args} "
            if \$::verbose {::pd::post \[info level 0\]}
            ::pd::create_iolets $classname \$self
            ::pd::call_classmethod $classname \$self constructor {*}\$args
            # object dispatcher
            proc ::\$self {selector args} \"
             if \\\$::verbose {::pd::post \\\[info level 0\\\]}
             ::pd::call_classmethod $classname \$self \\\$selector {*}\\\$args
            \"
            return \$self
        "

        # TODO: c->c_gobj = (typeflag >= CLASS_GOBJ)
        set flag [expr {
            8 * ($noinlet_flag != 0) +
            3 * ($patchable_flag != 0)
        }]

        # this wraps the call to class_new()
        tclpd_class_new $classname $flag
    }

    # wrapper to post() withouth vargs
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

