package require Tclpd 0.2.2
package require TclpdLib 0.18

pd::class binbuf-test {
    constructor {
        pd::add_outlet $self list
    }

    destructor {
    }

    0_bang {
        pd::outlet $self 0 list [pd::get_binbuf $self]
    }
}
