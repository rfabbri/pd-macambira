# this script makes it so that the cords are hidden when not in edit mode

proc set_cords_by_editmode {mytoplevel eventname} {
    if {$mytoplevel eq ".pdwindow"} {return}
    if { ! [winfo exists $mytoplevel] } {return}
    set tkcanvas [tkcanvas_name $mytoplevel]
    if {$::editmode($mytoplevel) == 1} {
        $tkcanvas raise cord
    } else {
        $tkcanvas lower cord
    }
}

bind PatchWindow <<EditMode>> {+set_cords_by_editmode %W editmode}
bind PatchWindow <<Loaded>> {+set_cords_by_editmode %W loaded}
