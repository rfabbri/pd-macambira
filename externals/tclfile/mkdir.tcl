package require Tclpd 0.2.3
package require TclpdLib 0.19

puts stderr "Loading mkdir.tcl"

if {[namespace exists mkdir]} {return}
namespace eval mkdir {
}

proc+ mkdir::constructor {self args} {
    # add outlet
    pd::add_outlet $self list
}

# HOT inlet
proc+ mkdir::0_list {self args} {
    puts stderr "tclfile/mkdir: list"
    #pd::outlet $self 0 list $@curlist
}

# HOT inlet
proc+ mkdir::0_symbol {self args} {
    puts stderr "tclfile/mkdir: symbol"
#    pd::outlet $self 0 list $@curlist
}

proc+ mkdir::0_bang {self} {
    puts stderr "tclfile/mkdir: bang"
#    pd::outlet $self 0 list $@curlist
}

puts stderr "pd::class mkdir"
pd::class mkdir

puts stderr "Finished reading mkdir.tcl"
