package require Tclpd 0.2.2
package require TclpdLib 0.17

pd::class binbuf-test {
    constructor {
        pd::add_outlet $self list
    }

    destructor {
    }

    0_bang {
        set binbuf [tclpd_get_object_binbuf $self]
        pd::outlet $self 0 list [list "symbol binbuf" "symbol $binbuf"]

        set len [binbuf_getnatom $binbuf]
        pd::outlet $self 0 list [list "symbol len" "symbol $len"]

        #for {set i 0} {$i < $len} {incr i} {
        #    pd::post $i:[tclpd_binbuf_get_atom $binbuf $i]
        #}
    }
}
