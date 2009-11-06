/*
	$Id: flow_objects.c 4097 2008-10-03 19:49:03Z matju $

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
#ifdef DESIRE
#include "desire.h"
#else
extern "C" {
#include "bundled/g_canvas.h"
#include "bundled/m_imp.h"
extern t_class *text_class;
};
#endif
#include <algorithm>
#include <errno.h>
#include <sys/time.h>
#include <string>

typedef int (*comparator_t)(const void *, const void *);

#ifndef DESIREDATA
struct _outconnect {
    struct _outconnect *next;
    t_pd *to;
};
struct _outlet {
    t_object *owner;
    struct _outlet *next;
    t_outconnect *connections;
    t_symbol *sym;
};
#endif

//****************************************************************

struct ArgSpec {
	t_symbol *name;
	t_symbol *type;
	t_atom defaultv;
};

\class Args : FObject {
	ArgSpec *sargv;
	int sargc;
	\constructor (...) {
		sargc = argc;
		sargv = new ArgSpec[argc];
		for (int i=0; i<argc; i++) {
			if (argv[i].a_type==A_LIST) {
				t_binbuf *b = (t_binbuf *)argv[i].a_gpointer;
				int bac = binbuf_getnatom(b);
				t_atom *bat = binbuf_getvec(b);
				sargv[i].name = atom_getsymbolarg(0,bac,bat);
				sargv[i].type = atom_getsymbolarg(1,bac,bat);
				if (bac<3) SETNULL(&sargv[i].defaultv); else sargv[i].defaultv = bat[2];
			} else if (argv[i].a_type==A_SYMBOL) {
				sargv[i].name = argv[i].a_symbol;
				sargv[i].type = gensym("a");
				SETNULL(&sargv[i].defaultv);
			} else RAISE("expected symbol or nested list");
		}
		bself->noutlets_set(sargc);
	}
	~Args () {delete[] sargv;}
	\decl 0 bang ();
	\decl 0 loadbang ();
	void process_args (int argc, t_atom *argv);
};
/* get the owner of the result of canvas_getenv */
static t_canvas *canvas_getabstop(t_canvas *x) {
    while (!x->gl_env) if (!(x = x->gl_owner)) bug("t_canvasenvironment", x);
    return x;
} 
\def 0 bang () {post("%s shouldn't bang [args] anymore.",canvas_getabstop(bself->mom)->gl_name->s_name);}
void outlet_anything2 (t_outlet *o, int argc, t_atom *argv) {
	if (!argc) outlet_bang(o);
	else if (argv[0].a_type==A_SYMBOL) outlet_anything(o,argv[0].a_symbol,argc-1,argv+1);
	else if (argv[0].a_type==A_FLOAT && argc==1) outlet_float(o,argv[0].a_float);
	else outlet_anything(o,&s_list,argc,argv);
}
void pd_anything2 (t_pd *o, int argc, t_atom *argv) {
	if (!argc) pd_bang(o);
	else if (argv[0].a_type==A_SYMBOL) pd_typedmess(o,argv[0].a_symbol,argc-1,argv+1);
	else if (argv[0].a_type==A_FLOAT && argc==1) pd_float(o,argv[0].a_float);
	else pd_typedmess(o,&s_list,argc,argv);
}
\def 0 loadbang () {
	t_canvasenvironment *env = canvas_getenv(bself->mom);
	int ac = env->ce_argc;
	t_atom av[ac];
	for (int i=0; i<ac; i++) av[i] = env->ce_argv[i];
	//ac = handle_braces(ac,av);
	t_symbol *comma = gensym(",");
	int j;
	for (j=0; j<ac; j++) if (av[j].a_type==A_SYMBOL && av[j].a_symbol==comma) break;
	int jj = handle_braces(j,av);
	process_args(jj,av);
	while (j<ac) {
		j++;
		int k=j;
		for (; j<ac; j++) if (av[j].a_type==A_SYMBOL && av[j].a_symbol==comma) break;
		//outlet_anything2(bself->outlets[sargc],j-k,av+k);
		t_text *t = (t_text *)canvas_getabstop(bself->mom);
		if (!t->te_inlet) RAISE("can't send init-messages, because object has no [inlet]");
		pd_anything2((t_pd *)t->te_inlet,j-k,av+k);
	}
}
void Args::process_args (int argc, t_atom *argv) {
	t_canvas *canvas = canvas_getrootfor(bself->mom);
	t_symbol *wildcard = gensym("*");
	for (int i=sargc-1; i>=0; i--) {
		t_atom *v;
		if (i>=argc) {
			if (sargv[i].defaultv.a_type != A_NULL) {
				v = &sargv[i].defaultv;
			} else if (sargv[i].name!=wildcard) {
				pd_error(canvas,"missing argument $%d named \"%s\"", i+1,sargv[i].name->s_name);
				continue;
			}
		} else v = &argv[i];
		if (sargv[i].name==wildcard) {
			if (argc-i>0) outlet_list(bself->outlets[i],&s_list,argc-i,argv+i);
			else outlet_bang(bself->outlets[i]);
		} else {
			if (v->a_type==A_LIST) {
				t_binbuf *b = (t_binbuf *)v->a_gpointer;
				outlet_list(bself->outlets[i],&s_list,binbuf_getnatom(b),binbuf_getvec(b));
			} else if (v->a_type==A_SYMBOL) outlet_symbol(bself->outlets[i],v->a_symbol);
			else outlet_anything2(bself->outlets[i],1,v);
		}
	}
	if (argc>sargc && sargv[sargc-1].name!=wildcard) pd_error(canvas,"warning: too many args (got %d, want %d)", argc, sargc);
}
\end class {install("args",1,1);}

//****************************************************************

namespace {
template <class T> void swap (T &a, T &b) {T c; c=a; a=b; b=c;}
};

\class ListReverse : FObject {
	\constructor () {}
	\decl 0 list(...);
};
\def 0 list (...) {
	for (int i=(argc-1)/2; i>=0; i--) swap(argv[i],argv[argc-i-1]);
	outlet_list(bself->te_outlet,&s_list,argc,argv);
}
\end class {install("listreverse",1,1);}

\class ListFlatten : FObject {
	std::vector<t_atom2> contents;
	\constructor () {}
	\decl 0 list(...);
	void traverse (int argc, t_atom2 *argv) {
		for (int i=0; i<argc; i++) {
			if (argv[i].a_type==A_LIST) traverse(binbuf_getnatom(argv[i]),(t_atom2 *)binbuf_getvec(argv[i]));
			else contents.push_back(argv[i]);
		}
	}
};
\def 0 list (...) {
	traverse(argc,argv);
	outlet_list(bself->te_outlet,&s_list,contents.size(),&contents[0]);
	contents.clear();

}
\end class {install("listflatten",1,1);}

// does not do recursive comparison of lists.
static bool atom_eq (t_atom &a, t_atom &b) {
	if (a.a_type!=b.a_type) return false;
	if (a.a_type==A_FLOAT)   return a.a_float   ==b.a_float;
	if (a.a_type==A_SYMBOL)  return a.a_symbol  ==b.a_symbol;
	if (a.a_type==A_POINTER) return a.a_gpointer==b.a_gpointer;
	if (a.a_type==A_LIST)    return a.a_gpointer==b.a_gpointer;
	RAISE("don't know how to compare elements of type %d",a.a_type);
}

\class ListFind : FObject {
	int ac;
	t_atom *at;
	~ListFind() {if (at) delete[] at;}
	\constructor (...) {ac=0; at=0; _1_list(argc,argv);}
	\decl 0 list(...);
	\decl 1 list(...);
	\decl 0 float(float f);
	\decl 0 symbol(t_symbol *s);
};
\def 1 list (...) {
	if (at) delete[] at;
	ac = argc;
	at = new t_atom[argc];
	for (int i=0; i<argc; i++) at[i] = argv[i];
}
\def 0 list (...) {
	if (argc<1) RAISE("empty input");
	int i=0; for (; i<ac; i++) if (atom_eq(at[i],argv[0])) break;
	outlet_float(bself->outlets[0],i==ac?-1:i);
}
\def 0 float (float f) {
	int i=0; for (; i<ac; i++) if (atom_eq(at[i],argv[0])) break;
	outlet_float(bself->outlets[0],i==ac?-1:i);
}
\def 0 symbol (t_symbol *s) {
	int i=0; for (; i<ac; i++) if (atom_eq(at[i],argv[0])) break;
	outlet_float(bself->outlets[0],i==ac?-1:i);
}
//doc:_1_list,"list to search into"
//doc:_0_float,"float to find in that list"
//doc_out:_0_float,"position of the incoming float in the stored list"
\end class {install("listfind",2,1);}

void outlet_atom (t_outlet *self, t_atom *av) {
	if (av->a_type==A_FLOAT)   outlet_float(  self,av->a_float);    else
	if (av->a_type==A_SYMBOL)  outlet_symbol( self,av->a_symbol);   else
	if (av->a_type==A_POINTER) outlet_pointer(self,av->a_gpointer); else
	outlet_list(self,gensym("list"),1,av);
}

\class ListRead : FObject { /* sounds like tabread */
	int ac;
	t_atom *at;
	~ListRead() {if (at) delete[] at;}
	\constructor (...) {ac=0; at=0; _1_list(argc,argv);}
	\decl 0 float(float f);
	\decl 1 list(...);
};
\def 0 float(float f) {
	int i = int(f);
	if (i<0) i+=ac;
	if (i<0 || i>=ac) {outlet_bang(bself->outlets[0]); return;} /* out-of-range */
	outlet_atom(bself->outlets[0],&at[i]);
}
\def 1 list (...) {
	if (at) delete[] at;
	ac = argc;
	at = new t_atom[argc];
	for (int i=0; i<argc; i++) at[i] = argv[i];
}
\end class {install("listread",2,1);}

\class Range : FObject {
	t_float *mosusses;
	int nmosusses;
	\constructor (...) {
		nmosusses = argc;
		for (int i=0; i<argc; i++) if (argv[i].a_type!=A_FLOAT) RAISE("$%d: expected float",i+1);
		mosusses = new t_float[argc];
		for (int i=0; i<argc; i++) mosusses[i]=argv[i].a_float;
		bself-> ninlets_set(1+nmosusses);
		bself->noutlets_set(1+nmosusses);
	}
	~Range () {delete[] mosusses;}
	\decl 0 float(float f);
	\decl 0 list(float f);
	\decl void _n_float(int i, float f);
};
\def 0 list(float f) {_0_float(argc,argv,f);}
\def 0 float(float f) {
	int i; for (i=0; i<nmosusses; i++) if (f<mosusses[i]) break;
	outlet_float(bself->outlets[i],f);
}
 // precedence problem in dispatcher... does this problem still exist?
\def void _n_float(int i, float f) {if (!i) _0_float(argc,argv,f); else mosusses[i-1] = f;}
\end class {install("range",1,1);}

//****************************************************************

string ssprintf(const char *fmt, ...) {
	std::ostringstream os;
	va_list va;
	va_start(va,fmt);
	voprintf(os,fmt,va);
	va_end(va);
	return os.str();
}

\class GFPrint : FObject {
	t_symbol *prefix;
	t_pd *gp;
	//t_symbol *rsym;
	\constructor (t_symbol *s=0) {
		//rsym = gensym(const_cast<char *>(ssprintf("gf.print:%08x",this).data())); // not in use atm.
		prefix=s?s:gensym("print");
		t_atom a[1];
		SETSYMBOL(a,prefix);
		pd_typedmess(&pd_objectmaker,gensym("#print"),1,a);
		gp = pd_newest();
		SETPOINTER(a,(t_gpointer *)bself);
		//pd_typedmess(gp,gensym("dest"),1,a);
	}
	~GFPrint () {
		//pd_unbind((t_pd *)bself,rsym);
		pd_free(gp);
	}
	\decl 0 grid(...);
	\decl void anything (...);
};
std::ostream &operator << (std::ostream &self, const t_atom &a) {
	switch (a.a_type) {
		case A_FLOAT:   self << a.a_float; break;
		case A_SYMBOL:  self << a.a_symbol->s_name; break; // i would rather show backslashes here...
		case A_DOLLSYM: self << a.a_symbol->s_name; break; // for real, it's the same thing as A_SYMBOL in pd >= 0.40
		case A_POINTER: self << "\\p(0x" << std::hex << a.a_gpointer << std::dec << ")"; break;
		case A_COMMA:   self << ","; break;
		case A_SEMI:    self << ";"; break;
		case A_DOLLAR:  self << "$" << a.a_w.w_index; break;
		case A_LIST: {
			t_list *b = (t_list *)a.a_gpointer;
			int argc = binbuf_getnatom(b);
			t_atom *argv = binbuf_getvec(b);
			self << "(";
			for (int i=0; i<argc; i++) self << argv[i] << " )"[i==argc-1];
			break;
		}
		default: self << "\\a(" << a.a_type << " " << std::hex << a.a_gpointer << std::dec << ")"; break;
	}
	return self;
}
\def 0 grid(...) {pd_typedmess(gp,gensym("grid"),argc,argv);}
\def void anything(...) {
	std::ostringstream text;
	text << prefix->s_name << ":";
	if (argv[0]==gensym("_0_list") && argc>=2 && argv[1].a_type==A_FLOAT) {
		// don't show the selector.
	} else if (argv[0]==gensym("_0_list") && argc==2 && argv[1].a_type==A_SYMBOL) {
		text << " symbol";
	} else if (argv[0]==gensym("_0_list") && argc==2 && argv[1].a_type==A_POINTER) {
		text << " pointer";
	} else if (argv[0]==gensym("_0_list") && argc==1) {
		text << " bang";
	} else {
		text << " " << argv[0].a_symbol->s_name+3; // as is
	}
	for (int i=1; i<argc; i++) {text << " " << argv[i];}
	post("%s",text.str().data());
}
\end class {install("gf.print",1,0); add_creator3(fclass,"print");}

#ifdef HAVE_DESIREDATA
t_glist *glist_getcanvas(t_glist *foo) {return foo;}//dummy
void canvas_fixlinesfor(t_glist *foo,t_text *) {}//dummy
#endif

//#ifdef HAVE_DESIREDATA
static void display_update(void *x);
\class Display : FObject {
	bool selected;
	t_glist *canvas;
	t_symbol *rsym;
	int y,x,sy,sx;
	bool vis;
	std::ostringstream text;
	t_clock *clock;
	t_pd *gp;
	\constructor () {
		selected=false; canvas=0; y=0; x=0; sy=16; sx=80; vis=false; clock=0;
		std::ostringstream os;
		rsym = gensym(const_cast<char *>(ssprintf("display:%08x",this).data()));
		pd_typedmess(&pd_objectmaker,gensym("#print"),0,0);
		gp = pd_newest();
		t_atom a[1];
		SETFLOAT(a,20);
		pd_typedmess(gp,gensym("maxrows"),1,a);
		text << "...";
		pd_bind((t_pd *)bself,rsym);
		SETPOINTER(a,(t_gpointer *)bself);
		pd_typedmess(gp,gensym("dest"),1,a);
		clock = clock_new((void *)this,(void(*)())display_update);
	}
	~Display () {
		pd_unbind((t_pd *)bself,rsym);
		pd_free(gp);
		if (clock) clock_free(clock);
	}
	\decl void anything (...);
	\decl 0 set_size(int sy, int sx);
	\decl 0 grid(...);
	\decl 0 very_long_name_that_nobody_uses(...);
 	void show() {
		std::ostringstream quoted;
	//	def quote(text) "\"" + text.gsub(/["\[\]\n\$]/m) {|x| if x=="\n" then "\\n" else "\\"+x end } + "\"" end
		std::string ss = text.str();
		const char *s = ss.data();
		int n = ss.length();
		for (int i=0;i<n;i++) {
			if (s[i]=='\n') quoted << "\\n";
			else if (strchr("\"[]$",s[i])) quoted << "\\" << (char)s[i];
			else quoted << (char)s[i];
		}
		//return if not canvas or not @vis # can't show for now...
		/* we're not using quoting for now because there's a bug in it. */
		/* btw, this quoting is using "", but we're gonna use {} instead for now, because of newlines */
		sys_vgui("display_update %s %d %d #000000 #cccccc %s {Courier -12} .x%x.c {%s}\n",
			rsym->s_name,bself->te_xpix,bself->te_ypix,selected?"#0000ff":"#000000",canvas,ss.data());
	}
};
static void display_getrectfn(t_gobj *x, t_glist *glist, int *x1, int *y1, int *x2, int *y2) {
	BFObject *bself = (BFObject*)x; Display *self = (Display *)bself->self; self->canvas = glist;
	*x1 = bself->te_xpix-1;
	*y1 = bself->te_ypix-1;
	*x2 = bself->te_xpix+1+self->sx;
	*y2 = bself->te_ypix+1+self->sy;
}
static void display_displacefn(t_gobj *x, t_glist *glist, int dx, int dy) {
	BFObject *bself = (BFObject*)x; Display *self = (Display *)bself->self; self->canvas = glist;
	bself->te_xpix+=dx;
	bself->te_ypix+=dy;
	self->canvas = glist_getcanvas(glist);
	self->show();
	canvas_fixlinesfor(glist, (t_text *)x);
}
static void display_selectfn(t_gobj *x, t_glist *glist, int state) {
	BFObject *bself = (BFObject*)x; Display *self = (Display *)bself->self; self->canvas = glist;
	self->selected=!!state;
	sys_vgui(".x%x.c itemconfigure %s -outline %s\n",glist_getcanvas(glist),self->rsym->s_name,self->selected?"#0000ff":"#000000");
}
static void display_deletefn(t_gobj *x, t_glist *glist) {
	BFObject *bself = (BFObject*)x; Display *self = (Display *)bself->self; self->canvas = glist;
	if (self->vis) sys_vgui(".x%x.c delete %s %sTEXT\n",glist_getcanvas(glist),self->rsym->s_name,self->rsym->s_name);
	canvas_deletelinesfor(glist, (t_text *)x);
}
static void display_visfn(t_gobj *x, t_glist *glist, int flag) {
	BFObject *bself = (BFObject*)x; Display *self = (Display *)bself->self; self->canvas = glist;
	self->vis = !!flag;
	display_update(self);
}
static void display_update(void *x) {
	Display *self = (Display *)x;
	if (self->vis) self->show();
}
\def 0 set_size(int sy, int sx) {this->sy=sy; this->sx=sx;}
\def void anything (...) {
	string sel = string(argv[0]).data()+3;
	text.str("");
	if (sel != "float") {text << sel; if (argc>1) text << " ";}
	long col = text.str().length();
	char buf[MAXPDSTRING];
	for (int i=1; i<argc; i++) {
		atom_string(&argv[i],buf,MAXPDSTRING);
		text << buf;
		col += strlen(buf);
		if (i!=argc-1) {
			text << " ";
			col++;
			if (col>56) {text << "\\\\\n"; col=0;}
		}
	}
	clock_delay(clock,0);
}
\def 0 grid(...) {
	text.str("");
	pd_typedmess(gp,gensym("grid"),argc,argv);
	clock_delay(clock,0);
}
\def 0 very_long_name_that_nobody_uses(...) {
	if (text.str().length()) text << "\n";
	for (int i=0; i<argc; i++) text << (char)INT(argv[i]);
}
\end class {
#ifndef DESIRE
	install("display",1,0);
	t_class *qlass = fclass->bfclass;
	t_widgetbehavior *wb = new t_widgetbehavior;
	wb->w_getrectfn    = display_getrectfn;
	wb->w_displacefn   = display_displacefn;
	wb->w_selectfn     = display_selectfn;
	wb->w_activatefn   = 0;
	wb->w_deletefn     = display_deletefn;
	wb->w_visfn        = display_visfn;
	wb->w_clickfn      = 0;
	class_setwidget(qlass,wb);
	sys_gui("proc display_update {self x y fg bg outline font canvas text} { \n\
		$canvas delete ${self}TEXT \n\
		$canvas create text [expr $x+2] [expr $y+2] -fill $fg -font $font -text $text -anchor nw -tag ${self}TEXT \n\
		foreach {x1 y1 x2 y2} [$canvas bbox ${self}TEXT] {} \n\
		incr x -1 \n\
		incr y -1 \n\
		set sx [expr $x2-$x1+2] \n\
		set sy [expr $y2-$y1+4] \n\
		$canvas delete ${self} \n\
		$canvas create rectangle $x $y [expr $x+$sx] [expr $y+$sy] -fill $bg -tags $self -outline $outline \n\
		$canvas create rectangle $x $y [expr $x+7]         $y      -fill red -tags $self -outline $outline \n\
		$canvas lower $self ${self}TEXT \n\
		pd \"$self set_size $sy $sx;\" \n\
	}\n");
#endif
}
//#endif // ndef HAVE_DESIREDATA

//****************************************************************

\class UnixTime : FObject {
	\constructor () {}
	\decl 0 bang ();
};
\def 0 bang () {
	timeval tv;
	gettimeofday(&tv,0);
	time_t t = time(0);
	struct tm *tmp = localtime(&t);
	if (!tmp) RAISE("localtime: %s",strerror(errno));
	char tt[MAXPDSTRING];
	strftime(tt,MAXPDSTRING,"%a %b %d %H:%M:%S %Z %Y",tmp);
	t_atom a[6];
	SETFLOAT(a+0,tmp->tm_year+1900);
	SETFLOAT(a+1,tmp->tm_mon-1);
	SETFLOAT(a+2,tmp->tm_mday);
	SETFLOAT(a+3,tmp->tm_hour);
	SETFLOAT(a+4,tmp->tm_min);
	SETFLOAT(a+5,tmp->tm_sec);
	t_atom b[3];
	SETFLOAT(b+0,tv.tv_sec/86400);
	SETFLOAT(b+1,mod(tv.tv_sec,86400));
	SETFLOAT(b+2,tv.tv_usec);
	outlet_anything(bself->outlets[2],&s_list,6,a);
	outlet_anything(bself->outlets[1],&s_list,3,b);
	send_out(0,strlen(tt),tt);
}

\end class UnixTime {install("unix_time",1,3);}


//****************************************************************

/* if using a DB-25 female connector as found on a PC, then the pin numbering is like:
  13 _____ 1
  25 \___/ 14
  1 = STROBE = the clock line is a square wave, often at 9600 Hz,
      which determines the data rate in usual circumstances.
  2..9 = D0..D7 = the eight ordinary data bits
  10 = -ACK (status bit 6 ?)
  11 = BUSY (status bit 7)
  12 = PAPER_END (status bit 5)
  13 = SELECT (status bit 4 ?)
  14 = -AUTOFD
  15 = -ERROR (status bit 3 ?)
  16 = -INIT
  17 = -SELECT_IN
  18..25 = GROUND
*/

//#include <linux/parport.h>
#define LPCHAR 0x0601
#define LPCAREFUL 0x0609 /* obsoleted??? wtf? */
#define LPGETSTATUS 0x060b /* return LP_S(minor) */
#define LPGETFLAGS 0x060e /* get status flags */

#include <sys/ioctl.h>

struct ParallelPort;
void ParallelPort_call(ParallelPort *self);
\class ParallelPort : FObject {
	FILE *f;
	int fd;
	int status;
	int flags;
	bool manually;
	t_clock *clock;
	~ParallelPort () {if (clock) clock_free(clock); if (f) fclose(f);}
	\constructor (string port, bool manually=0) {
		f = fopen(port.data(),"r+");
		if (!f) RAISE("open %s: %s",port.data(),strerror(errno));
		fd = fileno(f);
		status = 0xdeadbeef;
		flags  = 0xdeadbeef;
		this->manually = manually;
		clock = manually ? 0 : clock_new(this,(void(*)())ParallelPort_call);
		clock_delay(clock,0);
	}
	void call ();
	\decl 0 float (float x);
	\decl 0 bang ();
};
\def 0 float (float x) {
  uint8 foo = (uint8) x;
  fwrite(&foo,1,1,f);
  fflush(f);
}
void ParallelPort_call(ParallelPort *self) {self->call();}
void ParallelPort::call() {
	int flags;
	if (ioctl(fd,LPGETFLAGS,&flags)<0) post("ioctl: %s",strerror(errno));
	if (this->flags!=flags) outlet_float(bself->outlets[2],flags);
	this->flags = flags;
	int status;
	if (ioctl(fd,LPGETSTATUS,&status)<0) post("ioctl: %s",strerror(errno));
	if (this->status!=status) outlet_float(bself->outlets[1],status);
	this->status = status;
	if (clock) clock_delay(clock,2000);
}
\def 0 bang () {status = flags = 0xdeadbeef; call();}
//outlet 0 reserved (future use)
\end class {install("parallel_port",1,3);}

//****************************************************************

\class Route2 : FObject {
	int nsels;
	t_symbol **sels;
	~Route2() {if (sels) delete[] sels;}
	\constructor (...) {nsels=0; sels=0; _1_list(argc,argv); bself->noutlets_set(1+nsels);}
	\decl void anything(...);
	\decl 1 list(...);
};
\def void anything(...) {
	t_symbol *sel = gensym(argv[0].a_symbol->s_name+3);
	int i=0;
	for (i=0; i<nsels; i++) if (sel==sels[i]) break;
	outlet_anything(bself->outlets[i],sel,argc-1,argv+1);
}
\def 1 list(...) {
	for (int i=0; i<argc; i++) if (argv[i].a_type!=A_SYMBOL) {delete[] sels; RAISE("$%d: expected symbol",i+1);}
	if (sels) delete[] sels;
	nsels = argc;
	sels = new t_symbol*[argc];
	for (int i=0; i<argc; i++) sels[i] = argv[i].a_symbol;
}
\end class {install("route2",1,1);}

template <class T> int sgn(T a, T b=0) {return a<b?-1:a>b;}

\class Shunt : FObject {
	int n;
	\attr int index;
	\attr int mode;
	\attr int hi;
	\attr int lo;
	\constructor (int n=2, int i=0) {
		this->n=n;
		this->hi=n-1;
		this->lo=0;
		this->mode=0;
		this->index=i;
		bself->noutlets_set(n);
	}
	\decl void anything(...);
	\decl 1 float(int i);
};
\def void anything(...) {
	t_symbol *sel = gensym(argv[0].a_symbol->s_name+3);
	outlet_anything(bself->outlets[index],sel,argc-1,argv+1);
	if (mode) {
		index += sgn(mode);
		if (index<lo || index>hi) {
			int k = max(hi-lo+1,0);
			int m = gf_abs(mode);
			if (m==1) index = mod(index-lo,k)+lo; else {mode=-mode; index+=mode;}
		}
	}
}
\def 1 float(int i) {index = mod(i,n);}
\end class {install("shunt",2,0);}

struct Receives;
struct ReceivesProxy {
	t_pd x_pd;
	Receives *parent;
	t_symbol *suffix;
};
t_class *ReceivesProxy_class;

\class Receives : FObject {
	int ac;
	ReceivesProxy **av;
	t_symbol *prefix;
	t_symbol *local (t_symbol *suffix) {return gensym((string(prefix->s_name) + string(suffix->s_name)).data());}
	\constructor (t_symbol *prefix=&s_, ...) {
		this->prefix = prefix==gensym("empty") ? &s_ : prefix;
		int n = min(1,argc);
		do_bind(argc-n,argv+n);
	}
	\decl 0 bang ();
	\decl 0 symbol (t_symbol *s);
	\decl 0 list (...);
	void do_bind (int argc, t_atom2 *argv) {
		ac = argc;
		av = new ReceivesProxy *[argc];
		for (int i=0; i<ac; i++) {
			av[i] = (ReceivesProxy *)pd_new(ReceivesProxy_class);
			av[i]->parent = this;
			av[i]->suffix = argv[i];
			pd_bind(  (t_pd *)av[i],local(av[i]->suffix));
		}
	}
	void do_unbind () {
		for (int i=0; i<ac; i++) {
			pd_unbind((t_pd *)av[i],local(av[i]->suffix));
			pd_free((t_pd *)av[i]);
		}
		delete[] av;
	}
	~Receives () {do_unbind();}
};
\def 0 bang () {_0_list(0,0);}
\def 0 symbol (t_symbol *s) {t_atom2 a[1]; SETSYMBOL(a,s); _0_list(1,a);}
\def 0 list (...) {
	do_unbind();
	do_bind(argc,argv);
}
void ReceivesProxy_anything (ReceivesProxy *self, t_symbol *s, int argc, t_atom *argv) {
	outlet_symbol(  self->parent->bself->outlets[1],self->suffix);
	outlet_anything(self->parent->bself->outlets[0],s,argc,argv);
}
\end class {
	install("receives",1,2);
	ReceivesProxy_class = class_new(gensym("receives.proxy"),0,0,sizeof(ReceivesProxy),CLASS_PD|CLASS_NOINLET, A_NULL);
	class_addanything(ReceivesProxy_class,(t_method)ReceivesProxy_anything);
}

/* this can't report on bang,float,symbol,pointer,list because zgetfn can't either */
\class ClassExists : FObject {
	\constructor () {}
	\decl void _0_symbol(t_symbol *s);
};
\def void _0_symbol(t_symbol *s) {
	outlet_float(bself->outlets[0],!!zgetfn(&pd_objectmaker,s));
}
\end class {install("class_exists",1,1);}

\class ListEqual : FObject {
	t_list *list;
	\constructor (...) {list=0; _1_list(argc,argv);}
	\decl 0 list (...);
	\decl 1 list (...);
};
\def 1 list (...) {
	if (list) list_free(list);
	list = list_new(argc,argv);
}
\def 0 list (...) {
	if (binbuf_getnatom(list) != argc) {outlet_float(bself->outlets[0],0); return;}
	t_atom2 *at = (t_atom2 *)binbuf_getvec(list);
	for (int i=0; i<argc; i++) if (!atom_eq(at[i],argv[i])) {outlet_float(bself->outlets[0],0); return;}
	outlet_float(bself->outlets[0],1);
}
\end class {install("list.==",2,1);}

//****************************************************************
//#ifdef UNISTD
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/param.h>
#include <unistd.h>
//#endif
#if defined (__APPLE__) || defined (__FreeBSD__)
#define HZ CLK_TCK
#endif

uint64 cpu_hertz;
int uint64_compare(uint64 &a, uint64 &b) {return a<b?-1:a>b;}

\class UserTime : FObject {
	clock_t time;
	\constructor () {_0_bang(argc,argv);}
	\decl 0 bang ();
	\decl 1 bang ();
};
\def 0 bang () {struct tms t; times(&t); time = t.tms_utime;}
\def 1 bang () {struct tms t; times(&t); outlet_float(bself->outlets[0],(t.tms_utime-time)*1000/HZ);}
\end class {install("usertime",2,1);}
\class SystemTime : FObject {
	clock_t time;
	\constructor () {_0_bang(argc,argv);}
	\decl 0 bang ();
	\decl 1 bang ();
};
\def 0 bang () {struct tms t; times(&t); time = t.tms_stime;}
\def 1 bang () {struct tms t; times(&t); outlet_float(bself->outlets[0],(t.tms_stime-time)*1000/HZ);}
\end class {install("systemtime",2,1);}
\class TSCTime : FObject {
	uint64 time;
	\constructor () {_0_bang(argc,argv);}
	\decl 0 bang ();
	\decl 1 bang ();
};
\def 0 bang () {time=rdtsc();}
\def 1 bang () {outlet_float(bself->outlets[0],(rdtsc()-time)*1000.0/cpu_hertz);}
\end class {install("tsctime",2,1);
	struct timeval t0,t1;
	uint64 u0,u1;
	uint64 estimates[3];
	for (int i=0; i<3; i++) {
		u0=rdtsc(); gettimeofday(&t0,0); usleep(10000);
		u1=rdtsc(); gettimeofday(&t1,0);
		uint64 t = (t1.tv_sec-t0.tv_sec)*1000000+(t1.tv_usec-t0.tv_usec);
		estimates[i] = (u1-u0)*1000000/t;
	}
	qsort(estimates,3,sizeof(uint64),(comparator_t)uint64_compare);
	cpu_hertz = estimates[1];
}

\class GFError : FObject {
	string format;
	\constructor (...) {
		std::ostringstream o;
		char buf[MAXPDSTRING];
		for (int i=0; i<argc; i++) {
			atom_string(&argv[i],buf,MAXPDSTRING);
			o << buf;
			if (i!=argc-1) o << ' ';
		}
		format = o.str();
	}
	\decl 0 bang ();
	\decl 0 float (float f);
	\decl 0 symbol (t_symbol *s);
	\decl 0 list (...);
};
\def 0 bang () {_0_list(0,0);}
\def 0 float (float f) {_0_list(argc,argv);}
\def 0 symbol (t_symbol *s) {_0_list(argc,argv);}

\def 0 list (...) {
	std::ostringstream o;
	pd_oprintf(o,format.data(),argc,argv);
	t_canvas *canvas = canvas_getrootfor(bself->mom);
	string s = o.str();
	pd_error(canvas,"%s",s.data());
}
\end class {install("gf.error",1,0);}

//****************************************************************
\class ForEach : FObject {
	\constructor () {}
	\decl 0 list (...);
};
\def 0 list (...) {
	t_outlet *o = bself->outlets[0];
	for (int i=0; i<argc; i++) {
		if      (argv[i].a_type==A_FLOAT)  outlet_float( o,argv[i]);
		else if (argv[i].a_type==A_SYMBOL) outlet_symbol(o,argv[i]);
		else RAISE("oops. unsupported.");
	}
}
\end class {install("foreach",1,1);}

//****************************************************************

#define MOM \
	t_canvas *mom = bself->mom; \
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}

\class GFCanvasFileName : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 bang ();
};
\def 0 bang () {MOM; outlet_symbol(bself->outlets[0],mom->gl_name ? mom->gl_name : gensym("empty"));}
\end class {install("gf/canvas_filename",1,1);}
\class GFCanvasDollarZero : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 bang ();
};
\def 0 bang () {MOM; outlet_float(bself->outlets[0],canvas_getenv(mom)->ce_dollarzero);}
\end class {install("gf/canvas_dollarzero",1,1);}
\class GFCanvasGetPos : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 bang ();
};
\def 0 bang () {MOM;
	t_atom a[2];
	SETFLOAT(a+0,mom->gl_obj.te_xpix);
	SETFLOAT(a+1,mom->gl_obj.te_ypix);
	outlet_list(bself->outlets[0],&s_list,2,a);
}
\end class {install("gf/canvas_getpos",1,1);}
\class GFCanvasSetPos : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 list (...);
};
\def 0 list (...) {
	MOM;
	if (argc!=2) RAISE("wrong number of args");
	mom->gl_obj.te_xpix = atom_getfloatarg(0,argc,argv);
	mom->gl_obj.te_ypix = atom_getfloatarg(1,argc,argv);
	t_canvas *granny = mom->gl_owner;
	if (!granny) RAISE("no such canvas");
#ifdef DESIREDATA
	gobj_changed(mom);
#else
        gobj_vis((t_gobj *)mom,granny,0);
        gobj_vis((t_gobj *)mom,granny,1);
	canvas_fixlinesfor(glist_getcanvas(granny), (t_text *)mom);
#endif
}
\end class {install("gf/canvas_setpos",1,0);}
\class GFCanvasEditMode : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 bang ();
};
\def 0 bang () {MOM;
	t_atom a[1]; SETFLOAT(a+0,0);
	outlet_float(bself->outlets[0],mom->gl_edit);
}
\end class {install("gf/canvas_edit_mode",1,1);}
extern "C" void canvas_setgraph(t_glist *x, int flag, int nogoprect);
\class GFCanvasSetGOP : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 float (float gop);
};
\def 0 float (float gop) {MOM; t_atom a[1]; SETFLOAT(a+0,0); canvas_setgraph(mom,gop,0);}
\end class {install("gf/canvas_setgop",1,0);}
\class GFCanvasXID : FObject {
	int n;
	t_symbol *name;
	\constructor (int n_) {
		n=n_;
		name=symprintf("gf/canvas_xid:%lx",bself);
		pd_bind((t_pd *)bself,name);
	}
	~GFCanvasXID () {pd_unbind((t_pd *)bself,name);}
	\decl 0 bang ();
	\decl 0 xid (t_symbol *t, t_symbol *u);
};
\def 0 bang () {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	sys_vgui("pd %s xid [winfo id .x%lx.c] [winfo id .x%lx]\\;\n",name->s_name,long(mom));
}
\def 0 xid (t_symbol *t, t_symbol *u) {
	outlet_symbol(bself->outlets[0],t);
	outlet_symbol(bself->outlets[1],u);
}
\end class {install("gf/canvas_xid",1,2);}

\class GFCanvasHeHeHe : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 float (float y);
};
\def 0 float (float y) {MOM;
	// was 568
	mom->gl_screenx2 = mom->gl_screenx1 + 600;
	if (mom->gl_screeny2-mom->gl_screeny1 < y) mom->gl_screeny2 = mom->gl_screeny1+y;
	sys_vgui("wm geometry .x%lx %dx%d\n",long(mom),
	  int(mom->gl_screenx2-mom->gl_screenx1),
	  int(mom->gl_screeny2-mom->gl_screeny1));
}
\end class {install("gf/canvas_hehehe",1,1);}

#define DASHRECT "-outline #80d4b2 -dash {2 6 2 6}"

\class GFCanvasHoHoHo : FObject {
	int n;
	t_canvas *last;
	\constructor (int n) {this->n=n; last=0;}
	void hide () {if (last) sys_vgui(".x%lx.c delete %lxRECT\n",long(last),bself);}
	~GFCanvasHoHoHo () {hide();}
	\decl 0 list (int x1, int y1, int x2, int y2);
};
\def 0 list (int x1, int y1, int x2, int y2) {
	hide();
	MOM;
	last = mom;
	sys_vgui(".x%lx.c create rectangle %d %d %d %d "DASHRECT" -tags %lxRECT\n",long(last),x1,y1,x2,y2,bself);
}
\end class {install("gf/canvas_hohoho",1,0);}

#define canvas_each(y,x) for (t_gobj *y=x->gl_list; y; y=y->g_next)
\class GFCanvasCount : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 bang ();
};
\def 0 bang () {MOM; int k=0; canvas_each(y,mom) k++; outlet_float(bself->outlets[0],k);}
\end class {install("gf/canvas_count",1,1);}

\class GFCanvasLoadbang : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 float (float m);
};
\def 0 float (float m) {MOM;
	int k=0;
	canvas_each(y,mom) {
		k++;
		if (k>=m && pd_class((t_pd *)y)==canvas_class) canvas_loadbang((t_canvas *)y);
	}
	
}
\end class {
	install("gf/canvas_loadbang",1,0);
};

\class GFLOL : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 wire_dotted (int r, int g, int b);
	\decl 0 wire_hide ();
	\decl 0  box_dotted (int r, int g, int b);
	\decl 0  box_align (t_symbol *s, int x_start, int y_start, int incr);
};
#define BEGIN \
	t_outlet *ouch = ((t_object *)bself->mom)->te_outlet; \
	t_canvas *can = bself->mom->gl_owner; \
	if (!can) RAISE("no such canvas"); \
	for (int i=0; i<n; i++) {ouch = ouch->next; if (!ouch) {RAISE("no such outlet");}}
#define wire_each(wire,ouchlet) for (t_outconnect *wire = ouchlet->connections; wire; wire=wire->next)
\def 0 wire_dotted (int r, int g, int b) {
#ifndef DESIREDATA
	BEGIN
	wire_each(wire,ouch) {
		sys_vgui(".x%lx.c itemconfigure l%lx -fill #%02x%02x%02x -dash {3 3 3 3}\n",long(can),long(wire),r,g,b);
	}
#else
	post("doesn't work with DesireData");
#endif
}
\def 0 wire_hide () {
#ifndef DESIREDATA
	BEGIN
	wire_each(wire,ouch) sys_vgui(".x%lx.c delete l%lx\n",long(can),long(wire));
#else
	post("doesn't work with DesireData");
#endif
}
\def 0 box_dotted (int r, int g, int b) {
#ifndef DESIREDATA
	BEGIN
	wire_each(wire,ouch) {
		t_object *t = (t_object *)wire->to;
		int x1,y1,x2,y2;
		gobj_getrect((t_gobj *)wire->to,can,&x1,&y1,&x2,&y2);
		// was #00aa66 {3 5 3 5}
		sys_vgui(".x%lx.c delete %lxRECT; .x%lx.c create rectangle %d %d %d %d "DASHRECT" -tags %lxRECT\n",
			long(can),long(t),long(can),x1,y1,x2,y2,long(t));
	}
#else
	post("doesn't work with DesireData");
#endif
}
bool comment_sort_y_lt(t_object * const &a, t_object * const &b) /* is a StrictWeakOrdering */ {
	return a->te_ypix < b->te_ypix;
}
#define foreach(ITER,COLL) for(typeof(COLL.begin()) ITER = COLL.begin(); ITER != (COLL).end(); ITER++)
static t_class *inlet_class, *floatinlet_class, *symbolinlet_class, *pointerinlet_class;
static bool ISINLET(t_pd *o) {
  t_class *c=pd_class(o);
  return c==inlet_class || c==floatinlet_class || c==symbolinlet_class || c==pointerinlet_class;
}
struct _inlet {
    t_pd pd;
    struct _inlet *next;
    t_object *owner;
    t_pd *dest;
    t_symbol *symfrom;
    //union inletunion un;
};
\def 0 box_align (t_symbol *dir, int x_start, int y_start, int incr) {
	int x=x_start, y=y_start;
	bool horiz;
	if (dir==&s_x) horiz=false; else
	if (dir==&s_y) horiz=true;  else RAISE("$1 must be x or y");
#ifndef DESIREDATA
	std::vector<t_object *> v;
	BEGIN
	wire_each(wire,ouch) {
		//post("wire to object of class %s ISINLET=%d",pd_class(wire->to)->c_name->s_name,ISINLET(wire->to));
		t_object *to = ISINLET(wire->to) ? ((t_inlet *)wire->to)->owner : (t_object *)wire->to;
		v.push_back(to);
	}
	sort(v.begin(),v.end(),comment_sort_y_lt);
	foreach(tt,v) {
		t_object *t = *tt;
		if (t->te_xpix!=x || t->te_ypix!=y) {
			gobj_vis((t_gobj *)t,can,0);
			t->te_xpix=x;
			t->te_ypix=y;
			gobj_vis((t_gobj *)t,can,1);
			canvas_fixlinesfor(can,t);
		}
		int x1,y1,x2,y2;
		gobj_getrect((t_gobj *)t,can,&x1,&y1,&x2,&y2);
		if (horiz) x += x2-x1+incr;
		else       y += y2-y1+incr;
	}
	if (horiz) outlet_float(bself->outlets[0],x-x_start);
	else       outlet_float(bself->outlets[0],y-y_start);
#else
	post("doesn't work with DesireData");
#endif
}

extern t_widgetbehavior text_widgetbehavior;
t_widgetbehavior text_widgetbehavi0r;

/* i was gonna use gobj_shouldvis but it's only for >= 0.42 */

static int text_chou_de_vis(t_text *x, t_glist *glist) {
    return (glist->gl_havewindow ||
        (x->te_pd != canvas_class && x->te_pd->c_wb != &text_widgetbehavior) ||
        (x->te_pd == canvas_class && (((t_glist *)x)->gl_isgraph)) ||
        (glist->gl_goprect && (x->te_type == T_TEXT)));
}

static void text_visfn_hax0r (t_gobj *o, t_canvas *can, int vis) {
	text_widgetbehavior.w_visfn(o,can,vis);
	//if (vis) return; // if you want to see #X text inlets uncomment this line
      t_rtext *y = glist_findrtext(can,(t_text *)o);
	if (text_chou_de_vis((t_text *)o,can)) glist_eraseiofor(can,(t_object *)o,rtext_gettag(y));
}
\end class {
	install("gf/lol",1,1);
#ifndef DESIREDATA
	class_setpropertiesfn(text_class,(t_propertiesfn)0xDECAFFED);
	unsigned long *lol = (unsigned long *)text_class;
	int i=0;
	while (lol[i]!=0xDECAFFED) i++;
	*((char *)(lol+i+1) + 6) = 1;
	class_setpropertiesfn(text_class,0);
	t_object *bogus = (t_object *)pd_new(text_class);
	       inlet_class = pd_class((t_pd *)       inlet_new(bogus,0,0,0));
	  floatinlet_class = pd_class((t_pd *)  floatinlet_new(bogus,0));
	 symbolinlet_class = pd_class((t_pd *) symbolinlet_new(bogus,0));
	pointerinlet_class = pd_class((t_pd *)pointerinlet_new(bogus,0));
	memcpy(&text_widgetbehavi0r,&text_widgetbehavior,sizeof(t_widgetbehavior));
	text_widgetbehavi0r.w_visfn = text_visfn_hax0r;
	class_setwidget(text_class,&text_widgetbehavi0r);
#endif
}

\class GFStringReplace : FObject {
	t_symbol *from;
	t_symbol *to;
	\constructor (t_symbol *from, t_symbol *to=&s_) {this->from=from; this->to=to;}
	\decl 0 symbol (t_symbol *victim);
};
\def 0 symbol (t_symbol *victim) {
	string a = string(victim->s_name);
	string b = string(from->s_name);
	string c = string(to->s_name);
	for (size_t i=0;;) {
	  i = a.find(b,i);
	  if (i==string::npos) break;
	  a = a.replace(i,b.length(),c);
	  i += c.length();
	}
	outlet_symbol(bself->outlets[0],gensym(a.c_str()));
}
\end class {install("gf/string_replace",1,1);}

\class GFStringLessThan : FObject {
	t_symbol *than;
	\constructor (t_symbol *than=&s_) {this->than=than;}
	\decl 0 symbol (t_symbol *it);
	\decl 1 symbol (t_symbol *than);
};
\def 0 symbol (t_symbol *it) {outlet_float(bself->outlets[0],strcmp(it->s_name,than->s_name)<0);}
\def 1 symbol (t_symbol *than) {this->than=than;}
\end class {install("gf/string_<",2,1);}

void startup_flow_objects2 () {
	\startall
}
