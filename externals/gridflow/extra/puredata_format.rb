=begin
	$Id: puredata_format.rb,v 1.2 2006-03-15 04:40:47 matju Exp $

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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

class PureDataFileWriter
	def initialize filename
		@f = File.open filename, "w"
	end
	def close; @f.close end
	def write_patcher o
		pr = o.properties
		@f.puts "#N canvas #{pr[:wx]} #{pr[:wy]} #{pr[:ww]} #{pr[:wh]} 10;"
		ol = o.subobjects.keys
		x=0
		ol.find_all {|a| a.classname=="inlet"}.sort {|a,b| a.argv[0] <=> b.argv[0] }.each {|a|
			if x>a.properties[:x]
				a.properties[:x]=x+16
				STDERR.puts "warning: moving inlet #{a.argv[0]} to the right"
			end
			x=a.properties[:x]
		}
		x=0
		ol.find_all {|a| a.classname=="outlet"}.sort {|a,b| a.argv[0] <=> b.argv[0] }.each {|a|
			if x>a.properties[:x]
				a.properties[:x]=x+16
				STDERR.puts "warning: moving outlet #{a.argv[0]} to the right"
			end
			x=a.properties[:x]
		}
		ol.each {|so| write_object so }
		ol.each_with_index {|so,i|
			next if not so.instance_eval{defined? @outlets}
			so.outlets.each_with_index {|conns,outlet|
				next if not conns
				conns.each {|target,inlet|
					@f.puts "#X connect #{i} #{outlet} #{ol.index target} #{inlet};"
				}
			}
		}
	end

	def list_to_s l
		l.map {|x|
			if Array===x then "( " + list_to_s(x) + " )" else escape(x.to_s) end
		}.join " "
	end

	# what am i supposed to do?
	def escape  x; x.gsub(/[;,]/) {|x| " \\#{x}" }.gsub(/\n/) {"\\\n"} end
	def escape2 x; x.gsub(/[,]/) {|x| " \\#{x} " }.gsub(/[;\$\"]/) {|x| "\\#{x}" }.gsub(/\n/) {"\\\n"} end

	def write_object o
		pr = o.properties
		#classname = o.class.instance_eval{@foreign_name}
		classname = o.classname
		if classname=="jpatcher"
			#@f.print "#N canvas 0 0 "
			write_patcher o
		end

		case classname
		when "display"; classname="print"
		when "list"; classname="listmake"
		end

		t = case classname
		when "jcomment"; "text"
		when "messbox"; "msg"
		when "jpatcher"; "restore"
		when "intbox"; "floatatom"
		else "obj"
		end
		@f.print "#X #{t} #{pr[:x]} #{pr[:y]} "

		case classname
		when "button"
			@f.print "bng 15 250 50 0 empty empty empty 0 -6 0 8 -262144 -1 -1"
		when "jcomment"
			@f.print escape2(pr[:comment].to_s)
		when "messbox"
			av=o.argv[0]
			i=0
			dollar="$".intern
			while i<av.length
			  if av[i]==dollar then av[i,2]=("\\$"+av[i+1].to_s).intern else i+=1 end
			end
			@f.print(list_to_s(av))
		when "slider"
			#doradio = pr[:maxValue]-pr[:minValue]<=10
			doradio = false
			#p doradio
			name = case pr[:orientation]
				when 1;     if doradio then "hradio" else "hsl" end
				when 2,nil; if doradio then "vradio" else "vsl" end
				else raise "bogus slider orientation?" end
			@f.print "#{name} "+
			"#{pr[:w]} #{pr[:h]} #{pr[:minValue]} #{pr[:maxValue]} 0 0 "+
			"empty empty empty -2 -6 0 8 -262144 -1 -1 0 1"
		when "intbox"
			@f.print "5 0 0 0 - - -;"
		when "inlet"; @f.print "inlet"
		when "outlet"; @f.print "outlet"
		when "jpatcher"
			@f.print("pd ",list_to_s(o.argv))
		else
			@f.print(classname," ",list_to_s(o.argv))
		end
		@f.puts ";"
	end
end
