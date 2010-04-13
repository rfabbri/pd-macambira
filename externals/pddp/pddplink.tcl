
proc pddplink_open {filename dir} {
    if {[string first "://" $filename] > -1} {
        menu_openhtml $filename
    } elseif {[file pathtype $filename] eq "absolute"} {
        menu_openhtml $filename
    } elseif {[file exists [file join $dir $filename]]} {
        menu_doc_open $dir $filename
    } else {
        bell ;# beep on error to provide instant feedback
        pdtk_post "\[pddplink\] ERROR file not found: $filename\n"
    }
}
