package provide dzinc 0.1
package require Tkzinc

class_new Zinc {Thing}
def Zinc init {} {
	set @group [eval old_$self add group 1]
}
def Zinc unknown {args} {
	upvar 1 selector selector
	old_$self $selector {expand}$args
}

rename canvas old_canvas

proc option_replace {argz} {
	#set new_args {}
	foreach {option val} $argz {
		switch -- $option {
			-bg {set option -backcolor}
			-background {set option -backcolor}
			-bd {set option -borderwidth}
		}
		lappend new_args $option $val
	}
	puts "new_args:::: $new_args"
	return $new_args
}
#.bgerrorDialog.bitmap create oval 0 0 31 31 -fill red -outline black
#.x80e1c00.c -width 446 -height 296 -background white
proc canvas {name args} {
	set result [eval [concat [list zinc $name] [option_replace $args] -highlightthickness 1]]
        Zinc new_as $name
	return $result
}

def Zinc remove {orig mess} {
	set idx [lsearch $mess $orig]
	if {$idx < 0} {return $mess}
	set mess [lreplace $mess $idx $idx+1]
	return $mess
}


def Zinc replace {orig new mess} {
	set idx [lsearch $mess $orig]
	if {$idx < 0} {return $mess}
	set mess [lreplace $mess $idx $idx $new]
	return $mess
}

def Zinc replace2 {orig new mess} {
	set idx [lsearch $mess $orig]
	if {$idx < 0} {return $mess}
	set mess [lreplace $mess $idx $idx+1]
	foreach item [lreverse $new] {set mess [linsert $mess $idx $item]}
	return $mess
}

def Zinc insert {orig new mess} {
	set idx [lsearch $mess $orig]
	if {$idx < 0} {return $mess}
	#set mess [lreplace $mess $idx $idx+1]
	foreach item [lreverse $new] {set mess [linsert $mess $idx $item]}
	return $mess
}


def Zinc canvasx {val} {
	return $val
}

def Zinc canvasy {val} {
	return $val
}

def Zinc substitude {args} {
	set args [$self replace -width -linewidth $args]
	set args [$self replace -fill -linecolor $args]
	set args [$self replace2 -dash [list -linestyle dotted] $args]
	set args [$self replace -outline -linecolor $args]
	set args [$self remove -dashoffset $args]
	return $args
}

def Zinc create {args} {
	set i 0
	foreach item $args {
		if {[regexp {^-} $item]} {break}
		incr i
	}
	switch [lindex $args 0] {
		line {
			set type curve
			set args [$self replace -width -linewidth $args]
			set args [$self replace -fill -linecolor $args]
			set args [$self replace2 -dash [list -linestyle dotted] $args]
			set args [$self remove -dashoffset $args]
		}
		oval {
			set type arc
			set args [$self replace -fill -fillcolor $args]
			set args [$self replace -outline -linecolor $args]
		}
		window {
			set type window
		}
		rectangle {
			set type rectangle
			set args [$self replace -fill -fillcolor $args]
			set args [$self replace -outline -linecolor $args]
			set args [$self replace -width -linewidth $args]
			set args [$self replace2 -dash [list -linestyle dotted] $args]
		}
		text {
			set type text
			set args [$self replace -fill -color $args]
		}
		default {
			puts "UNKNOWN ITEM.. [lindex $args 0]"
		}
	}
	if {$i == 2} {
		set mess [concat $type $@group [lrange $args 1 end]]
	} else {
		set mess [concat $type $@group [list [lrange $args 1 $i-1]] [lrange $args $i end]]
	}
	if {$type == "window" || $type == "text"} {set mess [linsert $mess 2 -position]}
	#puts "mess >> $mess"
	eval [concat [list old_$self add] $mess]
}

def Zinc itemconfigure {args} {
	set mess [$self substitude $args]
	eval [concat [list old_$self itemconfig] $mess]
}

def Zinc coords {args} {
	eval [concat [list old_$self coords] [lindex $args 0] [list [lrange $args 1 end]]]
}


def Zinc configure {args} {
	eval [concat [list old_$self configure] [option_replace $args]]
}

def Zinc delete {args} {
	eval [concat [list old_$self remove] $args]
}

def Zinc gettags {args} {
	set result {} ;# bogus
	catch {set result [eval [concat [list old_$self gettags] $args]]}
	return $result
}

def Zinc lower {tag args} {
}