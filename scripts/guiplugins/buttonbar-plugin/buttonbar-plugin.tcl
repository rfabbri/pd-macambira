# this plugin creates a buttonbar on a patch window when that patch
# window is in Edit Mode

# this GUI plugin removes the menubars from any patch window that is
# not in Edit Mode.  Also, if a patch is switched to Run Mode, the
# menubar will be removed.

# TODO make it scroll the patch so it acts as an overlay

lappend ::auto_path $::current_plugin_loadpath

package require base64
package require tooltip 1.4.2

namespace eval buttonbar {
}

proc ::buttonbar::make_pd_button {tkpathname name description} {
    button $tkpathname.$name -image buttonbar::$name \
        -relief flat -borderwidth 0 -highlightthickness 0 \
        -highlightcolor grey -highlightbackground grey -padx 0 -pady 0 \
        -command "menu_send_float \$::focused_window $name 0"
    pack $tkpathname.$name -side left -padx 0 -pady 0
    ::tooltip::tooltip $tkpathname.$name $description
}

proc ::buttonbar::make_iemgui_button {tkpathname name description} {
    button $tkpathname.$name -image buttonbar::$name \
        -relief sunken -borderwidth 0 -highlightthickness 0 \
        -highlightcolor grey -highlightbackground grey -padx 0 -pady 0 \
        -command "menu_send \$::focused_window $name"
    pack $tkpathname.$name -side left -padx 0 -pady 0
    ::tooltip::tooltip $tkpathname.$name $description
}

proc ::buttonbar::showhide_buttonbar {mytoplevel} {
    set tkcanvas [tkcanvas_name $mytoplevel]
    set buttonbar_pathname $mytoplevel.buttonbar
    if { ! [winfo exists $buttonbar_pathname]} {
        frame $buttonbar_pathname -cursor arrow -background grey \
            -pady 0
        make_pd_button $buttonbar_pathname obj {Object (obj)}
        make_pd_button $buttonbar_pathname msg {Message (msg)}
        make_pd_button $buttonbar_pathname floatatom {Number (floatatom)}
        make_pd_button $buttonbar_pathname symbolatom {Symbol (symbolatom)}
        make_pd_button $buttonbar_pathname text {Comment}
        make_iemgui_button $buttonbar_pathname bng {Bang Button [bng]}
        make_iemgui_button $buttonbar_pathname toggle {Toggle [tgl]}
        make_iemgui_button $buttonbar_pathname numbox {Number2 [my_numbox]}
        make_iemgui_button $buttonbar_pathname hslider {Horizontal Slider [hslider]}
        make_iemgui_button $buttonbar_pathname vslider {Verical Slider [vslider]}
        make_iemgui_button $buttonbar_pathname hradio {Horizontal Radio Button [hradio]}
        make_iemgui_button $buttonbar_pathname vradio {Vertical Radio Button [vradio]}
        make_iemgui_button $buttonbar_pathname vumeter {VU Meter [vumeter]}
        make_iemgui_button $buttonbar_pathname mycnv {Canvas [mycnv]}
        make_iemgui_button $buttonbar_pathname menuarray {Array (menuarray)}
    }
    if {$::editmode($mytoplevel)} {
        pack forget $tkcanvas
        pack $buttonbar_pathname -side top -fill x
        pack $tkcanvas -side top -expand 1 -fill both
    } else {
        pack forget $mytoplevel.buttonbar
    }
}

proc ::buttonbar::load_button_images {loadpath} {
    image create photo buttonbar::obj -file $loadpath/obj.gif
    image create photo buttonbar::msg -file $loadpath/msg.gif
    image create photo buttonbar::floatatom -file $loadpath/floatatom.gif
    image create photo buttonbar::symbolatom -file $loadpath/symbolatom.gif
    image create photo buttonbar::text -file $loadpath/text.gif

    image create photo buttonbar::bng -file $loadpath/bng.gif
    image create photo buttonbar::toggle -file $loadpath/toggle.gif
    image create photo buttonbar::numbox -file $loadpath/numbox.gif
    image create photo buttonbar::hslider -file $loadpath/hslider.gif
    image create photo buttonbar::vslider -file $loadpath/vslider.gif
    image create photo buttonbar::hradio -file $loadpath/hradio.gif
    image create photo buttonbar::vradio -file $loadpath/vradio.gif
    image create photo buttonbar::vumeter -file $loadpath/vumeter.gif
    image create photo buttonbar::mycnv -file $loadpath/mycnv.gif

    image create photo buttonbar::menuarray -file $loadpath/menuarray.gif
}

::buttonbar::load_button_images $::current_plugin_loadpath

# only execute on FocusIn if it changes the state of the buttonbar
bind PatchWindow <FocusIn> {+::buttonbar::showhide_buttonbar %W}
bind PatchWindow <<EditMode>> {+::buttonbar::showhide_buttonbar %W}
