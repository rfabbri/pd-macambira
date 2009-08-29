source pdlib.tcl

pd::class list_change {
    # first 'hot' inlet is created by default

    # add 'cold' inlet:
    inlet list

    outlet list

    constructor {
        #pd::add_inlet $self float

        set @curlist {}
    }

    0_list {
        if {$args != $@curlist} {
            set @curlist $args
            pd::outlet $self 0 list $@curlist
            #0_bang
        }
    }

    0_bang {
        pd::outlet $self 0 list $@curlist
    }
}
