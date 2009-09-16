source pdlib.tcl

set ::script_path [file dirname [info script]]

pd::guiproc vslider2_draw_new {self c x y config} {
    set headsz [dict get $config -headsz]
    set x2 [expr {$x+[dict get $config -width]+1}]
    set y2 [expr {$y+[dict get $config -height]+1}]
    set fgcolor [dict get $config -fgcolor]
    set bgcolor [dict get $config -bgcolor]
    $c create rectangle $x $y $x2 $y2 \
        -outline $fgcolor -fill $bgcolor -tags [list $self border$self]
    $c create rectangle $x [expr {$y2-$headsz}] $x2 $y2 \
        -outline {} -fill $fgcolor -tags [list $self head$self]
    vslider2_update $self $c $x $y $config
}

pd::guiproc vslider2_update {self c x y config} {
    set f [dict get $config -initvalue]
    set w [dict get $config -width]
    set h [dict get $config -height]
    set b [dict get $config _min]
    set t [dict get $config _max]
    set r [dict get $config _rev]
    set realvalue [expr {1.0*($f-$b)/($t-$b)}]
    if {$realvalue < 0.0} {set realvalue 0}
    if {$realvalue > 1.0} {set realvalue 1}
    if {!$r} {set realvalue [expr {1.0-$realvalue}]}
    set headsz [dict get $config -headsz]
    set vr [expr {$h-$headsz}]
    $c coords head$self $x [expr {$y+$vr*$realvalue}] \
        [expr {$x+$w+1}] [expr {$y+$vr*$realvalue+$headsz}]
    set lbl [dict get $config -label]
    set lblpos [dict get $config -labelpos]
    set lblcol [dict get $config -lblcolor]
    $c delete label$self
    if {$lbl != {}} {
        switch $lblpos {
            top    {set lx [expr {$x+$w/2}]; set ly [expr {$y}]; set a "s"}
            bottom {set lx [expr {$x+$w/2}]; set ly [expr {$y+$h+2}]; set a "n"}
            left   {set lx [expr {$x}]; set ly [expr {$y+$h/2}]; set a "e"}
            right  {set lx [expr {$x+$w+2}]; set ly [expr {$y+$h/2}]; set a "w"}
        }
        $c create text $lx $ly -anchor $a -text $lbl -fill $lblcol \
             -tags [list $self label$self]
    }
}

pd::guiclass vslider2 {
    constructor {
        pd::add_outlet $self float
        sys_gui "source {[file join $::script_path properties.tcl]}\n"
        # set defaults:
        set @config {
            -width 15 -height 130 -headsz 3 -rangebottom 0 -rangetop 127
            -init 0 -initvalue 0 -jumponclick 0 -label "" -labelpos "top"
            -sendsymbol "" -receivesymbol ""
            -fgcolor "#000000" -bgcolor "#ffffff" -lblcolor "#000000"
            _min 0 _max 127 _rev 0
        }
        # expanded ($n) send/recv symbols:
        set @send {}
        set @recv {}
        ::$self 0 config {*}$args
    }

    destructor {
        if {[dict get $@config -receivesymbol] != {}} {
            pd_unbind [tclpd_get_instance_pd $self] $@recv
        }
    }

    0_loadbang {
        if {[dict get $@config -init]} {$self 0 bang}
    }

    0_config {
        if {$args == {}} {return $@config}
        set newconf [list]
        set optlist [pd::strip_selectors $args]
        set optlist [pd::strip_empty $optlist]
        set int_opts {-width -height -cellsize}
        set bool_opts {-init -jumponclick}
        set ui_opts {-fgcolor -bgcolor -lblcolor -width -height}
        set upd_opts {-rangebottom -rangetop -label -labelpos}
        set conn_opts {-sendsymbol -receivesymbol}
        set ui 0
        set upd 0
        foreach {k v} $optlist {
            if {![dict exists $@config $k]} {
                return -code error "unknown option '$k'"
            }
            if {[dict get $@config $k] == $v} {continue}
            if {[lsearch -exact $int_opts $k] != -1} {set v [expr {int($v)}]}
            if {[lsearch -exact $bool_opts $k] != -1} {set v [expr {int($v)!=0}]}
            if {[lsearch -exact $ui_opts $k] != -1} {set ui 1}
            if {[lsearch -exact $upd_opts $k] != -1} {set upd 1}
            dict set newconf $k $v
        }
        # process -{send,receive}symbol
        if {[dict exists $newconf -receivesymbol]} {
            set new_recv [dict get $newconf -receivesymbol]
            set selfpd [tclpd_get_instance_pd $self]
            if {[dict get $@config -receivesymbol] != {}} {
                pd_unbind $selfpd $@recv
            }
            if {$new_recv != {}} {
                set @recv [canvas_realizedollar \
                    [tclpd_get_glist $self] [gensym $new_recv]]
                pd_bind $selfpd $@recv
            } else {set @recv {}}
        }
        if {[dict exists $newconf -sendsymbol]} {
            set new_send [dict get $newconf -sendsymbol]
            if {$new_send != {}} {
                set @send [canvas_realizedollar \
                    [tclpd_get_glist $self] [gensym $new_send]]
            } else {set @send {}}
        }
        # no errors up to this point. we can safely merge options
        set @config [dict merge $@config $newconf]
        # adjust reverse range
        set a [dict get $@config -rangebottom]
        set b [dict get $@config -rangetop]
        if {$a > $b} {
            dict set @config _min $b
            dict set @config _max $a
            dict set @config _rev 1
        } else {
            dict set @config _min $a
            dict set @config _max $b
            dict set @config _rev 0
        }
        # recompute pix2units conversion
        set @pix2units [expr {1.0 * ( [dict get $@config -rangetop] - [dict get $@config -rangebottom]) / ( [dict get $@config -height] - [dict get $@config -headsz])}]
        # if ui changed, update it
        if {$ui && [info exists @c]} {
            sys_gui [list $@c delete $self]\n
            sys_gui [list vslider2_draw_new $self $@c $@x $@y $@config]\n
        } elseif {$upd && [info exists @c]} {
            sys_gui [list vslider2_update $self $@c $@x $@y $@config]\n
        }
    }
    
    0_set {
        set f [pd::arg 0 float]
        set b [dict get $@config _min]
        set t [dict get $@config _max]
        if {$f < $b} {set f $b}
        if {$f > $t} {set f $t}
        dict set @config -initvalue $f
        if {[info exists @c]} {
            # update ui:
            sys_gui [list vslider2_update $self $@c $@x $@y $@config]\n
        }
    }

    0_bang {
        set f [dict get $@config -initvalue]
        pd::outlet $self 0 float $f
        if {$@send != {}} {
            set s_thing [$@send cget -s_thing]
            if {$s_thing != {NULL}} {pd_float $s_thing $f}
        }
    }

    0_float {
        $self 0 set {*}$args
        $self 0 bang
    }

    object_save {
        return [list #X obj $@x $@y vslider2 {*}[pd::add_empty $@config] \;]
    }

    object_properties {
        gfxstub_new [tclpd_get_object_pd $self] [tclpd_get_instance $self] \
            [list propertieswindow %s $@config "\[vslider2\] properties"]\n
    }

    widgetbehavior_getrect {
        lassign $args x1 y1
        set x2 [expr {1+$x1+[dict get $@config -width]}]
        set y2 [expr {1+$y1+[dict get $@config -height]}]
        return [list $x1 $y1 $x2 $y2]
    }

    widgetbehavior_displace {
        lassign $args dx dy
        if {$dx != 0 || $dy != 0} {
            incr @x $dx
            incr @y $dy
            sys_gui [list $@c move $self $dx $dy]\n
        }
        return [list $@x $@y]
    }

    widgetbehavior_select {
        lassign $args sel
        sys_gui [list $@c itemconfigure $self -outline [lindex \
            [list [dict get $@config -fgcolor] {blue}] $sel]]\n
    }

    widgetbehavior_vis {
        lassign $args @c @x @y vis
        if {$vis} {
            sys_gui [list vslider2_draw_new $self $@c $@x $@y $@config]\n
        } else {
            sys_gui [list $@c delete $self]\n
        }
    }

    widgetbehavior_click {
        lassign $args x y shift alt dbl doit
        set h [dict get $@config -height]
        set ypix [expr {[lindex $args 1]-$@y-1}]
        if {$ypix < 0 || $ypix >= $h} {return}
        if {$doit} {
            set @motion_start_y $y
            set @motion_curr_y $y
            set @motion_start_v [dict get $@config -initvalue]
            tclpd_guiclass_grab [tclpd_get_instance $self] \
                [tclpd_get_glist $self] $x $y
        }
    }

    widgetbehavior_motion {
        lassign $args dx dy
        set @motion_curr_y [expr {$dy+$@motion_curr_y}]
        set pixdelta [expr {-1*($@motion_curr_y-$@motion_start_y)}]
        set f [expr {$@motion_start_v+$pixdelta*$@pix2units}]
        $self 0 float {*}[pd::add_selectors [list $f]]
    }
}
