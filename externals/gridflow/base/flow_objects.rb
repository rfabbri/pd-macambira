=begin
	$Id: flow_objects.rb,v 1.1 2005-10-04 02:02:13 matju Exp $

	GridFlow
	Copyright (c) 2001,2002,2003,2004 by Mathieu Bouchard

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

module GridFlow

#-------- fClasses for: control + misc

# a dummy class that gives access to any stuff global to GridFlow.
FObject.subclass("gridflow",1,1) {
	def _0_profiler_reset
		GridFlow.fobjects.each {|o,*| o.total_time = 0 }
		GridFlow.profiler_reset2 if GridFlow.respond_to? :profiler_reset2
	end
	def _0_profiler_dump
		ol = []
		total=0
		post "-"*32
		post "microseconds percent pointer  constructor"
		GridFlow.fobjects.each {|o,*| ol.push o }

		# HACK: BitPacking is not a real fobject
		# !@#$ is this still necessary?
		ol.delete_if {|o| not o.respond_to? :total_time }

		ol.sort! {|a,b| a.total_time <=> b.total_time }
		ol.each {|o| total += o.total_time }
		total=1 if total<1
		total_us = 0
		ol.each {|o|
			ppm = o.total_time * 1000000 / total
			us = (o.total_time*1E6/GridFlow.cpu_hertz).to_i
			total_us += us
			post "%12d %2d.%04d %08x %s", us,
				ppm/10000, ppm%10000, o.object_id, o.args
		}
		post "-"*32
		post "sum of accounted microseconds: #{total_us}"
		if GridFlow.respond_to? :memcpy_calls then
			post "memcpy calls: #{GridFlow.memcpy_calls} "+
				"; bytes: #{GridFlow.memcpy_bytes}"+
				"; time: #{GridFlow.memcpy_time}"
		end
		if GridFlow.respond_to? :malloc_calls then
			post "malloc calls: #{GridFlow.malloc_calls} "+
				"; bytes: #{GridFlow.malloc_bytes}"+
				"; time: #{GridFlow.malloc_time}"
		end
		post "-"*32
	end
	def _0_formats
		post "-"*32
		GridFlow.fclasses.each {|k,v|
			next if not /#in:/ =~ k
			modes = case v.flags
			when 2; "#out"
			when 4; "#in"
			when 6; "#in/#out"
			end
			post "%s %s: %s", modes, k, v.description
			if v.respond_to? :info then
				post "-> %s", v.info
			end
		}
		post "-"*32
	end
	# security issue if patches shouldn't be allowed to do anything they want
	def _0_eval(*l)
		s = l.map{|x|x.to_i.chr}.join""
		post "ruby: %s", s
		post "returns: %s", eval(s).inspect
	end
	add_creator "@global"
	GridFlow.bind "gridflow", "gridflow" rescue Exception
}
FObject.subclass("fps",1,1) {
	def initialize(*options)
		super
		@history = []   # list of delays between incoming messages
		@last = 0.0     # when was last time
		@duration = 0.0 # how much delay since last summary
		@period = 1     # minimum delay between summaries
		@detailed = false
		@mode = :real
		options.each {|o|
			case o
			when :detailed; @detailed=true
			when :real,:user,:system,:cpu; @mode=o
			end
		}
		def @history.moment(n=1)
			sum = 0
			each {|x| sum += x**n }
			sum/length
		end
	end
	def method_missing(*a) end # ignore non-bangs
	def _0_period x; @period=x end
	def publish
		@history.sort!
		n=@history.length
		fps = @history.length/@duration
		if not @detailed then send_out 0, fps; return end
		send_out 0, fps,
			1000*@history.min,
			500*(@history[n/2]+@history[(n-1)/2]),
			1000*@history.max,
			1000/fps,
			1000*(@history.moment(2) - @history.moment(1)**2)**0.5
	end
	def _0_bang
		t = case @mode
		when :real; Time.new.to_f
		when :user; Process.times.utime
		when :system; Process.times.stime
		when :cpu; GridFlow.rdtsc/GridFlow.cpu_hertz
		end
		@history.push t-@last
		@duration += t-@last
		@last = t
		return if @duration<@period
		fps = @history.length/@duration
		publish if fps>0.001
		@history.clear
		@duration = 0
	end
}

# to see what the messages look like when they get on the Ruby side.
FObject.subclass("rubyprint",1,0) {
	def initialize(*a)
		super
		@time = !!(a.length and a[0]==:time)
	end

	def method_missing(s,*a)
		s=s.to_s
		pre = if @time then sprintf "%10.6f  ", Time.new.to_f else "" end
		case s
		when /^_0_/; post "%s","#{pre}#{s[3..-1]}: #{a.inspect}"
		else super
		end
	end
}
FObject.subclass("printargs",0,0) {
	def initialize(*a) super; post a.inspect end
}
GridObject.subclass("#print",1,0) {
	install_rgrid 0, true
	attr_accessor :name
	def initialize(name=nil)
		super # don't forget super!!!
		if name then @name = name.to_s+": " else @name="" end
		@base=10; @format="d"; @trunc=70; @maxrows=50
	end
	def end_hook; end # other hijackability
	def format
		case @nt
		when :float32; '%6.6f'
		when :float64; '%14.14f'
		else "%#{@columns}#{@format}" end
	end
	def _0_base(x)
		@format = (case x
		when 2; "b"
		when 8; "o"
		when 10; "d"
		when 16; "x"
		else raise "base #{x} not supported" end)
		@base = x
	end
	def _0_trunc(x)
		x=x.to_f
		(0..240)===x or raise "out of range (not in 0..240 range)"
		@trunc = x
	end
	def _0_maxrows(x) @maxrows = x.to_i end
	def make_columns udata
		min = udata.min
		max = udata.max
		@columns = "" # huh?
		@columns = [
			sprintf(format,min).length,
			sprintf(format,max).length].max
	end
	def unpack data
		ps = GridFlow.packstring_for_nt @nt
		data.unpack ps
	end
	def _0_rgrid_begin
		@dim = inlet_dim 0
		@nt = inlet_nt 0
		@data = ""
	end
	def _0_rgrid_flow(data) @data << data end
	def _0_rgrid_end
		head = "#{name}Dim[#{@dim.join','}]"
		head << "(#{@nt})" if @nt!=:int32
		head << ": "
		if @dim.length > 3 then
			post head+" (not printed)"
		elsif @dim.length < 2 then
			udata = unpack @data
			make_columns udata
			post trunc(head + dump(udata))
		elsif @dim.length == 2 then
			post head
			udata = unpack @data
			make_columns udata
			sz = udata.length/@dim[0]
			rown = 1
			for row in 0...@dim[0] do
				post trunc(dump(udata[sz*row,sz]))
				rown += 1
				(post "..."; break) if rown>@maxrows
			end
		elsif @dim.length == 3 then
			post head
			make_columns unpack(@data)
			sz = @data.length/@dim[0]
			sz2 = sz/@dim[1]
			rown = 1
			for row in 0...@dim[0]
				column=0; str=""
				for col in 0...@dim[1]
					str << "(" << dump(unpack(@data[sz*row+sz2*col,sz2])) << ")"
					break if str.length>@trunc
				end
				post trunc(str)
				rown += 1
				(post "..."; break) if rown>@maxrows
			end
		end
		@data,@dim,@nt = nil
		end_hook
	end
	def dump(udata,sep=" ")
		f = format
		udata.map{|x| sprintf f,x }.join sep
	end
	def trunc s
		if s.length>@trunc then s[0...@trunc]+" [...]" else s end
	end
}
GridPack =
GridObject.subclass("#pack",1,1) {
	install_rgrid 0
	class<<self;attr_reader :ninlets;end
	def initialize(n=nil,cast=:int32)
		n||=self.class.ninlets
		n>=16 and raise "too many inlets"
		super
		@data=[0]*n
		@cast=cast
		@ps  =GridFlow.packstring_for_nt cast
	end
	def initialize2
		return if self.class.ninlets>1
		add_inlets @data.length-1
	end
	def _0_cast(cast)
		@ps   = GridFlow.packstring_for_nt cast
		@cast = cast
	end
	def self.define_inlet i
		module_eval "
			def _#{i}_int   x; @data[#{i}]=x; _0_bang; end
			def _#{i}_float x; @data[#{i}]=x; _0_bang; end
		"
	end
	(0...15).each {|x| define_inlet x }
	def _0_bang
		send_out_grid_begin 0, [@data.length], @cast
		send_out_grid_flow 0, @data.pack(@ps), @cast
	end
	self
}

# the install_rgrids in the following are hacks so that
# outlets can work. (install_rgrid is supposed to be for receiving)
# maybe GF-0.8 doesn't need that.
GridPack.subclass("@two",  2,1) { install_rgrid 0 }
GridPack.subclass("@three",3,1) { install_rgrid 0 }
GridPack.subclass("@four", 4,1) { install_rgrid 0 }
GridPack.subclass("@eight",8,1) { install_rgrid 0 }
GridObject.subclass("#unpack",1,0) {
  install_rgrid 0, true
  def initialize(n)
    @n=n
    n>=10 and raise "too many outlets"
    super
  end
  def initialize2; add_outlets @n end
  def _0_rgrid_begin
    inlet_dim(0)==[@n] or raise "expecting Dim[#{@n}], got Dim#{@dim}"
    inlet_set_factor 0,@n
  end
  def _0_rgrid_flow data
    @ps = GridFlow.packstring_for_nt inlet_nt(0)
    duh = data.unpack(@ps)
    i=duh.size-1
    until i<0 do send_out i,duh[i]; i-=1 end
  end
  def _0_rgrid_end; end
}

GridObject.subclass("#export_symbol",1,1) {
	install_rgrid 0
	def _0_rgrid_begin; @data="" end
	def _0_rgrid_flow data; @data << data; end
	def _0_rgrid_end
		send_out 0, :symbol, @data.unpack("I*").pack("c*").intern
	end
}
GridObject.subclass("unix_time",1,3) {
  install_rgrid 0
  def _0_bang
    t = Time.new
    tt = t.to_s
    send_out_grid_begin 0, [tt.length], :uint8
    send_out_grid_flow 0, tt, :uint8
    send_out 1, t.to_i
    send_out 2, t.to_f-t.to_f.floor
  end
}
### test with "shell xlogo &" -> [exec]
FObject.subclass("exec",1,0) {
        def _0_shell(*a) system(a.map!{|x| x.to_s }.join(" ")) end
}
FObject.subclass("renamefile",1,0) {
        def initialize; end
        def _0_list(a,b) File.rename(a.to_s,b.to_s) end
}
FObject.subclass("ls",1,1) {
        def _0_symbol(s) send_out 0, :list, *Dir.new(s.to_s).map {|x| x.intern } end
}

#-------- fClasses for: math

FPatcher.subclass("gfmessagebox",1,1) {
  def initialize(*a) @a=a end
  def _0_float(x)  send_out 0, *@a.map {|y| if y=="$1".intern then x else y end } end
  def _0_symbol(x) send_out 0, *@a.map {|y| if y=="$1".intern then x else y end } end
}

FPatcher.subclass("@!",1,1) {
  @fobjects = ["# +","#type","gfmessagebox list $1 #"]
  @wires = [-1,0,1,0, 1,0,2,0, 2,0,0,1, -1,0,0,0, 0,0,-1,0]
  def initialize(sym)
	super
	@fobjects[0].send_in 0, case sym
		when :rand; "op rand"; when :sqrt; "op sqrt"
		when :abs;  "op abs-"; when :sq;   "op sq-"
		else raise "bork BORK bork" end
  end
}
FPatcher.subclass("@fold",2,1) {
  @fobjects = ["#fold +","gfmessagebox seed $1"]
  @wires = [-1,0,0,0, -1,1,1,0, 1,0,0,1, 0,0,-1,0]
  def initialize(op,seed=0) super; o=@fobjects[0]
	o.send_in 0, :op, op; o.send_in 0, :seed, seed end
}
FPatcher.subclass("@scan",2,1) {
  @fobjects = ["#scan +","gfmessagebox seed $1"]
  @wires = [-1,0,0,0, -1,1,1,0, 1,0,0,1, 0,0,-1,0]
  def initialize(op,seed=0) super; o=@fobjects[0]
	o.send_in 0, :op, op; o.send_in 0, :seed, seed end
}
FPatcher.subclass("@inner",3,1) {
  @fobjects = ["#inner","gfmessagebox seed $1"]
  @wires = [-1,0,0,0, -1,1,1,0, 1,0,0,0, 0,0,-1,0, -1,2,0,1]
  def initialize(op=:*,fold=:+,seed=0,r=0) super; o=@fobjects[0]
	o.send_in 0, :op, op; o.send_in 0, :fold, fold
	o.send_in 0, :seed, seed; o.send_in 1, r end
}
FPatcher.subclass("@convolve",2,1) {
  @fobjects = ["#convolve"]
  @wires = [-1,0,0,0, -1,2,0,1, 0,0,-1,0]
  def initialize(op=:*,fold=:+,seed=0,r=0) super; o=@fobjects[0]
	o.send_in 0, :op, op; o.send_in 0, :fold, fold
	o.send_in 0, :seed, seed; o.send_in 1, r end
}

#-------- fClasses for: video

FPatcher.subclass("@scale_to",2,1) {
	@fobjects = [
		"@for {0 0} {42 42} {1 1}","@ *","@ /",
		"@store","#dim","@redim {2}","#finished",
	]
	@wires = []
	for i in 1..3 do @wires.concat [i-1,0,i,0] end
	@wires.concat [3,0,-1,0, 4,0,5,0, 5,0,1,1, 6,0,0,0,
		-1,0,4,0, -1,0,3,1, -1,0,6,0, -1,1,0,1, -1,1,2,1]
	def initialize(size)
		(size.length==2 and Numeric===size[0] and Numeric===size[1]) or
			raise "expecting {height width}"
		super
		send_in 1, size
	end
}

#<vektor> told me to:
# RGBtoYUV : @fobjects = ["#inner ( 3 3 # 66 -38 112 128 -74 -94 25 112 -18 )",
#	"@ >> 8","@ + {16 128 128}"]
# YUVtoRGB : @fobjects = ["@ - ( 16 128 128 )",
#	"#inner ( 3 3 # 298 298 298 0 -100 516 409 -208 0 )","@ >> 8"]

FPatcher.subclass("#rotate",2,1) {
	@fobjects = ["@inner * + 0","@ >> 8"]
	@wires = [-1,0,0,0, 0,0,1,0, 1,0,-1,0]
	def update_rotator
		rotator = (0...@axis[2]).map {|i|
			(0...@axis[2]).map {|j|
				if i==j then 256 else 0 end
			}
		}
		th = @angle * Math::PI / 18000
		scale = 1<<8
		(0...2).each {|i|
			(0...2).each {|j|
				rotator[@axis[i]][@axis[j]] =
					(scale*Math.cos(th+(j-i)*Math::PI/2)).to_i
			}
		}
		@fobjects[0].send_in 2,
			@axis[2], @axis[2], "#".intern, *rotator.flatten
	end
	def _0_axis(from,to,total)
		total>=0 or raise "total-axis number incorrect"
		from>=0 and from<total or raise "from-axis number incorrect"
		to  >=0 and to  <total or raise   "to-axis number incorrect"
		@axis = [from,to,total]
		update_rotator
	end
	def initialize(rot=0,axis=[0,1,2])
		super
		@angle=0
		_0_axis(*axis)
		send_in 1, rot
	end
	def _1_int(angle) @angle = angle; update_rotator end
	alias _1_float _1_int
}

FObject.subclass("foreach",1,1) {
	def initialize() super end
	def _0_list(*a)
		a.each {|e|
			if Symbol===e then
				send_out 0,:symbol,e
			else
				send_out 0,e
			end
		}
	end
}
FObject.subclass("listflatten",1,1) {
	def initialize() super end
	def _0_list(*a) send_out 0,:list,*a.flatten end
}
FObject.subclass("rubysprintf",2,1) {
  def initialize(*format) _1_list(format) end
  def _0_list(*a) send_out 0, :symbol, (sprintf @format, *a).intern end
  alias _0_float _0_list
  alias _0_symbol _0_list
  def _1_list(*format) @format = format.join(" ") end
  alias _1_symbol _1_list
}

#-------- fClasses for: jMax compatibility

class JMaxUDPSend < FObject
	def initialize(host,port)
		super
		@socket = UDPSocket.new
		@host,@port = host.to_s,port.to_i
	end
	def encode(x)
		case x
		when Integer; "\x03" + [x].pack("N")
		when Float; "\x04" + [x].pack("g")
		when Symbol, String; "\x01" + x.to_s + "\x02"
		end
	end
	def method_missing(sel,*args)
		sel=sel.to_s.sub(/^_\d_/, "")
		@socket.send encode(sel) +
			args.map{|arg| encode(arg) }.join("") + "\x0b",
			0, @host, @port
	end
	def delete; @socket.close end
	install "jmax_udpsend", 1, 0
end

class JMaxUDPReceive < FObject
	def initialize(port)
		super
		@socket = UDPSocket.new
		@port = port.to_i
		@socket.bind nil, @port
		@clock = Clock.new self
		@clock.delay 0
	end
	def decode s
		n = s.length
		i=0
		m = []
		case s[i]
		when 3; i+=5; m << s[i-4,4].unpack("N")[0]
		when 4; i+=5; m << s[i-4,4].unpack("g")[0]
		when 1; i2=s.index("\x02",i); m << s[i+1..i2-1].intern; i=i2+1
		when 11; break
		else raise "unknown code in udp packet"
		end while i<n
		m
	end
	def call
		ready_to_read = IO.select [@socket],[],[],0
		return if not ready_to_read
		data,sender = @socket.recvfrom 1024
		return if not data
		send_out 1, sender.map {|x| x=x.intern if String===x; x }
		send_out 0, *(decode data)
		@clock.delay 50
	end
	def delete; @clock.unset; @socket.close end
	install "jmax_udpreceive", 0, 2
end

class JMax4UDPSend < FObject
	def initialize(host,port)
		super
		@socket = UDPSocket.new
		@host,@port = host.to_s,port.to_i
		@symbols = {}
	end
	def encode(x)
		case x
		when Integer; "\x01" + [x].pack("N")
		when Float; "\x02" + [x].pack("G")
		when Symbol, String
			x = x.to_s
			y = x.intern
			if not @symbols[y]
				@symbols[y]=true
				"\x04" + [y].pack("N") + x + "\0"
			else
				"\x03" + [y].pack("N")
			end
		end
	end
	def method_missing(sel,*args)
		sel=sel.to_s.sub(/^_\d_/, "")
		sel=(case sel; when "int","float"; ""; else encode(sel) end)
		args=args.map{|arg| encode(arg) }.join("")
		@socket.send(sel+args+"\x0f", 0, @host, @port)
	end
	def delete; @socket.close end
	install "jmax4_udpsend", 1, 0
end

class JMax4UDPReceive < FObject
	def initialize(port)
		super
		@socket = UDPSocket.new
		@port = port.to_i
		@socket.bind nil, @port
		@clock = Clock.new self
		@clock.delay 0
		@symbols = {}
	end
	def decode s
		n = s.length
		i=0
		m = []
		case s[i]
		when 1; i+=5; m << s[i-4,4].unpack("N")[0]
		when 2; i+=9; m << s[i-8,8].unpack("G")[0]
		when 3
			i+=5; sid = s[i-4,4].unpack("N")[0]
			m << @symbols[sid]
		when 4
			i+=5; sid = s[i-4,4].unpack("N")[0]
			i2=s.index("\x00",i)
			@symbols[sid] = s[i..i2-1].intern
			m << @symbols[sid]
			i=i2+1
		when 15; break
		else post "unknown code %d in udp packet %s", s[i], s.inspect; return m
		end while i<n
		m
	end
	def call
		ready_to_read = IO.select [@socket],[],[],0
		return if not ready_to_read
		data,sender = @socket.recvfrom 1024
		return if not data
		send_out 1, sender.map {|x| x=x.intern if String===x; x }
		send_out 0, *(decode data)
		@clock.delay 50
	end
	def delete; @clock.unset; @socket.close end
	install "jmax4_udpreceive", 0, 2
end

class PDNetSocket < FObject
	def initialize(host,port,protocol=:udp,*options)
		super
		_1_connect(host,port,protocol)
		@options = {}
		options.each {|k|
			k=k.intern if String===k
			@options[k]=true
		}
	end
	def _1_connect(host,port,protocol=:udp)
		host = host.to_s
		port = port.to_i
		@host,@port,@protocol = host.to_s,port.to_i,protocol
		case protocol
		when :udp
			@socket = UDPSocket.new
			if host=="-" then
				@socket.bind nil, port
			end
		when :tcp
			if host=="-" then
				@server = TCPServer.new("localhost",port)
			else
				@socket = TCPSocket.new(host,port)
			end
			
		end
		@clock = Clock.new self
		@clock.delay 0
		@data = ""
	end
	def encode(x)
		x=x.to_i if @options[:nofloat] and Float===x
		x.to_s
	end
	def method_missing(sel,*args)
		sel=sel.to_s
		sel.sub!(/^_\d_/, "") or return super
		sel=(case sel; when "int","float"; ""; else encode(sel) end)
		msg = [sel,*args.map{|arg| encode(arg) }].join(" ")
		if @options[:nosemicolon] then msg << "\n" else msg << ";\n" end
		post "encoding as: %s", msg.inspect if @options[:debug]
		case @protocol
		when :udp; @socket.send msg, 0, @host, @port
		when :tcp; @socket.send msg, 0
		end
	end
	def delete; @clock.unset; @socket.close end
	def decode s
		post "decoding from: %s", s.inspect if @options[:debug]
		s.chomp!("\n")
		s.chomp!("\r")
		s.chomp!(";")
		a=s.split(/[\s\0]+/)
		a.shift if a[0]==""
		a.map {|x|
			case x
			when /-?\d+$/; x.to_i
			when /-?\d/; x.to_f
			else x.intern
			end
		}
	end
	def call
		ready_to_accept = IO.select [@server],[],[],0 if @server
		if ready_to_accept
			@socket.close if @socket
			@socket = @server.accept
		end
		ready_to_read = IO.select [@socket],[],[],0 if @socket
		return if not ready_to_read
		case @protocol
		when :udp
			data,sender = @socket.recvfrom 1024
			send_out 1, sender.map {|x| x=x.intern if String===x; x }
			send_out 0, *(decode data)
		when :tcp
			@data << @socket.sysread(1024)
			sender = @socket.peeraddr
			loop do
				n = /\n/ =~ @data
				break if not n
				send_out 1, sender.map {|x| x=x.intern if String===x; x }
				send_out 0, *(decode @data.slice!(0..n))
			end
		end
		@clock.delay 50
	end
	install "pd_netsocket", 2, 2
end

PDNetSocket.subclass("pd_netsend",1,0) {}
PDNetSocket.subclass("pd_netreceive",0,2) {
	def initialize(port) super("-",port) end
}

# bogus class for representing objects that have no recognized class.
FObject.subclass("broken",0,0) {
	def args; a=@args.dup; a[7,0] = " "+classname; a end
}

FObject.subclass("fork",1,2) {
  def method_missing(sel,*args)
    sel.to_s =~ /^_(\d)_(.*)$/ or super
	send_out 1,$2.intern,*args
	send_out 0,$2.intern,*args
  end
}
FObject.subclass("shunt",2,0) {
	def initialize(n=2,i=0) super; @n=n; @i=i end
	def initialize2; add_outlets @n end
	def method_missing(sel,*args)
		sel.to_s =~ /^_(\d)_(.*)$/ or super
		send_out @i,$2.intern,*args
	end
	def _1_int i; @i=i.to_i % @n end
	alias :_1_float :_1_int
	# hack: this is an alias.
	class Demux < self; install "demux", 2, 0; end
}

#-------- fClasses for: jmax2pd

	FObject.subclass("button",1,1) {
		def method_missing(*) send_out 0 end
	}
	FObject.subclass("toggle",1,1) {
		def _0_bang; @state ^= true; trigger end
		def _0_int x; @state = x!=0; trigger end
		def trigger; send_out 0, (if @state then 1 else 0 end) end
	}
	FObject.subclass("jpatcher",0,0) {
		def initialize(*a) super; @subobjects={} end
		attr_accessor :subobjects
	}
	FObject.subclass("jcomment",0,0) {}
	FObject.subclass("loadbang",0,1) { def trigger; send_out 0 end }
	FObject.subclass("messbox",1,1) {
		def _0_bang; send_out 0, *@argv end
		def clear; @argv=[]; end
		def append(*argv) @argv<<argv; end
	}

#-------- fClasses for: list manipulation (jMax-compatible)

	FObject.subclass("listmake",2,1) {
		def initialize(*a) @a=a end
		def _0_list(*a) @a=a; _0_bang end
		def _1_list(*a) @a=a end
		def _0_bang; send_out 0, :list, *@a end
	}
	FObject.subclass("listlength",1,1) {
		def initialize() super end
		def _0_list(*a) send_out 0, a.length end
	}
	FObject.subclass("listelement",2,1) {
		def initialize(i=0) super; @i=i.to_i end
		def _1_int(i) @i=i.to_i end; alias _1_float _1_int
		def _0_list(*a)
			e=a[@i]
			if Symbol===e then
				send_out 0, :symbol, e
			else
				send_out 0, e
			end
		end
	}
	FObject.subclass("listsublist",3,1) {
		def initialize(i=0,n=1) super; @i,@n=i.to_i,n.to_i end
		def _1_int(i) @i=i.to_i end; alias _1_float _1_int
		def _2_int(n) @n=n.to_i end; alias _2_float _2_int
		def _0_list(*a) send_out 0, :list, *a[@i,@n] end
	}
	FObject.subclass("listprepend",2,1) {
		def initialize(*b) super; @b=b end
		def _0_list(*a) a[0,0]=@b; send_out 0, :list, *a end
		def _1_list(*b) @b=b end
	}
	FObject.subclass("listappend",2,1) {
		def initialize(*b) super; @b=b end
		def _0_list(*a) a[a.length,0]=@b; send_out 0, :list, *a end
		def _1_list(*b) @b=b end
	}
	FObject.subclass("listreverse",1,1) {
		def initialize() super end
		def _0_list(*a) send_out 0,:list,*a.reverse end
	}
	FObject.subclass("messageprepend",2,1) {
		def initialize(*b) super; @b=b end
		def _0_(*a) a[0,0]=@b; send_out 0, *a end
		def _1_list(*b) @b=b end
		def method_missing(sym,*a)
			(m = /(_\d_)(.*)/.match sym.to_s) or return super
			_0_ m[2].intern, *a
		end
	}
	FObject.subclass("messageappend",2,1) {
		def initialize(*b) super; @b=b end
		def _0_(*a) a[a.length,0]=@b; send_out 0, *a end
		def _1_list(*b) @b=b end
		def method_missing(sym,*a)
			(m = /(_\d_)(.*)/.match sym.to_s) or return super
			_0_ m[2].intern, *a
		end
	}

# this was the original demo for the Ruby/jMax/PureData bridges
# FObjects are Ruby Objects that are exported to the PureData system.
# _0_bang means bang message on inlet 0
# FObject#send_out sends a message through an outlet
FObject.subclass("for",3,1) {
	attr_accessor :start, :stop, :step
	def cast(key,val)
		val = Integer(val) if Float===val
		raise ArgumentError, "#{key} isn't a number" unless Integer===val
	end
	def initialize(start,stop,step)
		super
		cast("start",start)
		cast("stop",stop)
		cast("step",step)
		@start,@stop,@step = start,stop,step
	end
	def _0_bang
		x = start
		if step > 0
			(send_out 0, x; x += step) while x < stop
		elsif step < 0
			(send_out 0, x; x += step) while x > stop
		end
	end
	def _0_float(x) self.start=x; _0_bang end
	alias _1_float stop=
	alias _2_float stop=
}
FObject.subclass("oneshot",2,1) {
	def initialize(state=true) @state=state!=0 end
	def method_missing(sel,*a)
		m = /^_0_(.*)$/.match(sel.to_s) or return super
		send_out 0, m[1].intern, *a if @state
		@state=false
	end
	def _1_int(state) @state=state!=0 end
	alias _1_float _1_int
	def _1_bang; @state=true end
}
FObject.subclass("inv+",2,1) {
  def initialize(b=0) @b=b end; def _1_float(b) @b=b end
  def _0_float(a) send_out 0, :float, @b-a end
}
FObject.subclass("inv*",2,1) {
  def initialize(b=0) @b=b end; def _1_float(b) @b=b end
  def _0_float(a) send_out 0, :float, @b/a end
}  
FObject.subclass("range",1,1) {
  def initialize(*a) @a=a end
  def initialize2
    add_inlets  @a.length
    add_outlets @a.length
  end
  def _0_float(x) i=0; i+=1 until @a[i]==nil or x<@a[i]; send_out i,x end
  def method_missing(sel,*a)
    m = /^(_\d+_)(.*)/.match(sel.to_s) or return super
    m[2]=="float" or return super
    @a[m[1].to_i-1] = a[0]
    post "setting a[#{m[1].to_i-1}] = #{a[0]}"
  end
}

#-------- fClasses for: GUI

module Gooey # to be included in any FObject class
	def initialize(*)
		super
		@selected=false
		@bg  = "#ffffff" # white background
		@bgb = "#000000" # black border
		@bgs = "#0000ff" # blue border when selected
		@fg  = "#000000" # black foreground
		@rsym = "#{self.class}#{self.object_id}".intern # unique id for use in Tcl
		@can = nil    # the canvas number
		@canvas = nil # the canvas string
		@y,@x = 0,0 # position on canvas
		@sy,@sx = 16,16 # size on canvas
		@font = "Courier -12"
		@vis = nil
	end
	attr_reader :canvas
	attr_reader :selected
	def canvas=(can)
		@can = can if Integer===can
		@canvas = case can
		  when String; can
		  when Integer; ".x%x.c"%(4*can)
		  else raise "huh?"
		end
	end
	def initialize2(*) GridFlow.bind self, @rsym.to_s end
	def pd_displace(can,x,y) self.canvas||=can; @x+=x; @y+=y; pd_show(can) end
	def pd_activate(can,*) self.canvas||=can end
	def quote(text) # for tcl (isn't completely right ?)
		text=text.gsub(/[\{\}]/) {|x| "\\"+x }
		"{#{text}}"
	end
	def pd_vis(can,vis)
	self.canvas||=can; @vis=vis!=0; update end
	def update; pd_show @can if @vis end
	def pd_getrect(can)
		self.canvas||=can
		@x,@y = get_position(can)
		# the extra one-pixel on each side was for #peephole only
		# not sure what to do with this
		[@x-1,@y-1,@x+@sx+1,@y+@sy+1]
	end
	def pd_click(can,x,y,shift,alt,dbl,doit) return 0 end
	def outline; if selected then @bgs else "#000000" end end
	def pd_select(can,sel)
		self.canvas||=can
		@selected=sel!=0
		GridFlow.gui %{ #{canvas} itemconfigure #{@rsym} -outline #{outline} \n }
	end
	def pd_delete(can) end
	def pd_show(can)
		self.canvas||=can
		@x,@y = get_position can if can
	end
	def highlight(color,ratio) # doesn't use self
		c = /^#(..)(..)(..)/.match(color)[1..3].map {|x| x.hex }
		c.map! {|x| [255,(x*ratio).to_i].min }
		"#%02x%02x%02x" % c
	end
end

class Display < FObject; include Gooey
	attr_accessor :text
	def initialize()
		super
		@sel = nil; @args = [] # contents of last received message
		@text = "..."
		@sy,@sx = 16,80 # default size of the widget
		@bg,@bgs,@fg = "#6774A0","#00ff80","#ffff80"
	end
	def _0_set_size(sy,sx) @sy,@sx=sy,sx end
	def atom_to_s a
		case a
		when Float; sprintf("%.5f",a).gsub(/\.?0+$/, "")
		else a.to_s
		end
	end
	def method_missing(sel,*args)
		m = /^(_\d+_)(.*)/.match(sel.to_s) or return super
		@sel,@args = m[2].intern,args
		@text = case @sel
			when nil; "..."
			when :float; atom_to_s @args[0]
			else @sel.to_s + ": " + @args.map{|a| atom_to_s a }.join(' ')
			end
		update
	end
	def pd_show(can)
		super
		return if not canvas or not @vis # can't show for now...
		GridFlow.gui %{
			set canvas #{canvas}
			$canvas delete #{@rsym}TEXT
			set y #{@y+2}
			foreach line [split #{quote @text} \\n] {
				$canvas create text #{@x+2} $y -fill #{@fg} -font #{quote @font}\
					-text $line -anchor nw -tag #{@rsym}TEXT
				set y [expr $y+14]
			}
			foreach {x1 y1 x2 y2} [$canvas bbox #{@rsym}TEXT] {}
			set sx [expr $x2-$x1+1]
			set sy [expr $y2-$y1+3]
			$canvas delete #{@rsym}
			$canvas create rectangle #{@x} #{@y} \
				[expr #{@x}+$sx] [expr #{@y}+$sy] -fill #{@bg} \
				-tags #{@rsym} -outline #{outline}
			$canvas lower #{@rsym} #{@rsym}TEXT
			pd \"#{@rsym} set_size $sy $sx;\n\";
		}
	end
	def pd_delete(can)
		if @vis
			GridFlow.gui %{ #{canvas} delete #{@rsym} #{@rsym}TEXT \n}
		end
		super
	end
	def delete; super end
	def _0_grid(*foo) # big hack!
		# hijacking a [#print]
		gp = FObject["#print"]
		@text = ""
		overlord = self
		gp.instance_eval { @overlord = overlord }
		def gp.post(fmt,*args) @overlord.text << sprintf(fmt,*args) << "\n" end
		def gp.end_hook
			@overlord.instance_eval{@text.chomp!}
			@overlord.update
		end
		#gp.send_in 0, :trunc, 70
		gp.send_in 0, :maxrows, 20
		gp.send_in 0, :grid, *foo
	end

	install "display", 1, 1
	gui_enable if GridFlow.bridge_name =~ /puredata/
end

class GridEdit < GridObject; include Gooey
	def initialize(grid)
		super
		@store = GridStore.new
		@store.connect 0,self,2
		@fin = GridFinished.new
		@fin.connect 0,self,3
		@bg,@bgs,@fg = "#609068","#0080ff","#ff80ff"
		@bghi = highlight(@bg,1.25) # "#80C891"   # highlighted @bg
		#@bghihi = highlight(@bghi,1.5) # very highlighted @bg
		@cellsy,@cellsx = 16,48
		@i,@j = nil,nil # highlighted cell dex
		send_in 0, grid
	end
	def _0_cell_size(sy,sx) @cellsy,@cellsx=sy,sx; update end
	def _0_float(*a) @store.send_in 1,:float,*a; @store.send_in 0; update end
	def _0_list (*a) @store.send_in 1, :list,*a; @store.send_in 0; update end
	def _0_grid (*a) @store.send_in 1, :grid,*a;   @fin.send_in 0, :grid,*a end
	def _3_bang; @store.send_in 0; update end
	def edit_start(i,j)
		edit_end if @i
		@i,@j=i,j
		GridFlow.gui %{
			set canvas #{canvas}
			$canvas itemconfigure #{@rsym}CELL_#{@i}_#{@j} -fill #{@bghi}
		}
	end
	def edit_end
		GridFlow.gui %{
			set canvas #{canvas}
			$canvas itemconfigure #{@rsym}CELL_#{@i}_#{@j} -fill #{@bg}
		}
		unfocus @can
	end
	def _2_rgrid_begin
		@data = []
		@dim = inlet_dim 2
		@nt = inlet_nt 2
		post "_2_rgrid_begin: dim=#{@dim.inspect} nt=#{@nt.inspect}"
		send_out_grid_begin 0, @dim, @nt
	end
	def _2_rgrid_flow data
		ps = GridFlow.packstring_for_nt @nt
		@data[@data.length,0] = data.unpack(ps)		
		post "_2_rgrid_flow: data=#{@data.inspect}"
		send_out_grid_flow 0, data
	end
	def _2_rgrid_end
		post "_2_rgrid_end"
	end
	def pd_click(can,x,y,shift,alt,dbl,doit)
		post "pd_click: %s", [can,x,y,shift,alt,dbl,doit].inspect
		return 0 if not doit!=0
		i = (y-@y-1)/@cellsy
		j = (x-@x-1)/@cellsx
		post "%d,%d", i,j
		ny = @dim[0] || 1
		nx = @dim[1] || 1
		if (0...ny)===i and (0...nx)===j then
			focus @can,x,y
			edit_start i,j
		end
		return 0
	end
	def pd_key(key)
		post "pd_key: %s", [key].inspect
		if key==0 then unfocus @can; return end
	end
	def pd_motion(dx,dy)
		post "pd_motion: %s", [dx,dy].inspect
		ny = @dim[0] || 1
		nx = @dim[1] || 1
		k = @i*nx+@j
		post "@data[#{k}]=#{@data[k]} before"
		@data[k]-=dy
		@store.send_in 1, :put_at, [@i,@j]
		@store.send_in 1, @data[k]
		@store.send_in 0
		post "@data[#{k}]=#{@data[k]} after"
		update
	end
	def pd_show(can)
		super
		return if not can
		ny = @dim[0] || 1
		nx = @dim[1] || 1
		@sy = 2+@cellsy*ny
		@sx = 2+@cellsx*nx
		g = %{
			set canvas #{canvas}
			$canvas delete #{@rsym} #{@rsym}CELL
			$canvas create rectangle #{@x} #{@y} #{@x+@sx} #{@y+@sy} \
				-fill #{@bg} -tags #{@rsym} -outline #{outline}
		}
		ny.times {|i|
		  nx.times {|j|
		    y1 = @y+1+i*@cellsy; y2 = y1+@cellsy
		    x1 = @x+1+j*@cellsx; x2 = x1+@cellsx
		    v = @data[i*nx+j]
		    g << %{
			$canvas create rectangle #{x1} #{y1} #{x2} #{y2} -fill #{@bg} \
				-tags {#{@rsym}CELL #{@rsym}CELL_#{i}_#{j}} -outline #{outline}
			$canvas create text #{x2-4} #{y1+2} -text "#{v}" -anchor ne -fill #ffffff \
				-tags {#{@rsym}CELL_#{i}_#{j}_T}
		    }
		  }
		}
		GridFlow.gui g
	end
	install "#edit", 2, 1
	install_rgrid 2, true
	gui_enable if GridFlow.bridge_name =~ /puredata/
end

class Peephole < FPatcher; include Gooey
	@fobjects = ["#dim","#export_list","#downscale_by 1 smoothly","#out","#scale_by 1",
	proc{Demux.new(2)}]
	@wires = [-1,0,0,0, 0,0,1,0, -1,0,5,0, 2,0,3,0, 4,0,3,0, 5,0,2,0, 5,1,4,0, 3,0,-1,0]
	def initialize(sy=32,sx=32,*args)
		super
		@fobjects[1].connect 0,self,2
		post "Peephole#initialize: #{sx} #{sy} #{args.inspect}"
		@scale = 1
		@down = false
		@sy,@sx = sy,sx # size of the widget
		@fy,@fx = 0,0   # size of last frame after downscale
		@bg,@bgs = "#A07467","#00ff80"
	end
	def pd_show(can)
		super
		return if not can
		if not @open
			GridFlow.gui %{
				pd \"#{@rsym} open [eval list [winfo id #{@canvas}]] 1;\n\";
			}
			@open=true
		end
		# round-trip to ensure this is done after the open
		GridFlow.gui %{
			pd \"#{@rsym} set_geometry #{@y} #{@x} #{@sy} #{@sx};\n\";
		}
		GridFlow.gui %{
			set canvas #{canvas}
			$canvas delete #{@rsym}
			$canvas create rectangle #{@x} #{@y} #{@x+@sx} #{@y+@sy} \
				-fill #{@bg} -tags #{@rsym} -outline #{outline}
		}
		set_geometry_for_real_now
	end
	def set_geometry_for_real_now
		@fy,@fx=@sy,@sx if @fy<1 or @fx<1
		@down = (@fx>@sx or @fy>@sx)
		if @down then
			@scale = [(@fy+@sy-1)/@sy,(@fx+@sx-1)/@sx].max
			@scale=1 if @scale<1 # what???
			@fobjects[2].send_in 1, @scale
			sy2 = @fy/@scale
			sx2 = @fx/@scale
		else
			@scale = [@sy/@fy,@sx/@fx].min
			@fobjects[4].send_in 1, @scale
			sy2 = @fy*@scale
			sx2 = @fx*@scale
		end
		begin
			@fobjects[5].send_in 1, (if @down then 0 else 1 end)
			x2=@y+(@sy-sy2)/2
			y2=@x+(@sx-sx2)/2
			@fobjects[3].send_in 0, :set_geometry,
				x2, y2, sy2, sx2
		rescue StandardError => e
			post "peeperr: %s", e.inspect
		end
		post "set_geometry_for_real_now (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d)",
			@x+1,@y+1,@sx,@sy,@fx,@fy,sx2,sy2,x2,y2
	end
	def _0_open(wid,use_subwindow)
		post "%s", [wid,use_subwindow].inspect
		@use_subwindow = use_subwindow==0 ? false : true
		if @use_subwindow then
			@fobjects[3].send_in 0, :open,:x11,:here,:embed_by_id,wid
		end
	end
	def _0_set_geometry(y,x,sy,sx)
		@sy,@sx = sy,sx
		@y,@x = y,x
		set_geometry_for_real_now
	end
	def _0_fall_thru(flag) # never worked ?
		post "fall_thru: #{flag}"
		@fobjects[3].send_in 0, :fall_thru, flag
	end
	# note: the numbering here is a FPatcher gimmick... -1,0 goes to _1_.
	def _1_position(y,x,b)
		s=@scale
		if @down then y*=s;x*=s else y*=s;x*=s end
		send_out 0,:position,y,x,b
	end
	def _2_list(sy,sx,chans)
		@fy,@fx = sy,sx
		set_geometry_for_real_now
	end
	def _0_paint()
		post "paint()"
		@fobjects[3].send_in 0, "draw"
	end
	def delete
		post "deleting peephole"
		GridFlow.gui %{ #{canvas} delete #{@rsym} \n}
		@fobjects[3].send_in 0, :close
		super
	end
	def method_missing(s,*a)
		#post "%s: %s", s.to_s, a.inspect
		super rescue NameError
	end

	install "#peephole", 1, 1
	gui_enable if GridFlow.bridge_name =~ /puredata/
	#GridFlow.addtomenu "#peephole" # was this IMPD-specific ?
end

#-------- fClasses for: Hardware

if const_defined? :USB

class<<USB
	attr_reader :busses
end

class DelcomUSB < GridFlow::FObject
	Vendor,Product=0x0FC5,0x1222
	def self.find
		r=[]
		USB.busses.each {|dir,bus|
			bus.each {|dev|
				r<<dev if dev.idVendor==Vendor and dev.idProduct==Product
			}
		}
		r
	end
	def initialize #(bus=nil,dev=nil)
		r=DelcomUSB.find
		raise "no such device" if r.length<1
		raise "#{r.length} such devices (which one???)" if r.length>1
		@usb=USB.new(r[0])
		if_num=nil
		r[0].config.each {|config|
			config.interface.each {|interface|
				if_num = interface.bInterfaceNumber
			}
		}
		# post "Interface # %i\n", if_num
		@usb.set_configuration 1
		@usb.claim_interface if_num
		@usb.set_altinterface 0 rescue ArgumentError
	end
	# libdelcom had this:
        # uint8 recipient, deviceModel, major, minor, dataL, dataM;
        # uint16 length; uint8[8] extension;
	def _0_send_command(major,minor,dataL,dataM,extension="\0\0\0\0\0\0\0\0")
		raise "closed" if not @usb
		raise "extension.length!=8" if extension.length!=8
		@usb.control_msg(
			0x000000c8, 0x00000012,
			minor*0x100+major,
			dataM*0x100+dataL,
			extension, 5000)
	end
	def delete; @usb.close; end
	install "delcomusb", 1, 1
end

# Klippeltronics
FObject.subclass("multio",1,1) {
	Vendor,Product=0xDEAD,0xBEEF
	def self.find
	  r=[]
	  USB.busses.each {|dir,bus|
	    bus.each {|dev|
	      post "dir=%s, vendor=%x, product=%x",
		      dir, dev.idVendor, dev.idProduct
	      r<<dev if dev.idVendor==Vendor and dev.idProduct==Product
	    }
	  }
	  r
	end
	def initialize
		r=self.class.find
		raise "no such device" if r.length<1
		raise "#{r.length} such devices (which one???)" if r.length>1
		$iobox=@usb=USB.new(r[0])
		if_num=nil
		r[0].config.each {|config|
			config.interface.each {|interface|
				#post "interface=%s", interface.to_s
				if_num = interface.bInterfaceNumber
			}
		}
		# post "Interface # %i\n", if_num
		# @usb.set_configuration 0
		@usb.claim_interface if_num
		@usb.set_altinterface 0 rescue ArgumentError
	end
	#@usb.control_msg(0b10100001,0x01,0,0,"",1000)
	#@usb.control_msg(0b10100001,0x01,0,1," ",0)
	def delete; @usb.close; end
}
end # if const_defined? :USB

# requires Ruby 1.8.0 because of bug in Ruby 1.6.x
FObject.subclass("joystick_port",0,1) {
  def initialize(port)
    raise "sorry, requires Ruby 1.8" if RUBY_VERSION<"1.8"
    @f = File.open(port.to_s,"r+")
    @status = nil
    @clock = Clock.new self
    @clock.delay 0
    @f.nonblock=true
  end
  def delete; @clock.unset; @f.close end
  def call
    loop{
      begin
        event = @f.read(8)
      rescue Errno::EAGAIN
	@clock.delay 0
        return
      end
      return if not event
      return if event.length<8
      send_out 0, *event.unpack("IsCC")
    }
  end
}

# plotter control (HPGL)
FObject.subclass("plotter_control",1,1) {
  def puts(x)
    x<<"\n"
    x.each_byte {|b| send_out 0, b }
    send_out 0
  end
  def _0_pu; puts "PU;" end
  def _0_pd; puts "PD;" end
  def _0_pa x,y; puts "PA#{x},#{y};" end
  def _0_sp c; puts "SP#{c};"; end
  def _0_ip(*v) puts "IP#{v.join','};" end
  def _0_other(command,*v) puts "#{command.to_s.upcase}#{v.join','};" end
  def _0_print(*text) puts "LB#{text.join(' ')}\003;" end
  def _0_print_from_ascii(*codes)
    _0_print codes.map{|code| code.chr }.join("")
  end
}

(begin require "linux/ParallelPort"; true; rescue LoadError; false end) and
FObject.subclass("parallel_port",1,3) {
  def initialize(port,manually=0)
    @f = File.open(port.to_s,"r+")
    @f.extend Linux::ParallelPort
    @status = nil
    @flags = nil
    @manually = manually!=0
    @clock = (if @manually then nil else Clock.new self end)
    @clock.delay 0 if @clock
  end
  def delete; @clock.unset unless @manually; @f.close end
  def _0_int(x) @f.write x.to_i.chr; @f.flush end
  alias _0_float _0_int
  def call
    flags = @f.port_flags
    send_out 2, flags if @flags != flags
    @flags = flags
    status = @f.port_status
    send_out 1, status if @status != status
    @status = status
    @clock.delay 20 if @clock
  end
  def _0_bang
    @status = @flags = nil
    call
  end
  # outlet 0 reserved (future use)
}

(begin require "linux/SoundMixer"; true; rescue LoadError; false end) and
FObject.subclass("SoundMixer",1,1) {
  # BUG? i may have the channels (left,right) backwards
  def initialize(filename)
    super
    @file = File.open filename.to_s, 0
    @file.extend Linux::SoundMixer
    $sm = self
  end
  @@vars = Linux::SoundMixer.instance_methods.grep(/=/)
  @@vars_h = {}
  @@vars.each {|attr|
    attr.chop!
    eval %{ def _0_#{attr}(x) @file.#{attr} = x[0]*256+x[1] end }
    @@vars_h[attr]=true
  }
  def _0_get(sel=nil)
    if sel then
      sels=sel.to_s
      sel=sels.intern
      raise if not @@vars_h.include? sel.to_s
      begin
        x = @file.send sel
        send_out 0, sel, "(".intern, (x>>8)&255, x&255, ")".intern
      rescue
        send_out 0, sel, "(".intern, -1, -1, ")".intern
      end
    else
      @@vars.each {|var| _0_get var }
    end
  end
}

# experimental
FObject.subclass("rubyarray",2,1) {
  def initialize() @a=[]; @i=0; end
  def _0_float i; @i=i; send_out 0, *@a[@i]; end
  def _1_list(*l) @a[@i]=l; end
  def _0_save(filename,format=nil)
    f=File.open(filename.to_s,"w")
    if format then
      @a.each {|x| f.puts(format.to_s%x) }
    else
      @a.each {|x| f.puts(x.join(",")) }
    end
    f.close
  end
  def _0_load(filename)
    f=File.open(filename.to_s,"r")
    @a.clear
    f.each {|x| @a.push x.split(",").map {|y| Float(y) rescue y.intern }}
    f.close
  end
}

end # module GridFlow
