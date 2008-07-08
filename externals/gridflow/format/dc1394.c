/*
	$Id: dc1394.c 3978 2008-07-04 20:18:01Z matju $

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
*/

#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#include "../gridflow.h.fcs"

/* speeds are numbered 0 to 5, worth 100<<speednum */
/* framerates are numbers 32 to 39, worth 1.875<<(frameratenum-32) */

#define MODE(x,y,palette) /* nothing for now */

static std::map<int,string> feature_names;

static void setup_modes () {
    int i=64; // format 0
    MODE(160,120,YUV444);
    MODE(320,240,YUV422);
    MODE(640,480,YUV411);
    MODE(640,480,YUV422);
    MODE(640,480,RGB);
    MODE(640,480,MONO);
    MODE(640,480,MONO16);
    i=96; // format 1
    MODE(800,600,YUV422);
    MODE(800,600,RGB);
    MODE(800,600,MONO);
    MODE(1024,768,YUV422);
    MODE(1024,768,RGB);
    MODE(1024,768,MONO);
    MODE(800,600,MONO16);
    MODE(1024,768,MONO16);
    i=128; // format 2
    MODE(1280,960,YUV422);
    MODE(1280,960,RGB);
    MODE(1280,960,MONO);
    MODE(1600,1200,YUV422);
    MODE(1600,1200,RGB);
    MODE(1600,1200,MONO);
    MODE(1280,960,MONO16);
    MODE(1600,1200,MONO16);
    i=256; // format 6
    // MODE_EXIF= 256
    i=288; // format 7
    //MODE_FORMAT7_0,
    //MODE_FORMAT7_1,
    //MODE_FORMAT7_2,
    //MODE_FORMAT7_3,
    //MODE_FORMAT7_4,
    //MODE_FORMAT7_5,
    //MODE_FORMAT7_6,
    //MODE_FORMAT7_7

// format7 color modes start at #320 and are MONO8 YUV411 YUV422 YUV444 RGB8 MONO16 RGB16 MONO16S RGB16S RAW8 RAW16
// trigger modes start at #352 and are 0 1 2 3
// image formats start at #384 and are VGA_NONCOMPRESSED SVGA_NONCOMPRESSED_1 SVGA_NONCOMPRESSED_2
//   and continue at #390 and are STILL_IMAGE FORMAT_SCALABLE_IMAGE_SIZE

#define FEATURE(foo) feature_names[i++] = #foo;

    i=416;
    FEATURE(BRIGHTNESS);
    FEATURE(EXPOSURE);
    FEATURE(SHARPNESS);
    FEATURE(WHITE_BALANCE);
    FEATURE(HUE);
    FEATURE(SATURATION);
    FEATURE(GAMMA);
    FEATURE(SHUTTER);
    FEATURE(GAIN);
    FEATURE(IRIS);
    FEATURE(FOCUS);
    FEATURE(TEMPERATURE);
    FEATURE(TRIGGER);
    FEATURE(TRIGGER_DELAY);
    FEATURE(WHITE_SHADING);
    FEATURE(FRAME_RATE);
    i+=16;/* 16 reserved features */
    FEATURE(ZOOM);
    FEATURE(PAN);
    FEATURE(TILT);
    FEATURE(OPTICAL_FILTER);
    i+=12;/* 12 reserved features */
    FEATURE(CAPTURE_SIZE);
    FEATURE(CAPTURE_QUALITY);
    i+=14;/* 14 reserved features */

    i=480; // operation modes
    //OPERATION_MODE_LEGACY
    //OPERATION_MODE_1394B

    i=512; // sensor layouts
    //RGGB
    //GBRG,
    //GRBG,
    //BGGR

    i=544; // IIDC_VERSION
#if 0
    IIDC_VERSION(1_04);
    IIDC_VERSION(1_20);
    IIDC_VERSION(PTGREY);
    IIDC_VERSION(1_30);
    IIDC_VERSION(1_31);
    IIDC_VERSION(1_32);
    IIDC_VERSION(1_33);
    IIDC_VERSION(1_34);
    IIDC_VERSION(1_35);
    IIDC_VERSION(1_36);
    IIDC_VERSION(1_37);
    IIDC_VERSION(1_38);
    IIDC_VERSION(1_39);
#endif

// Return values are SUCCESS FAILURE NO_FRAME NO_CAMERA

// Parameter flags for dc1394_setup_format7_capture()
//#define QUERY_FROM_CAMERA -1
//#define USE_MAX_AVAIL     -2
//#define USE_RECOMMENDED   -3

// The video1394 policy: blocking (wait for a frame forever) or polling (returns if no frames in buffer
// WAIT=0 POLL=1
};

typedef raw1394handle_t RH;
typedef nodeid_t NID;

#define IO(func,args...) if (func(rh,usenode,args)!=DC1394_SUCCESS) RAISE(#func " failed");

\class FormatDC1394 : Format {
	RH rh;
	int useport;
	int usenode;
	int framerate_e;
	int height;
	int width;
	dc1394_cameracapture camera;
	dc1394_feature_set features;
	std::map<int,int> feature_index;
	\constructor (t_symbol *mode) {
		bool gotone=false;
		post("DC1394: hello world");
		rh = raw1394_new_handle();
		if (!rh) RAISE("could not get a handle for /dev/raw1394 and /dev/video1394");
		int numPorts = raw1394_get_port_info(rh,0,0);
		raw1394_destroy_handle(rh);
		post("there are %d Feuerweuer ports",numPorts);
		if (mode!=gensym("in")) RAISE("sorry, read-only");
		for(int port=0; port<numPorts; port++) {
			post("trying port #%d...",port);
			RH rh = dc1394_create_handle(port);
			int numCameras=0xDEADBEEF;
			NID *nodes = dc1394_get_camera_nodes(rh,&numCameras,0);
			post("port #%d has %d cameras",port,numCameras);
			for (int i=0; i<numCameras; i++) {
				post("camera at node #%d",nodes[i]);
				if (!gotone) {gotone=true; useport=port; usenode=nodes[i];}
			}
			dc1394_destroy_handle(rh);
		}
		if (!gotone) RAISE("no cameras available");
		this->rh = dc1394_create_handle(useport);
		IO(dc1394_get_camera_feature_set,&features);
		dc1394_print_feature_set(&features);
		post("NUM_FEATURES=%d",NUM_FEATURES);
		for (int i=0; i<NUM_FEATURES; i++) {
			dc1394_feature_info &f = features.feature[i];
			int id = f.feature_id;
			string name = feature_names.find(id)==feature_names.end() ? "(unknown)" : feature_names[id];
			bool is_there = f.available;
			post("  feature %d '%s' is %s",id,name.data(),is_there?"present":"absent");
			if (!is_there) continue;
			post("    min=%u max=%u abs_min=%u abs_max=%u",f.min,f.max,f.abs_min,f.abs_max);
		}
		framerate_e = FRAMERATE_30;
		height = 480;
		width = 640;
		setup();
	}
	\decl 0 bang ();
	\attr float framerate();
	\attr unsigned brightness();
	\attr unsigned hue();
	\attr unsigned colour();
	//\attr uint16 contrast();
	//\attr uint16 whiteness();
	void setup ();
	\decl 0 get (t_symbol *s=0);
	\decl 0 size (int height, int width);
};

\def 0 get (t_symbol *s=0) {
	FObject::_0_get(argc,argv,s);
	t_atom a[2];
	if (!s) {
		SETFLOAT(a+0,camera.frame_height);
		SETFLOAT(a+1,camera.frame_width);
		outlet_anything(bself->outlets[0],gensym("size"),2,a); // abnormal (does not use nested list)
		unsigned int width,height;
		IO(dc1394_query_format7_max_image_size,MODE_FORMAT7_0,&width,&height);
		SETFLOAT(a+0,height);
		SETFLOAT(a+1,width);
		outlet_anything(bself->outlets[0],gensym("maxsize"),2,a); // abnormal (does not use nested list)
	}
}
\def 0 size (int height, int width) {
	IO(dc1394_set_format7_image_size,MODE_FORMAT7_0,width,height);
	this->height = height;
	this->width = width;
	setup();
}

\def unsigned brightness () {unsigned value;  dc1394_get_brightness(rh,usenode,&value); return value;}
\def 0        brightness    (unsigned value) {dc1394_set_brightness(rh,usenode, value);}
\def unsigned hue        () {unsigned value;  dc1394_get_hue(       rh,usenode,&value); return value;}
\def 0        hue           (unsigned value) {dc1394_set_hue(       rh,usenode, value);}
\def unsigned colour     () {unsigned value;  dc1394_get_saturation(rh,usenode,&value); return value;}
\def 0        colour        (unsigned value) {dc1394_set_saturation(rh,usenode, value);}
  
void FormatDC1394::setup () {
	//dc1394_set_format7_image_size(rh,usenode,0,width,height);
	IO(dc1394_setup_capture,0,FORMAT_VGA_NONCOMPRESSED,MODE_640x480_MONO,SPEED_400,framerate_e,&camera);
	//IO(dc1394_setup_format7_capture,0,MODE_FORMAT7_0,SPEED_400,QUERY_FROM_CAMERA,0,0,width,height,&camera);
        if (dc1394_set_trigger_mode(rh,usenode,TRIGGER_MODE_0) != DC1394_SUCCESS) RAISE("dc1394_set_trigger_mode error");
 	if (dc1394_start_iso_transmission(rh,usenode)!=DC1394_SUCCESS) RAISE("dc1394_start_iso_transmission error");
}

\def float framerate() {
	return 1.875 * (1<<(framerate_e-FRAMERATE_1_875));
}

\def 0 framerate(float framerate) {
	framerate_e = FRAMERATE_1_875;
	while (framerate>=1.875 && framerate_e <= FRAMERATE_240) {framerate/=2; framerate_e++;}
	setup();
}

static volatile int timeout=0;
static void rien (int) {timeout=1; post("timeout2");}

\def 0 bang () {
	//struct itimerval tval;
	//tval.it_interval.tv_sec = 1;
	//tval.it_interval.tv_usec = 0;
	//tval.it_value = tval.it_interval;
	//setitimer(ITIMER_REAL,&tval,0);
	//signal(SIGALRM,rien);
	if (dc1394_single_capture(rh,&camera)!=DC1394_SUCCESS) RAISE("dc1394_single_capture error");
	//setitimer(ITIMER_REAL,0,0);
	out=new GridOutlet(this,0,new Dim(height,width,1));
	//out->send(out->dim->prod(),(uint8 *)camera.capture_buffer);
	for (int i=0; i<height; i++) out->send(out->dim->prod(1),(uint8 *)camera.capture_buffer+640*i);
	//if (dc1394_stop_iso_transmission(rh,usenode)!=DC1394_SUCCESS) RAISE("dc1394_stop_iso_transmission error");
	//post("frame_height=%d",camera.frame_height);
	//post("frame_width=%d" ,camera.frame_width);
	//post("quadlets_per_frame=%d" ,camera.quadlets_per_frame);
	//post("quadlets_per_packet=%d" ,camera.quadlets_per_packet);
}

\end class FormatDC1394 {
	install_format("#io.dc1394",4,"");
	setup_modes();
}
void startup_dc1394 () {
	\startall
}
