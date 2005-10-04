/*
	$Id: mpeg3.c,v 1.1 2005-10-04 02:02:15 matju Exp $

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

#define LIBMPEG_INCLUDE_HERE
#include "../base/grid.h.fcs"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

\class FormatMPEG3 < Format
struct FormatMPEG3 : Format {
	mpeg3_t *mpeg;
	P<BitPacking> bit_packing;
	int track;
	FormatMPEG3 () : track(0) {}
	\decl void initialize (Symbol mode, Symbol source, String filename);
	\decl void seek (int frame);
	\decl Ruby frame ();
	\decl void close ();
};

\def void seek (int frame) { mpeg3_set_frame(mpeg,frame,track); }

\def Ruby frame () {
	int nframe = mpeg3_get_frame(mpeg,track);
	if (nframe >= mpeg3_video_frames(mpeg,track)) return Qfalse;

	int sx = mpeg3_video_width(mpeg,track);
	int sy = mpeg3_video_height(mpeg,track);
	int npixels = sx*sy;
	int channels = 3;
	Pt<uint8> buf = ARRAY_NEW(uint8,sy*sx*channels+16);
	uint8 *rows[sy];
	for (int i=0; i<sy; i++) rows[i]=buf+i*sx*channels;
	int result = mpeg3_read_frame(mpeg,rows,0,0,sx,sy,sx,sy,MPEG3_RGB888,track);

	GridOutlet out(this,0,new Dim(sy, sx, channels),
		NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
	int bs = out.dim->prod(1);
	STACK_ARRAY(int32,b2,bs);
	for(int y=0; y<sy; y++) {
		Pt<uint8> row = buf+channels*sx*y;
		/* bit_packing->unpack(sx,row,b2); out.send(bs,b2); */
		out.send(bs,row);
	}
	delete[] (uint8 *)buf;
	return INT2NUM(nframe);
}

\def void close () {
//	fprintf(stderr, "begin mpeg3_close...\n");
	if (mpeg) { mpeg3_close(mpeg); mpeg=0; }
	rb_call_super(argc,argv);
//	fprintf(stderr, "end mpeg3_close...\n");
}

// libmpeg3 may be nice, but it won't take a filehandle, only filename
\def void initialize (Symbol mode, Symbol source, String filename) {
	rb_call_super(argc,argv);
	if (mode!=SYM(in)) RAISE("read-only, sorry");
	if (source!=SYM(file)) RAISE("usage: mpeg file <filename>");
	if (TYPE(filename)!=T_STRING) RAISE("PATATE POILUE");
	filename = rb_funcall(mGridFlow,SI(find_file),1,filename);
	mpeg = mpeg3_open(rb_str_ptr(filename));
	if (!mpeg) RAISE("IO Error: can't open file `%s': %s", filename, strerror(errno));
	uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
	bit_packing = new BitPacking(is_le(),3,3,mask);
}

\classinfo {
	IEVAL(rself,"install '#in:mpeg',1,1;@flags=4;"
	"@comment='Motion Picture Expert Group Format"
	" (using HeroineWarrior\\'s)';suffixes_are'mpg,mpeg'");
}
\end class FormatMPEG3
void startup_mpeg3 () {
	\startall
}
