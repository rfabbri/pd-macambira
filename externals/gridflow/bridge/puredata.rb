=begin
	$Id: puredata.rb,v 1.1 2005-10-04 02:02:13 matju Exp $

	GridFlow
	Copyright (c) 2001,2002 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
=end

#  <matju> alx1: in puredata.rb, just after the header, you have a %w() block,
#  and in it you write the name of your object, and if your helpfile is not
#  named like your object, then you add an equal sign and the filename

#!@#$ DON'T PUT ABSTRACTIONS IN THE %w() !!!
# @mouse=help_mouse @motion_detection=help_motion_detect @fade=help_fade
# @apply_colormap_channelwise @checkers @complex_sq @contrast
# @posterize @ravel @greyscale_to_rgb @rgb_to_greyscale @solarize @spread
#rgb_to_yuv=#rgb_to_yuv_and_#yuv_to_rgb
#yuv_to_rgb=#rgb_to_yuv_and_#yuv_to_rgb
#clip #contrast #fade #numop #remap_image

# NEW help files
#!@#$ (what's #+-help.pd ? #print-help2.pd ?)
%w(
	# #cast #dim #reverse
	  #pack=#unpack-#pack
	#unpack=#unpack-#pack
	renamefile
	#in plotter_control
	listelement exec ls #print unix_time
).each {|name|
	if name =~ /=/ then name,file = name.split(/=/) else file = name end
	begin
		x = GridFlow.fclasses[name]
		x.set_help "gridflow/flow_classes/#{file}-help.pd"
	rescue Exception => e
		GridFlow.post "for [#{name}], #{e.class}: #{e}" # + ":\n" + e.backtrace.join("\n")
	end
}

# OLD help files
%w(
	@cast
	@convolve @downscale_by @draw_polygon @export=@importexport
	@finished @fold @for @global @grade
	@import=@importexport @inner=@foldinnerouter
	@in=@inout @join @layer @outer=@foldinnerouter @out=@inout
	@! @perspective printargs @print @redim
	rubyprint @scale_by @scale_to @scan
	@store
).each {|name|
	if name =~ /=/ then name,file = name.split(/=/) else file = name end
	begin
		GridFlow.fclasses[name].set_help "gridflow/#{file}.pd"
	rescue Exception => e
		GridFlow.post "ruby #{e.class}: #{e}:\n" + e.backtrace.join("\n")
	end
}

#GridFlow.gui "frame .controls.gridflow -relief ridge -borderwidth 2\n"
#GridFlow.gui "button .controls.gridflow.button -text FOO\n"
#GridFlow.gui "pack .controls.gridflow.button -side left\n"
#GridFlow.gui "pack .controls.gridflow -side right\n"

GridFlow.gui %q{

if {[catch {
	# pd 0.37
	menu .mbar.gridflow -tearoff $pd_tearoff
	.mbar add cascade -label "GridFlow" -menu .mbar.gridflow
	set gfmenu .mbar.gridflow
}]} {
	# pd 0.36
	###the problem is that GridFlow.bind requires 0.37
}
catch {
$gfmenu add command -label "profiler_dump"  -command {pd "gridflow profiler_dump;"}
$gfmenu add command -label "profiler_reset" -command {pd "gridflow profiler_reset;"}
$gfmenu add command -label "formats"        -command {pd "gridflow formats;"}
}

if {[string length [info command post]] == 0} {
	proc post {x} {puts $x}
}

# if not running Impd:
if {[string length [info command listener_new]] == 0} {
# start of part duplicated from Impd
proc listener_new {self name} {
	global _
	set _($self:hist) {}
	set _($self:histi) 0
	frame $self
	label $self.label -text "$name: "
	entry $self.entry -width 40
#	entry $self.count -width 5
	pack $self.label -side left
	pack $self.entry -side left -fill x -expand yes
#	pack $self.count -side left
	pack $self -fill x -expand no
	bind $self.entry <Up>     "listener_up   $self"
	bind $self.entry <Down>   "listener_down $self"
}

proc listener_up {self} {
	global _
	if {$_($self:histi) > 0} {set _($self:histi) [expr -1+$_($self:histi)]}
	$self.entry delete 0 end
	$self.entry insert 0 [lindex $_($self:hist) $_($self:histi)]
	$self.entry icursor end
#	$self.count delete 0 end
#	$self.count insert 0 "$_($self:histi)/[llength $_($self:hist)]"
}

proc listener_down {self} {
	global _
	if {$_($self:histi) < [llength $_($self:hist)]} {incr _($self:histi)}
	$self.entry delete 0 end
	$self.entry insert 0 [lindex $_($self:hist) $_($self:histi)]
	$self.entry icursor end
#	$self.count delete 0 end
#	$self.count insert 0 "$_($self:histi)/[llength $_($self:hist)]"
}

proc listener_append {self v} {
	global _
	lappend _($self:hist) $v
	set _($self:histi) [llength $_($self:hist)]
}

proc tcl_eval {} {
	set l [.tcl.entry get]
	post "tcl: $l"
	post "returns: [eval $l]"
	listener_append .tcl [.tcl.entry get]
	.tcl.entry delete 0 end

}
if {[catch {
	listener_new .tcl "Tcl"
	bind .tcl.entry <Return> {tcl_eval}
}]} {
	listener_new .tcl "Tcl" {tcl_eval}
}


}
# end of part duplicated from Impd

proc ruby_eval {} {
	set l {}
	foreach c [split [.ruby.entry get] ""] {lappend l [scan $c %c]}
	pd "gridflow eval $l;"
	listener_append .ruby [.ruby.entry get]
	.ruby.entry delete 0 end
}

if {[catch {
	listener_new .ruby "Ruby"
	bind .ruby.entry <Return> {ruby_eval}
}]} {
	listener_new .ruby "Ruby" {ruby_eval}
}

} # GridFlow.gui

if false
GridFlow.gui %q{
catch {
	if {[file exists ${pd_guidir}/lib/gridflow/icons/peephole.gif]} {
		global pd_guidir
		image create photo icon_peephole -file ${pd_guidir}/lib/gridflow/icons/peephole.gif
		global butt
		button_bar_add peephole {guiext peephole}
	} {
		puts $stderr GAAAH
	}
}
} # GridFlow.gui
end
