source pdlib.tcl

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
        pd::add_outlet $self float

        set @sz [pd::default_arg 0 int 15]
        if {$@sz < 4} {set @sz 4}
        set @w [pd::default_arg 1 int 8]
        set @h [pd::default_arg 2 int 8]

        set @data [list]
        set z 2
        for {set i 0} {$i < $@h} {incr i} {
            for {set j 0} {$j < $@w} {incr j} {
                lappend @data [expr {0!=[pd::default_arg [incr z] int 0]}]
            }
        }
    }
    
    0_getrow {
        set r [list]
        set n [pd::arg 0 int]
        for {set i [expr {$n*$@w}]} {$i < [expr {($n+1)*$@w}]} {incr i} {
            lappend r [list float [lindex $@data $i]]
        }
        pd::outlet $self 0 list $r
    }

    0_getcol {
        set r [list]
        set n [pd::arg 0 int]
        for {set i [expr {$n}]} {$i < [expr {$@w*$@h}]} {incr i $@w} {
            lappend r [list float [lindex $@data $i]]
        }
        pd::outlet $self 0 list $r
    }

    0_getcell {
        set r [pd::arg 0 int]
        set c [pd::arg 1 int]
        pd::outlet $self 0 float [lindex $@data [expr {$r*$@w+$c}]]
    }

    0_setrow {
        set row [pd::arg 0 int]
        set z 1
        set col 0
        for {set i [expr {$row*$@w}]} {$i < [expr {($row+1)*$@w}]} {incr i} {
            set d [expr {0!=[pd::arg $z int]}]
            lset @data $i $d
            sys_gui [list $@c itemconfigure cell_${col}_${row}_$self -fill [lindex {white black} $d]]\n
            incr z
            incr col
        }
    }

    0_setcol {
        set col [pd::arg 0 int]
        set z 1
        set row 0
        for {set i [expr {$col}]} {$i < [expr {$@w*$@h}]} {incr i $@w} {
            set d [expr {0!=[pd::arg $z int]}]
            lset @data $i $d
            sys_gui [list $@c itemconfigure cell_${col}_${row}_$self -fill [lindex {white black} $d]]\n
            incr z
            incr row
        }
    }

    0_setcell {
        set r [pd::arg 0 int]
        set c [pd::arg 1 int]
        set d [expr {0!=[pd::arg 2 int]}]
        lset @data [expr {$r*$@w+$c}] $d
        sys_gui [list $@c itemconfigure cell_${r}_${c}_$self -fill [lindex {white black} $d]]\n
    }

    object_save {
        return [list #X obj $@x $@y bitmap $@sz $@w $@h {*}$@data \;]
    }

    widgetbehavior_getrect {
        lassign $args x1 y1
        set x2 [expr {1+$x1+$@w*$@sz}]
        set y2 [expr {1+$y1+$@h*$@sz}]
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
        sys_gui [list $@c itemconfigure $self -outline [lindex {black blue} $sel]]\n
    }

    widgetbehavior_activate {
    }

    widgetbehavior_vis {
        set @c [lindex $args 0]
        set @x [lindex $args 1]
        set @y [lindex $args 2]
        set vis [lindex $args 3]
        if {$vis} {
            sys_gui [list bitmap_draw_new $self $@c $@x $@y $@sz $@w $@h $@data ]\n
        } else {
            sys_gui [list $@c delete $self]\n
        }
    }

    widgetbehavior_click {
        set xpix [expr {[lindex $args 0]-$@x-1}]
        set ypix [expr {[lindex $args 1]-$@y-1}]
        if {$xpix < 0 || $xpix >= $@w*$@sz} {return}
        if {$ypix < 0 || $ypix >= $@h*$@sz} {return}
        set shift [lindex $args 2]
        set alt [lindex $args 3]
        set dbl [lindex $args 4]
        set doit [lindex $args 5]
        if {$doit} {
            set j [expr {$xpix/$@sz}]
            set i [expr {$ypix/$@sz}]
            set idx [expr {$@w*${i}+${j}}]
            puts stderr "RELX=$xpix RELY=$ypix IDX=$idx"
            set d [expr {[lindex $@data $idx]==0}]
            lset @data $idx $d
            sys_gui [list $@c itemconfigure cell_${j}_${i}_$self -fill [lindex {white black} $d]]\n
        }
    }
}
