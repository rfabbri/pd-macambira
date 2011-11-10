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
    set @filename [pd::arg 0 symbol]
    exists::0_bang $self
}

proc+ exists::0_bang {self} {
    if {$@filename == {}} return
    pd::outlet $self 0 float [file exists $@filename]
}

proc+ exists::1_symbol {self args} {
    # COLD inlet
    set @filename [pd::arg 0 symbol]
}

pd::class exists
