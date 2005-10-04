=begin
	$Id: main.rb,v 1.1 2005-10-04 02:02:13 matju Exp $

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

# ENV["RUBY_VERBOSE_GC"]="yes"

# this file gets loaded by main.c upon startup
# module GridFlow is supposed to be created by main.c
# this includes GridFlow.post_string(s)

# because Ruby1.6 has no #object_id and Ruby1.8 warns on #id
unless Object.instance_methods(true).include? "object_id"
	class Object; alias object_id id end
end

# in case of bug in Ruby ("Error: Success")
module Errno; class E000 < StandardError; end; end

#$post_log = File.open "/tmp/gridflow.log", "w"
$post_log = nil

class Array
	def split(elem)
		r=[]
		j=0
		for i in 0...length
			(r<<self[j,i-j]; j=i+1) if self[i]==elem
		end
		r<<self[j,length-j]
	end
end

module GridFlow #------------------

def self.post(s,*a)
	post_string(sprintf("%s"+s,post_header,*a))
	($post_log << sprintf(s,*a); $post_log.flush) if $post_log
end

class<<self
	attr_accessor :data_path
	attr_accessor :post_header
	attr_accessor :verbose
	attr_reader :fobjects
	attr_reader :fclasses
	attr_reader :cpu_hertz
	attr_reader :subprocesses
	attr_reader :bridge_name
	alias gfpost post
end

@subprocesses={}
@verbose=false
@data_path=[]
if GridFlow.respond_to? :config then
  @data_path << GridFlow.config["PUREDATA_PATH"]+"/extra/gridflow/images"
end

def self.hunt_zombies
	#STDERR.puts "GridFlow.hunt_zombies"
	# the $$ value is bogus
	begin
		died = []
		subprocesses.each {|x,v|
			Process.waitpid2(x,Process::WNOHANG) and died<<x
		}
	rescue Errno::ECHILD
	end
	#STDERR.puts died.inspect
	died.each {|x| subprocesses.delete x }
end

def self.packstring_for_nt(nt)
	case nt
	when :u, :u8,  :uint8; "C*"
	when :s, :i16, :int16; "s*"
	when :i, :i32, :int32; "l*"
	when :f, :f32, :float32; "f*"
	when :d, :f64, :float64; "d*"
	else raise "no decoder for #{nt.inspect}"
	end
end

self.post_header = "[gf] "

def self.gfpost2(fmt,s); post("%s",s) end

if GridFlow.bridge_name then
  post "This is GridFlow #{GridFlow::GF_VERSION} within Ruby version #{RUBY_VERSION}"
  post "base/main.c was compiled on #{GridFlow::GF_COMPILE_TIME}"
  post "Please use at least 1.6.6 if you plan to use sockets" if RUBY_VERSION<"1.6.6"
end

if not GridFlow.bridge_name then
  require "gridflow/bridge/placebo"
end

Brace1 = "{".intern
Brace2 = "}".intern
Paren1 = "(".intern
Paren2 = ")".intern

def self.parse(m)
	m = m.gsub(/(\{|\})/," \\1 ").split(/\s+/)
	m.map! {|x| case x
		when Integer, Symbol; x
		when /^[+\-]?[0-9]+$/; x.to_i
		when String; x.intern
		end
	}
	m
end

def self.stringify_list(argv)
	argv.map {|x| stringify x }.join(" ")
end

def self.stringify(arg)
	case arg
	when Integer, Float, Symbol; arg.to_s
	when Array; "{#{stringify_list arg}}"
	end
end

::Object.module_eval do def FloatOrSymbol(x) Float(x) rescue x.intern end end

# adding some functionality to that:
class FObject
	@broken_ok = false
	@do_loadbangs = true
	class<<self
		# global
		attr_accessor :broken_ok
		# per-class
		attr_reader :ninlets
		attr_reader :noutlets
		attr_accessor :do_loadbangs
		attr_accessor :comment
		def foreign_name; @foreign_name if defined? @foreign_name end
	end
	def post(*a) GridFlow.post(*a) end
	def self.subclass(*args,&b)
		qlass = Class.new self
		qlass.install(*args)
		qlass.module_eval(&b)
	end
	alias :total_time :total_time_get
	alias :total_time= :total_time_set
	attr_writer :args # String
	attr_accessor :argv # Array
	attr_reader :outlets
	attr_accessor :parent_patcher
	attr_accessor :properties
	attr_accessor :classname
	def initialize2; end
	def args
		if defined? @args
			@args
		else
			"[#{self.class} ...]"
		end
	end
	alias info args
	def connect outlet, object, inlet
		@outlets ||= []
		@outlets[outlet] ||= []
		@outlets[outlet].push [object, inlet]
	end
	def self.name_lookup sym
		qlasses = GridFlow.fclasses
		qlass = qlasses[sym.to_s]
		if not qlass
			return qlasses['broken'] if @broken_ok
			raise "object class '#{sym}' not found"
		end
		qlass
	end
	def self.[](*m)
		o=nil
		if m.length==1 and m[0] =~ / /
			o="[#{m[0]}]"
			m=GridFlow.parse(m[0]) 
		else
			o=m.inspect
		end
		GridFlow.handle_braces!(m)
		ms = m.split ','.intern
		m = ms.shift
		qlass = m.shift
		qlassname = qlass.to_s
		qlass = name_lookup qlass.to_s unless Class===qlass
		r = qlass.new(*m)
		r.classname = qlassname
		GridFlow.post "%s",r.args if GridFlow.verbose
		for x in ms do r.send_in(-2, *x) end if FObject.do_loadbangs
		r
	end
	def inspect
		if args then "#<#{self.class} #{args}>" else super end
	end
	def initialize(*argv)
		s = GridFlow.stringify_list argv
		@argv = argv
		@args = "["
		@args << (self.class.foreign_name || self.to_s)
		@args << " " if s.length>0
		@args << s << "]"
		@parent_patcher = nil
		@properties = {}
		@init_messages = []
	end
end

class FPatcher < FObject
	class << self
		attr_reader :fobjects
		attr_reader :wires
	end
	def initialize(*)
		super
		fobjects = self.class.fobjects
		wires = self.class.wires
		@fobjects = fobjects.map {|x| if String===x then FObject[x] else x.call end }
		@inlets = []
		@ninlets = self.class.ninlets or raise "oops"
		i=0
		@fobjects << self
		while i<wires.length do
			a,b,c,d = wires[i,4]
			if a==-1 then
				a=self
				@inlets[b]||=[]
				@inlets[b] << [@fobjects[c],d]
			else
				if c==-1 then
					@fobjects[a].connect b,self,d+@ninlets
				else
					@fobjects[a].connect b,@fobjects[c],d
				end
			end
			i+=4
		end
	end
	def method_missing(sym,*args)
		sym=sym.to_s
		if sym =~ /^_(\d)_(.*)/ then
			inl = Integer $1
			sym = $2.intern
			if inl<@ninlets then
			raise "#{inspect} has not @inlets[#{inl}]" if not @inlets[inl]
				for x in @inlets[inl] do
				 x[0].send_in x[1],sym,*args end
			else
				send_out(inl-@ninlets,sym,*args)
			end
		else super end
	end
end

def GridFlow.estimate_cpu_clock
	u0,t0=GridFlow.rdtsc,Time.new.to_f; sleep 0.01
	u1,t1=GridFlow.rdtsc,Time.new.to_f; (u1-u0)/(t1-t0)
end

begin
	@cpu_hertz = (0...3).map {
		GridFlow.estimate_cpu_clock
	}.sort[1] # median of three tries
rescue
	GridFlow.post $!
end

def GridFlow.find_file s
	s=s.to_s
	if s==File.basename(s) then
		dir = GridFlow.data_path.find {|x| File.exist? "#{x}/#{s}" }
		if dir then "#{dir}/#{s}" else s end
	elsif GridFlow.respond_to? :find_file_2
		GridFlow.find_file_2 s
	else
		s
	end
end

def GridFlow.macerr(i)
  begin
    f=File.open("/System/Library/Frameworks/CoreServices.framework/"+
      "Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers/"+
      "MacErrors.h")
    while f.gets
      m = /^\s*(\w+)\s*=\s*(-\d+),\s*\/\*\s*(.*)\s*\*\/$/.match $_
      next if not m
      if m[2].to_i == i then return "#{m[2]}: \"#{m[3]}\"" end
    end
    return "no error message available for this error number"
  rescue FileError
    return "Can't find Apple's precious copyrighted list of error messages on this system."
  ensure
    f.close if f	
  end
end

end # module GridFlow

class IO
  def nonblock= flag
    bit = Fcntl::O_NONBLOCK
    state = fcntl(Fcntl::F_GETFL, 0)
    fcntl(Fcntl::F_SETFL, (state & ~bit) |
      (if flag; bit else 0 end))
  end
end

def protect
  yield
rescue Exception => e
  STDERR.puts "#{e.class}: #{e}"
  STDERR.puts e.backtrace
end

def GridFlow.load_user_config
	require "gridflow/bridge/puredata.rb" if GridFlow.bridge_name == "puredata"
	user_config_file = ENV["HOME"] + "/.gridflow_startup"
	begin
		load user_config_file if File.exist? user_config_file
	rescue Exception => e
		GridFlow.post "#{e.class}: #{e}:\n" + e.backtrace.join("\n")
		GridFlow.post "while loading ~/.gridflow_startup"
	end
end

require "gridflow/base/flow_objects.rb"
require "gridflow/format/main.rb"

%w(
  # #for #finished #type #dim #transpose #perspective #store #outer
  #grade #redim #import #export #export_list #cast
  #scale_by #downscale_by #draw_polygon #draw_image #layer
  #print #pack #export_symbol #rotate
  #in #out
).each {|k|
	GridFlow::FObject.name_lookup(k).add_creator k.gsub(/#/,"@")
}

END {
	GridFlow.fobjects.each {|k,v| k.delete if k.respond_to? :delete }
	GridFlow.fobjects.clear
	GC.start
}

