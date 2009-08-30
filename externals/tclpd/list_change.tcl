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
        puts stderr "**** called [info level 0]"
        puts stderr ">> inlet 0 is [pd::inlet $self 0]"
        puts stderr ">> inlet 1 is [pd::inlet $self 1]"
        #if {$args != $@curlist} {
        #    set @curlist $args
        #    pd::outlet $self 0 list $@curlist
            #0_bang
        #}
    }

    0_bang {
        puts stderr "**** called [info level 0]"
        puts stderr ">> inlet 0 is [pd::inlet $self 0]"
        puts stderr ">> inlet 1 is [pd::inlet $self 1]"
        #pd::outlet $self 0 list $@curlist
    }

}
