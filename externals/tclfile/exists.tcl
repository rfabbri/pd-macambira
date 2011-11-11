package require Tclpd 0.2.3
package require TclpdLib 0.19

proc exists::constructor {self args} {
    if {![namespace exists $self]} {
        namespace eval $self {}
    }
    variable ${self}::current_canvas [canvas_getcurrent]
    # set to blank so the var always exists
    variable ${self}::filename {}

    # add second inlet (first created by default)
    pd::add_inlet $self list

    # add outlet
    pd::add_outlet $self list
}

proc exists::0_symbol {self args} {
    # HOT inlet
    variable ${self}::filename [pd::arg 0 symbol]
    exists::0_bang $self
}

proc exists::0_bang {self} {
    variable ${self}::current_canvas
    variable ${self}::filename
    if {[file pathtype $filename] eq "absolute"} {
        pd::outlet $self 0 float [file exists $filename]
    } else {
        set dir [[canvas_getdir $current_canvas] cget -s_name]
        pd::outlet $self 0 float [file exists [file join $dir $filename]]
    }
}

proc exists::1_symbol {self args} {
    # COLD inlet
    variable ${self}::filename [pd::arg 0 symbol]
}

pd::class exists
