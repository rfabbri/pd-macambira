//
// merge_additive - add color components of two images, until white
//

#include "plugin.h"
#include "pixels.h"

INFO("add color components of two images until white");

void perform_copy(_frame f1, _frame f2, _args a)
{
	pixels p1(f1), p2(f2);
	int r, g, b;

	while(!p1.eof() && !p2.eof())
	{
		r = (p1.red() + p2.red());
		g = (p1.green() + p2.green());
		b = (p1.blue() + p2.blue());

		r = r<256 ? r : 255;
		g = g<256 ? g : 255;
		b = b<256 ? b : 255;

		p2.putrgb(r, g, b);

		p1.next();
		p2.next();
	}
}
