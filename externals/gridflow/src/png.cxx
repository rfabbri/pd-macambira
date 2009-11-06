/*
	$Id: png.c 4620 2009-11-01 21:16:58Z matju $

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

/* !@#$ not handling abort on compress */
/* !@#$ not handling abort on decompress */

#include "gridflow.hxx.fcs"
extern "C" {
#include <libpng12/png.h>
};

\class FormatPNG : Format {
	P<BitPacking> bit_packing;
	png_structp png;
	png_infop info;
	\constructor (t_symbol *mode, string filename) {
		Format::_0_open(0,0,mode,filename);
		uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
		bit_packing = new BitPacking(is_le(),3,3,mask);
	}
	\decl 0 bang ();
	\grin 0 int
};

GRID_INLET(0) {
	if (in->dim->n!=3)      RAISE("expecting 3 dimensions: rows,columns,channels");
	int c = in->dim->get(2);
	if (c!=3)               RAISE("expecting 3 channels (got %d)",in->dim->get(2));
	in->set_chunk(0);
} GRID_FLOW {
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) RAISE("!png");
	info = png_create_info_struct(png);
	if (!info) {if (png) png_destroy_write_struct(&png, NULL); RAISE("!info");}
	if (setjmp(png_jmpbuf(png))) {png_destroy_write_struct(&png, &info);     RAISE("png write error");}
  if (setjmp(png->jmpbuf)) {png_write_destroy(png); free(png); free(info); RAISE("png write error");}
  png_init_io(png, f);
  info->width  = in->dim->get(1);
  info->height = in->dim->get(0);
  info->bit_depth = 8;
//  info->color_type = channels==3 ?  PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_GRAY;
  info->color_type = PNG_COLOR_TYPE_RGB;
//    info->color_type |= PNG_COLOR_MASK_ALPHA;
  info->interlace_type = 1;
  png_write_info(png,info);
	png_set_packing(png);
// this would have been the GRID_FLOW section
  int rowsize = in->dim->get(1)*in->dim->get(2);
	int rowsize2 = in->dim->get(1)*3;
	uint8 row[rowsize2];
	while (n) {
	  post("n=%ld",long(n));
		bit_packing->pack(in->dim->get(1),data,row);
		png_write_row(png,row);
		n-=rowsize; data+=rowsize;
	}
// this would have been the GRID_FINISH section
  post("GRID FINISH 1");
  png_write_end(png,info);
  post("GRID FINISH 2");
  png_write_destroy(png);
  post("GRID FINISH 3");
	fflush(f);
  free(png);
  free(info);
	fclose(f);
} GRID_FINISH {
} GRID_END

\def 0 bang () {
	uint8 sig[8];
	if (!fread(sig, 1, 8, f)) {outlet_bang(bself->te_outlet); return;}
	if (!png_check_sig(sig, 8)) RAISE("bad signature");
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) RAISE("!png");
	info = png_create_info_struct(png);
	if (!info) {png_destroy_read_struct(&png, NULL, NULL); RAISE("!info");}
	if (setjmp(png_jmpbuf(png))) {png_destroy_read_struct(&png, &info, NULL); RAISE("png read error");}
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
	uint8 *image_data = new uint8[rowbytes*height];
	row_pointers = new png_bytep[height];
	//gfpost("png: color_type=%d channels=%d, width=%d, rowbytes=%ld, height=%ld, gamma=%f",
	//	color_type, channels, width, rowbytes, height, gamma);
	for (int i=0; i<(int)height; i++) row_pointers[i] = image_data + i*rowbytes;
	if ((uint32)rowbytes != width*channels)
		RAISE("rowbytes mismatch: %d is not %d*%d=%d", rowbytes, width, channels, width*channels);
	png_read_image(png, row_pointers);
	delete[] row_pointers;
	row_pointers = 0;
	png_read_end(png, 0);
	GridOutlet out(this,0,new Dim(height, width, channels), cast);
	out.send(rowbytes*height,image_data);
	delete[] image_data;
	png_destroy_read_struct(&png, &info, NULL);
}

\classinfo {install_format("#io.png",4,"png");}
\end class FormatPNG
void startup_png () {
	\startall
}
