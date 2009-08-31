source pdlib.tcl

pd::class list_change {

    constructor {
        # add second inlet (first created by default)
        pd::add_inlet $self list

        # add outlet
        pd::add_outlet $self list

        set @curlist {}
    }

    0_list {
        # HOT inlet
        if {$args != $@curlist} {
            set @curlist $args
            pd::outlet $self 0 list $@curlist
        }
    }

    0_bang {
        pd::outlet $self 0 list $@curlist
    }

    1_list {
        # COLD inlet
        set @curlist $args
    }
}
