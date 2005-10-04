=begin
	$Id: main.rb,v 1.1 2005-10-04 02:02:15 matju Exp $

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

require "socket"
require "fcntl"

module GridFlow

class<<self
	def max_rank; 16; end
	def max_size; 64*1024**2; end
	def max_packet; 1024*2; end
end

ENDIAN_BIG,ENDIAN_LITTLE,ENDIAN_SAME,ENDIAN_DIFF = 0,1,2,3

OurByteOrder = case [1].pack("L")
        when "\0\0\0\1"; ENDIAN_BIG     # Mac, Sun, SiliconGraphics
        when "\1\0\0\0"; ENDIAN_LITTLE  # Intel
        else raise "Cannot determine byte order" end

class Format < GridObject
	FF_R,FF_W = 4,2 # flags indicating support of :in and :out respectively.
	attr_accessor :parent
=begin API (version 0.8)
	mode is :in or :out
	def initialize(mode,*args) :
		open a file handler (do it via .new of class)
	attr_reader :description :
		a _literal_ (constant) string describing the format handler
	def self.info() optional :
		return a string describing the format handler differently
		than self.description(). in particular, it can list
		compile-time options and similar things. for example,
		quicktime returns a list of codecs.
	def frame() :
		read one frame, send through outlet 0
		return values :
			Integer >= 0 : frame number of frame read.
			false : no frame was read : end of sequence.
			nil : a frame was read, but can't say its number.
		note that trying to read a nonexistent frame should no longer
		rewind automatically (@in handles that part), nor re-read the
		last frame (mpeg/quicktime used to do this)
	def seek(Integer i) :     select one frame to be read next (by number)
	def length() : ^Integer   returns number of frames (never implemented ?)
	def close() :             close a handler
	inlet 0 :
		grid : frame to write
		other : special options
	outlet 0 : grid : frame just read
	outlet 1 : everything else
=end

	def initialize(mode,*)
		super
		@cast = :int32
		@colorspace = :rgb
		@mode = mode
		@frame = 0
		@parent = nil
		@stream = nil
		flags = self.class.instance_eval{if defined?@flags then @flags else 6 end}
		# FF_W, FF_R, FF_RW
		case mode
		when  :in; flags[2]==1
		when :out; flags[1]==1
		else raise "Format opening mode is incorrect"
		end or raise \
			"Format '#{self.class.instance_eval{@symbol_name}}'"\
			" does not support mode '#{mode}'"
	end

	def close
		@stream.close if defined? @stream and @stream
	end

	def self.suffixes_are(*suffixes)
		suffixes.map{|s|s.split(/[, ]/)}.flatten.each {|suffix|
			Format.suffixes[suffix] = self
		}
	end

	class<<self
		attr_reader :symbol_name
		attr_reader :description
		attr_reader :flags
		attr_reader :suffixes
	end
	@suffixes = {}
	def seek frame
		(rewind; return) if frame == 0
		raise "don't know how to seek for frame other than # 0"
	end

	# this is what you should use to rewind
	# different file-sources may redefine this as something else
	# (eg: gzip)
	def rewind
		raise "Nothing to rewind about..." if not @stream
		@stream.seek 0,IO::SEEK_SET
		@frame = 0
	end

	# This is different from IO#eof, which waits until a read has failed
	# doesn't work in nonblocking mode? (I don't recall why)
	def eof?
		thispos = (@stream.seek 0,IO::SEEK_CUR; @stream.tell)
		lastpos = (@stream.seek 0,IO::SEEK_END; @stream.tell)
		@stream.seek thispos,IO::SEEK_SET
		return thispos == lastpos
	rescue Errno::ESPIPE # just ignore if seek is not possible
		return false
	end

	# "ideal" buffer size or something
	# the buffer may be bigger than this but usually not by much.
	def self.buffersize; 16384 end

	def _0_headerless(*args) #!@#$ goes in FormatGrid ?
		args=args[0] if Array===args[0]
		#raise "expecting dimension list..."
		args.map! {|a|
			Numeric===a or raise "expecting dimension list..."
			a.to_i
		}
		@headerless = args
	end
	def _0_headerful #!@#$ goes in FormatGrid ?
		@headerless = nil
	end
	def _0_type arg
		#!@#$ goes in FormatGrid ?
		#!@#$ bug: should not be able to modify this _during_ a transfer
		case arg
		when :uint8; @bpv=8; @bp=BitPacking.new(ENDIAN_LITTLE,1,[0xff])
		when :int16; @bpv=16; @bp=BitPacking.new(ENDIAN_LITTLE,1,[0xffff])
		when :int32; @bpv=32; @bp=nil
		else raise "unsupported number type: #{arg}"
		end
	end
	def _0_cast arg
		case arg
		when :uint8, :int16, :int32, :int64, :float32, :float64
			@cast = arg
		else raise "unsupported number type: #{arg}"
		end
	end
	def frame; @frame+=1; @frame-1 end
end

# common parts between GridIn and GridOut
module GridIO
	def check_file_open; if not @format then raise "can't do that: file not open" end end
	def _0_close; check_file_open; @format.close; @format = nil end
	def delete; @format.close if @format; @format = nil; super end
	attr_reader :format

	def _0_open(sym,*a)
		sym = sym.intern if String===sym
		if a.length==0 and /\./ =~ sym.to_s then a=[sym]; sym=:file end
		qlass = GridFlow.fclasses["\#io:#{sym}"]
		if not qlass then raise "unknown file format identifier: #{sym}" end
		_0_close if @format
		@format = qlass.new @mode, *a
		@format.connect 0,self,1
		@format.connect 1,self,2
		@format.parent = self
		@loop = true
	end

	def _0_timelog flag; @timelog = Integer(flag)!=0 end
	def _0_loop    flag;    @loop = Integer(flag)!=0 end
	def method_missing(*message)
		sel = message[0].to_s
		if sel =~ /^_0_/
			message[0] = sel.sub(/^_0_/,"").intern
			@format.send_in 0, *message
		elsif sel =~ /^_2_/
			sel = sel.sub(/^_2_/,"").intern
			message.shift
			send_out 1, sel, *message
		else
			return super
		end
	end
end

GridObject.subclass("#in",1,2) {
	install_rgrid 0
	include GridIO
	def initialize(*a)
		super
		@format = nil
		@timelog = false
		@framecount = 0
		@time = Time.new
		@mode = :in
		return if a.length==0
		_0_open(*a)
	end
	def _0_bang
		check_file_open
		framenum = @format.frame
		if framenum == false
			send_out 1
			return if not @loop
			@format.seek 0
			framenum = @format.frame
			if framenum == false
				raise "can't read frame: the end is at the beginning???"
			end
		end
		send_out 1, framenum if framenum
	end
	def _0_float frame; _0_set frame; _0_bang end
	def _0_set frame; check_file_open; @format.seek frame end
	def _0_reset; check_file_open; @format.seek 0; end
	def _1_grid(*a) send_out 0,:grid,*a end
	def _0_load name; _0_open name; _0_bang; _0_close end
}

GridObject.subclass("#out",1,1) {
	include GridIO
	def initialize(*a)
		super
		@format = nil
		@timelog = false
		@framecount = 0
		@time = Time.new
		@mode = :out
		return if a.length==0
		if Integer===a[0] or Float===a[0]
			_0_open :x11,:here
			_0_out_size a[0],a[1]
		else
			_0_open(*a)
		end
	end

	def _0_list(*a) @format._0_list(*a) end

	# hacks
	def _1_grid(*a) send_out 0,:grid,*a end # for aalib
	def _1_position(*a) send_out 0,:position,*a end
	def _1_keypress(*a) send_out 0,:keypress,*a end
	def _1_keyrelease(*a) send_out 0,:keyrelease,*a end

	def _0_grid(*a)
		check_file_open
		@format._0_grid(*a)
		send_out 0,:bang
		log if @timelog
		@framecount+=1
	end

	def log
		time = Time.new
		post("\#out: frame#%04d time: %10.3f s; diff: %5d ms",
			@framecount, time, ((time-@time)*1000).to_i)
		@time = time
	end
	install_rgrid 0
}

class BitPacking
	alias pack pack2
	alias unpack unpack2
end

# adding event-driven IO to a Format class
module EventIO
	def read_wait?; !!@action; end

	def initialize(*)
		@acceptor = nil
		@buffer = nil
		@action = nil
		@chunksize = nil
		@rewind_redefined = false
		@clock = Clock.new self
		@delay = 100 # ms
		super
	end

	def call() try_read end

	def on_read(n,&action)
		@action = action
		@chunksize = n
	end

	def try_accept
		#!@#$ use setsockopt(SO_REUSEADDR) here???
		TCPSocket.do_not_reverse_lookup = true # hack
		@acceptor.nonblock = true
		@stream = @acceptor.accept
		@stream.nonblock = true
		@stream.sync = true
		@clock.unset
#		send_out 0, :accept # does not work
	rescue Errno::EAGAIN
	end

	def try_read(dummy=nil)
		n = @chunksize-(if @buffer then @buffer.length else 0 end)
		t = @stream.read(n) # or raise EOFError
		if not t
			raise "heck" if not @stream.eof?
			rewind
			t = @stream.read(n) or raise "can't read any of #{n} bytes?"
		end
		if @buffer then @buffer << t else @buffer = t end
		if @buffer.length == @chunksize
			action,buffer = @action,@buffer
			@action,@buffer = nil,""
			@clock.unset
			action.call buffer
		end
	rescue Errno::EAGAIN
		post "read would block"
	end

	def raw_open_gzip_in(filename)
		r,w = IO.pipe
		if pid=fork
			GridFlow.subprocesses[pid]=true
			w.close
			@stream = r
		else
			r.close
			STDOUT.reopen w
			STDIN.reopen @stream
			@stream = File.open filename, "r"
			exec "gzip", "-dc"
		end
	end
	def raw_open_gzip_out(filename)
		r,w = IO.pipe
		if pid=fork
			GridFlow.subprocesses[pid]=true
			r.close
			@stream = w
		else
			w.close
			STDIN.reopen r
			STDOUT.reopen @stream
			@stream = File.open filename, "w"
			exec "gzip", "-c"
		end
	end
	def raw_open(mode,source,*args)
		@raw_open_args = mode,source,*args
		fmode = case mode
			when :in; "r"
			when :out; "w"
			else raise "bad mode" end
		close
		case source
		when :file
			filename = args[0].to_s
			filename = GridFlow.find_file filename if mode==:in
			@stream = File.open filename, fmode
		when :gzfile
			filename = args[0].to_s
			filename = GridFlow.find_file filename if mode==:in
			if mode==:in then
				raw_open_gzip_in filename
			else
				raw_open_gzip_out filename
			end
			def self.rewind
				raw_open(*@raw_open_args)
				@frame = 0
			end unless @rewind_redefined
			@rewind_redefined = true
		when :tcp
			if RUBY_VERSION < "1.6.6"
				raise "use at least 1.6.6 (reason: bug in socket code)"
			end
			post "-----------"
			time = Time.new
			TCPSocket.do_not_reverse_lookup = true # hack
			@stream = TCPSocket.open(args[0].to_s,args[1].to_i)
			post "----------- #{Time.new-time}"
			@stream.nonblock = true
			@stream.sync = true
			@clock.delay @delay
		when :tcpserver
			TCPSocket.do_not_reverse_lookup = true # hack
			TCPServer.do_not_reverse_lookup = true # hack
			post "-----------"
			time = Time.new
			@acceptor = TCPServer.open(args[0].to_s)
			post "----------- #{Time.new-time}"
			@acceptor.nonblock = true
			#$tasks[self] = proc {self.try_accept} #!!!!!
		else
			raise "unknown access method '#{source}'"
		end
	end
	def close
		@acceptor.close if @acceptor
		@stream.close if @stream
		GridFlow.hunt_zombies
	end
end

Format.subclass("#io:file",1,1) {
	def self.new(mode,file)
		file=file.to_s
		a = [mode,:file,file]
		if not /\./=~file then raise "no filename suffix?" end
		suf=file.split(/\./)[-1]
		h=Format.suffixes[suf]
		if not h then raise "unknown suffix '.#{suf}'" end
		h.new(*a)
	end
	@comment="format autodetection proxy"
}

Format.subclass("#io:grid",1,1) {
	include EventIO
	install_rgrid 0
	@comment = "GridFlow file format"
	suffixes_are "grid"
=begin
	This is the Grid format I defined:
	1 uint8: 0x7f
	4 uint8: "GRID" big endian | "grid" little endian
	1 uint8: type {
		number of bits in 8,16,32,64, plus one of: 1:unsigned 2:float
		but float8,float16 are not allowed (!)
	}
	1 uint8: reserved (supported: 0)
	1 uint8: number of dimensions N (supported: at least 0..4)
	N uint32: number of elements per dimension D[0]..D[N-1]
	raw data goes there.
=end
	# bits per value: 32 only
	attr_accessor :bpv # Fixnum: bits-per-value
	# endianness
	# attr_accessor :endian # ENDIAN_LITTLE or ENDIAN_BIG
	# IO or File or TCPSocket
	attr_reader :stream
	# nil=headerful; array=assumed dimensions of received grids
	#attr_accessor :headerless

	def initialize(mode,source,*args)
		super
		@bpv = 32
		@headerless = nil
		@endian = OurByteOrder
		raw_open mode,source,*args
	end

	def post(*s)
		# because i'm using miller_0_38 and it can't disable the console
		# i am using fprintf stderr instead of post.
		### STDERR.puts(sprintf(*s))
		# disabled because i don't need it now
	end

	# rewinding and starting
	def frame
		raise "can't get frame when there is no connection" if not @stream
		raise "already waiting for input" if read_wait?
		return false if eof?
		post "----- 1"
		if @headerless then
			@n_dim=@headerless.length
			@dim = @headerless
			@dex = 0
			set_bufsize
			send_out_grid_begin 0, @dim
			on_read(bufsize) {|data| frame3 data }
		else
			on_read(8) {|data| frame1 data }
		end
		post "----- 2"
		(try_read nil while read_wait?) if not TCPSocket===@stream
		post "----- 3"
		super
		post "----- 4"
	end

	def set_bufsize
		@prod = 1
		@dim.each {|x| @prod *= x }
		n = @prod/@dim[0]
		k = GridFlow.max_packet / n
		k=1 if k<1
		@bufsize = k*n*@bpv/8
		@bufsize = @prod if @bufsize > @prod
	end

	# the header
	def frame1 data
		post "----- frame1"
		head,@bpv,reserved,@n_dim = data.unpack "a5ccc"
		@endian = case head
			when "\x7fGRID"; ENDIAN_BIG
			when "\x7fgrid"; ENDIAN_LITTLE
			else raise "grid header: invalid (#{data.inspect})" end
		case bpv
		when 8, 16, 32; # ok
		else raise "unsupported bpv (#{@bpv})"
		end
		if reserved!=0
			raise "reserved field is not zero"
		end
		if @n_dim > GridFlow.max_rank
			raise "too many dimensions (#{@n_dim})"
		end
		on_read(4*@n_dim) {|data| frame2 data }
	end

	# the dimension list
	def frame2 data
		post "----- frame2"
		@dim = data.unpack(if @endian==ENDIAN_LITTLE then "V*" else "N*" end)
		set_bufsize
		if @prod > GridFlow.max_size
			raise "dimension list: invalid prod (#{@prod})"
		end
		send_out_grid_begin 0, @dim, @cast

		on_read(bufsize) {|data| frame3 data }
		@dex = 0
	end

	attr_reader :bufsize

	# for each slice of the body
	def frame3 data
		post "----- frame3 with dex=#{@dex.inspect}, prod=#{@prod.inspect}"
		n = data.length
		nn = n*8/@bpv
		# is/was there a problem with the size of the data being read?
		case @bpv
		when 8
			@bp = BitPacking.new(@endian,1,[0xff])
			send_out_grid_flow(0, @bp.unpack(data))
			@dex += data.length
		when 16
			@bp = BitPacking.new(@endian,2,[0xffff])
			send_out_grid_flow(0, @bp.unpack(data))
			@dex += data.length/2
		when 32
			data.swap32! if @endian!=OurByteOrder
			send_out_grid_flow 0, data
			@dex += data.length/4
		end
		if @dex >= @prod
			@clock.unset
		else
			on_read(bufsize) {|data| frame3 data }
		end
	end

	def _0_rgrid_begin
		if not @stream
			raise "can't send frame when there is no connection"
		end
		@dim = inlet_dim 0
		post "@dim=#{@dim.inspect}"
		return if @headerless
		# header
		@stream.write(
			[if @endian==ENDIAN_LITTLE then "\x7fgrid" else "\x7fGRID" end,
			 @bpv,0,@dim.length].pack("a5ccc"))
		# dimension list
		@stream.write(
			@dim.to_a.pack(if @endian==ENDIAN_LITTLE then "V*" else "N*" end))
	end

	def _0_rgrid_flow data
		case @bpv
		when 8, 16
			@stream.write @bp.pack(data)
		when 32
			data.swap32! if GridFlow::OurByteOrder != @endian
			@stream.write data
		end
	end

	def _0_rgrid_end; @stream.flush end

	def endian(a)
		@endian = case a
		when :little; ENDIAN_LITTLE
		when :big;    ENDIAN_BIG
		when :same;   ENDIAN_SAME
		else raise "argh"
		end
	end

	def headerless(*args)
		args=args[0] if Array===args[0]
		args.map! {|a|
			Numeric===a or raise "expecting dimension list..."
			a.to_i
		}
		@headerless = args
	end

	def headerful; @headerless = nil end

	#!@#$ method name conflict ?
	def type(nt)
		#!@#$ bug: should not be able to modify this _during_ a transfer
		case nt
		when :uint8; @bpv= 8; @bp=BitPacking.new(ENDIAN_LITTLE,1,[0xff])
		when :int16; @bpv=16; @bp=BitPacking.new(ENDIAN_LITTLE,1,[0xffff])
		when :int32; @bpv=32; @bp=nil
		else raise "unsupported number type"
		end
	end
}

module PPMandTarga
	# "and false" disables features that may cause crashes and don't
	# accelerate gridflow that much.
	def frame_read_body height, width, channels
		bs = width*channels
		n = bs*height
		bs = (self.class.buffersize/bs)*bs+bs # smallest multiple of bs over BufferSize
		buf = ""
		if RUBY_VERSION >= "1.8.0" and false
			data = "x"*bs # must preallocate (bug in 1.8.0.pre1-3)
			while n>0 do
				bs=n if bs>n
				@stream.read(bs,data) or raise EOFError
				if @bp then
					send_out_grid_flow 0, @bp.unpack(data,buf)
				else
					send_out_grid_flow 0, data, :uint8
				end
				n-=bs
			end
		else
			nothing = ""
			while n>0 do
				bs=n if bs>n
				data = @stream.read(bs) or raise EOFError
				if @bp then
					send_out_grid_flow 0, @bp.unpack(data,buf)
				else
					send_out_grid_flow 0, data, :uint8
				end
				data.replace nothing and false # prevent clogging memory
				n-=bs
			end
		end
	end
end

Format.subclass("#io:ppm",1,1) {
	install_rgrid 0
	@comment = "Portable PixMap (PPM) File Format"
	suffixes_are "ppm"
	include EventIO, PPMandTarga

	def initialize(mode,source,*args)
		@bp = if mode==:out
			BitPacking.new(ENDIAN_LITTLE,3,[0x0000ff,0x00ff00,0xff0000])
		else nil end
		super
		raw_open mode,source,*args
	end
	def frame
		#@stream.sync = false
		metrics=[]
		return false if eof?
		line = @stream.gets
		(rewind; line = @stream.gets) if not line # hack
		line.chomp!
		if line != "P6" then raise "Wrong format (needing PPM P6)" end
		while metrics.length<3
			line = @stream.gets
			next if line =~ /^#/
			metrics.push(*(line.split(/\s+/).map{|x| Integer x }))
		end
		metrics[2]==255 or
			raise "Wrong color depth (max_value=#{metrics[2]} instead of 255)"

		send_out_grid_begin 0, [metrics[1], metrics[0], 3], @cast
		frame_read_body metrics[1], metrics[0], 3
		super
	end

	def _0_rgrid_begin
		dim = inlet_dim 0
		raise "expecting (rows,columns,channels)" if dim.length!=3
		raise "expecting channels=3" if dim[2]!=3
		@stream.write "P6\n"
		@stream.write "# generated using GridFlow #{GF_VERSION}\n"
		@stream.write "#{dim[1]} #{dim[0]}\n255\n"
		@stream.flush
		inlet_set_factor 0, 3
	end
	def _0_rgrid_flow(data) @stream.write @bp.pack(data) end
	def _0_rgrid_end; @stream.flush end
}

Format.subclass("#io:targa",1,1) {
	install_rgrid 0
	@comment = "TrueVision Targa"
	suffixes_are "tga"
	include EventIO, PPMandTarga
=begin
targa header is like:
	[:comment, Uint8, :length],
	[:colortype, Uint8],
	[:colors,  Uint8], 5,
	[:origin_x, Int16],
	[:origin_y, Int16],
	[:w, Uint16],
	[:h, Uint16],
	[:depth, Uint8], 1,
	[:comment, String8Unpadded, :data],
=end
	def initialize(mode,source,*args)
		super
		raw_open mode,source,*args
	end

	def set_bitpacking depth
		@bp = case depth
		#!@#$ endian here doesn't seem to be changing much ?
		when 24; BitPacking.new(ENDIAN_LITTLE,3,[0xff0000,0x00ff00,0x0000ff])
		when 32; BitPacking.new(ENDIAN_LITTLE,4,
			[0x00ff0000,0x0000ff00,0x000000ff,0xff000000])
		else
			raise "tga: unsupported colour depth: #{depth}\n"
		end
	end

	def frame
		return false if eof?
		head = @stream.read(18)
		comment_length,colortype,colors,w,h,depth = head.unpack("cccx9vvcx")
		comment = @stream.read(comment_length)
		raise "unsupported color format: #{colors}" if colors != 2
#		post "tga: size y=#{h} x=#{w} depth=#{depth} colortype=#{colortype}"
#		post "tga: comment: \"#{comment}\""
		set_bitpacking depth
		send_out_grid_begin 0, [ h, w, depth/8 ], @cast
		frame_read_body h, w, depth/8
		super
	end

	def _0_rgrid_begin
		dim = inlet_dim 0
		raise "expecting (rows,columns,channels)" if dim.length!=3
		raise "expecting channels=3 or 4" if dim[2]!=3 and dim[2]!=4
		# comment = "created using GridFlow"
		#!@#$ why did i use that comment again?
		comment = "generated using GridFlow #{GF_VERSION}"
		@stream.write [comment.length,colortype=0,colors=2,"\0"*9,
		dim[1],dim[0],8*dim[2],(8*(dim[2]-3))|32,comment].pack("ccca9vvcca*")
		set_bitpacking 8*dim[2]
		inlet_set_factor 0, dim[2]
	end
	def _0_rgrid_flow data; @stream.write @bp.pack(data) end
	def _0_rgrid_end; @stream.flush end
}
end # module GridFlow
