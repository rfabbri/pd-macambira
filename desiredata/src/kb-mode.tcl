package provide kb-mode 0.1

set kbmode 1

proc kb-mode_init {canvas} {
	puts "init kb-mode"
	set self "kbcursor"
	append_callback kb-mode key kb-mode_key
	Kb_cursor new_as $self $canvas
	$self draw
	return $self
}

def Canvas kb-mode_key {x y key iso shift} {
	set c [$self widget]
	set step [$@kbcursor step]
	if {$@keyprefix} {
		$@kbcursor get_key $key
		set @keyprefix 0
	}
	switch $key {
		BackSpace {$self delete_selection}
		Up       {$@kbcursor move 0 -$step; if {$shift} {$self arrow_key 0 -$step}}
		Down     {$@kbcursor move 0 +$step; if {$shift} {$self arrow_key 0 +$step}}
		Left     {$@kbcursor move -$step 0; if {$shift} {$self arrow_key -$step 0}}
		Right    {$@kbcursor move +$step 0; if {$shift} {$self arrow_key +$step 0}}
		t        {$self kbcursor_select}
		Return {$self return_key $x $y $key $iso $shift}
		default {}
	}
}

def Canvas kbcursor_move_up {} {$@kbcursor move 0 -[$@kbcursor step]}
def Canvas kbcursor_move_down {} {$@kbcursor move 0 [$@kbcursor step]}
def Canvas kbcursor_move_left {} {$@kbcursor move -[$@kbcursor step] 0}
def Canvas kbcursor_move_right {} {$@kbcursor move [$@kbcursor step] 0}

def Canvas move_selection_up {} {$self arrow_key 0 -[$@kbcursor step]}
def Canvas move_selection_down {} {$self arrow_key 0 +[$@kbcursor step]}
def Canvas move_selection_left {} {$self arrow_key -[$@kbcursor step] 0}
def Canvas move_selection_right {} {$self arrow_key 0 +[$@kbcursor step] 0}

class_new Kb_cursor {View}

def Kb_cursor init {canvas} {
	super
	set @canvas $canvas
	set @x 200
	set @y 200
	set @h [expr [font metrics [$self look font] -linespace]+3]
	set @step [expr ceil($@h*1.5)]
	set @prefixkeys {}
	$self init_keys
	$self recenter
}


def Kb_cursor xy {} {return [list $@x $@y]}
def Kb_cursor step {} {return $@step}

def Kb_cursor draw {} {
	set c [$@canvas widget]
	set line1 [list $@x $@y $@x [expr $@y+$@h]]
	set line2 [list $@x $@y [expr $@x+3] $@y]
	set line3 [list $@x [expr $@y+$@h-1] [expr $@x+3] [expr $@y+$@h-1]]
	$self item LINE1 line $line1 -fill red -width 2
	$self item LINE2 line $line2 -fill red -width 1
	$self item LINE3 line $line3 -fill red -width 1

	$c raise $self
}

def Kb_cursor move {x y} {
	set @x [expr $@x + $x]
	set @y [expr $@y + $y]
	$self test_bounds
	$self draw
	set action [$@canvas action]
	if {$action != "none"} {
		if {[$action class] == "SelRect"} {
			$action motion $@x $@y 256 "none"
		}
		
	}
}

def Kb_cursor scroll_canvas {} {
	set c [$@canvas widget]
	mset {x1 y1 x2 y2} [$c bbox all]
	puts "xview [$c xview]"
	set x2 [expr $x2]; set y2 [expr $y2]
}

def Kb_cursor test_bounds {} {
	set c [$@canvas widget]
	set width [winfo width $c]; set height [winfo height $c]
	set x1 [$c canvasx 2]; set y1 [$c canvasy 2]
	set x2 [expr $x1+$width-2]
	set y2 [expr $y1+$height-2]
	if {$@x >= $x2} {$self scroll r [expr int($@x-$x2)]}
	if {$@x <= $x1} {$self scroll l [expr int($@x-$x1)]}
	if {$@y >= $y2} {$self scroll b [expr int($@y-$y2)]}
	if {$@y <= $y1} {$self scroll t [expr int($@y-$y1)]}
}

def Kb_cursor scroll {direction diff} {
	set c [$@canvas widget]
	set width [winfo width $c]; set height [winfo height $c]
	set x1 [$c canvasx 2]; set y1 [$c canvasy 2]
	mset {l r} [$c xview]
	mset {t b} [$c yview]
	foreach {n0 n1 n2 n3 region} [$c configure -scrollregion] {mset {rx1 ry1 rx2 ry2} $region}
	switch $direction {
		r {set axis "x"; if {$r == 1} {set rx2 [expr $rx2+$width]}}
		l {set axis "x"; if {$l == 0} {set rx1 [expr $rx1-$width]}}
		b {set axis "y"; if {$b == 1} {set ry2 [expr $ry2+$height]}}
		t {set axis "y"; if {$t == 0} {set ry1 [expr $ry1-$height]}}
	}
	$c configure -scrollregion [list $rx1 $ry1 $rx2 $ry2]
	$c [list $axis]view scroll $diff units
}

def Canvas kbcursor_recenter {} {$@kbcursor recenter}

def Kb_cursor recenter {} {
	set c [$@canvas widget]
	set width [winfo width $c]; set height [winfo height $c]
	set x1 [$c canvasx 2]; set y1 [$c canvasy 2]
	set x2 [expr $x1+$width]
	set y2 [expr $y1+$height]
	set @x [expr $x1+($width/2)]
	set @y [expr $y1+($height/2)]
	$self draw
}

def Canvas kbcursor_Object  {} {mset {x y} [$@kbcursor xy]; $self new_objectxy [expr $x+4] $y obj}
def Canvas kbcursor_Message {} {mset {x y} [$@kbcursor xy]; $self new_objectxy [expr $x+4] $y msg}
def Canvas kbcursor_bng     {} {mset {x y} [$@kbcursor xy]; $self new_objectxy [expr $x+4] $y obj bng}
def Canvas kbcursor_tgl     {} {mset {x y} [$@kbcursor xy]; $self new_objectxy [expr $x+4] $y obj tgl}
def Canvas kbcursor_nbx     {} {mset {x y} [$@kbcursor xy]; $self new_objectxy [expr $x+4] $y obj nbx}

def Canvas kbcursor_select {} {
	mset {x y} [lmap + [$@kbcursor xy] 4]; set x [expr $x+4]
	mset {type id detail} [$self identify_target $x $y 0]
	puts "type:: $type || id:: $id || detail:: $detail"
	if {$type == "object"} {
		if {![$id selected?]} {$self selection+= $id} else {$self selection-= $id}
	}
}

def Kb_cursor init_keys {} {
	global newkey accels
	#@keys used for prefix keycommands
	dict set @prefixkeys "1" "kbcursor_Object"
	dict set @prefixkeys "2" "kbcursor_Message"
	dict set @prefixkeys "b" "kbcursor_bng"
	dict set @prefixkeys "t" "kbcursor_tgl"
	dict set @prefixkeys "3" "kbcursor_nbx"
	#@ctrlkeys overwrites the existing keybindings
	foreach {key command} $newkey {
		if {[catch {set vars [dict get $accels $key]}]} {puts "$key not bound yet......";set vars {}}
		set new_val {}
		foreach item $vars {
			mset {class cmd} [split $item ":"]
			if {$class == [$@canvas class]} {
				set n $class:$command
				lappend new_val $n
			} else {
				lappend new_val $item
			}
		}
		if {$new_val == ""} {set new_val [$@canvas class]:$command}
		dict set accels $key $new_val
	}
	
}

def Kb_cursor get_key {key} {
	if {[catch {set command [dict get $@prefixkeys $key]}]} {
		post "key Ctrl-x $key not bound"
	} {
		puts "run $command"
		$@canvas $command
	}
} 

set newkey {
	Ctrl+p {kbcursor_move_up}
	Ctrl+n {kbcursor_move_down}
	Ctrl+f {kbcursor_move_right}
	Ctrl+b {kbcursor_move_left}
	Ctrl+P {move_selection_up}
	Ctrl+N {move_selection_down}
	Ctrl+F {move_selection_right}
	Ctrl+B {move_selection_left}
	Alt+f {}
	Alt+b {}
	Ctrl+space {kbcursor_mark}
}

def Canvas kbcursor_mark {} {
	mset {x y} [$@kbcursor xy]
	if {$@action == "none"} {
		set @action [SelRect new $self $x $y 256 "none"]
	} else {
		if {[$@action class] == "SelRect"} {
			$@action unclick $x $y 236 "none"
		}
	}
}