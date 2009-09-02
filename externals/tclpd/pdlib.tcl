# TCL objectized library for PD api
# by Federico Ferri <mescalinum@gmail.com> - (C) 2007-2009

package provide pdlib 0.1

package require Tcl 8.5

set verbose 0

namespace eval ::pd {
    proc error_msg {m} {
        return "pdlib: [uplevel {lindex [info level 0] 0}]: error: $m"
    }

    proc add_inlet {self sel} {
        if $::verbose {post [info level 0]}
        variable _
        tclpd_add_proxyinlet [tclpd_get_instance $self]
    }

    proc add_outlet {self sel} {
        if $::verbose {post [info level 0]}
        if {[lsearch -exact {bang float list symbol} $sel] == -1} {
                return -code error [error_msg "unsupported selector: $sel"]
        }
        variable _
        set o [outlet_new [tclpd_get_object $self] [gensym $sel]]
        lappend _($self:x_outlet) $o
        return $o
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

    # used internally (from dispatcher) to call a class method
    proc call_classmethod {classname self inlet sel args} {
        if $::verbose {post [info level 0]}
        set m_sel "::${classname}_${inlet}_${sel}"
        if {[llength [info commands $m_sel]] > 0} {
            return [$m_sel $self {*}$args]
        }
        set m_any "::${classname}_${inlet}_anything"
        if {[llength [info commands $m_any]] > 0} {
            return [$m_any $self [list symbol $sel] {*}$args]
        }
        post "Tcl class $classname: inlet $inlet: no such method: $sel"
    }

    proc read_class_definition {classname def} {
        set patchable_flag 1
        set noinlet_flag 0

        proc ::${classname}_object_save {self args} {return ""}

        foreach {id arg} $def {
            switch -- $id {
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
            ::${classname}_constructor \$self {*}\$args
            # object dispatcher
            proc ::\$self {inlet selector args} \"
             if \\\$::verbose {::pd::post \\\[info level 0\\\]}
             ::pd::call_classmethod $classname \$self \\\$inlet \\\$selector {*}\\\$args
            \"
            return \$self
        "

        # TODO: c->c_gobj = (typeflag >= CLASS_GOBJ)
        set flag [expr {
            8 * ($noinlet_flag != 0) +
            3 * ($patchable_flag != 0)
        }]

        return $flag
    }

    # this handles the pd::class definition
    proc class {classname def} {
        if $::verbose {post [lrange [info level 0] 0 end-1]}

        set flag [read_class_definition $classname $def]

        # this wraps the call to class_new()
        tclpd_class_new $classname $flag
    }

    proc guiclass {classname def} {
        if $::verbose {post [lrange [info level 0] 0 end-1]}

        set flag [read_class_definition $classname $def]

        # this wraps the call to class_new()
        tclpd_guiclass_new $classname $flag
    }

    # wrapper to post() withouth vargs
    proc post {args} {
        poststring2 [concat {*}$args]
    }

    proc args {} {
        return [uplevel 1 "llength \$args"]
    }

    proc arg {n {assertion any}} {
        set v [uplevel 1 "lindex \$args $n"]
        set i 0
        foreach {selector value} $v {break}
        if {$assertion == {int}} {
            set assertion {float}
            set i 1
        }
        if {$assertion != {any}} {
            if {$selector != $assertion} {
                return -code error "arg #$n is $selector, must be $assertion"
            }
        }
        if {$assertion == {float} && $i && $value != int($value)} {
            return -code error "arg #$n is float, must be int"
        }
        if {$assertion == {float} && $i} {
            return [expr {int($value)}]
        } else {
            return $value
        }
    }

    # mechanism for uploading procs to gui interp, without the hassle of escaping [encoder]
    proc guiproc {name argz body} {
        # upload the decoder
        sys_gui "proc guiproc {name argz body} {set map {}; for {set i 0} {\$i < 256} {incr i} {lappend map %\[format %02x \$i\] \[format %c \$i\]}; foreach x {name argz body} {set \$x \[string map \$map \[set \$x\]\]}; uplevel \[list proc \$name \$argz \$body\]}\n"
        # build the mapping
        set map {}
        for {set i 0} {$i < 256} {incr i} {
            set chr [format %c $i]
            set hex [format %02x $i]
            if {[regexp {[^A-Za-z0-9]} $chr]} {lappend map $chr %$hex}
        }
        # encode data
        foreach x {name argz body} {set $x [string map $map [set $x]]}
        # upload proc
        sys_gui "guiproc $name $argz $body\n"
    }
}

