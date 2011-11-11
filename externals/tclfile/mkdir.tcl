package require Tclpd 0.2.3
package require TclpdLib 0.19

proc mkdir::constructor {self args} {
    if {![namespace exists $self]} {
        namespace eval $self {}
    }
    # set to blank so the var always mkdir
    variable ${self}::filename {}
    variable ${self}::current_canvas [canvas_getcurrent]

    # add second inlet (first created by default)
    pd::add_inlet $self list
}

proc mkdir::0_symbol {self args} {
    # HOT inlet
    variable ${self}::filename [pd::arg 0 symbol]
    mkdir::0_bang $self
}

proc mkdir::0_bang {self} {
    variable ${self}::current_canvas
    variable ${self}::filename
    if {[file pathtype $filename] eq "absolute"} {
        file mkdir $filename
    } else {
        set dir [[canvas_getdir $current_canvas] cget -s_name]
        file mkdir [file join $dir $filename]
    }
}

proc+ mkdir::1_symbol {self args} {
    # COLD inlet
    variable ${self}::filename [pd::arg 0 symbol]
}

pd::class mkdir
