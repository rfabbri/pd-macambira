source pdlib.tcl

set ::script_path [file dirname [info script]]

pd::guiproc bitmap_draw_new {self c x y sz w h data} {
    set z 0
    for {set i 0} {$i < $h} {incr i} {
        for {set j 0} {$j < $w} {incr j} {
            $c create rectangle \
                [expr {0+$x+$j*$sz}] [expr {0+$y+$i*$sz}] \
                [expr {1+$x+($j+1)*$sz}] [expr {1+$y+($i+1)*$sz}] \
                -outline black -fill [lindex {white black} [lindex $data $z]] \
                -tags [list $self cell_${j}_${i}_$self]
            incr z
        }
    }
    set x2 [expr {$x+$w*$sz+1}]
    set y2 [expr {$y+$h*$sz+1}]
    $c create rectangle $x $y $x2 $y2 -outline black -tags [list $self border$self]
}

pd::guiclass bitmap {
    constructor {
        set s [file join $::script_path properties.tcl]
        sys_gui "source {$s}\n"

        pd::add_outlet $self float

        # set defaults:
        set @config [list]
        lappend @config -width 8
        lappend @config -height 8
        lappend @config -cellsize 16
        lappend @config -label ""
        lappend @config -labelpos "top"
        lappend @config -sendsymbol ""
        lappend @config -receivesymbol ""
        lappend @config -fgcolor "#000000"
        lappend @config -bgcolor "#ffffff"
        lappend @config -lblcolor "#000000"
        set @data {
            0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
            0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
            0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
            0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        }

        ::$self 0 config {*}$args

        set @rcvLoadData {#bitmap}
        pd_bind [tclpd_get_instance_pd $self] [gensym $@rcvLoadData]
    }

    0_config {
        if {$args == {}} {
            return $@config
        } else {
            set newconf [list]
            set optlist [pd::strip_selectors $args]
            set optlist [pd::strip_empty $optlist]
            for {set i 0} {$i < [llength $optlist]} {} {
                set k [lindex $optlist $i]
                if {![dict exists $@config $k]} {
                    return -code error "unknown option '$k'"
                }
                incr i
                set v [lindex $optlist $i]
                if {[lsearch -exact {-width -height -cellsize} $k] != -1} {
                    set v [expr {int($v)}]
                }
                dict set newconf $k $v
                incr i
            }
            if {[dict get $@config -width] != [dict get $newconf -width] ||
                [dict get $@config -height] != [dict get $newconf -height]} {
                $self 0 resize {*}[pd::add_selectors [list \
                    [dict get $newconf -width] \
                    [dict get $newconf -height] \
                    ]]
            }
            set ui 0
            foreach opt {label labelpos cellsize fgcolor bgcolor lblcolor} {
                set old [dict get $@config -$opt]
                if {[dict exists $newconf -$opt]} {
                    set new [dict get $newconf -$opt]
                    if {$old != $new} {
                        dict set @config -$opt $new
                        set ui 1
                    }
                }
            }
            if {$ui && [info exists @c]} {
                sys_gui [list $@c delete $self]\n
                sys_gui [list bitmap_draw_new $self \
                    $@c $@x $@y \
                    [dict get $@config -cellsize] \
                    [dict get $@config -width] \
                    [dict get $@config -height] \
                    $@data]\n
            }
        }
    }
    
    0_resize {
        set w [pd::arg 0 int]
        set h [pd::arg 1 int]
        set oldw [dict get $@config -width]
        set oldh [dict get $@config -height]
        set newd {}
        for {set y 0} {$y < $h} {incr y} {
            for {set x 0} {$x < $w} {incr x} {
                if {$x < $oldw && $y < $oldh} {
                    lappend newd [lindex $d [expr {$y*$oldw+$x}]]
                } else {
                    lappend newd 0
                }
            }
        }
        dict set @config -width $w
        dict set @config -height $h
        set @data $newd
    }

    0_getrow {
        set r [list]
        set n [pd::arg 0 int]
        set w [dict get $@config -width]
        for {set i [expr {$n*$w}]} {$i < [expr {($n+1)*$w}]} {incr i} {
            lappend r [list float [lindex $@data $i]]
        }
        pd::outlet $self 0 list $r
    }

    0_getcol {
        set r [list]
        set n [pd::arg 0 int]
        set w [dict get $@config -width]
        set h [dict get $@config -height]
        for {set i [expr {$n}]} {$i < [expr {$w*$h}]} {incr i $w} {
            lappend r [list float [lindex $@data $i]]
        }
        pd::outlet $self 0 list $r
    }

    0_getcell {
        set r [pd::arg 0 int]
        set c [pd::arg 1 int]
        set w [dict get $@config -width]
        pd::outlet $self 0 float [lindex $@data [expr {$r*$w+$c}]]
    }

    0_setrow {
        set row [pd::arg 0 int]
        set z 1
        set col 0
        set w [dict get $@config -width]
        for {set idx [expr {$row*$w}]} {$idx < [expr {($row+1)*$w}]} {incr idx} {
            set d [expr {0!=[pd::arg $z int]}]
            lset @data $idx $d
            sys_gui [list $@c itemconfigure cell_${col}_${row}_$self \
                -fill [lindex {white black} $d]]\n
            incr z
            incr col
        }
    }

    0_setcol {
        set col [pd::arg 0 int]
        set z 1
        set row 0
        set w [dict get $@config -width]
        set h [dict get $@config -height]
        for {set idx [expr {$col}]} {$idx < [expr {$w*$h}]} {incr idx $w} {
            set d [expr {0!=[pd::arg $z int]}]
            lset @data $idx $d
            sys_gui [list $@c itemconfigure cell_${col}_${row}_$self \
                -fill [lindex {white black} $d]]\n
            incr z
            incr row
        }
    }

    0_setcell {
        set r [pd::arg 0 int]
        set c [pd::arg 1 int]
        set d [expr {0!=[pd::arg 2 int]}]
        set w [dict get $@config -width]
        set idx [expr {$r*$w+$c}]
        lset @data $idx $d
        sys_gui [list $@c itemconfigure cell_${r}_${c}_$self \
            -fill [lindex {white black} $d]]\n
    }

    0_setdata {
        set d [pd::strip_selectors $args]
        set l [llength $d]
        set w [dict get $@config -width]
        set h [dict get $@config -height]
        if {$l != $w*$h} {
            return -code error "bad data size"
        }
        set @data [list]
        foreach i $d {lappend @data [expr {int($i)}]}
        if {$@rcvLoadData != {}} {
            pd_unbind [tclpd_get_instance_pd $self] [gensym $@rcvLoadData]
            set @rcvLoadData {}
        }
    }

    object_save {
        return [list #X obj $@x $@y bitmap {*}[pd::add_empty $@config] \; \
            \#bitmap setdata {*}$@data \; ]
    }

    object_properties {
        sys_gui [list propertieswindow .prop:$self \
            $@config {Bitmap properties}]\n
    }

    widgetbehavior_getrect {
        lassign $args x1 y1
        set w [dict get $@config -width]
        set h [dict get $@config -height]
        set sz [dict get $@config -cellsize]
        set x2 [expr {1+$x1+$w*$sz}]
        set y2 [expr {1+$y1+$h*$sz}]
        return [list $x1 $y1 $x2 $y2]
    }

    widgetbehavior_displace {
        set dx [lindex $args 0]
        set dy [lindex $args 1]
        if {$dx != 0 || $dy != 0} {
            incr @x $dx
            incr @y $dy
            sys_gui [list $@c move $self $dx $dy]\n
        }
        return [list $@x $@y]
    }

    widgetbehavior_select {
        set sel [lindex $args 0]
        sys_gui [list $@c itemconfigure $self \
            -outline [lindex {black blue} $sel]]\n
    }

    widgetbehavior_activate {
    }

    widgetbehavior_vis {
        set @c [lindex $args 0]
        set @x [lindex $args 1]
        set @y [lindex $args 2]
        set vis [lindex $args 3]
        set w [dict get $@config -width]
        set h [dict get $@config -height]
        set sz [dict get $@config -cellsize]
        if {$vis} {
            sys_gui [list bitmap_draw_new $self \
                $@c $@x $@y $sz $w $h $@data ]\n
        } else {
            sys_gui [list $@c delete $self]\n
        }
    }

    widgetbehavior_click {
        set w [dict get $@config -width]
        set h [dict get $@config -height]
        set sz [dict get $@config -cellsize]
        set xpix [expr {[lindex $args 0]-$@x-1}]
        set ypix [expr {[lindex $args 1]-$@y-1}]
        if {$xpix < 0 || $xpix >= $w*$sz} {return}
        if {$ypix < 0 || $ypix >= $h*$sz} {return}
        set shift [lindex $args 2]
        set alt [lindex $args 3]
        set dbl [lindex $args 4]
        set doit [lindex $args 5]
        if {$doit} {
            set j [expr {$xpix/$sz}]
            set i [expr {$ypix/$sz}]
            set idx [expr {$w*${i}+${j}}]
            set d [expr {[lindex $@data $idx]==0}]
            lset @data $idx $d
            sys_gui [list $@c itemconfigure cell_${j}_${i}_$self \
                -fill [lindex {white black} $d]]\n
        }
    }
}
