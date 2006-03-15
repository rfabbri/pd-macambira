# $Id: test.rb,v 1.2 2006-03-15 04:37:28 matju Exp $

$:.delete_if {|x| x=='.' }
require "gridflow"

include GridFlow
GridFlow.verbose=true

$imdir = "./images"
$animdir = "./images/movies"
srand Time.new.to_i
$port = 4200+rand(100)

def pressakey; puts "press return to continue."; readline; end

FO = FObject # shortcut

class Expect < FO
	def praise(*a)
		#raise(*a)
		puts a
	end
	def expect(*v)
		@count=0
		@v=v
		@expecting=true
		yield
		@expecting=false
		praise "wrong number of messages (#{@count}), expecting #{@v.inspect}" if @count!=@v.length
	end
	def _0_list(*l)
		return if not @expecting
		praise "wrong number of messages (#{@count})" if @count==@v.length
		praise "got #{l.inspect} expecting #{@v.inspect}" if @v[@count]!=l
		@count+=1
	end
	def method_missing(s,*a)
		praise "stray message: #{s}: #{a.inspect}"
	end
	install "expect", 1, 0
end

def cast value, type
	case type
	when :b, :u8,:uint8; value & 0xff
	when :s,:i16,:int16; (value & 0x7fff) - (value & 0x8000)
	when :i,:i32,:int32; value
	when :l,:i64,:int64; value
	when :f,:f32,:float32; value.to_f
	when :d,:f64,:float64; value.to_f
	when :r,:ruby; value
	else raise "hell"
	end
end

def test_bitpacking
	#!@#$ WRITE ME
end

def test_numops
	#!@#$ WRITE ME
end

#def tnt() for nt in [:b,:s,:i,:l,:f,:d,:r] do yield end end
def _(o,s,i,d) o.connect(s,i,d) end
def chain(*a)
	(a.length-1).times {|i| a[i].connect 0,a[i+1],a }
	a[-1]
end

def test_math
for nt in [:b,:s,:i,:l,:f,:d,:r] do
	hm = "#".intern
	#GridFlow.verbose = false

	(e=FO["#export_list"])
	(x=Expect.new)
	_ e,0,x,0 

	x.expect([1,2,3,11,12,13,21,22,23]) {
		e.send_in 0, 3,3,nt,hm,1,2,3,11,12,13,21,22,23 }

	(a=FO["fork"])
	(b=FO["@ +"])
	_ a,0,b,0
	_ a,1,b,1
	_ b,0,e,0
	x.expect([4]) { a.send_in 0, 2 }

	x.expect([2,3,5,7]) { e.send_in 0,:list,nt,2,3,5,7 }
	a = FO["#fold + , seed {#{nt} # 0}"]
	_ a,0,e,0
	x.expect([cast(420000,nt)]) { a.send_in 0,"10000 #{nt} # 42" }

	a = FO["# + {#{nt} 0 10}"]
	_ a,0,e,0
	x.expect([1,12,4,18,16,42,64]) {
		a.send_in 0,:list,nt, 1,2,4,8,16,32,64 }

	a = FO["# + {#{nt} 2 3 5}"]
	b = FO["#fold + , seed {#{nt} # 0}"]
	_ a,0,b,0
	_ b,0,e,0
	x.expect([cast(45332,nt)]) { a.send_in 0, 1000,nt,hm,42 }

	a = FO["@ + {#{nt} # 42}"]
	_ a,0,e,0
	x.expect((43..169).to_a) {
		a.send_in 0,:list,nt, *(1..127).to_a }

	x.expect([3,5,9,15]) {
		a.send_in 1,:list,4,nt,hm, 2,3,5,7
		a.send_in 0,:list,4,nt,hm, 1,2,4,8 }
	x.expect([11,12,14,18]) {
		a.send_in 1, "list #{nt} # 10"
		a.send_in 0,:list,nt, 1,2,4,8 }

if nt!=:b and nt!=:f and nt!=:d
	a=FO["# / {#{nt} # 3}"  ]; _ a,0,e,0; x.expect([-2,-1,-1,-1,0,0,0,0,0,1,1,1,2]) { a.send_in(0,:list,nt, *(-6..6).to_a) }
	a=FO["# div {#{nt} # 3}"]; _ a,0,e,0; x.expect([-2,-2,-2,-1,-1,-1,0,0,0,1,1,1,2]) { a.send_in(0, :list, nt, *(-6..6).to_a) }
end

	(a = FO["# ignore {#{nt} # 42}"]).connect 0,e,0
	x.expect((42..52).to_a) { a.send_in(0, :list, nt, *(42..52).to_a) }

	(a = FO["# put {#{nt} # 42}"]).connect 0,e,0
	x.expect([42]*13) { a.send_in(0, :list, nt, *(-6..6).to_a) }

if nt!=:b
	(a = FO["# abs-"]).connect 0,e,0
	x.expect([2,3,5,7]) {
		a.send_in 0,:list,nt, -2,3,-5,7 }
end

	(a = FO["#fold *, seed {#{nt} # 1}"]).connect 0,e,0
	x.expect([210]) { a.send_in 0,:list,nt, 2,3,5,7 }
	x.expect([128]) { a.send_in 0,:list,nt, 1,1,2,1,2,2,2,1,1,2,1,2,2 }

	(a = FO["#fold +, seed {#{nt} 0 0}"]).connect 0,e,0
	x.expect([18,23]) { a.send_in 0, 3,2,nt,hm,2,3,5,7,11,13 }

	(a = FO["#scan +, seed {#{nt} 0 0}"]).connect 0,e,0
	x.expect([2,3,7,10,18,23]) { a.send_in 0, 3,2,nt,hm,2,3,5,7,11,13 }

	(a = FO["#scan *, seed {#{nt} # 1}"]).connect 0,e,0
	x.expect([2,6,30,210]) { a.send_in 0,:list,nt, 2,3,5,7 }
	x.expect([1,1,2,2,4,8,16,16,16,32,32,64,128]) {
		a.send_in 0,:list,nt, 1,1,2,1,2,2,2,1,1,2,1,2,2 }

	(a = FO["#scan +, seed {#{nt} 0 0 0}"]).connect 0,e,0
	x.expect([1,2,3,5,7,9,12,15,18]) {
		a.send_in 0,:list,3,3,nt,hm,*(1..9).to_a }

	(a = FO["#scan +, seed {#{nt} # 0}"]).connect 0,e,0
	x.expect([1,3,6, 4,9,15, 7,15,24]) {
		a.send_in 0,:list,3,3,nt,hm,*(1..9).to_a }

	(a = FO["#outer +"]).connect 0,e,0
	x.expect([9,10,12,17,18,20,33,34,36]) {
		a.send_in 1,:list,nt, 1,2,4
		a.send_in 0,:list,nt, 8,16,32 }

	x.expect((0...100).to_a) {
		a.send_in 1,(0...10).to_a
		a.send_in 0,(0...10).map{|i| 10*i }}

if nt!=:b and nt!=:f and nt!=:d
	(a = FO["#outer",:%,[nt,3,-3]]).connect 0,e,0
	x.expect([0,0,1,-2,2,-1,0,0,1,-2,2,-1,0,0]) {
		a.send_in 0,:list,nt, -30,-20,-10,0,+10,+20,+30 }

	(a = FO["#outer","swap%".intern,[nt,3,-3]]).connect 0,e,0
	x.expect([-27,-3,-17,-3,-7,-3,0,0,3,7,3,17,3,27]) {
		a.send_in 0,:list,nt, -30,-20,-10,0,+10,+20,+30 }
end

	(a = FO["#import {3}"]).connect 0,e,0; x.expect([2,3,5]) { [2,3,5].each {|v| a.send_in 0,:list,nt,hm,v }}
	(a = FO["#import {3}"]).connect 0,e,0; x.expect([2,3,5]) { [2,3,5].each {|v| a.send_in 0,:list,nt,v }}
	(a = FO["#redim {5}"]).connect 0,e,0; x.expect([2,3,5,2,3]) { a.send_in 0,:list,2,3,5 }
	(a = FO["#redim {5}"]).connect 0,e,0; x.expect([0,0,0,0,0]) { a.send_in 0,:list }
	(a = FO["#redim {0}"]).connect 0,e,0; x.expect([]) { a.send_in 0,:list,42,37,69 }

	(a = FO["#inner {2 2 #{nt} # 2 3 5 7}, seed {#{nt} # 0}"]).connect 0,e,0
	(i0 = FO["@redim {2 2}"]).connect 0,a,0
	x.expect([12,17,48,68]) { i0.send_in 0,:list,nt, 1,2,4,8 }

	(a = FO["#outer * {3 2 #{nt} # 1 2 3}"]).connect 0,e,0
	b = FO["#dim"]
	c = FO["#export_list"]
	a.connect 0,b,0
	y = Expect.new
	b.connect 0,c,0
	c.connect 0,y,0

	y.expect([2,3,2]) {
		x.expect([1,2,3,1,2,3,10,20,30,10,20,30]) {
			a.send_in 0,:list,nt, 1, 10 }}

	#pr=GridPrint.new
	(b = FO["#redim {5 5}"]).connect 0,e,0
	(a = FO["#convolve, seed {#{nt} # 0}"]).connect 0,b,0
	(i0 = FO["#redim {5 5 1}"]).connect 0,a,0
	(i1 = FO["#redim {3 1}"]).connect 0,a,1
	i1.send_in 1, 3,3
	x.expect([5,6,5,4,4,4,6,7,6,4,3,3,6,7,5,4,2,3,6,6,5,4,3,4,5]) {
		a.send_in 1,:list,3,3,nt,hm, 1,1,1,1,1,1,1,1,1
		i0.send_in 0,:list,nt, 1,1,1,0,0,0 }

	(a = FO["#convolve, seed {#{nt} # 0}"]).connect 0,e,0
	x.expect([1,3,6,4,0]) {
		a.send_in 1, 1,2,nt,hm, 1,1
		a.send_in 0, 1,5,nt,hm, 0,1,2,4,0 }

	(a = FO["#import {4}"]).connect 0,e,0
	x.expect([2,3,5,7]) {
		[2,3,5,7].each {|v| a.send_in 0,v }}
	x.expect([1,2,3],[4,5,6],[7,8,9]) {
		a.send_in 1, :list, 3
		a.send_in 0, :list, *(1..9).to_a}

	for o in ["#store"]
		(a = FO[o]).connect 0,e,0
		a.send_in 1, 5, 4, nt, hm, 1,2,3,4,5
		x.expect([1,2,3,4,4,5,1,2,2,3,4,5]) {
			a.send_in 0, 3,1, hm, 0,2,4 }
		x.expect([1,2,3,4,5]*24) { a.send_in 0, 2,3,0,hm }
		x.expect([1,2,3,4,5]*4)  { a.send_in 0, 0,hm }
		x.expect([1,2,3,4,5]*4)  { a.send_in 0 }
		x.expect([1,2,3,4]) { a.send_in 0,[0] }
		a.send_in 1,:put_at,[0,0]
		a.send_in 1,2,2,nt,hm,6,7,8,9
		x.expect([6,7,3,4, 8,9,2,3, 4,5,1,2, 3,4,5,1, 2,3,4,5]) { a.send_in 0 }
		x.expect([6,7,3,4]) { a.send_in 0,[0] }
		x.expect([8,9,2,3]) { a.send_in 0,[1] }
		a.send_in 1,:put_at,[1,1]
		a.send_in 1,2,2,nt,hm,11,13,17,19
		x.expect([6,7,3,4, 8,11,13,3, 4,17,19,2, 3,4,5,1, 2,3,4,5]) { a.send_in 0 }
	end

	b = FO["#dim"]
	c = FO["#export_list"]
	a.connect 0,b,0
	y = Expect.new
	b.connect 0,c,0
	c.connect 0,y,0

if nt!=:b and nt!=:f and nt!=:d and nt!=:l
	(a = FO["#for {#{nt} # 0} {#{nt} # 10} {#{nt} # 1}"]).connect 0,e,0
	a.connect 0,b,0
	y.expect([10]) {
		x.expect((0...10).to_a) {
			a.send_in 0 } }

	(a = FO["#for {#{nt} # 0} {#{nt} # -10} {#{nt} # 1}"]).connect 0,e,0
	a.connect 0,b,0
	y.expect([0]) { x.expect([]) { a.send_in 0 } }

	(a = FO["#for {#{nt} # 0} {#{nt} # -10} {#{nt} # -1}"]).connect 0,e,0
	a.connect 0,b,0
	y.expect([10]) { x.expect([0,-1,-2,-3,-4,-5,-6,-7,-8,-9]) { a.send_in 0 } }

	(a = FO["#for {#{nt} 0} {#{nt} 10} {#{nt} 1}"]).connect 0,e,0
	a.connect 0,b,0
	y.expect([10,1]) {
		x.expect((0...10).to_a) {
			a.send_in 0 } }

	(a = FO["#for {#{nt} 2 3} {#{nt} 5 7} {#{nt} 1 1}"]).connect 0,e,0
	a.connect 0,b,0
	y.expect([3,4,2]) {
		x.expect([2,3,2,4,2,5,2,6,3,3,3,4,3,5,3,6,4,3,4,4,4,5,4,6]) {
			a.send_in 0 } }
end

	(a = FO["@complex_sq"]).connect 0,e,0
	x.expect([8,0]) { a.send_in 0, 2, 2 }
	x.expect([0,9]) { a.send_in 0, 0, 3 }

	(a = FO["#rotate 3000 {1 2 5}"]).connect 0,e,0
	a.send_in 0, "5 5 # 1000 0 0 0 0 0" 

#if nt==:f or nt==:d
#	(a = FO["@matrix_solve"]).connect 0,e,0
#	x.expect([1,0,0,0,1,0,0,0,1]) { a.send_in 0, 3, 3, nt, hm, 1,0,0,0,1,0,0,0,1 }
#end
	GridFlow.gfpost "ending test for #{nt}"
end # for nt

	(a = FO["#pack 2"]).connect 0,e,0
	x.expect([42,0]) { a.send_in 0,42 }
	x.expect([42,28]) { a.send_in 1,28 }
	x.expect([1313,28]) { a.send_in 0,1313 }

	(a = FO["#pack 3"]).connect 0,e,0
	x.expect([42,0,0]) { a.send_in 0,42 }
	x.expect([42,28,0]) { a.send_in 1,28 }
	x.expect([42,28,-1]) { a.send_in 2,-1 }

	(a = FO["#pack 4"]).connect 0,e,0
	x.expect([42,0,0,0]) { a.send_in 0,42 }
	x.expect([42,0,0,-42]) { a.send_in 3,-42 }

	(a = FO["#pack 5"]).connect 0,e,0
	x.expect([3.5,0,0,0]) { a.send_in 0,3.5 }
	x.expect([3.5,0,0,-3.5]) { a.send_in 3,-3.5 }

	e = FO["#export_list"]
	e.connect 0,x,0

	a = FO["#import per_message"]
	a.connect 0,e,0
	x.expect([1,2,3]) { a.send_in 0,1,2,3 }
	x.expect([102,111,111]) { a.send_in 0,:symbol,:foo }
	x.expect([ 70, 79, 79]) { a.send_in 0,:symbol,:FOO }

	a = FO["@join 1"]
	a.connect 0,e,0
	a.send_in 1,2,2,nt,hm,11,13,17,19
	x.expect([2,3,11,13,5,7,17,19]) { a.send_in 0,2,2,nt,hm,2,3,5,7 }

if nt!=:d
	a.send_in 1, 5,1,nt,hm,42
	y.expect([5,4]) {
		x.expect([2,3,5,42,7,11,13,42,17,19,23,42,29,31,37,42,41,43,47,42]) {
			a.send_in 0, 5,3,nt,hm,2,3,5,7,11,13,17,19,23,29,31,37,41,43,47 }}
end
	a = FO["@join 0"]
	a.connect 0,e,0
	a.send_in 1,2,2,nt,hm,11,13,17,19
	x.expect([2,3,5,7,11,13,17,19]) { a.send_in 0,2,2,nt,hm,2,3,5,7 }

	a = FO["@join 0 {2 2 2 #{nt} # 1 2 3}"]
	a.connect 0,e,0
	a.connect 0,b,0
	y.expect([2,2,2]) { x.expect([1,2,3,1,2,3,1,2]) { a.send_in 0,0,2,2,nt,hm }}

	a = FO["#ravel"]
	b = FO["#dim"]
	be = FO["#export_list"]
	bx = Expect.new
	a.connect 0,e,0
	a.connect 0,b,0
	b.connect 0,be,0
	be.connect 0,bx,0
	bx.expect([9]) {
		x.expect([1,2,3,2,4,6,3,6,9]) {
			o = FO["#outer *"]
			o.connect 0,a,0
			o.send_in 1,1,2,3
			o.send_in 0,1,2,3
		}
	}

	a = FO["#grade"]
	a.connect 0,e,0
	x.expect([0,2,4,6,8,9,7,5,3,1]) { a.send_in 0, 0,9,1,8,2,7,3,6,4,5 }
	x.expect([0,9,1,8,2,7,3,6,4,5]) { a.send_in 0, 0,2,4,6,8,9,7,5,3,1 }
	x.expect([7,6,5,4,3,2,1,0]) { a.send_in 0, 7,6,5,4,3,2,1,0 }

	a = FO["#grade"]
	b = FO["#fold +"]
	a.connect 0,b,0
	b.connect 0,e,0
	x.expect([100*99/2]) { a.send_in 0, (0...100).map { (rand*0x10000).to_i }}
	x.expect([100*99/2]) { a.send_in 0, (0...100).map { (rand*0x10).to_i }}
	x.expect([100*99/2]) { a.send_in 0, (0...100).map { 0 }}

	a = FO["#perspective"]
	a.connect 0,e,0
	c = []
	8.times {|v|
		3.times {|i|
			c << (v[i] * 1000 - 500) + (if i==2 then 2000 else 0 end)
		}
	}
	x.expect([
		-85,-85,85,-85,-85,85,85,85,
		-51,-51,51,-51,-51,51,51,51]) {
			a.send_in 0, 8,3,hm,*c }
			
# regressiontests for past bugs
	a = FO["#inner"] # that's it.
end

def test_rtmetro
	rt = FO["rtmetro 1000"]
	pr = FO["rubyprint"]
	rt.connect 0,pr,0
	GridFlow.post "trying to start the rtmetro"
	rt.send_in 0,1
	$mainloop.timers.after(10.0) {
		GridFlow.post "trying to stop the rtmetro (after 10 sec delay)"
		rt.send_in 0,0
	}
	$mainloop.loop
end

def test_print
	i = FO["#redim {3}"]
	pr = FO["#print"]
#	pr = GridFlow::RubyPrint.new
	i.connect 0,pr,0
	i.send_in 0, 85, 170, 255
	i.send_in 1, 3, 3
	i.send_in 0, 1, 0, 0, 0
	i.send_in 1, 2, 2, 2
	i.send_in 0, 2, 3, 5, 7, 11, 13, 17, 19
end

class Barf < GridObject
	def _0_rgrid_begin
		raise "barf"
	end
	install_rgrid 0
	install "barf", 1, 0
end

def test_nonsense
#	(a = FO["@! abs"]).connect 0,e,0
#	x.expect_error {
#		a.send_in 1, 42,42 }

	a = FO["#import {3}"]
	b = Barf.new
	a.connect 0,b,0
	begin
	  a.send_in 0, 1, 2, 3
	rescue StandardError
	  nil
	else
	  raise "Expected StandardError"
	end
	p b.inlet_dim(0)
end

def test_store
	a = FO["#in file #{$imdir}/teapot.png"]
	b = FO["#store"]
	c = FO["#out x11"]
	a.connect 0,b,1
	a.send_in 0,"cast uint8"
	a.send_in 0
	b.connect 0,c,0
	d = FO["#for {0 0} {256 256} {1 1}"]
	e = FO["# ^ 85"]
	d.connect 0,e,0
	e.connect 0,b,0
	f = FO["fps detailed"]
	c.connect 0,f,0
	pr = FO["rubyprint"]
	f.connect 0,pr,0
	GridFlow.verbose = false
	256.times {|t|
		e.send_in 1,t
		d.send_in 0
	}
end

# generates recursive checkerboard pattern (munchies) in bluish colours.
class Munchies < FPatcher
	@FOs = ["fork","fork","#for 0 64 1","#for 0 64 1","#for 2 5 1",
	"#outer ^","#outer *"]
	@wires = [-1,0,0,0, 0,0,1,0, 1,1,4,0, 4,0,6,1,
	1,0,3,0, 3,0,5,1, 0,0,2,0, 2,0,5,0, 5,0,6,0, 6,0,-1,0 ]
	def initialize() super end
	install "munchies",1,1
end

def test_munchies
	m=Munchies.new
	gout = FO["#out window"]
	m.connect 0,gout,0
	m.send_in 0
	$mainloop.loop
end

def test_image command
	gin = FO["#in"]
	gout = FO["#out window"]
	gin.connect 0,gout,0
#	31.times {
	3.times {
		gin.send_in 0,"open #{command}"
		gout.send_in 0,"timelog 1"
		gin.send_in 0
	}
	FO["#global"].send_in 0, "profiler_dump"
	$mainloop.loop
end

def test_ppm2
	gin = FO["#in"]
	store = FO["#store"]
	pa = FO["#convolve << + 0"]
	pb = FO["# / 9"]
	ra = FO["#redim {3 3}"]
	gout = FO["#out window"]
	gin.connect 0,store,1
	store.connect 0,pa,0
	pa.connect 0,pb,0
	pb.connect 0,gout,0
	ra.connect 0,pa,1
	ra.send_in 0,"0 0"
	gout.send_in 0,"timelog 1"
	gin.send_in 0,"open file #{$imdir}/teapot.png"
#	gin.send_in 0,"open file #{$imdir}/g001.jpg"
	gin.send_in 0
#	40.times { store.send_in 0 }
	loop { store.send_in 0 }
	v4j = FO["#global"]
	v4j.send_in 0,"profiler_dump"
#	$mainloop.loop
end

def test_foo
	foo = FO["#for {0 0} {64 64} {1 1}"]
	che = FO["#checkers"]
	sca = FO["#scale_by {5 3}"]
	out = FO["#out window"]
	foo.connect 0,che,0
	che.connect 0,sca,0
	sca.connect 0,out,0
	foo.send_in 0
	$mainloop.loop
end

def test_anim(*msgs)
	GridFlow.verbose = false
	gin = FO["#in"]
	gout1 = FO["#out window"]
	#gout1 = FO["@out quicktime file test.mov"]
	#gout1.send_in 0, :codec, :jpeg
	fps = FO["fps detailed"]
	rpr = FO["rubyprint"]
	gout1.connect 0,fps,0
	#fps.connect 0,rpr,0
=begin
	gout1 = FO["#downscale_by {3 2}"]
	gout2 = FO["#rgb_to_greyscale"]
	gout3 = FO["#out aalib X11 -height 60 -width 132"]
	gout1.connect 0,gout2,0
	gout2.connect 0,gout3,0
=end

	rpr = FO["rubyprint"]
#	gin.connect 1,rpr,0

	gin.connect 0,gout1,0
=begin
	layer=FO["@layer"]
	gin.connect 0,layer,0
	layer.connect 0,gout1,0
	check=FO["@checkers"]
	phor=FO["@for {0 0} {256 256} {1 1}"]
	phor.connect 0,check,0
	check.connect 0,layer,1
	phor.send_in 0
=end

#	scale = FO["@scale_by 3"]
#	gin.connect 0,scale,0
#	scale.connect 0,gout1,0

#	pr = FO["rubyprint time"]; gout.connect 0,pr,0
	msgs.each {|m| gin.send_in 0,m }
	gin.send_in 0, "cast uint8"
#	gout.send_in 0,"timelog 1"
	d=Time.new
	frames=2000
	frames.times {|n|
		#GridFlow.post "%d", n
		gin.send_in 0
		#gin.send_in 0, rand(1000)
	}
#	loop { gin.send_in 0 }
#	metro = FO["rtmetro 80"]
#	metro.connect 0,gin,0
#	metro.send_in 0,1
#	$mainloop.loop

	d=Time.new-d
	printf "%d frames in %.6f seconds (avg %.6f ms, %.6f fps)\n",
		frames, d, 1000*d/frames, frames/d
#	global.send_in 0,"dfgdfgdkfjgl"
	gout1.send_in 0, :close
	global = FO["@global"]
	global.send_in 0,"profiler_dump"
end

class TestTCP < FO
	attr_accessor :idle
	def initialize
		@idle = true
	end
	def _0_bang
		# GridFlow.gfpost "tick"
		# avoid recursion
		$mainloop.timers.after(0) {
			($in_client.send_in 0; @idle=false) if @idle
		}
	end
	install "tcptest",1,1
end

def test_tcp
	if fork
		# client (is receiving)
		GridFlow.post_header = "[client] "
		$in_client = in1 = FO["@in"]
		out = FO["@out x11"]
		in1.connect 0,out,0
		out.send_in 0,"timelog 1"
		out.send_in 0,"autodraw 2"
		GridFlow.post "test: waiting 1 second"
		sleep 1
		p "HI"
		#in1.send_in 0,"open grid tcp localhost #{$port}"
		in1.send_in 0,"open grid tcp 127.0.0.1 #{$port}"
		p "HI"

		test_tcp = TestTCP.new

		out.connect 0,test_tcp,0
		test_tcp.connect 0,in1,0

		GridFlow.post "entering mainloop..."
		$mainloop.loop
	else
		# server (is sending)
		GridFlow.post_header = "[server] "
		$in1_server = in1 = FO["@in"]
		$in2_server = in2 = FO["@in"]
		$out = out = FO["@out"]
		toggle = 0
		in1.connect 0,out,0
		in2.connect 0,out,0
		in1.send_in 0,"open #{$imdir}/r001.jpg"
		in2.send_in 0,"open #{$imdir}/b001.jpg"
		out.send_in 0,"open grid tcpserver #{$port}"
		out.send_in 0,"type uint8"
		test_tcp = GridFlow::FO.new
		def test_tcp._0_bang
			# GridFlow.post "tick"
			@toggle ||= 0
			# avoid recursion
			$mainloop.timers.after(0.01) {
				if $out.format.stream
					if @toggle==0; $in1_server else $in2_server end.send_in 0
					@toggle ^= 1
				end
				_0_bang
			}
		end
		out.connect 0,test_tcp,0
		test_tcp.send_in 0
		GridFlow.post "entering mainloop..."
		$mainloop.loop
	end
end

def test_layer
	
	gin = FO["@in png file ShaunaKennedy/atmosphere-bleu.png"]
#	gin1 = FO["@in file #{$imdir}/r001.jpg"]
#	gin = FO["@join 2 {240 320 1 # 128}"]
#	gin1.connect 0,gin,0

#	gfor = FO["@for {0 0} {120 160} {1 1}"]
#	gfor = FO["@for {0 0} {240 320} {1 1}"]
	gfor = FO["@for {0 0} {480 640} {1 1}"]
	gche = FO["@checkers"]

	gove = FO["@layer"]
#	gove = FO["@fold + {0 0 0 0}"]
#	gout = FO["@print"]
#	gove = FO["@inner2 * + 0 {3 4 # 1 0 0 0 0}"]
#	gout = FO["@out sdl"]
	gout = FO["@out x11"]

	gin.connect 0,gove,0
	gfor.connect 0,gche,0
	gche.connect 0,gove,1
	gove.connect 0,gout,0

	gfor.send_in 0

	fps = FO["fps detailed"]
	pr = FO["rubyprint"]
	gout.connect 0,fps,0
	fps.connect 0,pr,0

	loop{gin.send_in 0}
#	gin.send_in 0
#	gin1.send_in 0
	$mainloop.loop
end

Images = [
	"png file opensource.png",
	"#{$imdir}/ruby0216.jpg",
	"#{$imdir}/g001.jpg",
#	"#{$imdir}/teapot.tga",
	"grid gzfile #{$imdir}/foo.grid.gz",
	"grid gzfile #{$imdir}/foo2.grid.gz",
#	"videodev /dev/video0",
]

def test_formats_speed
	gin = FO["@in"]
	gout = FO["@out x11"]
	gin.connect 0,gout,0
	GridFlow.verbose=false
	t1=[]
	Images.each {|command|
		gin.send_in 0,"open #{command}"
		t0 = Time.new
		10.times {gin.send_in 0}
		t1 << (Time.new - t0)
		sleep 1
	}
	p t1
end

def test_formats
	gin = FO["@in"]
	gout = FO["@out window"]
	gs = FO["@ + {int16 # 0}"]
	gin.connect 0,gs,0
	gs.connect 0,gout,0
#	GridFlow.verbose=false
	t1=[]
	Images.each {|command|
		GridFlow.post "SENDING open %s", command
		gin.send_in 0,"open #{command}"
		gin.send_in 0,"cast int16"
		# test for load, rewind, load
		5.times {|x| gs.send_in 1, [:int16, '#'.intern, x*128]; gin.send_in 0}
		# test for filehandle leak
		#1000.times { gin.send_in 0,"open #{command}" }
		sleep 1
	}
	p t1	
end

def test_rewind
	gin = FO["@in videodev /dev/video1 noinit"]
	gin.send_in 0, "transfer read"
	gout = FO["@out ppm file /tmp/foo.ppm"]
#	gout = FO["@out x11"]
	gin.connect 0,gout,0
	loop {
		gin.send_in 0
		gout.send_in 0, "rewind"
	}
end

def test_formats_write
	# read files, store and save them, reload, compare, expect identical
	a = FO["@in"]
	b = FO["@out"]; a.connect 0,b,0
	c = FO["@ -"]; a.connect 0,c,1
	d = FO["@in"]; d.connect 0,c,0
	e = FO["@fold +"]; c.connect 0,e,0
	f = FO["@fold +"]; e.connect 0,f,0
	g = FO["@fold +"]; f.connect 0,g,0
	h = FO["@ / 15000"]; g.connect 0,h,0
	i = FO["@export_list"]; h.connect 0,i,0
	x = Expect.new; i.connect 0,x,0
	[
		["ppm file", "#{$imdir}/g001.jpg"],
		["targa file", "#{$imdir}/teapot.tga"],
		["targa file", "#{$imdir}/tux.tga"],
		["jpeg file", "#{$imdir}/ruby0216.jpg"],
		["grid gzfile", "#{$imdir}/foo.grid.gz", "endian little"],
		["grid gzfile", "#{$imdir}/foo.grid.gz", "endian big"],
	].each {|type,file,*rest|
		a.send_in 0, "open #{type} #{file}"
		b.send_in 0, "open #{type} /tmp/patate"
		rest.each {|r| b.send_in 0,r }
		a.send_in 0
		b.send_in 0, "close"
		raise "written file does not exist" if not File.exist? "/tmp/patate"
		d.send_in 0, "open #{type} /tmp/patate"
		x.expect([0]) { d.send_in 0 }
#		d.send_in 0
	}	
end

def test_mpeg_write
	a = FO["@in ppm file /opt/mex/r.ppm.cat"]
	b = FO["@out x11"]
	a.connect 0,b,0
	loop{a.send_in 0}
end

def test_headerless
	gout = FO["@out"]
	gout.send_in 0, "open grid file #{$imdir}/hello.txt"
	gout.send_in 0, "headerless"
	gout.send_in 0, "type uint8"
	gout.send_in 0, "104 101 108 108 111 32 119 111 114 108 100 33 10"
	gout.send_in 0, "close"
	gin = FO["@in"]
	pr = FO["@print"]
	gin.connect 0,pr,0
	gin.send_in 0, "open grid file #{$imdir}/hello.txt"
	gin.send_in 0, "headerless 13"
	gin.send_in 0, "type uint8"
	gin.send_in 0
end


def test_sound
#	o2 = FO["@ * 359"] # @ 439.775 Hz
#	o1 = FO["@for 0 44100 1"] # 1 sec @ 1.225 Hz ?
	o1 = FO["@for 0 4500 1"]
	o2 = FO["@ * 1600"] # @ 439.775 Hz
	o3 = FO["@ sin* 255"]
	o4 = FO["@ gamma 400"]
	o5 = FO["@ << 7"]
	out = FO["@out"]
	o1.connect 0,o2,0
	o2.connect 0,o3,0
	o3.connect 0,o4,0
	o4.connect 0,o5,0
	o5.connect 0,out,0
#	out.send_in 0,"open raw file /dev/dsp"
	out.send_in 0,"open grid file /dev/dsp"
	out.send_in 0,"type int16"
	x=0
	loop {
		o4.send_in 1, x
		o1.send_in 0
		x+=10
	}
end

include Math
def test_polygon
	o1 = FO["@for 0 5 1"]
	o2 = FO["@ * 14400"]
	o3 = FO["@outer + {0 9000}"]
	o4 = FO["@ +"]
	o5 = FO["@ cos* 112"]
	o6 = FO["@ + {120 160}"]
	poly = FO["@draw_polygon + {3 uint8 # 255}"]
if false
	out1 = FO["@cast int32"]
	out2 = FO["@solarize"]
#	out1 = FO["@downscale_by 2 smoothly"]
	out3 = FO["@out x11"]
	out1.connect 0,out2,0
	out2.connect 0,out3,0
else
	out1 = FO["@out x11"]
	fps = FO["fps detailed cpu"]
	out1.connect 0,fps,0
	pr = FO["rubyprint"]
	fps.connect 0,pr,0
end
	store = FO["@store"]; store.send_in 1, "240 320 3 uint8 # 0"
#	store2 = FO["@store"]
	store.connect 0,poly,0
	poly.connect 0,store,1
#	store2.connect 0,store,1
	o1.connect 0,o2,0
	o2.connect 0,o3,0
	o3.connect 0,o4,0
	o4.connect 0,o5,0
	o5.connect 0,o6,0
	o6.connect 0,poly,2
#	cast = FO["@cast int32"]
#	poly.connect 0,cast,0
#	cast.connect 0,out1,0
	poly.connect 0,out1,0
	x=0
	GridFlow.verbose=false
	task=proc {
		o4.send_in 1, 5000*x
		o5.send_in 1, 200+200*sin(x)
		poly.send_in 1,:list,:uint8, *(0..2).map{|i| 4+4*cos(0.2*x+i*PI*2/3) }
		o1.send_in 0
		store.send_in 0
# 		store2.send_in 0
		x+=1
		if x<1000 then $mainloop.timers.after(0.0) {task[]}
		else GridGlobal.new.send_in 0,"profiler_dump"; exit end
	}
	task[]
	$mainloop.loop
end

class FRoute2 < FO
	def initialize(selector)
		@selector = selector.to_s
	end
	def method_missing(sym,*a)
		sym=sym.to_s
		if sym =~ /^_0_(.*)/
			send_out((if $1==@selector then 0 else 1 end), $1.intern, *a)
		else super end
	end
	install "route2", 1, 2
end

def test_aalib
#	gin = FO["@in ppm file #{$imdir}/r001.jpg"]
	gin = FO["@in ppm file #{$animdir}/b.jpg.cat"]
	grey = FO["@rgb_to_greyscale"]
	cont = FO["@ << 1"]
	clip = FO["@ min 255"]
	gout = FO["@out aalib X11"]
	sto = FO["@store"]
	op = FO["aa_fill_with_text {localhost 4242}"]
	filt = FO["route2 grid"]
	gin.connect 0,grey,0
	grey.connect 0,cont,0
	cont.connect 0,clip,0
	clip.connect 0,gout,0
	gout.connect 0,filt,0
	filt.connect 0,sto,1
	sto.connect 0,op,0
	op.connect 0,gout,0
	gout.send_in 0, :autodraw, 0
	GridFlow.verbose = false
	task=proc{
		gin.send_in 0
		gout.send_in 0, :dump
		sto.send_in 0
		gout.send_in 0, :draw
		$mainloop.timers.after(0.1,&task)
	}
	task[]
	$mainloop.loop
end

def test_store2
	o = [
		FO["@for {0 0} {240 320} {1 1}"],
		FO["@ / 2"],
		FO["@ + 0"],
		FO["@ + 0"],
		FO["@store"],
		FO["@out x11"]]
	(0..4).each {|x| o[x].connect 0,o[x+1],0 }
	q = FO["@in ppm file images/r001.jpg"]
	q.connect 0,o[4],1
	q.send_in 0
	o[0].send_in 0
	$mainloop.loop
end

def test_remap
	rem = FO["@remap_image"]
	rot = FO["@rotate 4000"]
	rem.connect 1,rot,0
	rot.connect 0,rem,1
	gin = FO["@in ppm file #{$imdir}/teapot.png"]
	gout = FO["@out x11"]
	gin.connect 0,rem,0
	rem.connect 0,gout,0
	gin.send_in 0
	$mainloop.loop
end

def test_asm
	GridFlow.verbose=false
	a = FO["@in ppm file images/r001.jpg"]
	aa = FO["@cast uint8"]
	b = FO["@store"]
	d = FO["@store"]
	a.connect 0,aa,0
	aa.connect 0,b,1
	aa.connect 0,d,1
	a.send_in 0
	c = FO["@ + {uint8 # 0}"]
	t0 = Time.new; 1000.times {b.send_in 0}; t1 = Time.new-t0
	t1 *= 1
	b.connect 0,c,0
	stuff=proc{
		3.times{
			t0 = Time.new; 1000.times {b.send_in 0}; t2 = Time.new-t0
			t2 *= 1
			GridFlow.post "   %f   %f   %f", t1, t2, t2-t1
		}
	}
	puts "map:"
	stuff[]
	d.connect 0,c,1 # for zip
	d.send_in 0
	puts "zip:"
	stuff[]	
end

def test_metro
	o1 = RtMetro.new(1000,:geiger)
	o2 = RubyPrint.new(:time)
	o1.connect 0,o2,0
	o1.send_in 0,1
	$mainloop.loop
end

def test_outer
	o = FO["@outer + {0}"]
	o.send_in 0, 25, 240, 320, 3, "#".intern, 42
	g = FO["@global"]
	g.send_in 0, :profiler_dump
end

def test_jmax_to_pd filename
	require "gridflow/extra/jmax_format.rb"
	require "gridflow/extra/puredata_format.rb"
	jfr = JMaxFileReader.new(File.open(filename),FO)
	FO.broken_ok = true
	my_patcher = jfr.parse
#	my_patcher.subobjects.each {|x,| x.trigger if LoadBang===x }
#	$mainloop.loop
#	$p=my_patcher; ARGV.clear; load "/home/matju/bin/iruby"
	filename = File.basename filename
	filename[File.extname(filename)]=".pd"
	filename[0,0]="pd_examples/"
	pfw = PureDataFileWriter.new(filename)
	pfw.write_patcher my_patcher
	pfw.close
end

def test_error
	x = FO["@store"]
	x.send_in 0
end

if ARGV[0] then
	name = ARGV.shift
	send "test_#{name}", *ARGV
#	ARGV.each {|a| send "test_#{a}" }
	exit 0
end

#test_polygon
#test_math
#test_munchies
#test_image "grid file #{$imdir}/foo.grid"
#test_image "grid gzfile #{$imdir}/foo.grid.gz"
#test_print
#test_nonsense
#test_ppm2
#test_anim "open file #{$imdir}/g001.jpg"#,"loop 0"
#test_anim "open ppm file #{$animdir}/b.ppm.cat"
#test_anim "open jpeg file #{$imdir}/rgb.jpeg.cat"
#test_anim "open quicktime file BLAH"
#test_anim "open quicktime file #{$imdir}/rgb_uncompressed.mov"
#test_anim "open quicktime file #{$imdir}/test_mjpega.mov"
#test_anim "open ppm gzfile motion_tracking.ppm.cat.gz"
#test_anim "open videodev /dev/video","channel 1","size 480 640"
#test_anim "open videodev /dev/video1 noinit","transfer read"
#test_anim "open videodev /dev/video","channel 1","size 120 160"
#test_anim "open mpeg file /home/matju/net/Animations/washington_zoom_in.mpeg"
#test_anim "open quicktime file /home/matju/Shauna/part_1.mov"
#test_anim "open quicktime file #{$imdir}/gt.mov"
#test_anim "open quicktime file /home/matju/pics/domopers_hi.mov"
#test_anim "open quicktime file /home/matju/net/c.mov"
#test_formats
#test_tcp
#test_sound
#test_metro
#$mainloop.loop

=begin
a = FO["@print"]
a.send_in 0, "3 3 #{nt} # 1 0 0 0"
a.send_in 0, "3 3 3 #{nt} # 1 2 3 4"
a.send_in 0, "base 16"
a.send_in 0, "3 3 3 #{nt} # 255 0 0 0"
=end
