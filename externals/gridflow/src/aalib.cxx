/*
	$Id: aalib.c 4620 2009-11-01 21:16:58Z matju $

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

#include "gridflow.hxx.fcs"
#define aa_hardwareparams aa_hardware_params
#include <aalib.h>
#include <map>

/* MINNOR is a typo in aalib.h, sorry */
typedef
#if AA_LIB_MINNOR == 2
      int
#else
      enum aa_attribute
#endif
AAAttr;

static std::map<string,const aa_driver *> drivers;

\class FormatAALib : Format {
	aa_context *context;
	aa_renderparams *rparams;
	\attr bool autodraw;
	bool raw_mode;
	/* !@#$ varargs missing here */
	\constructor (t_symbol *mode, string target) {
		context=0; autodraw=1;
		argc-=2; argv+=2;
		char *argv2[argc];
		for (int i=0; i<argc; i++) argv2[i] = strdup(string(argv[i]).data());
		if (mode!=gensym("out")) RAISE("write-only, sorry");
		aa_parseoptions(0,0,&argc,argv2);
		for (int i=0; i<argc; i++) free(argv2[i]);
		if (drivers.find(target)==drivers.end()) RAISE("unknown aalib driver '%s'",target.data());
		const aa_driver *driver = drivers[target];
		context = aa_init(driver,&aa_defparams,0);
		rparams = aa_getrenderparams();
		if (!context) RAISE("opening aalib didn't work");
		int32 v[]={context->imgheight,context->imgwidth,1};
		post("aalib image size: %s",(new Dim(3,v))->to_s());
	}
	~FormatAALib () {if (context) aa_close(context);}
	\decl 0 hidecursor ();
	\decl 0 print (int y, int x, int a, string text);
	\decl 0 draw ();
	\decl 0 dump ();
	\grin 0 int
};

GRID_INLET(0) {
	if (!context) RAISE("boo");
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	switch (in->dim->get(2)) {
	case 1: raw_mode = false; break;
	case 2: raw_mode = true; break;
	default:
		RAISE("expecting 1 greyscale channel (got %d)",in->dim->get(2));
	}
	in->set_chunk(1);
} GRID_FLOW {
	int f = in->dim->prod(1);
	if (raw_mode) {
		int sx = min(f,aa_scrwidth(context));
		int y = dex/f;
		while (n) {
			if (y>=aa_scrheight(context)) return;
			for (int x=0; x<sx; x++) {
				context->textbuffer[y*aa_scrwidth(context)+x]=data[x*2+0];
				context->attrbuffer[y*aa_scrwidth(context)+x]=data[x*2+1];
			}
			y++;
			n-=f;
			data+=f;
		}
	} else {
		int sx = min(f,context->imgwidth);
		int y = dex/f;
		while (n) {
			if (y>=context->imgheight) return;
			for (int x=0; x<sx; x++) aa_putpixel(context,x,y,data[x]);
			y++;
			n-=f;
			data+=f;
		}
	}
} GRID_FINISH {
	if (!raw_mode) {
		aa_palette pal;
		for (int i=0; i<256; i++) aa_setpalette(pal,i,i,i,i);
		aa_renderpalette(context,pal,rparams,0,0,
			aa_scrwidth(context),aa_scrheight(context));
	}
	if (autodraw==1) aa_flush(context);
} GRID_END

\def 0 hidecursor () { aa_hidemouse(context); }
\def 0 draw () { aa_flush(context); }
\def 0 print (int y, int x, int a, string text) {
	aa_puts(context,x,y,(AAAttr)a,(char *)text.data());
	if (autodraw==1) aa_flush(context);
}

\def 0 dump () {
	int32 v[] = {aa_scrheight(context), aa_scrwidth(context), 2};
	GridOutlet out(this,0,new Dim(3,v));
	for (int y=0; y<aa_scrheight(context); y++) {
		for (int x=0; x<aa_scrwidth(context); x++) {
			int32 data[2];
			data[0] = context->textbuffer[y*aa_scrwidth(context)+x];
			data[1] = context->attrbuffer[y*aa_scrwidth(context)+x];
			out.send(2,data);
		}
	}
}

\end class FormatAALib {
	const aa_driver *const *p = aa_drivers;
	for (; *p; p++) drivers[(*p)->shortname] = *p;
	install_format("#io.aalib",2,"");
}
void startup_aalib () {
	\startall
}
