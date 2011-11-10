package require Tclpd 0.2.3
package require TclpdLib 0.19

proc+ mkdir::constructor {self args} {
    set @current_canvas [canvas_getcurrent]
    # set to blank so the var always mkdir
    set @filename {}

    # add second inlet (first created by default)
    pd::add_inlet $self list
}

proc+ mkdir::0_symbol {self args} {
    # HOT inlet
    set @filename [pd::arg 0 symbol]
    mkdir::0_bang $self
}

proc+ mkdir::0_bang {self} {
    if {[file pathtype $@filename] eq "absolute"} {
        file mkdir $@filename
    } else {
        set dir [[canvas_getdir $@current_canvas] cget -s_name]
        file mkdir [file join $dir $@filename]
    }
}

proc+ mkdir::1_symbol {self args} {
    # COLD inlet
    set @filename [pd::arg 0 symbol]
}

pd::class mkdir
