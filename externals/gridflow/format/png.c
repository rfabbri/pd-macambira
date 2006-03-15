/*
	$Id: png.c,v 1.2 2006-03-15 04:37:46 matju Exp $

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

/* !@#$ not handling abort on compress */
/* !@#$ not handling abort on decompress */

#include "../base/grid.h.fcs"
extern "C" {
#include <libpng12/png.h>
};

\class FormatPNG < Format
struct FormatPNG : Format {
	P<BitPacking> bit_packing;
	png_structp png;
	png_infop info;
	int fd;
	FILE *f;
	FormatPNG () : bit_packing(0), png(0), f(0) {}
	\decl Ruby frame ();
	\decl void initialize (Symbol mode, Symbol source, String filename);
	\grin 0 int
};

GRID_INLET(FormatPNG,0) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels (got %d)",in->dim->get(2));
	in->set_factor(in->dim->get(1)*in->dim->get(2));
	RAISE("bother, said pooh, as the PNG encoding was found unimplemented");
} GRID_FLOW {
	int rowsize = in->dim->get(1)*in->dim->get(2);
	int rowsize2 = in->dim->get(1)*3;
	uint8 row[rowsize2];
	while (n) {
		bit_packing->pack(in->dim->get(1),data,Pt<uint8>(row,rowsize));
		n-=rowsize; data+=rowsize;
	}
} GRID_FINISH {
} GRID_END

\def Ruby frame () {
	uint8 sig[8];
	if (!fread(sig, 1, 8, f)) return Qfalse;
	if (!png_check_sig(sig, 8)) RAISE("bad signature");
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) RAISE("!png");
	info = png_create_info_struct(png);
	if (!info) {
		png_destroy_read_struct(&png, NULL, NULL); RAISE("!info");
	}
	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &info, NULL); RAISE("png read error");
	}
	png_init_io(png, f);
	png_set_sig_bytes(png, 8);  // we already read the 8 signature bytes
	png_read_info(png, info);   // read all PNG info up to image data
	png_uint_32 width, height;
	int bit_depth, color_type;
	png_get_IHDR(png, info, &width, &height, &bit_depth, &color_type, 0,0,0);

	png_bytepp row_pointers = 0;
	if (color_type == PNG_COLOR_TYPE_PALETTE
		|| (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		|| png_get_valid(png, info, PNG_INFO_tRNS))
			png_set_expand(png);
	// 16bpp y, 32bpp ya, 48bpp rgb, 64bpp rgba...
	if (bit_depth == 16) png_set_strip_16(png);

	double display_gamma = 2.2;
	double gamma;
	if (png_get_gAMA(png, info, &gamma))
		png_set_gamma(png, display_gamma, gamma);
	png_read_update_info(png, info);

	int rowbytes = png_get_rowbytes(png, info);
	int channels = (int)png_get_channels(png, info);
	Pt<uint8> image_data = ARRAY_NEW(uint8,rowbytes*height);
	row_pointers = new png_bytep[height];
	//gfpost("png: color_type=%d channels=%d, width=%d, rowbytes=%ld, height=%ld, gamma=%f",
	//	color_type, channels, width, rowbytes, height, gamma);
	for (int i=0; i<(int)height; i++) row_pointers[i] = image_data + i*rowbytes;
	if ((uint32)rowbytes != width*channels)
		RAISE("rowbytes mismatch: %d is not %d*%d=%d",
			rowbytes, width, channels, width*channels);
	png_read_image(png, row_pointers);
	delete row_pointers;
	row_pointers = 0;
	png_read_end(png, 0);

	GridOutlet out(this,0,new Dim(height, width, channels),
		NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
	out.send(rowbytes*height,image_data);
	free(image_data);
	png_destroy_read_struct(&png, &info, NULL);
	return Qnil;
}

\def void initialize (Symbol mode, Symbol source, String filename) {
	rb_call_super(argc,argv);
	if (source!=SYM(file)) RAISE("usage: png file <filename>");
	rb_funcall(rself,SI(raw_open),3,mode,source,filename);
	Ruby stream = rb_ivar_get(rself,SI(@stream));
	fd = NUM2INT(rb_funcall(stream,SI(fileno),0));
	f = fdopen(fd,mode==SYM(in)?"r":"w");
	uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
	bit_packing = new BitPacking(is_le(),3,3,mask);
}

\classinfo {
	IEVAL(rself,
	"install '#io:png',1,1;@mode=4;include GridFlow::EventIO; suffixes_are'png'");
}
\end class FormatPNG
void startup_png () {
	\startall
}
