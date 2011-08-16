# this plugin adds arrowheads to the ends of cords when in editmode in
# order to show the direction that the messages are flowing

proc add_arrows_to_cords {mytoplevel} {
    if { ! [winfo exists $mytoplevel] } {return}
    if {$mytoplevel eq ".pdwindow"} {return}
    set tkcanvas [tkcanvas_name $mytoplevel]
    if {$::editmode($mytoplevel) == 1} {
        $tkcanvas itemconfigure cord -arrow last
    } else {
        $tkcanvas itemconfigure cord -arrow none
    }
}


bind PatchWindow <<EditMode>> {+add_arrows_to_cords %W}
