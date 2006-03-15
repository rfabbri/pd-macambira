/*
	$Id: aalib.c,v 1.2 2006-03-15 04:37:46 matju Exp $

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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

#include "../base/grid.h.fcs"
#define aa_hardwareparams aa_hardware_params
#include <aalib.h>

/* MINNOR is a typo in aalib.h, sorry */
typedef 
#if AA_LIB_MINNOR == 2
      int
#else
      enum aa_attribute
#endif
AAAttr;

\class FormatAALib < Format
struct FormatAALib : Format {
	aa_context *context;
	aa_renderparams *rparams;
	int autodraw; /* as for X11 */
	bool raw_mode;

	FormatAALib () : context(0), autodraw(1) {}

	\decl void initialize (Symbol mode, Symbol target);
	\decl void close ();
	\decl void _0_hidecursor ();
	\decl void _0_print (int y, int x, int a, Symbol text);
	\decl void _0_draw ();
	\decl void _0_autodraw (int autodraw);
	\decl void _0_dump ();
	\grin 0 int
};

GRID_INLET(FormatAALib,0) {
	if (!context) RAISE("boo");
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	switch (in->dim->get(2)) {
	case 1: raw_mode = false; break;
	case 2: raw_mode = true; break;
	default:
		RAISE("expecting 1 greyscale channel (got %d)",in->dim->get(2));
	}
	in->set_factor(in->dim->get(1)*in->dim->get(2));
} GRID_FLOW {
	int f = in->factor();
	if (raw_mode) {
		int sx = min(f,aa_scrwidth(context));
		int y = in->dex/f;
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
		int y = in->dex/f;
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

\def void close () {
	if (context) {
		aa_close(context);
		context=0;
	}
}

\def void _0_hidecursor () { aa_hidemouse(context); }
\def void _0_draw () { aa_flush(context); }
\def void _0_print (int y, int x, int a, Symbol text) {
	aa_puts(context,x,y,(AAAttr)a,(char *)rb_sym_name(text));
	if (autodraw==1) aa_flush(context);
}
\def void _0_autodraw (int autodraw) {
	if (autodraw<0 || autodraw>1)
		RAISE("autodraw=%d is out of range",autodraw);
	this->autodraw = autodraw;
}
\def void _0_dump () {
	int32 v[] = {aa_scrheight(context), aa_scrwidth(context), 2};
	GridOutlet out(this,0,new Dim(3,v));
	for (int y=0; y<aa_scrheight(context); y++) {
		for (int x=0; x<aa_scrwidth(context); x++) {
			STACK_ARRAY(int32,data,2);
			data[0] = context->textbuffer[y*aa_scrwidth(context)+x];
			data[1] = context->attrbuffer[y*aa_scrwidth(context)+x];
			out.send(2,data);
		}
	}		
}

/* !@#$ varargs missing here */
\def void initialize (Symbol mode, Symbol target) {
	rb_call_super(argc,argv);
	argc-=2; argv+=2;
	char *argv2[argc];
	for (int i=0; i<argc; i++)
		argv2[i] = strdup(rb_str_ptr(rb_funcall(argv[i],SI(to_s),0)));
	if (mode!=SYM(out)) RAISE("write-only, sorry");
	aa_parseoptions(0,0,&argc,argv2);
	for (int i=0; i<argc; i++) free(argv2[i]);
	Ruby drivers = rb_ivar_get(rb_obj_class(rself),SI(@drivers));
	Ruby driver_address = rb_hash_aref(drivers,target);
	if (driver_address==Qnil)
		RAISE("unknown aalib driver '%s'",rb_sym_name(target));
	aa_driver *driver = FIX2PTR(aa_driver,driver_address);
	context = aa_init(driver,&aa_defparams,0);
	rparams = aa_getrenderparams();
	if (!context) RAISE("opening aalib didn't work");
	int32 v[]={context->imgheight,context->imgwidth,1};
	gfpost("aalib image size: %s",(new Dim(3,v))->to_s());
}

\classinfo {
	Ruby drivers = rb_ivar_set(rself,SI(@drivers),rb_hash_new());
	const aa_driver *const *p = aa_drivers;
	for (; *p; p++) {
		rb_hash_aset(drivers,ID2SYM(rb_intern((*p)->shortname)), PTR2FIX(*p));
	}
// IEVAL(rself,"GridFlow.post('aalib supports: %s', @drivers.keys.join(', '))");
	IEVAL(rself,"install '#in:aalib',1,1;@flags=2;@comment='Ascii Art Library'");
}
\end class FormatAALib
void startup_aalib () {
	\startall
}
