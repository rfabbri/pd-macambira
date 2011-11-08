package require Tclpd 0.2.3
package require TclpdLib 0.19

proc+ exists::constructor {self args} {
    # add second inlet (first created by default)
    pd::add_inlet $self list

    # add outlet
    pd::add_outlet $self list
}

proc+ exists::0_symbol {self args} {
    # HOT inlet
    set @filename [lindex {*}$args 1]
    exists::0_bang $self
}

proc+ exists::0_anything {self args} {
    # HOT inlet
    set @filename [lindex {*}$args 1]
    pd::post "anything: $@filename"
    exists::0_bang $self
}

proc+ exists::0_bang {self} {
    if {$@filename == {}} return
    pd::outlet $self 0 [list float [file exists $@filename]]
}

proc+ exists::1_anything {self args} {
    # COLD inlet
    set @filename [lindex {*}$args 1]
}

pd::class exists
