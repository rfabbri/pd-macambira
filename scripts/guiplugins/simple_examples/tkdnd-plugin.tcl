package require tkdnd

dnd bindtarget .pdwindow text/uri-list <Drop> {
    foreach file %D {open_file $file}
}

bind PatchWindow <<Loaded>> \
    {+dnd bindtarget %W text/uri-list <Drop> "pdtk_canvas_makeobjs %W %%D %%X %%Y"}

proc pdtk_canvas_makeobjs {mytoplevel files x y} {
    set c 0
    set rootx [winfo rootx $mytoplevel]
    set rooty [winfo rooty $mytoplevel]
    for {set n 0} {$n < [llength $files]} {incr n} {
        if {[regexp {.*/(.+).pd$} [lindex $files $n] file obj] == 1} {
            ::pdwindow::error " do it $file $obj $x $y $c"
            pdsend "$mytoplevel obj [expr $x - $rootx] [expr $y - $rooty + ($c * 30)] $obj"
            incr c
        }
    } 
}
