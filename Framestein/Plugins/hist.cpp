//
// build an array where x=color intensity and y=number of colors with that intensity.
// use with fs.hist.pd.
//
// array is passed in a file, a better solution would be to use vframe...
//

#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include "plugin.h"
#include "pixels.h"

INFO("helper for fs.hist")

void perform_effect(_frame f, _args a)
{
	if(!a.s) return;

	// parameters: <color> <filename for output> <pd receivename to bang>
	// get gol
	byte col = atoi(a.s);

	// get filename
	char *t;
	if(!(t = strstr(a.s, " "))) return;
	char *filename = t+1;

	// get receivename
	if(!(t = strstr(t+1, " "))) return;
	char *retname = t+1;
	t[0]=0; // tell filename and receivename apart

	pixels p(f);
	signed short hist[256]; // 16bits, for read16 on pd

// count colors from zero up
	memset(hist, 0, sizeof(hist));
// or count from -2^15 (bottom of graph)?
//	short i, v=1<<15;
//	for(i=0; i<256; hist[i++]=v) ;

	while(!p.eof())
	{
		++hist[col==0 ? p.red() : col==1 ? p.green() : col==2 ? p.blue() : 0];
		p.next();
	}

	ofstream of(filename, ios::out|ios::binary);
	of.write((char *)&hist, sizeof(hist));
	of.close();

	sprintf(a.ret, "%s=1", retname); // bang to indicate we're done
}
