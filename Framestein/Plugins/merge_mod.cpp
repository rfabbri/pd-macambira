//
// merge_additive - add color components of two images, mod 255
//

#include "plugin.h"
#include "pixels.h"

INFO("add color components of two images, modulus 255");

void perform_copy(_frame f1, _frame f2, _args a)
{
	arguments ar(a.s);
	pixels p1(f1), p2(f2);
	int r, g, b;

	float f = ar.count()>=1 ? atof(ar[0]) : 1;

	while(!p1.eof() && !p2.eof())
	{
		r = (f * p1.red() + p2.red());
		g = (f * p1.green() + p2.green());
		b = (f * p1.blue() + p2.blue());

		r = r % 255;
		g = g % 255;
		b = b % 255;

		p2.putrgb(r, g, b);

		p1.next();
		p2.next();
	}
}
