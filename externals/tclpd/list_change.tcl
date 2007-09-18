source pdlib.tcl

pd::class list_change {
#    inlet float
    outlet list
#    outlet float

    constructor {
        if [pd::args] {
            set n [pd::arg_int 0]
            for {set i 0} {$i < $n} {incr i} {
                pd::add_inlet $self float
            }
        }
        set @curlist {}
    }

    0_list {
        set newlist $args
        if {$newlist != $@curlist} {
            pd::outlet $self 0 list $newlist
        }
        set @curlist $newlist

        pd::outlet $self 1 float [pd::inlet $self 1]
    }

    0_bang {
        pd::post "right value is: [pd::inlet $self 1]"
    }
}

