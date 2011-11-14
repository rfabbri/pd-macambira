package require Tclpd 0.3.0
package require TclpdLib 0.20
package require tclfile

proc isfile::constructor {self args} {
    variable ${self}::current_canvas [canvas_getcurrent]
    # set to blank so the var always exists
    variable ${self}::filename {}

    # add second inlet (first created by default)
    pd::add_inlet $self list

    # add outlet
    pd::add_outlet $self list
}

# HOT inlet --------------------------------------------------------------------
proc isfile::0_symbol {self args} {
    variable ${self}::filename [pd::arg 0 symbol]
    isfile::0_bang $self
}

proc isfile::0_anything {self args} {
    variable ${self}::filename [tclfile::make_symbol $args]
    isfile::0_bang $self
}

proc isfile::0_bang {self} {
    variable ${self}::current_canvas
    variable ${self}::filename
    if {[file pathtype $filename] eq "absolute"} {
        pd::outlet $self 0 float [file isfile $filename]
    } else {
        set dir [canvas_getdir $current_canvas]
        pd::outlet $self 0 float [file isfile [file join $dir $filename]]
    }
}

# COLD inlet -------------------------------------------------------------------
proc isfile::1_symbol {self args} {
    variable ${self}::filename [pd::arg 0 symbol]
}

proc isfile::1_anything {self args} {
    variable ${self}::filename [tclfile::make_symbol $args]
}

pd::class isfile
