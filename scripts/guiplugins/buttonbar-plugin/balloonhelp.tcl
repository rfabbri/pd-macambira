package provide balloonhelp 0.1

package require Tk

namespace eval balloonhelp {
    set mytoplevel ".balloonhelp"
}

proc ::balloonhelp::setBalloonHelp {w msg args} {
    variable mytoplevel
    array set opt [concat {
        -tag ""
    } $args]
    if {$msg ne ""} then {
        set toolTipScript\
            [list balloonhelp::showBalloonHelp %W [string map {% %%} $msg]]
        set enterScript [list after 1000 $toolTipScript]
        set leaveScript [list after cancel $toolTipScript]
        append leaveScript \n [list after 200 [list destroy $mytoplevel]]
    } else {
        set enterScript {}
        set leaveScript {}
    }
    if {$opt(-tag) ne ""} then {
        switch -- [winfo class $w] {
            Text {
                $w tag bind $opt(-tag) <Enter> $enterScript
                $w tag bind $opt(-tag) <Leave> $leaveScript
            }
            Canvas {
                $w bind $opt(-tag) <Enter> $enterScript
                $w bind $opt(-tag) <Leave> $leaveScript
            }
            default {
                bind $w <Enter> $enterScript
                bind $w <Leave> $leaveScript
            }
        }
    } else {
        bind $w <Enter> $enterScript
        bind $w <Leave> $leaveScript
    }
}

proc ::balloonhelp::showBalloonHelp {w msg} {
    variable mytoplevel
    catch {destroy $mytoplevel}
    toplevel $mytoplevel -bg grey
    wm overrideredirect $mytoplevel yes
    switch -- $::windowingsystem {
        "aqua" {
            wm attributes $mytoplevel -topmost 1 -transparent 1 -alpha 0.8
            ::tk::unsupported::MacWindowStyle style $mytoplevel floating {noTitleBar noShadow}
        }
        "x11" {
            wm attributes $mytoplevel -alpha 0.8
        }
        "win32" {
            wm attributes $mytoplevel -topmost 1 -alpha 0.8
        }
    }
    pack [label $mytoplevel.l -text [subst $msg] -bg yellow -font {Helvetica 9}]\
        -padx 1\
        -pady 1
    set width [expr {[winfo reqwidth $mytoplevel.l] + 2}]
    set height [expr {[winfo reqheight $mytoplevel.l] + 2}]
    set xMax [expr {[winfo screenwidth $w] - $width}]
    set yMax [expr {[winfo screenheight $w] - $height}]
    set x [expr [winfo pointerx $w] + 5]
    set y [expr {[winfo pointery $w] + 10}]
    if {$x > $xMax} then {
        set x $xMax
    }
    if {$y > $yMax} then {
        set y $yMax
    }
    wm geometry $mytoplevel +$x+$y
    set destroyScript [list destroy .balloonhelp]
    bind $mytoplevel <Enter> [list after cancel $destroyScript]
    bind $mytoplevel <Leave> $destroyScript
}

# demo
if false {
    pack [button .b -text tryme -command {puts "you did it!"}]
    balloonhelp::setBalloonHelp .b "Text that describes\nwhat the button does"
    #
    pack [text .t -width 30 -height 5] -expand yes -fill both
    .t insert end abcDEFghi
    .t tag configure yellow -background yellow
    .t tag add yellow 1.1 1.6
    balloonhelp::setBalloonHelp .t "Colorised Text" -tag yellow
    #
    pack [canvas .c] -expand yes -fill both
    set id [.c create rectangle 10 10 100 100 -fill white]
    balloonhelp::setBalloonHelp .c {Geometry: [.c coords $::id]} -tag $id
}
