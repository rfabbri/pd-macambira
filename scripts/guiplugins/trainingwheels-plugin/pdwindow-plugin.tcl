
pdtk_post "==========================asdfasdfasdfasfd=========="

package provide pdwindow_trainingwheels 0.1

package require pdwindow 0.1

namespace eval ::pdwindow_trainingwheels:: {
    variable printout_buffer ""
    variable pdwindow_search_index
    variable history_position 0
    variable linecolor 0 ;# is toggled to alternate text line colors
    variable maxverbosity 5
    variable defaultverbosity 4

    variable lasttag "line0"

    namespace export create_window
    namespace export pdtk_pd_dsp
}

#--busy cursor support---------------------------------------------------------#

# grab focus on part of the Pd window when Pd is busy
rename ::pdwindow::busygrab {}
proc ::pdwindow::busygrab {} {
    # set the mouse cursor to look busy and grab focus so it stays that way    
    .pdwindow.text configure -cursor watch
    grab set .pdwindow.text
}

# release focus on part of the Pd window when Pd is finished
rename ::pdwindow::busyrelease {}
proc ::pdwindow::busyrelease {} {
    .pdwindow.text configure -cursor xterm
    grab release .pdwindow.text
}

#--bindings specific to the Pd window------------------------------------------#

proc ::pdwindow_trainingwheels::pdwindow_bindings {} {
    # these bindings are for the whole Pd window, minus the Tcl entry
    foreach window {.pdwindow.text .pdwindow.header} {
        bind $window <$::modifier-Key-x> "tk_textCut .pdwindow.text"
        bind $window <$::modifier-Key-c> "tk_textCopy .pdwindow.text"
        bind $window <$::modifier-Key-v> "tk_textPaste .pdwindow.text"
    }
    # Select All doesn't seem to work unless its applied to the whole window
    bind .pdwindow <$::modifier-Key-a> ".pdwindow.text tag add sel 1.0 end"
    # the "; break" part stops executing another binds, like from the Text class
    bind .pdwindow.text <Key-Tab> "focus .pdwindow.tcl.entry; break"

    # these don't do anything in the Pd window, so alert the user, then break
    # so no more bindings run
    bind .pdwindow <$::modifier-Key-s> "bell; break"
    bind .pdwindow <$::modifier-Shift-Key-S> "bell; break"
    bind .pdwindow <$::modifier-Key-p> "bell; break"

    # ways of hiding/closing the Pd window
    if {$::windowingsystem eq "aqua"} {
        # on Mac OS X, you can close the Pd window, since the menubar is there
        bind .pdwindow <$::modifier-Key-w>   "wm withdraw .pdwindow"
        wm protocol .pdwindow WM_DELETE_WINDOW "wm withdraw .pdwindow"
    } else {
        # TODO should it possible to close the Pd window and keep Pd open?
        bind .pdwindow <$::modifier-Key-w>   "wm iconify .pdwindow"
        wm protocol .pdwindow WM_DELETE_WINDOW "pdsend \"pd verifyquit\""
    }
}

#--create the window-----------------------------------------------------------#

proc ::pdwindow_trainingwheels::create_window {} {
    set ::loaded(.pdwindow) 0

    # colorize by class before creating anything
    option add *PdWindow*Entry.highlightBackground "grey" startupFile
    option add *PdWindow*Frame.background "grey" startupFile
    option add *PdWindow*Label.background "grey" startupFile
    option add *PdWindow*Checkbutton.background "grey" startupFile
    option add *PdWindow*Menubutton.background "grey" startupFile
    option add *PdWindow*Text.background "white" startupFile
    option add *PdWindow*Entry.background "white" startupFile

    toplevel .pdwindow -class PdWindow
    wm title .pdwindow [_ "Pd window"]
    set ::windowname(.pdwindow) [_ "Pd window"]
    if {$::windowingsystem eq "x11"} {
        wm minsize .pdwindow 400 75
    } else {
        wm minsize .pdwindow 400 51
    }
    wm geometry .pdwindow =500x400+20+50
    .pdwindow configure -menu .menubar

    frame .pdwindow.header -borderwidth 1 -relief flat -background lightgray
    pack .pdwindow.header -side top -fill x

    label .pdwindow.header.label -background lightgray -justify right \
        -text [_ "The DSP needs to be on in order to hear sound:"]
    pack .pdwindow.header.label -side left -expand 1 -fill x -anchor e

    checkbutton .pdwindow.header.dsp -text [_ "DSP"] -variable ::dsp \
        -font {$::font_family 18 bold} -takefocus 1 -background lightgray \
        -borderwidth 0  -command {pdsend "pd dsp $::dsp"}
    pack .pdwindow.header.dsp -side right -fill y -anchor e -padx 15 -pady 0

# TODO this should use the pd_font_$size created in pd-gui.tcl    
    text .pdwindow.text -relief raised -bd 2 -font {-size 10} \
        -highlightthickness 0 -borderwidth 1 -relief flat \
        -yscrollcommand ".pdwindow.scroll set" -width 60 \
        -undo true -autoseparators true -maxundo -1 -takefocus 0   
    scrollbar .pdwindow.scroll -command ".pdwindow.text yview"
    pack .pdwindow.scroll -side right -fill y
    pack .pdwindow.text -side right -fill both -expand 1
    raise .pdwindow
    focus .pdwindow.text
    pdwindow_bindings

    set ::loaded(.pdwindow) 1

    # wait until .pdwindow.tcl.entry is visible before opening files so that
    # the loading logic can grab it and put up the busy cursor
    tkwait visibility .pdwindow.text
}

destroy .pdwindow
set ::loaded(.pdwindow) 0
::pdwindow_trainingwheels::create_window
