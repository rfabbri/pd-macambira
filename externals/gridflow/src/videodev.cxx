/*
	$Id: videodev.c 4620 2009-11-01 21:16:58Z matju $

	GridFlow
	Copyright (c) 2001-2009 by Mathieu Bouchard

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

/* bt878 on matju's comp supports only palette 4 */
/* bt878 on heri's comp supports palettes 3, 6, 7, 8, 9, 13 */
/* pwc supports palettes 12 and 15 */

#include "gridflow.hxx.fcs"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/videodev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "pwc-ioctl.h"

//#define error post
static bool debug=0;

/* **************************************************************** */

typedef video_capability VideoCapability;
typedef video_channel    VideoChannel   ;
typedef video_tuner      VideoTuner     ;
typedef video_window     VideoWindow    ;
typedef video_picture    VideoPicture   ;
typedef video_mbuf       VideoMbuf      ;
typedef video_mmap       VideoMmap      ;

#define FLAG(_num_,_name_,_desc_) #_name_,
#define  OPT(_num_,_name_,_desc_) #_name_,

/*
static const char *video_type_flags[] = {
	FLAG( 0,CAPTURE,       "Can capture")
	FLAG( 1,TUNER,         "Can tune")
	FLAG( 2,TELETEXT,      "Does teletext")
	FLAG( 3,OVERLAY,       "Overlay onto frame buffer")
	FLAG( 4,CHROMAKEY,     "Overlay by chromakey")
	FLAG( 5,CLIPPING,      "Can clip")
	FLAG( 6,FRAMERAM,      "Uses the frame buffer memory")
	FLAG( 7,SCALES,        "Scalable")
	FLAG( 8,MONOCHROME,    "Monochrome only")
	FLAG( 9,SUBCAPTURE,    "Can capture subareas of the image")
	FLAG(10,MPEG_DECODER,  "Can decode MPEG streams")
	FLAG(11,MPEG_ENCODER,  "Can encode MPEG streams")
	FLAG(12,MJPEG_DECODER, "Can decode MJPEG streams")
	FLAG(13,MJPEG_ENCODER, "Can encode MJPEG streams")
};
*/

static const char *tuner_flags[] = {
	FLAG(0,PAL,      "")
	FLAG(1,NTSC,     "")
	FLAG(2,SECAM,    "")
	FLAG(3,LOW,      "Uses KHz not MHz")
	FLAG(4,NORM,     "Tuner can set norm")
	FLAG(5,DUMMY5,   "")
	FLAG(6,DUMMY6,   "")
	FLAG(7,STEREO_ON,"Tuner is seeing stereo")
	FLAG(8,RDS_ON,   "Tuner is seeing an RDS datastream")
	FLAG(9,MBS_ON,   "Tuner is seeing an MBS datastream")
};

static const char *channel_flags[] = {
	FLAG(0,TUNER,"")
	FLAG(1,AUDIO,"")
	FLAG(2,NORM ,"")
};

static const char *video_palette_choice[] = {
	OPT( 0,NIL,     "(nil)")
	OPT( 1,GREY,    "Linear greyscale")
	OPT( 2,HI240,   "High 240 cube (BT848)")
	OPT( 3,RGB565,  "565 16 bit RGB")
	OPT( 4,RGB24,   "24bit RGB")
	OPT( 5,RGB32,   "32bit RGB")
	OPT( 6,RGB555,  "555 15bit RGB")
	OPT( 7,YUV422,  "YUV422 capture")
	OPT( 8,YUYV,    "")
	OPT( 9,UYVY,    "The great thing about standards is ...")
	OPT(10,YUV420,  "")
	OPT(11,YUV411,  "YUV411 capture")
	OPT(12,RAW,     "RAW capture (BT848)")
	OPT(13,YUV422P, "YUV 4:2:2 Planar")
	OPT(14,YUV411P, "YUV 4:1:1 Planar")
	OPT(15,YUV420P, "YUV 4:2:0 Planar")
	OPT(16,YUV410P, "YUV 4:1:0 Planar")
};

static const char *video_mode_choice[] = {
	OPT( 0,PAL,  "pal")
	OPT( 1,NTSC, "ntsc")
	OPT( 2,SECAM,"secam")
	OPT( 3,AUTO, "auto")
};

#define WH(_field_,_spec_) \
	sprintf(buf+strlen(buf), "%s=" _spec_ " ", #_field_, self->_field_);
#define WHYX(_name_,_fieldy_,_fieldx_) \
	sprintf(buf+strlen(buf), "%s=(%d %d) ", #_name_, self->_fieldy_, self->_fieldx_);
#define WHFLAGS(_field_,_table_) { \
	char *foo; \
	sprintf(buf+strlen(buf), "%s:%s ", #_field_, \
		foo=flags_to_s(self->_field_,COUNT(_table_),_table_)); \
	free(foo);}
#define WHCHOICE(_field_,_table_) { \
	char *foo; \
	sprintf(buf+strlen(buf), "%s=%s; ", #_field_, \
		foo=choice_to_s(self->_field_,COUNT(_table_),_table_));\
	free(foo);}

static char *flags_to_s(int value, int n, const char **table) {
	char foo[256];
	*foo = 0;
	for(int i=0; i<n; i++) {
		if ((value & (1<<i)) == 0) continue;
		if (*foo) strcat(foo," | ");
		strcat(foo,table[i]);
	}
	if (!*foo) strcat(foo,"0");
	return strdup(foo);
}
static char *choice_to_s(int value, int n, const char **table) {
	if (value < 0 || value >= n) {
		char foo[64];
		sprintf(foo,"(Unknown #%d)",value);
		return strdup(foo);
	} else {
		return strdup(table[value]);
	}
}
static void gfpost(VideoChannel *self) {
	char buf[256] = "[VideoChannel] ";
	WH(channel,"%d");
	WH(name,"\"%.32s\"");
	WH(tuners,"%d");
	WHFLAGS(flags,channel_flags);
	WH(type,"0x%04x");
	WH(norm,"%d");
	post("%s",buf);
}
static void gfpost(VideoTuner *self) {
	char buf[256] = "[VideoTuner] ";
	WH(tuner,"%d");
	WH(name,"\"%.32s\"");
	WH(rangelow,"%lu");
	WH(rangehigh,"%lu");
	WHFLAGS(flags,tuner_flags);
	WHCHOICE(mode,video_mode_choice);
	WH(signal,"%d");
	post("%s",buf);
}
static void gfpost(VideoWindow *self) {
	char buf[256] = "[VideoWindow] ";
	WHYX(pos,y,x);
	WHYX(size,height,width);
	WH(chromakey,"0x%08x");
	WH(flags,"0x%08x");
	WH(clipcount,"%d");
	post("%s",buf);
}
static void gfpost(VideoMbuf *self) {
	char buf[256] = "[VideoMBuf] ";
	WH(size,"%d");
	WH(frames,"%d");
	sprintf(buf+strlen(buf), "offsets=[");
	for (int i=0; i<self->frames; i++) {
		/* WH(offsets[i],"%d"); */
	        sprintf(buf+strlen(buf), "%d%s", self->offsets[i],
			i+1==self->frames?"]":", ");
	}
	post("%s",buf);
}
static void gfpost(VideoMmap *self) {
	char buf[256] = "[VideoMMap] ";
	WH(frame,"%u");
	WHYX(size,height,width);
	WHCHOICE(format,video_palette_choice);
	post("%s",buf);
};

/* **************************************************************** */

\class FormatVideoDev : Format {
	VideoCapability vcaps;
	VideoPicture vp;
	VideoMbuf vmbuf;
	VideoMmap vmmap;
	uint8 *image;
	int queue[8], queuesize, queuemax, next_frame;
	int current_channel, current_tuner;
	bool use_mmap, use_pwc;
	P<BitPacking> bit_packing;
	P<Dim> dim;
	bool has_frequency, has_tuner, has_norm;
	int fd;
	int palettes; /* bitfield */

	\constructor (string mode, string filename) {
		queuesize=0; queuemax=2; next_frame=0; use_mmap=true; use_pwc=false; bit_packing=0; dim=0;
		has_frequency=false;
		has_tuner=false;
		has_norm=false;
		image=0;
		f = fopen(filename.data(),"r+");
		if (!f) RAISE("can't open device '%s': %s",filename.data(),strerror(errno));
		fd = fileno(f);
		initialize2();
	}
	void frame_finished (uint8 *buf);

	void alloc_image ();
	void dealloc_image ();
	void frame_ask ();
	void initialize2 ();
	~FormatVideoDev () {if (image) dealloc_image();}

	\decl 0 bang ();
	\grin 0 int

	\attr int channel();
	\attr int tuner();
	\attr int norm();
	\decl 0 size (int sy, int sx);
	\decl 0 transfer (string sym, int queuemax=2);

	\attr t_symbol *colorspace;
	\attr int32  frequency();
	\attr uint16 brightness();
	\attr uint16 hue();
	\attr uint16 colour();
	\attr uint16 contrast();
	\attr uint16 whiteness();

	\attr bool   pwc(); /* 0..1 */
	\attr uint16 framerate();
	\attr uint16 white_mode(); /* 0..1 */
	\attr uint16 white_red();
	\attr uint16 white_blue();
	\attr uint16 white_speed();
	\attr uint16 white_delay();
	\attr int    auto_gain();
	\attr int    noise_reduction(); /* 0..3 */
	\attr int    compression();     /* 0..3 */
	\attr t_symbol *name;

	\decl 0 get (t_symbol *s=0);
};

#define DEBUG(args...) 42
//#define DEBUG(args...) post(args)

#define  IOCTL( F,NAME,ARG) \
  (DEBUG("fd%d.ioctl(0x%08x,0x%08x)",F,NAME,ARG), ioctl(F,NAME,ARG))
#define WIOCTL( F,NAME,ARG) \
  (IOCTL(F,NAME,ARG)<0 && (error("ioctl %s: %s",#NAME,strerror(errno)),1))
#define WIOCTL2(F,NAME,ARG) \
  (IOCTL(F,NAME,ARG)<0 && (error("ioctl %s: %s",#NAME,strerror(errno)), RAISE("ioctl error"), 0))

\def 0 get (t_symbol *s=0) {
	// this is abnormal for a get-function
	if (s==gensym("frequency") && !has_frequency  ) return;
	if (s==gensym("tuner")     && !has_tuner      ) return;
	if (s==gensym("norm")      && !has_norm       ) return;
	if (s==gensym("channel")   && vcaps.channels<2) return;
	if (!use_pwc && (s==gensym("white_mode")      || s==gensym("white_red")   || s==gensym("white_blue") ||
			 s==gensym("white_speed")     || s==gensym("white_delay") || s==gensym("auto_gain")  ||
			 s==gensym("noise_reduction") || s==gensym("compression") || s==gensym("framerate"))) return;
	FObject::_0_get(argc,argv,s);
	if (!s) {
		t_atom a[2];
		SETFLOAT(a+0,vcaps.minheight);
		SETFLOAT(a+1,vcaps.minwidth);
		outlet_anything(bself->outlets[0],gensym("minsize"),2,a);
		SETFLOAT(a+0,vcaps.maxheight);
		SETFLOAT(a+1,vcaps.maxwidth);
		outlet_anything(bself->outlets[0],gensym("maxsize"),2,a);
		char *foo = choice_to_s(vp.palette,COUNT(video_palette_choice),video_palette_choice);
		SETSYMBOL(a,gensym(foo));
		free(foo);
		outlet_anything(bself->outlets[0],gensym("palette"),1,a);
		SETSYMBOL(a,use_mmap ? gensym("mmap") : gensym("read"));
		outlet_anything(bself->outlets[0],gensym("transfer"),1,a);
		SETFLOAT(a+0,dim->v[0]);
		SETFLOAT(a+1,dim->v[1]);
		outlet_anything(bself->outlets[0],gensym("size"),2,a); // abnormal (does not use nested list)
	}
}

\def 0 size (int sy, int sx) {
	VideoWindow grab_win;
	// !@#$ bug here: won't flush the frame queue
	dim = new Dim(sy,sx,3);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	if (debug) gfpost(&grab_win);
	grab_win.clipcount = 0;
	grab_win.flags = 0;
	if (sy && sx) {
		grab_win.height = sy;
		grab_win.width  = sx;
	}
	if (debug) gfpost(&grab_win);
	WIOCTL(fd, VIDIOCSWIN, &grab_win);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	if (debug) gfpost(&grab_win);
}

void FormatVideoDev::dealloc_image () {
	if (!image) return;
	if (use_mmap) {
		munmap(image, vmbuf.size);
		image=0;
	} else {
		delete[] (uint8 *)image;
	}
}

void FormatVideoDev::alloc_image () {
	if (use_mmap) {
		WIOCTL2(fd, VIDIOCGMBUF, &vmbuf);
		//gfpost(&vmbuf);
		//size_t size = vmbuf.frames > 4 ? vmbuf.offsets[4] : vmbuf.size;
		image = (uint8 *)mmap(0,vmbuf.size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
		if (((long)image)==-1) {image=0; RAISE("mmap: %s", strerror(errno));}
	} else {
		image = new uint8[dim->prod(0,1)*bit_packing->bytes];
	}
}

void FormatVideoDev::frame_ask () {
	if (queuesize>=queuemax) RAISE("queue is full (queuemax=%d)",queuemax);
	if (queuesize>=vmbuf.frames) RAISE("queue is full (vmbuf.frames=%d)",vmbuf.frames);
	vmmap.frame = queue[queuesize++] = next_frame;
	vmmap.format = vp.palette;
	vmmap.width  = dim->get(1);
	vmmap.height = dim->get(0);
	WIOCTL2(fd, VIDIOCMCAPTURE, &vmmap);
	//gfpost(&vmmap);
	next_frame = (next_frame+1) % vmbuf.frames;
}

static uint8 clip(int x) {return x<0?0 : x>255?255 : x;}

void FormatVideoDev::frame_finished (uint8 *buf) {
	string cs = colorspace->s_name;
	int downscale = cs=="magic";
	/* picture is converted here. */
	int sy = dim->get(0)>>downscale;
	int sx = dim->get(1)>>downscale;
	int bs = dim->prod(1)>>downscale;
	uint8 b2[bs];
	//post("sy=%d sx=%d bs=%d",sy,sx,bs);
	//post("frame_finished, vp.palette = %d; colorspace = %s",vp.palette,cs.data());
	if (vp.palette==VIDEO_PALETTE_YUV420P) {
		GridOutlet out(this,0,cs=="magic"?new Dim(sy,sx,3):(Dim *)dim,cast);
		if (cs=="y") {
			out.send(sy*sx,buf);
		} else if (cs=="rgb") {
			for(int y=0; y<sy; y++) {
				uint8 *bufy = buf+sx* y;
				uint8 *bufu = buf+sx*sy    +(sx/2)*(y/2);
				uint8 *bufv = buf+sx*sy*5/4+(sx/2)*(y/2);
				int Y1,Y2,U,V;
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					Y1=bufy[x]   - 16;
					Y2=bufy[x+1] - 16;
					U=bufu[x/2] - 128;
					V=bufv[x/2] - 128;
					b2[xx+0]=clip((298*Y1         + 409*V)>>8);
					b2[xx+1]=clip((298*Y1 - 100*U - 208*V)>>8);
					b2[xx+2]=clip((298*Y1 + 516*U        )>>8);
					b2[xx+3]=clip((298*Y2         + 409*V)>>8);
					b2[xx+4]=clip((298*Y2 - 100*U - 208*V)>>8);
					b2[xx+5]=clip((298*Y2 + 516*U        )>>8);
				}
				out.send(bs,b2);
			}
		} else if (cs=="yuv") {
			for(int y=0; y<sy; y++) {
				uint8 *bufy = buf+sx* y;
				uint8 *bufu = buf+sx*sy    +(sx/2)*(y/2);
				uint8 *bufv = buf+sx*sy*5/4+(sx/2)*(y/2);
				int U,V;
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					U=bufu[x/2];
					V=bufv[x/2];
					b2[xx+0]=clip(((bufy[x+0]-16)*298)>>8);
					b2[xx+1]=clip(128+(((U-128)*293)>>8));
					b2[xx+2]=clip(128+(((V-128)*293)>>8));
					b2[xx+3]=clip(((bufy[x+1]-16)*298)>>8);
					b2[xx+4]=clip(128+(((U-128)*293)>>8));
					b2[xx+5]=clip(128+(((V-128)*293)>>8));
				}
				out.send(bs,b2);
			}
		} else if (cs=="magic") {
			for(int y=0; y<sy; y++) {
				uint8 *bufy = buf        +4*sx*y;
				uint8 *bufu = buf+4*sx*sy+  sx*y;
				uint8 *bufv = buf+5*sx*sy+  sx*y;
				for (int x=0,xx=0; x<sx; x++,xx+=3) {
					b2[xx+0]=bufy[x+x];
					b2[xx+1]=bufu[x];
					b2[xx+2]=bufv[x];
				}
				out.send(bs,b2);
			}
		}
	} else if (vp.palette==VIDEO_PALETTE_RGB32 || vp.palette==VIDEO_PALETTE_RGB24 || vp.palette==VIDEO_PALETTE_RGB565) {
		GridOutlet out(this,0,dim,cast);
		uint8 rgb[sx*3];
		uint8 b2[sx*3];
		if (cs=="y") {
			for(int y=0; y<sy; y++) {
			        bit_packing->unpack(sx,buf+y*sx*bit_packing->bytes,rgb);
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					b2[x+0] = (76*rgb[xx+0]+150*rgb[xx+1]+29*rgb[xx+2])>>8;
					b2[x+1] = (76*rgb[xx+3]+150*rgb[xx+4]+29*rgb[xx+5])>>8;
				}
				out.send(bs,b2);
			}
		} else if (cs=="rgb") {
			for(int y=0; y<sy; y++) {
			        bit_packing->unpack(sx,buf+y*sx*bit_packing->bytes,rgb);
				out.send(bs,rgb);
			}
		} else if (cs=="yuv") {
			for(int y=0; y<sy; y++) {
				bit_packing->unpack(sx,buf+y*sx*bit_packing->bytes,rgb);
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					b2[xx+0] = clip(    ((  76*rgb[xx+0] + 150*rgb[xx+1] +  29*rgb[xx+2])>>8));
					b2[xx+1] = clip(128+((- 44*rgb[xx+0] -  85*rgb[xx+1] + 108*rgb[xx+2])>>8));
					b2[xx+2] = clip(128+(( 128*rgb[xx+0] - 108*rgb[xx+1] -  21*rgb[xx+2])>>8));
					b2[xx+3] = clip(    ((  76*rgb[xx+3] + 150*rgb[xx+4] +  29*rgb[xx+5])>>8));
					b2[xx+4] = clip(128+((- 44*rgb[xx+3] -  85*rgb[xx+4] + 108*rgb[xx+5])>>8));
					b2[xx+5] = clip(128+(( 128*rgb[xx+3] - 108*rgb[xx+4] -  21*rgb[xx+5])>>8));
				}
				out.send(bs,b2);
			}
		} else if (cs=="magic") {
			RAISE("magic colorspace not supported with a RGB palette");
		}
	} else {
		RAISE("unsupported palette %d",vp.palette);
	}
}

/* these are factors for RGB to analog YUV */
// Y =   66*R + 129*G +  25*B
// U = - 38*R -  74*G + 112*B
// V =  112*R -  94*G -  18*B

// strange that read2 is not used and read3 is used instead
static int read2(int fd, uint8 *image, int n) {
	int r=0;
	while (n>0) {
		int rr=read(fd,image,n);
		if (rr<0) return rr; else {r+=rr; image+=rr; n-=rr;}
	}
	return r;
}

static int read3(int fd, uint8 *image, int n) {
	int r=read(fd,image,n);
	if (r<0) return r;
	return n;
}

\def 0 bang () {
	if (!image) alloc_image();
	if (!use_mmap) {
		/* picture is read at once by frame() to facilitate debugging. */
		int tot = dim->prod(0,1) * bit_packing->bytes;
		int n = (int) read3(fd,image,tot);
		if (n==tot) frame_finished(image);
		if (0> n) RAISE("error reading: %s", strerror(errno));
		if (n < tot) RAISE("unexpectedly short picture: %d of %d",n,tot);
		return;
	}
	while(queuesize<queuemax) frame_ask();
	vmmap.frame = queue[0];
	//uint64 t0 = gf_timeofday();
	WIOCTL2(fd, VIDIOCSYNC, &vmmap);
	//uint64 t1 = gf_timeofday();
	//if (t1-t0 > 100) gfpost("VIDIOCSYNC delay: %d us",t1-t0);
	frame_finished(image+vmbuf.offsets[queue[0]]);
	queuesize--;
	for (int i=0; i<queuesize; i++) queue[i]=queue[i+1];
	frame_ask();
}

GRID_INLET(0) {
	RAISE("can't write.");
} GRID_FLOW {
} GRID_FINISH {
} GRID_END

\def 0 norm (int value) {
	VideoTuner vtuner;
	vtuner.tuner = current_tuner;
	if (value<0 || value>3) RAISE("norm must be in range 0..3");
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) {
		post("no tuner #%d", value);
	} else {
		vtuner.mode = value;
		gfpost(&vtuner);
		WIOCTL(fd, VIDIOCSTUNER, &vtuner);
	}
}

\def int norm () {
	VideoTuner vtuner;
	vtuner.tuner = current_tuner;
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) {post("no tuner #%d", current_tuner); return -1;}
	return vtuner.mode;
}

\def 0 tuner (int value) {
	VideoTuner vtuner;
	vtuner.tuner = current_tuner = value;
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) RAISE("no tuner #%d", value);
	vtuner.mode = VIDEO_MODE_NTSC; //???
	gfpost(&vtuner);
	WIOCTL(fd, VIDIOCSTUNER, &vtuner);
	has_norm = (vtuner.mode<=3);
	int meuh;
	has_frequency = (ioctl(fd, VIDIOCGFREQ, &meuh)>=0);
}
\def int tuner () {return current_tuner;}

#define warn(fmt,stuff...) post("warning: " fmt,stuff)

\def 0 channel (int value) {
	VideoChannel vchan;
	vchan.channel = value;
	current_channel = value;
	if (0> IOCTL(fd, VIDIOCGCHAN, &vchan)) warn("no channel #%d", value);
	//gfpost(&vchan);
	WIOCTL(fd, VIDIOCSCHAN, &vchan);
	if (vcaps.type & VID_TYPE_TUNER) _0_tuner(0,0,0);
	has_tuner = (vcaps.type & VID_TYPE_TUNER && vchan.tuners > 1);
}
\def int channel () {return current_channel;}

\def 0 transfer (string sym, int queuemax=2) {
	if (sym=="read") {
		dealloc_image();
		use_mmap = false;
		post("transfer read");
	} else if (sym=="mmap") {
		dealloc_image();
		use_mmap = true;
		alloc_image();
		queuemax=min(8,min(queuemax,vmbuf.frames));
		post("transfer mmap with queuemax=%d (max max is vmbuf.frames=%d)", queuemax,vmbuf.frames);
		this->queuemax=queuemax;
	} else RAISE("don't know that transfer mode");
}

#define PICTURE_ATTR(_name_) {\
	WIOCTL(fd, VIDIOCGPICT, &vp); \
	vp._name_ = _name_; \
	WIOCTL(fd, VIDIOCSPICT, &vp);}

#define PICTURE_ATTRGET(_name_) { \
	WIOCTL(fd, VIDIOCGPICT, &vp); \
	/*gfpost("getting %s=%d",#_name_,vp._name_);*/ \
	return vp._name_;}

\def uint16 brightness ()                 {PICTURE_ATTRGET(brightness)}
\def 0      brightness (uint16 brightness){PICTURE_ATTR(   brightness)}
\def uint16 hue        ()                 {PICTURE_ATTRGET(hue)}
\def 0      hue        (uint16 hue)       {PICTURE_ATTR(   hue)}
\def uint16 colour     ()                 {PICTURE_ATTRGET(colour)}
\def 0      colour     (uint16 colour)    {PICTURE_ATTR(   colour)}
\def uint16 contrast   ()                 {PICTURE_ATTRGET(contrast)}
\def 0      contrast   (uint16 contrast)  {PICTURE_ATTR(   contrast)}
\def uint16 whiteness  ()                 {PICTURE_ATTRGET(whiteness)}
\def 0      whiteness  (uint16 whiteness) {PICTURE_ATTR(   whiteness)}
\def int32 frequency  () {
	int32 value;
	//if (ioctl(fd, VIDIOCGFREQ, &value)<0) {has_frequency=false; return 0;}
	WIOCTL(fd, VIDIOCGFREQ, &value);
	return value;
}
\def 0 frequency (int32 frequency) {
    long frequency_ = frequency;
	WIOCTL(fd, VIDIOCSFREQ, &frequency_);
}

\def 0 colorspace (t_symbol *colorspace) { /* y yuv rgb magic */
	string c = colorspace->s_name;
	if      (c=="y") {}
	else if (c=="yuv") {}
	else if (c=="rgb") {}
	else if (c=="magic") {}
	else RAISE("got '%s' but supported colorspaces are: y yuv rgb magic",c.data());
	WIOCTL(fd, VIDIOCGPICT, &vp);
	int palette = (palettes&(1<<VIDEO_PALETTE_RGB24)) ? VIDEO_PALETTE_RGB24 :
	              (palettes&(1<<VIDEO_PALETTE_RGB32)) ? VIDEO_PALETTE_RGB32 :
	              (palettes&(1<<VIDEO_PALETTE_RGB565)) ? VIDEO_PALETTE_RGB565 :
                      VIDEO_PALETTE_YUV420P;
	vp.palette = palette;
	WIOCTL(fd, VIDIOCSPICT, &vp);
	WIOCTL(fd, VIDIOCGPICT, &vp);
	if (vp.palette != palette) {
		post("this driver is unsupported: it wants palette %d instead of %d",vp.palette,palette);
		return;
	}
        if (palette == VIDEO_PALETTE_RGB565) {
            //uint32 masks[3] = { 0x00fc00,0x003e00,0x00001f };
            uint32 masks[3] = { 0x00f800,0x007e0,0x00001f };
	    bit_packing = new BitPacking(is_le(),2,3,masks);
	} else if (palette == VIDEO_PALETTE_RGB32) {
            uint32 masks[3] = { 0xff0000,0x00ff00,0x0000ff };
	    bit_packing = new BitPacking(is_le(),4,3,masks);
	} else {
            uint32 masks[3] = { 0xff0000,0x00ff00,0x0000ff };
	    bit_packing = new BitPacking(is_le(),3,3,masks);
	}
	this->colorspace=gensym(c.data());
	dim = new Dim(dim->v[0],dim->v[1],c=="y"?1:3);
}

\def bool pwc ()         {return use_pwc;}
\def 0    pwc (bool pwc) {use_pwc=pwc;}

void set_pan_and_tilt(int fd, char what, int pan, int tilt) { /*unused*/
	// if (!use_pwc) return;
	struct pwc_mpt_angles pma;
	pma.absolute=1;
	WIOCTL(fd, VIDIOCPWCMPTGANGLE, &pma);
	pma.pan = pan;
	pma.tilt = tilt;
	WIOCTL(fd, VIDIOCPWCMPTSANGLE, &pma);
}

\def uint16 framerate() {
	if (!use_pwc) return 0;
	struct video_window vwin;
	WIOCTL(fd, VIDIOCGWIN, &vwin);
	return (vwin.flags & PWC_FPS_MASK) >> PWC_FPS_SHIFT;
}

\def 0 framerate(uint16 framerate) {
	if (!use_pwc) return;
	struct video_window vwin;
	WIOCTL(fd, VIDIOCGWIN, &vwin);
	vwin.flags &= ~PWC_FPS_FRMASK;
	vwin.flags |= (framerate << PWC_FPS_SHIFT) & PWC_FPS_FRMASK;
	WIOCTL(fd, VIDIOCSWIN, &vwin);
}

/* those functions are still mostly unused */
//void set_compression_preference(int fd, int pref) {if (use_pwc) WIOCTL(fd, VIDIOCPWCSCQUAL, &pref);}

\def int auto_gain() {int auto_gain=0; if (use_pwc) WIOCTL(fd, VIDIOCPWCGAGC, &auto_gain); return auto_gain;}
\def 0   auto_gain   (int auto_gain) {if (use_pwc) WIOCTL(fd, VIDIOCPWCSAGC, &auto_gain);}

//void set_shutter_speed(int fd, int pref) {if (use_pwc) WIOCTL(fd, VIDIOCPWCSSHUTTER, &pref);}

\def uint16 white_mode () {
	if (!use_pwc) return 0;
	struct pwc_whitebalance pwcwb;
	WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	if (pwcwb.mode==PWC_WB_AUTO)   return 0;
	if (pwcwb.mode==PWC_WB_MANUAL) return 1;
	return 2;
}

\def 0 white_mode (uint16 white_mode) {
	if (!use_pwc) return;
	struct pwc_whitebalance pwcwb;
	WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	if      (white_mode==0) pwcwb.mode = PWC_WB_AUTO;
	else if (white_mode==1) pwcwb.mode = PWC_WB_MANUAL;
	/*else if (strcasecmp(mode, "indoor") == 0)  pwcwb.mode = PWC_WB_INDOOR;*/
	/*else if (strcasecmp(mode, "outdoor") == 0) pwcwb.mode = PWC_WB_OUTDOOR;*/
	/*else if (strcasecmp(mode, "fl") == 0)      pwcwb.mode = PWC_WB_FL;*/
	else {error("unknown mode number %d", white_mode); return;}
	WIOCTL(fd, VIDIOCPWCSAWB, &pwcwb);}

\def uint16 white_red() {if (!use_pwc) return 0;
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb); return pwcwb.manual_red;}
\def uint16 white_blue() {if (!use_pwc) return 0;
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb); return pwcwb.manual_blue;}
\def 0 white_red(uint16 white_red) {if (!use_pwc) return;
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	pwcwb.manual_red = white_red;  WIOCTL(fd, VIDIOCPWCSAWB, &pwcwb);}
\def 0 white_blue(uint16 white_blue) {if (!use_pwc) return;
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	pwcwb.manual_blue = white_blue;WIOCTL(fd, VIDIOCPWCSAWB, &pwcwb);}

\def uint16 white_speed() {if (!use_pwc) return 0;
	struct pwc_wb_speed pwcwbs;         WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs); return pwcwbs.control_speed;}
\def uint16 white_delay() {if (!use_pwc) return 0;
	struct pwc_wb_speed pwcwbs;         WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs); return pwcwbs.control_delay;}
\def 0 white_speed(uint16 white_speed) {if (!use_pwc) return;
	struct pwc_wb_speed pwcwbs;         WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs);
	pwcwbs.control_speed = white_speed; WIOCTL(fd, VIDIOCPWCSAWBSPEED, &pwcwbs);}
\def 0 white_delay(uint16 white_delay) {if (!use_pwc) return;
	struct pwc_wb_speed pwcwbs;         WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs);
	pwcwbs.control_delay = white_delay; WIOCTL(fd, VIDIOCPWCSAWBSPEED, &pwcwbs);}

void set_led_on_time(int fd, int val) {
	struct pwc_leds pwcl; WIOCTL(fd, VIDIOCPWCGLED, &pwcl);
	pwcl.led_on = val;    WIOCTL(fd, VIDIOCPWCSLED, &pwcl);}
void set_led_off_time(int fd, int val) {
	struct pwc_leds pwcl; WIOCTL(fd, VIDIOCPWCGLED, &pwcl);
	pwcl.led_off = val;   WIOCTL(fd, VIDIOCPWCSLED, &pwcl);}
void set_sharpness(int fd, int val) {WIOCTL(fd, VIDIOCPWCSCONTOUR, &val);}
void set_backlight_compensation(int fd, int val) {WIOCTL(fd, VIDIOCPWCSBACKLIGHT, &val);}
void set_antiflicker_mode(int fd, int val) {WIOCTL(fd, VIDIOCPWCSFLICKER, &val);}

\def int noise_reduction() {
	if (!use_pwc) return 0;
	int noise_reduction;
	WIOCTL(fd, VIDIOCPWCGDYNNOISE, &noise_reduction);
	return noise_reduction;
}
\def 0 noise_reduction(int noise_reduction) {
	if (!use_pwc) return;
	WIOCTL(fd, VIDIOCPWCSDYNNOISE, &noise_reduction);
}
\def int compression() {
	if (!use_pwc) return 0;
	int compression;
	WIOCTL(fd, VIDIOCPWCSCQUAL, &compression);
	return compression;
}
\def 0 compression(int compression) {
	if (!use_pwc) return;
	WIOCTL(fd, VIDIOCPWCGCQUAL, &compression);
}

void FormatVideoDev::initialize2 () {
	WIOCTL(fd, VIDIOCGCAP, &vcaps);
	_0_size(0,0,vcaps.maxheight,vcaps.maxwidth);
	char namebuf[33];
	memcpy(namebuf,vcaps.name,sizeof(vcaps.name));
	int i;
	for (i=32; i>=1; i--) if (!namebuf[i] || !isspace(namebuf[i])) break;
	namebuf[i]=0;
	while (--i>=0) if (isspace(namebuf[i])) namebuf[i]='_';
	name = gensym(namebuf);
	WIOCTL(fd, VIDIOCGPICT,&vp);
	palettes=0;
	int checklist[] = {VIDEO_PALETTE_RGB565,VIDEO_PALETTE_RGB24,VIDEO_PALETTE_RGB32,VIDEO_PALETTE_YUV420P};
#if 1
	for (size_t i=0; i<sizeof(checklist)/sizeof(*checklist); i++) {
		int p = checklist[i];
#else
	for (size_t p=0; p<17; p++) {
#endif
		vp.palette = p;
		ioctl(fd, VIDIOCSPICT,&vp);
		ioctl(fd, VIDIOCGPICT,&vp);
		if (vp.palette == p) {
			palettes |= 1<<p;
			post("palette %d supported",p);
		}
	}
	_0_colorspace(0,0,gensym("rgb"));
	_0_channel(0,0,0);
}

\end class FormatVideoDev {install_format("#io.videodev",4,"");}
void startup_videodev () {
	\startall
}
