// Easy iteration thru pixels, see green.cpp and tile.cpp for examples
// NOTE: using this class could be a bit slower than manipulating the bits directly..
//
// changes to 0.20
// - pixelformat-aware functions red() green() blue() and putrgb()
// - dot16() dot24() and dot32() added
// - get/put dot() removed, use above

#ifndef _PIXELSH
#define _PIXELSH

#include <vector>
#include "plugin.h"

class pixels
{
private:
	_frame m_f;
	pixel8 *p8;	// pointer to a row of 8-bit pixels
	pixel16 *p16;
	pixel24 *p24;
	pixel32 *p32;

	__inline void updaterowp();
public:
	int x, y;

	pixels(const _frame &f)
	{
		m_f = f;
		x = -1;
		y = 0;
		p8 = 0;
		next();
	}

	__inline pixel8 dot8() { return p8[x]; }
	__inline pixel16 dot16() { return p16[x]; }
	__inline pixel24 dot24() { return p24[x]; }
	__inline pixel32 dot32() { return p32[x]; }

	__inline void dot8(pixel8 c) { p8[x] = c; }
	__inline void dot16(pixel16 c) { p16[x] = c; }
	__inline void dot24(pixel24 c) { p24[x] = c; }
	__inline void dot32(pixel32 c) { p32[x] = c; }

	__inline byte red();
	__inline byte green();
	__inline byte blue();

	__inline void putrgb(byte r, byte g, byte b);

	__inline void moveto(int tox, int toy)
	 { if(tox<m_f.width) x=tox; if(toy<m_f.height) { y=toy; updaterowp(); } }

	__inline int eof() { return y==m_f.height ? 1 : 0; }
	void next();

	__inline int width() { return m_f.width; }
	__inline int height() { return m_f.height; }
};

byte pixels::red()
{
	switch(m_f.pixelformat)
	{
		case 16: return r16(p16[x]);
		case 24: return r24(p24[x]);
		case 32: return r32(p32[x]);
		default: return 0;
	}
}

byte pixels::green()
{
	switch(m_f.pixelformat)
	{
		case 16: return g16(p16[x]);
		case 24: return g24(p24[x]);
		case 32: return g32(p32[x]);
		default: return 0;
	}
}

byte pixels::blue()
{
	switch(m_f.pixelformat)
	{
		case 16: return b16(p16[x]);
		case 24: return b24(p24[x]);
		case 32: return b32(p32[x]);
		default: return 0;
	}
}

void pixels::putrgb(byte r, byte g, byte b)
{
	switch(m_f.pixelformat)
	{
		case 16: p16[x] = rgbtocolor16(r, g, b); break;
		case 24: p24[x] = rgbtocolor24(r, g, b); break;
		case 32: p32[x] = rgbtocolor32(r, g, b); break;
	}
}

void pixels::next()
{
	x++;
	if(x==m_f.width) { y++; x=0; p8=0; }
	if(!p8) updaterowp();
}

void pixels::updaterowp()
{
	p8 = scanline(m_f, y);
	p16 = (pixel16 *)p8;
	p24 = (pixel24 *)p8;
	p32 = (pixel32 *)p8;
}

//
// arguments-class, to make it easier to parse parameters.
// using vector<char *> for this might be a bit overkill (??),
// but I like the ease of it.

class arguments
{
private:
	std::vector<char *> m_ptrs;
public:
	arguments(char *s)
	{
		char *t = s;
		if(!t || !t[0]) return;
		if(t[0]) m_ptrs.push_back(t);
		while(t = strstr(t+1, " "))
		{
			t[0]=0;
			m_ptrs.push_back(t+1);
		}
	}
	char *operator[](int i) { return m_ptrs[i]; }
	int count() { return m_ptrs.size(); }
};

#endif // #ifndef _PIXELSH
