# META NAME Kiosk
# META DESCRIPTION all windows in fullscreen mode
# META DESCRIPTION main window invisible
# META DESCRIPTION no keybindings

# META AUTHOR IOhannes m zmölnig <zmoelnig@iem.at>


package require Tcl 8.5
package require Tk
package require pdwindow 0.1

namespace eval ::kiosk:: {
    variable showmenu False
    variable fullscreen True
    variable hidemain True
    variable windowtitle "FOO"
}


## hide the Pd window
if { $::kiosk::hidemain } {
    set ::stderr 1 
    wm state .pdwindow withdraw
}


# this is just an empty menu
menu .kioskmenu


proc ::kiosk::makekiosk {mywin} {
puts "makekiosk $mywin"
#remove menu
    if { $::kiosk::showmenu } { } {
        $mywin configure -menu .kioskmenu; 
    }

# make fullscreen
    if { $::kiosk::fullscreen } {
    	wm attributes $mywin -fullscreen 1
    }


    if { info exists ::kiosk::windowtitle $::kiosk::windowtitle } {
}

# it seems like this is getting not called for initially opened windows
# i guess it is like that:
#  pd starts up
#  pd sends initial data to the GUI
#  the guiplugins are invoked (too late to do anything with existing windows)

#proc ::pd_bindings::patch_bindings {mytoplevel} {
#puts "foo $mytoplevel"
#	wm attributes $mytoplevel -fullscreen 1
#}



foreach kioskwin [array names ::loaded] { 
    ::kiosk::makekiosk $kioskwin 
}

#bind PatchWindow <FocusIn> { 
#    makekiosk %W;
#}

