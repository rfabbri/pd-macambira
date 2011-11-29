
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

# expand things like ~ $HOME $ProgramFiles
proc tclfile::expand_vars {filename} {
    set sub $filename
    foreach var [regexp -all -inline -- {\$\w+} $filename] {
        regexp -- {\$(\w+)} $var -> varname
        if {[catch {set got $::env($varname)} fid]} {
            #puts stderr "caught $fid"
        } else {
            set sub [string map [list "\$$varname" $got] $filename]
            # TODO this should really be a regex that properly
            # recognizes {} around the symbol as separate from just {}
            # used in a filename.  But first, Pd will need a full
            # escaping mechanism so it can allow {}
            #set sub [regsub "\(.+\)HOME\(.+\)" $sub "==\1==$got==\2=="]
        }
    }
    return [string map {\{ "" \} ""} $sub]
}
