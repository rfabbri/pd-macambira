#include <string.h>
#include "plugin.h"

INFO("helper for fs.rgb")

void perform_effect(_frame f, _args a)
{
	char *t;
	short x, y;
	pixel16 c16;
	pixel24 c24;
	pixel32 c32;
	byte r, g, b;
	char *ret_r, *ret_g, *ret_b;

	if(!a.s) return;

	// get x and y params
	x = atoi(a.s);
	if(!(t = strstr(a.s, " "))) return;
	y = atoi(t+1);

	// get returnvaluereceivenames
	if(!(t = strstr(t+1, " "))) return;
	ret_r = t+1;
	if(!(t = strstr(t+1, " "))) return;
	ret_g = t+1;
	t[0]=0;
	if(!(t = strstr(t+1, " "))) return;
	ret_b = t+1;
	t[0]=0;

	if(x<0||x>=f.width) return;
	if(y<0||y>=f.height) return;

	switch(f.pixelformat)
	{
		case 16:
			c16 = scanline16(f, y)[x];
			r = r16(c16);
			g = g16(c16);
			b = b16(c16);
			break;
		case 24:
			c24 = scanline24(f, y)[x];
			r = r24(c24);
			g = g24(c24);
			b = b24(c24);
			break;
		case 32:
			c32 = scanline32(f, y)[x];
			r = r32(c32);
			g = g32(c32);
			b = b32(c32);
			break;
	}

	// return-values:
	//
	// framestein will send data given in the form "pd_receiver_name=value"
	// back to pd.

	sprintf(a.ret, "%s=%d;%s=%d;%s=%d", ret_b, b, ret_g, g, ret_r, r);
}
