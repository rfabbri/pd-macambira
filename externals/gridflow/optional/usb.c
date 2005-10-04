/*
	$Id: usb.c,v 1.1 2005-10-04 02:02:15 matju Exp $

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
*/

#include <usb.h>
#include "../base/grid.h.fcs"

static Ruby cUSB;

struct named_int {const char *name; int v;};

named_int usb_class_choice[] = {
	{"USB_CLASS_PER_INTERFACE",0},
	{"USB_CLASS_AUDIO",1},
	{"USB_CLASS_COMM",2},
	{"USB_CLASS_HID",3},
	{"USB_CLASS_PRINTER",7},
	{"USB_CLASS_MASS_STORAGE",8},
	{"USB_CLASS_HUB",9},
	{"USB_CLASS_DATA",10},
	{"USB_CLASS_VENDOR_SPEC",0xff},
	{0,0},
};

named_int usb_descriptor_types_choices[] = {
	{"USB_DT_DEVICE",0x01},
	{"USB_DT_CONFIG",0x02},
	{"USB_DT_STRING",0x03},
	{"USB_DT_INTERFACE",0x04},
	{"USB_DT_ENDPOINT",0x05},
	{"USB_DT_HID",0x21},
	{"USB_DT_REPORT",0x22},
	{"USB_DT_PHYSICAL",0x23},
	{"USB_DT_HUB",0x29},
	{0,0},
};

named_int usb_descriptor_types_sizes[] = {
	{"USB_DT_DEVICE_SIZE",18},
	{"USB_DT_CONFIG_SIZE",9},
	{"USB_DT_INTERFACE_SIZE",9},
	{"USB_DT_ENDPOINT_SIZE",7},
	{"USB_DT_ENDPOINT_AUDIO_SIZE",9},/* Audio extension */
	{"USB_DT_HUB_NONVAR_SIZE",7},
	{0,0},
};

named_int usb_endpoints_choices[] = {
	{"USB_ENDPOINT_ADDRESS_MASK",0x0f},/* in bEndpointAddress */
	{"USB_ENDPOINT_DIR_MASK",0x80},
	{"USB_ENDPOINT_TYPE_MASK",0x03},/* in bmAttributes */
	{"USB_ENDPOINT_TYPE_CONTROL",0},
	{"USB_ENDPOINT_TYPE_ISOCHRONOUS",1},
	{"USB_ENDPOINT_TYPE_BULK",2},
	{"USB_ENDPOINT_TYPE_INTERRUPT",3},
	{0,0},
};

named_int usb_requests_choice[] = {
	{"USB_REQ_GET_STATUS",0x00},
	{"USB_REQ_CLEAR_FEATURE",0x01},
	{"USB_REQ_SET_FEATURE",0x03},
	{"USB_REQ_SET_ADDRESS",0x05},
	{"USB_REQ_GET_DESCRIPTOR",0x06},
	{"USB_REQ_SET_DESCRIPTOR",0x07},
	{"USB_REQ_GET_CONFIGURATION",0x08},
	{"USB_REQ_SET_CONFIGURATION",0x09},
	{"USB_REQ_GET_INTERFACE",0x0A},
	{"USB_REQ_SET_INTERFACE",0x0B},
	{"USB_REQ_SYNCH_FRAME",0x0C},
	{0,0},
};

named_int usb_type_choice[] = {
	{"USB_TYPE_STANDARD",(0x00 << 5)},
	{"USB_TYPE_CLASS",(0x01 << 5)},
	{"USB_TYPE_VENDOR",(0x02 << 5)},
	{"USB_TYPE_RESERVED",(0x03 << 5)},
	{0,0},
};

named_int usb_recipient_choice[] = {
	{"USB_RECIP_DEVICE",0x00},
	{"USB_RECIP_INTERFACE",0x01},
	{"USB_RECIP_ENDPOINT",0x02},
	{"USB_RECIP_OTHER",0x03},
	{0,0},
};

named_int usb_misc[] = {
	{"USB_MAXENDPOINTS",32},
	{"USB_MAXINTERFACES",32},
	{"USB_MAXALTSETTING",128},
	{"USB_MAXCONFIG",8},
	{"USB_ENDPOINT_IN",0x80},
	{"USB_ENDPOINT_OUT",0x00},
	{"USB_ERROR_BEGIN",500000},
	{0,0},
};

named_int* usb_all_defines[] = {
	usb_class_choice,
	usb_descriptor_types_choices,
	usb_descriptor_types_sizes,
	usb_endpoints_choices,
	usb_requests_choice,
	usb_type_choice,
	usb_recipient_choice,
	usb_misc,
};

#define COMMA ,

//14
#define USB_DEVICE_DESCRIPTOR(MANGLE,SEP) \
	MANGLE(bLength)SEP\
	MANGLE(bDescriptorType)SEP\
	MANGLE(bcdUSB)SEP\
	MANGLE(bDeviceClass)SEP\
	MANGLE(bDeviceSubClass)SEP\
	MANGLE(bDeviceProtocol)SEP\
	MANGLE(bMaxPacketSize0)SEP\
	MANGLE(idVendor)SEP\
	MANGLE(idProduct)SEP\
	MANGLE(bcdDevice)SEP\
	MANGLE(iManufacturer)SEP\
	MANGLE(iProduct)SEP\
	MANGLE(iSerialNumber)SEP\
	MANGLE(bNumConfigurations)

//8
#define USB_ENDPOINT_DESCRIPTOR(MANGLE,SEP) \
	MANGLE(bLength)SEP\
	MANGLE(bDescriptorType)SEP\
	MANGLE(bEndpointAddress)SEP\
	MANGLE(bmAttributes)SEP\
	MANGLE(wMaxPacketSize)SEP\
	MANGLE(bInterval)SEP\
	MANGLE(bRefresh)SEP\
	MANGLE(bSynchAddress)
//	  MANGLE(extras)

//9
#define USB_INTERFACE_DESCRIPTOR(MANGLE,SEP) \
	MANGLE(bLength)SEP\
	MANGLE(bDescriptorType)SEP\
	MANGLE(bInterfaceNumber)SEP\
	MANGLE(bAlternateSetting)SEP\
	MANGLE(bNumEndpoints)SEP\
	MANGLE(bInterfaceClass)SEP\
	MANGLE(bInterfaceSubClass)SEP\
	MANGLE(bInterfaceProtocol)SEP\
	MANGLE(iInterface)
//        MANGLE(endpoint)
//        MANGLE(extras)

//8
#define USB_CONFIG_DESCRIPTOR(MANGLE,SEP) \
	MANGLE(bLength)SEP\
	MANGLE(bDescriptorType)SEP\
	MANGLE(wTotalLength)SEP\
	MANGLE(bNumInterfaces)SEP\
	MANGLE(bConfigurationValue)SEP\
	MANGLE(iConfiguration)SEP\
	MANGLE(bmAttributes)SEP\
	MANGLE(MaxPower)
//	  MANGLE(interface)
//	  MANGLE(extras)

static Ruby usb_get_endpoint (struct usb_endpoint_descriptor *self) {
#define MANGLE(X) INT2NUM(self->X)
	return rb_funcall(rb_const_get(cUSB,SI(Endpoint)),SI(new),8,
		USB_ENDPOINT_DESCRIPTOR(MANGLE,COMMA));
#undef MANGLE
}

static Ruby usb_get_interface (struct usb_interface_descriptor *self) {
#define MANGLE(X) INT2NUM(self->X)
	return rb_funcall(rb_const_get(cUSB,SI(Interface)),SI(new),9+1,
		USB_INTERFACE_DESCRIPTOR(MANGLE,COMMA),
		usb_get_endpoint(self->endpoint));
#undef MANGLE
}

static Ruby usb_get_config (struct usb_config_descriptor *self) {
	if (!self) {
		fprintf(stderr,"warning: usb_get_config: null pointer\n");
		return Qnil;
	}
#define MANGLE(X) INT2NUM(self->X)
	Ruby interface = rb_ary_new();
	for (int i=0; i<self->interface->num_altsetting; i++) {
		rb_ary_push(interface, usb_get_interface(&self->interface->altsetting[i]));
	}
	return rb_funcall(rb_const_get(cUSB,SI(Config)),SI(new),8+1,
		USB_CONFIG_DESCRIPTOR(MANGLE,COMMA), interface);
#undef MANGLE
}

static Ruby usb_scan_bus (usb_bus *bus) {
	Ruby rbus = rb_ary_new();
	for (struct usb_device *dev=bus->devices; dev; dev=dev->next) {
		struct usb_device_descriptor *devd = &dev->descriptor;
		Ruby config = rb_ary_new();
		for (int i=0; i<devd->bNumConfigurations; i++) {
			rb_ary_push(config,usb_get_config(&dev->config[i]));
		}
#define MANGLE(X) INT2NUM(devd->X)
		rb_ary_push(rbus, rb_funcall(rb_const_get(cUSB,SI(Device)),SI(new),14+3,
			USB_DEVICE_DESCRIPTOR(MANGLE,COMMA),
			rb_str_new2(dev->filename),
			PTR2FIX(dev),
			config));
#undef MANGLE
	}
	return rbus;
}

\class USB < CObject
class USB : public CObject {
	usb_dev_handle *h;
public:
	\decl void initialize (Ruby dev);
	\decl int close ();
	\decl int bulk_write (int ep, String s, int timeout);
	\decl int bulk_read (int ep, String s, int timeout);
	\decl int claim_interface(int interface);
	\decl int release_interface(int interface);
	\decl int set_configuration(int configuration);
	\decl int set_altinterface(int alternate);
	\decl int control_msg(int requesttype, int request, int value, int index, String s, int timeout);
	\decl int resetep(int ep);
	\decl int clear_halt(int ep);
	\decl int reset();
	//\decl int get_string(int index, int langid, String s);
	//\decl int get_string_simple(int index, String s);
};

\def void initialize (Ruby dev) {
	Ruby ptr = rb_funcall(dev,SI(ptr),0);
	rb_ivar_set(rself, SI(@dev), ptr);
	h = usb_open(FIX2PTR(struct usb_device,ptr));
	if (!h) RAISE("usb_open returned null handle");
}

\def int close () {
	if (!h) RAISE("USB closed");
	int r = usb_close(h);
	h=0;
	return r;
}

#define TRAP(stuff) int r=(stuff); if (r<0) RAISE("%s", usb_strerror()); else return r;

\def int bulk_write (int ep, String s, int timeout) {
	if (!h) RAISE("USB closed");
	TRAP(usb_bulk_write(h, ep, rb_str_ptr(s), rb_str_len(s), timeout));}
\def int bulk_read (int ep, String s, int timeout) {
	if (!h) RAISE("USB closed");
	gfpost("%d, '%s', %d",ep,rb_str_ptr(s),timeout);
	TRAP(usb_bulk_read(h, ep, rb_str_ptr(s), rb_str_len(s), timeout));}
\def int claim_interface(int interface) {
	TRAP(usb_claim_interface(h, interface));}
\def int release_interface(int interface) {
	TRAP(usb_release_interface(h, interface));}
\def int set_configuration(int configuration) {
	TRAP(usb_set_configuration(h, configuration));}
\def int set_altinterface(int alternate) {
	TRAP(usb_set_altinterface(h, alternate));}
\def int control_msg(int requesttype, int request, int value, int index, String s, int timeout) {
	TRAP(usb_control_msg(h, requesttype, request, value, index, rb_str_ptr(s), rb_str_len(s), timeout));}
\def int resetep(int ep) {
	TRAP(usb_resetep(h, ep));}
\def int clear_halt(int ep) {
	TRAP(usb_clear_halt(h, ep));}
\def int reset() {
	TRAP(usb_reset(h));}
/*\def int get_string(int index, int langid, String s) {
	TRAP(usb_get_string(h, index, langid, rb_str_ptr(s), rb_str_len(s)));}*/
/*\def int get_string_simple(int index, String s) {
	TRAP(usb_get_string_simple(h, index, rb_str_ptr(s), rb_str_len(s)));}*/

// not handled yet:
// struct usb_string_descriptor
// struct usb_hid_descriptor
// void usb_set_debug(int level);
// Device usb_device(USB dev);
// get_string and get_string_simple (not present in libusb 0.1.4)

\classinfo
\end class USB

static Ruby USB_s_new(Ruby argc, Ruby *argv, Ruby qlass) {
	USB *self = new USB();
	Ruby rself = Data_Wrap_Struct(qlass, 0, CObject_free, self);
	self->rself = rself;
	Ruby keep = rb_ivar_get(mGridFlow, rb_intern("@fobjects"));
	rb_hash_aset(keep,rself,Qtrue); /* prevent sweeping (leak) (!@#$ WHAT??) */
	rb_funcall2(rself,SI(initialize),argc,argv);
	return rself;
}

#define SDEF(_class_,_name_,_argc_) \
	rb_define_singleton_method(c##_class_,#_name_,(RMethod)_class_##_s_##_name_,_argc_)

void startup_usb () {
	cUSB = rb_define_class_under(mGridFlow, "USB", rb_cObject);
	//rb_define_singleton_method(cUSB, "new", USB_new, 1);
	EVAL("class Symbol; def decap; x=to_s; x[0..0]=x[0..0].downcase; x.intern end end");
	SDEF(USB,new,-1);
	define_many_methods(cUSB, ciUSB.methodsn, ciUSB.methods);
#define MANGLE(X) rb_funcall(SYM(X),SI(decap),0)
	rb_const_set(cUSB, SI(Device), rb_funcall(EVAL("Struct"),SI(new),14+3,
		USB_DEVICE_DESCRIPTOR(MANGLE,COMMA), SYM(filename), SYM(ptr), SYM(config)));
	rb_const_set(cUSB, SI(Endpoint), rb_funcall(EVAL("Struct"),SI(new),8,
		USB_ENDPOINT_DESCRIPTOR(MANGLE,COMMA)));
	rb_const_set(cUSB, SI(Interface), rb_funcall(EVAL("Struct"),SI(new),9+1,
		USB_INTERFACE_DESCRIPTOR(MANGLE,COMMA), SYM(endpoint)));
	rb_const_set(cUSB, SI(Config), rb_funcall(EVAL("Struct"),SI(new),8+1,
		USB_CONFIG_DESCRIPTOR(MANGLE,COMMA), SYM(interface)));
#undef MANGLE
	for(int i=0; i<COUNT(usb_all_defines); i++) {
		named_int *ud = usb_all_defines[i];
		for(; ud->name; ud++) {
			rb_const_set(cUSB, rb_intern(ud->name), INT2NUM(ud->v));
		}
	}
	//usb_set_debug(42);
	usb_init();
	usb_find_busses();
	usb_find_devices();
	Ruby busses = rb_hash_new();
	rb_ivar_set(cUSB, SI(@busses), busses);
	for (usb_bus *bus=usb_get_busses(); bus; bus=bus->next) {
		rb_hash_aset(busses,rb_str_new2(bus->dirname),usb_scan_bus(bus));
	}
	//IEVAL(cUSB,"STDERR.print '@busses = '; STDERR.puts @busses.inspect");
}

