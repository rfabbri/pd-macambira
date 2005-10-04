/*
	$Id: videodev.c,v 1.1 2005-10-04 02:02:15 matju Exp $

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
*/

#include "../base/grid.h.fcs"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/videodev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>

/* **************************************************************** */

typedef video_capability VideoCapability;
typedef video_channel    VideoChannel   ;
typedef video_tuner      VideoTuner     ;
typedef video_window     VideoWindow    ;
typedef video_picture    VideoPicture   ;
typedef video_mbuf       VideoMbuf      ;
typedef video_mmap       VideoMmap      ;

static const char *ruby_code =
\ruby
flags :name,:VideoTypeFlags,:start,0,:values,%w(
	CAPTURE TUNER TELETEXT OVERLAY CHROMAKEY CLIPPING FRAMERAM
	SCALES MONOCHROME SUBCAPTURE
	MPEG_DECODER MPEG_ENCODER MJPEG_DECODER MJPEG_ENCODER
)
flags :name,:TunerFlags,:start,0,:values,%w(
	PAL NTSC SECAM LOW NORM DUMMY5 DUMMY6 STEREO_ON RDS_ON MBS_ON
)
flags :name,:ChannelFlags,:start,0,:values,%w(
	TUNER AUDIO NORM
)
choice :name,:VideoPaletteChoice,:start,0,:values,%w(
	NIL GREY HI240
	RGB565 RGB24 RGB32 RGB555
	YUV422 YUYV UYVY YUV420 YUV411 RAW
	YUV422P YUV411P YUV420P YUV410P
)
choice :name,:VideoModeChoice,:start,0,:values,%w(
	PAL NTSC SECAM AUTO
)
\end ruby
;

/* **************************************************************** */

/*
#define WH(_field_,_spec_) \
	sprintf(buf+strlen(buf), "%s: " _spec_ "; ", #_field_, self->_field_);
#define WHYX(_name_,_fieldy_,_fieldx_) \
	sprintf(buf+strlen(buf), "%s: y=%d, x=%d; ", #_name_, self->_fieldy_, self->_fieldx_);
#define WHFLAGS(_field_,_table_) { \
	char *foo; \
	sprintf(buf+strlen(buf), "%s: %s; ", #_field_, \
		foo=flags_to_s(self->_field_,COUNT(_table_),_table_)); \
	delete[] foo;}
#define WHCHOICE(_field_,_table_) { \
	char *foo; \
	sprintf(buf+strlen(buf), "%s: %s; ", #_field_, \
		foo=choice_to_s(self->_field_,COUNT(_table_),_table_));\
	delete[] foo;}
static char *flags_to_s(int value, int n, named_int *table) {
	char foo[256];
	*foo = 0;
	for(int i=0; i<n; i++) {
		if ((value & (1<<i)) == 0) continue;
		if (*foo) strcat(foo," | ");
		strcat(foo,table[i].name);
	}
	if (!*foo) strcat(foo,"0");
	return strdup(foo);
}
static char *choice_to_s(int value, int n, named_int *table) {
	if (value < 0 || value >= n) {
		char foo[64];
		sprintf(foo,"(Unknown #%d)",value);
		return strdup(foo);
	} else {
		return strdup(table[value].name);
	}
}
*/

class RStream {
public:
	Ruby a;
	RStream(Ruby a) : a(a) {}
	RStream &operator <<(/*Ruby*/ void *v) { rb_ary_push(a,(Ruby)v); return *this; }
	RStream &operator <<(int v) { return *this<<(void *)INT2NUM(v); }
};

static void gfpost(VideoChannel *self) {
	RStream rs(rb_ary_new());
	rs << (void *)SYM(VideoChannel) << self->channel
		<< (void *)rb_str_new(self->name,strnlen(self->name,32))
		<< self->tuners << self->flags << self->type << self->norm;
	rb_p(rs.a);
}

static void gfpost(VideoTuner *self) {
	RStream rs(rb_ary_new());
	rs << (void *)SYM(VideoTuner) << self->tuner
		<< (void *)rb_str_new(self->name,strnlen(self->name,32))
		<< self->rangelow << self->rangehigh
		<< self->flags << self->mode << self->signal;
	rb_p(rs.a);
}

static void gfpost(VideoCapability *self) {
	RStream rs(rb_ary_new());
	rs << (void *)SYM(VideoCapability)
		<< (void *)rb_str_new(self->name,strnlen(self->name,32))
		<< self->type
		<< self->channels << self->audios
		<< self->maxheight << self->maxwidth
		<< self->minheight << self->minwidth;
	rb_p(rs.a);
}

static void gfpost(VideoWindow *self) {
	RStream rs(rb_ary_new());
	rs << (void *)SYM(VideoWindow)
		<< self->y << self->x 
		<< self->height << self->width
		<< self->chromakey << self->flags << self->clipcount;
	rb_p(rs.a);
}

static void gfpost(VideoPicture *self) {
	RStream rs(rb_ary_new());
	rs << (void *)SYM(VideoPicture)
		<< self->brightness << self->contrast << self->colour
		<< self->hue << self->whiteness << self->depth << self->palette;
	rb_p(rs.a);
}

static void gfpost(VideoMbuf *self) {
	RStream rs(rb_ary_new());
	rs << (void *)SYM(VideoMBuf) << self->size << self->frames;
	for (int i=0; i<4; i++) rs << self->offsets[i];
	rb_p(rs.a);
}

static void gfpost(VideoMmap *self) {
	RStream rs(rb_ary_new());
	rs << (void *)SYM(VideoMMap) << self->frame
		<< self->height << self->width << self->format;
	rb_p(rs.a);
};

/* **************************************************************** */

\class FormatVideoDev < Format
struct FormatVideoDev : Format {
	VideoCapability vcaps;
	VideoMbuf vmbuf;
	VideoMmap vmmap;
	Pt<uint8> image;
	int palette;
	int queue[8], queuesize, queuemax, next_frame;
	int current_channel, current_tuner;
	bool use_mmap;
	P<BitPacking> bit_packing;
	P<Dim> dim;

	FormatVideoDev () : queuesize(0), queuemax(2), next_frame(0), use_mmap(true), bit_packing(0), dim(0) {}
	void frame_finished (Pt<uint8> buf);

	\decl void initialize (Symbol mode, String filename, Symbol option=Qnil);
	\decl void initialize2 ();
	\decl void close ();
	\decl void alloc_image ();
	\decl void dealloc_image ();
	\decl void frame ();
	\decl void frame_ask ();
	\grin 0 int

	\decl void _0_size (int sy, int sx);
	\decl void _0_norm (int value);
	\decl void _0_tuner (int value);
	\decl void _0_channel (int value);
	\decl void _0_frequency (int value);
	\decl void _0_transfer (Symbol sym, int queuemax=2);
	\decl void _0_colorspace (Symbol c);
	\decl void _0_get        (Symbol attr=0);
	\decl void _0_brightness (uint16 value);
	\decl void _0_hue        (uint16 value);
	\decl void _0_colour     (uint16 value);
	\decl void _0_contrast   (uint16 value);
	\decl void _0_whiteness  (uint16 value);
};

#define DEBUG(args...) 42
//#define DEBUG(args...) gfpost

#define IOCTL(_f_,_name_,_arg_) \
	(DEBUG("fd%d.ioctl(0x%08x(:%s),0x%08x)\n",_f_,_name_,#_name_,_arg_), \
		ioctl(_f_,_name_,_arg_))

#define WIOCTL(_f_,_name_,_arg_) \
	(DEBUG("fd%d.ioctl(0x%08x(:%s),0x%08x)\n",_f_,_name_,#_name_,_arg_), \
	ioctl(_f_,_name_,_arg_) < 0) && \
		(gfpost("ioctl %s: %s",#_name_,strerror(errno)),1)

#define WIOCTL2(_f_,_name_,_arg_) \
	((DEBUG("fd%d.ioctl(0x%08x(:%s),0x%08x)\n",_f_,_name_,#_name_,_arg_), \
	ioctl(_f_,_name_,_arg_) < 0) && \
		(gfpost("ioctl %s: %s",#_name_,strerror(errno)), \
		 RAISE("ioctl error"), 0))

#define GETFD NUM2INT(rb_funcall(rb_ivar_get(rself,SI(@stream)),SI(fileno),0))

\def void _0_size (int sy, int sx) {
	int fd = GETFD;
	VideoWindow grab_win;
	// !@#$ bug here: won't flush the frame queue
	dim = new Dim(sy,sx,3);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	gfpost(&grab_win);
	grab_win.clipcount = 0;
	grab_win.flags = 0;
	if (sy && sx) {
		grab_win.height = sy;
		grab_win.width  = sx;
	}
	gfpost(&grab_win);
	WIOCTL(fd, VIDIOCSWIN, &grab_win);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	gfpost(&grab_win);
}

\def void dealloc_image () {
	if (!image) return;
	if (!use_mmap) {
		delete[] (uint8 *)image;
	} else {
		munmap(image, vmbuf.size);
		image = Pt<uint8>();
	}
}

\def void alloc_image () {
	if (!use_mmap) {
		image = ARRAY_NEW(uint8,dim->prod(0,1)*bit_packing->bytes);
		return;
	}
	int fd = GETFD;
	WIOCTL2(fd, VIDIOCGMBUF, &vmbuf);
	image = Pt<uint8>((uint8 *)
		mmap(0,vmbuf.size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0),
		vmbuf.size);
	if (((int)image)<=0) {
		image=Pt<uint8>();
		RAISE("mmap: %s", strerror(errno));
	}
}

\def void frame_ask () {
	int fd = GETFD;
	if (queuesize>=queuemax) RAISE("queue is full (queuemax=%d)",queuemax);
	if (queuesize>=vmbuf.frames) RAISE("queue is full (vmbuf.frames=%d)",vmbuf.frames);
	vmmap.frame = queue[queuesize++] = next_frame;
	vmmap.format = palette;
	vmmap.width  = dim->get(1);
	vmmap.height = dim->get(0);
	WIOCTL2(fd, VIDIOCMCAPTURE, &vmmap);
	next_frame = (next_frame+1) % vmbuf.frames;
}

void FormatVideoDev::frame_finished (Pt<uint8> buf) {
	GridOutlet out(this,0,dim,NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
	/* picture is converted here. */
	int sy = dim->get(0);
	int sx = dim->get(1);
	int bs = dim->prod(1);
	if (palette==VIDEO_PALETTE_YUV420P) {
		STACK_ARRAY(uint8,b2,bs);
		for(int y=0; y<sy; y++) {
			Pt<uint8> bufy = buf+sx*y;
			Pt<uint8> bufu = buf+sx*sy    +(sx/2)*(y/2);
			Pt<uint8> bufv = buf+sx*sy*5/4+(sx/2)*(y/2);
			for (int x=0; x<sx; x++) {
				b2[x*3+0]=bufy[x];
				b2[x*3+1]=bufu[x/2];
				b2[x*3+2]=bufv[x/2];
			}
			out.send(bs,b2);
		}
	} else if (bit_packing) {
		STACK_ARRAY(uint8,b2,bs);
		for(int y=0; y<sy; y++) {
			Pt<uint8> buf2 = buf+bit_packing->bytes*sx*y;
			bit_packing->unpack(sx,buf2,b2);
			out.send(bs,b2);
		}
	} else {
		out.send(sy*bs,buf);
	}
}

static int read2(int fd, uint8 *image, int n) {
	int r=0;
	for (; n>0; ) {
		int rr=read(fd,image,n);
		if (rr<0) return rr;
		r+=rr, image+=rr, n-=rr;
	}
	return r;
}

static int read3(int fd, uint8 *image, int n) {
	int r=read(fd,image,n);
	if (r<0) return r;
	return n;
}

\def void frame () {
	if (!image) rb_funcall(rself,SI(alloc_image),0);
	int fd = GETFD;
	if (!use_mmap) {
		/* picture is read at once by frame() to facilitate debugging. */
		int tot = dim->prod(0,1) * bit_packing->bytes;
		int n = (int) read3(fd,image,tot);
		if (n==tot) frame_finished(image);
		if (0> n) RAISE("error reading: %s", strerror(errno));
		if (n < tot) RAISE("unexpectedly short picture: %d of %d",n,tot);
		return;
	}
	while(queuesize<queuemax) rb_funcall(rself,SI(frame_ask),0);
	vmmap.frame = queue[0];
	uint64 t0 = gf_timeofday();
	WIOCTL2(fd, VIDIOCSYNC, &vmmap);
	uint64 t1 = gf_timeofday();
	//if (t1-t0 > 100) gfpost("VIDIOCSYNC delay: %d us",t1-t0);
	frame_finished(image+vmbuf.offsets[queue[0]]);
	queuesize--;
	for (int i=0; i<queuesize; i++) queue[i]=queue[i+1];
	rb_funcall(rself,SI(frame_ask),0);
}

GRID_INLET(FormatVideoDev,0) {
	RAISE("can't write.");
} GRID_FLOW {
} GRID_FINISH {
} GRID_END

\def void _0_norm (int value) {
	int fd = GETFD;
	VideoTuner vtuner;
	vtuner.tuner = current_tuner;
	if (value<0 || value>3) RAISE("norm must be in range 0..3");
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) {
		gfpost("no tuner #%d", value);
	} else {
		vtuner.mode = value;
		gfpost(&vtuner);
		WIOCTL(fd, VIDIOCSTUNER, &vtuner);
	}
}

\def void _0_tuner (int value) {
	int fd = GETFD;
	VideoTuner vtuner;
	vtuner.tuner = current_tuner = value;
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) RAISE("no tuner #%d", value);
	vtuner.mode = VIDEO_MODE_NTSC; //???
	gfpost(&vtuner);
	WIOCTL(fd, VIDIOCSTUNER, &vtuner);
}

\def void _0_channel (int value) {
	int fd = GETFD;
	VideoChannel vchan;
	vchan.channel = value;
	current_channel = value;
	if (0> IOCTL(fd, VIDIOCGCHAN, &vchan)) RAISE("no channel #%d", value);
	gfpost(&vchan);
	WIOCTL(fd, VIDIOCSCHAN, &vchan);
	if (vcaps.type & VID_TYPE_TUNER) rb_funcall(rself,SI(_0_tuner),1,INT2NUM(0));
}

\def void _0_frequency (int value) {
	int fd = GETFD;
	if (0> IOCTL(fd, VIDIOCSFREQ, &value)) RAISE("can't set frequency to %d",value);
}

\def void _0_transfer (Symbol sym, int queuemax=2) {
	if (sym == SYM(read)) {
		rb_funcall(rself,SI(dealloc_image),0);
		use_mmap = false;
		gfpost("transfer read");
	} else if (sym == SYM(mmap)) {
		rb_funcall(rself,SI(dealloc_image),0);
		use_mmap = true;
		rb_funcall(rself,SI(alloc_image),0);
		queuemax=min(queuemax,vmbuf.frames);
		gfpost("transfer mmap with queuemax=%d (max max is vmbuf.frames=%d)"
			,queuemax,vmbuf.frames);
		this->queuemax=queuemax;
	} else RAISE("don't know that transfer mode");
}

#define PICTURE_ATTR(_name_) {\
	int fd = GETFD; \
	VideoPicture vp; \
	WIOCTL(fd, VIDIOCGPICT, &vp); \
	vp._name_ = value; \
	WIOCTL(fd, VIDIOCSPICT, &vp);}

\def void _0_brightness (uint16 value) {PICTURE_ATTR(brightness)}
\def void _0_hue      (uint16 value)   {PICTURE_ATTR(hue)}
\def void _0_colour (uint16 value)     {PICTURE_ATTR(colour)}
\def void _0_contrast (uint16 value)   {PICTURE_ATTR(contrast)}
\def void _0_whiteness (uint16 value)  {PICTURE_ATTR(whiteness)}

#define PICTURE_ATTR_GET(_name_) { \
	int fd = GETFD; \
	VideoPicture vp; \
	WIOCTL(fd, VIDIOCGPICT, &vp); \
	Ruby argv[3] = {INT2NUM(1), SYM(_name_), INT2NUM(vp._name_)}; \
	send_out(COUNT(argv),argv);}

\def void _0_get (Symbol attr) {
	if (!attr) {
		_0_get(0,0,SYM(brightness));
		_0_get(0,0,SYM(hue       ));
		_0_get(0,0,SYM(colour    ));
		_0_get(0,0,SYM(contrast  ));
		_0_get(0,0,SYM(whiteness ));
		_0_get(0,0,SYM(frequency ));
	} else if (attr==SYM(brightness)) { PICTURE_ATTR_GET(brightness);
	} else if (attr==SYM(hue       )) { PICTURE_ATTR_GET(hue       );
	} else if (attr==SYM(colour    )) { PICTURE_ATTR_GET(colour    );
	} else if (attr==SYM(contrast  )) { PICTURE_ATTR_GET(contrast  );
	} else if (attr==SYM(whiteness )) { PICTURE_ATTR_GET(whiteness );
	} else if (attr==SYM(frequency )) {
		int fd = GETFD;
		int value;
		WIOCTL(fd, VIDIOCGFREQ, &value);
		{Ruby argv[3] ={INT2NUM(1), SYM(frequency), INT2NUM(value)}; send_out(COUNT(argv),argv);}
	} else { RAISE("What you say?"); }
}

\def void close () {
	if (image) rb_funcall(rself,SI(dealloc_image),0);
	rb_call_super(argc,argv);
}

\def void _0_colorspace (Symbol c) {
	if (c==SYM(RGB24)) palette=VIDEO_PALETTE_RGB24;
	else if (c==SYM(YUV420P)) palette=VIDEO_PALETTE_YUV420P;
	else RAISE("supported: RGB24, YUV420P");

	int fd = GETFD;
	VideoPicture *gp = new VideoPicture;
	WIOCTL(fd, VIDIOCGPICT, gp);
	gp->palette = palette;
	WIOCTL(fd, VIDIOCSPICT, gp);
	WIOCTL(fd, VIDIOCGPICT, gp);
	//if (bit_packing) { delete bit_packing; bit_packing=0; }
	switch(palette) {
	case VIDEO_PALETTE_RGB24:{
		uint32 masks[3] = { 0xff0000,0x00ff00,0x0000ff };
		bit_packing = new BitPacking(is_le(),3,3,masks);
	}break;
	case VIDEO_PALETTE_YUV420P:{
		// woops, special case already, can't do that with bit_packing
	}
	default:
		RAISE("can't handle palette %d", gp->palette);
	}
	delete gp;
}

\def void initialize2 () {
	int fd = GETFD;
	VideoPicture *gp = new VideoPicture;
/*	long flags;
	fcntl(fd,F_GETFL,&flags);
	flags |= O_NONBLOCK;
	fcntl(fd,F_SETFL,&flags); */

	WIOCTL(fd, VIDIOCGCAP, &vcaps);
	gfpost(&vcaps);
	rb_funcall(rself,SI(_0_size),2,INT2NUM(vcaps.maxheight),INT2NUM(vcaps.maxwidth));
	WIOCTL(fd, VIDIOCGPICT, gp);
	gfpost(gp);
	char buf[1024] = "";
	int n = 17 /*COUNT(video_palette_choice)*/;
	for (int i=0; i<n; i++) {
		gp->palette = i;
		ioctl(fd, VIDIOCSPICT, gp);
		ioctl(fd, VIDIOCGPICT, gp);
		if (gp->palette == i) {
			if (*buf) strcpy(buf+strlen(buf),", ");
			//strcpy(buf+strlen(buf),video_palette_choice[i].name);
			sprintf(buf+strlen(buf),"%d",i);
		}
	}
	gfpost("This card supports palettes: %s", buf);
	_0_colorspace(0,0,SYM(RGB24));
	rb_funcall(rself,SI(_0_channel),1,INT2NUM(0));
	delete gp;
}

\def void initialize (Symbol mode, String filename, Symbol option=Qnil) {
	rb_call_super(argc,argv);
	image = Pt<uint8>();
	rb_ivar_set(rself,SI(@stream),
		rb_funcall(rb_cFile,SI(open),2,filename,rb_str_new2("r+")));
	rb_funcall(rself,SI(initialize2),0);
}

\classinfo {
	IEVAL(rself,"install '#io:videodev',1,1;@flags=4;@comment='Video4linux 1.x'");
}
\end class FormatVideoDev
void startup_videodev () {
	\startall
}
