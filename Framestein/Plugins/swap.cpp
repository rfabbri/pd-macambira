#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "plugin.h"
#include "pixels.h"

// swap selected area on two images
// args: swap sourcex1 sourcey1 sourcex2 sourcey2 destx desty
// (the selection is of equal size on both images)

void swapint(int *a, int *b)
{
	int i = *b;
	*b = *a;
	*a = i;
}

void perform_copy(_frame f1, _frame f2, _args a)
{
	if(!a.s) return;

	// get args
	int sx1, sy1, sx2, sy2, dx1, dy1;
	char *t;

	sx1 = atoi(a.s);
	if(!(t = strstr(a.s, " "))) return;
	sy1 = atoi(t);
	if(!(t = strstr(t+1, " "))) return;
	sx2 = atoi(t);
	if(!(t = strstr(t+1, " "))) return;
	sy2 = atoi(t);
	if(!(t = strstr(t+1, " "))) return;
	dx1 = atoi(t);
	if(!(t = strstr(t+1, " "))) return;
	dy1 = atoi(t);

	if(sx1>sx2) swapint(&sx1, &sx2);
	if(sy1>sy2) swapint(&sy1, &sy2);

printf("swap: %d %d %d %d - %d %d\n", sx1, sy1, sx2, sy2, dx1, dy1);

	int x, y, i, o;
	pixel16 c16;
	pixels p1(f1), p2(f2);

	for(y=sy1; y<=sy2; y++)
		for(x=sx1; x<=sx2; x++)
		{
			if(x>=f1.width || y>=f1.height) continue;
			i = dx1+(x-sx1);
			o = dy1+(y-sy1);
			if(i>=f2.width || o>=f2.height) continue;

			p1.moveto(x, y);
			p2.moveto(i, o);

			c16 = p2.dot16();
			p2.dot16(p1.dot16());
			p1.dot16(c16);
		}
}
