package require Tclpd 0.3.0
package require TclpdLib 0.20
package require tclfile

proc mkdir::constructor {self args} {
    variable ${self}::current_canvas [canvas_getcurrent]
    # set to blank so the var always exists
    variable ${self}::filename {}

    # add second inlet (first created by default)
    pd::add_inlet $self list
}

# HOT inlet --------------------------------------------------------------------
proc mkdir::0_symbol {self args} {
    variable ${self}::filename [pd::arg 0 symbol]
    mkdir::0_bang $self
}

proc mkdir::0_anything {self args} {
    variable ${self}::filename [tclfile::make_symbol $args]
    mkdir::0_bang $self
}

proc mkdir::0_bang {self} {
    variable ${self}::current_canvas
    variable ${self}::filename
    if {[file pathtype $filename] eq "absolute"} {
        file mkdir $filename
    } else {
        set dir [canvas_getdir $current_canvas]
        file mkdir [file join $dir $filename]
    }
}

# COLD inlet -------------------------------------------------------------------
proc mkdir::1_symbol {self args} {
    variable ${self}::filename [pd::arg 0 symbol]
}

proc mkdir::1_anything {self args} {
    variable ${self}::filename [tclfile::make_symbol $args]
}

pd::class mkdir
