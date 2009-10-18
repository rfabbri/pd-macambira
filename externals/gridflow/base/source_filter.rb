#!/usr/bin/env ruby
=begin
	$Id: source_filter.rb 4050 2008-07-18 16:44:51Z matju $

	GridFlow
	Copyright (c) 2001-2008 by Mathieu Bouchard

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

$stack = []
$classes = []
$exit = 0

ClassDecl = Struct.new(:name,:supername,:methods,:grins,:attrs,:info)
MethodDecl = Struct.new(:rettype,:selector,:arglist,:minargs,:maxargs,:where)
Arg  = Struct.new(:type,:name,:default)
Attr = Struct.new(:type,:name,:default,:virtual)

class MethodDecl
	def ==(o)
		return false unless rettype==o.rettype && maxargs==o.maxargs
		arglist.each_index{|i| arglist[i] == o.arglist[i] or return false }
		return true
	end
	def ===(o)
		return false unless rettype==o.rettype && maxargs==o.maxargs
		arglist.each_index{|i| arglist[i].type == o.arglist[i].type and arglist[i].default == o.arglist[i].default or return false }
		return true
	end
	attr_accessor :done
end

class Arg
  def canon(type)
    type="Grid *" if type=="PtrGrid"
    type
  end
  def ==(o) canon(type)==canon(o.type) && name==o.name end
end

In = File.open ARGV[0], "r"
Out = File.open ARGV[1], "w"

def handle_class(line)
	raise "already in class #{where}" if $stack[-1] and ClassDecl===$stack[-1]
	/^(\w+)(?:\s*[:<]\s*(\w+))?\s*(\{.*)?/.match line or raise "syntax error #{where}"
	classname = $1
	superclassname = $2
	rest = $3
	q=ClassDecl.new(classname,superclassname,{},{},{},false)
	$stack << q
	$classes << q
	Out.print "#define THISCLASS #{classname}\n\# #{$linenumber}\n"
	if rest and /^\{/ =~ rest then
		Out.print "struct #{classname} "
		Out.print ": #{superclassname}" if superclassname
		Out.print rest
	end
end

def parse_methoddecl(line,term)
	/^(\w+(?:\s*\*)?)\s+(\w+)\s*\(([^\)]*)\)\s*#{term}/.match line or
		raise "syntax error #{where} #{line}"
	rettype,selector,arglist = $1,$2,$3,$4
	if /^\d+$/ =~ rettype then
		selector = "_"+rettype+"_"+selector
		rettype = "void"
	end
	arglist,minargs,maxargs = parse_arglist arglist
	MethodDecl.new(rettype,selector,arglist,minargs,maxargs,where)
end

def parse_arglist(arglist)
	arglist = arglist.split(/,/)
	maxargs = arglist.length
	args = arglist.map {|arg|
		if /^\s*\.\.\.\s*$/.match arg then maxargs=-1; next end
		/^\s*([\w\s\*<>]+)\s*\b(\w+)\s*(?:\=(.*))?/.match arg or
			raise "syntax error in \"#{arg}\" #{where}"
		type,name,default=$1,$2,$3
		Arg.new(type.sub(/\s+$/,""),name,default)
	}.compact
	minargs = args.length
	minargs-=1 while minargs>0 and args[minargs-1].default
	[args,minargs,maxargs]
end

def unparse_arglist(arglist,with_default=true)
	arglist.map {|arg|
		x="#{arg.type} #{arg.name}"
		x << '=' << arg.default if with_default and arg.default
		x
	}.join(", ")
end

def where; "[#{ARGV[0]}:#{$linenumber}]" end

def handle_attr(line)
	line.gsub!(/\/\/.*$/,"") # remove comment
	frame = $stack[-1]
	type = line.gsub(%r"//.*$","").gsub(%r"/\*.*\*/","").gsub(%r";?\s*$","")
	virtual = !!type.slice!(/\(\)$/)
	name = type.slice!(/\w+$/)
	raise "missing \\class #{where}" if not $stack[-1] or not ClassDecl===frame
	handle_decl "void ___get(t_symbol *s);" if frame.attrs.size==0
	frame.attrs[name]=Attr.new(type,name,nil,virtual)
	if virtual then
		handle_decl "#{type} #{name}();"
	else
		Out.print line
	end
	type.gsub!(/\s+$/,"")
	type.gsub!(/^\s+/,"")
	if type=="bool" then
		handle_decl "0 #{name} (#{type} #{name}=true);"
	else
		handle_decl "0 #{name} (#{type} #{name});"
	end
end

def handle_decl(line)
	frame = $stack[-1]
	raise "missing \\class #{where}" if not frame or not ClassDecl===frame
	classname = frame.name
	m = parse_methoddecl(line,";\s*$")
	frame.methods[m.selector] = m
	Out.print "#{m.rettype} #{m.selector}(VA"
	Out.print ", #{unparse_arglist m.arglist}" if m.arglist.length>0
	Out.print "); static void #{m.selector}_wrap(#{classname} *self, VA); "
end

def handle_def(line)
	m = parse_methoddecl(line,"\\{?.*$")
	term = line[/\{.*/]
	qlass = $stack[-1]
	raise "missing \\class #{where}" if not qlass or not ClassDecl===qlass
	classname = qlass.name
	n = m
	if qlass.methods[m.selector]
		m = qlass.methods[m.selector]
		if !m===n then
			STDERR.puts "ERROR: def does not match decl:"
			STDERR.puts "#{m.where}: \\decl #{m.inspect}"
			STDERR.puts "#{n.where}: \\def #{n.inspect}"
			$exit = 1
		end
	else
		qlass.methods[m.selector] = m
	end
	Out.print "void #{classname}::#{m.selector}_wrap(#{classname} *self, VA) {"
	Out.print "static const char *methodspec = \"#{qlass.name}::#{m.selector}(#{unparse_arglist m.arglist,false})\";"
	Out.print "#{m.rettype} foo;" if m.rettype!="void"
	Out.print "if (argc<#{m.minargs}"
	Out.print "||argc>#{m.maxargs}" if m.maxargs!=-1
	Out.print ") RAISE(\"got %d args instead of %d..%d in %s\",argc,#{m.minargs},#{m.maxargs},methodspec);"
	Out.print "foo = " if m.rettype!="void"
	Out.print " self->#{m.selector}(argc,argv"
	m.arglist.each_with_index{|arg,i|
		if arg.default then
			Out.print ",argc<#{i+1}?#{arg.default}:convert(argv[#{i}],(#{arg.type}*)0)"
		else
			Out.print ",convert(argv[#{i}],(#{arg.type}*)0)"
		end
	}
	Out.print ");} #{m.rettype} #{classname}::#{m.selector}(VA"
	#puts "m=#{m} n=#{n}"
	Out.print ","+unparse_arglist(n.arglist,false) if m.arglist.length>0
	Out.print ")#{term} "
	qlass.methods[m.selector].done=true
end

def handle_constructor(line)
	frame = $stack[-1]
	raise "missing \\class #{where}" if not frame or not ClassDecl===frame
	m = parse_methoddecl("void constructor"+line,"(.*)$")
	Out.print "#{frame.name}(BFObject *bself, MESSAGE) : #{frame.supername}(bself,MESSAGE2) {"
	Out.print "static const char *methodspec = \"#{frame.name}::#{m.selector}(#{unparse_arglist m.arglist,false})\";"

	Out.print "if (argc<#{m.minargs}"
	Out.print "||argc>#{m.maxargs}" if m.maxargs!=-1
	Out.print ") RAISE(\"got %d args instead of %d..%d in %s\",argc,#{m.minargs},#{m.maxargs},methodspec);"
	Out.print "#{m.selector}(sel,argc,argv"
	m.arglist.each_with_index{|arg,i|
		if arg.default then
			Out.print ",argc<#{i+1}?#{arg.default}:convert(argv[#{i}],(#{arg.type}*)0)"
		else
			Out.print ",convert(argv[#{i}],(#{arg.type}*)0)"
		end
	}
	Out.print ");}"
	Out.print "#{m.rettype} #{m.selector}(MESSAGE"
	Out.print ", #{unparse_arglist m.arglist}" if m.arglist.length>0
	Out.print ") "+line[/\{.*/]
end

def handle_classinfo(line)
	frame = $stack[-1]
	cl = frame.name
	line="{}" if /^\s*$/ =~ line
	Out.print "static void #{cl}_startup (FClass *fclass);"
	Out.print "static FObject *#{cl}_allocator (BFObject *bself, MESSAGE) {return new #{cl}(bself,sel,argc,argv);}"
	Out.print "static MethodDecl #{cl}_methods[] = {"
	Out.print frame.methods.map {|foo,method| "{ \"#{method.selector}\",(FMethod)#{frame.name}::#{method.selector}_wrap }" }.join(",")
	Out.print "}; FClass ci#{cl} = {#{cl}_allocator,#{cl}_startup,#{cl.inspect},COUNT(#{cl}_methods),#{cl}_methods};"
	get="void ___get(t_symbol *s=0) {t_atom a[1];"
	frame.attrs.each {|name,attr|
		virtual = if attr.virtual then "(0,0)" else "" end
		get << "if (s==gensym(\"#{name}\")) set_atom(a,#{name}#{virtual}); else "
		if frame.methods["_0_"+name].done then
			#STDERR.puts "skipping already defined \\attr #{name}"
			next
		end
		type,name,default = attr.to_a
		handle_def "0 #{name} (#{type} #{name}) {this->#{name}=#{name}; changed(gensym(\"#{name}\"));}"
	}
	line.gsub!(/^\s*(\w+\s*)?\{/,"")
	get << "RAISE(\"unknown attr %s\",s->s_name); outlet_anything(bself->outlets[bself->noutlets-1],s,1,a);}"
	handle_def get if frame.attrs.size>0
	Out.print "void #{frame.name}_startup (FClass *fclass) {"
	frame.attrs.each {|name,attr| Out.print "fclass->attrs[\"#{name}\"] = new AttrDecl(\"#{name}\",\"#{attr.type}\");" }
	Out.print line.chomp
end

def handle_grin(line)
	fields = line.split(/\s+/)
	i = fields[0].to_i
	c = $stack[-1].name
	frame = $stack[-1]
	Out.print "template <class T> void grin_#{i}(GRIDHANDLER_ARGS(T));"
	Out.print "template <class T> static void grinw_#{i} (GRIDHANDLER_ARGS(T));"
	Out.print "static GridHandler grid_#{i}_hand;"
	handle_decl "#{i} grid(GridOutlet *foo);"
	handle_decl "#{i} list(...);"
	handle_decl "#{i} float(float f);"
	$stack[-1].grins[i] = fields.dup
end

def handle_end(line)
	frame = $stack.pop
	fields = line.split(/\s+/)
	n = fields.length
	if not ClassDecl===frame then raise "\\end: frame is not a \\class" end
	cl = frame.name
	if fields[0]!="class" or (n>1 and not /^\{/ =~ fields[1] and fields[1]!=cl) then raise "end not matching #{where}" end
	$stack.push frame
	frame.grins.each {|i,v|
		cli = "#{cl}::grinw_#{i}"
		k = case v[1]
		when    nil   ; [1,1,1,1,1,1]
		when   'int32'; [0,0,1,0,0,0]
		when   'int'  ; [1,1,1,1,0,0]
		when 'float'  ; [0,0,0,0,1,1]
		when 'float32'; [0,0,0,0,1,0]
		when 'float64'; [0,0,0,0,0,1]
		else raise 'BORK BORK BORK' end
		ks = k.map{|ke| if ke==0 then 0 else cli end}.join(",")
		Out.print "static GridHandler #{cl}_grid_#{i}_hand = GRIN(#{ks});"
		handle_def "#{i} grid(GridOutlet *foo) {CHECK_GRIN(#{cl},#{i});"+
			"in[#{i}]->begin(foo);}"
		handle_def "#{i} list(...) {CHECK_GRIN(#{cl},#{i});"+
			"in[#{i}]->from_list(argc,argv,int32_e);}" if not frame.methods["_#{i}_list"].done
		handle_def "#{i} float(float f) {CHECK_GRIN(#{cl},#{i});"+
			"t_atom2 a[1]; SETFLOAT(a,f);"+
			"in[#{i}]->from_atom(1,a);}" if not frame.methods["_#{i}_float"].done
	}
	if /^class\s*(\w+\s+)?\{(.*)/ =~ line then handle_classinfo("{"+$2) end
	$stack.pop
	Out.print "\n#undef THISCLASS\n\# #{$linenumber}\n"
end

def handle_startall(line)
	$classes.each {|q|
		Out.print "fclass_install(&ci#{q.name},"
		if q.supername then Out.print "&ci#{q.supername}" else Out.print "0" end
		Out.print ",sizeof(#{q.name}));"
	}
end

$linenumber=1
loop{
	x = In.gets
	break if not x
	if /^\s*\\(\w+)\s*(.*)$/.match x then
		begin
			send("handle_#{$1}",$2)
			Out.puts "//FCS"
		rescue StandardError => e
			STDERR.puts e.inspect, "at line #{$linenumber}", e.backtrace
			File.unlink ARGV[1]
			exit 1
		end
	else Out.puts x end
	$linenumber+=1
}

exit $exit
