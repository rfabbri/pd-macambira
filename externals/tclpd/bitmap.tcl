source pdlib.tcl

pd::guiproc bitmap_draw_new {self c x y sz w h} {
    set x2 [expr {$x+$w*$sz+1}]
    set y2 [expr {$y+$h*$sz+1}]
    $c create rectangle $x $y $x2 $y2 -outline black -tags [list $c border$c]
    for {set i 0} {$i < $h} {incr i} {
        for {set j 0} {$j < $w} {incr j} {
            $c create rectangle \
                [expr {1+$x+$j*$sz}] [expr {1+$y+$i*$sz}] \
                [expr {$x+($j+1)*$sz}] [expr {$y+($i+1)*$sz}] \
                -outline black -fill white -tags [list $c cell_${j}_${i}_$c]
        }
    }
}

pd::guiproc bitmap_draw_erase {self c x y sz w h} {
    $c delete foo$c
}

pd::guiproc bitmap_draw_select {self c sel} {
    $c itemconfigure border$c -outline [lindex {black blue} $sel]
}

pd::guiclass bitmap {
    constructor {
        pd::add_outlet $self float
        set @sz 8
        set @w 8
        set @h 8

        set @data [list]
        for {set i 0} {$i < $@h} {incr i} {
            for {set j 0} {$j < $@w} {incr j} {
                lappend @data 0
            }
        }
    }
    
    0_getrow {
        set r [list]
        set n [pd::arg 0 int]
        for {set i [expr {$n*$@sz}]} {$i < [expr {($n+1)*$@sz}]} {incr i} {
            lappend r [list float [lindex $@data $i]]
        }
        pd::outlet $self 0 list $r
    }

    object_save {
        return [list #X obj $@x $@y bitmap $@sz $@w $@h \;]
    }

    widgetbehavior_getrect {
        lassign $args x1 y1
        set x2 [expr {$x1+$@w*$@sz}]
        set y2 [expr {$y1+$@h*$@sz}]
        return [list $x1 $y1 $x2 $y2]
    }

    widgetbehavior_displace {
        set dx [lindex $args 0]
        set dy [lindex $args 1]
        if {$dx != 0 || $dy != 0} {
            incr @x $dx
            incr @y $dy
            sys_gui [list $@c move $@c $dx $dy]\n
        }
        return [list $@x $@y]
    }

    widgetbehavior_select {
        set sel [lindex $args 0]
        sys_gui [list bitmap_draw_select $self $@c $sel]\n
    }

    widgetbehavior_activate {
    }

    widgetbehavior_vis {
        set @c [lindex $args 0]
        set @x [lindex $args 1]
        set @y [lindex $args 2]
        set vis [lindex $args 3]
        if {$vis} {
            sys_gui [list bitmap_draw_new $self $@c $@x $@y $@sz $@w $@h]\n
        } else {
            sys_gui [list bitmap_draw_erase $self $@c $@x $@y $@sz $@w $@h]\n
        }
    }

    widgetbehavior_click {
        set xpix [lindex $args 0]
        set ypix [lindex $args 1]
        set shift [lindex $args 2]
        set alt [lindex $args 3]
        set dbl [lindex $args 4]
        set doit [lindex $args 5]
        if {$doit} {
            set j [expr {($xpix-$@x)/$@sz}]
            set i [expr {($ypix-$@y)/$@sz}]
            set idx [expr {$@sz*${i}+${j}}]
            set d [expr {[lindex $@data $idx]==0}]
            lset @data $idx $d
            sys_gui [list $@c itemconfigure cell_${j}_${i}_$@c -fill [lindex {white black} $d]]\n
        }
    }
}
