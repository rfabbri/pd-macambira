//
// send the name of the plugin to fs.frame, and fs will call
// perform_effect(struct frame f, struct args a) in your dll.
//
// send it to the right inlet of fs.copy, and
// perform_copy(struct frame f1, struct frame f2, struct args a)
// will be used as a copy operation.
//

#ifndef _PLUGINH
#define _PLUGINH

typedef unsigned char byte;
#define pixel8 byte
#define pixel16 unsigned short
#define pixel24 struct pixel24data
#define pixel32 unsigned long
#define _frame struct frame
#define _args struct args

struct pixel24data { byte b; byte g; byte r; };

struct frame
{
	byte *bits;		// pixel data
	int lpitch;		// used to get row position in bits, see scanline below
				// this is not always width*(pixelformat/8), you tell me why
	int width;
	int height;
	int pixelformat;	// pixelformat is bitcount of your screen, 8, 16, 24 or 32.
};

struct args
{
	char *s;		// effect/copy arguments in a string
	char *ret;		// return values. data given in the form
				// "pd_receive_name=value;..." will be sent back to Pd.
				// memory allocated: 256 characters.
};

// 8-bit pointer to row y
__inline byte *scanline(struct frame f, int y)	{ return &f.bits[y*f.lpitch]; }

// pointer to 16 bit pixels
__inline pixel16 *scanline16(struct frame f, int y) { return (pixel16 *)scanline(f, y); }

// pointer to 24 bit pixels
__inline pixel24 *scanline24(struct frame f, int y) { return (pixel24 *)scanline(f, y); }

// pointer to 32 bit pixels
__inline pixel32 *scanline32(struct frame f, int y) { return (pixel32 *)scanline(f, y); }

__inline byte r16(pixel16 color)
{
	return (color >> 11) << 3;
}

__inline byte g16(pixel16 color)
{
	return ((color & 2016) >> 5) << 2;
}

__inline byte b16(pixel16 color)
{
	return (color & 31) << 3;
}

__inline byte r24(pixel24 color)
{
	return color.r;
}

__inline byte g24(pixel24 color)
{
	return color.g;
}

__inline byte b24(pixel24 color)
{
	return color.b;
}

__inline byte r32(pixel32 color)
{
	return (byte)color;
}

__inline byte g32(pixel32 color)
{
	return (byte)(((pixel16)color) >> 8);
}

__inline byte b32(pixel32 color)
{
	return (byte)(color >> 16);
}

__inline pixel16 rgbtocolor16(byte r, byte g, byte b)
{
	return 	((r >> 3) << 11) |	// r value shifted
		((g >> 2) << 5) |	// g value shifted
		(b >> 3);		// add blue
}

__inline pixel24 rgbtocolor24(byte r, byte g, byte b)
{
	pixel24 p;
	p.r = r;
	p.g = g;
	p.b = b;
	return p;
}

__inline pixel32 rgbtocolor32(byte r, byte g, byte b)
{
	return (b << 16) | (g << 8) | r;
}

// restrict input to be 0..255 <olaf.matthes@gmx.de>
__inline byte klamp255(long in)
{
	byte out = in<0 ? 0 : in;
	out = 255<out ? 255 : out;
	return(out);
}

// restrict input to be 16..235 as it is usual with signals
// conforming to ITU-R 601     <olaf.matthes@gmx.de>
__inline byte klamp601(long in)
{
	byte out = in<16 ? 16 : in;
	out = 235<out ? 235 : out;
	return(out);
}

#endif // #ifndef _PLUGINH
