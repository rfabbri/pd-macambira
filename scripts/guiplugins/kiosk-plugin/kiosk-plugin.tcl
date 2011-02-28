# META NAME Kiosk
# META DESCRIPTION all windows in fullscreen mode
# META DESCRIPTION main window invisible
# META DESCRIPTION no keybindings

# META AUTHOR IOhannes m zmölnig <zmoelnig@iem.at>


package require Tcl 8.5
package require Tk
package require pdwindow 0.1

namespace eval ::kiosk:: {
    variable showmenu True
    variable fullscreen False
    variable hidemain False
    variable windowtitle "Pd KIOSK"
    variable hidepopup True
}


## hide the Pd window
if { $::kiosk::hidemain } {
    set ::stderr 1 
    wm state .pdwindow withdraw
}


## don't show popup menu on right-click
if { $::kiosk::hidepopup }  {
 proc ::pdtk_canvas::pdtk_canvas_popup {mytoplevel xcanvas ycanvas hasproperties hasopen} { }
}


# this is just an empty menu
menu .kioskmenu

proc ::kiosk::makekiosk {mywin} {
#puts "makekiosk $mywin"
#remove menu
    if { $::kiosk::showmenu } { } {
        $mywin configure -menu .kioskmenu; 
    }

# make fullscreen
    if { $::kiosk::fullscreen } {
    	wm attributes $mywin -fullscreen 1
    }

# set the title of the window 
# (makes mostly sense in non-fullscren...)
    if { $::kiosk::windowtitle != "" } {
        wm title $mywin $::kiosk::windowtitle
    }
}

foreach kioskwin [array names ::loaded] { 
    ::kiosk::makekiosk $kioskwin 
}

