package require Tclpd 0.2.3
package require TclpdLib 0.19

proc tclpd-console::constructor {self} {
    if {[info exist ::tclpd_console_loaded]} {
        return -code error "only one instance of tclpd-console allowed"
    }

    set ::tclpd_console_loaded 1
    set ::${self}_loaded 1

    pd_bind [tclpd_get_instance_pd $self] [gensym $self]

    sys_gui "set ::tclpd_console $self"
    sys_gui {
        frame .pdwindow.tcl.tclpd -borderwidth 0
        pack .pdwindow.tcl.tclpd -side bottom -fill x
        label .pdwindow.tcl.tclpd.label -text [_ "TclPd:"] -anchor e
        pack .pdwindow.tcl.tclpd.label -side left
        entry .pdwindow.tcl.tclpd.entry -width 200 \
            -exportselection 1 -insertwidth 2 -insertbackground blue \
            -textvariable ::pdwindow::tclpdentry -font {$::font_family 12}
        pack .pdwindow.tcl.tclpd.entry -side left -fill x
        bind .pdwindow.tcl.tclpd.entry <$::modifier-Key-a> { %W selection range 0 end; break }
        bind .pdwindow.tcl.tclpd.entry <Return> { ::pdsend "$::tclpd_console $::pdwindow::tclpdentry" }
        bind .pdwindow.tcl.tclpd.entry <KeyRelease> { .pdwindow.tcl.tclpd.entry configure -background [lindex {#FFF0F0 #FFFFFF} [info complete $::pdwindow::tclpdentry]] }
        bind .pdwindow.text <Key-Tab> { focus .pdwindow.tcl.tclpd.entry; break }
    }
}

proc tclpd-console::destructor {self} {
    if {[set ::${self}_loaded]} {
        pd_unbind [tclpd_get_instance_pd $self] [gensym $self]

        sys_gui { destroy .pdwindow.tcl.tclpd }
    }

    unset ::tclpd_console_loaded
    unset ::${self}_loaded
}

proc tclpd-console::0_anything {self args} {
    set tclcmd [pd::strip_selectors $args]
    pd::post [concat % $tclcmd]
    pd::post [uplevel #0 $tclcmd]
}

pd::class tclpd-console -noinlet 1
