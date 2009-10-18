/*
	$Id: quicktimehw.c 4062 2008-08-05 01:33:57Z matju $

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

#define QUICKTIMEHW_INCLUDE_HERE
#include "../gridflow.h.fcs"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <map>
#include <vector>

static std::map<string,std::vector<string> *> codecs;
static std::map<string,string> fourccs;

\class FormatQuickTimeHW : Format {
	quicktime_t *anim;
	int track;
	P<Dim> dim;
	char *codec;
	int colorspace;
	int channels;
	bool started;
	P<Dim> force;
	float64 framerate;
	P<BitPacking> bit_packing;
	int jpeg_quality; // in theory we shouldn't need this, but...
	~FormatQuickTimeHW() {if (anim) quicktime_close(anim);}
	\constructor (t_symbol *mode, string filename) {
		track=0; dim=0; codec=QUICKTIME_RAW; started=false; force=0; framerate=29.97; bit_packing=0; jpeg_quality=75;
// libquicktime may be nice, but it won't take a filehandle, only filename
		filename = gf_find_file(filename);
		anim = quicktime_open((char *)filename.data(),mode==gensym("in"),mode==gensym("out"));
		if (!anim) RAISE("can't open file `%s': %s (or some other reason that libquicktime won't tell us)",
			filename.data(), strerror(errno));
		if (mode==gensym("in")) {
	/* This doesn't really work: (is it just for encoding?)
			if (!quicktime_supported_video(anim,track))
				RAISE("quicktime: unsupported codec: %s",
				      quicktime_video_compressor(anim,track));
	*/
		}
		_0_colorspace(0,0,string("rgb"));
		quicktime_set_cpus(anim,1);
		uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
		bit_packing = new BitPacking(is_le(),3,3,mask);
	}
	\decl 0 bang ();
	\decl 0 seek (long frame);
	\decl 0 rewind ();
	\decl 0 force_size (int32 height, int32 width);
	\decl 0 codec (string c);
	\decl 0 colorspace (string c);
	\decl 0 parameter (string name, int32 value);
	\decl 0 framerate (float64 f);
	\decl 0 size (int32 height, int32 width);
	\decl 0 get ();
	\grin 0 int
};

\def 0 force_size (int32 height, int32 width) { force = new Dim(height, width); }
\def 0 seek (long frame) {
	quicktime_set_video_position(anim,clip(frame,0L,quicktime_video_length(anim,track)-1),track);
}
\def 0 rewind () {_0_seek(0,0,0);}

\def 0 bang () {
	long length = quicktime_video_length(anim,track);
	long nframe = quicktime_video_position(anim,track);
	if (nframe >= length) {outlet_bang(bself->te_outlet); return;}
	/* if it works, only do it once, to avoid silly stderr messages forgotten in LQT */
	if (!quicktime_reads_cmodel(anim,colorspace,0) && !started) {
		RAISE("LQT says this video cannot be decoded into the chosen colorspace");
	}
	int sx = quicktime_video_width(anim,track);
	int sy = quicktime_video_height(anim,track);
	int sz = quicktime_video_depth(anim,track);
	channels = sz/8; // hack. how do i get the video's native colormodel ?
	switch (sz) {
	case 24: colorspace=BC_RGB888; break;
	case 32: colorspace=BC_RGBA8888; break;
	default: post("strange quicktime. ask matju."); break;
	}
	if (force) {
		sy = force->get(0);
		sx = force->get(1);
	}
	uint8 buf[sy*sx*channels];
	uint8 *rows[sy]; for (int i=0; i<sy; i++) rows[i]=buf+i*sx*channels;
	quicktime_decode_scaled(anim,0,0,sx,sy,sx,sy,colorspace,rows,track);
	GridOutlet out(this,0,new Dim(sy,sx,channels),cast);
	out.send(sy*sx*channels,buf);
	started=true;
//	return INT2NUM(nframe);
}

//!@#$ should also support symbol values (how?)
\def 0 parameter (string name, int32 value) {
	int val = value;
	//post("quicktime_set_parameter %s %d",name.data(), val);
	quicktime_set_parameter(anim, const_cast<char *>(name.data()), &val);
	if (name=="jpeg_quality") jpeg_quality=value;
}

\def 0 framerate (float64 f) {
	framerate=f;
	quicktime_set_framerate(anim, f);
}

\def 0 size (int32 height, int32 width) {
	if (dim) RAISE("video size already set!");
	// first frame: have to do setup
	dim = new Dim(height, width, 3);
	quicktime_set_video(anim,1,dim->get(1),dim->get(0),framerate,codec);
	quicktime_set_cmodel(anim,colorspace);
}

GRID_INLET(0) {
	if (in->dim->n != 3)           RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2)!=channels) RAISE("expecting %d channels (got %d)",channels,in->dim->get(2));
	in->set_chunk(0);
	if (dim) {
		if (!dim->equal(in->dim)) RAISE("all frames should be same size");
	} else {
		// first frame: have to do setup
		dim = in->dim;
		quicktime_set_video(anim,1,dim->get(1),dim->get(0),framerate,codec);
		quicktime_set_cmodel(anim,colorspace);
		quicktime_set_depth(anim,8*channels,track);
	}
	//post("quicktime jpeg_quality %d", jpeg_quality);
	quicktime_set_parameter(anim, (char*)"jpeg_quality", &jpeg_quality);
} GRID_FLOW {
	int sx = quicktime_video_width(anim,track);
	int sy = quicktime_video_height(anim,track);
	uint8 *rows[sy];
	if (sizeof(T)>1) {
		uint8 data2[n];
		bit_packing->pack(sx*sy,data,(uint8 *)data2);
		for (int i=0; i<sy; i++) rows[i]=data2+i*sx*channels;
		quicktime_encode_video(anim,rows,track);
	} else {
		for (int i=0; i<sy; i++) rows[i]=(uint8 *)data+i*sx*channels;
		quicktime_encode_video(anim,rows,track);
	}
} GRID_FINISH {
} GRID_END

\def 0 codec (string c) {
#ifdef LQT_VERSION
	char buf[5];
	strncpy(buf,c.data(),4);
	for (int i=c.length(); i<4; i++) buf[i]=' ';
	buf[4]=0;
	if (fourccs.find(string(buf))==fourccs.end())
		RAISE("warning: unknown fourcc '%s'" /*" (%s)"*/, buf /*, rb_str_ptr(rb_inspect(rb_funcall(fourccs,SI(keys),0)))*/);
#endif	
	codec = strdup(buf);
}

\def 0 colorspace (string c) {
	if (0) {
	} else if (c=="rgb")     { channels=3; colorspace=BC_RGB888; 
	} else if (c=="rgba")    { channels=4; colorspace=BC_RGBA8888;
	} else if (c=="bgr")     { channels=3; colorspace=BC_BGR888;
	} else if (c=="bgrn")    { channels=4; colorspace=BC_BGR8888;
//	} else if (c=="yuv")     { channels=3; colorspace=BC_YUV888;
	} else if (c=="yuva")    { channels=4; colorspace=BC_YUVA8888;
	} else if (c=="YUV420P") { channels=3; colorspace=BC_YUV420P;
	} else RAISE("unknown colorspace '%s' (supported: rgb, rgba, bgr, bgrn, yuv, yuva)",c.data());
}

\def 0 get () {
/*	t_atom a[1];
	SETFLOAT(a,(float)length);
	outlet_anything(bself->te_outlet,gensym("frames"),1,a);
*/
	t_atom a[1];
	SETFLOAT(a,quicktime_video_length(anim,track));
	outlet_anything(bself->outlets[0],gensym("frames"),1,a);
	SETFLOAT(a,quicktime_frame_rate(anim,track));
	outlet_anything(bself->outlets[0],gensym("framerate"),1,a);
	SETFLOAT(a,quicktime_video_height(anim,track));
	outlet_anything(bself->outlets[0],gensym("height"),1,a);
	SETFLOAT(a,quicktime_video_width(anim,track));
	outlet_anything(bself->outlets[0],gensym("width"),1,a);
	SETFLOAT(a,quicktime_video_depth(anim,track));
	outlet_anything(bself->outlets[0],gensym("depth"),1,a);
	SETSYMBOL(a,gensym(quicktime_video_compressor(anim,track)));
	outlet_anything(bself->outlets[0],gensym("codec"),1,a);
	//SUPER;
}

\classinfo {install_format("#io.quicktime",6,"mov avi");
//  def self.info; %[codecs: #{@codecs.keys.join' '}] end
//#define L fprintf(stderr,"%s:%d in %s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__);
#ifdef LQT_VERSION
	lqt_registry_init();
	int n = lqt_get_num_video_codecs();
	for (int i=0; i<n; i++) {
		const lqt_codec_info_t *s = lqt_get_video_codec_info(i);
		if (!s->name) {
			fprintf(stderr,"[#in quicktime]: skipping codec with null name!\n");
			continue;
		}
		string name = string(s->name);
		std::vector<string> *f = new std::vector<string>(s->num_fourccs);
		if (!s->fourccs) {
			post("WARNING: no fourccs (quicktime library is broken?)");
			goto hell;
		}
		//fprintf(stderr,"num_fourccs=%d fourccs=%p\n",s->num_fourccs,s->fourccs);
		for (int j=0; j<s->num_fourccs; j++) {
			string fn = string(s->fourccs[j]);
			f->push_back(fn);
			fourccs[fn]=name;
		}
		codecs[name]=f;
		hell:;
	}
#endif
}
\end class FormatQuickTimeHW
void startup_quicktimehw () {
	\startall
}
