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
        set w .pdwindow.tcl.tclpd
        frame $w -borderwidth 0
        pack $w -side bottom -fill x
        label $w.label -text [_ "tclpd: "] -anchor e
        pack $w.label -side left
        entry $w.entry -width 200 \
            -exportselection 1 -insertwidth 2 -insertbackground blue \
            -textvariable ::tclpd_cmd -font {$::font_family 12}
        pack $w.entry -side left -fill x
        bind $w.entry <$::modifier-Key-a> "%W selection range 0 end; break"
        bind $w.entry <Return> {::pdsend "$::tclpd_console $::tclpd_cmd"}
        set bgrule {[lindex {#FFF0F0 #FFFFFF} [info complete $::tclpd_cmd]]}
        bind $w.entry <KeyRelease> "$w.entry configure -background $bgrule"
        bind .pdwindow.text <Key-Tab> "focus $w.entry; break"
    }
}

proc tclpd-console::destructor {self} {
    if {[set ::${self}_loaded]} {
        sys_gui { destroy .pdwindow.tcl.tclpd ; unset ::tclpd_console }

        pd_unbind [tclpd_get_instance_pd $self] [gensym $self]
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
