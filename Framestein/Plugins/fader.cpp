//
// fader - fade image according to brightness of another image
//

#include "plugin.h"
#include "pixels.h"

INFO("fade image according to brightness of another image");

void perform_copy(_frame f1, _frame f2, _args a)
{
	pixels p1(f1), p2(f2);
	byte r, g, b;

	while(!p1.eof() && !p2.eof())
	{
		r = p1.red() * (float)(p2.red() / 255.0);
		g = p1.green() * (float)(p2.green() / 255.0);
		b = p1.blue() * (float)(p2.blue() / 255.0);

		p2.putrgb(r, g, b);

		p1.next();
		p2.next();
	}
}
