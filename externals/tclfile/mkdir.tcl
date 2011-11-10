package require Tclpd 0.2.3
package require TclpdLib 0.19

pd::post "Loading mkdir.tcl"

proc+ mkdir::constructor {self args} {
    # add outlet
    pd::add_outlet $self list
}

# HOT inlet
proc+ mkdir::0_list {self args} {
    pd::post "tclfile/mkdir: list"
    #pd::outlet $self 0 list $@curlist
}

# HOT inlet
proc+ mkdir::0_symbol {self args} {
    pd::post "tclfile/mkdir: symbol"
#    pd::outlet $self 0 list $@curlist
}

proc+ mkdir::0_bang {self} {
    pd::post "tclfile/mkdir: bang"
#    pd::outlet $self 0 list $@curlist
}

pd::post "pd::class mkdir"
pd::class mkdir

pd::post "Finished reading mkdir.tcl"
