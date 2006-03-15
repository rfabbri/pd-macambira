=begin
	$Id: usb.rb,v 1.1 2006-03-15 04:46:22 matju Exp $

	GridFlow
	Copyright (c) 2001,2002,2003,2004,2005 by Mathieu Bouchard

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

class<<USB
	attr_reader :busses
end

FObject.subclass("delcomusb",1,1) {
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
}

FObject.subclass("klippeltronics",1,1) {
	def self.find
	  r=[]
	  USB.busses.each {|dir,bus|
	    bus.each {|dev|
	      GridFlow.post "dir=%s, vendor=%x, product=%x",
		      dir, dev.idVendor, dev.idProduct
	      r<<dev if dev.idVendor==0xDead and dev.idProduct==0xBEEF
	    }
	  }
	  r
	end
	def initialize
		r=self.class.find
		post "%s", r.inspect
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

end # module GridFlow
