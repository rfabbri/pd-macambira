/*
	$Id: mpeg3.c 4620 2009-11-01 21:16:58Z matju $

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

#define LIBMPEG_INCLUDE_HERE
#include "gridflow.hxx.fcs"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

\class FormatMPEG3 : Format {
	mpeg3_t *mpeg;
	int track;
	~FormatMPEG3 () {if (mpeg) {mpeg3_close(mpeg); mpeg=0;}}
	\constructor (t_symbol *mode, string filename) {
		track=0;
	// libmpeg3 may be nice, but it won't take a filehandle, only filename
		if (mode!=gensym("in")) RAISE("read-only, sorry");
		filename = gf_find_file(filename);
	#ifdef MPEG3_UNDEFINED_ERROR
		int err;
		mpeg = mpeg3_open((char *)filename.data(),&err);
		post("mpeg error code = %d",err);
	#else
		mpeg = mpeg3_open((char *)filename.data());
	#endif
		if (!mpeg) RAISE("IO Error: can't open file `%s': %s", filename.data(), strerror(errno));
	}
	\decl 0 seek (int32 frame);
	\decl 0 rewind ();
	\decl 0 bang ();
};

\def 0 seek (int32 frame) {
	mpeg3_set_frame(mpeg,clip(frame,int32(0),int32(mpeg3_video_frames(mpeg,track)-1)),track);
}
\def 0 rewind () {_0_seek(0,0,0);}

\def 0 bang () {
	int nframe = mpeg3_get_frame(mpeg,track);
	int nframes = mpeg3_video_frames(mpeg,track);
	//post("track=%d; nframe=%d; nframes=%d",track,nframe,nframes);
	if (nframe >= nframes) {outlet_bang(bself->te_outlet); return;}
	int sx = mpeg3_video_width(mpeg,track);
	int sy = mpeg3_video_height(mpeg,track);
	int channels = 3;
	/* !@#$ the doc says "You must allocate 4 extra bytes in the
	last output_row. This is scratch area for the MMX routines." */
	uint8 *buf = NEWBUF(uint8,sy*sx*channels+16);
	uint8 *rows[sy];
	for (int i=0; i<sy; i++) rows[i]=buf+i*sx*channels;
	mpeg3_read_frame(mpeg,rows,0,0,sx,sy,sx,sy,MPEG3_RGB888,track);
	GridOutlet out(this,0,new Dim(sy,sx,channels),cast);
	int bs = out.dim->prod(1);
	for(int y=0; y<sy; y++) out.send(bs,buf+channels*sx*y);
	DELBUF(buf);
//	return INT2NUM(nframe);
}

\classinfo {install_format("#io.mpeg",4,"mpg mpeg");}
\end class FormatMPEG3
void startup_mpeg3 () {
	\startall
}
