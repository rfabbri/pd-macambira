# META NAME Kiosk
# META DESCRIPTION all windows in fullscreen mode
# META DESCRIPTION main window invisible
# META DESCRIPTION no keybindings

# META AUTHOR IOhannes m zmölnig <zmoelnig@iem.at>


package require Tcl 8.5
package require Tk
package require pdwindow 0.1

namespace eval ::kiosk:: {
    variable ::kiosk::config
}


## default values
set ::kiosk::config(KioskNewWindow) False
set ::kiosk::config(ShowMenu) False
set ::kiosk::config(FullScreen) False
set ::kiosk::config(HideMain) True
set ::kiosk::config(WindowTitle) "Pd KIOSK"
set ::kiosk::config(HidePopup) True
set ::kiosk::config(ScrollBars) False



proc ::kiosk::readconfig {{fname kiosk.cfg}} {
  if {[file exists $fname]} {
    set fp [open $fname r]
  } else {
    return False
  }
  while {![eof $fp]} {
    set data [gets $fp]
    set ::kiosk::config([lindex $data 0]) [lindex $data 1]
  }


 return True
}

# this is just an empty menu
menu .kioskmenu


## KIOSkify a window
proc ::kiosk::makekiosk {mywin} {
## refuse to kioskify the main Pd window
    if { $mywin == ".pdwindow" } { return; }

    puts "KIOSKing $mywin"

#puts "makekiosk $mywin"
#remove menu
    if { $::kiosk::config(ShowMenu) } { } {
        $mywin configure -menu .kioskmenu; 
    }

# make fullscreen
    if { $::kiosk::config(FullScreen) } {
    	wm attributes $mywin -fullscreen 1
    }

# set the title of the window 
# (makes mostly sense in non-fullscren...)
    if { $::kiosk::config(WindowTitle) != "" } {
        wm title $mywin $::kiosk::config(WindowTitle)
    }
}



######################################

## read the default configuration file "kiosk.cfg"
::kiosk::readconfig 


###### do some global KIOSK-settings

## hide the Pd window
if { $::kiosk::config(HideMain) } {
    set ::stderr 1 
    wm state .pdwindow withdraw
}

## don't show popup menu on right-click
if { $::kiosk::config(HidePopup) }  {
 proc ::pdtk_canvas::pdtk_canvas_popup {mytoplevel xcanvas ycanvas hasproperties hasopen} { }
}

if { $::kiosk::config(ScrollBars) } { } {
    proc ::pdtk_canvas::pdtk_canvas_getscroll {tkcanvas} { }
}

# do the KIOSK-setting per existing window (those windows loaded at startup)
foreach kioskwin [array names ::loaded] { 
    ::kiosk::makekiosk $kioskwin 
}

# do the KIOSKification for newly created windows as well
if { $::kiosk::config(KioskNewWindow) }  {
 ## not the most elegant way: KIOSKifying each window as it get's focus
 bind PatchWindow <FocusIn> {
  ::kiosk::makekiosk %W;
 } 

}
