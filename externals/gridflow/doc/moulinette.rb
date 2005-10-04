=begin
	$Id: moulinette.rb,v 1.1 2005-10-04 02:02:14 matju Exp $
	convert GridFlow Documentation XML to HTML with special formatting.

	GridFlow
	Copyright (c) 2001,2002,2003,2004,2005 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
=end

GF_VERSION = "0.8.0"

#$use_rexml = true
$use_rexml = false

#require "gridflow"

if $use_rexml
	# this is a pure ruby xml-parser
	begin
		require "rexml/sax2parser"
	rescue LoadError
		require "rexml/parsers/sax2parser"
		include REXML::Parsers
	end
	include REXML
else
	# this uses libexpat.so
	require "xmlparser"
end

=begin todo

	[ ] make it use the mk() function as much as possible.
	[ ] make it validate
	[ ] make it find the size of the pictures (and insert width/height attrs)
	[ ] tune the output
	[ ] fix the header of the page

=end

if nil
	alias real_print print
	alias real_puts  puts
	def print(*x); real_print "[#{caller[0]}]"; real_print *x; end
	def puts (*x); real_print "[#{caller[0]}]"; real_puts  *x; end
end

def warn(text)
	STDERR.print "\e[1;031mWARNING:\e[0m "
	STDERR.puts text
end

$escape_map={
	"<" => "&lt;",
	">" => "&gt;",
	"&" => "&amp;",
}

# hackish transcoding from unicode to iso-8859-1
def multicode(text); text.gsub(/\xc2(.)/) { $1 } end

def html_quote(text)
	return nil if not text
	text = text.gsub(/[<>&]/) {|x| $escape_map[x] }
	text = multicode(text) if /\xc2/ =~ text
	text
end

def mk(tag,*values,&block)
	raise "value-list's length must be even" if values.length % 2 != 0
	print "<#{tag}"
	i=0
	while i<values.length
		print " #{values[i]}=\"#{values[i+1]}\""
		i+=2
	end
	print ">"
	(block[]; mke tag) if block
end
def mke(tag)
	print "</#{tag}>"
end

def mkimg(parent,alt=nil,prefix=nil)
	#STDERR.puts parent.to_s
	icon = parent.contents.find {|x| XNode===x and x.tag == 'icon' }
	name = parent.att["name"]
	url = prefix+"/"+name+"-icon.png"
	if icon and icon.att["src"]
		url = icon.att["src"]
		STDERR.puts "overriding #{url} with #{icon.att["src"]}"
	end
	url = url.sub(/,.*$/,"") # what's this for again?
	warn "icon #{url} not found" if not File.exist? url
	url = url.gsub(%r"#") {|x| sprintf "%%%02x", x[0] }
	alt = icon.att["text"] if icon and not alt
	alt = "[#{name}]"
	mk(:img, :src, url, :alt, alt, :border, 0)
end

class XString < String
	def show
		print html_quote(gsub(/[\r\n\t ]+$/," "))
	end
end

module HasOwnLayout; end

class XNode
	# subclass interface:
	#  #show_index : print as html in index
	#  #show : print as html in main part of the doc

	@valid_tags = {}
	class<<self
		attr_reader :valid_tags
		def register(*args,&b)
			qlass = (if b then Class.new self else self end)
			qlass.class_eval(&b) if b
			for k in args do XNode.valid_tags[k]=qlass end
			#qlass.class_eval {
			#	public :show
			#	public :show_index
			#}
		end
	end

	def initialize tag, att, *contents
		@tag,@att,@contents =
		 tag, att, contents
		contents.each {|c| c.parent = self if XNode===c }
	end

	attr_reader :tag, :att, :contents
	attr_accessor :parent
	def [] i; contents[i] end

	def show_index
		contents.each {|x| next unless XNode===x; x.show_index }
	end

	# this method segfaults in ruby 1.8
	# because of method lookup on Qundef or whatever.
	def show
		#STDERR.puts GridFlow.get_id(contents)
		#STDERR.puts self
		contents.each {|x|
			# STDERR.puts GridFlow.get_id(x)
			x.show
		}
	end
	def inspect; "#<XNode #{tag}>"; end
	def to_s; inspect; end
end

XNode.register("documentation") {}

XNode.register(*%w( icon help arg rest )) {public
	def show; end
}

XNode.register("section") {public
	def show
		write_black_ruler
		mk(:tr) { mk(:td,:colspan,4) {
			mk(:a,:name,att["name"].gsub(/ /,'_')) {}
			mk(:h4) { print att["name"] }}}
		
		contents.each {|x|
			if HasOwnLayout===x then
				x.show
			else
				mk(:tr) { mk(:td) {}; mk(:td) {}; mk(:td) { x.show }}
				puts ""
			end
		}

		mk(:tr) { mk(:td) { print "&nbsp;" }}
		puts ""
	end
	def show_index
		mk(:h4) {
			mk(:a,:href,"#"+att["name"].gsub(/ /,'_')) {
				print att["name"] }}
		print "<ul>\n"
		super
		print "</ul>\n"
	end
}

# basic text formatting nodes.
XNode.register(*%w( p i u b sup )) {public
	def show
		print "<#{tag}>"
		super
		print "</#{tag}>"
	end
}

XNode.register("k") {public
	def show
		print "<kbd><font color=\"#007777\">" # oughta be in stylesheet?
		super
		print "</font></kbd>"
	end
}

# explicit hyperlink on the web.
XNode.register("link") {public
	def show
		STDERR.puts "att = #{att.inspect}"
		raise if not att['to']
		print "<a href='#{att['to']}'>"
		super
		print att[:to] if contents.length==0
		print "</a>"
	end
}

XNode.register("list") {public
	attr_accessor :counter
	def show
		self.counter = att.fetch("start"){"1"}.to_i
		mk(:ul) {
			super # method call on Qundef ???
		}
	end
}

XNode.register("li") {public
	def show
		mk(:li) {
			print "<b>#{parent.counter}</b>", " : "
			parent.counter += 1
			super
		}
	end
}

# and "macro", "enum", "type", "use"
XNode.register("class") {public
	include HasOwnLayout
	def show
		tag = self.tag
		name = att['name'] or raise
		mk(:tr) {
		  mk(:td,:colspan,4,:bgcolor,"#ffb080") {
		    mk(:b) { print "&nbsp;"*2, "#{tag} " }
		    mk(:a,:name,name) { print name }
		  }
		}
		mk(:tr) {
		mk(:td) {}
		mk(:td,:valign,:top) {
		print "<br>\n"
		help = contents.find {|x| XNode===x and x.tag == 'help' }
		mkimg(self,nil,"flow_classes") if /reference|format/ =~ $file
		mk(:br,:clear,"left")
		2.times { mk(:br) }
		if help
			big = help.att['image'] || att['name']
			if big[0]==?@ then big="images/help_#{big}.png" end
			warn "help #{big} not found" if not File.exist?(big)
			#small = big.gsub(/png$/, 'jpg').gsub(/\//, '/ic_')
			mk(:a,:href,big) {
				#mk(:img,:src,small,:border,0)
				mk(:img,:src,"images/see_screenshot.png",:border,0)
			}
		end
		mk(:br,:clear,"left")
		mk(:br)
		}#/td
		mk(:td) {
			print "<br>\n"
			super
			print "<br>"
		}#/td
		}#/tr
	end
	def show_index
		icon = contents.find {|x| XNode===x && x.tag == "icon" }
		if not att["name"] then
			raise "name tag missing?"
		end
		mk(:li) { mk(:a,:href,"\#"+att["name"]) {
			mkimg(self,att["cname"],"flow_classes")
		}}
		puts
		super
	end
}

def nice_table
	mk(:table,:border,0,:bgcolor,:black,:cellspacing,1) {
		mk(:tr) {
			mk(:td,:valign,:top,:align,:left) {
				mk(:table,:bgcolor,:white,:border,0,
				:cellpadding,4,:cellspacing,1) {
					yield }}}}
end

XNode.register("attr") {public
	def show
		print "<br>"
		if parent.tag == "inlet" or parent.tag == "outlet"
			mk(:b) {
				print "#{parent.tag}&nbsp;#{parent.att['id']} "
				
			}
		end
		print "<b>#{tag}</b>&nbsp;"
		print "#{html_quote att['name']} <b>(</b>"
		s=html_quote(att["name"])
		s="<i>#{att['type']}</i> #{s}" if att['type']
		print "<b>#{s}</b>"
		print "<b>)</b> "
	end
}

XNode.register("method") {public
if true #
	def show
		print "<br>"
		if parent.tag == "inlet" or parent.tag == "outlet"
			mk(:b) {
				print "#{parent.tag}&nbsp;#{parent.att['id']} "
				
			}
		end
		print "<b>#{tag}</b>&nbsp;"
		print "#{html_quote att['name']} <b>(</b>"
		print contents.map {|x|
			next unless XNode===x
			case x.tag
			when "arg"
				s=html_quote(x.att["name"])
				s="<i>#{x.att['type']}</i> #{s}" if x.att['type']
				s
			when "rest"
				(x.att["name"]||"") + "..."
			end
		}.compact.join("<b>, </b>")
		print "<b>)</b> "
		super
		print "<br>\n"
	end
else #
	def show
		print "<br>"
		mk(:table) { mk(:tr) { mk(:td) {
		name = ""
		name << "#{parent.tag} #{parent.att['id']} " if \
			parent.tag == "inlet" or parent.tag == "outlet"
		name << tag.to_s
		mk(:b) { print name.gsub(/ /,"&nbsp;") }
		}; mk(:td) {
		nice_table { mk(:tr) {
			mk(:td,:width,1) { print html_quote(att['name']) }
			contents.each {|x|
				next unless XNode===x
				case x.tag
				when "arg"
					mk(:td,:bgcolor,:pink) {
						s = ""
						if x.att["type"]
							s << "<i>" << html_quote(x.att["type"]) << "</i>"
						end
						if x.att["name"]
							s << " " << html_quote(x.att["name"])
						end
						s<<"&nbsp;" if s.length==0
						mk(:b) { puts s }
					}
				when "rest"
					mk(:td,:bgcolor,:pink) {
						mk(:b) { print html_quote(x.att["name"]), "..."}
					}
				end
			}
		}}
		}; mk(:td) {
		super
		}}}
	end
end #
}

XNode.register("table") {public
	def show
		colors = ["#ffffff","#f0f8ff",]
		rows = contents.find_all {|x| XNode===x && x.tag=="row" }
		rows.each_with_index {|x,i| x.bgcolor = colors[i%2] }
		mk(:tr) {
		2.times { mk(:td) {} }
		mk(:td) {
		nice_table {
			mk(:tr) {
				columns = contents.find_all {|x| XNode===x && x.tag=="column" }
				columns.each {|x| mk(:td,:bgcolor,"#808080") {
				mk(:font,:color,"#ffffff") {
				mk(:b) {
					x.contents.each {|y| y.show }}}}}
			}
			super
		}}}
	end
}

XNode.register("column") {public
	def show; end
}

XNode.register("row") {public
	attr_accessor :bgcolor
	def show
		columns = parent.contents.find_all {|x| XNode===x && x.tag=="column" }
		mk(:tr) { columns.each {|x| mk(:td,:bgcolor,bgcolor) {
			id = x.att["id"]
			case x.att["type"]
			when "icon" # should fix this for non-op icons
				x = "op/#{att['cname']}-icon.png"
				if not File.exist? x
					warn "no icon for #{att['name']} (#{x})\n"
				end
				mk(:img,:src,x,:border,0,:alt,att["name"])
			else
				if id==""
				then contents.each {|x| x.show }
				else
#					print html_quote(att[id] || "--")
					print multicode(att[id] || "--")
				end
			end
		}}}
	end
}

XNode.register("inlet","outlet") {}

#----------------------------------------------------------------#

if $use_rexml
	class GFDocParser
		def initialize(file)
			@sax = SAX2Parser.new(File.open(file))
			@xml_lists = []
			@stack = [[]]
			@sax.listen(:start_element) {|a,b,c,d| startElement(b,d) }
			@sax.listen(  :end_element) {|a,b,c|   endElement(b) }
			@sax.listen(   :characters) {|a| @gfdoc.character(a) }
		end
		def do_it; @sax.parse; end
	end
else
	class GFDocParser
		def initialize(file)
			@xml = XMLParser.new("ISO-8859-1")
			foo=self; @xml.instance_eval { @gfdoc=foo }
			def @xml.startElement(tag,attrs) @gfdoc.startElement(tag,attrs) end
			def @xml.endElement(tag) @gfdoc.endElement(tag) end
			def @xml.character(text) @gfdoc.character(text) end
			@file = File.open file
			@xml_lists = []
			@stack = [[]]
		end
		def do_it; @xml.parse(@file.readlines.join("\n"), true) end
		def method_missing(sel,*args) @xml.send(sel,*args) end
	end
end

class GFDocParser
	attr_reader :stack
	def startElement(tag,attrs)
		if not XNode.valid_tags[tag] then
			raise XMLParserError, "unknown tag #{tag}"
		end
		@stack<<[tag,attrs]
	end
	def endElement(tag)
		node = XNode.valid_tags[tag].new(*@stack.pop)
		@stack.last << node
	end
	def character(text)
		if not String===@stack.last.last then
			@stack.last << XString.new("")
		end
		@stack.last.last << text
	end
end

#----------------------------------------------------------------#

def write_header(tree)
puts <<EOF
<html><head>
<!-- #{"$"}Id#{"$"} -->
<title>GridFlow #{GF_VERSION} - #{tree.att['title']}</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="gridflow.css" type="text/css">
</head>
<body bgcolor="#FFFFFF"
  leftmargin="0" topmargin="0"
  marginwidth="0" marginheight="0">
<table width="100%" bgcolor="white" border="0" cellspacing="2">
<tr><td colspan="4" bgcolor="#082069">
<img src="images/titre_gridflow.png" width="253" height="23">
</td></tr><tr><td>&nbsp;</td></tr>
EOF
write_black_ruler
puts <<EOF
<tr><td colspan="4" height="16"> 
    <h4>GridFlow #{GF_VERSION} - #{tree.att['title']}</h4>
</td></tr>
<tr> 
  <td width="5%"  rowspan="2">&nbsp;</td>
  <td width="15%" height="23">&nbsp;</td>
  <td width="80%" height="23">&nbsp;</td>
  <td width="5%"  height="23">&nbsp;</td>
</tr>
EOF
end

def write_black_ruler
puts <<EOF
<tr><td colspan="4" bgcolor="black">
<img src="images/black.png" width="1" height="2"></td></tr>
EOF
end

def write_footer
puts <<EOF
<td colspan="4" bgcolor="black">
<img src="images/black.png" width="1" height="2"></td></tr>
<tr><td colspan="4"> 
<p><font size="-1">
GridFlow #{GF_VERSION} Documentation<br>
Copyright &copy; 2001,2002,2003,2004,2005 by Mathieu Bouchard
<a href="mailto:matju@sympatico.ca">matju@artengine.ca</a>
</font></p>
</td></tr></table></body></html>
EOF
end

#----------------------------------------------------------------#

$nodes = {}
XMLParserError = Exception if $use_rexml

def read_one_page file
	begin
		STDERR.puts "reading #{file}"
		parser = GFDocParser.new(file)
		parser.do_it
		$nodes[file] = parser.stack[0][0]
	rescue Exception => e
		puts ""
		puts ""
		STDERR.puts e.inspect
		i = parser.stack.length-1
		(STDERR.puts "\tinside <#{parser.stack[i][0]}>"; i-=1) until i<1
		# strange that line numbers are doubled.
		# also the byte count is offset by the line count !?!?!?
		STDERR.puts "\tinside #{file}:#{parser.line/2 + 1}" +
			" (column #{parser.column}," +
			" byte #{parser.byteIndex - parser.line/2})"
		raise "why don't you fix the documentation"
	end
end

def write_one_page file
	begin
		$file = file
		output_name = file.sub(/\.xml/,".html")
		STDERR.puts "writing #{output_name}"
		STDOUT.reopen output_name, "w"
		tree = $nodes[file]
		write_header(tree)
		mk(:tr) { mk(:td,:colspan,2) { mk(:div,:cols,tree.att["indexcols"]||1) {
			tree.show_index
			puts "<br><br>"
		}}}
		tree.show
		write_footer
		puts ""
		puts ""
	rescue Exception => e
		STDERR.puts "#{e.class}: #{e.message}"
		STDERR.puts e.backtrace
	end
end

$files = %w(
	install.xml project_policy.xml
	reference.xml format.xml internals.xml architecture.xml)

$files.each {|input_name| read_one_page input_name }
$files.each {|input_name| write_one_page input_name }
