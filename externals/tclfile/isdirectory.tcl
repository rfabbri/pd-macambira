package require Tclpd 0.3.0
package require TclpdLib 0.20
package require tclfile

proc isdirectory::constructor {self args} {
    variable ${self}::current_canvas [canvas_getcurrent]
    # set to blank so the var always exists
    variable ${self}::filename {}

    # add second inlet (first created by default)
    pd::add_inlet $self list

    # add outlet
    pd::add_outlet $self list
}

# HOT inlet --------------------------------------------------------------------
proc isdirectory::0_symbol {self args} {
    variable ${self}::filename [tclfile::expand_vars [pd::arg 0 symbol]]
    isdirectory::0_bang $self
}

proc isdirectory::0_anything {self args} {
    variable ${self}::filename [tclfile::expand_vars [tclfile::make_symbol $args]]
    isdirectory::0_bang $self
}

proc isdirectory::0_bang {self} {
    variable ${self}::current_canvas
    variable ${self}::filename
    if {[file pathtype $filename] eq "absolute"} {
        pd::outlet $self 0 float [file isdirectory $filename]
    } else {
        set dir [canvas_getdir $current_canvas]
        pd::outlet $self 0 float [file isdirectory [file join $dir $filename]]
    }
}

# COLD inlet -------------------------------------------------------------------
proc isdirectory::1_symbol {self args} {
    variable ${self}::filename [tclfile::expand_vars [pd::arg 0 symbol]]
}

proc isdirectory::1_anything {self args} {
    variable ${self}::filename [tclfile::expand_vars [tclfile::make_symbol $args]]
}

pd::class isdirectory
