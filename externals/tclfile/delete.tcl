package require Tclpd 0.3.0
package require TclpdLib 0.20
package require tclfile

proc delete::constructor {self args} {
    variable ${self}::current_canvas [canvas_getcurrent]
    # set to blank so the var always exists
    variable ${self}::filename {}

    # add second inlet (first created by default)
    pd::add_inlet $self list
}

# HOT inlet --------------------------------------------------------------------
proc delete::0_symbol {self args} {
    variable ${self}::filename [tclfile::expand_vars [pd::arg 0 symbol]]
    delete::0_bang $self
}

proc delete::0_anything {self args} {
    variable ${self}::filename [tclfile::expand_vars [tclfile::make_symbol $args]]
    delete::0_bang $self
}

proc delete::0_bang {self} {
    variable ${self}::current_canvas
    variable ${self}::filename
    if {[file pathtype $filename] eq "absolute"} {
        file delete -- $filename
    } else {
        set dir [canvas_getdir $current_canvas]
        file delete -- [file join $dir $filename]
    }
}

# COLD inlet -------------------------------------------------------------------
proc delete::1_symbol {self args} {
    variable ${self}::filename [tclfile::expand_vars [pd::arg 0 symbol]]
}

proc delete::1_anything {self args} {
    variable ${self}::filename [tclfile::expand_vars [tclfile::make_symbol $args]]
}

pd::class delete
