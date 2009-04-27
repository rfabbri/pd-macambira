/* $Id: desire.c,v 1.1.2.217.2.235 2007-08-21 19:50:25 matju Exp $

  This file is part of DesireData.
  Copyright (c) 2004-2007 by Mathieu Bouchard.
  Portions Copyright (c) 1997-2005 Miller Puckette, Günter Geiger, Krzysztof Czaja,
                                   Johannes Zmoelnig, Thomas Musil, Joseph Sarlo, etc.
  The remains of IEMGUI Copyright (c) 2000-2001 Thomas Musil (IEM KUG Graz Austria)
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#define PD_PLUSPLUS_FACE
#include "m_pd.h"
#include "desire.h"
#include "s_stuff.h"
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m_simd.h"
#include <errno.h>
#include <sys/time.h>
#include <sstream>
#include <map>

#ifdef MSW
#include <io.h>
#define snprintf _snprintf
#else
#include <unistd.h>
#endif

/*
#define sys_vgui(args...) do { \
	fprintf(stderr,"\e[0;1;31m"); \
	L fprintf(stderr,args); \
	fprintf(stderr,"\e[0m"); \
	sys_vgui(args); } while(0)
*/

#define foreach(ITER,COLL) for(typeof(COLL.begin()) ITER = COLL.begin(); ITER != (COLL).end(); ITER++)

#define boxes_each(CHILD,BOXES)   for(t_gobj *CHILD=(BOXES)->first(); CHILD; CHILD=CHILD->next())
#define canvas_each(CHILD,CANVAS)   for(t_gobj *CHILD=(CANVAS)->boxes->first(); CHILD; CHILD=CHILD->next())
#define canvas_wires_each(WIRE,TRAV,CANVAS) \
	for (t_outconnect *WIRE=(t_outconnect *)666; WIRE==(t_outconnect *)666; ) \
		for (t_linetraverser TRAV(CANVAS); (WIRE=linetraverser_next(&TRAV)); )

#undef SET
#define SET(attr,value) do {gobj_changed(x,#attr); x->attr = (value);} while(0)

#define a_float  a_w.w_float
#define a_symbol a_w.w_symbol

#define CLAMP(_var,_min,_max) do { if (_var<_min) _var=_min; else if (_var>_max) _var=_max; } while(0)
#define IS_A_FLOAT(atom,index)   ((atom+index)->a_type == A_FLOAT)
#define IS_A_SYMBOL(atom,index)  ((atom+index)->a_type == A_SYMBOL)

int imin(int a, int b) {return a<b?a:b;}
int imax(int a, int b) {return a>b?a:b;}
t_symbol *s_empty, *s_pd, *s_Pd;

std::ostream &operator << (std::ostream &str, t_pd *self) {
	t_binbuf *b;
	if (self->_class->patchable && (b = ((t_text *)self)->binbuf)) {
		char *buf; int bufn;
		binbuf_gettext(b,&buf,&bufn);
		str << "[" << buf << "]";
		free(buf);
	} else str << "<" << self->_class->name->name << ":" << std::hex << (long)self << ">";
	return str;
}

//--------------------------------------------------------------------------

t_class *boxes_class;

struct t_boxes : t_gobj {
	typedef  std::map<int,t_gobj *> M;
	typedef std::pair<int,t_gobj *> KV;
private:
	std::map<int,t_gobj *> map;
public:
	void invariant () {size();}
	t_boxes() {}
	size_t size() {
		size_t n=0;
		boxes_each(g,this) n++;
		if (map.size()!=n) post("map size=%d list size=%d",map.size(),n);
		return n;
	}
	t_gobj *first() {return map.begin() != map.end() ? map.begin()->second : 0;}
	t_gobj *last()  {M::iterator iter = map.end(); iter--; return iter->second;}
	t_gobj *next(t_gobj *x) {
		M::iterator iter = map.begin();
		while (iter->second != x) iter++;
		iter++;
		return iter == map.end() ? 0 : iter->second;
	}
	t_gobj *get(int i) {return map.find(i)==map.end() ? 0 : map[i];}
	void add(t_gobj *x) {map.insert(KV(x->dix->index,x)); invariant();}
	void remove(int i) {map.erase(i); invariant();}
	void remove_by_value(t_gobj *x) {map.erase(x->dix->index); invariant();}
};

t_boxes *boxes_new() {
	t_boxes *self = (t_boxes *)pd_new(boxes_class);
	new(self) t_boxes;
	return self;
}

void boxes_notice(t_boxes *self, t_gobj *origin, int argc, t_atom *argv) {
        gobj_changed3(self,origin,argc,argv);
}

void boxes_free(t_boxes *self) {self->~t_boxes();}

t_gobj *_gobj::next() {return dix->canvas->boxes->next(this);}

//--------------------------------------------------------------------------

t_class *gop_filtre_class;

struct t_gop_filtre : t_gobj {
	t_canvas *canvas;
};

// test for half-open interval membership
bool inside (int x, int x0, int x1) {return x0<=x && x<x1;}

/* this method is always called when a canvas is in gop mode so we don't check for this */
/* it doesn't filtre out all that needs to be filtred out, but does not filtre out anything that has to stay */
void gop_filtre_notice(t_gop_filtre *self,t_gobj *origin, int argc, t_atom *argv) {
	/* here, assume that the canvas *is* a gop; else we wouldn't be in this method. */
	t_object *o = (t_object *)origin;
	t_canvas *c = self->canvas;
	if (0/* check for messagebox, comment, but you can't check for objectboxes in general */) return;
	if (c->goprect) {
		if (!inside(o->x, c->xmargin, c->xmargin + c->pixwidth )) return;
		if (!inside(o->y, c->ymargin, c->ymargin + c->pixheight)) return;
	}
	gobj_changed3(self,origin,argc,argv);
}

t_gobj *gop_filtre_new(t_canvas *canvas) {
	t_gop_filtre *self = (t_gop_filtre *)pd_new(gop_filtre_class);
	new(self) t_gop_filtre;
	self->canvas = canvas;
	gobj_subscribe(canvas->boxes,self);
	return self;
}

void gop_filtre_free(t_boxes *self) {}

//--------------------------------------------------------------------------
// t_appendix: an extension to t_gobj made by matju so that all t_gobj's may have new fields
// without sacrificing binary compat with externals compiled for PureMSP.

typedef t_hash<t_symbol *, t_arglist *> t_visual;

t_appendix *appendix_new (t_gobj *master) {
	//fprintf(stderr,"appendix_new %p\n",master);
	t_appendix *self = (t_appendix *)malloc(sizeof(t_appendix));
	self->canvas = 0;
	self->nobs = 0;
	self->obs = 0;
	self->visual = new t_visual(1);
	self->index = 0;
	self->elapsed = 0;
	return self;
}

void appendix_save (t_gobj *master, t_binbuf *b) {
	t_visual *h = master->dix->visual;
	//fprintf(stderr,"appendix_save %p size=%ld\n",master,hash_size(h));
	if (!h->size()) return;
	t_symbol *k;
	t_arglist *v;
	int i=0;
	binbuf_addv(b,"t","#V");
	hash_foreach(k,v,h) {
		t_arglist *al = (t_arglist *)v;
		binbuf_addv(b,"s",k);
		binbuf_add(b,al->c,al->v);
		if (size_t(i+1)==h->size()) binbuf_addv(b, ";"); else binbuf_addv(b,"t",",");
		i++;
        }
}

void appendix_free (t_gobj *master) {
	t_appendix *self = master->dix;
	t_symbol *k;
	t_arglist *v;
	if (self->visual) {
		hash_foreach(k,v,self->visual) free(v);
		delete self->visual;
	}
	free(self);
}

/* subscribing N spies takes N*N time, but it's not important for now */
/* subscription could become just a special use of t_outlet in the future, or sometimes become implied. */
/* perhaps use [bindelem] here? */
void gobj_subscribe(t_gobj *self, t_gobj *observer) {
	t_appendix *d = self->dix;
	for (size_t i=0; i<d->nobs; i++) if (d->obs[i]) return;
	d->obs=(t_gobj **)realloc(d->obs,sizeof(t_gobj *)*(1+d->nobs));
	d->obs[d->nobs++] = observer;
	t_onsubscribe ons = self->_class->onsubscribe;
	ons(self,observer);
	//post("x%p has %d observers",self,(int)d->nobs);
}

void gobj_unsubscribe (t_gobj *self, t_gobj *observer) {
	t_appendix *d = self->dix;
	size_t i;
	for (i=0; i<d->nobs; i++) if (d->obs[i]) break;
	if (i==d->nobs) return;
	d->nobs--;
	for (; i<d->nobs; i++) d->obs[i] = d->obs[i+1];
	// should have something like onunsubscribe too, to handle delete?... or just use onsubscribe differently.
}

void gobj_setcanvas (t_gobj *self, t_canvas *c) {
	if (self->dix->canvas == c) return;
	if (self->dix->canvas) gobj_unsubscribe(self,self->dix->canvas);
	self->dix->canvas = c;
	if (self->dix->canvas)   gobj_subscribe(self,self->dix->canvas);
}

/* for future use */
t_canvas *gobj_canvas (t_gobj *self) {return self->dix->canvas;}

// if !k then suppose all of the object might have changed.
void gobj_changed (t_gobj *self, const char *k) {
	int dirty = k ? (1<<class_getfieldindex(self->_class,k)) : -1;
	t_atom argv[1];
	SETFLOAT(argv,(float)dirty);
	gobj_changed3(self,self,1,argv);
}
//#define gobj_changed(SELF,K) do {L; gobj_changed(SELF,K);} while(0)

// if only a float is sent, it's a bitset of at most 25 elements
// else it may mean whatever else...
void gobj_changed2 (t_gobj *self, int argc, t_atom *argv) {
	gobj_changed3(self,self,argc,argv);
}

void gobj_changed3 (t_gobj *self, t_gobj *origin, int argc, t_atom *argv) {
	t_appendix *d = self->dix;
	std::ostringstream s;
	for (int i=0; i<argc; i++) s << " " << &argv[i];
	//fprintf(stderr,"gobj_changed3 self=%lx origin=%lx args=%s\n",(long)self,(long)origin,s.str().data());
	/* TRACE THE DIFFERENTIAL UPLOAD REQUESTS */
	//std::cerr << "gobj_changed3 self=" << self << " origin=" << origin << " args={" << s.str().data()+(!!argc) << "}\n";
	if (!d) {post("gobj_changed3: no appendix in %p",self); return;}
	for (size_t i=0; i<d->nobs; i++) {
		t_gobj *obs = d->obs[i];
		t_notice ice = obs->_class->notice;
		if (ice) ice(obs,origin,argc,argv);
		else post("null func ptr for class %s",self->_class->name->name);
	}
}

//--------------------------------------------------------------------------
// some simple ringbuffer-style queue
// when this becomes too small, use a bigger constant or a better impl.

#define QUEUE_SIZE 16384
struct t_queue {
	int start,len;
	t_pd *o[QUEUE_SIZE];
};

t_queue *queue_new () {
	t_queue *self = (t_queue *)getbytes(sizeof(t_queue));
	self->start = self->len = 0;
	return self;
}

static bool debug_queue=0;

static void pd_print (t_pd *self, const char *header) {
	if (self->_class->gobj && (object_table->get(self)&1)==0) {printf("%s %p dead\n",header,self); return;}
	if (self->_class->patchable) {
		t_binbuf *b = ((t_text *)self)->binbuf;
		if (b) {
			char *buf; int bufn;
			binbuf_gettext(b,&buf,&bufn);
			printf("%s %p [%.*s]\n",header,self,bufn,buf);
			return;
		}
	}
	printf("%s %p (%s)\n",header,self,self->_class->name->name);
}

void queue_put (t_queue *self, t_pd *stuff) {
	if (debug_queue) pd_print(stuff,"queue_put");
	if (self->len==QUEUE_SIZE) {bug("queue full"); return;}
	self->o[(self->start+self->len)%QUEUE_SIZE] = stuff;
	self->len++;
	if (debug_queue) post("queue_put: items in queue: %d",self->len);
}

void *queue_get (t_queue *self) {
	t_pd *stuff = self->o[self->start];
	self->start = (self->start+1)%QUEUE_SIZE;
	self->len--;
	if (debug_queue) {post("queue_get: items in queue: %d",self->len); pd_print(stuff,"queue_get");}
	return stuff;
}

#define queue_each(i,self) \
	for (int i=self->start; i!=(self->start+self->len)%QUEUE_SIZE; i=(i+1)%QUEUE_SIZE)

void queue_free (t_queue *self) {
	abort();
	free(self);
}

int queue_empty (t_queue *self) {return self->len==0;}
int queue_full  (t_queue *self) {return self->len==QUEUE_SIZE;}

//--------------------------------------------------------------------------
// reply system:

struct t_reply : t_gobj {
	short serial;
	void *answer;
};

t_class *reply_class;

static t_reply *reply_new (short serial, void *answer) {
	t_reply *self = (t_reply *)pd_new(reply_class);
	self->dix = appendix_new(self);
	self->serial = serial;
	self->answer = answer;
	return self;
}

static void reply_send (t_reply *self) {
	sys_vgui("serial %ld x%lx\n",(long)self->serial,(long)self->answer);
}

static void reply_free (t_reply *self) {}

//--------------------------------------------------------------------------
// update manager:

struct t_dirtyentry {
	long fields;
	size_t start,end;
	t_dirtyentry() {}
};

typedef t_hash <t_gobj *, t_dirtyentry *> t_dirtyset;

struct t_manager : t_text {
	t_queue *q;
	t_clock *clock;
	t_binbuf *b; /* reusable, for processing messages from the gui */
	t_dirtyset *dirty;
};

static t_class *manager_class;
t_manager *manager;

void manager_call (void *foo) {
	t_manager *self = (t_manager *)foo;
	while (!queue_empty(self->q)) {
		t_gobj *o = (t_gobj *)queue_get(self->q);
		if (!o) continue; /* cancelled notice */
		//fprintf(stderr,"manager_call, o->_class=%s\n",o->_class->c_name->name);
		if (o->_class == reply_class) {
			reply_send((t_reply *)o);
			pd_free(o);
		} else {
			if (self->dirty->exists(o)) {
				pd_upload(o);
				self->dirty->del(o);
			}
		}
	}
	clock_delay(self->clock,50);
}

void manager_notice (t_gobj *self_, t_gobj *origin, int argc, t_atom *argv) {
	t_manager *self = (t_manager *)self_;
	if (!self->dirty->exists(origin)) {
		//std::cerr << "manager_notice:";
		//for (int i=0; i<argc; i++) std::cerr << " " << &argv[i];
		//std::cerr << "\n";
		queue_put(self->q,origin);
		self->dirty->set(origin,0);
	}
}

t_manager *manager_new (t_symbol *s, int argc, t_atom *argv) {
	t_manager *self = (t_manager *)pd_new(manager_class);
	self->q = queue_new();
	self->clock = clock_new(self,(t_method)manager_call);
	self->b = binbuf_new();
	clock_delay(self->clock,0);
	self->dirty = new t_dirtyset(127);
	return self;
}

void manager_free (t_manager *self) {
	clock_free(self->clock);
	queue_free(self->q);
	binbuf_free(self->b);
	pd_free(self);
}

extern "C" void manager_anything (t_manager *self, t_symbol *s, int argc, t_atom *argv) {
	binbuf_clear(self->b);
	binbuf_addv(self->b,"s",s);
	binbuf_add(self->b,argc,argv);
	binbuf_eval(self->b,0,0,0);
}

//--------------------------------------------------------------------------
/* 
IOhannes changed the canvas_restore, so that it might accept $args as well
(like "pd $0_test") so you can make multiple & distinguishable templates.
*/

#define CANVAS_DEFCANVASWIDTH 450
#define CANVAS_DEFCANVASHEIGHT 300

#ifdef __APPLE__
#define CANVAS_DEFCANVASYLOC 22
#else
#define CANVAS_DEFCANVASYLOC 0
#endif

extern short next_object;
extern t_pd *newest;
t_class *canvas_class;
int canvas_dspstate;                /* whether DSP is on or off */
t_canvas *canvas_whichfind;         /* last canvas we did a find in */
std::map<t_canvas *,int> windowed_canvases; /* where int is dummy */
static void canvas_setbounds(t_canvas *x, t_floatarg x1, t_floatarg y1, t_floatarg x2, t_floatarg y2);
static t_symbol *canvas_newfilename = &s_;
static t_symbol *canvas_newdirectory = &s_;
static int canvas_newargc;
static t_atom *canvas_newargv;

/* add a canvas the list of "root" canvases (toplevels without parents.) */
/* should those two functions still exist? */
static void canvas_addtolist(t_canvas *x) {
    windowed_canvases.insert(std::pair<t_canvas *,int>(x,42));
    if (x->havewindow) gobj_subscribe(x,manager);
}
static void canvas_takeofflist(t_canvas *x) {windowed_canvases.erase(x);}

/* if there's an old one lying around free it here.
   This happens if an abstraction is loaded but never gets as far as calling canvas_new(). */
void canvas_setargs(int argc, t_atom *argv) {
    if (canvas_newargv) free(canvas_newargv);
    canvas_newargc = argc;
    canvas_newargv = (t_atom *)copybytes(argv, argc * sizeof(t_atom));
}

void glob_setfilename(void *self, t_symbol *filesym, t_symbol *dirsym) {
    canvas_newfilename = filesym;
    canvas_newdirectory = dirsym;
}

t_canvas *canvas_getcurrent () {return ((t_canvas *)pd_findbyclass(&s__X, canvas_class));}

t_canvasenvironment *canvas_getenv(t_canvas *x) {
    while (!x->env) x = x->dix->canvas;
    return x->env;
}

int canvas_getdollarzero () {
    t_canvas *x = canvas_getcurrent();
    t_canvasenvironment *env = x ? canvas_getenv(x) : 0;
    return env ? env->dollarzero : 0;
}

t_symbol *canvas_realizedollar(t_canvas *x, t_symbol *s) {
    if (strchr(s->name,'$')) {
        t_canvasenvironment *env = canvas_getenv(x);
        pd_pushsym(x);
        t_symbol *ret = binbuf_realizedollsym(s, env->argc, env->argv, 1);
        pd_popsym(x);
        return ret;
    }
    return s;
}

t_symbol *canvas_getcurrentdir ()    {return canvas_getenv(canvas_getcurrent())->dir;}
t_symbol *canvas_getdir(t_canvas *x) {return canvas_getenv(                  x)->dir;}

char *canvas_makefilename(t_canvas *x, char *file, char *result, int resultsize) {
    char *dir = canvas_getenv(x)->dir->name;
    if (file[0] == '/' || (file[0] && file[1] == ':') || !*dir) {
	if (!result) return strdup(file);
        strncpy(result, file, resultsize);
        result[resultsize-1] = 0;
    } else {
        if (result) {snprintf(result,resultsize,"%s/%s",dir,file); result[resultsize-1] = 0;}
	else        asprintf(&result,           "%s/%s",dir,file);
    }
    return result;
}

static void canvas_rename(t_canvas *x, t_symbol *s, t_symbol *dir) {
    t_symbol *bs = canvas_makebindsym(x->name);
    if (x->name!=s_Pd) pd_unbind(x, bs);
    SET(name,s);
    if (x->name!=s_Pd)   pd_bind(x, bs);
    if (dir && dir != &s_) {
	canvas_getenv(x)->dir = dir;
        gobj_changed(x,"dir");
    }
}

/* --------------- traversing the set of lines in a canvas ----------- */

t_linetraverser::t_linetraverser(t_canvas *canvas) {linetraverser_start(this,canvas);}

void linetraverser_start(t_linetraverser *t, t_canvas *x) {
    t->from = 0;
    t->canvas = x;
    t->next = 0;
    t->nextoutno = t->nout = 0;
}

t_outconnect *linetraverser_next(t_linetraverser *t) {
    t_outconnect *rval = t->next;
    while (!rval) {
        int outno = t->nextoutno;
        while (outno == t->nout) {
            t_object *ob = 0;
            t_gobj *y = t->from ? t->from->next() : t->canvas->boxes->first();
            for (; y; y = y->next()) if ((ob = pd_checkobject(y))) break;
            if (!ob) return 0;
            t->from = ob;
            t->nout = obj_noutlets(ob);
            outno = 0;
        }
        t->nextoutno = outno + 1;
        rval = obj_starttraverseoutlet(t->from, &t->outletp, outno);
        t->outlet = outno;
    }
    t->next = obj_nexttraverseoutlet(rval, &t->to, &t->inletp, &t->inlet);
    t->nin = obj_ninlets(t->to);
    if (!t->nin) bug("linetraverser_next");
    return rval;
}

/* -------------------- the canvas object -------------------------- */
static int hack = 1;

static t_canvas *canvas_new2() {
    t_canvas *x = (t_canvas *)pd_new(canvas_class);
    /* zero out every field except "pd" and "dix" */
    memset(((char *)x) + sizeof(t_gobj), 0, sizeof(*x) - sizeof(t_gobj));
    x->xlabel = (t_symbol **)getbytes(0);
    x->ylabel = (t_symbol **)getbytes(0);
    // only manage this canvas if it's not one of the 3 invisible builtin canvases
    x->boxes = boxes_new();
    return x;
}

static void canvas_vis(t_canvas *x, t_floatarg f);

/* make a new canvas.  It will either be a "root" canvas or else it appears as
   a "text" object in another window (canvas_getcurrent() tells us which.) */
static t_canvas *canvas_new(void *self, t_symbol *sel, int argc, t_atom *argv) {
    t_canvas *x = canvas_new2();
    t_canvas *owner = canvas_getcurrent();
    t_symbol *s = &s_;
    int width  = CANVAS_DEFCANVASWIDTH,  xloc = 0;
    int height = CANVAS_DEFCANVASHEIGHT, yloc = CANVAS_DEFCANVASYLOC;
    int vis=0, font = owner?owner->font:10;
    if (!owner) canvas_addtolist(x);
    /* toplevel vs subwindow */
    if      (argc==5) pd_scanargs(argc,argv,"iiiii", &xloc,&yloc,&width,&height,&font);
    else if (argc==6) pd_scanargs(argc,argv,"iiiisi",&xloc,&yloc,&width,&height,&s,&vis);
    /* (otherwise assume we're being created from the menu.) */
    if (canvas_newdirectory->name[0]) {
        static long dollarzero = 1000;
        t_canvasenvironment *env = x->env = (t_canvasenvironment *)getbytes(sizeof(*x->env));
        if (!canvas_newargv) canvas_newargv = (t_atom *)getbytes(0);
        env->dir = canvas_newdirectory;
        env->argc = canvas_newargc;
        env->argv = canvas_newargv;
        env->dollarzero = dollarzero++;
        env->path = 0;
        canvas_newdirectory = &s_;
        canvas_newargc = 0;
        canvas_newargv = 0;
    } else x->env = 0;

    if (yloc < CANVAS_DEFCANVASYLOC) yloc = CANVAS_DEFCANVASYLOC;
    if (xloc < 0) xloc = 0;
    x->x1 = 0; x->y1 = 0;
    x->x2 = 1; x->y2 = 1;
    canvas_setbounds(x, xloc, yloc, xloc + width, yloc + height);
    gobj_setcanvas(x,owner);
    x->name = *s->name ? s : canvas_newfilename ? canvas_newfilename : s_Pd;
    if (x->name != s_Pd) pd_bind(x, canvas_makebindsym(x->name));
    x->goprect = 0; /* no GOP rectangle unless it's turned on later */
    /* cancel "vis" flag if we're a subpatch of an abstraction inside another patch.
       A separate mechanism prevents the toplevel abstraction from showing up. */
    if (vis && gensym("#X")->thing && gensym("#X")->thing->_class == canvas_class) {
        t_canvas *z = (t_canvas *)(gensym("#X")->thing);
        while (z && !z->env) z = z->dix->canvas;
        if (z && canvas_isabstraction(z) && z->dix->canvas) vis = 0;
    }
    if (vis) canvas_vis(x,vis);
    x->font = 10 /*sys_nearestfontsize(font)*/;
    pd_pushsym(x);
    newest = x;
    return x;
}

void canvas_setgraph(t_canvas *x, int flag, int nogoprect);

static void canvas_coords(t_canvas *x, t_symbol *s, int argc, t_atom *argv) {
    pd_scanargs(argc,argv,"ffffii*",&x->x1,&x->y1,&x->x2,&x->y2,&x->pixwidth,&x->pixheight);
    if (argc <= 7) canvas_setgraph(x, atom_getintarg(6, argc, argv), 1);
    else {
        canvas_setgraph(x, atom_getintarg(6, argc, argv), 0);
        SET(xmargin,       atom_getintarg(7, argc, argv));
        SET(ymargin,       atom_getintarg(8, argc, argv));
    }
    gobj_changed(x,0);
}

template <class T> void swap(T &a, T &b) {T c=a; a=b; b=c;}

#define CANVAS_DEFGRAPHWIDTH 200
#define CANVAS_DEFGRAPHHEIGHT 140
/* make a new canvas and add it to this canvas.  It will appear as a "graph", not a text object.  */
static t_canvas *canvas_addcanvas(t_canvas *g, t_symbol *sym,
float  x1, float  y1, float  x2, float  y2,
float px1, float py1, float px2, float py2) {
    static int gcount = 0;
    int menu = 0;
    t_canvas *x = canvas_new2();
    if (!*sym->name) {
        sym = symprintf("graph%d", ++gcount);
        menu = 1;
    } else if (!strncmp(sym->name,"graph",5)) {
	int zz = atoi(sym->name+5);
	if (zz>gcount) gcount = zz;
    }
    /* in 0.34 and earlier, the pixel rectangle and the y bounds were reversed; this would behave the same,
       except that the dialog window would be confusing.  The "correct" way is to have "py1" be the value
       that is higher on the screen. */
    if (py2 < py1) {swap(y1,y2); swap(py1,py2);}
    if (x1 == x2 || y1 == y2) {x1=0; x2=100; y1=1; y2=-1;}
    if (px1 >= px2 || py1 >= py2) {
	px1=100; px2=px1+CANVAS_DEFGRAPHWIDTH;
	py1=20;  py2=py1+CANVAS_DEFGRAPHHEIGHT;
    }
    x->name = sym;
    SET(x1,x1); SET(y1,y1); SET(x,short(px1)); SET(pixwidth ,int(px2-px1));
    SET(x2,x2); SET(y2,y2); SET(y,short(py1)); SET(pixheight,int(py2-py1));
    x->font =  (canvas_getcurrent() ? canvas_getcurrent()->font : 42 /*sys_defaultfont*/);
    x->screenx1 = x->screeny1 = 0; x->screenx2 = 450; x->screeny2 = 300;
    if (x->name != s_Pd) pd_bind(x, canvas_makebindsym(x->name));
    gobj_setcanvas(x,g);
    x->gop = 1;
    x->goprect = 0;
    x->binbuf = binbuf_new();
    binbuf_addv(x->binbuf,"t","graph");
    if (!menu) pd_pushsym(x);
    canvas_add(g,x);
    return x;
}

static void canvas_canvas(t_canvas *g, t_symbol *s, int argc, t_atom *argv) {
    t_symbol *sym;
    float x1,y1,x2,y2,px1,py1,px2,py2;
    pd_scanargs(argc,argv,"sffffffff",&sym,&x1,&y1,&x2,&y2,&px1,&py1,&px2,&py2);
    canvas_addcanvas(g, sym, x1, y1, x2, y2, px1, py1, px2, py2);
}

static void canvas_redraw(t_canvas *x) {
	gobj_changed(x,0);
	canvas_each(y,x) if (y->_class==canvas_class) canvas_redraw((t_canvas *)y); else gobj_changed(y,0);
}

/* This is sent from the GUI to inform a toplevel that its window has been moved or resized. */
static void canvas_setbounds(t_canvas *x, t_floatarg x1, t_floatarg y1, t_floatarg x2, t_floatarg y2) {
    int heightwas = int(y2-y1);
    if (x->screenx1 == x1 && x->screeny1 == y1 &&
        x->screenx2 == x2 && x->screeny2 == y2) return;
    x->screenx1 = int(x1); x->screeny1 = int(y1);
    x->screenx2 = int(x2); x->screeny2 = int(y2);
    if (!x->gop && x->y2 < x->y1) {
        /* if it's flipped so that y grows upward, fix so that zero is bottom edge and redraw.
	   This is only appropriate if we're a regular "text" object on the parent. */
        float diff = x->y1 - x->y2;
        x->y1 = heightwas * diff;
        x->y2 = x->y1 - diff;
        canvas_redraw(x);
    }
}

t_symbol *canvas_makebindsym(t_symbol *s) {return symprintf("pd-%s",s->name);}

static void canvas_vis(t_canvas *x, t_floatarg f) {
	int hadwindow = x->havewindow;
	SET(havewindow,!!f);
	if (hadwindow && !x->havewindow) gobj_unsubscribe(x,manager);
	if (!hadwindow && x->havewindow)   gobj_subscribe(x,manager);
}

/* we call this on a non-toplevel canvas to "open" it into its own window. */
static void canvas_menu_open(t_canvas *x) {
    if (canvas_isvisible(x) && !canvas_istoplevel(x)) {
        if (!x->dix->canvas) {error("this works only on subpatch or abstraction"); return;}
        SET(havewindow,1);
    }
}

int canvas_isvisible(t_canvas *x) {return gstack_empty() && canvas_getcanvas(x)->havewindow;}

/* we consider a graph "toplevel" if it has its own window or if it appears as a box in its parent window
   so that we don't draw the actual contents there. */
int canvas_istoplevel(t_canvas *x) {return x->havewindow || !x->gop;}

static void canvas_free(t_canvas *x) {
    int dspstate = canvas_suspend_dsp();
    if (canvas_whichfind == x) canvas_whichfind = 0;
    t_gobj *y;
    while ((y = x->boxes->first())) canvas_delete(x, y);
    canvas_vis(x, 0);
    if (x->name != s_Pd) pd_unbind(x,canvas_makebindsym(x->name));
    if (x->env) {
        free(x->env->argv);
        free(x->env);
    }
    canvas_resume_dsp(dspstate);
    free(x->xlabel);
    free(x->ylabel);
    if (!x->dix->canvas) canvas_takeofflist(x);
}

/* kill all lines for one inlet or outlet */
void canvas_deletelinesforio(t_canvas *x, t_text *text, t_inlet *inp, t_outlet *outp) {
    canvas_wires_each(oc,t,x)
        if ((t.from == text && t.outletp == outp) ||
            (t.to   == text &&  t.inletp ==  inp))
		obj_disconnect(t.from, t.outlet, t.to, t.inlet);
}
void canvas_deletelinesfor(t_canvas *x, t_text *text) {
    canvas_wires_each(oc,t,x)
        if (t.from == text || t.to == text)
            obj_disconnect(t.from, t.outlet, t.to, t.inlet);
}

static void canvas_resortinlets(t_canvas *x);
static void canvas_resortoutlets(t_canvas *x);
static void canvas_push(t_canvas *x, t_floatarg f) {pd_pushsym(x);}
/* assuming that this only ever gets called on toplevel canvases (?) */
static void canvas_pop(t_canvas *x, t_floatarg fvis) {
    pd_popsym(x); canvas_resortinlets(x); canvas_resortoutlets(x);
    if (fvis) canvas_vis(x, 1);
}
/* called by m_class.c */
extern "C" void canvas_popabstraction(t_canvas *x) {
    pd_set_newest(x);
    pd_popsym(x); canvas_resortinlets(x); canvas_resortoutlets(x);
}

void canvas_objfor(t_canvas *gl, t_text *x, int argc, t_atom *argv);

void canvas_restore(t_canvas *x, t_symbol *s, int argc, t_atom *argv) {
    if (argc > 3) {
        t_atom *ap=argv+3;
        if (ap->a_type == A_SYMBOL) {
            t_canvasenvironment *e = canvas_getenv(canvas_getcurrent());
            canvas_rename(x, binbuf_realizedollsym(ap->a_symbol, e->argc, e->argv, 1), 0);
        }
    }
    canvas_pop(x,0); /* 0 means "don't touch" here. */
    t_pd *z = gensym("#X")->thing;
    if (!z) {error("out of context"); return;}
    if (z->_class != canvas_class) {error("wasn't a canvas"); return;}
    gobj_setcanvas(x,(t_canvas *)z);
    canvas_objfor((t_canvas *)z, x, argc, argv);
    newest = x;
}

static void canvas_loadbang(t_canvas *x);

static void canvas_loadbangabstractions(t_canvas *x) {
    canvas_each(y,x) if (y->_class == canvas_class) {
	t_canvas *z = (t_canvas *)y;
	if (canvas_isabstraction(z)) canvas_loadbang(z); else canvas_loadbangabstractions(z);
    }
}

void canvas_loadbangsubpatches(t_canvas *x) {
    t_symbol *s = gensym("loadbang");
    canvas_each(y,x) if (y->_class == canvas_class) {
	t_canvas *z = (t_canvas *)y;
        if (!canvas_isabstraction(z)) canvas_loadbangsubpatches(z);
    }
    canvas_each(y,x) if ((y->_class != canvas_class) && zgetfn(y,s)) pd_vmess(y,s,"");
}

static void canvas_loadbang(t_canvas *x) {
    canvas_loadbangabstractions(x);
    canvas_loadbangsubpatches(x);
}

/* When you ask a canvas its size the result is 2 pixels more than what you gave it to open it;
   perhaps there's a 1-pixel border all around it or something. Anyway, we just add the 2 pixels back here;
   seems we have to do this for linux but not MSW; not sure about MacOS. */
#ifdef MSW
#define HORIZBORDER 0
#define VERTBORDER 0
#else
#define HORIZBORDER 2
#define VERTBORDER 2
#endif

static void canvas_relocate(t_canvas *x, t_symbol *canvasgeom, t_symbol *topgeom) {
    int cxpix, cypix, cw, ch, txpix, typix, tw, th;
    if (sscanf(canvasgeom->name, "%dx%d+%d+%d", &cw, &ch, &cxpix, &cypix) < 4 ||
        sscanf(   topgeom->name, "%dx%d+%d+%d", &tw, &th, &txpix, &typix) < 4)
        bug("canvas_relocate");
    /* for some reason this is initially called with cw=ch=1 so we just suppress that here. */
    if (cw>5 && ch>5) canvas_setbounds(x, txpix, typix, txpix + cw - HORIZBORDER, typix + ch - VERTBORDER);
}

void pd_set_newest (t_pd *x) {newest = x;}

static void *subcanvas_new(t_symbol *s) {
    t_atom a[6];
    t_canvas *z = canvas_getcurrent();
    if (!*s->name) s = gensym("/SUBPATCH/");
    SETFLOAT(a, 0);
    SETFLOAT(a+1, CANVAS_DEFCANVASYLOC);
    SETFLOAT(a+2, CANVAS_DEFCANVASWIDTH);
    SETFLOAT(a+3, CANVAS_DEFCANVASHEIGHT);
    SETSYMBOL(a+4, s);
    SETFLOAT(a+5, 1);
    t_canvas *x = canvas_new(0, 0, 6, a);
    gobj_setcanvas(x,z);
    canvas_pop(x, 1);
    return x;
}

static void canvas_rename_method(t_canvas *x, t_symbol *s, int ac, t_atom *av) {
    if (ac && av->a_type == A_SYMBOL) canvas_rename(x, av->a_symbol, 0);
    else if (ac && av->a_type == A_DOLLSYM) {
        t_canvasenvironment *e = canvas_getenv(x);
        pd_pushsym(x);
        canvas_rename(x, binbuf_realizedollsym(av->a_symbol, e->argc, e->argv, 1), 0);
        pd_popsym(x);
    } else canvas_rename(x, gensym("Pd"), 0);
}

/* ------------------ table ---------------------------*/

static t_garray *graph_array(t_canvas *gl, t_symbol *s, t_symbol *tmpl, t_floatarg f, t_floatarg saveit);

static int tabcount = 0;

static void *table_new(t_symbol *s, t_floatarg f) {
    t_atom a[9];
    t_canvas *z = canvas_getcurrent();
    if (s == &s_) s = symprintf("table%d", tabcount++);
    if (f <= 1) f = 100;
    SETFLOAT(a, 0);
    SETFLOAT(a+1, CANVAS_DEFCANVASYLOC);
    SETFLOAT(a+2, 600);
    SETFLOAT(a+3, 400);
    SETSYMBOL(a+4, s);
    SETFLOAT(a+5, 0);
    t_canvas *x = canvas_new(0,0,6,a);
    gobj_setcanvas(x,z);
    /* create a graph for the table */
    t_canvas *gl = canvas_addcanvas(x, &s_, 0, -1, (f > 1 ? f-1 : 1), 1, 50, 350, 550, 50);
    graph_array(gl, s, &s_float, f, 0);
    canvas_pop(x, 0);
    return x;
}

/* return true if the "canvas" object is an abstraction (so we don't save its contents, fogr example.)  */
int canvas_isabstraction(t_canvas *x) {return x->env!=0;}

/* return true if the "canvas" object is a "table". */
int canvas_istable(t_canvas *x) {
    t_atom *argv = x->binbuf ? binbuf_getvec(  x->binbuf) : 0;
    int     argc = x->binbuf ? binbuf_getnatom(x->binbuf) : 0;
    return argc && argv[0].a_type == A_SYMBOL && argv[0].a_symbol == gensym("table");
}

/* return true if the "canvas" object should be treated as a text
   object.  This is true for abstractions but also for "table"s... */
/* JMZ: add a flag to gop-abstractions to hide the title */
static int canvas_showtext(t_canvas *x) {
    t_atom *argv = x->binbuf? binbuf_getvec(  x->binbuf) : 0;
    int     argc = x->binbuf? binbuf_getnatom(x->binbuf) : 0;
    int isarray = argc && argv[0].a_type == A_SYMBOL && argv[0].a_symbol == gensym("graph");
    return x->hidetext ? 0 : !isarray;
}

/* get the document containing this canvas */
t_canvas *canvas_getrootfor(t_canvas *x) {
    if (!x->dix->canvas || canvas_isabstraction(x)) return x;
    return canvas_getrootfor(x->dix->canvas);
}

/* ------------------------- DSP chain handling ------------------------- */

typedef struct _dspcontext t_dspcontext;

extern void ugen_start ();
extern void ugen_stop ();
extern "C" t_dspcontext *ugen_start_graph(int toplevel, t_signal **sp, int ninlets, int noutlets);
extern "C" void ugen_add(t_dspcontext *dc, t_object *x);
extern "C" void ugen_connect(t_dspcontext *dc, t_object *from, int outlet, t_object *to, int inlet);
extern "C" void ugen_done_graph(t_dspcontext *dc);

/* schedule one canvas for DSP.  This is called below for all "root"
   canvases, but is also called from the "dsp" method for sub-
   canvases, which are treated almost like any other tilde object.  */
static void canvas_dodsp(t_canvas *x, int toplevel, t_signal **sp) {
    t_object *ob;
    t_symbol *dspsym = gensym("dsp");
    /* create a new "DSP graph" object to use in sorting this canvas.
       If we aren't toplevel, there are already other dspcontexts around. */
    t_dspcontext *dc = ugen_start_graph(toplevel, sp, obj_nsiginlets(x), obj_nsigoutlets(x));
    /* find all the "dsp" boxes and add them to the graph */
    canvas_each(y,x) if ((ob = pd_checkobject(y)) && zgetfn(y,dspsym)) ugen_add(dc, ob);
    /* ... and all dsp interconnections */
    canvas_wires_each(oc,t,x)
        if (obj_issignaloutlet(t.from, t.outlet))
            ugen_connect(dc, t.from, t.outlet, t.to, t.inlet);
    /* finally, sort them and add them to the DSP chain */
    ugen_done_graph(dc);
}

static void canvas_dsp(t_canvas *x, t_signal **sp) {canvas_dodsp(x, 0, sp);}

/* this routine starts DSP for all root canvases. */
static void canvas_start_dsp() {
    if (canvas_dspstate) ugen_stop();
    else sys_gui("pdtk_pd_dsp 1\n");
    ugen_start();
    //timeval v0,v1; gettimeofday(&v0,0);
    foreach(x,windowed_canvases) canvas_dodsp(x->first,1,0);
    //gettimeofday(&v1,0); printf("canvas_start_dsp took %ld us\n",(v1.tv_sec-v0.tv_sec)*1000000+(v1.tv_usec-v0.tv_usec));
    canvas_dspstate = 1;
}

/*static*/ void canvas_stop_dsp() {
    if (canvas_dspstate) {
        ugen_stop();
        sys_gui("pdtk_pd_dsp 0\n");
        canvas_dspstate = 0;
    }
}

/* DSP can be suspended before, and resumed after, operations which might affect the DSP chain.
   For example, we suspend before loading and resume afterwards, so that DSP doesn't get resorted for every DSP object in the patch. */
int canvas_suspend_dsp () {
    int rval = canvas_dspstate;
    if (rval) canvas_stop_dsp();
    return rval;
}
void canvas_resume_dsp(int oldstate) {if (oldstate) canvas_start_dsp();}
/* this is equivalent to suspending and resuming in one step. */
void canvas_update_dsp () {if (canvas_dspstate) canvas_start_dsp();}

extern "C" void glob_dsp(void *self, t_symbol *s, int argc, t_atom *argv) {
    if (argc) {
        int newstate = atom_getintarg(0, argc, argv);
        if (newstate && !canvas_dspstate) {
            canvas_start_dsp();
            sys_set_audio_state(1);
        } else if (!newstate && canvas_dspstate) {
            sys_set_audio_state(0);
            canvas_stop_dsp();
        }
    } else post("dsp state %d", canvas_dspstate);
}

extern "C" void *canvas_getblock(t_class *blockclass, t_canvas **canvasp) {
    t_canvas *canvas = *canvasp;
    void *ret = 0;
    canvas_each(g,canvas) if (g->_class == blockclass) ret = g;
    *canvasp = canvas->dix->canvas;
    return ret;
}

/******************* redrawing  data *********************/

static t_float  slot_getcoord(t_slot *f, t_template *, t_word *wp, int loud);
static void     slot_setcoord(t_slot *f, t_template *, t_word *wp, float pix, int loud);
static t_float  slot_cvttocoord(t_slot *f, float val);
static t_template *template_new(t_symbol *sym, int argc, t_atom *argv);
static void template_free(t_template *x);
static int template_match(t_template *x1, t_template *x2);
static int template_find_field(t_template *x, t_symbol*name, int*p_onset, int*p_type, t_symbol **p_arraytype);
static t_float   template_getfloat( t_template *x, t_symbol *fieldname, t_word *wp,              int loud);
static void      template_setfloat( t_template *x, t_symbol *fieldname, t_word *wp, t_float f,   int loud);
static t_symbol *template_getsymbol(t_template *x, t_symbol *fieldname, t_word *wp,              int loud);
static void      template_setsymbol(t_template *x, t_symbol *fieldname, t_word *wp, t_symbol *s, int loud);
static t_template *gtemplate_get(t_gtemplate *x);
static t_template *template_findbyname(t_symbol *s);
static t_canvas *template_findcanvas(t_template *tmpl);
static void template_notify(t_template *, t_symbol *s, int argc, t_atom *argv);

/* find the template defined by a canvas, and redraw all elements for that */
void canvas_redrawallfortemplatecanvas(t_canvas *x, int action) {
    t_template *tmpl;
    t_symbol *s1 = gensym("struct");
    canvas_each(g,x) {
        t_object *ob = pd_checkobject(g);
        if (!ob || /* ob->type != T_OBJECT || */ binbuf_getnatom(ob->binbuf) < 2) continue;
        t_atom *argv = binbuf_getvec(ob->binbuf);
        if (argv[0].a_type != A_SYMBOL || argv[1].a_type != A_SYMBOL || argv[0].a_symbol != s1)
                continue;
        tmpl = template_findbyname(argv[1].a_symbol);
        //canvas_redrawallfortemplate(tmpl, action);
    }
    //canvas_redrawallfortemplate(0, action);
}

void canvas_disconnect(t_canvas *x, float from_, float outlet, float to_, float inlet) {
    int ifrom=(int)from_, ito=int(to_);
    t_gobj *from=0, *to=0;
    canvas_each(gfrom,x) if (gfrom->dix->index == ifrom) {from=gfrom; break;}
    canvas_each(  gto,x) if (  gto->dix->index ==   ito) {  to=  gto; break;}
    if (!from || !to) goto bad;
    obj_disconnect((t_object *)from, int(outlet), (t_object *)to, int(inlet));
    return;
bad:
    post("dumb.");
}

static t_binbuf *canvas_cut_wires(t_canvas *x, t_gobj *o);
static void canvas_paste_wires(t_canvas *x, t_binbuf *buf);

/* recursively check for abstractions to reload as result of a save.
   Don't reload the one we just saved ("except") though. */
/*  LATER try to do the same trick for externs. */
static void canvas_doreload(t_canvas *gl, t_symbol *name, t_symbol *dir, t_gobj *except) {
    int i=0, nobj = gl->boxes->size();
    for (t_gobj *g = gl->boxes->first(); g && i < nobj; i++) {
	if (g != except && g->_class == canvas_class) {
	    t_canvas *c = (t_canvas *)g;
	    if (canvas_isabstraction(c) && c->name==name && canvas_getdir(c) == dir) {
		/* we're going to remake the object, so "g" will go stale. Get its index here, and afterwards restore g.
                   Also, the replacement will be at the end of the list, so we don't do g = g->next() in this case. */
		int j = g->dix->index;
		int hadwindow = gl->havewindow;
		if (!hadwindow) canvas_vis(canvas_getcanvas(gl), 1);
		t_binbuf *buf = canvas_cut_wires(gl,g);
		gl->boxes->remove(j);
		//MISSING: remake the object here.
		canvas_paste_wires(gl,buf);
		if (!hadwindow) canvas_vis(canvas_getcanvas(gl), 0);
		continue;
            }
	    canvas_doreload(c,name,dir,except);
	}
	g = g->next();
    }
}

void canvas_reload(t_symbol *name, t_symbol *dir, t_gobj *except) {
    foreach(x,windowed_canvases) canvas_doreload(x->first, name, dir, except);
}

/* ------------------------ event handling ------------------------ */

/* set a canvas up as a graph-on-parent.
   Set reasonable defaults for any missing paramters and redraw things if necessary. */
void canvas_setgraph(t_canvas *x, int flag, int nogoprect) {
    if (!flag && x->gop) {
        x->gop = 0;
    } else if (flag) {
        if (x->pixwidth  <= 0) x->pixwidth  = CANVAS_DEFGRAPHWIDTH;
        if (x->pixheight <= 0) x->pixheight = CANVAS_DEFGRAPHHEIGHT;
        SET(gop,1);
	SET(hidetext,!!(flag&2));
        if (!nogoprect && !x->goprect) canvas_each(g,x) if (pd_checkobject(g)) {SET(goprect,1); break;}
        if (canvas_isvisible(x) && x->goprect) gobj_changed(x,0);
    }
}

/* keep me */
static int canvas_isconnected (t_canvas *x, t_text *ob1, int n1, t_text *ob2, int n2) {
    canvas_wires_each(oc,t,x)
	if (t.from == ob1 && t.outlet == n1 && t.to == ob2 && t.inlet == n2) return 1;
    return 0;
}

/* ----------------------------- window stuff ----------------------- */

void canvas_close(t_canvas *x) {
    if (x->dix->canvas) canvas_vis(x, 0); else pd_free(x);
}

static int canvas_find_index1, canvas_find_index2;
static t_binbuf *canvas_findbuf;
int binbuf_match(t_binbuf *inbuf, t_binbuf *searchbuf);

/* find an atom or string of atoms */
static int canvas_dofind(t_canvas *x, int *myindex1p) {
    int myindex1 = *myindex1p, myindex2=0;
    if (myindex1 >= canvas_find_index1) {
        canvas_each(y,x) {
            t_object *ob = pd_checkobject(y);
	    if (ob && binbuf_match(ob->ob_binbuf, canvas_findbuf)) {
                    if (myindex1 > canvas_find_index1 ||
                        (myindex1 == canvas_find_index1 && myindex2 > canvas_find_index2)) {
                        canvas_find_index1 = myindex1;
                        canvas_find_index2 = myindex2;
                        vmess(x,gensym("menu-open"),"");
                        return 1;
                    }
            }
	    myindex2++;
        }
    }
    myindex2=0;
    canvas_each(y,x) {
        if (y->_class == canvas_class) {
            (*myindex1p)++;
            if (canvas_dofind((t_canvas *)y, myindex1p)) return 1;
        }
	myindex2++;
    }
    return 0;
}

static void canvas_find_parent(t_canvas *x) {
    if (x->dix->canvas) canvas_vis(canvas_getcanvas(x->dix->canvas), 1);
}

static int canvas_dofinderror(t_canvas *gl, void *error_object) {
    canvas_each(g,gl) {
        if (g==error_object) {
            /* got it... now show it. */
            canvas_vis(canvas_getcanvas(gl), 1);
            return 1;
        } else if (g->_class == canvas_class) {
            if (canvas_dofinderror((t_canvas *)g, error_object)) return 1;
        }
    }
    return 0;
}

void canvas_finderror(void *error_object) {
    foreach(x,windowed_canvases) if (canvas_dofinderror(x->first, error_object)) return;
    post("... sorry, I couldn't find the source of that error.");
}

extern t_class *text_class;
extern t_class *dummy_class;

static int is_dummy (t_text *x) {return x->_class==dummy_class;}

long canvas_base_o_index(void);

void canvas_connect(t_canvas *x, t_floatarg ffrom, t_floatarg foutlet, t_floatarg fto,t_floatarg finlet) {
    int base = canvas_base_o_index();
    int ifrom=base+int(ffrom), outlet=(int)foutlet;
    int   ito=base+int(fto),    inlet=(int)finlet;
    t_gobj *gfrom=0, *gto=0;
    t_object *from=0, *to=0;
    t_outconnect *oc;
    if (ifrom<0) goto bad;
    if (ito  <0) goto bad;
    canvas_each(zfrom,x) if (zfrom->dix->index == ifrom) {gfrom=zfrom; break;}
    if (!gfrom) goto bad;
    canvas_each(  zto,x) if (  zto->dix->index ==   ito) {  gto=  zto; break;}
    if (!gto) goto bad;
    from = pd_checkobject(gfrom);
    to   = pd_checkobject(  gto);
    if (!from || !to) goto bad;
    /* if object creation failed, make dummy inlets or outlets as needed */
    if (is_dummy(from)) while (outlet >= obj_noutlets(from)) outlet_new(from, &s_);
    if (is_dummy(to))   while ( inlet >= obj_ninlets(to)) inlet_new(to,to,&s_,&s_);
    if (!(oc = obj_connect(from,outlet,to,inlet))) goto bad;
    pd_set_newest(oc);
    gobj_setcanvas(oc,x);
    oc->dix->index = x->next_w_index++;
    return;
bad:
    post("%s %d %d %d %d (%s->%s) connection failed", x->name->name,ifrom,outlet,ito,inlet,
            from ? class_getname(from->_class) : "???",
            to   ? class_getname(  to->_class) : "???");
}

#define ARRAYPAGESIZE 1000  /* this should match the page size in u_main.tk */
/* aux routine to bash leading '#' to '$' for dialogs in u_main.tk which can't send symbols
   starting with '$' (because the Pd message interpreter would change them!) */
static t_symbol *sharptodollar(t_symbol *s) {
    if (*s->name != '#') return s;
    return symprintf("$%s",s->name+1);
}

/* --------- "pure" arrays with scalars for elements. --------------- */

/* Pure arrays have no a priori graphical capabilities.
They are instantiated by "garrays" below or can be elements of other
scalars (g_scalar.c); their graphical behavior is defined accordingly. */

t_class *array_class;

static t_array *array_new(t_symbol *templatesym, t_gpointer *parent) {
    t_array *x = (t_array *)pd_new(array_class);
    t_template *t = template_findbyname(templatesym);
    x->templatesym = templatesym;
    x->n = 1;
    x->elemsize = sizeof(t_word) * t->n;
    /* aligned allocation */
    x->vec = (char *)getalignedbytes(x->elemsize);
    /* note here we blithely copy a gpointer instead of "setting" a new one; this gpointer isn't accounted for
       and needn't be since we'll be deleted before the thing pointed to gets deleted anyway; see array_free. */
    x->gp = *parent;
    word_init((t_word *)x->vec, t, parent);
    return x;
}

void array_resize(t_array *x, int n) {
    t_template *t = template_findbyname(x->templatesym);
    if (n < 1) n = 1;
    int oldn = x->n;
    int elemsize = sizeof(t_word) * t->n;
    x->vec = (char *)resizealignedbytes(x->vec, oldn * elemsize, n * elemsize);
    x->n = n;
    if (n > oldn) {
        char *cp = x->vec + elemsize * oldn;
        for (int i = n-oldn; i--; cp += elemsize) {
            t_word *wp = (t_word *)cp;
            word_init(wp, t, &x->gp);
        }
    }
}

static void array_resize_and_redraw(t_array *array, int n) {
/* what was that for??? */
    array_resize(array,n);
    gobj_changed(array,0);
}

void word_free(t_word *wp, t_template *t);

static void array_free(t_array *x) {
    t_template *scalartemplate = template_findbyname(x->templatesym);
    for (int i=0; i < x->n; i++) word_free((t_word *)(x->vec + x->elemsize*i), scalartemplate);
    freealignedbytes(x->vec, x->elemsize * x->n);
}

/* --------------------- graphical arrays (garrays) ------------------- */

t_class *garray_class;

static t_pd *garray_arraytemplatecanvas;

/* create invisible, built-in canvases to determine the templates for floats
and float-arrays. */

void pd_eval_text2(const char *s) {pd_eval_text(s,strlen(s));}

extern "C" void garray_init () {
    hack = 0; /* invisible canvases must be, uh, invisible */
    if (garray_arraytemplatecanvas) return;
    t_binbuf *b = binbuf_new();
    glob_setfilename(0, gensym("_float"), gensym("."));
    pd_eval_text2(
	"#N canvas 0 0 458 153 10;\n"
	"#X obj 43 31 struct _float_array array z float float style float linewidth float color;\n"
	"#X obj 43 70 plot z color linewidth 0 0 1 style;\n");
    vmess(s__X.thing, gensym("pop"), "i", 0);
    glob_setfilename(0, gensym("_float_array"), gensym("."));
    pd_eval_text2(
	"#N canvas 0 0 458 153 10;\n"
	"#X obj 39 26 struct float float y;\n");
    garray_arraytemplatecanvas = s__X.thing;
    vmess(s__X.thing, gensym("pop"), "i", 0);
    glob_setfilename(0, &s_, &s_);
    binbuf_free(b);
    hack = 1; /* enable canvas visibility for upcoming canvases */
}

/* create a new scalar attached to a symbol.  Used to make floating-point
arrays (the scalar will be of type "_float_array").  Currently this is
always called by graph_array() below; but when we make a more general way
to save and create arrays this might get called more directly. */

static t_garray *graph_scalar(t_canvas *gl, t_symbol *s, t_symbol *templatesym, int saveit) {
    if (!template_findbyname(templatesym)) return 0;
    t_garray *x = (t_garray *)pd_new(garray_class);
    x->scalar = scalar_new(gl, templatesym);
    x->realname = s;
    x->realname = canvas_realizedollar(gl, s);
    pd_bind(x,x->realname);
    x->usedindsp = 0;
    x->saveit = saveit;
    x->listviewing = 0;
    canvas_add(gl,x);
    x->canvas = gl;
    return x;
}

#define TEMPLATE_CHECK(tsym,ret) if (!t) {\
	error("couldn't find template %s", tsym->name); return ret;}

#define TEMPLATE_FLOATY(a,ret) if (!a) {\
        error("%s: needs floating-point 'y' field", x->realname); return ret;}

    /* get a garray's "array" structure. */
t_array *garray_getarray(t_garray *x) {
    int zonset, ztype;
    t_symbol *zarraytype;
    t_scalar *sc = x->scalar;
    t_template *t = template_findbyname(sc->t);
    TEMPLATE_CHECK(sc->t,0)
    if (!template_find_field(t, gensym("z"), &zonset, &ztype, &zarraytype)) {
        error("template %s has no 'z' field", sc->t->name);
        return 0;
    }
    if (ztype != DT_ARRAY) {
        error("template %s, 'z' field is not an array", sc->t->name);
        return 0;
    }
    return sc->v[zonset].w_array;
}

    /* get the "array" structure and furthermore check it's float */
static t_array *garray_getarray_floatonly(t_garray *x, int *yonsetp, int *elemsizep) {
    t_array *a = garray_getarray(x);
    int yonset, type;
    t_symbol *arraytype;
    t_template *t = template_findbyname(a->templatesym);
    if (!template_find_field(t,&s_y,&yonset,&type,&arraytype) || type != DT_FLOAT)
            return 0;
    *yonsetp = yonset;
    *elemsizep = a->elemsize;
    return a;
}

/* get the array's name.  Return nonzero if it should be hidden */
int garray_getname(t_garray *x, t_symbol **namep) {
//    *namep = x->name;
    *namep = x->realname;
    return x->hidename;
}


/* if there is one garray in a graph, reset the graph's coordinates
   to fit a new size and style for the garray */
static void garray_fittograph(t_garray *x, int n, int style) {
    t_array *array = garray_getarray(x);
    t_canvas *gl = x->canvas;
    if (gl->boxes->first() == x && !x->next()) {
        vmess(gl,gensym("bounds"),"ffff",0.,gl->y1, double(style == PLOTSTYLE_POINTS || n == 1 ? n : n-1), gl->y2);
                /* close any dialogs that might have the wrong info now... */
    }
    array_resize_and_redraw(array, n);
}

/* handle "array" message to canvases; call graph_scalar above with
an appropriate template; then set size and flags.  This is called
from the menu and in the file format for patches.  LATER replace this
by a more coherent (and general) invocation. */

t_garray *graph_array(t_canvas *gl, t_symbol *s, t_symbol *templateargsym, t_floatarg fsize, t_floatarg fflags) {
    int n = (int)fsize, zonset, ztype, saveit;
    t_symbol *zarraytype;
    t_symbol *templatesym = gensym("pd-_float_array");
    int flags = (int)fflags;
    int filestyle = (flags & 6)>>1;
    int style = filestyle == 0 ? PLOTSTYLE_POLY : filestyle == 1 ? PLOTSTYLE_POINTS : filestyle;
    if (templateargsym != &s_float) {error("%s: only 'float' type understood", templateargsym->name); return 0;}
    t_template *t = template_findbyname(templatesym);
    TEMPLATE_CHECK(templatesym,0)
    if (!template_find_field(t, gensym("z"), &zonset, &ztype, &zarraytype)) {
        error("template %s has no 'z' field", templatesym->name);
        return 0;
    }
    if (ztype != DT_ARRAY) {error("template %s, 'z' field is not an array", templatesym->name); return 0;}
    t_template *ztemplate = template_findbyname(zarraytype);
    if (!ztemplate) {error("no template of type %s", zarraytype->name); return 0;}
    saveit = (flags & 1) != 0;
    t_garray *x = graph_scalar(gl, s, templatesym, saveit);
    x->hidename = (flags>>3)&1;
    if (n <= 0) n = 100;
    array_resize(x->scalar->v[zonset].w_array, n);
    template_setfloat(t, gensym("style"),     x->scalar->v, style, 1);
    template_setfloat(t, gensym("linewidth"), x->scalar->v, style==PLOTSTYLE_POINTS?2:1, 1);
    t_pd *x2 = pd_findbyclass(gensym("#A"), garray_class);
    if (x2) pd_unbind(x2,gensym("#A"));
    pd_bind(x,gensym("#A"));
    garray_redraw(x);
    return x;
}

/* find the graph most recently added to this canvas; if none exists, return 0. */
static t_canvas *canvas_findgraph(t_canvas *x) {
    t_gobj *y = 0;
    canvas_each(z,x) if (z->_class==canvas_class && ((t_canvas *)z)->gop) y = z;
    return (t_canvas *)y;
}

/* this is called back from the dialog window to create a garray. 
   The otherflag requests that we find an existing graph to put it in. */
static void canvas_arraydialog(t_canvas *parent, t_symbol *name, t_floatarg size, t_floatarg fflags, t_floatarg otherflag) {
    t_canvas *gl;
    int flags = (int)fflags;
    if (size < 1) size = 1;
    if (otherflag == 0 || !(gl = canvas_findgraph(parent)))
        gl = canvas_addcanvas(parent, &s_, 0, 1, (size>1 ? size-1 : size), -1, 0, 0, 0, 0);
    graph_array(gl, sharptodollar(name), &s_float, size, flags);
}

void garray_arrayviewlist_close(t_garray *x) {
    x->listviewing = 0;
    sys_vgui("pdtk_array_listview_closeWindow %s\n", x->realname->name);
}

/* this is called from the properties dialog window for an existing array */
void garray_arraydialog(t_garray *x, t_symbol *name, t_floatarg fsize, t_floatarg fflags, t_floatarg deleteit) {
    int flags = (int)fflags;
    int saveit = (flags&1)!=0;
    int style = (flags>>1)&3;
    float stylewas = template_getfloat(template_findbyname(x->scalar->t), gensym("style"), x->scalar->v, 1);
    if (deleteit) {canvas_delete(x->canvas,x); return;}
    t_symbol *argname = sharptodollar(name);
    t_array *a = garray_getarray(x);
    t_template *scalartemplate;
    if (!a) {error("can't find array"); return;}
    if (!(scalartemplate = template_findbyname(x->scalar->t))) {error("no template of type %s", x->scalar->t->name); return;}
    if (argname != x->realname) {
        if (x->listviewing) garray_arrayviewlist_close(x);
        x->realname = argname; /* is this line supposed to exist? */
        pd_unbind(x,x->realname);
        x->realname = canvas_realizedollar(x->canvas, argname);
        pd_bind(x,x->realname);
        gobj_changed(x,0);
    }
    int size = max(1,int(fsize));
    if (size != a->n) garray_resize(x, size);
    else if (style != stylewas) garray_fittograph(x, size, style);
    template_setfloat(scalartemplate, gensym("style"), x->scalar->v, (float)style, 0);
    garray_setsaveit(x, saveit!=0);
    garray_redraw(x);
}

void garray_arrayviewlist_new(t_garray *x) {
    char *s = x->realname->name;
    int yonset=0, elemsize=0;
    char cmdbuf[200];
    t_array *a = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!a) {error("garray_arrayviewlist_new()"); return;}
    x->listviewing = 1;
    sprintf(cmdbuf, "pdtk_array_listview_new %%s %s %d\n",s,0);
    for (int i=0; i < ARRAYPAGESIZE && i < a->n; i++) {
        float yval = *(float *)(a->vec + elemsize*i + yonset);
        sys_vgui(".%sArrayWindow.lb insert %d {%d) %g}\n",s,i,i,yval);
    }
}

void garray_arrayviewlist_fillpage(t_garray *x, t_float page, t_float fTopItem) {
    char *s = x->realname->name;
    int yonset=0, elemsize=0, topItem=(int)fTopItem;
    t_array *a = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!a) {error("garray_arrayviewlist_fillpage()"); return;}
    if (page < 0) {
      page = 0;
      sys_vgui("pdtk_array_listview_setpage %s %d\n",s,(int)page);
    } else if ((page * ARRAYPAGESIZE) >= a->n) {
      page = (int)(((int)a->n - 1)/ (int)ARRAYPAGESIZE);
      sys_vgui("pdtk_array_listview_setpage %s %d\n",s,(int)page);
    }
    sys_vgui(".%sArrayWindow.lb delete 0 %d\n",s,ARRAYPAGESIZE-1);
    for (int i = (int)page * ARRAYPAGESIZE; (i < (page+1)*ARRAYPAGESIZE && i < a->n); i++)    {
        float yval = *(float *)(a->vec + elemsize*i + yonset);
        sys_vgui(".%sArrayWindow.lb insert %d {%d) %g}\n",s,i%ARRAYPAGESIZE,i,yval);
    }
    sys_vgui(".%sArrayWindow.lb yview %d\n",s,topItem);
}

static void garray_free(t_garray *x) {
    if (x->listviewing) garray_arrayviewlist_close(x);
    pd_unbind(x,x->realname);
    /* LATER find a way to get #A unbound earlier (at end of load?) */
    t_pd *x2;
    while ((x2 = pd_findbyclass(gensym("#A"), garray_class))) pd_unbind(x2, gensym("#A"));
}

/* ------------- code used by both array and plot widget functions ---- */

static void array_redraw(t_array *a, t_canvas *canvas) {
    /* what was that for? */
    scalar_redraw(a->gp.scalar, canvas);
}

static int canvas_xtopixels(t_canvas *x, float xval);
static int canvas_ytopixels(t_canvas *x, float yval);

    /* routine to get screen coordinates of a point in an array */
static void array_getcoordinate(t_canvas *canvas, char *elem, int xonset, int yonset, int wonset, int indx,
float basex, float basey, float xinc, t_slot *xslot, t_slot *yslot, t_slot *wslot,
float *xp, float *yp, float *wp) {
    float xval, yval, ypix, wpix;
    if (xonset >= 0) xval = *(float *)(elem + xonset); else xval = indx * xinc;
    if (yonset >= 0) yval = *(float *)(elem + yonset); else yval = 0;
    ypix = canvas_ytopixels(canvas, basey + slot_cvttocoord(yslot, yval));
    if (wonset >= 0) {
        /* found "w" field which controls linewidth. */
        float wval = *(float *)(elem + wonset);
        wpix = canvas_ytopixels(canvas, basey + slot_cvttocoord(yslot,yval) + slot_cvttocoord(wslot,wval)) - ypix;
        if (wpix < 0) wpix = -wpix;
    } else wpix = 1;
    *xp = canvas_xtopixels(canvas, basex + slot_cvttocoord(xslot, xval));
    *yp = ypix;
    *wp = wpix;
}

static struct {
  float xcumulative, ycumulative;
  t_slot *xfield, *yfield;
  t_canvas *canvas;
  t_scalar *scalar;
  t_array *array;
  t_word *wp;
  t_template *t;
  int npoints, elemsize;
  float initx, xperpix, yperpix;
  int lastx, fatten;
} ammo;

/* LATER protect against the template changing or the scalar disappearing
   probably by attaching a gpointer here ... */
#if 0
static void array_motion(void *z, t_floatarg dx, t_floatarg dy) {
    ammo.xcumulative += dx * ammo.xperpix;
    ammo.ycumulative += dy * ammo.yperpix;
    if (ammo.xfield) {// xy plot
        for (int i=0; i<ammo.npoints; i++) {
            t_word *thisword = (t_word *)(((char *)ammo.wp) + i*ammo.elemsize);
            float xwas = slot_getcoord(ammo.xfield, ammo.t, thisword, 1);
            float ywas = ammo.yfield ? slot_getcoord(ammo.yfield, ammo.t, thisword, 1) : 0;
            slot_setcoord(ammo.xfield, ammo.t, thisword, xwas + dx, 1);
            if (ammo.yfield) {
                if (ammo.fatten) {
                    if (i == 0) {
                        float newy = max(0.f,ywas+dy*ammo.yperpix);
                        slot_setcoord(ammo.yfield, ammo.t, thisword, newy, 1);
                    }
                } else slot_setcoord(ammo.yfield, ammo.t, thisword, ywas + dy*ammo.yperpix, 1);
            }
        }
    } else if (ammo.yfield) {// y plot
        int thisx = int(ammo.initx + ammo.xcumulative + 0.5), x2;
        int increment, nchange;
        float newy = ammo.ycumulative;
	float oldy = slot_getcoord(ammo.yfield, ammo.t, (t_word *)(((char *)ammo.wp) + ammo.elemsize * ammo.lastx), 1);
        float ydiff = newy-oldy;
        CLAMP(thisx,0,1);
        increment = thisx > ammo.lastx ? -1 : 1;
        nchange = 1 + increment * (ammo.lastx - thisx);
        x2 = thisx;
        for (int i=0; i<nchange; i++, x2 += increment) {
            slot_setcoord(ammo.yfield, ammo.t, (t_word *)(((char *)ammo.wp) + ammo.elemsize * x2), newy, 1);
            if (nchange > 1) newy -= ydiff/(nchange-1);
        }
        ammo.lastx = thisx;
    }
    if (ammo.scalar) scalar_redraw(ammo.scalar, ammo.canvas);
    if (ammo.array)   array_redraw(ammo.array,  ammo.canvas);
}
#endif

int scalar_doclick(t_word *data, t_template *t, t_scalar *sc, t_array *ap, t_canvas *owner, float xloc, float yloc,
int xpix, int ypix, int shift, int alt, int dbl, int doit);

static int array_getfields(t_symbol *elemtemplatesym, t_canvas **elemtemplatecanvasp,
t_template **elemtemplatep, int *elemsizep, t_slot *xslot, t_slot *yslot, t_slot *wslot,
int *xonsetp, int *yonsetp, int *wonsetp);

/* try clicking on an element of the array as a scalar (if clicking
   on the trace of the array failed) */
static int array_doclick_element(t_array *array, t_canvas *canvas, t_scalar *sc, t_array *ap,
t_symbol *elemtemplatesym, float linewidth, float xloc, float xinc, float yloc,
t_slot *xfield, t_slot *yfield, t_slot *wfield, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    t_canvas *elemtemplatecanvas;
    t_template *elemtemplate;
    int elemsize, yonset, wonset, xonset, incr;
    //float xsum=0;
    if (elemtemplatesym == &s_float) return 0;
    if (array_getfields(elemtemplatesym, &elemtemplatecanvas,
        &elemtemplate, &elemsize, xfield, yfield, wfield, &xonset, &yonset, &wonset))
                return 0;
    /* if it has more than 2000 points, just check 300 of them. */
    if (array->n < 2000) incr=1; else incr = array->n/300;
    for (int i=0; i < array->n; i += incr) {
	//float usexloc = xonset>=0 ? xloc + slot_cvttocoord(xfield, *(float *)&array->vec[elemsize*i+xonset]) : xloc + xsum;
        //if (xonset>=0) xsum += xinc;
        //float useyloc = yloc + (yonset>=0 ? slot_cvttocoord(yfield, *(float *)&array->vec[elemsize*i+yonset]):0);
        int hit = 0;
	/* hit = scalar_doclick((t_word *)&array->vec[elemsize*i],
            elemtemplate, 0, array, canvas, usexloc, useyloc, xpix, ypix, shift, alt, dbl, doit);*/
	if (hit) return hit;
    }
    return 0;
}

static float canvas_pixelstox(t_canvas *x, float xpix);
static float canvas_pixelstoy(t_canvas *x, float xpix);

/* convert an X screen distance to an X coordinate increment. */
static float canvas_dpixtodx(t_canvas*x,float dxpix){return dxpix*(canvas_pixelstox(x,1)-canvas_pixelstox(x,0));}
static float canvas_dpixtody(t_canvas*x,float dypix){return dypix*(canvas_pixelstoy(x,1)-canvas_pixelstoy(x,0));}

/* LATER move this and others back into plot parentwidget code, so
   they can be static (look in g_canvas.h for candidates). */
int array_doclick(t_array *array, t_canvas *canvas, t_scalar *sc, t_array *ap,
t_symbol *elemtemplatesym, float linewidth, float xloc, float xinc, float yloc, float scalarvis,
t_slot *xfield, t_slot *yfield, t_slot *wfield, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    t_canvas *elemtemplatecanvas;
    t_template *elemtemplate;
    int elemsize, yonset, wonset, xonset;
    if (!array_getfields(elemtemplatesym, &elemtemplatecanvas, &elemtemplate, &elemsize,
	xfield, yfield, wfield, &xonset, &yonset, &wonset)) {
        float best = 100;
        /* if it has more than 2000 points, just check 1000 of them. */
        int incr = (array->n <= 2000 ? 1 : array->n / 1000);
        for (int i=0; i < array->n; i += incr) {
            float pxpix, pypix, pwpix, dx, dy;
            array_getcoordinate(canvas, &array->vec[elemsize*i], xonset, yonset, wonset, i,
		xloc, yloc, xinc, xfield, yfield, wfield, &pxpix, &pypix, &pwpix);
            if (pwpix < 4) pwpix = 4;
            dx = fabs(pxpix-xpix); if (dx>8) continue;
            dy = fabs(pypix-ypix); if (dx+dy<best) best=dx+dy;
            if (wonset >= 0) {
                dy = fabs(pypix+pwpix-ypix); if (dx+dy < best) best = dx+dy;
                dy = fabs(pypix-pwpix-ypix); if (dx+dy < best) best = dx+dy;
            }
        } if (best > 8) {
            if (scalarvis != 0) return array_doclick_element(array, canvas, sc, ap, elemtemplatesym,
		linewidth, xloc, xinc, yloc, xfield, yfield, wfield, xpix, ypix, shift, alt, dbl, doit);
            return 0;
        }
        best += 0.001;  /* add truncation error margin */
        for (int i=0; i < array->n; i += incr) {
            float pxpix, pypix, pwpix, dx, dy, dy2, dy3;
            array_getcoordinate(canvas, &array->vec[elemsize*i], xonset, yonset, wonset, i,
		xloc, yloc, xinc, xfield, yfield, wfield, &pxpix, &pypix, &pwpix);
            if (pwpix < 4) pwpix = 4;
            dx = fabs(pxpix-xpix);
            dy = fabs(pypix-ypix);
            if (wonset >= 0) {
                dy2 = fabs(pypix+pwpix-ypix);
                dy3 = fabs(pypix-pwpix-ypix);
                if (yonset < 0) dy = 100;
            } else dy2 = dy3 = 100;
            if (dx + dy <= best || dx + dy2 <= best || dx + dy3 <= best) {
                if (dy<dy2 && dy<dy3) ammo.fatten = 0;
                else if (dy2<dy3)     ammo.fatten = -1;
                else                  ammo.fatten = 1;
                if (doit) {
                    char *elem = array->vec;
                    ammo.elemsize = elemsize;
                    ammo.canvas = canvas;
                    ammo.scalar = sc;
                    ammo.array = ap;
                    ammo.t = elemtemplate;
                    ammo.xperpix = canvas_dpixtodx(canvas, 1);
                    ammo.yperpix = canvas_dpixtody(canvas, 1);
                    if (alt && xpix < pxpix) { /* delete a point */
                        if (array->n <= 1) return 0;
                        memmove(&array->vec[elemsize*i], &array->vec[elemsize*(i+1)], (array->n-1-i) * elemsize);
                        array_resize_and_redraw(array, array->n - 1);
                        return 0;
                    } else if (alt) {
                        /* add a point (after the clicked-on one) */
                        array_resize_and_redraw(array, array->n + 1);
                        elem = array->vec;
                        memmove(elem + elemsize * (i+1), elem + elemsize*i, (array->n-i-1) * elemsize);
                        i++;
                    }
                    if (xonset >= 0) {
                        ammo.xfield = xfield;
                        ammo.xcumulative = slot_getcoord(xfield,ammo.t,(t_word *)(elem+elemsize*i),1);
                            ammo.wp = (t_word *)(elem + elemsize*i);
                        if (shift) ammo.npoints = array->n - i;
                        else ammo.npoints = 1;
                    } else {
                        ammo.xfield = 0;
                        ammo.xcumulative = 0;
                        ammo.wp = (t_word *)elem;
                        ammo.npoints = array->n;
                        ammo.initx = i;
                        ammo.lastx = i;
                        ammo.xperpix *= (xinc == 0 ? 1 : 1./xinc);
                    }
                    if (ammo.fatten) {
                        ammo.yfield = wfield;
                        ammo.ycumulative = slot_getcoord(wfield,ammo.t,(t_word *)(elem+elemsize*i),1);
                        ammo.yperpix *= -ammo.fatten;
                    } else if (yonset >= 0) {
                        ammo.yfield = yfield;
                        ammo.ycumulative = slot_getcoord(yfield,ammo.t,(t_word *)(elem+elemsize*i),1);
                    } else {
                        ammo.yfield = 0;
                        ammo.ycumulative = 0;
                    }
                    /* canvas_grab(canvas, 0, array_motion, 0, xpix, ypix); */
                }
		return 0;
            }
        }
    }
    return 0;
}

static void garray_save(t_gobj *z, t_binbuf *b) {
    t_garray *x = (t_garray *)z;
    t_array *array = garray_getarray(x);
    t_template *scalartemplate;
    /* LATER "save" the scalar as such */
    if (x->scalar->t != gensym("pd-_float_array")) {error("can't save arrays of type %s yet", x->scalar->t->name); return;}
    if (!(scalartemplate = template_findbyname(x->scalar->t))) {error("no template of type %s", x->scalar->t->name); return;}
    int style = (int)template_getfloat(scalartemplate, gensym("style"), x->scalar->v, 0);
    int filestyle = (style == PLOTSTYLE_POINTS ? 1 : (style == PLOTSTYLE_POLY ? 0 : style));
    binbuf_addv(b, "ttsisi;","#X","array", x->realname, array->n, &s_float, x->saveit+2*filestyle+8*x->hidename);
    if (x->saveit) {
        int n = array->n, n2 = 0;
        while (n2 < n) {
            int chunk = imin(n-n2,1000);
            binbuf_addv(b,"ti","#A",n2);
            for (int i=0; i<chunk; i++) binbuf_addv(b, "f", ((float *)array->vec)[n2+i]);
            binbuf_addv(b, ";");
            n2 += chunk;
        }
    }
}

/* required by d_array.c and d_soundfile */
void garray_redraw(t_garray *x) {gobj_changed(x,0);}

/* those three required by d_array.c */
void garray_usedindsp(t_garray *x) {x->usedindsp = 1;}
int garray_npoints(t_garray *x) {return         garray_getarray(x)->n;}   /* get the length */
char *garray_vec(t_garray *x)   {return (char *)garray_getarray(x)->vec;} /* get the contents */

/* routine that checks if we're just an array of floats and if so returns the goods */
int garray_getfloatarray(t_garray *x, int *size, t_float **vec) {
    int yonset, elemsize;
    t_array *a = garray_getarray_floatonly(x, &yonset, &elemsize);
    TEMPLATE_FLOATY(a,0)
    if (elemsize != sizeof(t_word)) {error("%s: has more than one field", x->realname); return 0;}
    *size = garray_npoints(x);
    *vec =  (float *)garray_vec(x);
    return 1;
}

/* set the "saveit" flag */
void garray_setsaveit(t_garray *x, int saveit) {
    if (x->saveit && !saveit) post("warning: array %s: clearing save-in-patch flag", x->realname->name);
    x->saveit = saveit;
}

static void garray_const(t_garray *x, t_floatarg g) {
    int yonset, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    TEMPLATE_FLOATY(array,)
    for (int i=0; i<array->n; i++) *((float *)(array->vec + elemsize*i) + yonset) = g;
    garray_redraw(x);
}

/* sum of Fourier components; called from functions below */
static void garray_dofo(t_garray *x, int npoints, float dcval, int nsin, t_float *vsin, int sineflag) {
    double phase, fj;
    int yonset, i, j, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    TEMPLATE_FLOATY(array,)
    if (npoints == 0) npoints = 512;  /* dunno what a good default would be... */
    if (npoints != (1 << ilog2(npoints)))
        post("%s: rounnding to %d points", array->templatesym->name, (npoints = (1<<ilog2(npoints))));
    garray_resize(x, npoints + 3);
    double phaseincr = 2. * 3.14159 / npoints;
    for (i=0, phase = -phaseincr; i < array->n; i++, phase += phaseincr) {
        double sum = dcval;
        if (sineflag) for (j=0, fj=phase; j<nsin; j++, fj+=phase) sum += vsin[j] * sin(fj);
        else          for (j=0, fj=    0; j<nsin; j++, fj+=phase) sum += vsin[j] * cos(fj);
        *((float *)(array->vec + elemsize*i) + yonset) = sum;
    }
    garray_redraw(x);
}

static void garray_sinesum(t_garray *x, t_symbol *s, int argc, t_atom *argv) {
    if (argc < 2) {error("%s: need number of points and partial strengths", x->realname->name); return;}
    t_float *svec = (t_float *)getbytes(sizeof(t_float) * argc);
    int npoints = atom_getintarg(0,argc--,argv++);
    argv++, argc--; /* is it normal that this happens a second time? */
    for (int i=0; i < argc; i++) svec[i] = atom_getfloatarg(i, argc, argv);
    garray_dofo(x, npoints, 0, argc, svec, 1);
    free(svec);
}
static void garray_cosinesum(t_garray *x, t_symbol *s, int argc, t_atom *argv) {
    if (argc < 2) {error("%s: need number of points and partial strengths", x->realname->name); return;}
    t_float *svec = (t_float *)getbytes(sizeof(t_float) * argc);
    int npoints = atom_getintarg(0,argc--,argv++);
    for (int i=0; i < argc; i++) svec[i] = atom_getfloatarg(i, argc, argv);
    garray_dofo(x, npoints, 0, argc, svec, 0);
    free(svec);
}

static void garray_normalize(t_garray *x, t_float f) {
    double maxv=0;
    int yonset, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    TEMPLATE_FLOATY(array,)
    if (f <= 0) f = 1;
    for (int i=0; i < array->n; i++) {
        double v = *((float *)(array->vec + elemsize*i) + yonset);
        if ( v > maxv) maxv =  v;
        if (-v > maxv) maxv = -v;
    }
    if (maxv > 0) {
	double renormer = f/maxv;
        for (int i=0; i < array->n; i++) *((float *)(array->vec + elemsize*i) + yonset) *= renormer;
    }
    garray_redraw(x);
}

/* list: the first value is an index; subsequent values are put in the "y" slot of the array. */
static void garray_list(t_garray *x, t_symbol *s, int argc, t_atom *argv) {
    int yonset, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    TEMPLATE_FLOATY(array,)
    if (argc < 2) return;
    else {
        int firstindex = atom_getintarg(0,argc--,argv++);
        if (firstindex < 0) { /* drop negative x values */
            argc += firstindex;
            argv -= firstindex;
            firstindex = 0;
        }
        if (argc + firstindex > array->n) argc = array->n - firstindex;
        for (int i=0; i < argc; i++)
            *((float *)(array->vec + elemsize * (i + firstindex)) + yonset) = atom_getfloat(argv + i);
    }
    garray_redraw(x);
}

static void garray_bounds(t_garray *x, t_floatarg x1, t_floatarg y1, t_floatarg x2, t_floatarg y2)
{vmess(x->canvas, gensym("bounds"), "ffff", x1, y1, x2, y2);}
static void garray_xticks(t_garray *x, t_floatarg point, t_floatarg inc, t_floatarg f)
{vmess(x->canvas, gensym("xticks"), "fff", point, inc, f);}
static void garray_yticks(t_garray *x, t_floatarg point, t_floatarg inc, t_floatarg f)
{vmess(x->canvas, gensym("yticks"), "fff", point, inc, f);}
static void garray_xlabel(t_garray *x, t_symbol *s, int argc, t_atom *argv) {typedmess(x->canvas, s, argc, argv);}
static void garray_ylabel(t_garray *x, t_symbol *s, int argc, t_atom *argv) {typedmess(x->canvas, s, argc, argv);}

static void garray_rename(t_garray *x, t_symbol *s) {
    if (x->listviewing) garray_arrayviewlist_close(x);
    pd_unbind(x,x->realname);
    x->realname = s;
    pd_bind(x,x->realname);
    garray_redraw(x);
}

static void garray_read(t_garray *x, t_symbol *filename) {
    FILE *fd;
    char *buf, *bufptr;
    int yonset, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    TEMPLATE_FLOATY(array,)
    int nelem = array->n;
    int filedesc = canvas_open2(canvas_getcanvas(x->canvas), filename->name, "", &buf, &bufptr, 0);
    if (filedesc<0)                    {error("%s: can't open", filename->name); free(buf); return;}
    if (!(fd = fdopen(filedesc, "r"))) {error("%s: can't open", filename->name); free(buf); return;}
    int i;
    for (i=0; i < nelem; i++) {
        if (!fscanf(fd, "%f", (float *)(array->vec + elemsize*i) + yonset)) {
            post("%s: read %d elements into table of size %d", filename->name, i, nelem);
            break;
        }
    }
    while (i < nelem) *((float *)(array->vec + elemsize*i) + yonset) = 0, i++;
    fclose(fd);
    garray_redraw(x);
    free(buf);
}

static void garray_write(t_garray *x, t_symbol *filename) {
    int yonset, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    TEMPLATE_FLOATY(array,)
    char *buf = canvas_makefilename(canvas_getcanvas(x->canvas),filename->name,0,0);
    sys_bashfilename(buf, buf);
    FILE *fd = fopen(buf, "w");
    if (!fd) {error("can't create file '%s'", buf); free(buf); return;}
    free(buf);
    for (int i=0; i < array->n; i++) {
        if (fprintf(fd, "%g\n", *(float *)(((array->vec + sizeof(t_word) * i)) + yonset)) < 1) {
            post("%s: write error", filename->name);
            break;
        }
    }
    fclose(fd);
}

/* d_soundfile.c uses this! */
int garray_ambigendian () {
    unsigned short s = 1;
    unsigned char c = *(char *)(&s);
    return c==0;
}

/* d_soundfile.c uses this! */
void garray_resize(t_garray *x, t_floatarg f) {
    t_array *array = garray_getarray(x);
    int n = f<1?1:(int)f;
    garray_fittograph(x, n, (int)template_getfloat(template_findbyname(x->scalar->t), gensym("style"), x->scalar->v, 1));
    array_resize_and_redraw(array, n);
    if (x->usedindsp) canvas_update_dsp();
}

static void garray_print(t_garray *x) {
    t_array *array = garray_getarray(x);
    post("garray %s: template %s, length %d", x->realname->name, array->templatesym->name, array->n);
}

static void g_array_setup() {
    t_class *c = garray_class = class_new2("array",0,garray_free,sizeof(t_garray),CLASS_GOBJ,"");
    class_addlist(garray_class, garray_list);
    class_addmethod2(c, garray_const, "const", "F");
    class_addmethod2(c, garray_bounds, "bounds", "ffff");
    class_addmethod2(c, garray_xticks, "xticks", "fff");
    class_addmethod2(c, garray_xlabel, "xlabel", "*");
    class_addmethod2(c, garray_yticks, "yticks", "fff");
    class_addmethod2(c, garray_ylabel, "ylabel", "*");
    class_addmethod2(c, garray_rename, "rename", "s");
    class_addmethod2(c, garray_read, "read", "s");
    class_addmethod2(c, garray_write, "write", "s");
    class_addmethod2(c, garray_resize, "resize", "f");
    class_addmethod2(c, garray_print, "print", "");
    class_addmethod2(c, garray_sinesum, "sinesum", "*");
    class_addmethod2(c, garray_cosinesum, "cosinesum", "*");
    class_addmethod2(c, garray_normalize, "normalize", "F");
    class_addmethod2(c, garray_arraydialog, "arraydialog", "sfff");
    class_addmethod2(c, garray_arrayviewlist_new, "arrayviewlistnew", "");
    class_addmethod2(c, garray_arrayviewlist_fillpage, "arrayviewlistfillpage", "fF");
    class_addmethod2(c, garray_arrayviewlist_close, "arrayviewclose", "");
    class_setsavefn(c, garray_save);
    array_class = class_new2("array_really",0,array_free,sizeof(t_array),CLASS_GOBJ,"");
}

static void graph_graphrect(t_gobj *z, t_canvas *canvas, int *xp1, int *yp1, int *xp2, int *yp2);

void canvas_add_debug(t_canvas *x, t_gobj *y) {
    if (!y->_class->patchable) {
	printf("canvas_add %p %p class=%s (non-t_text)\n",x,y,y->_class->name->name);
    } else {
	t_binbuf *bb = ((t_text *)y)->binbuf;
	if (binbuf_getvec(bb)) {
            char *buf; int bufn;
	    binbuf_gettext(bb,&buf,&bufn);
	    printf("canvas_add %p %p [%.*s]\n",x,y,bufn,buf);
	    free(buf);
	} else {
	    printf("canvas_add %p %p class=%s (binbuf without b_vec !)\n",x,y,y->_class->name->name);
	}
    }
}

void canvas_add(t_canvas *x, t_gobj *y, int index) {
    gobj_setcanvas(y,x);
    if (index<0) y->dix->index = x->next_o_index++;
    else         y->dix->index = index;
    x->boxes->add(y);
    if (x->gop && !x->goprect && pd_checkobject(y)) SET(goprect,1);
    //if (class_isdrawcommand(y->_class)) canvas_redrawallfortemplate(template_findbyname(canvas_makebindsym(canvas_getcanvas(x)->name)), 0);
}

/* delete an object from a canvas and free it */
void canvas_delete(t_canvas *x, t_gobj *y) {
    bool chkdsp = !!zgetfn(y,gensym("dsp"));
    //int drawcommand = class_isdrawcommand(y->_class);
    /* if we're a drawing command, erase all scalars now, before deleting it; we'll redraw them once it's deleted below. */
    //if (drawcommand) canvas_redrawallfortemplate(template_findbyname(canvas_makebindsym(canvas_getcanvas(x)->name)), 2);
    canvas_deletelinesfor(x,(t_text *)y);
    x->boxes->remove_by_value(y);
    /* BUG: should call gobj_onsubscribe here, to flush the zombie */
    pd_free(y);
    if (chkdsp) canvas_update_dsp();
    //if (drawcommand) canvas_redrawallfortemplate(template_findbyname(canvas_makebindsym(canvas_getcanvas(x)->name)), 1);
}

static void canvas_clear(t_canvas *x) {
    t_gobj *y;
    int dspstate = 0, suspended = 0;
    t_symbol *dspsym = gensym("dsp");
    /* to avoid unnecessary DSP resorting, we suspend DSP only if we find a DSP object. */
    canvas_each(y,x) if (!suspended && pd_checkobject(y) && zgetfn(y,dspsym)) {dspstate = canvas_suspend_dsp(); suspended=1;}
    while ((y = x->boxes->first())) x->boxes->remove_by_value(y);
    if (suspended) canvas_resume_dsp(dspstate);
}


t_canvas *canvas_getcanvas(t_canvas *x) {
    while (x->dix->canvas && !x->havewindow && x->gop) x = x->dix->canvas;
    return x;
}

static void scalar_getbasexy(t_scalar *x, float *basex, float *basey);

static float gobj_getxforsort(t_gobj *g) {
    if (g->_class!=scalar_class) return 0;
    float x1, y1;
    scalar_getbasexy((t_scalar *)g, &x1, &y1);
    return x1;
}

static t_gobj *canvas_merge(t_canvas *x, t_gobj *g1, t_gobj *g2) {
/*
    t_gobj *g = 0, *g9 = 0;
    float f1 = g1 ? gobj_getxforsort(g1) : 0;
    float f2 = g2 ? gobj_getxforsort(g2) : 0;
    while (1) {
	if (g1 && !(g2 && f1>f2)) {
	        if (g9) {g9->g_next = g1; g9 = g1;} else g9 = g = g1;
	        if ((g1 = g1->next())) f1 = gobj_getxforsort(g1);
	        g9->g_next = 0;
	        continue;
	}
	if (g1 || g2) {
	        if (g9) {g9->g_next = g2; g9 = g2;} else g9 = g = g2;
	        if ((g2 = g2->next())) f2 = gobj_getxforsort(g2);
	        g9->g_next = 0;
	        continue;
	}
        break;
    }
    return g;
*/
}

/*
static t_gobj *canvas_dosort(t_canvas *x, t_gobj *g, int nitems) {
    t_gobj *g2, *g3;
    int n1 = nitems/2, n2 = nitems - n1, i;
    if (nitems < 2) return g;
    int i=n1-1;
    for (g2 = g; i--; g2 = g2->next()) {}
    g3 = g2->next();
    g2->g_next = 0;
    g = canvas_dosort(x, g, n1);
    g3 = canvas_dosort(x, g3, n2);
    return canvas_merge(x, g, g3);
}

void canvas_sort(t_canvas *x) {
    int nitems = 0, foo = 0;
    float lastx = -1e37;
    canvas_each(g,x) {
        float x1 = gobj_getxforsort(g);
        if (x1 < lastx) foo = 1;
        lastx = x1;
        nitems++;
    }
    if (foo) x->list = canvas_dosort(x, x->list, nitems);
}
*/

static t_inlet *canvas_addinlet(t_canvas *x, t_pd *who, t_symbol *s, t_symbol* h) {
    t_inlet *ip = inlet_new(x,who,s,0); inlet_settip(ip,h);
    if (gstack_empty()) canvas_resortinlets(x);
    gobj_changed(x,0); return ip;
}
static t_outlet *canvas_addoutlet(t_canvas *x, t_pd *who, t_symbol *s) {
    t_outlet *op = outlet_new(x,s);
    if (gstack_empty()) canvas_resortoutlets(x);
    gobj_changed(x,0); return op;
}

static void canvas_rminlet(t_canvas *x, t_inlet *ip) {
    if (x->dix->canvas) canvas_deletelinesforio(x->dix->canvas,x,ip,0);
    inlet_free(ip); /*gobj_changed(x,0);*/
}
static void canvas_rmoutlet(t_canvas *x, t_outlet *op) {
    if (x->dix->canvas) canvas_deletelinesforio(x->dix->canvas,x,0,op);
    outlet_free(op); /*gobj_changed(x,0);*/
}

extern "C" t_inlet  *vinlet_getit(t_pd *x);
extern "C" t_outlet *voutlet_getit(t_pd *x);

typedef int (*t_order)(const void *, const void *);
int gobj_order_x (t_object **a, t_object **b) {return (*a)->x - (*b)->x;}

//{std::ostringstream s; s<<"disorder:"; for (int i=0; i<n; i++) s<<" "<<vec[i]->x; post("%s",s.str().data());}

void obj_moveinletfirst(t_object *x, t_inlet *i);
void obj_moveoutletfirst(t_object *x, t_outlet *o);

static void canvas_resortinlets(t_canvas *x) {
    int n=0; canvas_each(y,x) if (y->_class==vinlet_class) n++;
    t_object **vec = new t_object *[n], **vp = vec;
    canvas_each(y,x) if (y->_class==vinlet_class) *vp++ = (t_object *)y;
    qsort(vec,n,sizeof(t_object *),(t_order)gobj_order_x);
    for (int i=n; i--;) obj_moveinletfirst(x,vinlet_getit(vec[i]));
    delete[] vec;
}
static void canvas_resortoutlets(t_canvas *x) {
    int n=0; canvas_each(y,x) if (y->_class==voutlet_class) n++;
    t_object **vec = new t_object *[n], **vp = vec;
    canvas_each(y,x) if (y->_class==voutlet_class) *vp++ = (t_object *)y;
    qsort(vec,n,sizeof(t_object *),(t_order)gobj_order_x);
    for (int i=n; i--;) obj_moveoutletfirst(x,voutlet_getit(vec[i]));
    delete[] vec;
}

static void graph_bounds(t_canvas *x, t_floatarg x1, t_floatarg y1, t_floatarg x2, t_floatarg y2) {
    x->x1 = x1; x->y1 = y1;
    x->x2 = x2; x->y2 = y2;
    if (x->x2 == x->x1 || x->y2 == x->y1) {
        error("empty bounds rectangle");
        x->x1 = x->y1 = 0;
        x->x2 = x->y2 = 1;
    }
    gobj_changed(x,0);
}

static void graph_xticks(t_canvas *x, t_floatarg point, t_floatarg inc, t_floatarg f)
{t_tick *t = &x->xtick; t->point = point; t->inc = inc; t->lperb = (int)f; gobj_changed(x,"xticks");}
static void graph_yticks(t_canvas *x, t_floatarg point, t_floatarg inc, t_floatarg f)
{t_tick *t = &x->ytick; t->point = point; t->inc = inc; t->lperb = (int)f; gobj_changed(x,"yticks");}

static void graph_xlabel(t_canvas *x, t_symbol *s, int argc, t_atom *argv) {
    if (argc < 1) {error("graph_xlabel: no y value given"); return;}
    x->xlabely = atom_getfloatarg(0,argc--,argv++);
    x->xlabel = (t_symbol **)realloc(x->xlabel,argc*sizeof(void*));
    x->nxlabels = argc;
    for (int i=0; i < argc; i++) x->xlabel[i] = atom_gensym(&argv[i]);
    gobj_changed(x,"xlabel");
}
static void graph_ylabel(t_canvas *x, t_symbol *s, int argc, t_atom *argv) {
    if (argc < 1) {error("graph_ylabel: no x value given"); return;}
    x->ylabelx = atom_getfloatarg(0,argc--,argv++);
    x->ylabel = (t_symbol **)realloc(x->ylabel,argc*sizeof(void*));
    x->nylabels = argc;
    for (int i=0; i < argc; i++) x->ylabel[i] = atom_gensym(&argv[i]);
    gobj_changed(x,"ylabel");
}

/* if we appear as a text box on parent, our range in our coordinates (x1, etc.)
   specifies the coordinate range of a one-pixel square at top left of the window.
   if we're a graph when shown on parent, but own our own window right now, our range
   in our coordinates (x1, etc.) is spread over the visible window size, given by screenx1, etc.
   otherwise, we appear in a graph within a parent canvas, so get our screen rectangle on parent and transform. */
static float canvas_pixelstox(t_canvas *x, float xpix) {
    int x1, y1, x2, y2; float width = x->x2-x->x1;
    if (!x->gop)       return x->x1 + width * xpix;
    if (x->havewindow) return x->x1 + width * xpix / (x->screenx2-x->screenx1);
    graph_graphrect(x, x->dix->canvas, &x1, &y1, &x2, &y2);
    return x->x1 + width * (xpix-x1) / (x2-x1);
}
static float canvas_pixelstoy(t_canvas *x, float ypix) {
    int x1, y1, x2, y2; float height = x->y2-x->y1;
    if (!x->gop)       return x->y1 + height * ypix;
    if (x->havewindow) return x->y1 + height * ypix / (x->screeny2-x->screeny1);
    graph_graphrect(x, x->dix->canvas, &x1, &y1, &x2, &y2);
    return x->y1 + height * (ypix-y1) / (y2-y1);
}

/* convert an x coordinate value to an x pixel location in window */
static int canvas_xtopixels(t_canvas *x, float xval) {
    int x1, y1, x2, y2; float width = x->x2-x->x1;
    if (!x->gop)       return int((xval-x->x1)/width);
    if (x->havewindow) return int((x->screenx2-x->screenx1) * (xval-x->x1) / width);
    graph_graphrect(x, x->dix->canvas, &x1, &y1, &x2, &y2);
    return int(x1 + (x2-x1) * (xval-x->x1) / width);
}
static int canvas_ytopixels(t_canvas *x, float yval) {
    int x1, y1, x2, y2; float height = x->y2-x->y1;
    if (!x->gop)       return int((yval-x->y1)/height);
    if (x->havewindow) return int((x->screeny2-x->screeny1) * (yval-x->y1) / height);
    graph_graphrect(x, x->dix->canvas, &x1, &y1, &x2, &y2);
    return int(y1 + (y2-y1) * (yval-x->y1) / height);
}

/* --------------------------- widget behavior  ------------------- */
/* don't remove this code yet: has to be rewritten in tcl */
#if 1
#define FONT "pourier"
static void graph_vis(t_gobj *gr, int vis) {
	t_canvas *x = (t_canvas *)gr;
	t_canvas *c = canvas_getcanvas(x->dix->canvas);
	char tag[50];
	int x1=69, y1=69, x2=69, y2=69;
	sprintf(tag, "graph%lx", (t_int)x);
	if (vis) {
		sys_mgui(x,"ninlets=","i",0/*obj_ninlets(x)*/);
		sys_mgui(x,"noutlets=","i",0/*obj_noutlets(x)*/);
	}
	/* if we look like a graph but have been moved to a toplevel, just show the bounding rectangle */
	if (x->havewindow) {
		/*if (vis) sys_vgui(".x%lx.c create polygon %d %d %d %d %d %d %d %d %d %d -tags %s -fill #c0c0c0\n",
			(long)c, x1, y1, x1, y2, x2, y2, x2, y1, x1, y1, tag);*/
		return;
	}
        /* draw a rectangle around the graph */
        sys_vgui(".x%lx.c create line %d %d %d %d %d %d %d %d %d %d -tags %s\n",
            (long)c, x1, y1, x1, y2, x2, y2, x2, y1, x1, y1, tag);
	/* if there's just one "garray" in the graph, write its name along the top */
        int i = min(y1,y2)-1;
        t_symbol *arrayname;
        canvas_each(g,x) if (g->g_pd == garray_class && !garray_getname((t_garray *)g, &arrayname)) {
            // i -= sys_fontheight(glist_getfont(x));
            sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor nw\
              -font -*-courier-bold--normal--%d-* -tags %s\n",
                (long)canvas_getcanvas(x),  x1, i, arrayname->name,
                42/*sys_hostfontsize(canvas_getfont(x))*/, tag);
        }

        /* draw ticks on horizontal borders.  If lperb field is zero, this is disabled. */
	#define DRAWTICK(x1,y1,x2,y2) sys_vgui(".x%lx.c create line %d %d %d %d -tags %s\n", \
		(long)c, int(x1),int(y1),int(x2),int(y2),tag)
        float f;
        if (x->xtick.lperb) {
            float upix, lpix;
            if (y2<y1) {upix = y1; lpix = y2;}
            else       {upix = y2; lpix = y1;}
            for (i=0,f=x->xtick.point; f<0.99*x->x2+0.01*x->x1; i++, f+=x->xtick.inc) {
                int tickpix = i%x->xtick.lperb?2:4, x0 = canvas_xtopixels(x,f);
                DRAWTICK(x0,upix,x0,upix-tickpix);
                DRAWTICK(x0,lpix,x0,lpix+tickpix);
            }
            for (i=1,f=x->xtick.point-x->xtick.inc; f>0.99*x->x1+0.01*x->x2; i++,f-=x->xtick.inc) {
                int tickpix = i%x->xtick.lperb?2:4, x0 = canvas_xtopixels(x,f);
                DRAWTICK(x0,upix,x0,upix-tickpix);
                DRAWTICK(x0,lpix,x0,lpix+tickpix);
            }
        }
	/* draw ticks in vertical borders*/
        if (x->ytick.lperb) {
            float ubound, lbound;
            if (x->y2<x->y1) {ubound = x->y1; lbound = x->y2;}
	    else             {ubound = x->y2; lbound = x->y1;}
            for (i=0,f=x->ytick.point; f<0.99*ubound+0.01*lbound; i++, f += x->ytick.inc) {
                int tickpix = i%x->ytick.lperb?2:4, y0 = canvas_ytopixels(x,f);
                DRAWTICK(x1,y0,x1+tickpix,y0);
                DRAWTICK(x2,y0,x2-tickpix,y0);
            }
            for (i=1,f=x->ytick.point-x->ytick.inc; f>0.99*lbound+0.01*ubound; i++,f-=x->ytick.inc) {
                int tickpix = i%x->ytick.lperb?2:4, y0 = canvas_ytopixels(x,f);
                DRAWTICK(x1,y0,x1+tickpix,y0);
                DRAWTICK(x2,y0,x2-tickpix,y0);
            }
        }
    /* draw labels */
	#define DRAWLABEL(x1,y1) sys_vgui(".x%lx.c create text %d %d -text {%s} -font "FONT" -tags %s\n", (long)c, \
		int(canvas_xtopixels(x,x1)),int(canvas_ytopixels(x,y1)),s,42,tag);
        for (int i=0; i < x->nxlabels; i++) {char *s = x->xlabel[i]->name; DRAWLABEL(atof(s),x->xlabely);}
        for (int i=0; i < x->nylabels; i++) {char *s = x->ylabel[i]->name; DRAWLABEL(x->ylabelx,atof(s));}
}
#endif

static int text_xpix(t_text *x, t_canvas *canvas) {
    float width = canvas->x2-canvas->x1;
    if (canvas->havewindow || !canvas->gop) return x->x;
    if (canvas->goprect) return canvas->x+x->x-canvas->xmargin;
    return canvas_xtopixels(canvas, canvas->x1 + width * x->x / (canvas->screenx2-canvas->screenx1));
}
static int text_ypix(t_text *x, t_canvas *canvas) {
    float height = canvas->y2-canvas->y1;
    if (canvas->havewindow || !canvas->gop) return x->y;
    if (canvas->goprect) return canvas->y+x->y-canvas->ymargin;
    return canvas_ytopixels(canvas, canvas->y1 + height* x->y / (canvas->screeny2-canvas->screeny1));
}
static void graph_graphrect(t_gobj *z, t_canvas *canvas, int *xp1, int *yp1, int *xp2, int *yp2) {
    t_canvas *x = (t_canvas *)z;
    *xp1 = text_xpix(x,canvas); *xp2 = *xp1+x->pixwidth;
    *yp1 = text_ypix(x,canvas); *yp2 = *yp1+x->pixheight;
}

#if 1
static float graph_lastxpix, graph_lastypix;
static void graph_motion(void *z, t_floatarg dx, t_floatarg dy) {
    t_canvas *x = (t_canvas *)z;
    float newxpix = graph_lastxpix + dx, newypix = graph_lastypix + dy;
    t_garray *a = (t_garray *)x->boxes->first();
    int oldx = int(0.5 + canvas_pixelstox(x, graph_lastxpix));
    int newx = int(0.5 + canvas_pixelstox(x, newxpix));
    float oldy = canvas_pixelstoy(x, graph_lastypix);
    float newy = canvas_pixelstoy(x, newypix);
    graph_lastxpix = newxpix;
    graph_lastypix = newypix;
    // verify that the array is OK
    if (!a || a->_class != garray_class) return;
    int nelem;
    t_float *vec;
    if (!garray_getfloatarray(a, &nelem, &vec)) return;
    if (oldx < 0) oldx = 0; else if (oldx >= nelem) oldx = nelem - 1;
    if (newx < 0) newx = 0; else if (newx >= nelem) newx = nelem - 1;
    if      (oldx < newx - 1) {for (int i=oldx+1; i<=newx; i++) vec[i] = newy + (oldy-newy) * float(newx-i)/float(newx - oldx);}
    else if (oldx > newx + 1) {for (int i=oldx-1; i>=newx; i--) vec[i] = newy + (oldy-newy) * float(newx-i)/float(newx - oldx);}
    else vec[newx] = newy;
    garray_redraw(a);
}
#endif

/* functions to read and write canvases to files: canvas_savetofile() writes a root canvas to a "pd" file.
   (Reading "pd" files is done simply by passing the contents to the pd message interpreter.)
   Alternatively, the  glist_read() and glist_write() functions read and write "data" from and to files
   (reading reads into an existing canvas), using a file format as in the dialog window for data. */
static t_class *declare_class;
void canvas_savedeclarationsto(t_canvas *x, t_binbuf *b);

/* the following functions read "scalars" from a file into a canvas. */
static int canvas_scanbinbuf(int natoms, t_atom *vec, int *p_indexout, int *p_next) {
    int i;
    int indexwas = *p_next;
    *p_indexout = indexwas;
    if (indexwas >= natoms) return 0;
    for (i = indexwas; i < natoms && vec[i].a_type != A_SEMI; i++) {}
    if (i >= natoms) *p_next = i; else *p_next = i+1;
    return i-indexwas;
}
static int canvas_readscalar(t_canvas *x, int natoms, t_atom *vec, int *p_nextmsg, int selectit);
static void canvas_readerror(int natoms, t_atom *vec, int message, int nline, const char *s) {
    error(s);
    startpost("line was:");
    postatom(nline, vec + message);
    endpost();
}

/* fill in the contents of the scalar into the vector w. */
static void canvas_readatoms(t_canvas *x, int natoms, t_atom *vec,
int *p_nextmsg, t_symbol *templatesym, t_word *w, int argc, t_atom *argv) {
    t_template *t = template_findbyname(templatesym);
    if (!t) {
        error("%s: no such template", templatesym->name);
        *p_nextmsg = natoms;
        return;
    }
    word_restore(w, t, argc, argv);
    int n = t->n;
    for (int i=0; i<n; i++) {
        if (t->vec[i].type == DT_ARRAY) {
            t_array *a = w[i].w_array;
            int elemsize = a->elemsize, nitems = 0;
            t_symbol *arraytemplatesym = t->vec[i].arraytemplate;
            t_template *arraytemplate = template_findbyname(arraytemplatesym);
            if (!arraytemplate) error("%s: no such template", arraytemplatesym->name);
            else while (1) {
		int message;
                t_word *element;
                int nline = canvas_scanbinbuf(natoms, vec, &message, p_nextmsg);
                /* empty line terminates array */
                if (!nline) break;
                array_resize(a, nitems + 1);
                element = (t_word *)&a->vec[nitems*elemsize];
                canvas_readatoms(x, natoms, vec, p_nextmsg, arraytemplatesym, element, nline, vec + message);
                nitems++;
            }
        } else if (t->vec[i].type == DT_CANVAS) {
            while (1) {
                if (!canvas_readscalar(w->w_canvas, natoms, vec, p_nextmsg, 0)) break;
            }
        }
    }
}

static int canvas_readscalar(t_canvas *x, int natoms, t_atom *vec, int *p_nextmsg, int selectit) {
    int nextmsg = *p_nextmsg;
    //int wasvis = canvas_isvisible(x);
    if (nextmsg >= natoms || vec[nextmsg].a_type != A_SYMBOL) {
        if (nextmsg < natoms) post("stopping early: type %d", vec[nextmsg].a_type);
        *p_nextmsg = natoms; return 0;
    }
    t_symbol *ts = canvas_makebindsym(vec[nextmsg].a_symbol);
    *p_nextmsg = nextmsg + 1;
    t_template *t = template_findbyname(ts);
    if (!t) {error("%s: no such template", ts->name); *p_nextmsg = natoms; return 0;}
    t_scalar *sc = scalar_new(x, ts);
    if (!sc) {error("couldn't create scalar \"%s\"", ts->name);    *p_nextmsg = natoms; return 0;}
    //if (wasvis) canvas_getcanvas(x)->mapped = 0;
    canvas_add(x,sc);
    int message;
    int nline = canvas_scanbinbuf(natoms, vec, &message, p_nextmsg);
    canvas_readatoms(x, natoms, vec, p_nextmsg, ts, sc->v, nline, vec + message);
    //if (wasvis) canvas_getcanvas(x)->mapped = 1;
    gobj_changed(sc,0);//is this necessary?
    return 1;
}

static void canvas_readfrombinbuf(t_canvas *x, t_binbuf *b, const char *filename, int selectem) {
    int message, nextmsg = 0;
    int natoms = binbuf_getnatom(b);
    t_atom *vec = binbuf_getvec(b);
    /* check for file type */
    int nline = canvas_scanbinbuf(natoms, vec, &message, &nextmsg);
    if (nline!=1 && vec[message].a_type != A_SYMBOL && strcmp(vec[message].a_symbol->name, "data")) {
        error("%s: file apparently of wrong type", filename);
        binbuf_free(b);
        return;
    }
    /* read in templates and check for consistency */
    while (1) {
        t_template *newtemplate, *existtemplate;
        t_atom *templateargs = (t_atom *)getbytes(0);
        int ntemplateargs = 0;
        nline = canvas_scanbinbuf(natoms, vec, &message, &nextmsg);
        if (nline < 2) break;
        else if (nline > 2) canvas_readerror(natoms, vec, message, nline, "extra items ignored");
        else if (vec[message].a_type != A_SYMBOL || strcmp(vec[message].a_symbol->name, "template") ||
          vec[message+1].a_type != A_SYMBOL) {
            canvas_readerror(natoms, vec, message, nline, "bad template header");
            continue;
        }
        t_symbol *templatesym = canvas_makebindsym(vec[message + 1].a_symbol);
        while (1) {
            nline = canvas_scanbinbuf(natoms, vec, &message, &nextmsg);
            if (nline != 2 && nline != 3) break;
            int newnargs = ntemplateargs + nline;
            templateargs = (t_atom *)realloc(templateargs, sizeof(*templateargs) * newnargs);
            templateargs[ntemplateargs] = vec[message];
            templateargs[ntemplateargs + 1] = vec[message + 1];
            if (nline == 3) templateargs[ntemplateargs + 2] = vec[message + 2];
            ntemplateargs = newnargs;
        }
        newtemplate = template_new(templatesym, ntemplateargs, templateargs);
        free(templateargs);
        if (!(existtemplate = template_findbyname(templatesym))) {
            error("%s: template not found in current patch", templatesym->name);
            template_free(newtemplate);
            return;
        }
        if (!template_match(existtemplate, newtemplate)) {
            error("%s: template doesn't match current one", templatesym->name);
            template_free(newtemplate);
            return;
        }
        template_free(newtemplate);
    }
    while (nextmsg < natoms) canvas_readscalar(x, natoms, vec, &nextmsg, selectem);
}

static void canvas_doread(t_canvas *x, t_symbol *filename, t_symbol *format, int clearme) {
    t_binbuf *b = binbuf_new();
    t_canvas *canvas = canvas_getcanvas(x);
    int wasvis = canvas_isvisible(canvas);
    int cr = strcmp(format->name, "cr")==0;
    if (!cr && *format->name) error("unknown flag: %s", format->name);
    /* flag 2 means eval continuously. this is required to autodetect the syntax */
    if (binbuf_read_via_path(b, filename->name, canvas_getdir(canvas)->name, cr|2)) {
        error("read failed");
        binbuf_free(b);
        return;
    }
    if (wasvis) canvas_vis(canvas, 0);
    if (clearme) canvas_clear(x);
    /* canvas_readfrombinbuf(x, b, filename->name, 0); */ /* what's this for? */
    if (wasvis) canvas_vis(canvas, 1);
    binbuf_free(b);
}

static void canvas_read(     t_canvas *x, t_symbol *filename, t_symbol *format) {canvas_doread(x,filename,format,1);}
static void canvas_mergefile(t_canvas *x, t_symbol *filename, t_symbol *format) {canvas_doread(x,filename,format,0);}

/* read text from a "properties" window, in answer to scalar_properties().
   We try to restore the object; if successful
   we delete the scalar and put the new thing in its place on the list. */
void canvas_dataproperties(t_canvas *x, t_scalar *sc, t_binbuf *b) {
//    t_gobj *oldone = 0;
//    t_gobj *newone = 0;
    x->boxes->remove_by_value(sc);
//    if (!newone) {error("couldn't update properties (perhaps a format problem?)"); return;}
//    if (!oldone) {bug("data_properties: couldn't find old element"); return;}
    canvas_readfrombinbuf(x, b, "properties dialog", 0);
}

static void canvas_doaddtemplate(t_symbol *templatesym, int *p_ntemplates, t_symbol ***p_templatevec) {
    int n = *p_ntemplates;
    t_symbol **templatevec = *p_templatevec;
    for (int i=0; i < n; i++) if (templatevec[i] == templatesym) return;
    templatevec = (t_symbol **)realloc(templatevec, (n+1)*sizeof(*templatevec));
    templatevec[n] = templatesym;
    *p_templatevec = templatevec;
    *p_ntemplates = n+1;
}

static void canvas_writelist(t_gobj *y, t_binbuf *b);

static void canvas_writescalar(t_symbol *templatesym, t_word *w, t_binbuf *b, int amarrayelement) {
    t_template *t = template_findbyname(templatesym);
    t_atom *a = (t_atom *)getbytes(0);
    int n = t->n, natom = 0;
    if (!amarrayelement) {
        t_atom templatename;
        SETSYMBOL(&templatename, gensym(templatesym->name + 3));
        binbuf_add(b, 1, &templatename);
    }
    if (!t) bug("canvas_writescalar");
    /* write the atoms (floats and symbols) */
    for (int i=0; i<n; i++) {
	int ty = t->vec[i].type;
        if (ty==DT_FLOAT || ty==DT_SYMBOL) {
            a = (t_atom *)realloc(a, (natom+1)*sizeof(*a));
            if (t->vec[i].type == DT_FLOAT) SETFLOAT( a + natom, w[i].w_float);
            else                            SETSYMBOL(a + natom, w[i].w_symbol);
            natom++;
        }
    }
        /* array elements have to have at least something */
    if (natom == 0 && amarrayelement)
        SETSYMBOL(a + natom,  &s_bang), natom++;
    binbuf_add(b, natom, a);
    binbuf_addsemi(b);
    free(a);
    for (int i=0; i<n; i++) {
        if (t->vec[i].type == DT_ARRAY) {
            t_array *a = w[i].w_array;
            int elemsize = a->elemsize, nitems = a->n;
            t_symbol *arraytemplatesym = t->vec[i].arraytemplate;
            for (int j = 0; j < nitems; j++)
                canvas_writescalar(arraytemplatesym, (t_word *)&a->vec[elemsize*j], b, 1);
            binbuf_addsemi(b);
        } else if (t->vec[i].type == DT_CANVAS) {
            canvas_writelist(w->w_canvas->boxes->first(), b);
            binbuf_addsemi(b);
        }
    }
}

static void canvas_writelist(t_gobj *y, t_binbuf *b) {
    for (; y; y = y->next()) if (y->_class==scalar_class) {
	t_scalar *z = (t_scalar *)y;
	canvas_writescalar(z->t, z->v, b, 0);
    }
}

static void canvas_addtemplatesforlist(t_gobj *y, int *p_ntemplates, t_symbol ***p_templatevec);

static void canvas_addtemplatesforscalar(t_symbol *templatesym, t_word *w, int *p_ntemplates, t_symbol ***p_templatevec) {
    t_template *t = template_findbyname(templatesym);
    canvas_doaddtemplate(templatesym, p_ntemplates, p_templatevec);
    if (!t) {bug("canvas_addtemplatesforscalar"); return;}
    t_dataslot *ds = t->vec;
    for (int i=t->n; i--; ds++, w++) {
        if (ds->type == DT_ARRAY) {
            t_array *a = w->w_array;
            int elemsize = a->elemsize, nitems = a->n;
            t_symbol *arraytemplatesym = ds->arraytemplate;
            canvas_doaddtemplate(arraytemplatesym, p_ntemplates, p_templatevec);
            for (int j=0; j<nitems; j++)
                canvas_addtemplatesforscalar(arraytemplatesym, (t_word *)&a->vec[elemsize*j], p_ntemplates, p_templatevec);
        } else if (ds->type == DT_CANVAS)
            canvas_addtemplatesforlist(w->w_canvas->boxes->first(), p_ntemplates, p_templatevec);
    }
}

static void canvas_addtemplatesforlist(t_gobj *y, int *p_ntemplates, t_symbol ***p_templatevec) {
    for (; y; y = y->next()) if (y->_class == scalar_class) {
	t_scalar *z = (t_scalar *)y;
	canvas_addtemplatesforscalar(z->t, z->v, p_ntemplates, p_templatevec);
    }
}

static t_binbuf *canvas_writetobinbuf(t_canvas *x) {
    t_symbol **templatevec = (t_symbol **)getbytes(0);
    int ntemplates = 0;
    t_binbuf *b = binbuf_new();
    canvas_each(y,x) if (y->_class==scalar_class) {
	t_scalar *s = (t_scalar *)y;
	canvas_addtemplatesforscalar(s->t, s->v, &ntemplates, &templatevec);
    }
    binbuf_addv(b,"t;","data");
    for (int i=0; i<ntemplates; i++) {
        t_template *t = template_findbyname(templatevec[i]);
        int m = t->n;
        /* drop "pd-" prefix from template symbol to print it: */
        binbuf_addv(b,"tt;","template",templatevec[i]->name + 3);
        for (int j=0; j<m; j++) {
            t_symbol *type;
            switch (t->vec[j].type) {
                case DT_FLOAT:  type = &s_float; break;
                case DT_SYMBOL: type = &s_symbol; break;
                case DT_ARRAY:  type = gensym("array"); break;
                case DT_CANVAS: type = &s_list; break;
                default: type = &s_float; bug("canvas_write");
            }
            if (t->vec[j].type == DT_ARRAY)
                 binbuf_addv(b,"sst;", type, t->vec[j].name, t->vec[j].arraytemplate->name + 3);
            else binbuf_addv(b,"ss;",  type, t->vec[j].name);
        }
        binbuf_addsemi(b);
    }
    binbuf_addsemi(b);
    /* now write out the objects themselves */
    canvas_each(y,x) if (y->_class==scalar_class) {
	t_scalar *z = (t_scalar *)y;
	canvas_writescalar(z->t, z->v,  b, 0);
    }
    return b;
}

static void canvas_write(t_canvas *x, t_symbol *filename, t_symbol *format) {
    t_canvas *canvas = canvas_getcanvas(x);
    char *buf = canvas_makefilename(canvas,filename->name,0,0);
    int cr = strcmp(format->name, "cr")==0;
    if (!cr && *format->name) error("canvas_write: unknown flag: %s", format->name);
    t_binbuf *b = canvas_writetobinbuf(x);
    if (b) {
        if (binbuf_write(b, buf, "", cr)) error("%s: write failed", filename->name);
        binbuf_free(b);
    }
    free(buf);
}

/* ------ functions to save and restore canvases (patches) recursively. ----*/

/* save to a binbuf, called recursively; cf. canvas_savetofile() which saves the document, and is only called on root canvases. */
void canvas_savecontainerto(t_canvas *x, t_binbuf *b) {
    /* have to go to original binbuf to find out how we were named. */
    t_binbuf *bz = binbuf_new();
    t_symbol *patchsym = &s_;
    if (x->binbuf) {
        binbuf_addbinbuf(bz, x->binbuf);
        patchsym = atom_getsymbolarg(1, binbuf_getnatom(bz), binbuf_getvec(bz));
        binbuf_free(bz);
    }
    int x1=x->screenx1, xs=x->screenx2-x1;
    int y1=x->screeny1, ys=x->screeny2-y1;
    binbuf_addv(b,"ttiiii","#N","canvas",x1,y1,xs,ys);
    if (x->dix->canvas && !x->env) { /* subpatch */
	binbuf_addv(b, "si;", (patchsym != &s_ ? patchsym: gensym("(subpatch)")), x->havewindow);
    } else { /* root or abstraction */
        binbuf_addv(b, "i;", (int)x->font);
        canvas_savedeclarationsto(x, b);
    }
}

static void canvas_savecoordsto(t_canvas *x, t_binbuf *b) {
    /* if everything is the default, skip saving this line */
    if (!x->gop && x->x1==0 && x->y1==0 && x->x2==1 && x->y2==1 && x->pixwidth==0 && x->pixheight==0) return;
    /* if we have a graph-on-parent rectangle, we're new style. The format is arranged so
       that old versions of Pd can at least do something with it.
       otherwise write in 0.38-compatible form. */
    binbuf_addv(b,"ttffffffi","#X","coords", x->x1,x->y1,x->x2,x->y2, (float)x->pixwidth,(float)x->pixheight, x->gop?x->hidetext?2:1:0);
    if (x->goprect) binbuf_addv(b, "ff", (float)x->xmargin, (float)x->ymargin);
    binbuf_addv(b,";");
}

/* get the index of a gobj in a canvas.  If y is zero, return the total number of objects. */
int canvas_oldindex(t_canvas *x, t_gobj *y) {
    int i=0;
    canvas_each(y2,x) {if (y2==y) break; else i++;}
    return i;
}

static void canvas_saveto(t_canvas *x, t_binbuf *b) {
    canvas_savecontainerto(x,b);
    canvas_each(y,x) gobj_save(y, b);
    canvas_wires_each(oc,t,x) {
        int from = canvas_oldindex(x,t.from);
        int to   = canvas_oldindex(x,t.to);
        binbuf_addv(b, "ttiiii;","#X","connect", from, t.outlet, to, t.inlet);
	appendix_save(oc,b);
    }
    canvas_savecoordsto(x,b);
}

/* call this recursively to collect all the template names for a canvas or for the selection. */
static void canvas_collecttemplatesfor(t_canvas *x, int *ntemplatesp, t_symbol ***templatevecp) {
    canvas_each(y,x) {
        if (y->_class==scalar_class) {
		t_scalar *z = (t_scalar *)y;
                canvas_addtemplatesforscalar(z->t, z->v, ntemplatesp, templatevecp);
	} else if (y->_class==canvas_class) {
                canvas_collecttemplatesfor((t_canvas *)y, ntemplatesp, templatevecp);
	}
    }
}

/* save the templates needed by a canvas to a binbuf. */
static void canvas_savetemplatesto(t_canvas *x, t_binbuf *b) {
    t_symbol **templatevec = (t_symbol **)getbytes(0);
    int ntemplates = 0;
    canvas_collecttemplatesfor(x, &ntemplates, &templatevec);
    for (int i=0; i < ntemplates; i++) {
        t_template *t = template_findbyname(templatevec[i]);
        if (!t) {
            bug("canvas_savetemplatesto");
            continue;
        }
        /* drop "pd-" prefix from template symbol to print */
        binbuf_addv(b,"ttt","#N","struct",templatevec[i]->name+3);
        for (int j=0; j<t->n; j++) {
            t_symbol *type;
            switch (t->vec[j].type) {
                case DT_FLOAT:  type = &s_float; break;
                case DT_SYMBOL: type = &s_symbol; break;
                case DT_ARRAY:  type = gensym("array"); break;
                case DT_CANVAS: type = &s_list; break;
                default:        type = &s_float; bug("canvas_write");
            }
	    binbuf_addv(b,"ss",type,t->vec[j].name);
	    if (t->vec[j].type == DT_ARRAY) binbuf_addv(b, "t", t->vec[j].arraytemplate->name + 3);
        }
        binbuf_addsemi(b);
    }
}

/* save a "root" canvas to a file; cf. canvas_saveto() which saves the body (and which is called recursively.) */
static void canvas_savetofile(t_canvas *x, t_symbol *filename, t_symbol *dir) {
    t_binbuf *b = binbuf_new();
    int dsp_status = canvas_suspend_dsp();
    canvas_savetemplatesto(x, b);
    canvas_saveto(x, b);
    if (!binbuf_write(b, filename->name, dir->name, 0)) {
	/* if not an abstraction, reset title bar and directory */
        if (!x->dix->canvas) canvas_rename(x, filename, dir);
        post("saved to: %s/%s", dir->name, filename->name);
        canvas_reload(filename,dir,x);
    }
    binbuf_free(b);
    canvas_resume_dsp(dsp_status);
}

///////////////////////////////////////////////////////////////////////////
// from g_io.c

/* graphical inlets and outlets, both for control and signals.  */
/* iohannes added multiple samplerates support in vinlet/voutlet */

extern "C" void signal_setborrowed(t_signal *sig, t_signal *sig2);
extern "C" void signal_makereusable(t_signal *sig);
extern "C" void inlet_sethelp(t_inlet* i,t_symbol* s);

/* ------------------------- vinlet -------------------------- */
t_class *vinlet_class;

struct t_vinlet : t_object {
    t_canvas *canvas;
    t_inlet *inlet;
    int bufsize;
    t_float *buf;         /* signal buffer; zero if not a signal */
    t_float *endbuf;
    t_float *fill;
    t_float *read;
    int hop;
    /* if not reblocking, the next slot communicates the parent's inlet signal from the prolog to the DSP routine: */
    t_signal *directsignal;
    t_resample updown; /* IOhannes */
};

static void *vinlet_new(t_symbol *s) {
    t_vinlet *x = (t_vinlet *)pd_new(vinlet_class);
    x->canvas = canvas_getcurrent();
    x->inlet = canvas_addinlet(x->canvas,x,0,s);
    x->bufsize = 0;
    x->buf = 0;
    outlet_new(x, 0);
    return x;
}

static void vinlet_bang(t_vinlet *x)                    {outlet_bang(x->outlet);}
static void vinlet_pointer(t_vinlet *x, t_gpointer *gp) {outlet_pointer(x->outlet, gp);}
static void vinlet_float(t_vinlet *x, t_float f)        {outlet_float(x->outlet, f);}
static void vinlet_symbol(t_vinlet *x, t_symbol *s)     {outlet_symbol(x->outlet, s);}
static void vinlet_list(t_vinlet *x, t_symbol *s, int argc, t_atom *argv)     {outlet_list(x->outlet, s, argc, argv);}
static void vinlet_anything(t_vinlet *x, t_symbol *s, int argc, t_atom *argv) {outlet_anything(x->outlet, s, argc, argv);}

static void vinlet_free(t_vinlet *x) {
    canvas_rminlet(x->canvas, x->inlet);
    resample_free(&x->updown);
}

t_inlet *vinlet_getit(t_pd *x) {
    if (pd_class(x) != vinlet_class) bug("vinlet_getit");
    return ((t_vinlet *)x)->inlet;
}

/* ------------------------- signal inlet -------------------------- */
int vinlet_issignal(t_vinlet *x) {return x->buf!=0;}

t_int *vinlet_perform(t_int *w) {
    t_vinlet *x = (t_vinlet *)w[1];
    t_float *out = (t_float *)w[2];
    int n = int(w[3]);
    t_float *in = x->read;
    while (n--) *out++ = *in++;
    if (in == x->endbuf) in = x->buf;
    x->read = in;
    return w+4;
}

/* tb: vectorized */
t_int *vinlet_perf8(t_int *w) {
    t_vinlet *x = (t_vinlet *)w[1];
    t_float *out = (t_float *)w[2];
    int n = int(w[3]);
    t_float *in = x->read;
    for (; n; n -= 8, in += 8, out += 8) {
	out[0] = in[0]; out[1] = in[1]; out[2] = in[2]; out[3] = in[3];
	out[4] = in[4]; out[5] = in[5]; out[6] = in[6]; out[7] = in[7];
    }
    if (in == x->endbuf) in = x->buf;
    x->read = in;
    return w+4;
}

/* T.Grill: SIMD version */
t_int *vinlet_perfsimd(t_int *w) {
    t_vinlet *x = (t_vinlet *)(w[1]);
    t_float *in = x->read;
    copyvec_simd((t_float *)w[2],in,w[3]);
    if (in == x->endbuf) in = x->buf;
    x->read = in;
    return w+4;
}

static void vinlet_dsp(t_vinlet *x, t_signal **sp) {
    if (!x->buf) return; /* no buffer means we're not a signal inlet */
    t_signal *outsig = sp[0];
    if (x->directsignal) signal_setborrowed(sp[0], x->directsignal);
    else {
        const int vecsize = outsig->vecsize;
	/* if the outsig->v is aligned the x->read will also be... */
	if(vecsize&7) dsp_add(vinlet_perform, 3, x, outsig->v,vecsize);
	else if(SIMD_CHECK1(outsig->n,outsig->v))
	     dsp_add(vinlet_perfsimd, 3, x, outsig->v,vecsize);
	else dsp_add(vinlet_perf8,    3, x, outsig->v,vecsize);
        x->read = x->buf;
    }
}

/* prolog code: loads buffer from parent patch */
t_int *vinlet_doprolog(t_int *w) {
    t_vinlet *x = (t_vinlet *)w[1];
    t_float *in = (t_float *)w[2];
    int n = int(w[3]);
    t_float *out = x->fill;
    if (out == x->endbuf) {
        t_float *f1 = x->buf, *f2 = x->buf + x->hop;
        int nshift = x->bufsize - x->hop;
        out -= x->hop;
        while (nshift--) *f1++ = *f2++;
    }
    while (n--) *out++ = *in++;
    x->fill = out;
    return w+4;
}

extern "C" int inlet_getsignalindex(t_inlet *x);

/* set up prolog DSP code  */
extern "C" void vinlet_dspprolog(t_vinlet *x, t_signal **parentsigs, int myvecsize, int calcsize, int phase, int period,
int frequency, int downsample, int upsample, int reblock, int switched) {
    t_signal *insig;
    x->updown.downsample = downsample;
    x->updown.upsample   = upsample;
    /* if the "reblock" flag is set, arrange to copy data in from the parent. */
    if (reblock) {
        int parentvecsize, bufsize, oldbufsize, prologphase;
        int re_parentvecsize; /* resampled parentvectorsize: IOhannes */
        /* this should never happen: */
        if (!x->buf) return;
        /* the prolog code counts from 0 to period-1; the
           phase is backed up by one so that AFTER the prolog code
           runs, the "fill" phase is in sync with the "read" phase. */
        prologphase = (phase - 1) & (period - 1);
        if (parentsigs) {
            insig = parentsigs[inlet_getsignalindex(x->inlet)];
            parentvecsize = insig->vecsize;
            re_parentvecsize = parentvecsize * upsample / downsample;
        } else {
            insig = 0;
            parentvecsize = 1;
            re_parentvecsize = 1;
        }
        bufsize = max(re_parentvecsize,myvecsize);
        oldbufsize = x->bufsize;
        if (bufsize != oldbufsize) {
            t_float *buf = x->buf;
	    buf = (t_float *)resizealignedbytes(buf,oldbufsize * sizeof(*buf), bufsize * sizeof(*buf));
            memset((char *)buf, 0, bufsize * sizeof(*buf));
            x->bufsize = bufsize;
            x->endbuf = buf + bufsize;
            x->buf = buf;
        }
        if (parentsigs) {
            /* IOhannes { */
            x->hop = period * re_parentvecsize;
            x->fill = x->endbuf - (x->hop - prologphase * re_parentvecsize);
            if (upsample * downsample == 1)
                dsp_add(vinlet_doprolog, 3, x, insig->v, re_parentvecsize);
            else {
                resamplefrom_dsp(&x->updown, insig->v, parentvecsize, re_parentvecsize, x->updown.method);
                dsp_add(vinlet_doprolog, 3, x, x->updown.v, re_parentvecsize);
            }
            /* } IOhannes */
            /* if the input signal's reference count is zero, we have to free it here because we didn't in ugen_doit(). */
            if (!insig->refcount) signal_makereusable(insig);
        } else memset((char *)x->buf, 0, bufsize * sizeof(*x->buf));
        x->directsignal = 0;
    } else {
        /* no reblocking; in this case our output signal is "borrowed" and merely needs to be pointed to the real one. */
        x->directsignal = parentsigs[inlet_getsignalindex(x->inlet)];
    }
}

static void *vinlet_newsig(t_symbol *s) {
    t_vinlet *x = (t_vinlet *)pd_new(vinlet_class);
    x->canvas = canvas_getcurrent();
    x->inlet = canvas_addinlet(x->canvas,x,&s_signal,s);
    x->endbuf = x->buf = (t_float *)getalignedbytes(0);
    x->bufsize = 0;
    x->directsignal = 0;
    outlet_new(x, &s_signal);
    resample_init(&x->updown);
    /* this should be thought over: it might prove hard to provide consistency between labeled up- & downsampling methods
       maybe indices would be better...
       up till now we provide several upsampling methods and 1 single downsampling method (no filtering !) */
    if (s) {
      char c=*s->name;
      switch(c) {
      case'h':case'H':x->updown.method=RESAMPLE_HOLD;   break; /* up: sample and hold */
      case'l':case'L':x->updown.method=RESAMPLE_LINEAR; break; /* up: linear interpolation */
      case'b':case'B':x->updown.method=RESAMPLE_BLOCK;  break; /* down: ignore the 2nd half of the block */
      default:        x->updown.method=RESAMPLE_ZERO;          /* up: zero-padding */
      }
    }
    return x;
}

static void vinlet_setup() {
    t_class *c = vinlet_class = class_new2("inlet",vinlet_new,vinlet_free,sizeof(t_vinlet),CLASS_NOINLET,"S");
    class_addcreator2("inlet~",vinlet_newsig,"S");
    class_addbang(    c, vinlet_bang);
    class_addpointer( c, vinlet_pointer);
    class_addfloat(   c, vinlet_float);
    class_addsymbol(  c, vinlet_symbol);
    class_addlist(    c, vinlet_list);
    class_addanything(c, vinlet_anything);
    class_addmethod2( c, vinlet_dsp,"dsp","");
    class_sethelpsymbol(c, gensym("pd"));
}

/* ------------------------- voutlet -------------------------- */

t_class *voutlet_class;

struct t_voutlet : t_object {
    t_canvas *canvas;
    t_outlet *parentoutlet;
    int bufsize;
    t_float *buf;         /* signal buffer; zero if not a signal */
    t_float *endbuf;
    t_float *empty;       /* next to read out of buffer in epilog code */
    t_float *write;       /* next to write in to buffer */
    int hop;              /* hopsize */
    /* vice versa from the inlet, if we don't block, this holds the
       parent's outlet signal, valid between the prolog and the dsp setup functions.  */
    t_signal *directsignal;
        /* and here's a flag indicating that we aren't blocked but have to do a copy (because we're switched). */
    char justcopyout;
    t_resample updown; /* IOhannes */
};

static void *voutlet_new(t_symbol *s) {
    t_voutlet *x = (t_voutlet *)pd_new(voutlet_class);
    x->canvas = canvas_getcurrent();
    x->parentoutlet = canvas_addoutlet(x->canvas,x,0);
    inlet_new(x,x,0,0);
    x->bufsize = 0;
    x->buf = 0;
    return x;
}

static void voutlet_bang(t_voutlet *x)
{outlet_bang(x->parentoutlet);}
static void voutlet_pointer(t_voutlet *x, t_gpointer *gp)
{outlet_pointer(x->parentoutlet, gp);}
static void voutlet_float(t_voutlet *x, t_float f)
{outlet_float(x->parentoutlet, f);}
static void voutlet_symbol(t_voutlet *x, t_symbol *s)
{outlet_symbol(x->parentoutlet, s);}
static void voutlet_list(t_voutlet *x, t_symbol *s, int argc, t_atom *argv)
{outlet_list(x->parentoutlet, s, argc, argv);}
static void voutlet_anything(t_voutlet *x, t_symbol *s, int argc, t_atom *argv)
{outlet_anything(x->parentoutlet, s, argc, argv);}

static void voutlet_free(t_voutlet *x) {
    canvas_rmoutlet(x->canvas, x->parentoutlet);
    resample_free(&x->updown);
}

t_outlet *voutlet_getit(t_pd *x) {
    if (pd_class(x) != voutlet_class) bug("voutlet_getit");
    return ((t_voutlet *)x)->parentoutlet;
}

/* ------------------------- signal outlet -------------------------- */

int voutlet_issignal(t_voutlet *x) {return x->buf!=0;}

/* LATER optimize for non-overlapped case where the "+=" isn't needed */
t_int *voutlet_perform(t_int *w) {
    t_voutlet *x = (t_voutlet *)w[1];
    t_float *in = (t_float *)w[2];
    int n = int(w[3]);
    t_float *out = x->write, *outwas = out, *end = x->endbuf;
    while (n--) {
        *out++ += *in++;
    	if (out == end) out = x->buf;
    }
    outwas += x->hop;
    if (outwas >= end) outwas = x->buf;
    x->write = outwas;
    return w+4;
}

/* epilog code for blocking: write buffer to parent patch */
static t_int *voutlet_doepilog(t_int *w) {
    t_voutlet *x = (t_voutlet *)w[1];
    t_float *out = (t_float *)w[2]; /* IOhannes */
    t_float *in = x->empty;
    if (x->updown.downsample != x->updown.upsample)    out = x->updown.v; /* IOhannes */
    for (int n = (int)(w[3]); n--; in++) *out++ = *in, *in = 0;
    if (in == x->endbuf) in = x->buf;
    x->empty = in;
    return w+4;
}

/* IOhannes { */
static t_int *voutlet_doepilog_resampling(t_int *w) {
    t_voutlet *x = (t_voutlet *)w[1];
    t_float *in  = x->empty;
    t_float *out = x->updown.v; /* IOhannes */
    for (int n = (int)(w[2]); n--; in++) *out++ = *in, *in = 0;
    if (in == x->endbuf) in = x->buf;
    x->empty = in;
    return w+3;
}
/* } IOhannes */
extern "C" int outlet_getsignalindex(t_outlet *x);

/* prolog for outlets -- store pointer to the outlet on the parent, which, if "reblock" is false, will want to refer
   back to whatever we see on our input during the "dsp" method called later.  */
extern "C" void voutlet_dspprolog(t_voutlet *x, t_signal **parentsigs, int myvecsize, int calcsize, int phase, int period,
int frequency, int downsample, int upsample, int reblock, int switched) {
    x->updown.downsample=downsample;  x->updown.upsample=upsample; /* IOhannes */
    x->justcopyout = (switched && !reblock);
    if (reblock) {
        x->directsignal = 0;
    } else {
        if (!parentsigs) bug("voutlet_dspprolog");
        x->directsignal = parentsigs[outlet_getsignalindex(x->parentoutlet)];
    }
}

static void voutlet_dsp(t_voutlet *x, t_signal **sp) {
    if (!x->buf) return;
    t_signal *insig = sp[0];
    if (x->justcopyout) dsp_add_copy(insig->v, x->directsignal->v, insig->n);
    else if (x->directsignal) {
        /* if we're just going to make the signal available on the parent patch, hand it off to the parent signal. */
        /* this is done elsewhere--> sp[0]->refcount++; */
        signal_setborrowed(x->directsignal, sp[0]);
    } else dsp_add(voutlet_perform, 3, x, insig->v, insig->n);
}

/* set up epilog DSP code.  If we're reblocking, this is the
   time to copy the samples out to the containing object's outlets.
   If we aren't reblocking, there's nothing to do here.  */
extern "C" void voutlet_dspepilog(t_voutlet *x, t_signal **parentsigs,
int myvecsize, int calcsize, int phase, int period, int frequency, int downsample, int upsample, int reblock, int switched) {
    if (!x->buf) return;  /* this shouldn't be necesssary... */
    x->updown.downsample=downsample;
    x->updown.upsample=upsample; /* IOhannes */
    if (reblock) {
        t_signal *outsig;
        int parentvecsize, bufsize, oldbufsize;
        int re_parentvecsize; /* IOhannes */
        int bigperiod, epilogphase, blockphase;
        if (parentsigs) {
            outsig = parentsigs[outlet_getsignalindex(x->parentoutlet)];
            parentvecsize = outsig->vecsize;
            re_parentvecsize = parentvecsize * upsample / downsample;
        } else {
            outsig = 0;
            parentvecsize = 1;
            re_parentvecsize = 1;
        }
        bigperiod = myvecsize/re_parentvecsize; /* IOhannes */
        if (!bigperiod) bigperiod = 1;
        epilogphase = phase & (bigperiod - 1);
        blockphase = (phase + period - 1) & (bigperiod - 1) & (- period);
        bufsize = re_parentvecsize; /* IOhannes */
        if (bufsize < myvecsize) bufsize = myvecsize;
        if (bufsize != (oldbufsize = x->bufsize)) {
            t_float *buf = x->buf;
    	    buf = (t_float *)resizealignedbytes(buf,oldbufsize * sizeof(*buf),bufsize * sizeof(*buf));
            memset((char *)buf, 0, bufsize * sizeof(*buf));
            x->bufsize = bufsize;
            x->endbuf = buf + bufsize;
            x->buf = buf;
        }
        /* IOhannes: { */
        if (re_parentvecsize * period > bufsize) bug("voutlet_dspepilog");
        x->write = x->buf + re_parentvecsize * blockphase;
        if (x->write == x->endbuf) x->write = x->buf;
        if (period == 1 && frequency > 1) x->hop = re_parentvecsize / frequency;
        else x->hop = period * re_parentvecsize;
      /* } IOhannes */
        if (parentsigs) {
          /* set epilog pointer and schedule it */
          /* IOhannes { */
          x->empty = x->buf + re_parentvecsize * epilogphase;
          if (upsample*downsample==1)
            dsp_add(voutlet_doepilog, 3, x, outsig->v, re_parentvecsize);
          else {
            dsp_add(voutlet_doepilog_resampling, 2, x, re_parentvecsize);
            resampleto_dsp(&x->updown, outsig->v, re_parentvecsize, parentvecsize, x->updown.method);
          }
          /* } IOhannes */
        }
    }
    /* if we aren't blocked but we are switched, the epilog code just
       copies zeros to the output.  In this case the blocking code actually jumps over the epilog if the block is running. */
    else if (switched) {
        if (parentsigs) {
            t_signal *outsig = parentsigs[outlet_getsignalindex(x->parentoutlet)];
            dsp_add_zero(outsig->v, outsig->n);
        }
    }
}

static void *voutlet_newsig(t_symbol *s) {
    t_voutlet *x = (t_voutlet *)pd_new(voutlet_class);
    x->canvas = canvas_getcurrent();
    x->parentoutlet = canvas_addoutlet(x->canvas,x,&s_signal);
    inlet_new(x,x,&s_signal,&s_signal);
    x->endbuf = x->buf = (t_float *)getalignedbytes(0);
    x->bufsize = 0;
    resample_init(&x->updown);
    /* this should be though over: 
     * it might prove hard to provide consistency between labeled up- & downsampling methods
     * maybe indeces would be better...
     * up till now we provide several upsampling methods and 1 single downsampling method (no filtering !) */
    if (s) {
      char c=*s->name;
      switch(c) {
      case 'h': case 'H': x->updown.method=RESAMPLE_HOLD; break; /* up: sample and hold */
      case 'l': case 'L': x->updown.method=RESAMPLE_LINEAR; break; /* up: linear interpolation */
      case 'b': case 'B': x->updown.method=RESAMPLE_BLOCK; break; /* down: ignore the 2nd half of the block */
      default: x->updown.method=RESAMPLE_ZERO;            /* up: zero-padding */
      }
    }
    return x;
}

static void voutlet_setup() {
    t_class *c = voutlet_class = class_new2("outlet",voutlet_new,voutlet_free,sizeof(t_voutlet),CLASS_NOINLET,"S");
    class_addcreator2("outlet~",voutlet_newsig,"S");
    class_addbang(    c, voutlet_bang);
    class_addpointer( c, voutlet_pointer);
    class_addfloat(   c, (t_method)voutlet_float);
    class_addsymbol(  c, voutlet_symbol);
    class_addlist(    c, voutlet_list);
    class_addanything(c, voutlet_anything);
    class_addmethod2( c, voutlet_dsp, "dsp", "");
    class_sethelpsymbol(c, gensym("pd"));
}

/* This file defines the "scalar" object, which is not a text object, just a
   "gobj".  Scalars have templates which describe their structures, which can contain numbers, sublists, and arrays.
   IOhannes changed the canvas_restore, so that it might accept $args as well (like "pd $0_test")
   so you can make multiple & distinguishable templates; added Krzysztof Czajas fix to avoid crashing... */
t_class *scalar_class;

void word_init(t_word *wp, t_template *t, t_gpointer *gp) {
    t_dataslot *datatypes = t->vec;
    for (int i=0; i < t->n; i++, datatypes++, wp++) {
        int type = datatypes->type;
        if      (type == DT_FLOAT)  wp->w_float = 0; 
        else if (type == DT_SYMBOL) wp->w_symbol = &s_symbol;
        else if (type == DT_ARRAY)  wp->w_array = array_new(datatypes->arraytemplate, gp);
        else if (type == DT_CANVAS) {
            /* LATER test this and get it to work */
            wp->w_canvas = canvas_new(0,0,0,0);
        }
    }
}

void word_restore(t_word *wp, t_template *t, int argc, t_atom *argv) {
    t_dataslot *datatypes = t->vec;
    for (int i=0; i<t->n; i++, datatypes++, wp++) {
        int type = datatypes->type;
        if (type == DT_FLOAT) {
            float f=0;
            if (argc) {f =  atom_getfloat(argv); argv++; argc--;}
            wp->w_float = f;
        } else if (type == DT_SYMBOL) {
            t_symbol *s=&s_;
            if (argc) {s = atom_getsymbol(argv); argv++; argc--;}
            wp->w_symbol = s;
        }
    }
    if (argc) post("warning: word_restore: extra arguments");
}

void word_free(t_word *wp, t_template *t) {
    t_dataslot *dt = t->vec;
    for (int i=0; i<t->n; i++, dt++) {
        if      (dt->type == DT_ARRAY)  pd_free(wp[i].w_array);
        else if (dt->type == DT_CANVAS) pd_free(wp[i].w_canvas);
    }
}

static void gpointer_setcanvas(t_gpointer *gp, t_canvas *canvas, t_scalar *x);
static void gpointer_setarray(t_gpointer *gp, t_array *array, t_word *w);
static t_word *gpointer_word(t_gpointer *gp) {return gp->o->_class == array_class ? gp->w : gp->scalar->v;}
static t_canvas *gpointer_getcanvas(t_gpointer *gp) {
    if (gp->o->_class != array_class) return gp->canvas;
    return 0; /* FIXME */
}
static t_scalar *gpointer_getscalar(t_gpointer *gp) {
    if (gp->o->_class != array_class) return gp->scalar;
    return 0;
}

/* make a new scalar and add to the canvas.  We create a "gp" here which will be used for array items to point back here.
   This gp doesn't do reference counting or "validation" updates though; the parent won't go away without the contained
   arrays going away too.  The "gp" is copied out by value in the word_init() routine so we can throw our copy away. */
t_scalar *scalar_new(t_canvas *owner, t_symbol *templatesym) {
    t_gpointer gp;
    gpointer_init(&gp);
    t_template *t = template_findbyname(templatesym);
    TEMPLATE_CHECK(templatesym,0)
    t_scalar *x = (t_scalar *)getbytes(sizeof(t_scalar) + (t->n - 1) * sizeof(*x->v));
    x->_class = scalar_class;
    x->t = templatesym;
    gpointer_setcanvas(&gp, owner, x);
    word_init(x->v, t, &gp);
    return x;
}

/* Pd method to create a new scalar, add it to a canvas, and initialize it from the message arguments. */
int canvas_readscalar(t_canvas *x, int natoms, t_atom *vec, int *p_nextmsg, int selectit);
static void canvas_scalar(t_canvas *canvas, t_symbol *classname, t_int argc, t_atom *argv) {
    t_symbol *templatesym = canvas_makebindsym(atom_getsymbolarg(0, argc, argv));
    if (!template_findbyname(templatesym)) {error("%s: no such template", atom_getsymbolarg(0, argc, argv)->name); return;}
    int nextmsg;
    t_binbuf *b = binbuf_new();
    binbuf_restore(b, argc, argv);
    canvas_readscalar(canvas, binbuf_getnatom(b), binbuf_getvec(b), &nextmsg, 0);
    binbuf_free(b);
}

static void scalar_getbasexy(t_scalar *x, float *basex, float *basey) {
    t_template *t = template_findbyname(x->t);
    *basex = template_getfloat(t,&s_x,x->v,0);
    *basey = template_getfloat(t,&s_y,x->v,0);
}

/*
static void scalar_displace(t_gobj *z, t_canvas *canvas, int dx, int dy) {
    t_scalar *x = (t_scalar *)z;
    t_symbol *templatesym = x->t;
    t_template *t = template_findbyname(templatesym);
    t_symbol *zz;
    int xonset, yonset, xtype, ytype, gotx, goty;
    TEMPLATE_CHECK(templatesym,)
    gotx = template_find_field(t,&s_x,&xonset,&xtype,&zz);
    if (gotx && (xtype != DT_FLOAT)) gotx = 0;
    goty = template_find_field(t,&s_y,&yonset,&ytype,&zz);
    if (goty && (ytype != DT_FLOAT)) goty = 0;
    if (gotx) *(t_float *)(((char *)(x->v)) + xonset) += dx * (canvas_pixelstox(canvas, 1) - canvas_pixelstox(canvas, 0));
    if (goty) *(t_float *)(((char *)(x->v)) + yonset) += dy * (canvas_pixelstoy(canvas, 1) - canvas_pixelstoy(canvas, 0));
    scalar_redraw(x, canvas);
}*/

static void scalar_vis(t_gobj *z, t_canvas *owner, int vis) {
    t_scalar *x = (t_scalar *)z;
    t_template *t = template_findbyname(x->t);
    t_canvas *templatecanvas = template_findcanvas(t);
    float basex, basey;
    scalar_getbasexy(x, &basex, &basey);
    /* if we don't know how to draw it, make a small rectangle */
    if (!templatecanvas) {
        if (vis) {
            int x1 = canvas_xtopixels(owner, basex);
            int y1 = canvas_ytopixels(owner, basey);
            sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags scalar%lx\n",
                (long)canvas_getcanvas(owner), x1-1, y1-1, x1+1, y1+1, (long)x);
        } else sys_vgui(".x%lx.c delete scalar%lx\n", (long)canvas_getcanvas(owner), (long)x);
        return;
    }
    //canvas_each(y,templatecanvas) pd_getparentwidget(y)->w_parentvisfn(y,owner,x->v,t,basex,basey,vis);
    //sys_unqueuegui(x);
}

static void scalar_doredraw(t_gobj *client, t_canvas *canvas) {
    scalar_vis(client, canvas, 0);
    scalar_vis(client, canvas, 1);
}

void scalar_redraw(t_scalar *x, t_canvas *canvas) {
    //if (canvas_isvisible(canvas)) sys_queuegui(x, canvas, scalar_doredraw);
}

#if 0
int scalar_doclick(t_word *data, t_template *t, t_scalar *sc, t_array *ap, t_canvas *owner,
float xloc, float yloc, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    t_canvas *templatecanvas = template_findcanvas(t);
    float basex = template_getfloat(t,&s_x,data,0);
    float basey = template_getfloat(t,&s_y,data,0);
    canvas_each(y,templatecanvas) {
        int hit = pd_getparentwidget(y)->w_parentclickfn(y, owner, data, t, sc, ap, basex+xloc, basey+yloc,
            xpix, ypix, shift, alt, dbl, doit);
	if (hit) return hit;
    }*/
    return 0;
}
#endif

static int scalar_click(t_gobj *z, t_canvas *owner, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    t_scalar *x = (t_scalar *)z;
    t_template *t = template_findbyname(x->t);
    return scalar_doclick(x->v, t, x, 0, owner, 0, 0, xpix, ypix, shift, alt, dbl, doit);
}

void canvas_writescalar(t_symbol *templatesym, t_word *w, t_binbuf *b, int amarrayelement);

static void scalar_save(t_gobj *z, t_binbuf *b) {
    t_scalar *x = (t_scalar *)z;
    t_binbuf *b2 = binbuf_new();
    canvas_writescalar(x->t, x->v, b2, 0);
    binbuf_addv(b,"tt","#X","scalar");
    binbuf_addbinbuf(b, b2);
    binbuf_addsemi(b);
    binbuf_free(b2);
}

/*
static void scalar_properties(t_gobj *z, t_canvas *owner) {
    t_scalar *x = (t_scalar *)z;
    char *buf, buf2[80];
    int bufsize;
    t_binbuf *b;
    b = canvas_writetobinbuf(owner, 0);
    binbuf_gettext(b, &buf, &bufsize);
    binbuf_free(b);
    buf = (char *)realloc(buf, bufsize+1);
    buf[bufsize] = 0;
    sprintf(buf2, "pdtk_data_dialog %%s {");
    sys_gui(buf);
    sys_gui("}\n");
    free(buf);
}
*/

static void scalar_free(t_scalar *x) {
    t_template *t = template_findbyname(x->t);
    TEMPLATE_CHECK(x->t,)
    word_free(x->v, t);
    /* the "size" field in the class is zero, so Pd doesn't try to free us automatically (see pd_free()) */
    free(x);
}

static void g_scalar_setup() {
    scalar_class = class_new2("scalar",0,scalar_free,0,CLASS_GOBJ,"");
    class_setsavefn(scalar_class, scalar_save);
}

void array_redraw(t_array *a, t_canvas *canvas);

/*
This file contains text objects you would put in a canvas to define a
template.  Templates describe objects of type "array" (g_array.c) and "scalar" (g_scalar.c). */
/* the structure of a "struct" object (also the obsolete "gtemplate" you get when using the name "template" in a box.) */
struct t_gtemplate : t_object {
    t_template *t;
    t_canvas *owner;
    t_symbol *sym;
    t_gtemplate *next;
    int argc;
    t_atom *argv;
};

static void template_conformarray(t_template *tfrom, t_template *tto, int *conformaction, t_array *a);
static void template_conformcanvas(t_template *tfrom, t_template *tto, int *conformaction, t_canvas *canvas);
static t_class *gtemplate_class;
static t_class *template_class;

/* there's a pre-defined "float" template.  LATER should we bind this to a symbol such as "pd-float"??? */

/* return true if two dataslot definitions match */
static int dataslot_matches(t_dataslot *ds1, t_dataslot *ds2, int nametoo) {
    return (!nametoo || ds1->name == ds2->name) && ds1->type == ds2->type &&
            (ds1->type != DT_ARRAY || ds1->arraytemplate == ds2->arraytemplate);
}

/* -- templates, the active ingredient in gtemplates defined below. ------- */

static t_template *template_new(t_symbol *templatesym, int argc, t_atom *argv) {
    t_template *x = (t_template *)pd_new(template_class);
    x->n = 0;
    x->vec = (t_dataslot *)getbytes(0);
    while (argc > 0) {
        int newtype, oldn, newn;
        t_symbol *newname, *newarraytemplate = &s_, *newtypesym;
        if (argc < 2 || argv[0].a_type != A_SYMBOL || argv[1].a_type != A_SYMBOL) goto bad;
        newtypesym = argv[0].a_symbol;
        newname = argv[1].a_symbol;
        if (newtypesym == &s_float)       newtype = DT_FLOAT;
        else if (newtypesym == &s_symbol) newtype = DT_SYMBOL;
        else if (newtypesym == &s_list)   newtype = DT_CANVAS;
        else if (newtypesym == gensym("array")) {
            if (argc < 3 || argv[2].a_type != A_SYMBOL) {error("array lacks element template or name"); goto bad;}
            newarraytemplate = canvas_makebindsym(argv[2].a_symbol);
            newtype = DT_ARRAY;
            argc--;
            argv++;
        } else {error("%s: no such type", newtypesym->name); goto bad;}
        newn = (oldn = x->n) + 1;
        x->vec = (t_dataslot *)realloc(x->vec, newn*sizeof(*x->vec));
        x->n = newn;
        x->vec[oldn].type = newtype;
        x->vec[oldn].name = newname;
        x->vec[oldn].arraytemplate = newarraytemplate;
    bad:
        argc -= 2; argv += 2;
    }
    x->sym = templatesym;
    if (templatesym->name) pd_bind(x,x->sym);
    return x;
}

int template_size(t_template *x) {return x->n * sizeof(t_word);}

int template_find_field(t_template *x, t_symbol *name, int *p_onset, int *p_type, t_symbol **p_arraytype) {
    if (!x) {bug("template_find_field"); return 0;}
    for (int i = 0; i<x->n; i++) if (x->vec[i].name == name) {
        *p_onset = i*sizeof(t_word);
        *p_type      = x->vec[i].type;
        *p_arraytype = x->vec[i].arraytemplate;
        return 1;
    }
    return 0;
}

#define ERR(msg,ret) do {\
	if (loud) error("%s.%s: "msg, x->sym->name, fieldname->name);\
	return ret;} while(0);
static t_float template_getfloat(t_template *x, t_symbol *fieldname, t_word *wp, int loud) {
    int onset, type; t_symbol *arraytype;
    if (!template_find_field(x, fieldname, &onset, &type, &arraytype)) ERR("no such field",0);
    if (type != DT_FLOAT) ERR("not a number",0);
    return *(t_float *)(((char *)wp) + onset);
}
static t_symbol *template_getsymbol(t_template *x, t_symbol *fieldname, t_word *wp, int loud) {
    int onset, type; t_symbol *arraytype;
    if (!template_find_field(x, fieldname, &onset, &type, &arraytype)) ERR("no such field",&s_);
    if (type != DT_SYMBOL) ERR("not a symbol",&s_);
    return *(t_symbol **)(((char *)wp) + onset);
}
static void template_setfloat(t_template *x, t_symbol *fieldname, t_word *wp, t_float f, int loud) {
    int onset, type; t_symbol *arraytype;
    if (!template_find_field(x, fieldname, &onset, &type, &arraytype)) ERR("no such field",);
    if (type != DT_FLOAT) ERR("not a number",);
    *(t_float *)(((char *)wp) + onset) = f;
}
static void template_setsymbol(t_template *x, t_symbol *fieldname, t_word *wp, t_symbol *s, int loud) {
    int onset, type; t_symbol *arraytype;
    if (!template_find_field(x, fieldname, &onset, &type, &arraytype)) ERR("no such field",);
    if (type != DT_SYMBOL) ERR("not a symbol",);
    *(t_symbol **)(((char *)wp) + onset) = s;
}
#undef ERR

/* stringent check to see if a "saved" template, x2, matches the current
one (x1).  It's OK if x1 has additional scalar elements but not (yet)
arrays or lists.  This is used for reading in "data files". */
static int template_match(t_template *x1, t_template *x2) {
    if (x1->n < x2->n) return 0;
    for (int i=x2->n; i < x1->n; i++)
        if (x1->vec[i].type == DT_ARRAY || x1->vec[i].type == DT_CANVAS) return 0;
    if (x2->n > x1->n) post("add elements...");
    for (int i=0; i < x2->n; i++) if (!dataslot_matches(&x1->vec[i], &x2->vec[i], 1)) return 0;
    return 1;
}

/* --------------- CONFORMING TO CHANGES IN A TEMPLATE ------------ */

/* the following functions handle updating scalars to agree with changes
in their template.  The old template is assumed to be the "installed" one
so we can delete old items; but making new ones we have to avoid scalar_new
which would make an old one whereas we will want a new one (but whose array
elements might still be old ones.)
    LATER deal with graphics updates too... */

/* conform the word vector of a scalar to the new template */    
static void template_conformwords(t_template *tfrom, t_template *tto, int *conformaction, t_word *wfrom, t_word *wto) {
    for (int i=0; i<tto->n; i++) {
        if (conformaction[i] >= 0) {
            /* we swap the two, in case it's an array or list, so that when "wfrom" is deleted the old one gets cleaned up. */
            t_word wwas = wto[i];
            wto[i] = wfrom[conformaction[i]];
            wfrom[conformaction[i]] = wwas;
        }
    }
}

/* conform a scalar, recursively conforming sublists and arrays  */
static t_scalar *template_conformscalar(t_template *tfrom, t_template *tto, int *conformaction, t_canvas *canvas, t_scalar *scfrom) {
    t_scalar *x;
    t_template *scalartemplate;
    /* possibly replace the scalar */
    if (scfrom->t == tfrom->sym) {
        t_gpointer gp;
        /* see scalar_new() for comment about the gpointer. */
        gpointer_init(&gp);
        x = (t_scalar *)getbytes(sizeof(t_scalar) + (tto->n - 1) * sizeof(*x->v));
        x->_class = scalar_class;
        x->t = tfrom->sym;
        gpointer_setcanvas(&gp, canvas, x);
        /* Here we initialize to the new template, but array and list elements will still belong to old template. */
        word_init(x->v, tto, &gp);
        template_conformwords(tfrom, tto, conformaction, scfrom->v, x->v);
        /* replace the old one with the new one in the list */
	canvas->boxes->remove_by_value(scfrom);
	canvas->boxes->add(x);
        pd_free(scfrom);
        scalartemplate = tto;
    } else {
        x = scfrom;
        scalartemplate = template_findbyname(x->t);
    }
    /* convert all array elements and sublists */
    for (int i=0; i < scalartemplate->n; i++) {
        t_dataslot *ds = scalartemplate->vec + i;
        if (ds->type == DT_CANVAS) template_conformcanvas(tfrom, tto, conformaction, x->v[i].w_canvas);
        if (ds->type == DT_ARRAY)  template_conformarray( tfrom, tto, conformaction, x->v[i].w_array);
    }
    return x;
}

/* conform an array, recursively conforming sublists and arrays  */
static void template_conformarray(t_template *tfrom, t_template *tto, int *conformaction, t_array *a) {
    t_template *scalartemplate = 0;
    if (a->templatesym == tfrom->sym) {
        /* the array elements must all be conformed */
        int oldelemsize = sizeof(t_word) * tfrom->n;
	int newelemsize = sizeof(t_word) *   tto->n;
        char *newarray = (char *)getbytes(newelemsize * a->n);
        char *oldarray = a->vec;
        if (a->elemsize != oldelemsize) bug("template_conformarray");
        for (int i=0; i<a->n; i++) {
            t_word *wp = (t_word *)(newarray + newelemsize*i);
            word_init(wp, tto, &a->gp);
            template_conformwords(tfrom, tto, conformaction, (t_word *)(oldarray + oldelemsize*i), wp);
            word_free((t_word *)(oldarray + oldelemsize*i), tfrom);
        }
        scalartemplate = tto;
        a->vec = newarray;
        free(oldarray);
    } else scalartemplate = template_findbyname(a->templatesym);
    /* convert all arrays and sublist fields in each element of the array */
    for (int i=0; i<a->n; i++) {
        t_word *wp = (t_word *)(a->vec + sizeof(t_word) * a->n * i);
        for (int j=0; j < scalartemplate->n; j++) {
            t_dataslot *ds = scalartemplate->vec + j;
            if (ds->type == DT_CANVAS) template_conformcanvas(tfrom, tto, conformaction, wp[j].w_canvas);
            if (ds->type == DT_ARRAY)  template_conformarray( tfrom, tto, conformaction, wp[j].w_array);
        }
    }
}

/* this routine searches for every scalar in the canvas that belongs
   to the "from" template and makes it belong to the "to" template.  Descend canvases recursively.
   We don't handle redrawing here; this is to be filled in LATER... */
t_array *garray_getarray(t_garray *x);
static void template_conformcanvas(t_template *tfrom, t_template *tto, int *conformaction, t_canvas *canvas) {
    canvas_each(g,canvas) {
	t_class *c = g->_class;
	/* what's the purpose of the assignment here?... consult original code */
        if      (c==scalar_class) g = template_conformscalar(tfrom, tto, conformaction, canvas, (t_scalar *)g);
        else if (c==canvas_class) template_conformcanvas(tfrom, tto,  conformaction, (t_canvas *)g);
        else if (c==garray_class) template_conformarray(tfrom, tto, conformaction, garray_getarray((t_garray *)g));
    }
}

/* globally conform all scalars from one template to another */
void template_conform(t_template *tfrom, t_template *tto) {
    int nto = tto->n, nfrom = tfrom->n, doit = 0;
    int *conformaction = (int *)getbytes(sizeof(int) * nto);
    int *conformedfrom = (int *)getbytes(sizeof(int) * nfrom);
    for (int i=0; i<  nto; i++) conformaction[i] = -1;
    for (int i=0; i<nfrom; i++) conformedfrom[i] = 0;
    for (int i=0; i < nto; i++) {
        t_dataslot *dataslot = &tto->vec[i];
        for (int j=0; j<nfrom; j++) {
            t_dataslot *dataslot2 = &tfrom->vec[j];
            if (dataslot_matches(dataslot, dataslot2, 1)) {
                conformaction[i] = j;
                conformedfrom[j] = 1;
            }
        }
    }
    for (int i=0; i<nto; i++) if (conformaction[i] < 0) {
        t_dataslot *dataslot = &tto->vec[i];
        for (int j=0; j<nfrom; j++)
          if (!conformedfrom[j] && dataslot_matches(dataslot, &tfrom->vec[j], 0)) {
            conformaction[i] = j;
            conformedfrom[j] = 1;
          }
    }
    if (nto != nfrom) doit = 1;
    else for (int i=0; i<nto; i++) if (conformaction[i] != i) doit = 1;
    if (doit) {
        post("conforming template '%s' to new structure", tfrom->sym->name);
        for (int i=0; i<nto; i++) post("... %d", conformaction[i]);
        foreach(gl,windowed_canvases) template_conformcanvas(tfrom, tto, conformaction, gl->first);
    }
    free(conformaction);
    free(conformedfrom);
}

t_template *template_findbyname(t_symbol *s) {return (t_template *)pd_findbyclass(s, template_class);}

t_canvas *template_findcanvas(t_template *t) {
    if (!t) bug("template_findcanvas");
    t_gtemplate *gt = t->list;
    if (!gt) return 0;
    return gt->owner;
    /* return ((t_canvas *)pd_findbyclass(t->sym, canvas_class)); */
}

void template_notify(t_template *t, t_symbol *s, int argc, t_atom *argv) {
    if (t->list) outlet_anything(t->list->outlet, s, argc, argv);
}

/* bash the first of (argv) with a pointer to a scalar, and send on
   to template as a notification message */
static void template_notifyforscalar(t_template *t, t_canvas *owner, t_scalar *sc, t_symbol *s, int argc, t_atom *argv) {
    t_gpointer gp;
    gpointer_init(&gp);
    gpointer_setcanvas(&gp, owner, sc);
    SETPOINTER(argv, &gp);
    template_notify(t, s, argc, argv);
    gpointer_unset(&gp);
}

/* call this when reading a patch from a file to declare what templates
   we'll need.  If there's already a template, check if it matches.
   If it doesn't it's still OK as long as there are no "struct" (gtemplate)
   objects hanging from it; we just conform everyone to the new template.
   If there are still struct objects belonging to the other template, we're
   in trouble.  LATER we'll figure out how to conform the new patch's objects
   to the pre-existing struct. */
static void *template_usetemplate(void *dummy, t_symbol *s, int argc, t_atom *argv) {
    t_symbol *templatesym = canvas_makebindsym(atom_getsymbolarg(0, argc, argv));
    t_template *x = (t_template *)pd_findbyclass(templatesym, template_class);
    if (!argc) return 0;
    argc--; argv++;
    if (x) {
        t_template *y = template_new(&s_, argc, argv);
        /* If the new template is the same as the old one, there's nothing to do.  */
        if (!template_match(x, y)) { /* Are there "struct" objects upholding this template? */
            if (x->list) {
                error("%s: template mismatch", templatesym->name);
            } else {
                template_conform(x, y);
                pd_free(x);
                t_template *y2 = template_new(templatesym, argc, argv);
                y2->list = 0;
            }
        }
        pd_free(y);
    } else template_new(templatesym, argc, argv);
    return 0;
}

/* here we assume someone has already cleaned up all instances of this. */
void template_free(t_template *x) {
    if (*x->sym->name) pd_unbind(x,x->sym);
    free(x->vec);
}

/* ---------------- gtemplates.  One per canvas. ----------- */

/* "Struct": an object that searches for, and if necessary creates,
a template (above).  Other objects in the canvas then can give drawing
instructions for the template.  The template doesn't go away when the
"struct" is deleted, so that you can replace it with another one to add new fields, for example. */
static void *gtemplate_donew(t_symbol *sym, int argc, t_atom *argv) {
    t_gtemplate *x = (t_gtemplate *)pd_new(gtemplate_class);
    t_template *t = template_findbyname(sym);
    x->owner = canvas_getcurrent();
    x->next = 0;
    x->sym = sym;
    x->argc = argc;
    x->argv = (t_atom *)getbytes(argc * sizeof(t_atom));
    for (int i=0; i<argc; i++) x->argv[i] = argv[i];
    /* already have a template by this name? */
    if (t) {
        x->t = t;
        /* if it's already got a "struct" object we
           just tack this one to the end of the list and leave it there. */
        if (t->list) {
            t_gtemplate *x2, *x3;
            for (x2 = x->t->list; (x3 = x2->next); x2 = x3) {}
            x2->next = x;
            post("template %s: warning: already exists.", sym->name);
        } else {
            /* if there's none, we just replace the template with our own and conform it. */
            t_template *y = template_new(&s_, argc, argv);
            //canvas_redrawallfortemplate(t, 2);
            /* Unless the new template is different from the old one, there's nothing to do.  */
            if (!template_match(t, y)) {
                /* conform everyone to the new template */
                template_conform(t, y);
                pd_free(t);
                t = template_new(sym, argc, argv);
            }
            pd_free(y);
            t->list = x;
            //canvas_redrawallfortemplate(t, 1);
        }
    } else {
        /* otherwise make a new one and we're the only struct on it. */
        x->t = t = template_new(sym, argc, argv);
        t->list = x;
    }
    outlet_new(x,0);
    return x;
}

static void *gtemplate_new(t_symbol *s, int argc, t_atom *argv) {
    t_symbol *sym = atom_getsymbolarg(0, argc, argv);
    if (argc >= 1) {argc--; argv++;}
    return (gtemplate_donew(canvas_makebindsym(sym), argc, argv));
}

/* old version (0.34) -- delete 2003 or so */
static void *gtemplate_new_old(t_symbol *s, int argc, t_atom *argv) {
    t_symbol *sym = canvas_makebindsym(canvas_getcurrent()->name);
    static int warned;
    if (!warned) {
        post("warning -- 'template' (%s) is obsolete; replace with 'struct'", sym->name);
        warned = 1;
    }
    return gtemplate_donew(sym, argc, argv);
}

t_template *gtemplate_get(t_gtemplate *x) {return x->t;}

static void gtemplate_free(t_gtemplate *x) {
    /* get off the template's list */
    t_template *t = x->t;
    if (x == t->list) {
        //canvas_redrawallfortemplate(t, 2);
        if (x->next) {
            /* if we were first on the list, and there are others on the list, make a new template corresponding
               to the new first-on-list and replace the existing template with it. */
            t_template *z = template_new(&s_, x->next->argc, x->next->argv);
            template_conform(t, z);
            pd_free(t);
            pd_free(z);
            z = template_new(x->sym, x->next->argc, x->next->argv);
            z->list = x->next;
            for (t_gtemplate *y=z->list; y ; y=y->next) y->t=z;
        } else t->list = 0;
        //canvas_redrawallfortemplate(t, 1);
    } else {
        t_gtemplate *x2, *x3;
        for (x2=t->list; (x3=x2->next); x2=x3) if (x==x3) {x2->next=x3->next; break;}
    }
    free(x->argv);
}

/* ---------------  FIELD DESCRIPTORS (NOW CALLED SLOT) ---------------------- */
/* a field descriptor can hold a constant or a variable's name; in the latter case,
   it's the name of a field in the template we belong to. LATER, we might want to cache the offset
   of the field so we don't have to search for it every single time we draw the object.
*/
/* note: there is also t_dataslot which plays a similar role. could they be merged someday? */

struct _slot {
    char type;       /* LATER consider removing this? */
    char var;
    union {
        t_float f;        /* the field is a constant float */
        t_symbol *s;      /* the field is a constant symbol */
        t_symbol *varsym; /* the field is variable and this is the name */
    };
    float min,max;
    float scrmin,scrmax; /* min and max screen values */
    float quantum; /* quantization in value */
};

static void slot_setfloat_const( t_slot *fd, float f) {
    fd->type = A_FLOAT;  fd->var = 0; fd->f = f; fd->min = fd->max = fd->scrmin = fd->scrmax = fd->quantum = 0;}
static void slot_setsymbol_const(t_slot *fd, t_symbol *s) {
    fd->type = A_SYMBOL; fd->var = 0; fd->s = s; fd->min = fd->max = fd->scrmin = fd->scrmax = fd->quantum = 0;}

static void slot_setfloat_var(t_slot *fd, t_symbol *s) {
    char *s1, *s2, *s3;
    fd->type = A_FLOAT;
    fd->var = 1;
    if (!(s1 = strchr(s->name, '(')) || !(s2 = strchr(s->name, ')')) || s1>s2) {
        fd->varsym = s;
        fd->min = fd->max = fd->scrmin = fd->scrmax = fd->quantum = 0;
    } else {
        fd->varsym = symprintf("%.*s",s1-s->name,s->name);
        t_int got = sscanf(s1, "(%f:%f)(%f:%f)(%f)", &fd->min, &fd->max, &fd->scrmin, &fd->scrmax, &fd->quantum);
        if (got < 2) goto fail;
        if (got == 3 || (got < 4 && strchr(s2, '('))) goto fail;
        if (got < 5 && (s3 = strchr(s2, '(')) && strchr(s3+1, '(')) goto fail;
        if (got == 4) fd->quantum = 0;
        else if (got == 2) {
            fd->quantum = 0;
            fd->scrmin = fd->min;
            fd->scrmax = fd->max;
        }
        return;
    fail:
        error("parse error: %s", s->name);
        fd->min = fd->scrmin = fd->max = fd->scrmax = fd->quantum = 0;
    }
}

#define CLOSED 1
#define BEZ 2
#define NOMOUSE 4
#define A_ARRAY 55      /* LATER decide whether to enshrine this in m_pd.h */

static void slot_setfloatarg(t_slot *fd, int argc, t_atom *argv) {
        if (argc <= 0) slot_setfloat_const(fd, 0);
        else if (argv->a_type == A_SYMBOL) slot_setfloat_var(  fd, argv->a_symbol);
        else                               slot_setfloat_const(fd, argv->a_float);
}
static void slot_setsymbolarg(t_slot *fd, int argc, t_atom *argv) {
        if (argc <= 0) slot_setsymbol_const(fd, &s_);
        else if (argv->a_type == A_SYMBOL) {
            fd->type = A_SYMBOL;
            fd->var = 1;
            fd->varsym = argv->a_symbol;
            fd->min = fd->max = fd->scrmin = fd->scrmax = fd->quantum = 0;
        } else slot_setsymbol_const(fd, &s_);
}
static void slot_setarrayarg(t_slot *fd, int argc, t_atom *argv) {
        if (argc <= 0) slot_setfloat_const(fd, 0);
        else if (argv->a_type == A_SYMBOL) {
            fd->type = A_ARRAY;
            fd->var = 1;
            fd->varsym = argv->a_symbol;
        } else slot_setfloat_const(fd, argv->a_float);
}

/* getting and setting values via slots -- note confusing names; the above are setting up the slot itself. */

/* convert a variable's value to a screen coordinate via its slot */
static t_float slot_cvttocoord(t_slot *f, float val) {
    float coord, extreme, div;
    if (f->max == f->min) return val;
    div = (f->scrmax - f->scrmin)/(f->max - f->min);
    coord = f->scrmin + (val - f->min) * div;
    extreme = f->scrmin<f->scrmax ? f->scrmin : f->scrmax; if (coord<extreme) coord = extreme;
    extreme = f->scrmin>f->scrmax ? f->scrmin : f->scrmax; if (coord>extreme) coord = extreme;
    return coord;
}

/* read a variable via slot and convert to screen coordinate */
static t_float slot_getcoord(t_slot *f, t_template *t, t_word *wp, int loud) {
    if (f->type!=A_FLOAT) {if (loud) error("symbolic data field used as number"); return 0;}
    if (f->var) return slot_cvttocoord(f, template_getfloat(t, f->varsym, wp, loud));
    return f->f;
}
static t_float slot_getfloat(t_slot *f, t_template *t, t_word *wp, int loud) {
    if (f->type!=A_FLOAT) {if (loud) error("symbolic data field used as number"); return 0;}
    if (f->var) return template_getfloat(t, f->varsym, wp, loud);
    return f->f;
}
static t_symbol *slot_getsymbol(t_slot *f, t_template *t, t_word *wp, int loud) {
    if (f->type!=A_SYMBOL) {if (loud) error("numeric data field used as symbol"); return &s_;}
    if (f->var) return template_getsymbol(t, f->varsym, wp, loud);
    return f->s;
}

/* convert from a screen coordinate to a variable value */
static float slot_cvtfromcoord(t_slot *f, float coord) {
    if (f->scrmax == f->scrmin) return coord;
    else {
        float div = (f->max - f->min)/(f->scrmax - f->scrmin);
        float extreme;
	float val = f->min + (coord - f->scrmin) * div;
        if (f->quantum != 0) val = ((int)((val/f->quantum) + 0.5)) *  f->quantum;
        extreme = f->min<f->max ? f->min : f->max; if (val<extreme) val=extreme;
        extreme = f->min>f->max ? f->min : f->max; if (val>extreme) val=extreme;
	return val;
    }
 }

static void slot_setcoord(t_slot *f, t_template *t, t_word *wp, float coord, int loud) {
    if (f->type == A_FLOAT && f->var) {
        float val = slot_cvtfromcoord(f, coord);
        template_setfloat(t, f->varsym, wp, val, loud);
    } else {
        if (loud) error("attempt to set constant or symbolic data field to a number");
    }
}

#define FIELDSET(T,F,D) if (argc) slot_set##T##arg(&x->F,argc--,argv++); \
	else slot_setfloat_const(&x->F,D);

/* curves belong to templates and describe how the data in the template are to be drawn.
   The coordinates of the curve (and other display features) can be attached to fields in the template. */

t_class *curve_class;

/* includes polygons too */
struct t_curve : t_object {
    int flags; /* CLOSED and/or BEZ and/or NOMOUSE */
    t_slot fillcolor, outlinecolor, width, vis;
    int npoints;
    t_slot *vec;
    t_canvas *canvas;
};

static void *curve_new(t_symbol *classsym, t_int argc, t_atom *argv) {
    t_curve *x = (t_curve *)pd_new(curve_class);
    char *classname = classsym->name;
    int flags = 0;
    x->canvas = canvas_getcurrent();
    if (classname[0] == 'f') {classname += 6; flags |= CLOSED;} else classname += 4;
    slot_setfloat_const(&x->vis, 1);
    if (classname[0] == 'c') flags |= BEZ;
    while (1) {
        t_symbol *firstarg = atom_getsymbolarg(0, argc, argv);
        if (!strcmp(firstarg->name,"-v") && argc > 1) {
            slot_setfloatarg(&x->vis, 1, argv+1);
            argc -= 2; argv += 2;
        } else
	if (!strcmp(firstarg->name,"-x")) {
	    flags |= NOMOUSE;
	    argc -= 1; argv += 1;
        } else break;
    }
    x->flags = flags;
    if (flags&CLOSED&&argc) slot_setfloatarg(   &x->fillcolor, argc--,argv++);
    else                    slot_setfloat_const(&x->fillcolor, 0);
    FIELDSET(float,outlinecolor,0);
    FIELDSET(float,width,1);
    if (argc < 0) argc = 0;
    int nxy = argc + (argc&1);
    x->npoints = nxy>>1;
    x->vec = (t_slot *)getbytes(nxy * sizeof(t_slot));
    t_slot *fd = x->vec;
    for (int i=0; i<argc; i++, fd++, argv++) slot_setfloatarg(fd, 1, argv);
    if (argc & 1) slot_setfloat_const(fd, 0);
    return x;
}

static void curve_float(t_curve *x, t_floatarg f) {
    if (x->vis.type != A_FLOAT || x->vis.var) {
        error("global vis/invis for a template with variable visibility");
        return;
    }
    int viswas = x->vis.f!=0;
    if ((f!=0 && viswas) || (f==0 && !viswas)) return;
    canvas_redrawallfortemplatecanvas(x->canvas, 2);
    slot_setfloat_const(&x->vis, f!=0);
    canvas_redrawallfortemplatecanvas(x->canvas, 1);
}

static int rangecolor(int n) {return n*9/255;}

static void numbertocolor(int n, char *s) {
    n = (int)max(n,0);
    sprintf(s, "#%2.2x%2.2x%2.2x", rangecolor(n/100), rangecolor((n/10)%10), rangecolor(n%10));
}

static void curve_vis(t_gobj *z, t_canvas *canvas, t_word *data, t_template *t, float basex, float basey, int vis) {
    t_curve *x = (t_curve *)z;
    int n = x->npoints;
    t_slot *f = x->vec;
    if (!slot_getfloat(&x->vis, t, data, 0)) return;
    if (vis) {
        if (n > 1) {
            int flags = x->flags;
            char outline[20], fill[20];
            int pix[200];
            if (n > 100) n = 100;
            /* calculate the pixel values before we start printing out the TK message so that
	       "error" printout won't be interspersed with it.  Only show up to 100 points so we don't
               have to allocate memory here. */
            for (int i=0; i<n; i++, f += 2) {
                pix[2*i  ] = canvas_xtopixels(canvas, basex + slot_getcoord(f+0, t, data, 1));
                pix[2*i+1] = canvas_ytopixels(canvas, basey + slot_getcoord(f+1, t, data, 1));
            }
            numbertocolor((int)slot_getfloat(&x->outlinecolor, t, data, 1), outline);
            if (flags & CLOSED) {
                numbertocolor((int)slot_getfloat(&x->fillcolor, t, data, 1), fill);
                //sys_vgui(".x%lx.c create polygon\\\n", (long)canvas_getcanvas(canvas));
            } else sys_vgui(".x%lx.c create line\\\n", (long)canvas_getcanvas(canvas));
            for (int i=0; i<n; i++) sys_vgui("%d %d\\\n", pix[2*i], pix[2*i+1]);
            sys_vgui("-width %f\\\n", max(slot_getfloat(&x->width, t, data, 1),1.0f));
            if (flags & CLOSED) sys_vgui("-fill %s -outline %s\\\n", fill, outline);
            else sys_vgui("-fill %s\\\n", outline);
            if (flags & BEZ) sys_vgui("-smooth 1\\\n");
            sys_vgui("-tags curve%lx\n", (long)data);
        } else post("warning: curves need at least two points to be graphed");
    } else {
        if (n > 1) sys_vgui(".x%lx.c delete curve%lx\n", (long)canvas_getcanvas(canvas), (long)data);
    }
}

static struct {
  int field;
  float xcumulative, xbase, xper;
  float ycumulative, ybase, yper;
  t_canvas *canvas;
  t_scalar *scalar;
  t_array *array;
  t_word *wp;
  t_template *t;
  t_gpointer gpointer;
} cm;

/* LATER protect against the template changing or the scalar disappearing probably by attaching a gpointer here ... */
#if 0
static void curve_motion(void *z, t_floatarg dx, t_floatarg dy) {
    t_curve *x = (t_curve *)z;
    t_slot *f = x->vec + cm.field;
    t_atom at;
    if (!gpointer_check(&curve_motion_gpointer, 0)) {post("curve_motion: scalar disappeared"); return;}
    cm.xcumulative += dx;
    cm.ycumulative += dy;
    if (f[0].var && dx!=0) slot_setcoord(f,   cm.t, cm.wp, cm.xbase + cm.xcumulative*cm.xper, 1);
    if (f[1].var && dy!=0) slot_setcoord(f+1, cm.t, cm.wp, cm.ybase + cm.ycumulative*cm.yper, 1);
    /* LATER figure out what to do to notify for an array? */
    if (cm.scalar) template_notifyforscalar(cm.t, cm.canvas, cm.scalar, gensym("change"), 1, &at);
    if (cm.scalar) gobj_changed(cm.scalar,0); else gobj_changed(cm.array,0);
}
#endif

int iabs(int a) {return a<0?-a:a;}

static int curve_click(t_gobj *z, t_canvas *canvas, t_word *data, t_template *t, t_scalar *sc,
t_array *ap, float basex, float basey, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    t_curve *x = (t_curve *)z;
    int bestn = -1;
    int besterror = 0x7fffffff;
    t_slot *f = x->vec;
    if (!slot_getfloat(&x->vis, t, data, 0)) return 0;
    for (int i=0; i<x->npoints; i++, f += 2) {
        int xval = (int)slot_getcoord(f  , t, data, 0), xloc = canvas_xtopixels(canvas, basex + xval);
        int yval = (int)slot_getcoord(f+1, t, data, 0), yloc = canvas_ytopixels(canvas, basey + yval);
        int xerr = iabs(xloc-xpix);
	int yerr = iabs(yloc-ypix);
        if (!f->var && !(f+1)->var) continue;
        if (yerr > xerr) xerr = yerr;
        if (xerr < besterror) {
            cm.xbase = xval;
            cm.ybase = yval;
            besterror = xerr;
            bestn = i;
        }
    }
    if (besterror > 10) return 0;
    if (doit) {
        cm.xper = canvas_pixelstox(canvas, 1) - canvas_pixelstox(canvas, 0);
        cm.yper = canvas_pixelstoy(canvas, 1) - canvas_pixelstoy(canvas, 0);
        cm.xcumulative = cm.ycumulative = 0;
        cm.canvas = canvas;
        cm.scalar = sc;
        cm.array = ap;
        cm.wp = data;
        cm.field = 2*bestn;
        cm.t = t;
        if (cm.scalar) gpointer_setcanvas(&cm.gpointer, cm.canvas, cm.scalar);
        else gpointer_setarray(&cm.gpointer, cm.array, cm.wp);
        /* canvas_grab(canvas, z, curve_motion, 0, xpix, ypix); */
    }
    return 1;
}

t_class *plot_class;

struct t_plot : t_object {
    t_canvas *canvas;
    t_slot outlinecolor, width, xloc, yloc, xinc, style;
    t_slot data, xpoints, ypoints, wpoints;
    t_slot vis;
    t_slot scalarvis; /* true if drawing the scalar at each point */
};

static void *plot_new(t_symbol *classsym, t_int argc, t_atom *argv) {
    t_plot *x = (t_plot *)pd_new(plot_class);
    int defstyle = PLOTSTYLE_POLY;
    x->canvas = canvas_getcurrent();
    slot_setfloat_var(&x->xpoints,&s_x);
    slot_setfloat_var(&x->ypoints,&s_y);
    slot_setfloat_var(&x->wpoints, gensym("w"));
    slot_setfloat_const(&x->vis, 1);
    slot_setfloat_const(&x->scalarvis, 1);
    while (1) {
	const char *f = atom_getsymbolarg(0, argc, argv)->name;
	argc--; argv++;
        if (!strcmp(f, "curve") || !strcmp(f, "-c")) defstyle = PLOTSTYLE_BEZ;
        else if (!strcmp(f,"-v") &&argc>0) {slot_setfloatarg(&x->vis,      1,argv+1);argc--;argv++;}
        else if (!strcmp(f,"-vs")&&argc>0) {slot_setfloatarg(&x->scalarvis,1,argv+1);argc--;argv++;}
        else if (!strcmp(f,"-x") &&argc>0) {slot_setfloatarg(&x->xpoints,  1,argv+1);argc--;argv++;}
        else if (!strcmp(f,"-y") &&argc>0) {slot_setfloatarg(&x->ypoints,  1,argv+1);argc--;argv++;}
	else if (!strcmp(f,"-w") &&argc>0) {slot_setfloatarg(&x->wpoints,  1,argv+1);argc--;argv++;}
        else break;
    }
    FIELDSET(array,data,1);
    FIELDSET(float,outlinecolor,0);
    FIELDSET(float,width,1);
    FIELDSET(float,xloc,1);
    FIELDSET(float,yloc,1);
    FIELDSET(float,xinc,1);
    FIELDSET(float,style,defstyle);
    return x;
}

void plot_float(t_plot *x, t_floatarg f) {
    int viswas;
    if (x->vis.type != A_FLOAT || x->vis.var) {error("global vis/invis for a template with variable visibility"); return;}
    viswas = x->vis.f!=0;
    if ((f!=0 && viswas) || (f==0 && !viswas)) return;
    canvas_redrawallfortemplatecanvas(x->canvas, 2);
    slot_setfloat_const(&x->vis, f!=0);
    canvas_redrawallfortemplatecanvas(x->canvas, 1);
}

/* get everything we'll need from the owner template of the array being
   plotted. Not used for garrays, but see below */
static int plot_readownertemplate(t_plot *x, t_word *data, t_template *ownertemplate,
t_symbol **elemtemplatesymp, t_array **arrayp, float *linewidthp, float *xlocp, float *xincp, float *ylocp,
float *stylep, float *visp, float *scalarvisp) {
    int arrayonset, type;
    t_symbol *elemtemplatesym;
    t_array *array;
    if (x->data.type != A_ARRAY || !x->data.var) {error("needs an array field"); return -1;}
    if (!template_find_field(ownertemplate, x->data.varsym, &arrayonset, &type, &elemtemplatesym)) {
        error("%s: no such field", x->data.varsym->name);
        return -1;
    }
    if (type != DT_ARRAY) {error("%s: not an array", x->data.varsym->name); return -1;}
    array = *(t_array **)(((char *)data) + arrayonset);
    *linewidthp = slot_getfloat(&x->width, ownertemplate, data, 1);
    *xlocp  = slot_getfloat(&x->xloc,  ownertemplate, data, 1);
    *xincp  = slot_getfloat(&x->xinc,  ownertemplate, data, 1);
    *ylocp  = slot_getfloat(&x->yloc,  ownertemplate, data, 1);
    *stylep = slot_getfloat(&x->style, ownertemplate, data, 1);
    *visp   = slot_getfloat(&x->vis,   ownertemplate, data, 1);
    *scalarvisp = slot_getfloat(&x->scalarvis, ownertemplate, data, 1);
    *elemtemplatesymp = elemtemplatesym;
    *arrayp = array;
    return 0;
}

/* get everything else you could possibly need about a plot,
   either for plot's own purposes or for plotting a "garray" */
static int array_getfields(t_symbol *elemtemplatesym, t_canvas **elemtemplatecanvasp,
t_template **elemtemplatep, int *elemsizep, t_slot *xslot, t_slot *yslot, t_slot *wslot,
int *xonsetp, int *yonsetp, int *wonsetp) {
    int type;
    t_symbol *dummy, *varname;
    t_canvas *elemtemplatecanvas = 0;
    /* the "float" template is special in not having to have a canvas;
       template_findbyname is hardwired to return a predefined template. */
    t_template *elemtemplate = template_findbyname(elemtemplatesym);
    if (!elemtemplate) {error("%s: no such template", elemtemplatesym->name); return -1;}
    if (!(elemtemplatesym==&s_float || (elemtemplatecanvas = template_findcanvas(elemtemplate)))) {
        error("%s: no canvas for this template", elemtemplatesym->name);
        return -1;
    }
    *elemtemplatecanvasp = elemtemplatecanvas;
    *elemtemplatep = elemtemplate;
    *elemsizep = elemtemplate->n * sizeof(t_word);
#define FOO(f,name,onset) \
  varname = f && f->var ? f->varsym : gensym(name); \
  if (!template_find_field(elemtemplate,varname,&onset,&type,&dummy) || type!=DT_FLOAT) onset=-1;
    FOO(yslot,"y",*yonsetp)
    FOO(xslot,"x",*xonsetp)
    FOO(wslot,"w",*wonsetp)
#undef FOO
    return 0;
}

static void plot_vis(t_gobj *z, t_canvas *canvas, t_word *data, t_template *t, float basex, float basey, int tovis) {
    t_plot *x = (t_plot *)z;
    int elemsize, yonset, wonset, xonset;
    t_canvas *elemtemplatecanvas;
    t_template *elemtemplate;
    t_symbol *elemtemplatesym;
    float linewidth, xloc, xinc, yloc, style, xsum, yval, vis, scalarvis;
    t_array *array;
    if (plot_readownertemplate(x, data, t, &elemtemplatesym, &array, &linewidth, &xloc, &xinc, &yloc, &style, &vis, &scalarvis)) return;
    t_slot *xslot = &x->xpoints, *yslot = &x->ypoints, *wslot = &x->wpoints;
    if (!vis) return;
    if (array_getfields(elemtemplatesym, &elemtemplatecanvas, &elemtemplate, &elemsize,
	xslot, yslot, wslot, &xonset, &yonset, &wonset)) return;
    int nelem = array->n;
    char *elem = (char *)array->vec;
    if (tovis) {
        if (style == PLOTSTYLE_POINTS) {
            float minyval = 1e20, maxyval = -1e20;
            int ndrawn = 0;
	    xsum = basex + xloc;
            for (int i=0; i<nelem; i++) {
                float yval;
                int ixpix, inextx;
                if (xonset >= 0) {
                    float usexloc = basex + xloc + *(float *)((elem + elemsize*i) + xonset);
                    ixpix  = canvas_xtopixels(canvas, slot_cvttocoord(xslot, usexloc));
                    inextx = ixpix + 2;
                } else {
                    ixpix  = canvas_xtopixels(canvas, slot_cvttocoord(xslot, xsum)); xsum += xinc;
                    inextx = canvas_xtopixels(canvas, slot_cvttocoord(xslot, xsum));
                }
                yval = yonset>=0 ? yloc + *(float *)((elem + elemsize*i) + yonset) : 0;
                if (yval > maxyval) maxyval = yval;
                if (yval < minyval) minyval = yval;
                if (i == nelem-1 || inextx != ixpix) {
                    sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -width 0  -tags plot%lx\n",
                        (long)canvas_getcanvas(canvas), ixpix,
			(int) canvas_ytopixels(canvas, basey + slot_cvttocoord(yslot, minyval)), inextx,
			(int)(canvas_ytopixels(canvas, basey + slot_cvttocoord(yslot, maxyval))+linewidth),
			(long)data);
                    ndrawn++;
                    minyval = 1e20;
                    maxyval = -1e20;
                }
                if (ndrawn > 2000 || ixpix >= 3000) break;
            }
        } else {
            char outline[20];
            int lastpixel = -1, ndrawn = 0;
            float yval = 0, wval = 0;
            int ixpix = 0;
	    /* draw the trace */
            numbertocolor((int)slot_getfloat(&x->outlinecolor, t, data, 1), outline);
            if (wonset >= 0) {
	        /* found "w" field which controls linewidth.  The trace is a filled polygon with 2n points. */
                //sys_vgui(".x%lx.c create polygon \\\n", (long)canvas_getcanvas(canvas));
		xsum = xloc;
                for (int i=0; i<nelem; i++) {
                    float usexloc = xonset>=0 ? xloc+*(float *)(elem+elemsize*i+xonset) : (xsum+=xinc);
                    float    yval = yonset>=0 ?      *(float *)(elem+elemsize*i+yonset) : 0;
                    wval = *(float *)(elem+elemsize*i+wonset);
                    float xpix = canvas_xtopixels(canvas, basex + slot_cvttocoord(xslot, usexloc));
                    ixpix = (int)roundf(xpix);
                    if (xonset >= 0 || ixpix != lastpixel) {
                        sys_vgui("%d %f \\\n", ixpix, canvas_ytopixels(canvas,
				basey + slot_cvttocoord(yslot,yval+yloc)
				      - slot_cvttocoord(wslot,wval)));
                        ndrawn++;
                    }
                    lastpixel = ixpix;
                    if (ndrawn >= 1000) goto ouch;
                }
                lastpixel = -1;
                for (int i=nelem-1; i>=0; i--) {
                    float usexloc = xonset>=0 ? xloc+*(float *)(elem+elemsize*i+xonset) : (xsum-=xinc);
                    float    yval = yonset>=0 ?      *(float *)(elem+elemsize*i+yonset) : 0;
                    wval = *(float *)((elem + elemsize*i) + wonset);
                    float xpix = canvas_xtopixels(canvas, basex + slot_cvttocoord(xslot, usexloc));
                    ixpix = (int)roundf(xpix);
                    if (xonset >= 0 || ixpix != lastpixel) {
                        sys_vgui("%d %f \\\n", ixpix, canvas_ytopixels(canvas,
                        	basey + yloc + slot_cvttocoord(yslot, yval) + slot_cvttocoord(wslot, wval)));
                        ndrawn++;
                    }
                    lastpixel = ixpix;
                    if (ndrawn >= 1000) goto ouch;
                }
                /* TK will complain if there aren't at least 3 points. There should be at least two already. */
                if (ndrawn < 4) {
		    int y = int(slot_cvttocoord(yslot, yval));
		    int w = int(slot_cvttocoord(wslot, wval));
                    sys_vgui("%d %f %d %f\\\n",
			ixpix + 10, canvas_ytopixels(canvas, basey + yloc + y + w),
                        ixpix + 10, canvas_ytopixels(canvas, basey + yloc + y - w));
                }
            ouch:
                sys_vgui(" -width 1 -fill %s -outline %s\\\n", outline, outline);
                if (style == PLOTSTYLE_BEZ) sys_vgui("-smooth 1\\\n");
                sys_vgui("-tags plot%lx\n", (long)data);
	    } else if (linewidth > 0) {
		/* no "w" field.  If the linewidth is positive, draw a segmented line with the
		   requested width; otherwise don't draw the trace at all. */
                sys_vgui(".x%lx.c create line \\\n", (long)canvas_getcanvas(canvas));
		xsum = xloc;
                for (int i=0; i<nelem; i++) {
                    float usexloc = xonset>=0 ? xloc+*(float *)(elem+elemsize*i+xonset) : xsum; if (xonset>=0) xsum+=(int)xinc;
                    float    yval = yonset>=0 ?      *(float *)(elem+elemsize*i+yonset) : 0;
                    float xpix = canvas_xtopixels(canvas, basex + slot_cvttocoord(xslot, usexloc));
                    ixpix = (int)roundf(xpix);
                    if (xonset >= 0 || ixpix != lastpixel) {
                        sys_vgui("%d %f \\\n", ixpix, canvas_ytopixels(canvas, basey + yloc + slot_cvttocoord(yslot, yval)));
                        ndrawn++;
                    }
                    lastpixel = ixpix;
                    if (ndrawn >= 1000) break;
                }
                /* TK will complain if there aren't at least 2 points... */
                if (ndrawn == 0) sys_vgui("0 0 0 0 \\\n");
                else if (ndrawn == 1) sys_vgui("%d %f \\\n", ixpix + 10,
                    canvas_ytopixels(canvas, basey + yloc + slot_cvttocoord(yslot, yval)));
                sys_vgui("-width %f -fill %s -smooth %d -tags plot%lx",linewidth,outline,style==PLOTSTYLE_BEZ,(long)data);
            }
        }
	/* We're done with the outline; now draw all the points. This code is inefficient since
	   the template has to be searched for drawing instructions for every last point. */
        if (scalarvis != 0) {
	    int xsum = (int)xloc;
            for (int i=0; i<nelem; i++) {
                //float usexloc = xonset>=0 ? basex + xloc + *(float *)(elem+elemsize*i+xonset) : basex+xsum;
		if (xonset>=0) xsum+=int(xinc);
                yval = yonset>=0 ? *(float *)(elem+elemsize*i+yonset) : 0;
                //float useyloc = basey + yloc + slot_cvttocoord(yslot, yval);
                /*canvas_each(y,elemtemplatecanvas) pd_getparentwidget(y)->w_parentvisfn(y, canvas,
			(t_word *)(elem+elemsize*i), elemtemplate, usexloc, useyloc, tovis);*/
            }
        }
    } else {
	/* un-draw the individual points */
	/* if (scalarvis != 0)
            for (int i=0; i<nelem; i++)
                canvas_each(y,elemtemplatecanvas)
                    pd_getparentwidget(y)->w_parentvisfn(y, canvas, (t_word *)(elem+elemsize*i), elemtemplate,0,0,0);*/
	/* and then the trace */
        sys_vgui(".x%lx.c delete plot%lx\n", (long)canvas_getcanvas(canvas), (long)data);
    }
}

static int plot_click(t_gobj *z, t_canvas *canvas, t_word *data, t_template *t, t_scalar *sc,
t_array *ap, float basex, float basey, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    t_plot *x = (t_plot *)z;
    t_symbol *elemtemplatesym;
    float linewidth, xloc, xinc, yloc, style, vis, scalarvis;
    t_array *array;
    if (plot_readownertemplate(x, data, t, &elemtemplatesym, &array, &linewidth, &xloc, &xinc,
	&yloc, &style, &vis, &scalarvis)) return 0;
    if (!vis) return 0;
    return array_doclick(array, canvas, sc, ap, elemtemplatesym, linewidth, basex + xloc, xinc,
	basey + yloc, scalarvis, &x->xpoints, &x->ypoints, &x->wpoints, xpix, ypix, shift, alt, dbl, doit);
}

/* ---------------- drawnumber: draw a number (or symbol) ---------------- */
/*  drawnumbers draw numeric fields at controllable locations, with controllable color and label.
    invocation: (drawnumber|drawsymbol) [-v <visible>] variable x y color label */

t_class *drawnumber_class;

#define DRAW_SYMBOL 1

struct t_drawnumber : t_object {
    t_slot value, xloc, yloc, color, vis;
    t_symbol *label;
    int flags;
    t_canvas *canvas;
};

static void *drawnumber_new(t_symbol *classsym, t_int argc, t_atom *argv) {
    t_drawnumber *x = (t_drawnumber *)pd_new(drawnumber_class);
    char *classname = classsym->name;
    int flags = 0;
    if (classname[4] == 's') flags |= DRAW_SYMBOL;
    x->flags = flags;
    slot_setfloat_const(&x->vis, 1);
    x->canvas = canvas_getcurrent();
    while (1) {
        t_symbol *firstarg = atom_getsymbolarg(0, argc, argv);
        if (!strcmp(firstarg->name,"-v") && argc > 1) {
            slot_setfloatarg(&x->vis, 1, argv+1);
            argc -= 2; argv += 2;
        } else break;
    }
    if (flags & DRAW_SYMBOL) {
        if (argc) slot_setsymbolarg(   &x->value,argc--,argv++);
	else      slot_setsymbol_const(&x->value,&s_);
    } else FIELDSET(float,value, 0);
    FIELDSET(float,xloc,0);
    FIELDSET(float,yloc,0);
    FIELDSET(float,color,1);
    if (argc) x->label = atom_getsymbolarg(0, argc, argv); else x->label = &s_;
    return x;
}

void drawnumber_float(t_drawnumber *x, t_floatarg f) {
    if (x->vis.type != A_FLOAT || x->vis.var) {error("global vis/invis for a template with variable visibility"); return;}
    int viswas = x->vis.f!=0;
    if ((f != 0 && viswas) || (f == 0 && !viswas)) return;
    canvas_redrawallfortemplatecanvas(x->canvas, 2);
    slot_setfloat_const(&x->vis, f!=0);
    canvas_redrawallfortemplatecanvas(x->canvas, 1);
}

static void drawnumber_vis(t_gobj *z, t_canvas *canvas, t_word *data, t_template *t, float basex, float basey, int vis) {
    t_drawnumber *x = (t_drawnumber *)z;
    if (!slot_getfloat(&x->vis, t, data, 0)) return;
    if (vis) {
        t_atom at;
        int xloc = canvas_xtopixels(canvas, basex + slot_getcoord(&x->xloc, t, data, 0));
        int yloc = canvas_ytopixels(canvas, basey + slot_getcoord(&x->yloc, t, data, 0));
        char colorstring[20];
        numbertocolor((int)slot_getfloat(&x->color, t, data, 1), colorstring);
        if (x->flags & DRAW_SYMBOL) SETSYMBOL(&at, slot_getsymbol(&x->value, t, data, 0));
        else                        SETFLOAT( &at, slot_getfloat( &x->value, t, data, 0));
        std::ostringstream buf;
	buf << x->label->name;
	atom_ostream(&at,buf);
        sys_vgui(".x%lx.c create text %d %d -anchor nw -fill %s -text {%s}",
                (long)canvas_getcanvas(canvas), xloc, yloc, colorstring, buf.str().data());
        sys_vgui(" -font {Courier 42} -tags drawnumber%lx\n", (long)data); /*sys_hostfontsize(canvas_getfont(canvas))*/
    } else sys_vgui(".x%lx.c delete drawnumber%lx\n", (long)canvas_getcanvas(canvas), (long)data);
}

static struct {
  float ycumulative;
  t_canvas *canvas;
  t_scalar *scalar;
  t_array *array;
  t_word *wp;
  t_template *t;
  t_gpointer gpointer;
  int symbol;
  int firstkey;
} dn;

/* LATER protect against the template changing or the scalar disappearing probably by attaching a gpointer here ... */
#if 0
static void drawnumber_motion(void *z, t_floatarg dx, t_floatarg dy) {
    t_drawnumber *x = (t_drawnumber *)z;
    t_slot *f = &x->value;
    t_atom at;
    if (!gpointer_check(&dn.gpointer, 0)) {post("drawnumber_motion: scalar disappeared"); return;}
    if (dn.symbol) {post("drawnumber_motion: symbol"); return;}
    dn.ycumulative -= dy;
    template_setfloat(dn.t, f->varsym, dn.wp, dn.ycumulative, 1);
    if (dn.scalar) gobj_changed(dn.scalar); else gobj_changed(dn.array);
}
#endif

static void drawnumber_key(void *z, t_floatarg fkey) {
    //t_drawnumber *x = (t_drawnumber *)z;
    int key = (int)fkey;
    if (!gpointer_check(&dn.gpointer, 0)) {
        post("drawnumber_motion: scalar disappeared");
        return;
    }
    if (key == 0) return;
    if (dn.symbol) {
        /* key entry for a symbol field... has to be rewritten in Tcl similarly to TextBox for edition of [drawsymbol] */
        // template_getsymbol(dn.t, f->varsym, dn.wp, 1)->name;
    } else {
        /* key entry for a numeric field... same here... [drawnumber] */
        //t_slot *f = &x->value;
        //float newf;
        //if (sscanf(sbuf, "%g", &newf) < 1) newf = 0;
        //template_setfloat(dn.t, f->varsym, dn.wp, newf, 1);
        //t_atom at;
        //if (dn.scalar) template_notifyforscalar(dn.t, dn.canvas, dn.scalar, gensym("change"), 1, &at);
        //if (dn.scalar) gobj_changed(dn.scalar,0); else gobj_changed(dn.array,0);
    }
}

#if 0
static int drawnumber_click(t_gobj *z, t_canvas *canvas, t_word *data, t_template *t, t_scalar *sc, t_array *ap, float basex, float basey, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    t_drawnumber *x = (t_drawnumber *)z;
    int x1, y1, x2, y2;
    drawnumber_getrect(z, canvas, data, t, basex, basey, &x1, &y1, &x2, &y2);
    if (xpix >= x1 && xpix <= x2 && ypix >= y1 && ypix <= y2
      && x->value.var && slot_getfloat(&x->vis, t, data, 0)) {
        if (doit) {
            dn.canvas = canvas;
            dn.wp = data;
            dn.t = t;
            dn.scalar = sc;
            dn.array = ap;
            dn.firstkey = 1;
            dn.ycumulative = slot_getfloat(&x->value, t, data, 0);
            dn.symbol = (x->flags & DRAW_SYMBOL)!=0;
            if (dn.scalar)
                gpointer_setcanvas(&dn.gpointer, dn.canvas, dn.scalar);
            else gpointer_setarray(&dn.gpointer, dn.array, dn.wp);
           /* canvas_grab(glist, z, drawnumber_motion, drawnumber_key, xpix, ypix); */
        }
        return 1;
    } else return 0;
}
#endif

static void drawnumber_free(t_drawnumber *x) {}

static void g_template_setup() {
    template_class = class_new2("template",0,template_free,sizeof(t_template),CLASS_PD,"");
    class_addmethod2(pd_canvasmaker._class, template_usetemplate, "struct", "*");
    gtemplate_class = class_new2("struct",gtemplate_new,gtemplate_free,sizeof(t_gtemplate),CLASS_NOINLET,"*");
    class_addcreator2("template",gtemplate_new_old,"*");

    curve_class = class_new2("drawpolygon",curve_new,0,sizeof(t_curve),0,"*");
    class_setdrawcommand(curve_class);
    class_addcreator2("drawcurve",    curve_new,"*");
    class_addcreator2("filledpolygon",curve_new,"*");
    class_addcreator2("filledcurve",  curve_new,"*");
    class_addfloat(curve_class, curve_float);
    plot_class = class_new2("plot",plot_new,0,sizeof(t_plot),0,"*");
    class_setdrawcommand(plot_class);
    class_addfloat(plot_class, plot_float);
    drawnumber_class = class_new2("drawnumber",drawnumber_new,drawnumber_free,sizeof(t_drawnumber),0,"*");
    class_setdrawcommand(drawnumber_class);
    class_addfloat(drawnumber_class, drawnumber_float);
    class_addcreator2("drawsymbol",drawnumber_new,"*");
}

/* ------------- gpointers - safe pointing --------------- */

/* call this to verify that a pointer is fresh, i.e., that it either points to real data or to the head of a list,
   and that in either case the object hasn't disappeared since this pointer was generated. Unless "headok" is set,
   the routine also fails for the head of a list. */
int gpointer_check(const t_gpointer *gp, int headok) {
    if (!gp->o) return 0;
    if (gp->o->_class == array_class) return 1;
    return headok || gp->scalar;
}

/* get the template for the object pointer to.  Assumes we've already checked freshness.  Returns 0 if head of list. */
static t_symbol *gpointer_gettemplatesym(const t_gpointer *gp) {
    if (gp->o->_class == array_class) return gp->array->templatesym;
    t_scalar *sc = gp->scalar;
    return sc ? sc->t : 0;
}

/* copy a pointer to another, assuming the second one hasn't yet been initialized.  New gpointers should be initialized
   either by this routine or by gpointer_init below. */
void gpointer_copy(const t_gpointer *gpfrom, t_gpointer *gpto) {
    *gpto = *gpfrom;
    if (gpto && gpto->o) gpto->o->refcount++; else bug("gpointer_copy");
}

void gpointer_unset(t_gpointer *gp) {
    if (gp->scalar) {
	if (!--gp->o->refcount) pd_free(gp->o);
	gp->o=0;
	gp->scalar=0;
    }
}

static void gpointer_setcanvas(t_gpointer *gp, t_canvas *o, t_scalar *x) {
    gpointer_unset(gp);
    gp->o=o; gp->scalar = x; gp->o->refcount++;
}
static void gpointer_setarray(t_gpointer *gp, t_array *o, t_word *w) {
    gpointer_unset(gp);
    gp->o=o; gp->w = w; gp->o->refcount++;
}

void gpointer_init(t_gpointer *gp) {
    gp->o = 0;
    gp->scalar = 0;
}

/* ---------------------- pointers ----------------------------- */

static t_class *ptrobj_class;

struct t_typedout {
    t_symbol *type;
    t_outlet *outlet;
};

struct t_ptrobj : t_object {
    t_gpointer gp;
    t_typedout *typedout;
    int ntypedout;
    t_outlet *otherout;
    t_outlet *bangout;
};

static void *ptrobj_new(t_symbol *classname, int argc, t_atom *argv) {
    t_ptrobj *x = (t_ptrobj *)pd_new(ptrobj_class);
    t_typedout *to;
    gpointer_init(&x->gp);
    x->typedout = to = (t_typedout *)getbytes(argc * sizeof (*to));
    x->ntypedout = argc;
    for (int n=argc; n--; to++) {
        to->outlet = outlet_new(x,&s_pointer);
        to->type = canvas_makebindsym(atom_getsymbol(argv++));
    }
    x->otherout = outlet_new(x,&s_pointer);
    x->bangout = outlet_new(x,&s_bang);
    pointerinlet_new(x,&x->gp);
    return x;
}

static void ptrobj_traverse(t_ptrobj *x, t_symbol *s) {
    t_canvas *canvas = (t_canvas *)pd_findbyclass(s, canvas_class);
    if (canvas) gpointer_setcanvas(&x->gp, canvas, 0);
    else error("list '%s' not found", s->name);
}

static void ptrobj_vnext(t_ptrobj *x, float f) {
    t_gpointer *gp = &x->gp;
    int wantselected = f!=0;
    if (!gp->o) {error("next: no current pointer"); return;}
    if (gp->o->_class == array_class) {error("next: lists only, not arrays"); return;}
    t_canvas *canvas = gp->canvas;
    if (wantselected) {error("next: next-selected unsupported in desiredata"); return;}
    /* if (wantselected && !canvas_isvisible(canvas)) {error("next: next-selected only works for a visible window"); return;} */
    t_gobj *gobj = gp->scalar;
    if (!gobj) gobj = canvas->boxes->first();
    else gobj = gobj->next();
    while (gobj && (gobj->_class != scalar_class || wantselected)) gobj = gobj->next();
    if (gobj) {
        t_typedout *to = x->typedout;
        t_scalar *sc = (t_scalar *)gobj;
        gp->scalar = sc;
        for (int n = x->ntypedout; n--; to++)
            if (to->type == sc->t) {outlet_pointer(to->outlet, &x->gp); return;}
        outlet_pointer(x->otherout, &x->gp);
    } else {
        gpointer_unset(gp);
        outlet_bang(x->bangout);
    }
}

static void ptrobj_next(t_ptrobj *x) {ptrobj_vnext(x, 0);}

static void ptrobj_sendwindow(t_ptrobj *x, t_symbol *s, int argc, t_atom *argv) {
    if (!gpointer_check(&x->gp, 1)) {error("bang: empty pointer"); return;}
    t_canvas *canvas = gpointer_getcanvas(&x->gp);
    if (argc && argv->a_type == A_SYMBOL) pd_typedmess(canvas_getcanvas(canvas), argv->a_symbol, argc-1, argv+1);
    else error("send-window: no message?");
}

static void ptrobj_bang(t_ptrobj *x) {
    t_typedout *to = x->typedout;
    if (!gpointer_check(&x->gp, 1)) {error("bang: empty pointer"); return;}
    t_symbol *templatesym = gpointer_gettemplatesym(&x->gp);
    for (int n=x->ntypedout; n--; to++)
        if (to->type == templatesym) {outlet_pointer(to->outlet, &x->gp); return;}
    outlet_pointer(x->otherout, &x->gp);
}

static void ptrobj_pointer(t_ptrobj *x, t_gpointer *gp) {
    gpointer_unset(&x->gp);
    gpointer_copy(gp, &x->gp);
    ptrobj_bang(x);
}

static void ptrobj_rewind(t_ptrobj *x) {
    if (!gpointer_check(&x->gp, 1)) {error("rewind: empty pointer"); return;}
    if (x->gp.o->_class == array_class) {error("rewind: sorry, unavailable for arrays"); return;}
    gpointer_setcanvas(&x->gp, x->gp.canvas, 0);
    ptrobj_bang(x);
}

static void ptrobj_free(t_ptrobj *x) {
    free(x->typedout);
    gpointer_unset(&x->gp);
}

/* ---------------------- get ----------------------------- */

static t_class *get_class;
struct t_getvariable {
    t_symbol *sym;
    t_outlet *outlet;
};
struct t_get : t_object {
    t_symbol *templatesym;
    int nout;
    t_getvariable *variables;
};
static void *get_new(t_symbol *why, int argc, t_atom *argv) {
    t_get *x = (t_get *)pd_new(get_class);
    x->templatesym = canvas_makebindsym(atom_getsymbolarg(0, argc, argv));
    if (argc) argc--, argv++;
    t_getvariable *sp = x->variables = (t_getvariable *)getbytes(argc * sizeof (*x->variables));
    x->nout = argc;
    for (int i=0; i < argc; i++, sp++) {
        sp->sym = atom_getsymbolarg(i, argc, argv);
        sp->outlet = outlet_new(x,0);
        /* LATER connect with the template and set the outlet's type
        correctly.  We can't yet guarantee that the template is there
        before we hit this routine. */
    }
    return x;
}

static void get_pointer(t_get *x, t_gpointer *gp) {
    int nitems = x->nout;
    t_template *t = template_findbyname(x->templatesym);
    t_getvariable *vp;
    TEMPLATE_CHECK(x->templatesym,)
    if (!gpointer_check(gp, 0)) {error("stale or empty pointer"); return;}
    t_word *vec = gpointer_word(gp);
    vp = x->variables + nitems-1;
    for (int i=nitems-1; i>=0; i--, vp--) {
        int onset, type;
        t_symbol *arraytype;
        if (template_find_field(t, vp->sym, &onset, &type, &arraytype)) {
            if       (type == DT_FLOAT)  outlet_float(vp->outlet,   *(t_float *)(((char *)vec) + onset));
            else if (type == DT_SYMBOL) outlet_symbol(vp->outlet, *(t_symbol **)(((char *)vec) + onset));
            else error("%s.%s is not a number or symbol", t->sym->name, vp->sym->name);
        } else error("%s.%s: no such field", t->sym->name, vp->sym->name);
    }
}

static void get_free(t_get *x) {free(x->variables);}

/* ---------------------- set ----------------------------- */

static t_class *set_class;

struct t_setvariable {
    t_symbol *sym;
    union word w;
};

struct t_set : t_object {
    t_gpointer gp;
    t_symbol *templatesym;
    int nin;
    int issymbol;
    t_setvariable *variables;
};

static void *set_new(t_symbol *why, int argc, t_atom *argv) {
    t_set *x = (t_set *)pd_new(set_class);
    if (argc && (argv[0].a_type == A_SYMBOL) && !strcmp(argv[0].a_symbol->name,"-symbol")) {
        x->issymbol = 1;
        argc--;
        argv++;
    }
    else x->issymbol = 0;
    x->templatesym = canvas_makebindsym(atom_getsymbolarg(0, argc, argv));
    if (argc) argc--, argv++;
    t_setvariable *sp = x->variables = (t_setvariable *)getbytes(argc * sizeof (*x->variables));
    x->nin = argc;
    if (argc) {
        for (int i=0; i<argc; i++, sp++) {
            sp->sym = atom_getsymbolarg(i, argc, argv);
            if (x->issymbol) sp->w.w_symbol = &s_;
            else sp->w.w_float = 0;
            if (i) {
                if (x->issymbol) symbolinlet_new(x,&sp->w.w_symbol);
                else              floatinlet_new(x, &sp->w.w_float);
            }
        }
    }
    pointerinlet_new(x,&x->gp);
    gpointer_init(&x->gp);
    return x;
}

static void set_bang(t_set *x) {
    int nitems = x->nin;
    t_template *t = template_findbyname(x->templatesym);
    t_gpointer *gp = &x->gp;
    TEMPLATE_CHECK(x->templatesym,)
    if (!gpointer_check(gp, 0)) {error("empty pointer"); return;}
    if (gpointer_gettemplatesym(gp) != x->templatesym) {
        error("%s: got wrong template (%s)",x->templatesym->name,gpointer_gettemplatesym(gp)->name);
        return;
    }
    if (!nitems) return;
    t_word *vec = gpointer_word(gp);
    t_setvariable *vp=x->variables;
    if (x->issymbol) for (int i=0; i<nitems; i++,vp++) template_setsymbol(t, vp->sym, vec, vp->w.w_symbol, 1);
    else              for (int i=0; i<nitems; i++,vp++) template_setfloat(t, vp->sym, vec, vp->w.w_float, 1);
    scalar_redraw(gp->scalar, gpointer_getcanvas(gp)); /* but ought to use owner_array->gp.scalar */
}

static void set_float(t_set *x, t_float f) {
    if (x->nin && !x->issymbol) {x->variables[0].w.w_float = f; set_bang(x);}
    else error("type mismatch or no field specified");
}
static void set_symbol(t_set *x, t_symbol *s) {
    if (x->nin && x->issymbol)  {x->variables[0].w.w_symbol = s; set_bang(x);}
    else error("type mismatch or no field specified");
}

static void set_free(t_set *x) {
    free(x->variables);
    gpointer_unset(&x->gp);
}

/* ---------------------- elem ----------------------------- */

static t_class *elem_class;

struct t_elem : t_object {
    t_symbol *templatesym;
    t_symbol *fieldsym;
    t_gpointer gp;
    t_gpointer gparent;
};

static void *elem_new(t_symbol *templatesym, t_symbol *fieldsym) {
    t_elem *x = (t_elem *)pd_new(elem_class);
    x->templatesym = canvas_makebindsym(templatesym);
    x->fieldsym = fieldsym;
    gpointer_init(&x->gp);
    gpointer_init(&x->gparent);
    pointerinlet_new(x,&x->gparent);
    outlet_new(x,&s_pointer);
    return x;
}

static void elem_float(t_elem *x, t_float f) {
    int indx = (int)f, nitems, onset;
    t_symbol *fieldsym = x->fieldsym, *elemtemplatesym;
    t_template *t = template_findbyname(x->templatesym);
    t_template *elemtemplate;
    t_gpointer *gparent = &x->gparent;
    t_array *array;
    int elemsize, type;
    if (!gpointer_check(gparent, 0)) {error("empty pointer"); return;}
    if (gpointer_gettemplatesym(gparent) != x->templatesym) {
        error("%s: got wrong template (%s)", x->templatesym->name, gpointer_gettemplatesym(gparent)->name);
        return;
    }
    t_word *w = gpointer_word(gparent);
    TEMPLATE_CHECK(x->templatesym,)
    if (!template_find_field(t, fieldsym, &onset, &type, &elemtemplatesym)) {
        error("couldn't find array field %s", fieldsym->name);
        return;
    }
    if (type != DT_ARRAY) {error("element: field %s not of type array", fieldsym->name); return;}
    if (!(elemtemplate = template_findbyname(elemtemplatesym))) {
        error("couldn't find field template %s", elemtemplatesym->name);
        return;
    }
    elemsize = elemtemplate->n * sizeof(t_word);
    array = *(t_array **)(((char *)w) + onset);
    nitems = array->n;
    if (indx < 0) indx = 0;
    if (indx >= nitems) indx = nitems-1;
    gpointer_setarray(&x->gp, array, (t_word *)&array->vec[indx*elemsize]);
    outlet_pointer(x->outlet, &x->gp);
}

static void elem_free(t_elem *x, t_gpointer *gp) {
    gpointer_unset(&x->gp);
    gpointer_unset(&x->gparent);
}

/* ---------------------- getsize ----------------------------- */

static t_class *getsize_class;

struct t_getsize : t_object {
    t_symbol *templatesym;
    t_symbol *fieldsym;
};

static void *getsize_new(t_symbol *templatesym, t_symbol *fieldsym) {
    t_getsize *x = (t_getsize *)pd_new(getsize_class);
    x->templatesym = canvas_makebindsym(templatesym);
    x->fieldsym = fieldsym;
    outlet_new(x,&s_float);
    return x;
}

static void getsize_pointer(t_getsize *x, t_gpointer *gp) {
    int onset, type;
    t_symbol *fieldsym = x->fieldsym, *elemtemplatesym;
    t_template *t = template_findbyname(x->templatesym);
    TEMPLATE_CHECK(x->templatesym,)
    if (!template_find_field(t, fieldsym, &onset, &type, &elemtemplatesym)) {
        error("couldn't find array field %s", fieldsym->name);
        return;
    }
    if (type != DT_ARRAY) {error("field %s not of type array", fieldsym->name); return;}
    if (!gpointer_check(gp, 0)) {error("stale or empty pointer"); return;}
    if (gpointer_gettemplatesym(gp) != x->templatesym) {
        error("%s: got wrong template (%s)", x->templatesym->name, gpointer_gettemplatesym(gp)->name);
        return;
    }
    t_word *w = gpointer_word(gp);
    t_array *array = *(t_array **)(((char *)w) + onset);
    outlet_float(x->outlet, (float)(array->n));
}

/* ---------------------- setsize ----------------------------- */

static t_class *setsize_class;

struct t_setsize : t_object {
    t_symbol *templatesym;
    t_symbol *fieldsym;
    t_gpointer gp;
};

static void *setsize_new(t_symbol *templatesym, t_symbol *fieldsym, t_floatarg newsize) {
    t_setsize *x = (t_setsize *)pd_new(setsize_class);
    x->templatesym = canvas_makebindsym(templatesym);
    x->fieldsym = fieldsym;
    gpointer_init(&x->gp);
    pointerinlet_new(x,&x->gp);
    return x;
}

static void setsize_float(t_setsize *x, t_float f) {
    int onset, type;
    t_template *t = template_findbyname(x->templatesym);
    int newsize = (int)f;
    t_gpointer *gp = &x->gp;
    if (!gpointer_check(&x->gp, 0)) {error("empty pointer"); return;}
    if (gpointer_gettemplatesym(&x->gp) != x->templatesym) {
        error("%s: got wrong template (%s)", x->templatesym->name, gpointer_gettemplatesym(&x->gp)->name);
        return;
    }
    t_word *w = gpointer_word(gp);
    TEMPLATE_CHECK(x->templatesym,)
    t_symbol *elemtemplatesym;
    if (!template_find_field(t, x->fieldsym, &onset, &type, &elemtemplatesym)) {
        error("couldn't find array field %s", x->fieldsym->name);
        return;
    }
    if (type != DT_ARRAY) {error("field %s not of type array", x->fieldsym->name); return;}
    t_template *elemtemplate = template_findbyname(elemtemplatesym);
    if (!elemtemplate) {
        error("couldn't find field template %s", elemtemplatesym->name);
        return;
    }
    int elemsize = elemtemplate->n * sizeof(t_word);
    t_array *array = *(t_array **)(((char *)w) + onset);
    if (elemsize != array->elemsize) bug("setsize_gpointer");
    int nitems = array->n;
    if (newsize < 1) newsize = 1;
    if (newsize == nitems) return;

    /* here there was something to erase the array before resizing it */

    /* now do the resizing and, if growing, initialize new scalars */
    array->vec = (char *)resizealignedbytes(array->vec, elemsize * nitems, elemsize * newsize);
    array->n = newsize;
    if (newsize > nitems) {
        char *newelem = &array->vec[nitems*elemsize];
        int nnew = newsize - nitems;
        while (nnew--) {
            word_init((t_word *)newelem, elemtemplate, gp);
            newelem += elemsize;
        }
    }
    gobj_changed(gpointer_getscalar(gp),0);
}

static void setsize_free(t_setsize *x) {gpointer_unset(&x->gp);}

/* ---------------------- append ----------------------------- */

static t_class *append_class;

struct t_appendvariable {
    t_symbol *sym;
    t_float f;
};

struct t_append : t_object {
    t_gpointer gp;
    t_symbol *templatesym;
    int nin;
    t_appendvariable *variables;
};

static void *append_new(t_symbol *why, int argc, t_atom *argv) {
    t_append *x = (t_append *)pd_new(append_class);
    x->templatesym = canvas_makebindsym(atom_getsymbolarg(0, argc, argv));
    if (argc) argc--, argv++;
    x->variables = (t_appendvariable *)getbytes(argc * sizeof (*x->variables));
    x->nin = argc;
    if (argc) {
        t_appendvariable *sp = x->variables;
        for (int i=0; i<argc; i++, sp++) {
            sp->sym = atom_getsymbolarg(i, argc, argv);
            sp->f = 0;
            if (i) floatinlet_new(x,&sp->f);
        }
    }
    pointerinlet_new(x,&x->gp);
    outlet_new(x,&s_pointer);
    gpointer_init(&x->gp);
    return x;
}

static void append_float(t_append *x, t_float f) {
    int nitems = x->nin;
    t_template *t = template_findbyname(x->templatesym);
    t_gpointer *gp = &x->gp;
    TEMPLATE_CHECK(x->templatesym,)
    if (!gp->o) {error("no current pointer"); return;}
    if (gp->o->_class == array_class) {error("lists only, not arrays"); return;}
    t_canvas *canvas = gp->canvas;
    if (!nitems) return;
    x->variables[0].f = f;
    t_scalar *sc = scalar_new(canvas,x->templatesym);
    if (!sc) {error("%s: couldn't create scalar", x->templatesym->name); return;}
    canvas->boxes->add(sc);
    gobj_changed(sc,0);
    gp->scalar = sc;
    t_word *vec = sc->v;
    t_appendvariable *vp=x->variables;
    for (int i=0; i<nitems; i++,vp++) template_setfloat(t, vp->sym, vec, vp->f, 1);
    scalar_redraw(sc, canvas);
    outlet_pointer(x->outlet, gp);
}

static void append_free(t_append *x) {
    free(x->variables);
    gpointer_unset(&x->gp);
}

/* ---------------------- sublist ----------------------------- */

static t_class *sublist_class;

struct t_sublist : t_object {
    t_symbol *templatesym;
    t_symbol *fieldsym;
    t_gpointer gp;
};

static void *sublist_new(t_symbol *templatesym, t_symbol *fieldsym) {
    t_sublist *x = (t_sublist *)pd_new(sublist_class);
    x->templatesym = canvas_makebindsym(templatesym);
    x->fieldsym = fieldsym;
    gpointer_init(&x->gp);
    outlet_new(x,&s_pointer);
    return x;
}

static void sublist_pointer(t_sublist *x, t_gpointer *gp) {
    t_symbol *dummy;
    t_template *t = template_findbyname(x->templatesym);
    int onset, type;
    TEMPLATE_CHECK(x->templatesym,)
    if (!gpointer_check(gp, 0)) {error("stale or empty pointer"); return;}
    if (!template_find_field(t, x->fieldsym, &onset, &type, &dummy)) {
        error("couldn't find field %s", x->fieldsym->name);
        return;
    }
    if (type != DT_CANVAS) {error("field %s not of type list", x->fieldsym->name); return;}
    t_word *w = gpointer_word(gp);
    gpointer_setcanvas(&x->gp, *(t_canvas **)(((char *)w) + onset), 0);
    outlet_pointer(x->outlet, &x->gp);
}

static void sublist_free(t_sublist *x, t_gpointer *gp) {gpointer_unset(&x->gp);}

static void g_traversal_setup() {
    t_class *c = ptrobj_class = class_new2("pointer",ptrobj_new,ptrobj_free,sizeof(t_ptrobj),0,"*");
    class_addmethod2(c, ptrobj_traverse,"traverse", "s");
    class_addmethod2(c, ptrobj_next,"next","");
    class_addmethod2(c, ptrobj_vnext,"vnext","F");
    class_addmethod2(c, ptrobj_sendwindow,"send-window","*");
    class_addmethod2(c, ptrobj_rewind, "rewind","");
    class_addpointer(c, ptrobj_pointer);
    class_addbang(c, ptrobj_bang);
    get_class = class_new2("get",get_new,get_free,sizeof(t_get),0,"*");
    class_addpointer(get_class, get_pointer);
    set_class = class_new2("set",set_new,set_free,sizeof(t_set),0,"*");
    class_addfloat(set_class, set_float);
    class_addsymbol(set_class, set_symbol);
    class_addbang(set_class, set_bang);
    elem_class = class_new2("element",elem_new,elem_free,sizeof(t_elem),0,"SS");
    class_addfloat(elem_class, elem_float);
    getsize_class = class_new2("getsize",getsize_new,0,sizeof(t_getsize),0,"SS");
    class_addpointer(getsize_class, getsize_pointer);
    setsize_class = class_new2("setsize",setsize_new,setsize_free,sizeof(t_setsize),0,"SSFF");
    class_addfloat(setsize_class, setsize_float);
    append_class = class_new2("append",append_new,append_free,sizeof(t_append),0,"*");
    class_addfloat(append_class, append_float);
    sublist_class = class_new2("sublist",sublist_new,sublist_free,sizeof(t_sublist),0,"SS");
    class_addpointer(sublist_class, sublist_pointer);
}

/*EXTERN*/ void canvas_savecontainerto(t_canvas *x, t_binbuf *b);

struct t_iemgui : t_object {
    t_canvas *canvas;
    int h,w;
    int ldx,ldy;
    int isa; /* bit 0: loadinit; bit 20: scale */
    int font_style, fontsize;
    int fcol,bcol,lcol; /* foreground, background, label colors */
    t_symbol *snd,*rcv,*lab; /* send, receive, label symbols */
};

struct t_bng : t_iemgui {
    int      count;
    int      ftbreak; /* flash time break (ms) */
    int      fthold;  /* flash time hold  (ms) */
};

struct t_slider : t_iemgui {
    t_float   val;
    t_float   min,max;
    int      steady;
    int      is_log;
    int      orient;
};

struct t_radio : t_iemgui {
    int      on, on_old;
    int      change;
    int      number;
    t_atom   at[2];
    int      orient;
    int      oldstyle;
};

struct t_toggle : t_iemgui {
    float    on;
    float    nonzero;
};

struct t_cnv : t_iemgui {
    t_atom   at[3];
    int      vis_w, vis_h;
};

struct t_vu : t_iemgui {
    int      led_size;
    int      peak,rms;
    float    fp,fr;
    int      scale;
};

struct t_nbx : t_iemgui {
    double   val;
    double   min,max;
    double   k;
    char     buf[32];
    int      log_height;
    int      change;
    int      is_log;
};

struct t_foo { int argc; t_atom *argv; t_binbuf *b; };
static int pd_pickle(t_foo *foo, const char *fmt, ...);
static int pd_savehead(t_binbuf *b, t_iemgui *x, const char *name);

static t_class *radio_class, *slider_class;
static t_symbol *sym_hdl, *sym_hradio, *sym_vdl, *sym_vradio, *sym_vsl, *sym_vslider;

t_class *dummy_class;
/*static*/ t_class *text_class;
static t_class *mresp_class;
static t_class *message_class;
static t_class *gatom_class;

void canvas_text(t_canvas *gl, t_symbol *s, int argc, t_atom *argv) {
    t_text *x = (t_text *)pd_new(text_class);
    x->binbuf = binbuf_new();
    x->x = atom_getintarg(0, argc, argv);
    x->y = atom_getintarg(1, argc, argv);
    if (argc > 2) binbuf_restore(x->binbuf, argc-2, argv+2);
    canvas_add(gl,x);
    pd_set_newest(x);
}

void canvas_getargs(int *argcp, t_atom **argvp) {
    t_canvasenvironment *e = canvas_getenv(canvas_getcurrent());
    *argcp = e->argc;
    *argvp = e->argv;
}

static void canvas_objtext(t_canvas *gl, int xpix, int ypix, t_binbuf *b, int index=-1) {
    t_text *x=0;
    int argc, n;
    t_atom *argv;
    char *s;
    newest = 0;
    binbuf_gettext(b,&s,&n);
    pd_pushsym(gl);
    canvas_getargs(&argc, &argv);
    binbuf_eval(b, &pd_objectmaker, argc, argv);
    if (binbuf_getnatom(b)) {
	if (!newest) {
	    char *s = binbuf_gettext2(b);
    	    error("couldn't create %s",s);
	    free(s);
	} else if (!(x = pd_checkobject(newest))) {
	    char *s = binbuf_gettext2(b);
    	    error("didn't return a patchable object: %s",s);
	    free(s);
	}
    }
    /* make a "broken object", that is, one that should appear with a dashed contour. */
    if (!x) {
	x = (t_text *)pd_new(dummy_class);
        pd_set_newest(x);
    }
    x->binbuf = b;
    x->x = xpix;
    x->y = ypix;
    canvas_add(gl,x,index);
    if (x->_class== vinlet_class)  canvas_resortinlets(canvas_getcanvas(gl));
    if (x->_class==voutlet_class) canvas_resortoutlets(canvas_getcanvas(gl));
    pd_popsym(gl);
}

void canvas_obj(t_canvas *gl, t_symbol *s, int argc, t_atom *argv) {
    t_binbuf *b = binbuf_new();
    if (argc >= 2) {
    	binbuf_restore(b, argc-2, argv+2);
    	canvas_objtext(gl, atom_getintarg(0, argc, argv), atom_getintarg(1, argc, argv), b);
    } else canvas_objtext(gl,0,0,b);
}

void canvas_objfor(t_canvas *gl, t_text *x, int argc, t_atom *argv) {
    x->binbuf = binbuf_new();
    x->x = atom_getintarg(0, argc, argv);
    x->y = atom_getintarg(1, argc, argv);
    if (argc > 2) binbuf_restore(x->binbuf, argc-2, argv+2);
    canvas_add(gl,x);
}

struct t_mresp : t_pd {
    t_outlet *outlet;
};
struct t_message : t_text {
    t_mresp mresp;
    t_canvas *canvas;
};

static void mresp_bang(t_mresp *x)                {outlet_bang(x->outlet);}
static void mresp_float(t_mresp *x, t_float f)    {outlet_float(x->outlet, f);}
static void mresp_symbol(t_mresp *x, t_symbol *s) {outlet_symbol(x->outlet, s);}
static void mresp_list(t_mresp *x, t_symbol *s, int argc, t_atom *argv)
	{outlet_list(x->outlet, s, argc, argv);}
static void mresp_anything(t_mresp *x, t_symbol *s, int argc, t_atom *argv)
	{outlet_anything(x->outlet, s, argc, argv);}

static void message_bang(t_message *x)
{binbuf_eval(x->binbuf,&x->mresp, 0, 0);}
static void message_float(t_message *x, t_float f)
{t_atom at; SETFLOAT(&at, f);  binbuf_eval(x->binbuf, &x->mresp, 1, &at);}
static void message_symbol(t_message *x, t_symbol *s)
{t_atom at; SETSYMBOL(&at, s); binbuf_eval(x->binbuf, &x->mresp, 1, &at);}
static void message_list(t_message *x, t_symbol *s, int argc, t_atom *argv)
{binbuf_eval(x->binbuf, &x->mresp, argc, argv);}
static void message_add2(t_message *x, t_symbol *s, int argc, t_atom *argv)
{binbuf_add(x->binbuf, argc, argv); gobj_changed(x,"binbuf");}
static void message_set(t_message *x, t_symbol *s, int argc, t_atom *argv)
{binbuf_clear(x->binbuf); message_add2(x,s,argc,argv);}
static void message_add(t_message *x, t_symbol *s, int argc, t_atom *argv)
{binbuf_add(x->binbuf, argc, argv); binbuf_addsemi(x->binbuf);        gobj_changed(x,"binbuf");}
static void message_addcomma(t_message *x)
{t_atom a; SETCOMMA(&a);           binbuf_add(x->binbuf, 1, &a);      gobj_changed(x,"binbuf");}
static void message_adddollar(t_message *x, t_floatarg f)
{t_atom a; SETDOLLAR(&a, f<0?0:(int)f); binbuf_add(x->binbuf, 1, &a); gobj_changed(x,"binbuf");}
//static void message_adddollsym(t_message *x, t_symbol *s)
//{t_atom a; SETDOLLSYM(&a, s);      binbuf_add(x->binbuf, 1, &a);      gobj_changed(x,"binbuf");}

static void message_adddollsym(t_message *x, t_symbol *s) {
    t_atom a;
    SETDOLLSYM(&a, symprintf("$%s",s->name));
    binbuf_add(x->binbuf, 1, &a);
    gobj_changed(x,"binbuf");
}

static void message_addsemi(t_message *x) {message_add(x,0,0,0);}

void canvas_msg(t_canvas *gl, t_symbol *s, int argc, t_atom *argv) {
    t_message *x = (t_message *)pd_new(message_class);
    x->mresp._class = mresp_class;
    x->mresp.outlet = outlet_new(x,&s_float);
    x->binbuf = binbuf_new();
    x->canvas = gl;
    pd_set_newest(x);
    if (argc > 1) {
    	x->x = atom_getintarg(0, argc, argv);
    	x->y = atom_getintarg(1, argc, argv);
    	if (argc > 2) binbuf_restore(x->binbuf, argc-2, argv+2);
    } else {
    	x->x = 0;
    	x->y = 0;
    }
    canvas_add(gl,x);
}

struct t_gatom : t_text {
    t_atom atom;      /* this holds the value and the type */
    t_canvas *canvas;   /* owning canvas */
    t_float max,min;
    t_symbol *label;  /* symbol to show as label next to box */
    t_symbol *rcv;
    t_symbol *snd;
    char wherelabel;  /* 0-3 for left, right, up, down */
    t_symbol *expanded_to; /* snd after $0, $1, ...  expansion */
    short width;
};

/* prepend "-" as necessary to avoid empty strings, so we can use them in Pd messages.
   A more complete solution would be to introduce some quoting mechanism;
   but then we'd be much more complicated. */
static t_symbol *gatom_escapit(t_symbol *s) {
    if (!s || !*s->name) return gensym("-");
    if (*s->name == '-') {
    	char shmo[1000];
	snprintf(shmo,1000,"-%s",s->name);
	shmo[999] = 0;
    	return gensym(shmo);
    }
    else return s;
}

/* undo previous operation: strip leading "-" if found. */
static t_symbol *gatom_unescapit(t_symbol *s) {
    if (*s->name == '-') return gensym(s->name+1);
    return s;
}

static void gatom_set(t_gatom *x, t_symbol *s, int argc, t_atom *argv) {
    t_atom oldatom = x->atom;
    if (!argc) return;
    if (x->atom.a_type == A_FLOAT) {
	SETFLOAT( &x->atom,atom_getfloat(argv));  if (x->atom.a_float !=oldatom.a_float ) gobj_changed(x,"atom");
    } else if (x->atom.a_type == A_SYMBOL) {
	SETSYMBOL(&x->atom,atom_getsymbol(argv)); if (x->atom.a_symbol!=oldatom.a_symbol) gobj_changed(x,"atom");
    }
}

static void gatom_bang(t_gatom *x) {
    t_symbol *s = x->expanded_to;
    t_outlet *o = x->outlet;
    if (x->atom.a_type == A_FLOAT) {
    	if (o) outlet_float(o, x->atom.a_float);
	if (*s->name && s->thing) {
	    if (x->snd == x->rcv) goto err;
	    pd_float(s->thing, x->atom.a_float);
    	}
    } else if (x->atom.a_type == A_SYMBOL) {
    	if (o) outlet_symbol(o, x->atom.a_symbol);
	if (*s->name && s->thing) {
	    if (x->snd == x->rcv) goto err;
	    pd_symbol(s->thing, x->atom.a_symbol);
    	}
    }
    return;
err:
    error("%s: atom with same send/receive name (infinite loop)", x->snd->name);
}

static void gatom_float(t_gatom *x, t_float f)
{t_atom at; SETFLOAT(&at, f); gatom_set(x, 0, 1, &at); gatom_bang(x);}
static void gatom_symbol(t_gatom *x, t_symbol *s)
{t_atom at; SETSYMBOL(&at, s); gatom_set(x, 0, 1, &at); gatom_bang(x);}

static void gatom_reload(t_gatom *x, t_symbol *sel, int argc, t_atom *argv) {
    int width; t_float wherelabel;
    t_symbol *rcv,*snd;
    if (!pd_scanargs(argc,argv,"ifffaaa",&width,&x->min,&x->max,&wherelabel,&x->label,&rcv,&snd)) return;
    gobj_changed(x,0);
    SET(label,gatom_unescapit(x->label));
    if (x->min>=x->max) {SET(min,0); SET(max,0);}
    CLAMP(width,1,80);
    SET(width,width);
    SET(wherelabel,(int)wherelabel&3);
    if (x->rcv) pd_unbind(x, canvas_realizedollar(x->canvas, x->rcv));
    SET(rcv,gatom_unescapit(rcv));
    if (x->rcv) pd_bind(  x, canvas_realizedollar(x->canvas, x->rcv));
    SET(snd,gatom_unescapit(snd));
    SET(expanded_to,canvas_realizedollar(x->canvas, x->snd));
}

/* We need a list method because, since there's both an "inlet" and a
   "nofirstin" flag, the standard list behavior gets confused. */
static void gatom_list(t_gatom *x, t_symbol *s, int argc, t_atom *argv) {
    if (!argc) gatom_bang(x);
    else if (argv->a_type == A_FLOAT)  gatom_float(x, argv->a_w.w_float);
    else if (argv->a_type == A_SYMBOL) gatom_symbol(x, argv->a_w.w_symbol);
    else error("gatom_list: need float or symbol");
}

void canvas_atom(t_canvas *gl, t_atomtype type, int argc, t_atom *argv) {
    t_gatom *x = (t_gatom *)pd_new(gatom_class);
    if (type == A_FLOAT) {SET(width, 5); SETFLOAT(&x->atom, 0);}
    else                 {SET(width,10); SETSYMBOL(&x->atom, &s_symbol);}
    x->canvas = gl;
    SET(min,0);
    SET(max,0);
    SET(wherelabel,0);
    SET(label,0);
    SET(rcv,0);
    SET(snd,0);
    x->expanded_to = &s_; //???
    x->binbuf = binbuf_new();
    binbuf_add(x->binbuf, 1, &x->atom);
    SET(x,atom_getintarg(0, argc, argv));
    SET(y,atom_getintarg(1, argc, argv));
    inlet_new(x,x,0,0);
    outlet_new(x, type == A_FLOAT ? &s_float: &s_symbol);
    if (argc>2) gatom_reload(x,&s_,argc-2,argv+2);
    canvas_add(gl,x);
    pd_set_newest(x);
}

void canvas_floatatom( t_canvas *gl, t_symbol *s, int argc, t_atom *argv) {canvas_atom(gl, A_FLOAT,  argc, argv);}
void canvas_symbolatom(t_canvas *gl, t_symbol *s, int argc, t_atom *argv) {canvas_atom(gl, A_SYMBOL, argc, argv);}
static void gatom_free(t_gatom *x) {if (x->rcv) pd_unbind(x, canvas_realizedollar(x->canvas, x->rcv));}

extern "C" void text_save(t_gobj *z, t_binbuf *b) {
    t_text *x = (t_text *)z;
    t_canvas *c = (t_canvas *)z; /* in case it is */
    if (x->_class == message_class) {
    	binbuf_addv(b,"ttii","#X","msg", (t_int)x->x, (t_int)x->y);
        binbuf_addbinbuf(b, x->binbuf);
    } else if (x->_class == gatom_class) {
	t_gatom *g = (t_gatom *)x;
	t_symbol *sel = g->atom.a_type==A_SYMBOL? gensym("symbolatom") : gensym("floatatom");
	binbuf_addv(b,"tsii","#X", sel, (t_int)x->x, (t_int)x->y);
	binbuf_addv(b,"iffi", (t_int)g->width, g->min, g->max, (t_int)g->wherelabel);
	binbuf_addv(b,"sss", gatom_escapit(g->label), gatom_escapit(g->rcv), gatom_escapit(g->snd));
    } else if (x->_class == text_class) {
    	binbuf_addv(b, "ttii","#X","text", (t_int)x->x, (t_int)x->y);
        binbuf_addbinbuf(b, x->binbuf);
    } else {
    	if (zgetfn(x,gensym("saveto")) &&
	 !(x->_class==canvas_class && (canvas_isabstraction(c) || canvas_istable(c)))) {
    	    mess1(x,gensym("saveto"),b);
    	    binbuf_addv(b,"ttii","#X","restore", (t_int)x->x, (t_int)x->y);
    	} else {
    	    binbuf_addv(b,"ttii","#X","obj",     (t_int)x->x, (t_int)x->y);
        }
	if (x->binbuf) {
	        binbuf_addbinbuf(b, x->binbuf);
	} else {
		/*bug("binbuf missing at #X restore !!!");*/
	}
    }
    binbuf_addv(b, ";");
}

static t_binbuf *canvas_cut_wires(t_canvas *x, t_gobj *o) {
    t_binbuf *buf = binbuf_new();
    canvas_wires_each(oc,t,x) {
        if ((o==t.from) != (o==t.to))
            binbuf_addv(buf,"ttiiii;","#X","connect", t.from->dix->index, t.outlet, t.to->dix->index, t.inlet);
    }
    return buf;
}
static void canvas_paste_wires(t_canvas *x, t_binbuf *buf) {
    pd_bind(x,gensym("#X"));
    binbuf_eval(buf,0,0,0);
    pd_unbind(x,gensym("#X"));
}

static void text_setto(t_text *x, t_canvas *canvas, char *buf, int bufsize) {
	if (x->_class == message_class || x->_class == gatom_class || x->_class == text_class) {
		binbuf_text(x->binbuf, buf, bufsize);
		gobj_changed(x,"binbuf");
		pd_set_newest(x);
		return;
	}
	t_binbuf *b = binbuf_new();
	binbuf_text(b, buf, bufsize);
	int natom1 = binbuf_getnatom(x->binbuf); t_atom *vec1 = binbuf_getvec(x->binbuf);
	int natom2 = binbuf_getnatom(b);         t_atom *vec2 = binbuf_getvec(b);
	/* special case: if  pd args change just pass the message on. */
	if (natom1 >= 1 && natom2 >= 1 &&
	  vec1[0].a_type == A_SYMBOL && vec1[0].a_symbol == s_pd &&
	  vec2[0].a_type == A_SYMBOL && vec2[0].a_symbol == s_pd) {
	    typedmess(x,gensym("rename"),natom2-1,vec2+1);
	    binbuf_free(x->binbuf);
	    x->binbuf = b;
	    pd_set_newest(x); /* fake object creation, for simplicity of client */
	} else {
	    int xwas=x->x, ywas=x->y;
	    int backupi = x->dix->index;
	    t_binbuf *buf = canvas_cut_wires(canvas_getcanvas(canvas),x);
	    canvas_delete(canvas,x);
	    canvas_objtext(canvas,xwas,ywas,b,backupi);
	    t_pd *backup = newest;
	    post("backupi=%d newest->index=%d",backupi,((t_object *)newest)->dix->index);
	    if (newest && pd_class(newest) == canvas_class) canvas_loadbang((t_canvas *)newest);
	    canvas_paste_wires(canvas_getcanvas(canvas), buf);
	    newest = backup;
	}
}

t_object *symbol2opointer(t_symbol *s) {
	t_text *o;
	if (sscanf(s->name,"x%lx",(long*)&o)<1) {error("expected object-id"); return 0;}
	if (!object_table->exists(o)) {error("%s target is not a currently valid pointer",s->name); return 0;}
	if ((object_table->get(o)&1)==0) {
		error("%s target is zombie? object_table says '%ld'",s->name,object_table->get(o));
		return 0;
	}
	if (!o->_class->patchable) {error("%s target not a patchable object"); return 0;}
	return o;
}

t_object *atom2opointer(t_atom *a) {return symbol2opointer(atom_getsymbol(a));}

static void canvas_text_setto(t_canvas *x, t_symbol *s, int argc, t_atom *argv) {
	t_text *o = atom2opointer(&argv[0]); if (!o) return;
	char str[4096];
	for (int i=0; i<argc-1; i++) str[i] = atom_getint(argv+i+1);
	str[argc-1]=0;
	text_setto(o,x,str,argc-1);
}

static void canvas_object_moveto(t_canvas *x, t_symbol *name, t_floatarg px, t_floatarg py) {
	t_text *o = symbol2opointer(name); if (!o) return;
	o->x=(int)px; gobj_changed(o,"x");
	o->y=(int)py; gobj_changed(o,"y");
}
static void canvas_object_delete(t_canvas *x, t_symbol *name) {
	t_text *o = symbol2opointer(name); if (!o) return;
	fprintf(stderr,"canvas_object_delete %p\n",o);
	canvas_delete(x,o);
}

static void canvas_object_get_tips(t_canvas *x, t_symbol *name) {
	t_text *o = symbol2opointer(name); if (!o) return;
	char foo[666];
	if (o->_class->firstin) strcpy(foo,o->_class->firsttip->name); else strcpy(foo,"");
	int n = obj_ninlets(x);
	//char *foop = foo;
	for (int i=!!o->_class->firstin; i<n; i++) {
		strcat(foo," ");
		strcat(foo,inlet_tip(o->inlet,i));
	}
	sys_mgui(o,"tips=","S",foo);
}

extern "C" void open_via_helppath(const char *name, const char *dir);
static void canvas_object_help(t_canvas *x, t_symbol *name) {
	t_text *o = symbol2opointer(name); if (!o) return;
        const char *hn = class_gethelpname(o->_class);
        bool suffixed = strcmp(hn+strlen(hn)-3, ".pd")==0;
	char *buf;
	asprintf(&buf,"%s%s",hn,suffixed?"":".pd");
        open_via_helppath(buf, o->_class->externdir->name);
	free(buf);
}

static long canvas_children_count(t_canvas *x) {
	long n=0;
	canvas_each(y,x) n++;
	return n;
}

static long canvas_children_last(t_canvas *x) {
	long n=0;
	canvas_each(y,x) n=x->dix->index;
	return n;
}

static void canvas_reorder_last(t_canvas *x, int dest) {
	int i = canvas_children_last(x);
	t_gobj *foo = x->boxes->get(i);
	x->boxes->remove(i);
	fprintf(stderr,"canvas_reorder_last(x=%p,dest=%d) i=%d\n",x,dest,i);
	fprintf(stderr,"foo=%p\n",foo);
	if (!foo) {bug("couldn't remove box #%d",i); return;}
	foo->dix->index = dest;
	x->boxes->add(foo);
}

/* this supposes that $2=#X */
static void canvas_object_insert(t_canvas *x, t_symbol *s, int argc, t_atom *argv) {
	//t_text *o;
	//t_binbuf *b;
	if (argc<1) {error("not enough args"); return;}
	if (argv[0].a_type != A_FLOAT) {error("$1 must be float"); return;}
	int i = atom_getint(argv);
	if (argv[2].a_type != A_SYMBOL) {error("$2 must be symbol"); return;}
	post("will insert object at position %d",i);
	s = argv[2].a_symbol;
	pd_typedmess(x,s,argc-3,argv+3);
	canvas_reorder_last(x,i);
/*err: pd_popsym(x);*/
}

static void g_text_setup() {
    t_class *c;
    text_class  = class_new2("text", 0,0,sizeof(t_text),CLASS_NOINLET|CLASS_PATCHABLE,0);
    dummy_class = class_new2("dummy",0,0,sizeof(t_text),CLASS_NOINLET|CLASS_PATCHABLE,0);

    c = mresp_class = class_new2("messresponder",0,0,sizeof(t_text),CLASS_PD,"");
    class_addbang(    c, mresp_bang);
    class_addfloat(   c, (t_method) mresp_float);
    class_addsymbol(  c, mresp_symbol);
    class_addlist(    c, mresp_list);
    class_addanything(c, mresp_anything);

    c = message_class = class_new2("message",0,0,sizeof(t_message),CLASS_PATCHABLE,"");
    class_addbang(c, message_bang);
    class_addfloat(c, message_float);
    class_addsymbol(c, message_symbol);
    class_addlist(c, message_list);
    class_addanything(c, message_list);
    class_addmethod2(c, message_set, "set","*");
    class_addmethod2(c, message_add, "add","*");
    class_addmethod2(c, message_add2,"add2","*");
    class_addmethod2(c, message_addcomma,   "addcomma",  "");
    class_addmethod2(c, message_addsemi,    "addsemi",   "");
    class_addmethod2(c, message_adddollar,  "adddollar", "f");
    class_addmethod2(c, message_adddollsym, "adddollsym","s");

    c = gatom_class = class_new2("gatom",0,gatom_free,sizeof(t_gatom),CLASS_NOINLET|CLASS_PATCHABLE,"");
    class_addbang(c, gatom_bang);
    class_addfloat(c, gatom_float);
    class_addsymbol(c, gatom_symbol);
    class_addlist(c, gatom_list);
    class_addmethod2(c, gatom_set, "set","*");
    class_addmethod2(c, gatom_reload, "reload","*");
}

static int iemgui_color_hex[]= {
	0xfcfcfc, 0xa0a0a0, 0x404040, 0xfce0e0, 0xfce0c0,
	0xfcfcc8, 0xd8fcd8, 0xd8fcfc, 0xdce4fc, 0xf8d8fc,
	0xe0e0e0, 0x7c7c7c, 0x202020, 0xfc2828, 0xfcac44,
	0xe8e828, 0x14e814, 0x28f4f4, 0x3c50fc, 0xf430f0,
	0xbcbcbc, 0x606060, 0x000000, 0x8c0808, 0x583000,
	0x782814, 0x285014, 0x004450, 0x001488, 0x580050
};

static int iemgui_clip_size(int size) {return max(8,size);}

int convert_color2(int x) {
	return ~ (((0xfc0000&x)>>6) | ((0xfc00&x)>>4) | ((0xfc&x)>>2));
}

static int convert_color(int x) {
	if (x>=0) return iemgui_color_hex[x%30];
	x=~x;
	return ((x&0x3f000)<<6) | ((x&0xfc0)<<4) | ((x&0x3f)<<2);
}

static void iemgui_send(t_iemgui *x, t_symbol *s) {
    SET(snd,canvas_realizedollar(x->canvas, s));
    if (x->snd==s_empty) SET(snd,0);
}

static void iemgui_receive(t_iemgui *x, t_symbol *s) {
    t_symbol *rcv = canvas_realizedollar(x->canvas, s);
    if (rcv==s_empty) rcv=0;
    if (rcv==x->rcv) return;
    if(x->rcv) pd_unbind(x,x->rcv);
    SET(rcv,rcv);
    if(x->rcv)   pd_bind(x,x->rcv);
}

static void iemgui_label(t_iemgui *x, t_symbol *s) {
    SET(lab,s==s_empty?0:s);
}
static void iemgui_label_pos(t_iemgui *x, t_float ldx, t_float ldy) {
    SET(ldx,(int)ldx);
    SET(ldy,(int)ldy);
}
static void iemgui_label_font(t_iemgui *x, t_symbol *s, int ac, t_atom *av) {
    SET(fontsize,max(4,(int)atom_getintarg(1, ac, av)));
    SET(font_style,atom_getintarg(0, ac, av));
}
static void iemgui_delta(t_iemgui *x, t_symbol *s, int ac, t_atom *av) {
    SET(x,x->x+(int)atom_getintarg(0, ac, av));
    SET(y,x->y+(int)atom_getintarg(1, ac, av));
}
static void iemgui_pos(t_iemgui *x, t_symbol *s, int ac, t_atom *av) {
    SET(x,x->x+(int)atom_getintarg(0, ac, av));
    SET(y,x->y+(int)atom_getintarg(1, ac, av));
}

static int iemgui_compatible_col(int i) {return i>=0 ? iemgui_color_hex[i%30] : (~i)&0xffffff;}

static void iemgui_color(t_iemgui *x, t_symbol *s, int ac, t_atom *av) {
    int i=0;
    SET(bcol,iemgui_compatible_col(atom_getintarg(i++, ac, av)));
    if(ac > 2) SET(fcol,iemgui_compatible_col(atom_getintarg(i++, ac, av)));
    SET(lcol,iemgui_compatible_col(atom_getintarg(i++, ac, av)));
}

#define NEXT p=va_arg(val,void*); /*printf("p=%p\n",p);*/
int pd_vscanargs(int argc, t_atom *argv, const char *fmt, va_list val) {
    int optional=0;
    int i,j=0;
    for (i=0; fmt[i]; i++) {
	switch (fmt[i]) {
	    case 0: error("too many args"); return 0;
	    case '*': goto break1; /* rest is any type */
	    case 'F': case 'f': case 'd': case 'i': case 'c': case 'b':
		if (!IS_A_FLOAT(argv,j)) {error("expected float in $%d",i+1); return 0;}
		j++; break;
	    case 'S': case 's':
		if (!IS_A_SYMBOL(argv,j)) {error("expected symbol in $%d",i+1); return 0;}
		j++; break;
	    case '?': break;
	    case 'a':
		if (!IS_A_FLOAT(argv,j) && !IS_A_SYMBOL(argv,j)) {error("expected float or symbol in $%d",i+1); return 0;}
		j++; break;
	    case ';': optional=1; break;
	    default: error("bad format string"); return 0;
	}
    }
    if (j<argc && !optional) {error("not enough args"); return 0;}
break1:
    i=0;
    for (int j=0; fmt[j] || i<argc; j++) {
	void *p;
	switch (fmt[j]) {
	    case ';': continue; /*ignore*/
	    case '*': goto break2;
	    case '?': case 'F': case 'S': break; /* skip */ /* what are those for, again? */
	    case 'd': NEXT; *(double*)p = atom_getfloatarg(i,argc,argv); break;
	    case 'f': NEXT;  *(float*)p = atom_getfloatarg(i,argc,argv); break;
	    case 'i': NEXT;    *(int*)p =   atom_getintarg(i,argc,argv); break;
	    case 'b': NEXT;    *(int*)p = !!atom_getintarg(i,argc,argv); break; /* 0 or 1 */
	    case 'c': NEXT;    *(int*)p = convert_color(atom_getintarg(i,argc,argv)); break; /* IEM-style 8:8:8 colour */
	    case 's': NEXT; *(t_symbol**)p=atom_getsymbolarg(i,argc,argv); break;
	    case 'a': NEXT; /* send-symbol, receive-symbol, or IEM-style label */
		if (IS_A_SYMBOL(argv,i))
			*(t_symbol**)p = atom_getsymbolarg(i,argc,argv);
			if (*(t_symbol**)p == s_empty) *(t_symbol**)p = 0;
		else if (IS_A_FLOAT(argv,i)) {
			char str[80];
			sprintf(str, "%d", (int)atom_getintarg(i,argc,argv));
			*(t_symbol**)p = gensym(str);
		}
	    break;
	    default: post("WARNING: bug using pd_scanargs()"); return 0; /* hmm? */
	}
	i++;
    }
break2:
    return 1;
}

/* exceptionally we're using pointers for each of the args even though
   we are saving. this is so we can copy+paste pd_scanargs lines almost
   directly. in the future, this could be merged with pd_scanargs and
   made declarative, by storing a list of &(0->blah) relative offsets
   into each struct...
*/
int pd_vsaveargs(t_binbuf *b, const char *fmt, va_list val) {
    t_atom a;
    int i;
    for (i=0; ; i++) {
	switch (fmt[i]) {
	    case 0: goto break2;
	    case ';': continue; /* skip */
	    case '?': case 'F': case 'S': break; /* skip */
	    case 'd': SETFLOAT(&a,*(va_arg(val,double*))); break;
	    case 'f': SETFLOAT(&a,*(va_arg(val,float *))); break;
	    case 'i': SETFLOAT(&a,*(va_arg(val,  int *))); break;
	    case 'b': SETFLOAT(&a,!!*(va_arg(val,int *))); break;
	    case 'c': /* colour, from IEM format to RGB 8:8:8 format */
		SETFLOAT(&a,convert_color2(*(va_arg(val,  int *)))); break;
	    case 'a': 
	    case 's': { t_symbol *s = *(va_arg(val,t_symbol**));
		SETSYMBOL(&a,s?s:s_empty); } break;
	    default: post("WARNING: bug using pd_saveargs()"); goto err; /* WHAT? */
	}
	binbuf_add(b,1,&a);
    }
break2:
    binbuf_addv(b, ";");
    return 1;
err:
    post("WARNING: pd_saveargs failed; fmt=%s, i=%d",fmt,i);
    return 0;
}

int pd_scanargs(int argc, t_atom *argv, const char *fmt, ...) {
	int i;
	va_list val;
	va_start(val,fmt);
	i=pd_vscanargs(argc,argv,fmt,val);
	va_end(val);
	return i;
}

int pd_saveargs(t_binbuf *b, const char *fmt, ...) {
	int i;
	va_list val;
	va_start(val,fmt);
	i=pd_vsaveargs(b,fmt,val);
	va_end(val);
	return i;
}

int pd_pickle(t_foo *foo, const char *fmt, ...) {
	va_list val;
	va_start(val,fmt);
	int r = foo->b ?
		pd_vsaveargs(foo->b,fmt,val) :
		pd_vscanargs(foo->argc,foo->argv,fmt,val);
	va_end(val);
	return r;
}

static int pd_savehead(t_binbuf *b, t_iemgui *x, const char *name) {
    binbuf_addv(b, "ttiit","#X","obj", (t_int)x->x, (t_int)x->y, name);
    return 1;
}

void pd_upload(t_gobj *self) {
	long alive = (long)object_table->get(self) & 1;
	if (!alive) {
		sys_mgui(self,"delete","");
		pd_free_zombie(self);
		return;
	}
	t_binbuf *b = binbuf_new();
	t_class *c = self->_class;
	t_text *x = (t_text *)self;
	if (c==canvas_class) {
		/* just the "#N canvas" line, not the contents */
		canvas_savecontainerto((t_canvas *)self,b);
		canvas_savecoordsto((t_canvas *)self,b); /* this may be too early */
		binbuf_addv(b, "ttii", "#X","restore", (t_int)x->x, (t_int)x->y);
		if (x->binbuf) {
			//pd_print(x,"pd_upload");
			binbuf_addbinbuf(b, x->binbuf);
		} else {
			/*bug("binbuf missing at #X restore !!!");*/
		}
		binbuf_addv(b, ";");
	} else { /* this was outside of the "else" for a while. why? I don't remember */
		c->savefn(self,b);
	}
	int n;
	char *s;
	appendix_save(self,b);
	binbuf_gettext(b,&s,&n);
	if (s[n-1]=='\n') n--;
	if (c->patchable) {
		sys_vgui("change x%lx x%lx %d {%.*s} %d %d %d\n",(long)self,(long)self->dix->canvas,self->dix->index,n,s,
			obj_ninlets((t_text *)self), obj_noutlets((t_text *)self), x->_class!=dummy_class);
	} else {
		sys_vgui("change x%lx x%lx %d {%.*s}\n",(long)self,(long)self->dix->canvas,self->dix->index,n,s);
	}
	binbuf_free(b);
	free(s);
	if (c==canvas_class) {
		t_canvas *can = (t_canvas *)self;
		sys_mgui(self,"name=","s",can->name);
		sys_mgui(self,"folder=","s",canvas_getenv(can)->dir);
		sys_mgui(self,"havewindow=","i",can->havewindow);
	}
	if (c==gatom_class) {
		t_gatom *g = (t_gatom *)x;
		if (g->atom.a_type==A_SYMBOL) sys_mgui(g,"set","s",g->atom.a_symbol);
		else                          sys_mgui(g,"set","f",g->atom.a_float);
	}
	if (object_table->exists(self)) {
		object_table->set(self,object_table->get(self)|2); /* has been uploaded */
	} else post("object_table is broken");
}

void sys_mgui(void *self_, const char *sel, const char *fmt, ...) {
	t_gobj *self = (t_gobj *)self_;
	char buf[4096];
	int i=0, n=sizeof(buf);
	va_list val;
	va_start(val,fmt);
	i+=snprintf(buf+i,n-i,"x%lx %s", (long)self, sel);
	if (i>=n) goto over;
	while (*fmt) {
		switch (*fmt) {
		  case 'f': case 'd': i+=snprintf(buf+i,n-i," %f",va_arg(val,double)); break;
		  case 'i': i+=snprintf(buf+i,n-i," %d",va_arg(val,int)); break;
		  case 'p': i+=snprintf(buf+i,n-i," x%lx",(long)va_arg(val,void*)); break;
		  /*
		  case 's': i+=snprintf(buf+i,n-i," \"%s\"",va_arg(val,t_symbol *)->name); break;
		  case 'S': i+=snprintf(buf+i,n-i," \"%s\"",va_arg(val,const char *)); break;
		  */
		  case 's': i+=snprintf(buf+i,n-i," {%s}",va_arg(val,t_symbol *)->name); break;
		  case 'S': i+=snprintf(buf+i,n-i," {%s}",va_arg(val,const char *)); break;
		}
		if (i>=n) goto over;
		fmt++;
	}
	va_end(val);
	i+=snprintf(buf+i,n-i,"\n");
	if (i>=n) goto over;
	sys_gui(buf);
	return;
over:
	post("sys_mgui: can't send: buffer overflow");
	abort();
}

static void iemgui_subclass (t_class *c) {
    class_addmethod2(c, iemgui_delta, "delta","*");
    class_addmethod2(c, iemgui_pos, "pos","*");
    class_addmethod2(c, iemgui_color, "color","*");
    class_addmethod2(c, iemgui_send, "send","S");
    class_addmethod2(c, iemgui_receive, "receive","S");
    class_addmethod2(c, iemgui_label, "label","S");
    class_addmethod2(c, iemgui_label_pos, "label_pos","ff");
    class_addmethod2(c, iemgui_label_font, "label_font","*");
}

t_symbol *pd_makebindsym(t_pd *x) {return symprintf(".x%lx",(long)x);}

t_iemgui *iemgui_new(t_class *qlass) {
	t_iemgui *x = (t_iemgui *)pd_new(qlass);
	x->canvas = canvas_getcurrent();
	x->w = x->h = 15;
	x->ldx=0;
	x->ldy=-6;
	x->isa=0;
	x->font_style = 0;
	x->fontsize = 8;
	x->snd = 0;
	x->rcv = 0;
	x->lab = s_empty;
	x->bcol = 0xffffff;
	x->fcol = 0x000000;
	x->lcol = 0x000000;
	pd_bind(x,pd_makebindsym(x));
	return x;
}

static void iemgui_constrain(t_iemgui *x) {
    SET(fontsize,max(x->fontsize,4));
    SET(h,iemgui_clip_size(x->h));
    SET(w,iemgui_clip_size(x->w));
}

void iemgui_init(t_iemgui *x, t_floatarg f) {SET(isa,(x->isa&~1)|!!f);}

void binbuf_update(t_iemgui *x, t_symbol *qlass, int argc, t_atom *argv) {
    t_binbuf *buf = x->binbuf;
    if (!buf) return;
    binbuf_clear(buf);
    t_atom foo;
    SETSYMBOL(&foo,qlass);
    binbuf_add(buf,1,&foo);
    binbuf_add(buf,argc,argv);
}

static /*bool*/ int iemgui_loadbang (t_iemgui *self) {
	return !sys_noloadbang && self->isa&1;
}

static /*bool*/ int iemgui_forward (t_iemgui *self) {
	return !self->snd || !self->rcv || self->snd != self->rcv;
}

static t_class *bng_class;

static void bng_check_minmax(t_bng *x) {
    if(x->ftbreak > x->fthold) {
	int h = x->ftbreak;
	SET(ftbreak,x->fthold);
	SET(fthold,h);
    }
    SET(ftbreak,max(x->ftbreak,10));
    SET(fthold ,max(x->fthold, 50));
}

static void bng_set(t_bng *x) {
	SET(count,x->count+1);
	sys_mgui(x,"bang","i",x->count);
}

static void bng_bout2(t_bng *x) {
    outlet_bang(x->outlet);
    if(x->snd && x->snd->thing) pd_bang(x->snd->thing);
}

static void bng_bang(t_bng *x) {
    bng_set(x);
    outlet_bang(x->outlet);
    if(x->snd && x->snd->thing && iemgui_forward(x)) pd_bang(x->snd->thing);
}
static void bng_bang2   (t_bng *x) {                       {bng_set(x); bng_bout2(x);}}
static void bng_loadbang(t_bng *x) {if(iemgui_loadbang(x)) {bng_set(x); bng_bout2(x);}}

static void bng_size(t_bng *x, t_symbol *s, int ac, t_atom *av) {
    SET(w,iemgui_clip_size((int)atom_getintarg(0, ac, av)));
    SET(h,x->w);
}

static void bng_flashtime(t_bng *x, t_symbol *s, int ac, t_atom *av) {
    SET(ftbreak,atom_getintarg(0, ac, av));
    SET(fthold  ,atom_getintarg(1, ac, av));
    bng_check_minmax(x);
}

static int bng_pickle(t_bng *x, t_foo *foo) {
    return pd_pickle(foo,"iiiiaaaiiiiccc",&x->w,&x->fthold,&x->ftbreak,&x->isa,&x->snd,&x->rcv,&x->lab,
	&x->ldx,&x->ldy,&x->font_style,&x->fontsize,&x->bcol,&x->fcol,&x->lcol);
}

static void bng_savefn(t_bng *x, t_binbuf *b) {
    t_foo foo = {0,0,b}; if (!b) return;
    pd_savehead(b,x,"bng"); bng_pickle(x,&foo);
}

static void bng_reload(t_bng *x, t_symbol *s, int argc, t_atom *argv) {
    t_foo foo = { argc, argv, 0 };
    binbuf_update(x,gensym("bng"),argc,argv);
    if (!bng_pickle(x,&foo)) return;
    SET(h,x->w);
    bng_check_minmax(x);
    iemgui_constrain(x);
    if (x->rcv) pd_bind(x,x->rcv);
}

static void *bng_new(t_symbol *s, int argc, t_atom *argv) {
    t_bng *x = (t_bng *)iemgui_new(bng_class);
    SET(ftbreak,250);
    SET(fthold,50);
    SET(count,0);
    bng_check_minmax(x);
    outlet_new(x, &s_bang);
    if (argc) bng_reload(x,0,argc,argv);
    return x;
}

static void iemgui_free(t_iemgui *x) {
    if(x->rcv) pd_unbind(x,x->rcv);
}

static t_class *toggle_class;

static void toggle_action(t_toggle *x) {
    outlet_float(x->outlet, x->on);
    if(x->snd && x->snd->thing) pd_float(x->snd->thing, x->on);
}

static void toggle_bang(t_toggle *x) {SET(on,x->on?0.0:x->nonzero); toggle_action(x);}
static void toggle_set(t_toggle *x, t_floatarg f) {SET(on,f); if(f) SET(nonzero,f);}
static void toggle_float(t_toggle *x, t_floatarg f) {toggle_set(x,f);if(iemgui_forward(x)) toggle_action(x);}
static void toggle_fout (t_toggle *x, t_floatarg f) {toggle_set(x,f);                      toggle_action(x);}
static void toggle_loadbang(t_toggle *x) {if(iemgui_loadbang(x)) toggle_fout(x, (float)x->on);}
static void toggle_nonzero(t_toggle *x, t_floatarg f) {if (f) SET(nonzero,f);}

static void toggle_size(t_toggle *x, t_symbol *s, int ac, t_atom *av) {
    SET(w,iemgui_clip_size((int)atom_getintarg(0, ac, av)));
    SET(h,x->w);
}

static int toggle_pickle(t_toggle *x, t_foo *foo) {
    return pd_pickle(foo,"iiaaaiiiicccf;f",&x->w,&x->isa,&x->snd,&x->rcv,&x->lab,
	&x->ldx,&x->ldy,&x->font_style,&x->fontsize,&x->bcol,&x->fcol,&x->lcol,&x->on,&x->nonzero);
}

static void toggle_savefn(t_toggle *x, t_binbuf *b) {
    t_foo foo = {0,0,b}; if (!b) return;
    pd_savehead(b,x,"tgl"); toggle_pickle(x,&foo);
}

static void toggle_reload(t_toggle *x, t_symbol *s, int argc, t_atom *argv) {
    t_foo foo = { argc, argv, 0 };
    binbuf_update(x,gensym("tgl"),argc,argv);
    if (!toggle_pickle(x,&foo)) return;
    SET(h,x->w);
    SET(on,x->isa&1 && x->on ? x->nonzero : 0.0);
    SET(nonzero,argc==14 && IS_A_FLOAT(argv,13) ? atom_getfloatarg(13, argc, argv) : 1.0);
    if (!x->nonzero) SET(nonzero,1.0);
    iemgui_constrain(x);
    if (x->rcv) pd_bind(x,x->rcv);
}

static void *toggle_new(t_symbol *s, int argc, t_atom *argv) {
    t_toggle *x = (t_toggle *)iemgui_new(toggle_class);
    SET(on,0.0);
    SET(nonzero,1.0);
    outlet_new(x, &s_float);
    if (argc) toggle_reload(x,0,argc,argv);
    return x;
}

static void radio_set(t_radio *x, t_floatarg f) {
    int i=(int)f;
    int old=x->on_old;
    CLAMP(i,0,x->number-1);
    if(x->on!=old) SET(on_old,x->on);
    SET(on,i);
    if(x->on!=old) SET(on_old,old);
}

static void radio_send2(t_radio *x, float a, float b) {
	SETFLOAT(x->at,a);
	SETFLOAT(x->at+1,b);
	outlet_list(x->outlet, &s_list, 2, x->at);
	if(x->snd && x->snd->thing) pd_list(x->snd->thing, &s_list, 2, x->at);
}

static void radio_send(t_radio *x, float a) {
    	outlet_float(x->outlet,a);
	if(x->snd && x->snd->thing) pd_float(x->snd->thing,a);
}

static void radio_bang(t_radio *x) {
    if (x->oldstyle) {
	if(x->change && x->on!=x->on_old) radio_send2(x,x->on_old,0.0);
	SET(on_old,x->on);
	radio_send2(x,x->on,1.0);
    } else {
	radio_send(x,x->on);
    }
}

static void radio_fout2(t_radio *x, t_floatarg f, int forwardonly) {
    int i=(int)f;
    CLAMP(i,0,x->number-1);
    if (x->oldstyle) {
	/* compatibility with earlier "hdial" behavior */
	if(x->change && i!=x->on_old && (!forwardonly || iemgui_forward(x))) radio_send2(x,x->on_old,0.0);
	SET(on_old,x->on);
	SET(on,i);
	SET(on_old,x->on);
	radio_send2(x,x->on,1.0);
	if (!forwardonly || iemgui_forward(x)) radio_send2(x,x->on,1.0);
    } else {
    	SET(on,i);
	if (!forwardonly || iemgui_forward(x)) radio_send(x,x->on);
	SET(on_old,x->on);
    }
}

static void radio_fout (t_radio *x, t_floatarg f) {radio_fout2(x,f,0);}
static void radio_float(t_radio *x, t_floatarg f) {radio_fout2(x,f,1);}
static void radio_loadbang(t_radio *x) {if(iemgui_loadbang(x)) radio_bang(x);}
static void radio_orient(t_radio *x,t_floatarg v) {SET(orient,!!v);
post("v=%f, !!v=%d, orient=%d",v,!!v,x->orient);}

static void radio_number(t_radio *x, t_floatarg num) {
    int n=(int)num;
    CLAMP(n,1,128);
    if (n != x->number) {
	SET(number,n);
	CLAMP(x->on,0,x->number-1); gobj_changed(x,"on");
	SET(on_old,x->on);
    }
}

static void radio_size(t_radio *x, t_float size) {
    SET(w,iemgui_clip_size((int)size));
    SET(h,x->w);
}

static void radio_double_change(t_radio *x) {SET(change,1);}
static void radio_single_change(t_radio *x) {SET(change,0);}

static int radio_pickle(t_radio *x, t_foo *foo) {
    return pd_pickle(foo, "ibiiaaaiiiiccci",&x->w,&x->change,&x->isa,&x->number,&x->snd,&x->rcv,&x->lab,
	&x->ldx,&x->ldy,&x->font_style,&x->fontsize,&x->bcol,&x->fcol,&x->lcol,&x->on);
}

static t_symbol *radio_flavor(t_radio *x) {
	return x->orient?x->oldstyle?sym_vdl:sym_vradio:x->oldstyle?sym_hdl:sym_hradio;
}

static void radio_savefn(t_radio *x, t_binbuf *b) {
    t_foo foo = {0,0,b}; if (!b) return;
    pd_savehead(b,x,radio_flavor(x)->name);
    radio_pickle(x,&foo);
}

static void radio_reload(t_radio *x, t_symbol *s, int argc, t_atom *argv) {
    t_foo foo = { argc, argv, 0 };
    binbuf_update(x,radio_flavor(x),argc,argv);
    if (!radio_pickle(x,&foo)) return;
    iemgui_constrain(x);
    if (x->rcv) pd_bind(x,x->rcv);
    gobj_changed(x,0);
}

static void *radio_new(t_symbol *s, int argc, t_atom *argv) {
    t_radio *x = (t_radio *)iemgui_new(radio_class);
    SET(on_old,0);
    SET(on,0);
    SET(number,8);
    SET(change,1);
    if (s==sym_hdl)    {SET(orient,0); SET(oldstyle,1);} else
    if (s==sym_vdl)    {SET(orient,1); SET(oldstyle,1);} else
    if (s==sym_hradio) {SET(orient,0); SET(oldstyle,0);} else
    if (s==sym_vradio) {SET(orient,1); SET(oldstyle,0);}
    SET(on,x->isa&1 ? x->on : 0);
    SET(on_old,x->on);
    outlet_new(x, &s_list);
    if (argc) radio_reload(x,0,argc,argv);
    return x;
}

#define IEM_SL_DEFAULTSIZE 128
#define IEM_SL_MINSIZE 2

static void slider_check_width(t_slider *x, int w) {
    double l = (double)(x->orient ? x->h : x->w)-1;
    int m = (int)(l*100);
    if(w < IEM_SL_MINSIZE) w = IEM_SL_MINSIZE;
    if (x->orient) SET(h,w); else SET(w,w);
    if(x->val > m) SET(val,m);
}

static void slider_check_minmax(t_slider *x) {
    double min=x->min, max=x->max;
    if(x->is_log) {
	if(min == 0.0 && max == 0.0) max = 1.0;
	if(max > 0.0) { if (min<=0.0) min = 0.01*max; }
	else          { if (min >0.0) max = 0.01*min; }
    }
    SET(min,min);
    SET(max,max);
}

// the value/centipixel ratio
static double slider_ratio (t_slider *x) {
	double diff = x->is_log ? log(x->max/x->min) : (x->max-x->min);
	return diff / (double)(x->orient ? (x->h-1) : (x->w-1));
}

static void slider_set(t_slider *x, t_floatarg f) {
    if(x->min > x->max) CLAMP(f,x->max,x->min);
    else                CLAMP(f,x->min,x->max);
    SET(val,floor(100.0 * (x->is_log ? log(f/x->min) : (f-x->min)) / slider_ratio(x) + 0.5));
}

static void slider_bang(t_slider *x) {
    double t = (double)x->val * slider_ratio(x) * 0.01;
    double out = x->is_log ? x->min*exp(t) : x->min+t;
    if (fabs(out) < 1.0e-10) out = 0.0;
    outlet_float(x->outlet, out);
    if(x->snd && x->snd->thing) pd_float(x->snd->thing, out);
}

static void slider_size(t_slider *x, t_symbol *s, int ac, t_atom *av) {
    int a = atom_getintarg(0,ac,av);
    int b = ac>1 ? atom_getintarg(1,ac,av) : 0;
    if (x->orient) {
	SET(w,iemgui_clip_size(a));
        if(ac>1) slider_check_width(x,b);
    } else {
	slider_check_width(x,a);
	if(ac>1) SET(h,iemgui_clip_size(b));
    }
}

static void slider_range(t_slider *x, t_float min, t_float max)
{SET(min,min); SET(max,max); slider_check_minmax(x);}
static void slider_lin(t_slider *x) {SET(is_log,0); slider_check_minmax(x);}
static void slider_log(t_slider *x) {SET(is_log,1); slider_check_minmax(x);}
static void slider_steady(t_slider *x, t_floatarg f) {SET(steady,!!f);}
static void slider_float(t_slider *x, t_floatarg f) {slider_set(x,f);if(iemgui_forward(x))slider_bang(x);}
static void slider_loadbang(t_slider *x) {if(iemgui_loadbang(x)) slider_bang(x);}
static void slider_orient(t_slider *x,t_floatarg v) {SET(orient,!!v);}

static int slider_pickle(t_slider *x, t_foo *foo) {
    return pd_pickle(foo, "iiffbiaaaiiiicccf;b",
	&x->w,&x->h,&x->min,&x->max,&x->is_log,&x->isa,&x->snd,&x->rcv,&x->lab,
	&x->ldx,&x->ldy,&x->font_style,&x->fontsize,&x->bcol,&x->fcol,&x->lcol,&x->val,&x->steady);
}

static void slider_savefn(t_slider *x, t_binbuf *b) {
    t_foo foo = {0,0,b}; if (!b) return;
    pd_savehead(b,x,(char *)(x->orient?"vsl":"hsl")); slider_pickle(x,&foo);
}

static void slider_reload(t_slider *x, t_symbol *s, int argc, t_atom *argv) {
    t_foo foo = { argc, argv, 0 };
    binbuf_update(x,gensym((char *)(x->orient?"vsl":"hsl")),argc,argv);
    if (!slider_pickle(x,&foo)) return;
//this is wrong because it should happen when loading a file but not when loading from properties:
    SET(val,x->isa&1 ? x->val : 0);
//end wrong.
    iemgui_constrain(x);
    slider_check_minmax(x);
    slider_check_width(x, x->orient ? x->h : x->w);
    if(x->rcv) pd_bind(x,x->rcv);
    gobj_changed(x,0);
}

static void *slider_new(t_symbol *s, int argc, t_atom *argv) {
    t_slider *x = (t_slider *)iemgui_new(slider_class);
    SET(orient,s==sym_vslider||s==sym_vsl);
    SET(is_log,0);
    SET(min,0.0);
    SET(steady,1);
    SET(max,(double)(IEM_SL_DEFAULTSIZE-1));
    if (x->orient) SET(h,IEM_SL_DEFAULTSIZE); else SET(w,IEM_SL_DEFAULTSIZE);
    outlet_new(x, &s_float);
    if (argc) slider_reload(x,0,argc,argv);
    return x;
}

static t_class *nbx_class;

static void nbx_clip(t_nbx *x) {CLAMP(x->val,x->min,x->max);}

static int nbx_check_minmax(t_nbx *x) {
    double min=x->min, max=x->max;
    int val=(int)x->val;
    if(x->is_log) {
	if(min==0.0 && max==0.0) max = 1.0;
	if(max>0.0 && min<=0.0) min = 0.01*max;
	if(max<=0.0 && min>0.0) max = 0.01*min;
    } else {
	if(min>max) swap(min,max);
    }
    SET(min,min);
    SET(max,max);
    CLAMP(x->val,x->min,x->max);
    SET(k,x->is_log ? exp(log(x->max/x->min)/(double)(x->log_height)) : 1.0);
    return x->val!=val;
}

static void nbx_bang(t_nbx *x) {
    outlet_float(x->outlet, x->val);
    if(x->snd && x->snd->thing) pd_float(x->snd->thing, x->val);
}

static void nbx_set(t_nbx *x, t_floatarg f) {SET(val,f); nbx_clip(x);}
static void nbx_float(t_nbx *x, t_floatarg f) {nbx_set(x, f); if(iemgui_forward(x)) nbx_bang(x);}

static void nbx_log_height(t_nbx *x, t_floatarg lh) {
    SET(log_height,max(10,(int)lh));
    SET(k,x->is_log ? exp(log(x->max/x->min)/(double)(x->log_height)) : 1.0);
}

static void nbx_size(t_nbx *x, t_symbol *s, int ac, t_atom *av) {
    SET(w,max(1,(int)atom_getintarg(0, ac, av)));
    if(ac > 1) SET(h,max(8,(int)atom_getintarg(1, ac, av)));
}

static void nbx_range(t_nbx *x, t_float min, t_float max)
{SET(min,min); SET(max,max); nbx_check_minmax(x);}

static void nbx_lin(t_nbx *x) {SET(is_log,0);                     }
static void nbx_log(t_nbx *x) {SET(is_log,1); nbx_check_minmax(x);}
static void nbx_loadbang(t_nbx *x) {if(iemgui_loadbang(x)) nbx_bang(x);}

static void nbx_list(t_nbx *x, t_symbol *s, int ac, t_atom *av) {
    if (!IS_A_FLOAT(av,0)) return;
    nbx_set(x, atom_getfloatarg(0, ac, av));
    nbx_bang(x);
}

static int nbx_pickle(t_nbx *x, t_foo *foo) {
    return pd_pickle(foo,"iiddbiaaaiiiicccd;i",
	&x->w,&x->h,&x->min,&x->max,&x->is_log,&x->isa,&x->snd,&x->rcv,&x->lab,
	&x->ldx,&x->ldy,&x->font_style,&x->fontsize,&x->bcol,&x->fcol,&x->lcol,&x->val,&x->log_height);
}

static void nbx_savefn(t_nbx *x, t_binbuf *b) {
    t_foo foo = {0,0,b};
    if (!b) return;
    pd_savehead(b,x,"nbx");
    nbx_pickle(x,&foo);
}

static void nbx_reload(t_nbx *x, t_symbol *s, int argc, t_atom *argv) {
    t_foo foo = { argc, argv, 0 };
    binbuf_update(x,gensym("nbx"),argc,argv);
    if (!nbx_pickle(x,&foo)) return;
    if (!x->isa&1) SET(val,0.0);
    iemgui_constrain(x);
    SET(w,max(x->w,1));
    nbx_check_minmax(x);
    SET(w,max(x->w,1));
    if (x->rcv) pd_bind(x,x->rcv);
    gobj_changed(x,0);
}

static void *nbx_new(t_symbol *s, int argc, t_atom *argv) {
    t_nbx *x = (t_nbx *)iemgui_new(nbx_class);
    SET(log_height,256);
    SET(is_log,0);
    SET(w,5);
    SET(h,14);
    SET(min,-1.0e+37);
    SET(max,1.0e+37);
    x->buf[0]=0;
    SET(change,0);
    outlet_new(x, &s_float);
    if (argc) nbx_reload(x,0,argc,argv);
    return x;
}

#define IEM_VU_STEPS 40

static char vu_db2i[]= {
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
     5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
     9, 9, 9, 9, 9,10,10,10,10,10,11,11,11,11,11,12,12,12,12,12,
    13,13,13,13,14,14,14,14,15,15,15,15,16,16,16,16,17,17,17,18,
    18,18,19,19,19,20,20,20,21,21,22,22,23,23,24,24,25,26,27,28,
    29,30,31,32,33,33,34,34,35,35,36,36,37,37,37,38,38,38,39,39,
    39,39,39,39,40,40
};

static void vu_check_height(t_vu *x, int h) {
    int n=max(h/IEM_VU_STEPS,2);
    SET(led_size,n-1);
    SET(h,IEM_VU_STEPS * n);
}

static void vu_scale(t_vu *x, t_floatarg fscale) {SET(scale,!!fscale);}

static void vu_size(t_vu *x, t_symbol *s, int ac, t_atom *av) {
    SET(w,    iemgui_clip_size((int)atom_getintarg(0, ac, av)));
    if(ac>1) vu_check_height(x, (int)atom_getintarg(1, ac, av));
}

static int vuify(t_vu *x, float v) {
    return v<=-99.9 ? 0 :
	v>=12.0 ? IEM_VU_STEPS :
	vu_db2i[(int)(2.0*(v+100.0))];
}

static float vu_round(float v) {return 0.01*(int)(100.0*v+0.5);}

static void vu_float0(t_vu *x, t_floatarg v) {
	SET(rms, vuify(x,v)); SET(fr,vu_round(v)); outlet_float(x->out(0), x->fr);
	sys_mgui(x,"rms=","i",x->rms);}
static void vu_float1(t_vu *x, t_floatarg v) {
	SET(peak,vuify(x,v)); SET(fp,vu_round(v)); outlet_float(x->out(1),x->fp);
	sys_mgui(x,"peak=","i",x->peak);}

static void vu_bang(t_vu *x) {
    outlet_float(x->out(1), x->fp);
    outlet_float(x->out(0),  x->fr);
}

static int vu_pickle(t_vu *x, t_foo *foo) {
    return pd_pickle(foo,"iiaaiiiiccb;i",&x->w,&x->h,&x->rcv,&x->lab,&x->ldx,&x->ldy,&x->font_style,
	&x->fontsize,&x->bcol,&x->lcol,&x->scale,&x->isa);
}

static void vu_savefn(t_vu *x, t_binbuf *b) {
    t_foo foo = {0,0,b}; if (!b) return;
    pd_savehead(b,x,"vu"); vu_pickle(x,&foo);
}

static void vu_reload(t_vu *x, t_symbol *s, int argc, t_atom *argv) {
    t_foo foo = { argc, argv, 0 };
    binbuf_update(x,gensym("vu"),argc,argv);
    if (!vu_pickle(x,&foo)) return;
    iemgui_constrain(x);
    if(x->rcv) pd_bind(x,x->rcv);
    gobj_changed(x,0);
}

static t_class *vu_class;

static void *vu_new(t_symbol *s, int argc, t_atom *argv) {
    t_vu *x = (t_vu *)iemgui_new(vu_class);
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    SET(bcol,0x000000);
    SET(h,IEM_VU_STEPS*3);
    SET(scale,1);
    SET(rms,0); /* ??? */
    SET(peak,0);
    SET(fp,-101.0);
    SET(fr,-101.0);
    vu_check_height(x,x->h);
    inlet_new(x,x,&s_float,gensym("ft1"));
    if (argc) vu_reload(x,0,argc,argv);
    return x;
}

static t_class *cnv_class;

static void cnv_get_pos(t_cnv *x) {
    error("unimplemented (TODO)");
//    if(x->snd && x->snd->thing) {x->at[0].a_float = x; x->at[1].a_float = y; pd_list(x->snd->thing, &s_list, 2, x->at);}
}

static void cnv_size(t_cnv *x, t_symbol *s, int ac, t_atom *av) {
  SET(h,max(1,(int)atom_getintarg(0, ac, av)));
  SET(w,x->h);
}

static void cnv_vis_size(t_cnv *x, t_symbol *s, int ac, t_atom *av) {
    SET(vis_w,max(1,(int)atom_getintarg(0, ac, av)));
    SET(vis_h,x->w);
    if(ac > 1)   SET(vis_h,max(1,(int)atom_getintarg(1, ac, av)));
    gobj_changed(x,0);
}

static int cnv_pickle(t_cnv *x, t_foo *foo) {
    return pd_pickle(foo,"iiiaaaiiiicc;i",&x->w,&x->vis_w,&x->vis_h,
	&x->snd,&x->rcv,&x->lab,&x->ldx,&x->ldy,&x->font_style,&x->fontsize,&x->bcol,&x->lcol,&x->isa);
}

static void cnv_savefn(t_cnv *x, t_binbuf *b) {
    t_foo foo = {0,0,b}; if (!b) return;
    pd_savehead(b,x,"cnv"); cnv_pickle(x,&foo);
}

static void cnv_reload(t_cnv *x, t_symbol *s, int argc, t_atom *argv) {
    t_foo foo = { argc, argv, 0 };
    binbuf_update(x,gensym("cnv"),argc,argv);
    if (!cnv_pickle(x,&foo)) return;
    SET(w,max(x->w,1));
    SET(h,x->w);
    SET(vis_w,max(x->vis_w,1));
    SET(vis_h,max(x->vis_h,1));
    x->at[0].a_type = x->at[1].a_type = A_FLOAT; //???
    iemgui_constrain(x);
    if (x->rcv) pd_bind(x,x->rcv);
    gobj_changed(x,0);
}
#undef FOO

static void *cnv_new(t_symbol *s, int argc, t_atom *argv) {
    t_cnv *x = (t_cnv *) iemgui_new(cnv_class);
    SET(bcol,0xe0e0e0);
    SET(fcol,0x000000);
    SET(lcol,0x404040);
    SET(w,15);
    SET(vis_w,100);
    SET(vis_h,60);
    if (argc) cnv_reload(x,0,argc,argv);
    return x;
}

void canvas_notice(t_gobj *x, t_gobj *origin, int argc, t_atom *argv) {
	t_canvas *self = (t_canvas *)x;
	gobj_changed3(self,origin,argc,argv);
}

void gobj_onsubscribe(t_gobj *x, t_gobj *observer) {gobj_changed(x,0);}

void canvas_onsubscribe(t_gobj *x, t_gobj *observer) {
	t_canvas *self = (t_canvas *)x;
	gobj_onsubscribe(x,observer);
	canvas_each(         y,self)  y->_class->onsubscribe( y,observer);
	canvas_wires_each(oc,t,self) oc->_class->onsubscribe(oc,observer);
}

/* [declare] and canvas_open come from 0.40 */
/* ------------------------------- declare ------------------------ */

/* put "declare" objects in a patch to tell it about the environment in
which objects should be created in this canvas.  This includes directories to
search ("-path", "-stdpath") and object libraries to load
("-lib" and "-stdlib").  These must be set before the patch containing
the "declare" object is filled in with its contents; so when the patch is
saved,  we throw early messages to the canvas to set the environment
before any objects are created in it. */

struct t_declare : t_object {
    int useme;
};

static void *declare_new(t_symbol *s, int argc, t_atom *argv) {
    t_declare *x = (t_declare *)pd_new(declare_class);
    x->useme = 1;
    /* LATER update environment and/or load libraries */
    return x;
}

static void declare_free(t_declare *x) {
    x->useme = 0;
    /* LATER update environment */
}

void canvas_savedeclarationsto(t_canvas *x, t_binbuf *b) {
    canvas_each(y,x) {
        if (pd_class(y) == declare_class) {
            binbuf_addv(b,"t","#X");
            binbuf_addbinbuf(b, ((t_declare *)y)->binbuf);
            binbuf_addv(b, ";");
        } else if (pd_class(y) == canvas_class) canvas_savedeclarationsto((t_canvas *)y, b);
    }
}

static void canvas_declare(t_canvas *x, t_symbol *s, int argc, t_atom *argv) {
    t_canvasenvironment *e = canvas_getenv(x);
#if 0
    startpost("declare:: %s", s->name);
    postatom(argc, argv);
    endpost();
#endif
    for (int i=0; i<argc; i++) {
        char *buf;
        char *flag = atom_getsymbolarg(i, argc, argv)->name;
        if ((argc > i+1) && !strcmp(flag, "-path")) {
            e->path = namelist_append(e->path, atom_getsymbolarg(i+1, argc, argv)->name, 0);
            i++;
        } else if (argc>i+1 && !strcmp(flag, "-stdpath")) {
            asprintf(&buf, "%s/%s", sys_libdir->name, atom_getsymbolarg(i+1, argc, argv)->name);
            e->path = namelist_append(e->path,buf,0);
            i++;
        } else if (argc>i+1 && !strcmp(flag, "-lib")) {
            sys_load_lib(x, atom_getsymbolarg(i+1, argc, argv)->name);
            i++;
        } else if (argc>i+1 && !strcmp(flag, "-stdlib")) {
            asprintf(&buf, "%s/%s", sys_libdir->name, atom_getsymbolarg(i+1, argc, argv)->name);
            sys_load_lib(0,buf);
            i++;
        } else post("declare: %s: unknown declaration", flag);
    }
}

/* utility function to read a file, looking first down the canvas's search path (set with "declare"
   objects in the patch and recursively in calling patches), then down the system one. The filename
   is the concatenation of "name" and "ext". "Name" may be absolute, or may be relative with slashes.
   If anything can be opened, the true directory is put in the buffer dirresult (provided by caller),
   which should be "size" bytes.  The "nameresult" pointer will be set somewhere in the interior of
   "dirresult" and will give the file basename (with slashes trimmed).  If "bin" is set a 'binary'
   open is attempted, otherwise ASCII (this only matters on Microsoft.) If "x" is zero, the file is
   sought in the directory "." or in the global path.*/
int canvas_open2(t_canvas *x, const char *name, const char *ext, char **dirresult, char **nameresult, int bin) {
    int fd = -1;
    /* first check if "name" is absolute (and if so, try to open) */
    if (sys_open_absolute(name, ext, dirresult, nameresult, bin, &fd)) return fd;
    /* otherwise "name" is relative; start trying in directories named in this and parent environments */
    for (t_canvas *y=x; y; y = y->dix->canvas) if (y->env) {
        t_canvas *x2 = x;
        while (x2 && x2->dix->canvas) x2 = x2->dix->canvas;
        const char *dir = x2 ? canvas_getdir(x2)->name : ".";
        for (t_namelist *nl = y->env->path; nl; nl = nl->nl_next) {
            char *realname;
            asprintf(&realname, "%s/%s", dir, nl->nl_string);
            if ((fd = sys_trytoopenone(realname, name, ext, dirresult, nameresult, bin)) >= 0) return fd;
        }
    }
    return  open_via_path2((x ? canvas_getdir(x)->name : "."), name, ext, dirresult, nameresult, bin);
}
/* end miller 0.40 */

int canvas_open(t_canvas *x, const char *name, const char *ext, char *dirresult, char **nameresult, unsigned int size, int bin) {
    char *dirr;
    int r = canvas_open2(x,name,ext,&dirr,nameresult,bin);
    if (dirr) {strncpy(dirresult,dirr,size); dirresult[size-1]=0; free(dirr);}
    return r;
}

static void canvas_with_reply (t_pd *x, t_symbol *s, int argc, t_atom *argv) {
	if (!( argc>=2 && IS_A_FLOAT(argv,0) && IS_A_SYMBOL(argv,1) )) return;
	pd_typedmess(x,atom_getsymbol(&argv[1]),argc-2,argv+2);
	queue_put(manager->q,reply_new((short)atom_getfloat(&argv[0]),newest));
}

static void canvas_get_elapsed (t_canvas *x) {
	canvas_each(y,x) {
		sys_mgui(y,"elapsed","f",y->dix->elapsed / 800000000.0);
	}
}

static void g_canvas_setup() {
    reply_class = class_new2("reply",0,reply_free,sizeof(t_reply),CLASS_GOBJ,"!");
//    class_setsavefn(reply_class, (t_savefn)reply_savefn);
    declare_class = class_new2("declare",declare_new,declare_free,sizeof(t_declare),CLASS_NOINLET,"*");
    t_class *c = canvas_class = class_new2("canvas",0,canvas_free,sizeof(t_canvas),CLASS_NOINLET,"");
    /* here is the real creator function, invoked in patch files
       by sending the "canvas" message to #N, which is bound to pd_canvasmaker. */
    class_addmethod2(pd_canvasmaker._class,canvas_new,"canvas","*");
    class_addmethod2(c,canvas_restore,"restore","*");
    class_addmethod2(c,canvas_coords,"coords","*");
    class_addmethod2(c,canvas_setbounds,"bounds","ffff");
    class_addmethod2(c,canvas_obj,"obj","*");
    class_addmethod2(c,canvas_msg,"msg","*");
    class_addmethod2(c,canvas_floatatom,"floatatom","*");
    class_addmethod2(c,canvas_symbolatom,"symbolatom","*");
    class_addmethod2(c,canvas_text,"text","*");
    class_addmethod2(c,canvas_canvas,"graph","*");
    class_addmethod2(c,canvas_scalar,"scalar","*");
    class_addmethod2(c,canvas_declare,"declare","*");
    class_addmethod2(c,canvas_push,"push","");
    class_addmethod2(c,canvas_pop,"pop","F");
    class_addmethod2(c,canvas_loadbang,"loadbang","");
    class_addmethod2(c,canvas_relocate,"relocate","ss");
    class_addmethod2(c,canvas_vis,"vis","f");
    class_addmethod2(c,canvas_menu_open,"menu-open","");
    class_addmethod2(c,canvas_clear,"clear","");
    class_addcreator2("pd",subcanvas_new,"S");
    class_addcreator2("page",subcanvas_new,"S");
    class_addmethod2(c,canvas_dsp,"dsp","");
    class_addmethod2(c,canvas_rename_method,"rename","*");
    class_addcreator2("table",table_new,"SF");
    class_addmethod2(c,canvas_close,"close","F");
    class_addmethod2(c,canvas_redraw,"redraw","");
    class_addmethod2(c,canvas_find_parent,"findparent","");
    class_addmethod2(c,canvas_arraydialog,"arraydialog","sfff");
    class_addmethod2(c,canvas_connect,"connect","ffff");
    class_addmethod2(c,canvas_disconnect,"disconnect","ffff");
    class_addmethod2(c,canvas_write,"write","sS");
    class_addmethod2(c,canvas_read, "read","sS");
    class_addmethod2(c,canvas_mergefile,  "mergefile","sS");
    class_addmethod2(c,canvas_savetofile,"savetofile","ss");
    class_addmethod2(c,canvas_saveto,    "saveto","!");
    class_addmethod2(c,graph_bounds,"bounds","ffff");
    class_addmethod2(c,graph_xticks,"xticks","fff");
    class_addmethod2(c,graph_xlabel,"xlabel","*");
    class_addmethod2(c,graph_yticks,"yticks","fff");
    class_addmethod2(c,graph_ylabel,"ylabel","*");
    class_addmethod2(c,graph_array,"array","sfsF");
    //class_addmethod2(c,canvas_sort,"sort","");
// dd-specific
    class_addmethod2(c,canvas_object_moveto,"object_moveto","sff");
    class_addmethod2(c,canvas_object_delete,"object_delete","s");
    class_addmethod2(c,canvas_object_insert,"object_insert","*");
    class_addmethod2(c,canvas_object_get_tips,"object_get_tips","s");
    class_addmethod2(c,canvas_object_help,"object_help","s");
    class_addmethod2(c,canvas_text_setto,"text_setto","*");
    class_addmethod2(c,canvas_with_reply,"with_reply","*");
    class_addmethod2(pd_canvasmaker._class,canvas_with_reply,"with_reply","*");
    class_addmethod2(c,canvas_get_elapsed,"get_elapsed","");
    class_setnotice(c, canvas_notice);
    class_setonsubscribe(c, canvas_onsubscribe);
}

t_class *visualloader_class;

static t_pd *visualloader_new(t_symbol *s, int argc, t_atom *argv) {return pd_new(visualloader_class);}
static void visualloader_free(t_pd *self) {free(self);}
static void copy_atoms(int argc, t_atom *argvdest, t_atom *argvsrc) {memcpy(argvdest,argvsrc,argc*sizeof(t_atom));}
static void visualloader_anything(t_gobj *self, t_symbol *s, int argc, t_atom *argv) {
	int i=0,j=0;
	//printf("visualloader_anything start newest=%p\n",newest);
	while (j<argc) {
		i=j;
		while (j<argc && atom_getsymbolarg(j,argc,argv)!=gensym(",")) j++;
		if (i==j) {j++; continue;}
		t_arglist *al = (t_arglist *) malloc(sizeof(t_arglist) + (j-i)*sizeof(t_atom));
		al->c=j-i;
		copy_atoms(al->c,al->v,&argv[i]);
		//printf("#V reading '%s':\n",s->name);
		if (!newest) {error("#V: there is no newest object\n"); return;}
		t_visual *h = ((t_gobj *)newest)->dix->visual;
		if (h->exists(s)) {
			//printf("'%s' exists, deleting\n",s->name);
			free(h->get(s));
		}
		h->set(s,al);
		//fprintf(stderr,"visualloader... %p %d\n",newest,hash_size(h));
		j++;
		if (j<argc) {s=atom_getsymbolarg(j,argc,argv);j++;}
	}
	//printf("visualloader_anything end\n");
	gobj_changed(self,0);
}

extern "C" void glob_update_path ();

void glob_help(t_pd *bogus, t_symbol *s) {
        t_class *c = class_find(s);
        if (!c) {
		//post("help: no such class '%s'",s->name); return;
		t_binbuf *b = binbuf_new();
		binbuf_addv(b,"s",s);
		newest = 0;
		binbuf_eval(b,&pd_objectmaker,0,0);
		if (!newest) {post("help: no such class '%s'",s->name); return;}
		c = newest->_class;
		pd_free(newest);
	}
        const char *hn = class_gethelpname(c);
        char *buf;
        bool suffixed = strcmp(hn+strlen(hn)-3, ".pd")==0;
	asprintf(&buf,"%s%s",hn,suffixed?"":".pd");
        open_via_helppath(buf, c->externdir->name);
	free(buf);
}

extern "C" void glob_update_class_list (t_pd *self, t_symbol *cb_recv, t_symbol *cb_sel) {
    t_symbol *k; t_class *v;
    sys_gui("global class_list; set class_list {");
    hash_foreach(k,v,class_table) sys_vgui("%s ", ((t_symbol *)k)->name);
    sys_gui("}\n");
    sys_vgui("%s %s\n",cb_recv->name, cb_sel->name);
}

EXTERN t_class *glob_pdobject;

t_pd *pd_new2(int argc, t_atom *argv) {
	if (argv[0].a_type != A_SYMBOL) {error("pd_new2: start with symbol please"); return 0;}
	pd_typedmess(&pd_objectmaker,argv[0].a_symbol,argc-1,argv+1);
	return newest;
}
t_pd *pd_new3(const char *s) {
	t_binbuf *b = binbuf_new();
	binbuf_text(b,(char *)s,strlen(s));
	t_pd *self = pd_new2(binbuf_getnatom(b),binbuf_getvec(b));
	binbuf_free(b);
	return self;
}

extern "C" void boxes_init() {
    t_class *c;
    c =      boxes_class = class_new2("__boxes"     ,0/*boxes_new*/     ,     boxes_free,sizeof(t_boxes),CLASS_GOBJ,"");
    class_setnotice(c,t_notice(boxes_notice));
    c = gop_filtre_class = class_new2("__gop_filtre",0/*gop_filtre_new*/,gop_filtre_free,sizeof(t_boxes),CLASS_GOBJ,"");
    class_setnotice(c,t_notice(gop_filtre_notice));
}

static void desire_setup() {
    t_class *c;
    s_empty = gensym("empty");
    s_Pd = gensym("Pd");
    s_pd = gensym("pd");
    manager_class = class_new2("__manager",manager_new,manager_free,sizeof(t_manager),0,"*");
    class_addanything(manager_class,manager_anything);
    class_setnotice(manager_class,manager_notice);
    manager = manager_new(0,0,0);
#define S(x) x##_setup();
    S(vinlet) S(voutlet) S(g_array) S(g_canvas) S(g_scalar) S(g_template) S(g_traversal) S(g_text)
#undef S

    c = bng_class = class_new2("bng",bng_new,iemgui_free,sizeof(t_bng),0,"*");
    iemgui_subclass(c);
    class_addbang    (c, bng_bang);
    class_addfloat   (c, bng_bang2);
    class_addsymbol  (c, bng_bang2);
    class_addpointer (c, bng_bang2);
    class_addlist    (c, bng_bang2);
    class_addanything(c, bng_bang2);
    class_addmethod2(c,bng_reload,"reload","*");
    class_addmethod2(c,bng_loadbang,"loadbang","");
    class_addmethod2(c,bng_size,"size","*");
    class_addmethod2(c,bng_flashtime,"flashtime","*");
    class_addmethod2(c,iemgui_init,"init","f");
    class_setsavefn(c, (t_savefn)bng_savefn);
    class_sethelpsymbol(c, gensym("bng"));
    class_setfieldnames(c, "foo bar x1 y1 class w hold break isa snd rcv lab ldx ldy fstyle fs bcol fcol lcol");

    c = toggle_class = class_new2("tgl",toggle_new,iemgui_free,sizeof(t_toggle),0,"*");
    class_addcreator2("toggle",toggle_new,"*");
    iemgui_subclass(c);
    class_addbang(c, toggle_bang);
    class_addfloat(c, toggle_float);
    class_addmethod2(c,toggle_reload,"reload","*");
    class_addmethod2(c,toggle_loadbang,"loadbang","");
    class_addmethod2(c,toggle_set,"set","f");
    class_addmethod2(c,toggle_size,"size","*");
    class_addmethod2(c,iemgui_init,"init","f");
    class_addmethod2(c,toggle_nonzero,"nonzero","f");
    class_setsavefn(c, (t_savefn)toggle_savefn);
    class_sethelpsymbol(c, gensym("toggle"));

    c = radio_class = class_new2("radio",radio_new,iemgui_free,sizeof(t_radio),0,"*");
    iemgui_subclass(c);
    class_addbang(c, radio_bang);
    class_addfloat(c, radio_float);
    class_addmethod2(c,radio_reload, "reload","*");
    class_addmethod2(c,radio_loadbang, "loadbang","");
    class_addmethod2(c,radio_set, "set","f");
    class_addmethod2(c,radio_size, "size","f");
    class_addmethod2(c,iemgui_init, "init","f");
    class_addmethod2(c,radio_fout, "fout","f");
    class_addmethod2(c,radio_number, "number","f");
    class_addmethod2(c,radio_orient,"orient","f");
    class_addmethod2(c,radio_single_change, "single_change","");
    class_addmethod2(c,radio_double_change, "double_change","");
    sym_hdl = gensym("hdl"); sym_hradio = gensym("hradio");
    sym_vdl = gensym("vdl"); sym_vradio = gensym("vradio");
    class_setsavefn(c,(t_savefn)radio_savefn);
    class_sethelpsymbol(c, gensym("hradio"));
    class_addcreator2("hradio",radio_new,"*");
    class_addcreator2("vradio",radio_new,"*");
    class_addcreator2("hdl",radio_new,"*");
    class_addcreator2("vdl",radio_new,"*");
    class_addcreator2("rdb",radio_new,"*");
    class_addcreator2("radiobut",radio_new,"*");
    class_addcreator2("radiobutton",radio_new,"*");

    c = slider_class = class_new2("slider",slider_new,iemgui_free,sizeof(t_slider),0,"*");
    class_addcreator2("hslider",slider_new,"*");
    class_addcreator2("vslider",slider_new,"*");
    class_addcreator2("hsl"    ,slider_new,"*");
    class_addcreator2("vsl"    ,slider_new,"*");

    iemgui_subclass(c);
    class_addbang(c,slider_bang);
    class_addfloat(c,slider_float);
    class_addmethod2(c,slider_reload,"reload","*");
    class_addmethod2(c,slider_loadbang,"loadbang","");
    class_addmethod2(c,slider_set,"set","f");
    class_addmethod2(c,slider_size,"size","*");
    class_addmethod2(c,slider_range,"range","ff");
    class_addmethod2(c,slider_log,"log","");
    class_addmethod2(c,slider_lin,"lin","");
    class_addmethod2(c,iemgui_init,"init","f");
    class_addmethod2(c,slider_steady,"steady","f");
    class_addmethod2(c,slider_orient,"orient","f");
    sym_vsl     = gensym("vsl");
    sym_vslider = gensym("vslider");
    class_setsavefn(c,(t_savefn)slider_savefn);
    class_sethelpsymbol(c, gensym("hslider"));

    c = nbx_class = class_new2("nbx",nbx_new,iemgui_free,sizeof(t_nbx),0,"*");
    iemgui_subclass(c);
    class_addbang(c,nbx_bang);
    class_addfloat(c,nbx_float);
    class_addlist(c, nbx_list);
    class_addmethod2(c,nbx_reload,"reload","*");
    class_addmethod2(c,nbx_loadbang,"loadbang","");
    class_addmethod2(c,nbx_set,"set","f");
    class_addmethod2(c,nbx_size,"size","*");
    class_addmethod2(c,nbx_range,"range","*");
    class_addmethod2(c,nbx_log,"log","");
    class_addmethod2(c,nbx_lin,"lin","");
    class_addmethod2(c,iemgui_init,"init","f");
    class_addmethod2(c,nbx_log_height,"log_height","f");
    class_setsavefn(c,(t_savefn)nbx_savefn);
    class_sethelpsymbol(c, gensym("numbox2"));

    c = cnv_class = class_new2("cnv",cnv_new,iemgui_free,sizeof(t_cnv),CLASS_NOINLET,"*");
    class_addmethod2(c,cnv_reload,"reload","*");
    class_addmethod2(c,cnv_size,"size","*");
    class_addmethod2(c,cnv_vis_size,"vis_size","*");
    class_addmethod2(c,cnv_get_pos,"get_pos","");
    iemgui_subclass(c);
    class_setsavefn(c,(t_savefn)cnv_savefn);
    class_sethelpsymbol(c, gensym("my_canvas"));

    c = vu_class = class_new2("vu",vu_new,iemgui_free,sizeof(t_vu),0,"*");
    iemgui_subclass(c);
    class_addbang(c,vu_bang);
    class_addfloat(c,vu_float0);
    class_addmethod2(c,vu_float1,"ft1","f");
    class_addmethod2(c,vu_reload,"reload","*");
    class_addmethod2(c,vu_size,"size","*");
    class_addmethod2(c,vu_scale,"scale","F");
    class_setsavefn(c,(t_savefn)vu_savefn);
    class_sethelpsymbol(c, gensym("vu"));

    visualloader_class = class_new2("#V",visualloader_new,visualloader_free,sizeof(t_object),CLASS_GOBJ,"*");
    class_addanything(visualloader_class,visualloader_anything);
    pd_bind(pd_new(visualloader_class),gensym("#V"));
}

/* ---------------------------------------------------------------- */
/* formerly m_glob.c */

t_class *glob_pdobject;
static t_class *maxclass;

#define IGN(sym) if (s==gensym(sym)) return;
void max_default(t_pd *x, t_symbol *s, int argc, t_atom *argv) {
    IGN("audioindev");
    IGN("audiooutdev");
    IGN("audioininfo");
    IGN("audiooutinfo");
    IGN("testaudiosettingresult");
    IGN("audiodevice");
    IGN("xrun");
    IGN("audio_started");
    IGN("sys_lock_timeout");
    IGN("midiindev");
    IGN("midioutdev");
    IGN("midicurrentindev");
    IGN("midicurrentoutdev");
    IGN("audiocurrentininfo");
    IGN("audiocurrentoutinfo");
    IGN("asiolatency");
    startpost("%s: unknown message %s ", class_getname(pd_class(x)), s->name);
    std::ostringstream buf;
    for (int i = 0; i < argc; i++) {buf << " "; atom_ostream(argv+i,buf);}
    post("%s",buf.str().data());
    endpost();
}

static void openit(const char *dirname, const char *filename) {
    char *dirbuf;
    char *nameptr;
    int fd = open_via_path2(dirname,filename,"",&dirbuf,&nameptr,0);
    if (fd<0) {error("%s: can't open", filename); return;}
    close(fd);
    glob_evalfile(0, gensym(nameptr), gensym(dirbuf));
    free(dirbuf);
}

extern "C" t_socketreceiver *netreceive_newest_receiver(t_text *x);

/* this should be rethought for multi-client */
void glob_initfromgui(void *dummy, t_symbol *s) {
    char buf[256], buf2[256];
    char cwd[666];
    getcwd(cwd,665);
    sys_socketreceiver=netreceive_newest_receiver(sys_netreceive);
    sys_vgui("%s",lost_posts.str().data());
    /* load dynamic libraries specified with "-lib" args */
    for (t_namelist *nl=sys_externlist; nl; nl = nl->nl_next)
        if (!sys_load_lib(0, nl->nl_string))
            post("%s: can't load library", nl->nl_string);
    /* open patches specified with "-open" args */
    for  (t_namelist *nl=sys_openlist; nl; nl = nl->nl_next) openit(cwd, nl->nl_string);
    namelist_free(sys_openlist);
    sys_openlist = 0;
    /* send messages specified with "-send" args */
    for (t_namelist *nl=sys_messagelist; nl; nl = nl->nl_next) {
        t_binbuf *b = binbuf_new();
        binbuf_text(b, nl->nl_string, strlen(nl->nl_string));
        binbuf_eval(b, 0, 0, 0);
        binbuf_free(b);
    }
    namelist_free(sys_messagelist);
    sys_messagelist = 0;
    sys_get_audio_apis(buf);
    sys_get_midi_apis(buf2);
    sys_vgui("pd_startup {%s} %s %s\n", pd_version, buf, buf2);
/*
    fprintf(stdout,"This line was printed on stdout\n");
    fprintf(stderr,"This line was printed on stderr\n");
*/
}

void glob_meters(void *dummy, t_floatarg f);
void glob_audiostatus(void *dummy);
void glob_audio_properties(t_pd *dummy, t_floatarg flongform);
void glob_audio_dialog(t_pd *dummy, t_symbol *s, int argc, t_atom *argv);
void glob_audio_setapi(t_pd *dummy, t_floatarg f);
void glob_midi_properties(t_pd *dummy, t_floatarg flongform);
void glob_midi_dialog(t_pd *dummy, t_symbol *s, int argc, t_atom *argv);
void glob_midi_setapi(t_pd *dummy, t_floatarg f);
void glob_start_path_dialog(t_pd *dummy, t_floatarg flongform);
void glob_path_dialog(t_pd *dummy, t_symbol *s, int argc, t_atom *argv);
void glob_start_startup_dialog(t_pd *dummy, t_floatarg flongform);
void glob_startup_dialog(t_pd *dummy, t_symbol *s, int argc, t_atom *argv);
void glob_ping(t_pd *dummy);
extern "C" {
void glob_finderror(t_pd *dummy);
};
/* tb: message-based audio configuration { */
void glob_audio_testaudiosetting(t_pd * dummy, t_symbol *s, int ac, t_atom *av);
void glob_audio_getaudioindevices(t_pd * dummy, t_symbol *s, int ac, t_atom *av);
void glob_audio_getaudiooutdevices(t_pd * dummy, t_symbol *s, int ac, t_atom *av);
void glob_audio_getaudioininfo(t_pd * dummy, t_float f);
void glob_audio_getaudiooutinfo(t_pd * dummy, t_float f);
//void glob_audio_samplerate(t_pd * dummy, t_float f);
//void glob_audio_delay(t_pd * dummy, t_float f);
//void glob_audio_dacblocksize(t_pd * dummy, t_float f);
//void glob_audio_scheduler(t_pd * dummy, t_float f);
void glob_audio_device(t_pd * dummy, t_symbol *s, int argc, t_atom *argv);
//void glob_audio_device_in(t_pd * dummy, t_symbol *s, int argc, t_atom *argv);
//void glob_audio_device_out(t_pd * dummy, t_symbol *s, int argc, t_atom *argv);
void glob_audio_getcurrent_devices ();
void glob_audio_asio_latencies(t_pd * dummy, t_float f);
void glob_midi_getindevs( t_pd *dummy, t_symbol *s, int ac, t_atom *av);
void glob_midi_getoutdevs(t_pd *dummy, t_symbol *s, int ac, t_atom *av);
void glob_midi_getcurrentindevs(t_pd *dummy);
void glob_midi_getcurrentoutdevs(t_pd *dummy);
/* tb } */

static void glob_object_table() {
	t_symbol *s_inlet = gensym("inlet");
	t_symbol *s___list = gensym("__list");
	size_t inlets=0, lists=0, zombies=0;
	t_pd *k; long v;
	post("object_table = {");
	hash_foreach(k,v,object_table) {
		t_pd *x = (t_pd *)k;
		if (!(long)v&1) {zombies++; continue;} /* skip zombies */
		//post("  %p %ld %s",k,(long)v,x->_class->name->name);
		if (x->_class->name == s_inlet) {inlets++; continue;}
		if (x->_class->name == s___list) {lists++; continue;}
		int nobs = x->_class->gobj ? ((t_gobj *)x)->dix->nobs : 0;
		// this has been duplicated as the ostream operator of t_pd * (see above).
		t_binbuf *b;
		if (x->_class->patchable && (b = ((t_text *)x)->binbuf)) {
			char *buf; int bufn;
			binbuf_gettext(b,&buf,&bufn);
			post("  %p %ld (%dobs) %s [%.*s]",k,(long)v,nobs,x->_class->name->name,bufn,buf);
		} else post("  %p %ld (%dobs) %s",k,(long)v,nobs,x->_class->name->name);
	}
	post("} (%ld non-omitted objects, plus %ld [inlet], plus %ld [__list], plus %ld zombies)",
		object_table->size()-inlets-lists-zombies,inlets,lists,zombies);
}

extern t_class *glob_pdobject;
extern "C" void glob_init () {
    /* can this one really be called "max"? isn't that a name conflict? */
    maxclass = class_new2("max",0,0,sizeof(t_pd),CLASS_DEFAULT,"");
    class_addanything(maxclass, max_default);
    pd_bind((t_pd *)&maxclass, gensym("max"));

    /* this smells bad... a conflict with [pd] subpatches */
    t_class *c = glob_pdobject = class_new2("pd",0,0,sizeof(t_pd),CLASS_DEFAULT,"");
    class_addmethod2(c,glob_initfromgui, "init", "*");
    class_addmethod2(c,glob_setfilename, "filename", "ss");
    class_addmethod2(c,glob_evalfile, "open", "ss");
    class_addmethod2(c,glob_quit, "quit", "");
    class_addmethod2(c,glob_dsp, "dsp", "*");
    class_addmethod2(c,glob_meters, "meters", "f");
    class_addmethod2(c,glob_audiostatus, "audiostatus", "");
    class_addmethod2(c,glob_finderror, "finderror", "");
    class_addmethod2(c,glob_audio_properties, "audio-properties", "F");
    class_addmethod2(c,glob_audio_dialog,     "audio-dialog", "*");
    class_addmethod2(c,glob_audio_setapi,     "audio-setapi", "f");
    class_addmethod2(c,glob_midi_setapi,      "midi-setapi", "f");
    class_addmethod2(c,glob_midi_properties,  "midi-properties", "F");
    class_addmethod2(c,glob_midi_dialog,      "midi-dialog", "*");
    class_addmethod2(c,glob_ping, "ping","");
    /* tb: message-based audio configuration { */
//    class_addmethod2(c,glob_audio_samplerate, "audio-samplerate", "F");
//    class_addmethod2(c,glob_audio_delay,      "audio-delay", "F");
//    class_addmethod2(c,glob_audio_dacblocksize,"audio-dacblocksize", "F");
//    class_addmethod2(c,glob_audio_scheduler,  "audio-scheduler", "F");
    class_addmethod2(c,glob_audio_device,       "audio-device", "*");
//    class_addmethod2(c,glob_audio_device_in,  "audio-device-in", "*");
//    class_addmethod2(c,glob_audio_device_out, "audio-device-out", "*");
    class_addmethod2(c,glob_audio_getaudioindevices, "getaudioindev", "*");
    class_addmethod2(c,glob_audio_getaudiooutdevices,"getaudiooutdev", "*");
    class_addmethod2(c,glob_audio_getaudioininfo,    "getaudioininfo", "f");
    class_addmethod2(c,glob_audio_getaudiooutinfo,   "getaudiooutinfo", "f");
    class_addmethod2(c,glob_audio_testaudiosetting,  "testaudiosetting", "*");
    class_addmethod2(c,glob_audio_getcurrent_devices,"getaudiodevice", "");
    class_addmethod2(c,glob_audio_asio_latencies,    "getasiolatencies", "F");
    class_addmethod2(c,glob_midi_getoutdevs,"getmidioutdev", "*");
    class_addmethod2(c,glob_midi_getindevs, "getmidiindev", "*");
    class_addmethod2(c,glob_midi_getcurrentoutdevs, "getmidicurrentoutdev", "");
    class_addmethod2(c,glob_midi_getcurrentindevs, "getmidicurrentindev", "");
    /* tb } */
#ifdef UNIX
    class_addmethod2(c,glob_watchdog, "watchdog", "");
#endif
    class_addmethod2(c,glob_update_class_list, "update-class-list", "ss");
    class_addmethod2(c,glob_update_class_info, "update-class-info", "sss");
    class_addmethod2(c,glob_update_path,       "update-path", "");
    class_addmethod2(c,glob_help,              "help", "s");
    class_addmethod2(c,glob_object_table,"object_table","");
    class_addanything(c, max_default);
    pd_bind((t_pd *)&glob_pdobject, gensym("pd"));
}

/* ---------------------------------------------------------------- */
/* formerly s_print.c */

t_printhook sys_printhook;
FILE *sys_printtofh = 0; /* send to console by default */

static void dopost(const char *s) {
    if (sys_printhook) sys_printhook(s);
    else if (sys_printtofh) fprintf(sys_printtofh, "%s", s);
    else {
        std::ostringstream t;
        for(int i=0; s[i]; i++) {
            if (strchr("\\\"[]$\n",s[i])) t << '\\';
	    t << char(s[i]=='\n' ? 'n' : s[i]);
        }
        sys_vgui("pdtk_post \"%s\"\n",t.str().data());
    }
}

void      post(const char *fmt, ...) {
    char *buf; va_list ap; va_start(ap, fmt);
    size_t n = vasprintf(&buf, fmt, ap); va_end(ap);
    buf=(char*)realloc(buf,n+2); strcpy(buf+n,"\n");
    dopost(buf); free(buf);
}
void startpost(const char *fmt, ...) {
    char *buf; va_list ap; va_start(ap, fmt);
    vasprintf(&buf, fmt, ap); va_end(ap);
    dopost(buf); free(buf);
}

void poststring(const char *s) {dopost(" "); dopost(s);}

void postatom(int argc, t_atom *argv) {
    std::ostringstream buf;
    for (int i=0; i<argc; i++) {buf << " "; atom_ostream(argv+i,buf);}
    dopost(buf.str().data());
}

/* what's the point? */
void postfloat(float f) {t_atom a; SETFLOAT(&a, f); postatom(1, &a);}
void endpost () {dopost("\n");}

static t_pd *error_object;
static char *error_string;
void canvas_finderror(void *object);

void verror(const char *fmt, va_list ap) {
    if (error_string) free(error_string);
    dopost("error: ");
    vasprintf(&error_string,fmt,ap);
    dopost(error_string);
    dopost("\n");
    //post("at stack level %ld\n",pd_stackn);
    error_object = pd_stackn>=0 ? pd_stack[pd_stackn-1].self : 0;
}
void error(               const char *fmt, ...) {va_list ap; va_start(ap,fmt); verror(fmt,ap); va_end(ap);}
void pd_error(void *moot, const char *fmt, ...) {va_list ap; va_start(ap,fmt); verror(fmt,ap); va_end(ap);}

void verbose(int level, const char *fmt, ...) {
    char *buf;
    va_list ap;
    if (level>sys_verbose) return;
    dopost("verbose(");
    postfloat((float)level);
    dopost("):");
    va_start(ap, fmt);
    vasprintf(&buf,fmt,ap);
    va_end(ap);
    dopost(buf);
    dopost("\n");
}

extern "C" void glob_finderror(t_pd *dummy) {
    if (!error_object) {post("no findable error yet."); return;}
    post("last trackable error was for object x%lx: %s", error_object, error_string);
    sys_mgui(error_object,"show_error","S",error_string);
    canvas_finderror(error_object);
}

void bug(const char *fmt, ...) {
    char *buf;
    va_list ap;
    dopost("bug: ");
    va_start(ap, fmt);
    vasprintf(&buf,fmt,ap);
    va_end(ap);
    dopost(buf);
    free(buf);
    dopost("\n");
}

static const char *errobject;
static const char *errstring;

void sys_logerror (const char *object, const char *s){errobject=object; errstring = s;}
void sys_unixerror(const char *object)               {errobject=object; errstring = strerror(errno);}

void sys_ouch () {
    if (*errobject) error("%s: %s", errobject, errstring); else error("%s", errstring);
}

/* properly close all open root canvases */
extern "C" void glob_closeall(void *dummy, t_floatarg fforce) {
    foreach(x,windowed_canvases) canvas_close(x->first);
}

/* ---------------------------------------------------------------- */
/* formerly m_conf.c */

void builtins_setup ();
void builtins_dsp_setup ();
void desire_setup ();
void d_soundfile_setup ();
void d_ugen_setup ();

extern "C" void conf_init () {
    builtins_setup();
    builtins_dsp_setup();
    desire_setup();
    d_soundfile_setup();
    d_ugen_setup();
}

// and just to make some externs happy:
#define BYE error("%s unimplemented in desiredata!", __PRETTY_FUNCTION__);
extern "C" {
  void glist_grab () {BYE}
  void glist_xtopixels () {BYE}
  void glist_ytopixels () {BYE}
  void *glist_findrtext () {BYE return 0;}
  void canvas_fixlinesfor () {BYE}
  void class_setpropertiesfn () {BYE}
  void glist_eraseiofor () {BYE}
  void glist_getcanvas () {BYE}
  void rtext_gettag () {BYE}
  void class_setwidget () {BYE}
  void glist_isvisible () {BYE}
  void gobj_vis () {BYE}
  void gfxstub_deleteforkey () {BYE}
  void gfxstub_new () {BYE}

  //redundantwards-compatibility
  void canvas_setcurrent  (t_canvas *x) {pd_pushsym(x);}
  void canvas_unsetcurrent(t_canvas *x)  {pd_popsym(x);}
};
