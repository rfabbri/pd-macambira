#!/usr/bin/env tclsh

proc mset {vars list} {uplevel 1 "foreach {$vars} {$list} {break}"}
proc p {text} {write [list #X text 10 $::y $text]; incr ::y 60}
proc write {list} {
	set v [join $list " "]
	regsub -all "," $v " \\, " v
	regsub -all ";" $v " \\; " v
	regsub -all "\\$" $v "\\$" v
	puts $::fh "$v;"
}
set oid 0
proc obj  {args} {write [concat [list #X obj ] $args]; incr ::oid}
proc msg  {args} {write [concat [list #X msg ] $args]; incr ::oid}
proc text {args} {write [concat [list #X text] $args]; incr ::oid}

set fh [open numop.pd w]
write [list #N canvas 0 0 1024 768 10]
set y 0
set row 0
set msgboxes {}
set col1 96
set col2 512
set col3 768
set col4 1024
set rowsize 32

obj 0 $y cnv 15 $col4 30 empty empty empty 20 12 0 14 20 -66577 0
text 10 $y op name
text $col1 $y description
text $col2 $y "effect on pixels"
text $col3 $y "effect on coords"
incr y 32

# onpixels = meaning in pixel context (pictures, palettes)
# oncoords = meaning in spatial context (indexmaps, polygons)

# for vecops, the two analogy-columns were labelled:
#   meaning in geometric context (indexmaps, polygons, in which each complex number is a point)
#   meaning in spectrum context (FFT) in which each number is a (cosine,sine) pair
proc op {op desc {extra1 ""} {extra2 ""}} {
	global y
	if {$::row&1} {set bg -233280} {set bg -249792}
	obj 0 $y cnv 15 $::col4 [expr $::rowsize-2] empty empty empty 20 12 0 14 $bg -66577 0
	lappend ::msgboxes $::oid
	msg 10 $y op $op
	text $::col1 $y $desc
	if {$extra1 != ""} {text $::col2 $y $extra1}
	if {$extra2 != ""} {text $::col3 $y $extra2}
	incr ::row
	incr ::y $::rowsize
}

proc draw_columns {} {
	obj [expr $::col1-1] 0 cnv 0 0 $::y empty empty empty -1 12 0 14 0 -66577 0
	obj [expr $::col2-1] 0 cnv 0 0 $::y empty empty empty -1 12 0 14 0 -66577 0
	obj [expr $::col3-1] 0 cnv 0 0 $::y empty empty empty -1 12 0 14 0 -66577 0
}

proc numbertype {op desc {extra1 ""} {extra2 ""}} {op $op $desc $extra1 $extra2}

set sections {}
proc section {desc} {
	lappend ::sections [list $::y $desc]
	incr ::y 20
}

section {numops}
op {ignore} { A } {no effect} {no effect}
op {put} { B } {replace by} {replace by}
op {+} { A + B } {brightness, crossfade} {move, morph}
op {-} { A - B } {brightness, motion detection} {move, motion detection}
op {inv+} { B - A } {negate then contrast} {180 degree rotate then move}
op {*} { A * B } {contrast} {zoom out}
op {/} { A / B, rounded towards zero } {contrast} {zoom in}
op {div} { A / B, rounded downwards } {contrast} {zoom in}
op {inv*} { B / A, rounded towards zero }
op {swapdiv} { B / A, rounded downwards }
op {%} { A % B, modulo (goes with div) } {--} {tile}
op {swap%} { B % A, modulo (goes with div) }
op {rem} { A % B, remainder (goes with /) }
op {swaprem} { B % A, remainder (goes with /) }
op {gcd} {greatest common divisor}
op {lcm} {least common multiple}
op {|} { A or B, bitwise } {bright munchies} {bottomright munchies}
op {^} { A xor B, bitwise } {symmetric munchies (fractal checkers)} {symmetric munchies (fractal checkers)}
op {&} { A and B, bitwise } {dark munchies} {topleft munchies}
op {<<} { A * (2**(B % 32)), which is left-shifting } {like *} {like *}
op {>>} { A / (2**(B % 32)), which is right-shifting } {like /,div} {like /,div}
op {||} { if A is zero then B else A }
op {&&} { if A is zero then zero else B}
op {min} { the lowest value in A,B } {clipping} {clipping (of individual points)}
op {max} { the highest value in A,B } {clipping} {clipping (of individual points)}
op {cmp} { -1 when A&lt;B; 0 when A=B; 1 when A&gt;B. }
op {==} { is A equal to B ? 1=true, 0=false }
op {!=} { is A not equal to B ? }
op {>} { is A greater than B ? }
op {<=} { is A not greater than B ? }
op {<} { is A less than B ? }
op {>=} {is A not less than B ? }
op {sin*} { B * sin(A) in centidegrees } {--} {waves, rotations}
op {cos*} { B * cos(A) in centidegrees } {--} {waves, rotations}
op {atan} { arctan(A/B) in centidegrees } {--} {find angle to origin (part of polar transform)}
op {tanh*} { B * tanh(A) in centidegrees } {smooth clipping} {smooth clipping (of individual points), neural sigmoid, fuzzy logic}
op {log*} { B * log(A) (in base e) }
op {gamma} { floor(pow(a/256.0,256.0/b)*256.0) } {gamma correction}
op {**} { A**B, that is, A raised to power B } {gamma correction}
op {abs-} { absolute value of (A-B) }
op {rand} { randomly produces a non-negative number below A }
op {sqrt} { square root of A, rounded downwards }
op {sq-} { (A-B) times (A-B) }
op {avg} { (A+B)/2 }
op {hypot} { distance function: square root of (A*A+B*B) }
op {clip+} { like A+B but overflow causes clipping instead of wrapping around (coming soon) }
op {clip-} { like A-B but overflow causes clipping instead of wrapping around (coming soon) }
op {erf*} { integral of e^(-x*x) dx ... (coming soon; what ought to be the scaling factor?) }
op {weight} { number of "1" bits in an integer}
op {sin} {sin(A-B) in radians, float only}
op {cos} {cos(A-B) in radians, float only}
op {atan2} {atan2(A,B) in radians, float only}
op {tanh} {tanh(A-B) in radians, float only}
op {exp} {exp(A-B) in radians, float only}
op {log} {log(A-B) in radians, float only}

section {vecops for complex numbers}
op {C.*    } {A*B}
op {C.*conj} {A*conj(B)}
op {C./    } {A/B}
op {C./conj} {A/conj(B)}
op {C.sq-  } {(A-B)*(A-B)}
op {C.abs- } {abs(A-B)}
op {C.sin  } {sin(A-B)}
op {C.cos  } {cos(A-B)}
op {C.tanh } {tanh(A-B)}
op {C.exp  } {exp(A-B)}
op {C.log  } {log(A-B)}

#section {vecops for other things}
#op {cart2pol}
#op {pol2cart}

incr y 10
set outletid $oid
obj 10 $y outlet
incr y 20

foreach msgbox $msgboxes {write [list #X connect $msgbox 0 $outletid 0]}

draw_columns

foreach section $sections {
	mset {y1 desc} $section
	obj 0 $y1 cnv 15 $::col4 18 empty empty empty 20 12 0 14 -248881 -66577 0
	text 10 $y1 $desc
}

p {
	note: a centidegree is 0.01 degree. There are 36000 centidegrees in a circle.
        Some angle operators use centidegrees, while some others use radians. To
        convert degrees into centidegrees, multiply by 100.
        To convert degrees into radians, divide by 57.2957 .
        Thus, to convert centidegrees into radians, divide by 5729.57 .
}

close $fh
set fh [open numtype.pd w]
write [list #N canvas 0 0 1024 768 10]
set y 0
set row 0
set oid 0
set col1 192
set col2 384
set col3 608
set col4 1024
set rowsize 64

obj 0 $y cnv 15 $col4 30 empty empty empty 20 12 0 14 20 -66577 0
text 10 $y op names
text $col1 $y range
text $col2 $y precision
text $col3 $y description
incr y 32

numbertype {b  u8   uint8} {0 to 255} {1} {
	unsigned 8-bit integer. this is the usual size of numbers taken from files and cameras, and
	written to files and to windows. (however #in converts to int32 unless otherwise specified.)}
numbertype {s i16   int16} {-32768 to 32767} {1}
numbertype {i i32   int32} {-(1<<31) to (1<<31)-1} {1} {
	signed 32-bit integer. this is used by default throughout GridFlow.
}
numbertype {l i64   int64} {-(1<<63) to (1<<63)-1} {1}
numbertype {f f32 float32} {-(1<<128) to (1<<128)} {23 bits or 0.000012%}
numbertype {d f64 float64} {-(1<<2048) to (1<<2048)} {52 bits or 0.000000000000022%}

draw_columns

p {High-performance computation requires precise and quite peculiar
	definitions of numbers and their representation.}
p {Inside most programs, numbers are written down as strings of 
	bits. A bit is either zero or one. Just like the decimal system 
	uses units, tens, hundreds, the binary system uses units, twos, 
	fours, eights, sixteens, and so on, doubling every time.}
p {One notation, called integer allows for only integer values to be 
	written (no fractions). when it is unsigned, no negative values may 
	be written. when it is signed, one bit indicates whether the number 
	is positive or negative. Integer storage is usually fixed-size, so you have 
	bounds on the size of numbers, and if a result is too big it "wraps around", truncating the biggest 
	bits.}
p {Another notation, called floating point (or float) stores numbers using 
	a fixed number of significant digits, and a scale factor that allows for huge numbers 
	and tiny fractions at once. Note that 1/3 has periodic digits, but even 0.1 has periodic digits, 
	in binary coding; so expect some slight roundings; the precision offered should be 
	sufficient for most purposes. Make sure the errors of rounding don't accumulate, though.}

close $fh
