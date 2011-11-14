package require Tclpd 0.2.3

package provide tclfile 0.1
namespace eval ::tclfile {
}

proc tclfile::make_symbol {argslist} {
    set output [pd::strip_selectors $argslist]
    set selector [lindex $output 0]
    if {$selector eq "list" || $selector eq "float"} {
        set output [lrange $output 1 end]
    }
    return $output
}
