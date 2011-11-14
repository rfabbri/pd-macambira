package require Tclpd 0.3.0
package require TclpdLib 0.20

proc delete::constructor {self args} {
    if {![namespace exists $self]} {
        namespace eval $self {}
    }
    # set to blank so the var always delete
    variable ${self}::filename {}
    variable ${self}::current_canvas [canvas_getcurrent]

    # add second inlet (first created by default)
    pd::add_inlet $self list
}

proc delete::0_symbol {self args} {
    # HOT inlet
    variable ${self}::filename [pd::arg 0 symbol]
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

proc+ delete::1_symbol {self args} {
    # COLD inlet
    variable ${self}::filename [pd::arg 0 symbol]
}

pd::class delete
