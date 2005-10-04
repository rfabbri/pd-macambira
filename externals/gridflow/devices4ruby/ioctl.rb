# general-purpose code for performing
# less-than-trivial IOCTL operations.
# note that this is quite hackish
# but is still better than writing actual C code.

module Linux; DEVICES_VERSION = "0.1.1"; end

module IoctlClass
	def ioctl_reader(sym,cmd_in)
		module_eval %{def #{sym}
			ioctl_intp_in(#{cmd_in})
		end}
	end
	def ioctl_writer(sym,cmd_out)
		module_eval %{def #{sym}=(v)
			ioctl_intp_out(#{cmd_out},v)
			#{sym} if respond_to? :#{sym}
		end}
	end
	def ioctl_accessor(sym,cmd_in,cmd_out)
		ioctl_reader(sym,cmd_in)
		ioctl_writer(sym,cmd_out)
	end
end

module Ioctl
	# this method is not used anymore
	def int_from_4(foo)
		# if it crashes, just insert foo=foo.reverse here.
		(foo[0]+0x100*foo[1])+0x10000*(foo[2]+0x100*foo[3])
	end

# this was a big hack (from hell) that i used until I actually
# learned the other feature of ioctl().
=begin
	def ioctl_intp_out(arg1,arg2)
		tmp = arg2 + 2**32
		foo = [2*tmp.id + 16].pack("l").unpack("P4")[0]
		tmp_ptr = int_from_4(foo)
#		STDOUT.printf "tmp_ptr=%x\n", tmp_ptr
		ioctl(arg1,tmp_ptr)
	end

	def ioctl_intp_in(arg1)
		tmp = 0xdeadbeef + 2**32
		foo = [2*tmp.id + 16].pack("l").unpack("P4")[0]
		tmp_ptr = int_from_4(foo)
#		tmp_ptr = foo.unpack("l")[0]
#		STDOUT.printf "tmp_ptr=%x\n", tmp_ptr
		ioctl(arg1,tmp_ptr)
		tmp & (2**32-1)
	end
=end

	def ioctl_intp_out(arg1,arg2)
		ioctl(arg1,[arg2].pack("l"))
	end

	def ioctl_intp_in(arg1)
		ioctl(arg1,s="blah")
		return s.unpack("l")[0]
	end

end

class IO; include Ioctl; end
