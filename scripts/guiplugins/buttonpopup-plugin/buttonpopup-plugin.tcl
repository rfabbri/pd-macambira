# this plugin creates a buttonpopup on a patch window when that patch
# window is in Edit Mode

# this GUI plugin removes the menubars from any patch window that is
# not in Edit Mode.  Also, if a patch is switched to Run Mode, the
# menubar will be removed.

# TODO make it scroll the patch so it acts as an overlay

lappend ::auto_path $::current_plugin_loadpath

package require base64
package require tooltip 1.4.2

namespace eval buttonpopup {
    namespace export show_buttonpopup
    namespace export hide_buttonpopup
}

proc ::buttonpopup::make_pd_button {tkpathname name description} {
    button $tkpathname.$name -image buttonpopup::$name \
        -relief flat -borderwidth 0 -highlightthickness 0 \
        -highlightcolor grey -highlightbackground grey -padx 0 -pady 0 \
        -command "menu_send_float \$::focused_window $name 0"
    pack $tkpathname.$name -side left -padx 0 -pady 0
    ::tooltip::tooltip $tkpathname.$name $description
}

proc ::buttonpopup::make_iemgui_button {tkpathname name description} {
    button $tkpathname.$name -image buttonpopup::$name \
        -relief sunken -borderwidth 0 -highlightthickness 0 \
        -highlightcolor grey -highlightbackground grey -padx 0 -pady 0 \
        -command "menu_send \$::focused_window $name"
    pack $tkpathname.$name -side left -padx 0 -pady 0
    ::tooltip::tooltip $tkpathname.$name $description
}

proc ::buttonpopup::hide {w} {
    set mytoplevel [winfo toplevel $w]
    set tkcanvas [tkcanvas_name $mytoplevel]
    $tkcanvas delete buttonpopup_window
}

proc ::buttonpopup::show {w x y} {
    set mytoplevel [winfo toplevel $w]
    set tkcanvas [tkcanvas_name $mytoplevel]
    set buttonpopup_pathname $tkcanvas.buttonpopup
    if { ! [winfo exists $buttonpopup_pathname]} {
        frame $buttonpopup_pathname -cursor arrow -background grey \
            -pady 0
        make_pd_button $buttonpopup_pathname obj {Object (obj)}
        make_pd_button $buttonpopup_pathname msg {Message (msg)}
        make_pd_button $buttonpopup_pathname floatatom {Number (floatatom)}
        make_pd_button $buttonpopup_pathname symbolatom {Symbol (symbolatom)}
        make_pd_button $buttonpopup_pathname text {Comment}
        make_iemgui_button $buttonpopup_pathname bng {Bang Button [bng]}
        make_iemgui_button $buttonpopup_pathname toggle {Toggle [tgl]}
        make_iemgui_button $buttonpopup_pathname numbox {Number2 [my_numbox]}
        make_iemgui_button $buttonpopup_pathname hslider {Horizontal Slider [hslider]}
        make_iemgui_button $buttonpopup_pathname vslider {Verical Slider [vslider]}
        make_iemgui_button $buttonpopup_pathname hradio {Horizontal Radio Button [hradio]}
        make_iemgui_button $buttonpopup_pathname vradio {Vertical Radio Button [vradio]}
        make_iemgui_button $buttonpopup_pathname vumeter {VU Meter [vumeter]}
        make_iemgui_button $buttonpopup_pathname mycnv {Canvas [mycnv]}
        make_iemgui_button $buttonpopup_pathname menuarray {Array (menuarray)}
        bind $buttonpopup_pathname <KeyPress-Escape> {::buttonpopup::hide %W}
    }
    if {$::editmode($mytoplevel)} {
        $tkcanvas create window $x $y -anchor nw -window $buttonpopup_pathname \
            -tags buttonpopup_window
    }
}

proc ::buttonpopup::load_button_images {loadpath} {
    image create photo buttonpopup::obj -file $loadpath/obj.gif
    image create photo buttonpopup::msg -file $loadpath/msg.gif
    image create photo buttonpopup::floatatom -file $loadpath/floatatom.gif
    image create photo buttonpopup::symbolatom -file $loadpath/symbolatom.gif
    image create photo buttonpopup::text -file $loadpath/text.gif

    image create photo buttonpopup::bng -file $loadpath/bng.gif
    image create photo buttonpopup::toggle -file $loadpath/toggle.gif
    image create photo buttonpopup::numbox -file $loadpath/numbox.gif
    image create photo buttonpopup::hslider -file $loadpath/hslider.gif
    image create photo buttonpopup::vslider -file $loadpath/vslider.gif
    image create photo buttonpopup::hradio -file $loadpath/hradio.gif
    image create photo buttonpopup::vradio -file $loadpath/vradio.gif
    image create photo buttonpopup::vumeter -file $loadpath/vumeter.gif
    image create photo buttonpopup::mycnv -file $loadpath/mycnv.gif

    image create photo buttonpopup::menuarray -file $loadpath/menuarray.gif
}

::buttonpopup::load_button_images $::current_plugin_loadpath

bind all <Double-ButtonRelease-1> {+::buttonpopup::show %W %x %y}
bind all <ButtonRelease> {+after 100 ::buttonpopup::hide %W}
