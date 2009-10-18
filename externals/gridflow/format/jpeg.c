/*
	$Id: jpeg.c 4110 2008-11-09 22:07:08Z matju $

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

//!@#$ not handling abort on compress
//!@#$ not handling abort on decompress

#include "../gridflow.h.fcs"
/* removing macros (removing warnings) */
#undef HAVE_PROTOTYPES
#undef HAVE_STDLIB_H
#undef EXTERN
extern "C" {
#include <jpeglib.h>
};

\class FormatJPEG < Format {
	P<BitPacking> bit_packing;
	struct jpeg_compress_struct cjpeg;
	struct jpeg_decompress_struct djpeg;
	struct jpeg_error_mgr jerr;
	short quality;
	\constructor (t_symbol *mode, string filename) {
		Format::_0_open(0,0,mode,filename);
		uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
		bit_packing = new BitPacking(is_le(),3,3,mask);
		quality = 75;
	}
	\decl 0 bang ();
	\decl 0 quality (short quality);
	\grin 0 int
};

GRID_INLET(0) {
	if (in->dim->n!=3)      RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2)!=3) RAISE("expecting 3 channels (got %d)",in->dim->get(2));
	in->set_chunk(1);
	cjpeg.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cjpeg);
	jpeg_stdio_dest(&cjpeg,f);
	cjpeg.image_width = in->dim->get(1);
	cjpeg.image_height = in->dim->get(0);
	cjpeg.input_components = 3;
	cjpeg.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cjpeg);
	jpeg_set_quality(&cjpeg,quality,false);
	jpeg_start_compress(&cjpeg,TRUE);
} GRID_FLOW {
	int rowsize = in->dim->get(1)*in->dim->get(2);
	int rowsize2 = in->dim->get(1)*3;
	uint8 row[rowsize2];
	uint8 *rows[1] = {row};
	while (n) {
		bit_packing->pack(in->dim->get(1),data,row);
		jpeg_write_scanlines(&cjpeg,rows,1);
		n-=rowsize; data+=rowsize;
	}
} GRID_FINISH {
	jpeg_finish_compress(&cjpeg);
	jpeg_destroy_compress(&cjpeg);
} GRID_END

static bool gfeof(FILE *f) {
	off_t cur,end;
	cur = ftell(f);
	fseek(f,0,SEEK_END);
	end = ftell(f);
	fseek(f,cur,SEEK_SET);
	return cur==end;
}

\def 0 bang () {
	//off_t off = ftell(f);
	//fseek(f,off,SEEK_SET);
	if (gfeof(f)) {outlet_bang(bself->te_outlet); return;}
	djpeg.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&djpeg);
	jpeg_stdio_src(&djpeg,f);
	jpeg_read_header(&djpeg,TRUE);
	int sx=djpeg.image_width, sy=djpeg.image_height, chans=djpeg.num_components;
	GridOutlet out(this,0,new Dim(sy,sx,chans),cast);
	jpeg_start_decompress(&djpeg);
	uint8 row[sx*chans];
	uint8 *rows[1] = { row };
	for (int n=0; n<sy; n++) {
		jpeg_read_scanlines(&djpeg,rows,1);
		out.send(sx*chans,row);
	}
	jpeg_finish_decompress(&djpeg);
	jpeg_destroy_decompress(&djpeg);
}

\def 0 quality (short quality) {this->quality = min(max((int)quality,0),100);}

\classinfo {install_format("#io.jpeg",6,"jpeg jpg");}
\end class FormatJPEG
void startup_jpeg () {
	\startall
}
