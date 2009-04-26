/* $Id: kernel.c,v 1.1.2.92 2007/09/09 21:34:56 matju Exp $
 * Copyright 2006-2007 Mathieu Bouchard.
 * Copyright (c) 1997-2006 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution. */

/* IOhannes :
 * changed the canvas_restore in "g_canvas.c", so that it might accept $args as well (like "pd $0_test")
 * so you can make multiple & distinguishable templates
 * 1511:forum::f�r::uml�ute:2001
 * change marked with IOhannes
 */

#define PD_PLUSPLUS_FACE
#include "desire.h"
#include "m_simd.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sstream>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#ifdef UNISTD
#include <unistd.h>
#endif
#ifdef MSW
#include <io.h>
#endif

#define a_float    a_w.w_float
#define a_symbol   a_w.w_symbol
#define a_gpointer a_w.w_gpointer
#define a_index    a_w.w_index

using namespace std;

/* T.Grill - bit alignment for signal vectors (must be a multiple of 8!) */
/* if undefined no alignment occurs */
#ifdef SIMD_BYTEALIGN
    #define VECTORALIGNMENT (SIMD_BYTEALIGN*8)
#else
    #define VECTORALIGNMENT 128
#endif

void *getbytes(size_t nbytes) {
    if (nbytes < 1) nbytes = 1;
    void *ret = (void *)calloc(nbytes, 1);
    if (!ret) error("pd: getbytes() failed -- out of memory");
    return ret;
}

void *copybytes(void *src, size_t nbytes) {
    void *ret = getbytes(nbytes);
    if (nbytes) memcpy(ret, src, nbytes);
    return ret;
}

void *resizebytes(void *old, size_t oldsize, size_t newsize) {
    if (newsize < 1) newsize = 1;
    if (oldsize < 1) oldsize = 1;
    void *ret = (void *)realloc((char *)old, newsize);
    if (newsize > oldsize && ret) memset(((char *)ret) + oldsize, 0, newsize - oldsize);
    if (!ret) error("pd: resizebytes() failed -- out of memory");
    return ret;
}

void freebytes(void *old, size_t nbytes) {free(old);}

/* in the following size_t is assumed to have the same size as a pointer type !!! */

/* T.Grill - get aligned memory */
void *getalignedbytes(size_t nbytes) {
    /* to align the region we also need some extra memory to save the original pointer location
       it is saved immediately before the aligned vector memory */
    void *vec = getbytes(nbytes+(VECTORALIGNMENT/8-1)+sizeof(void *));
    if (!vec) return 0;
    t_int alignment = ((t_int)vec+sizeof(void *))&(VECTORALIGNMENT/8-1);
    void *ret = (unsigned char *)vec+sizeof(void *)+(alignment == 0?0:VECTORALIGNMENT/8-alignment);
    *(void **)((unsigned char *)ret-sizeof(void *)) = vec;
    return ret;
}

/* T.Grill - free aligned vector memory */
void freealignedbytes(void *ptr,size_t nbytes) {
	free(*(void **)((unsigned char *)ptr-sizeof(void *)));
}

/* T.Grill - resize aligned vector memory */
void *resizealignedbytes(void *ptr,size_t oldsize, size_t newsize) {
	if (newsize<1) newsize=1;
	void *ori = *(void **)((unsigned char *)ptr-sizeof(void *));
	void *vec = realloc(ori,newsize+(VECTORALIGNMENT/8-1)+sizeof(void *));
	t_int alignment = ((t_int)vec+sizeof(void *))&(VECTORALIGNMENT/8-1);
	void *ret = (unsigned char *)vec+sizeof(void *)+(alignment == 0?0:VECTORALIGNMENT/8-alignment);
	*(void **)((unsigned char *)ret-sizeof(void *)) = vec;
	return ret;
}

/* TB: copy to aligned vector memory */
void *copyalignedbytes(void *src, size_t nbytes) {
    void *ret = getalignedbytes(nbytes);
    if (nbytes) memcpy(ret, src, nbytes);
    return ret;
}

t_class *hash_class;

/*extern "C"*/ void hash_setup () {
  hash_class = class_new(gensym("#V"), (t_newmethod)0 /*hash_new*/,
	0 /*(t_method)hash_free*/, sizeof(t_object), CLASS_PD, A_GIMME, 0);
}

/* convenience routines for checking and getting values of atoms.
   There's no "pointer" version since there's nothing safe to return if there's an error. */

t_float     atom_getfloat( t_atom *a) {return a->a_type==A_FLOAT ? a->a_float : 0;}
t_int       atom_getint(   t_atom *a) {return (t_int)atom_getfloat(a);}
t_symbol *  atom_getsymbol(t_atom *a) {return a->a_type==A_SYMBOL ? a->a_symbol : &s_symbol;}
const char *atom_getstring(t_atom *a) {return atom_getsymbol(a)->name;}

t_symbol *atom_gensym(t_atom *a) { /* this works better for graph labels */
    if (a->a_type == A_SYMBOL) return a->a_symbol;
    if (a->a_type == A_FLOAT) {char buf[30]; sprintf(buf, "%g", a->a_float); return gensym(buf);}
    return gensym("???");
}

t_float atom_getfloatarg(int which, int argc, t_atom *argv) {
    if (argc <= which) return 0;
    argv += which;
    return argv->a_type==A_FLOAT ? argv->a_float : 0;
}

t_int atom_getintarg(int which, int argc, t_atom *argv)
{return (t_int)atom_getfloatarg(which, argc, argv);}

t_symbol *atom_getsymbolarg(int which, int argc, t_atom *argv) {
    if (argc <= which) return &s_;
    argv += which;
    return argv->a_type==A_SYMBOL ? argv->a_symbol : &s_;
}

const char *atom_getstringarg(int which, int argc, t_atom *argv) {
    return atom_getsymbolarg(which,argc,argv)->name;
}

/* convert an atom into a string, in the reverse sense of binbuf_text (q.v.)
   special attention is paid to symbols containing the special characters
   ';', ',', '$', and '\'; these are quoted with a preceding '\', except that
   the '$' only gets quoted at the beginning of the string. */

//static int should_quote(char *s) {return strchr(";,\\{}\"",*s) || isspace(*s) || (*s=='$' && isdigit(s[1]));}
static int should_quote(char *s) {return strchr(";,\\{}\" ",*s) || (*s=='$' && isdigit(s[1]));}

void atom_ostream(t_atom *a, ostream &buf) {
    switch(a->a_type) {
    case A_SEMI:    buf << ";"; break;
    case A_COMMA:   buf << ","; break;
    case A_POINTER: buf << "(pointer)"; break;
    case A_FLOAT:   buf << a->a_float; break;
    case A_SYMBOL: {
        bool quote=0;
        for (char *sp = a->a_symbol->name; *sp; sp++) if (should_quote(sp)) {quote = 1; break;}
        if (quote) {
            for (char *sp = a->a_symbol->name; *sp; sp++) {
                if (should_quote(sp)) buf << '\\';
                buf << *sp;
            }
        } else buf << a->a_symbol->name;
    } break;
    case A_DOLLAR:  buf << "$" << a->a_index; break;
    case A_DOLLSYM: buf << a->a_symbol->name; break;
    default: bug("%s",__PRETTY_FUNCTION__);
    }
}

/* this is not completely compatible with Miller's, as it won't do anything special for short bufsizes. */
void atom_string(t_atom *a, char *buf, unsigned int bufsize) {
     ostringstream b;
     atom_ostream(a,b);
     strncpy(buf,b.str().data(),bufsize);
     buf[bufsize-1]=0;
}

void atom_init(t_atom *a, size_t n) {
	for (size_t i=0; i<n; i++) {
		a[i].a_type = A_FLOAT;
		a[i].a_float = 0.0;
	}
}

void atom_copy(t_atom *a, t_atom *b, size_t n) {
	memcpy(a,b,n*sizeof(t_atom));
	/* here I should handle incref */
}

void atom_delete(t_atom *a, size_t n) {
	/* here I should handle decref */
}

/* in which the value has bit 0 set if the key object is not a zombie,
   and has bit 1 set if the object has been uploaded to the client */
t_hash<t_pd *,long> *object_table;

t_pd *pd_new(t_class *c) {
    if (!c) bug("pd_new: apparently called before setup routine");
    t_pd *x = (t_pd *)getbytes(c->size);
    x->_class = c;
    object_table->set(x,1);
    if (c->gobj) ((t_gobj *)x)->g_adix = appendix_new((t_gobj *)x);
    if (c->patchable) {
        ((t_object *)x)->inlet = 0;
        ((t_object *)x)->outlet = 0;
    }
    return x;
}

void pd_free_zombie(t_pd *x) {
	t_class *c = x->_class;
	if (c->gobj) appendix_free((t_gobj *)x);
	if (c->size) free(x);
	object_table->del(x);
}

void pd_free(t_pd *x) {
    t_class *c = x->_class;
    if (c->freemethod) ((t_gotfn)(c->freemethod))(x);
    if (c->patchable) {
	t_object *y = (t_object *)x;
	while (y->outlet) outlet_free(y->outlet);
	while (y-> inlet)  inlet_free(y-> inlet);
	if (y->binbuf)    binbuf_free(y->binbuf);
    }
    /* schedule for deletion if need to keep the allocation around */
    if (c->gobj && (object_table->get(x)&2)) {
	object_table->set(x,object_table->get(x)&~1);
	gobj_changed((t_gobj *)x,"");
	//char *xx = (char *)x; for (int i=0; i<c->size; i++) xx[i]="\xde\xad\xbe\xef"[i&3];
    } else pd_free_zombie(x);
}

void gobj_save(t_gobj *x, t_binbuf *b) {
    t_class *c = x->g_pd;
    if (c->savefn) c->savefn(x, b);
}

/* deal with several objects bound to the same symbol.  If more than one,
   we actually bind a collection object to the symbol, which forwards messages sent to the symbol. */

static t_class *bindlist_class;

struct t_bindelem {
    t_pd *who;
    t_bindelem *next;
};

struct t_bindlist : t_pd {
    t_bindelem *list;
};

#define bind_each(e,x) for (t_bindelem *e = x->list; e; e = e->next)
static void bindlist_bang    (t_bindlist *x)              {bind_each(e,x)   pd_bang(e->who);}
static void bindlist_float   (t_bindlist *x, t_float f)   {bind_each(e,x)  pd_float(e->who,f);}
static void bindlist_symbol  (t_bindlist *x, t_symbol *s) {bind_each(e,x) pd_symbol(e->who,s);}
static void bindlist_pointer (t_bindlist *x, t_gpointer *gp)                      {bind_each(e,x)   pd_pointer(e->who, gp);}
static void bindlist_list    (t_bindlist *x, t_symbol *s, int argc, t_atom *argv) {bind_each(e,x)      pd_list(e->who, s,argc,argv);}
static void bindlist_anything(t_bindlist *x, t_symbol *s, int argc, t_atom *argv) {bind_each(e,x) pd_typedmess(e->who, s,argc,argv);}

static t_bindelem *bindelem_new(t_pd *who, t_bindelem *next) {
	t_bindelem *self = (t_bindelem *)malloc(sizeof(t_bindelem));
	self->who = who;
	self->next = next;
	return self;
}

void pd_bind(t_pd *x, t_symbol *s) {
    if (s->thing) {
        if (s->thing->_class == bindlist_class) {
            t_bindlist *b = (t_bindlist *)s->thing;
            b->list = bindelem_new(x,b->list);
        } else {
            t_bindlist *b = (t_bindlist *)pd_new(bindlist_class);
            b->list = bindelem_new(x,bindelem_new(s->thing,0));
            s->thing = b;
        }
    } else s->thing = x;
}

/* bindlists always have at least two elements... if the number
   goes down to one, get rid of the bindlist and bind the symbol
   straight to the remaining element. */
void pd_unbind(t_pd *x, t_symbol *s) {
    if (s->thing == x) {s->thing = 0; return;}
    if (s->thing && s->thing->_class == bindlist_class) {
        t_bindlist *b = (t_bindlist *)s->thing;
        t_bindelem *e, *e2;
        if ((e = b->list)->who == x) {
            b->list = e->next;
            free(e);
        } else for (e = b->list; (e2=e->next); e = e2) if (e2->who == x) {
            e->next = e2->next;
            free(e2);
            break;
        }
        if (!b->list->next) {
            s->thing = b->list->who;
            free(b->list);
            pd_free(b);
        }
    } else error("%s: couldn't unbind", s->name);
}

t_pd *pd_findbyclass(t_symbol *s, t_class *c) {
    t_pd *x = 0;
    if (!s->thing) return 0;
    if (s->thing->_class == c) return s->thing;
    if (s->thing->_class == bindlist_class) {
        t_bindlist *b = (t_bindlist *)s->thing;
        int warned = 0;
        bind_each(e,b) if (e->who->_class == c) {
            if (x && !warned) {post("warning: %s: multiply defined", s->name); warned = 1;}
            x = e->who;
        }
    }
    return x;
}

/* stack for maintaining bindings for the #X symbol during nestable loads. */

#undef g_next

struct t_gstack {
    t_pd *what;
    t_symbol *loading_abstr;
    t_gstack *next;
    long base_o_index;
};

static t_gstack *gstack_head = 0;
static t_pd *lastpopped;
static t_symbol *pd_loading_abstr;

int pd_setloadingabstraction(t_symbol *sym) {
    t_gstack *foo = gstack_head;
    for (foo = gstack_head; foo; foo = foo->next) if (foo->loading_abstr == sym) return 1;
    pd_loading_abstr = sym;
    return 0;
}
 
int gstack_empty() {return !gstack_head;}

long canvas_base_o_index() {
    return gstack_head ? gstack_head->base_o_index : 0;
}

void pd_pushsym(t_pd *x) {
    t_gstack *y = (t_gstack *)malloc(sizeof(*y));
    y->what = s__X.thing;
    y->next = gstack_head;
    y->loading_abstr = pd_loading_abstr;
    y->base_o_index = x->_class == canvas_class ? ((t_canvas *)x)->next_o_index : -666;
    pd_loading_abstr = 0;
    gstack_head = y;
    s__X.thing = x;
}

void pd_popsym(t_pd *x) {
    if (!gstack_head || s__X.thing != x) {bug("gstack_pop"); return;}
    t_gstack *headwas = gstack_head;
    s__X.thing = headwas->what;
    gstack_head = headwas->next;
    free(headwas);
    lastpopped = x;
}

static void stackerror(t_pd *x) {error("stack overflow");}

/* to enable multithreading, make those variables "thread-local". this means that they have to go in
   a thread-specific place instead of plain global. do not ever use tim's atomic counters for this,
   as they count all threads together as if they're one, and they're especially incompatible with
   use of the desiredata-specific stack[] variable. */
int pd_stackn = 0; /* how much of the stack is in use */
t_call pd_stack[STACKSIZE];

static inline uint64 rdtsc() {uint64 x; __asm__ volatile (".byte 0x0f, 0x31":"=A"(x)); return x;}

//#define PROFILER
#ifdef PROFILER
#define ENTER_PROF uint64 t = rdtsc();
#define LEAVE_PROF if (x->_class->gobj && ((t_gobj *)x)->dix) ((t_gobj *)x)->dix->elapsed += rdtsc() - t;
#else
#define ENTER_PROF
#define LEAVE_PROF
#endif

#define ENTER(SELECTOR) if(pd_stackn >= STACKSIZE) {stackerror(x); return;} \
	pd_stack[pd_stackn].self = x; pd_stack[pd_stackn].s = SELECTOR; pd_stackn++; ENTER_PROF
#define LEAVE pd_stackn--; LEAVE_PROF

/* matju's 2007.07.14 inlet-based stack check needs to be implemented in:
     pd_bang pd_float pd_pointer pd_symbol pd_string pd_list pd_typedmess */
void pd_bang(t_pd *x)                    {ENTER(&s_bang);    x->_class->bangmethod(x);       LEAVE;}
void pd_float(t_pd *x, t_float f)        {ENTER(&s_float);   x->_class->floatmethod(x,f);    LEAVE;}
void pd_pointer(t_pd *x, t_gpointer *gp) {ENTER(&s_pointer); x->_class->pointermethod(x,gp); LEAVE;}
void pd_symbol(t_pd *x, t_symbol *s)     {ENTER(&s_symbol);  x->_class->symbolmethod(x,s);   LEAVE;}
/* void pd_string(t_pd *x, const char *s){ENTER(&s_symbol); x->_class->stringmethod(x,s);   LEAVE;} future use */
void pd_list(t_pd *x, t_symbol *s, int ac, t_atom *av) {ENTER(s); x->_class->listmethod(x,&s_list,ac,av); LEAVE;}

/* this file handles Max-style patchable objects, i.e., objects which
can interconnect via inlets and outlets; also, the (terse) generic
behavior for "gobjs" appears at the end of this file.  */

union inletunion {
    t_symbol *symto;
    t_gpointer *pointerslot;
    t_float *floatslot;
    t_symbol **symslot;
    t_sample floatsignalvalue;
};

struct _inlet : t_pd {
    struct _inlet *next;
    t_object *owner;
    t_pd *dest;
    t_symbol *symfrom;
    union inletunion u;
    t_symbol* tip;
};

static t_class *inlet_class, *pointerinlet_class, *floatinlet_class, *symbolinlet_class;

#define ISINLET(pd) ( \
    pd->_class == inlet_class || \
    pd->_class == pointerinlet_class || \
    pd->_class == floatinlet_class || \
    pd->_class == symbolinlet_class)

/* --------------------- generic inlets ala max ------------------ */

static void object_append_inlet(t_object *owner, t_inlet *x) {
    t_inlet *y = owner->inlet, *y2;
    if (y) {
        while ((y2 = y->next)) y = y2;
        y->next = x;
    } else owner->inlet = x;
}

t_inlet *inlet_new(t_object *owner, t_pd *dest, t_symbol *s1, t_symbol *s2) {
    t_inlet *x = (t_inlet *)pd_new(inlet_class);
    x->owner = owner;
    x->dest = dest;
    if (s1 == &s_signal) x->u.floatsignalvalue = 0; else x->u.symto = s2;
    x->symfrom = s1;
    x->next = 0;
    x->tip = gensym("?");
    object_append_inlet(owner,x);
    return x;
}

t_inlet *signalinlet_new(t_object *owner, t_float f) {
    t_inlet *x = inlet_new(owner, owner, &s_signal, &s_signal);
    x->u.floatsignalvalue = f;
    return x;
}

static void inlet_wrong(t_inlet *x, t_symbol *s) {
    error("inlet: expected '%s' but got '%s'", x->symfrom->name, s->name);
}

void inlet_settip(t_inlet* i,t_symbol* s) {i->tip = s;}

const char *inlet_tip(t_inlet* i,int num) {
  if (num < 0) return "???";
  while (num-- && i) i = i->next;
  if (i && i->tip) return i->tip->name;
  return "?";
}

/* LATER figure out how to make these efficient: */
static void inlet_bang(t_inlet *x) {
    if (x->symfrom == &s_bang) pd_vmess(x->dest, x->u.symto, "");
    else if (!x->symfrom) pd_bang(x->dest);
    else inlet_wrong(x, &s_bang);
}
static void inlet_pointer(t_inlet *x, t_gpointer *gp) {
    if (x->symfrom == &s_pointer) pd_vmess(x->dest, x->u.symto, "p", gp);
    else if (!x->symfrom) pd_pointer(x->dest, gp);
    else inlet_wrong(x, &s_pointer);
}
static void inlet_float(t_inlet *x, t_float f) {
    if (x->symfrom == &s_float) pd_vmess(x->dest, x->u.symto, "f", (t_floatarg)f);
    else if (x->symfrom == &s_signal) x->u.floatsignalvalue = f;
    else if (!x->symfrom) pd_float(x->dest, f);
    else inlet_wrong(x, &s_float);
}

static void inlet_symbol(t_inlet *x, t_symbol *s) {
    if (x->symfrom == &s_symbol) pd_vmess(x->dest, x->u.symto, "s", s);
    else if (!x->symfrom) pd_symbol(x->dest, s);
    else inlet_wrong(x, &s_symbol);
}

static void inlet_list(t_inlet *x, t_symbol *s, int argc, t_atom *argv) {
    if (x->symfrom == &s_list || x->symfrom == &s_float || x->symfrom == &s_symbol || x->symfrom == &s_pointer)
            typedmess(x->dest, x->u.symto, argc, argv);
    else if (!x->symfrom) pd_list(x->dest, s, argc, argv);
    else inlet_wrong(x, &s_list);
}

static void inlet_anything(t_inlet *x, t_symbol *s, int argc, t_atom *argv) {
    if (x->symfrom == s) typedmess(x->dest, x->u.symto, argc, argv);
    else if (!x->symfrom) typedmess(x->dest, s, argc, argv);
    else inlet_wrong(x, s);
}

void inlet_free(t_inlet *x) {
    t_object *y = x->owner;
    if (y->inlet == x) y->inlet = x->next;
    else for (t_inlet *x2 = y->inlet; x2; x2 = x2->next) if (x2->next == x) {
        x2->next = x->next;
        break;
    }
    pd_free(x);
}

/* ----- pointerinlets, floatinlets, syminlets: optimized inlets ------- */

static void pointerinlet_pointer(t_inlet *x, t_gpointer *gp) {
    gpointer_unset(x->u.pointerslot);
    *(x->u.pointerslot) = *gp;
    if (gp->o) gp->o->refcount++;
}

static void floatinlet_float(  t_inlet *x, t_float f)   { *(x->u.floatslot) = f; }
static void symbolinlet_symbol(t_inlet *x, t_symbol *s) { *(x->u.symslot)   = s; }

#define COMMON \
    x->owner = owner; \
    x->dest = 0; \
    x->next = 0; \
    object_append_inlet(owner,x); \
    return x;

t_inlet *floatinlet_new(t_object *owner, t_float *fp) {
    t_inlet *x = (t_inlet *)pd_new(floatinlet_class);
    x->symfrom = &s_float; x->u.floatslot = fp; COMMON
}
t_inlet *symbolinlet_new(t_object *owner, t_symbol **sp) {
    t_inlet *x = (t_inlet *)pd_new(symbolinlet_class);
    x->symfrom = &s_symbol; x->u.symslot = sp; COMMON
}
t_inlet *pointerinlet_new(t_object *owner, t_gpointer *gp) {
    t_inlet *x = (t_inlet *)pd_new(pointerinlet_class);
    x->symfrom = &s_pointer; x->u.pointerslot = gp; COMMON
}
#undef COMMON

/* ---------------------- routine to handle lists ---------------------- */

/* objects interpret lists by feeding them to the individual inlets. Before you call this,
   check that the object doesn't have a more specific way to handle lists. */
void obj_list(t_object *x, t_symbol *s, int argc, t_atom *argv) {
    t_atom *ap;
    int count;
    t_inlet *ip = ((t_object *)x)->inlet;
    if (!argc) return;
    for (count = argc-1, ap = argv+1; ip && count--; ap++, ip = ip->next) {
        if      (ap->a_type == A_POINTER) pd_pointer(ip,ap->a_gpointer);
        else if (ap->a_type == A_FLOAT)     pd_float(ip,ap->a_float);
        else                               pd_symbol(ip,ap->a_symbol);
    }
    if      (argv->a_type == A_POINTER) pd_pointer(x, argv->a_gpointer);
    else if (argv->a_type == A_FLOAT)     pd_float(x, argv->a_float);
    else                                 pd_symbol(x, argv->a_symbol);
}

void obj_init () {
           inlet_class = class_new(gensym("inlet"), 0, 0, sizeof(t_inlet), CLASS_PD, 0);
      floatinlet_class = class_new(gensym("inlet"), 0, 0, sizeof(t_inlet), CLASS_PD, 0);
     symbolinlet_class = class_new(gensym("inlet"), 0, 0, sizeof(t_inlet), CLASS_PD, 0);
    pointerinlet_class = class_new(gensym("inlet"), 0, 0, sizeof(t_inlet), CLASS_PD, 0);
    class_addbang(inlet_class, inlet_bang);
    class_addpointer(inlet_class, inlet_pointer);
    class_addfloat(inlet_class, inlet_float);
    class_addsymbol(inlet_class, inlet_symbol);
    class_addlist(inlet_class, inlet_list);
    class_addanything(inlet_class, inlet_anything);
    class_addfloat(    floatinlet_class,   floatinlet_float);
    class_addsymbol(  symbolinlet_class,  symbolinlet_symbol);
    class_addpointer(pointerinlet_class, pointerinlet_pointer);
}

/* --------------------------- outlets ------------------------------ */

/* this is fairly obsolete stuff, I think */
static int outlet_eventno;
void outlet_setstacklim () {outlet_eventno++;}
int sched_geteventno( void) {return outlet_eventno;}

struct _outlet {
    t_object *owner;
    struct _outlet *next;
    t_outconnect *connections;
    t_symbol *sym;
};

t_inlet  *t_object:: in(int n) {t_inlet  *i= inlet; while(n--) i=i->next; return i;}
t_outlet *t_object::out(int n) {t_outlet *o=outlet; while(n--) o=o->next; return o;}

t_class *wire_class;
t_wire *wire_new (t_symbol *s, int argc, t_atom *argv) {
    t_wire *self = (t_wire *)pd_new(wire_class);
    self->g_adix = appendix_new((t_gobj *)self);
    return self;
}
void wire_free (t_wire *self) {/* nothing here */}

/* this is only used for pd_upload yet, right? so, it can use the new indices instead already */
void wire_save (t_wire *self, t_binbuf *b) {
//	t_canvas *c = self->dix->canvas;
	binbuf_addv(b,"ttiiii;","#X","connect",
//		canvas_getindex(c,self->from), self->outlet,
//		canvas_getindex(c,self->to  ), self-> inlet);
		self->from->dix->index, self->outlet,
		self->to  ->dix->index, self-> inlet);
	appendix_save((t_gobj *)self,b);
}

t_outlet *outlet_new(t_object *owner, t_symbol *s) {
    t_outlet *x = (t_outlet *)malloc(sizeof(*x)), *y, *y2;
    x->owner = owner;
    x->next = 0;
    y = owner->outlet;
    if (y) {
        while ((y2 = y->next)) y = y2;
        y->next = x;
    } else owner->outlet = x;
    x->connections = 0;
    x->sym = s;
    return x;
}

#define each_connect(oc,x) for (t_outconnect *oc = x->connections; oc; oc = oc->next)
void outlet_bang(t_outlet *x)                                          {each_connect(oc,x) pd_bang(oc->oc_to);}
void outlet_pointer(t_outlet *x, t_gpointer *gp) {t_gpointer gpointer = *gp; each_connect(oc,x) pd_pointer(oc->oc_to, &gpointer);}
void outlet_float(t_outlet *x, t_float f)                              {each_connect(oc,x) pd_float(oc->oc_to, f);}
void outlet_symbol(t_outlet *x, t_symbol *s)                           {each_connect(oc,x) pd_symbol(oc->oc_to, s);}
void outlet_list(t_outlet *x, t_symbol *s, int argc, t_atom *argv)     {each_connect(oc,x) pd_list(  oc->oc_to,s,argc,argv);}
void outlet_anything(t_outlet *x, t_symbol *s, int argc, t_atom *argv) {each_connect(oc,x) typedmess(oc->oc_to,s,argc,argv);}

void outlet_atom(t_outlet *x, t_atom *a) {
    if      (a->a_type == A_FLOAT  ) outlet_float(  x,a->a_float);
    else if (a->a_type == A_SYMBOL ) outlet_symbol( x,a->a_symbol);
    else if (a->a_type == A_POINTER) outlet_pointer(x,a->a_gpointer);
    else error("can't send atom whose type is %d",a->a_type);
}

/* get the outlet's declared symbol */
t_symbol *outlet_getsymbol(t_outlet *x) {return x->sym;}

void outlet_free(t_outlet *x) {
    t_object *y = x->owner;
    if (y->outlet == x) y->outlet = x->next;
    else for (t_outlet *x2 = y->outlet; x2; x2 = x2->next) if (x2->next == x) {
        x2->next = x->next;
        break;
    }
    free(x);
}

#define each_inlet(i,obj)  for ( t_inlet *i=obj->inlet; i; i=i->next)
#define each_outlet(o,obj) for (t_outlet *o=obj->outlet; o; o=o->next)

static t_pd *find_inlet(t_object *to, int inlet) {
    if (to->_class->firstin) {if (inlet) inlet--; else return (t_pd *)to;}
    each_inlet(i,to) if (inlet) inlet--; else return (t_pd *)i;
    return 0;
}

static t_outlet *find_outlet(t_object *from, int outlet) {
    each_outlet(o,from) if (outlet) outlet--; else return o;
    return 0;
}

t_outconnect *obj_connect(t_object *from, int outlet, t_object *to, int inlet) {
    t_outlet *o = find_outlet(from,outlet);
    t_pd *i = find_inlet(to,inlet);
    if (!o||!i) return 0;
    t_outconnect *oc = wire_new(0,0,0), *oc2;
    oc->next = 0;
    oc->oc_to = i;
    oc->from = from; oc->outlet = outlet;
    oc->to   =   to; oc->inlet  =  inlet;
    /* append it to the end of the list */
    /* LATER we might cache the last "oc" to make this faster. */
    if ((oc2 = o->connections)) {
        while (oc2->next) oc2 = oc2->next;
        oc2->next = oc;
    } else o->connections = oc;
    if (o->sym == &s_signal) canvas_update_dsp();
    return oc;
}

void obj_disconnect(t_object *from, int outlet, t_object *to, int inlet) {
    t_outlet *o = find_outlet(from,outlet); if (!o) {post("outlet does not exist"); return;}
    t_pd *i     =  find_inlet(to,   inlet); if (!i) {post( "inlet does not exist"); return;}
    t_outconnect *oc = o->connections, *oc2;
    if (!oc) {post("outlet has no connections"); return;}
    if (oc->oc_to == i) {
        o->connections = oc->next;
        pd_free(oc);
        goto done;
    }
    while ((oc2 = oc->next)) {
        if (oc2->oc_to == i) {
            oc->next = oc2->next;
            pd_free(oc2);
            goto done;
        }
        oc = oc2;
    }
    post("connection not found");
done:
    if (o->sym == &s_signal) canvas_update_dsp();
}

/* ------ traversal routines for code that can't see our structures ------ */

int obj_noutlets(t_object *x) {
    int n=0;
    each_outlet(o,x) n++;
    return n;
}

int obj_ninlets(t_object *x) {
    int n=!!x->_class->firstin;
    each_inlet(i,x) n++;
    return n;
}

t_outconnect *obj_starttraverseoutlet(t_object *x, t_outlet **op, int nout) {
    t_outlet *o = x->outlet;
    while (nout-- && o) o = o->next;
    *op = o;
    return o ? o->connections : 0;
}

t_outconnect *obj_nexttraverseoutlet(t_outconnect *lastconnect, t_object **destp, t_inlet **inletp, int *whichp) {
    t_pd *y = lastconnect->oc_to;
    if (ISINLET(y)) {
        t_inlet *i = (t_inlet *)y;
        t_object *dest = i->owner;
        int n = dest->_class->firstin;
        each_inlet(i2,dest) if (i2==i) break; else n++;
        *whichp = n;
        *destp = dest;
        *inletp = i;
    } else {
        *whichp = 0;
        *inletp = 0;
        *destp = (t_object *)y;
    }
    return lastconnect->next;
}

/* this one checks that a pd is indeed a patchable object, and returns it,
   correctly typed, or zero if the check failed. */
t_object *pd_checkobject(t_pd *x) {
    return x->_class->patchable ? (t_object *)x : 0;
}

/* move an inlet or outlet to the head of the list. this code is not safe with the latest additions in t_outconnect ! */
void obj_moveinletfirst( t_object *x,  t_inlet *i) {
    if (x->inlet == i) return;
    each_inlet( i2,x) if (i2->next == i) {i2->next = i->next; i->next = x-> inlet; x-> inlet = i; return;}}
void obj_moveoutletfirst(t_object *x, t_outlet *o) {
    if (x->outlet == o) return;
    each_outlet(o2,x) if (o2->next == o) {o2->next = o->next; o->next = x->outlet; x->outlet = o; return;}}

/* routines for DSP sorting, which are used in d_ugen.c and g_canvas.c */
/* LATER try to consolidate all the slightly different routines. */

int obj_nsiginlets(t_object *x) {
    int n=0;
    each_inlet(i,x) if (i->symfrom == &s_signal) n++;
    if (x->_class->firstin && x->_class->floatsignalin) n++;
    return n;
}
int obj_nsigoutlets(t_object *x) {
    int n=0;
    each_outlet(o,x) if (o->sym == &s_signal) n++;
    return n;
}

/* get the index, among signal inlets, of the mth inlet overall */
int obj_siginletindex(t_object *x, int m) {
    int n=0;
    if (x->_class->firstin && x->_class->floatsignalin) {if (!m--) return 0; else n++;}
    each_inlet(i,x)  if (i->symfrom == &s_signal) {if (!m) return n; else {n++; m--;}}
    return -1;
}
int obj_sigoutletindex(t_object *x, int m) {
    int n=0;
    each_outlet(o,x) if (o->sym     == &s_signal) {if (!m) return n; else {n++; m--;}}
    return -1;
}

int obj_issignalinlet(t_object *x, int m) {
    if (x->_class->firstin) {if (!m) return x->_class->floatsignalin; else m--;}
    t_inlet *i;
    for (i = x->inlet; i && m; i = i->next, m--) {}
    return i && i->symfrom==&s_signal;
}
int obj_issignaloutlet(t_object *x, int m) {
    t_outlet *o2;
    for (o2 = x->outlet; o2 && m--; o2 = o2->next) {}
    return o2 && o2->sym==&s_signal;
}

t_sample *obj_findsignalscalar(t_object *x, int m) {
    int n = 0;
    t_inlet *i;
    if (x->_class->firstin && x->_class->floatsignalin) {
        if (!m--) return x->_class->floatsignalin > 0 ? (t_sample *)(((char *)x) + x->_class->floatsignalin) : 0;
        n++;
    }
    for (i = x->inlet; i; i = i->next, m--) if (i->symfrom == &s_signal) {
        if (m == 0) return &i->u.floatsignalvalue;
        n++;
    }
    return 0;
}

/* and those two are only used in desire.c... */
int inlet_getsignalindex(t_inlet *x) {
    int n=0; for ( t_inlet *i = x->owner-> inlet; i; i = i->next) if (i==x) return n; else if (i->symfrom == &s_signal) n++;
    return -1;}
int outlet_getsignalindex(t_outlet *x) {
    int n=0; for (t_outlet *o = x->owner->outlet; o; o = o->next) if (o==x) return n; else if (o->sym     == &s_signal) n++;
    return -1;}

#ifdef QUALIFIED_NAME
static char *pd_library_name = 0;
void pd_set_library_name(char *libname){
  pd_library_name=libname;
}
#endif

t_hash<t_symbol *, t_class *> *class_table=0;
static t_symbol *class_loadsym;     /* name under which an extern is invoked */
static void pd_defaultfloat(t_pd *x, t_float f);
static void pd_defaultlist(t_pd *x, t_symbol *s, int argc, t_atom *argv);
t_pd pd_objectmaker;    /* factory for creating "object" boxes */
t_pd pd_canvasmaker;    /* factory for creating canvases */

static t_symbol *class_extern_dir = &s_;

static void pd_defaultanything(t_pd *x, t_symbol *s, int argc, t_atom *argv) {
    error("%s: no method for '%s'", x->_class->name->name, s->name);
}

static void pd_defaultbang(t_pd *x) {
    t_class *c = pd_class(x);
    if (c->listmethod != pd_defaultlist) c->listmethod(x,0,0,0);
    else c->anymethod(x,&s_bang,0,0);
}

static void pd_defaultfloat(t_pd *x, t_float f) {
    t_class *c = pd_class(x); t_atom at; SETFLOAT(&at, f);
    if (c->listmethod != pd_defaultlist) c->listmethod(x,0,1,&at); else c->anymethod(x,&s_float,1,&at);
}
static void pd_defaultsymbol(t_pd *x, t_symbol *s) {
    t_class *c = pd_class(x); t_atom at; SETSYMBOL(&at, s);
    if (c->listmethod != pd_defaultlist) c->listmethod(x,0,1,&at); else c->anymethod(x,&s_symbol,1,&at);
}
static void pd_defaultpointer(t_pd *x, t_gpointer *gp) {
    t_class *c = pd_class(x); t_atom at; SETPOINTER(&at, gp);
    if (c->listmethod != pd_defaultlist) c->listmethod(x,0,1,&at); else c->anymethod(x,&s_pointer,1,&at);
}

void obj_list(t_object *x, t_symbol *s, int argc, t_atom *argv);
static void class_nosavefn(t_gobj *z, t_binbuf *b);

/* handle "list" messages to Pds without explicit list methods defined. */
static void pd_defaultlist(t_pd *x, t_symbol *s, int argc, t_atom *argv) {
    t_class *c = pd_class(x);
    /* a list with no elements is handled by the 'bang' method if one exists. */
    if (argc == 0 && c->bangmethod != pd_defaultbang) {c->bangmethod(x); return;}
    /* a list with one element which is a number can be handled by a
       "float" method if any is defined; same for "symbol", "pointer". */
    if (argc == 1) {
#define HANDLE(A,M,D,F) if (argv->a_type==A && c->M != D) {c->M(x, argv->a_w.F); return;}
	HANDLE(A_FLOAT  ,floatmethod  ,pd_defaultfloat  ,w_float)
	HANDLE(A_SYMBOL ,symbolmethod ,pd_defaultsymbol ,w_symbol)
	HANDLE(A_POINTER,pointermethod,pd_defaultpointer,w_gpointer)
    }
    /* Next try for an "anything" method; if the object is patchable (i.e.,
       can have proper inlets) send it on to obj_list which will unpack the
       list into the inlets. otherwise gove up and complain. */
    if (c->anymethod != pd_defaultanything) c->anymethod(x,&s_list,argc,argv);
    else if (c->patchable) obj_list((t_object *)x, s, argc, argv);
    else pd_defaultanything(x, &s_list, argc, argv);
}

t_symbol *qualified_name(t_symbol *s) {
    char *buf;
    asprintf(&buf, "%s%s%s", pd_library_name, QUALIFIED_NAME, s->name);
    t_symbol *sym = gensym(buf);
    free(buf);
    return sym;
}

#undef class_new2
#undef class_addcreator2
#undef class_addmethod2

/* Note that some classes such as "select", are actually two classes of the same name,
   one for the single-argument form, one for the multiple one; see select_setup() to
   find out how this is handled.  */
t_class *class_new2(const char *ss, t_newmethod newmethod, t_method freemethod,
size_t size, int flags, const char *sig) {
    t_symbol *s = gensym(ss);
    int typeflag = flags & CLASS_TYPEMASK;
    if (!typeflag) typeflag = CLASS_PATCHABLE;
#ifdef QUALIFIED_NAME
    if (pd_library_name) s = qualified_name(s);
#endif
    if (pd_objectmaker._class && newmethod) {
        /* add a "new" method by the name specified by the object */
        class_addmethod2(pd_objectmaker._class, (t_method)newmethod, s->name, sig);
        if (class_loadsym) {
            /* if we're loading an extern it might have been invoked by a
            longer file name; in this case, make this an admissible name too. */
            char *loadstring = class_loadsym->name, l1 = strlen(s->name), l2 = strlen(loadstring);
            if (l2 > l1 && !strcmp(s->name, loadstring + (l2 - l1)))
                class_addmethod2(pd_objectmaker._class, (t_method)newmethod, class_loadsym->name, sig);
        }
    }
    t_class *c = (t_class *)malloc(sizeof(*c));
    c->name = c->helpname = s;
    c->size = size;
    c->methods = (t_methodentry *)malloc(1);
    c->nmethod = 0;
    c->freemethod    = (t_method)freemethod;
    c->bangmethod    = pd_defaultbang;
    c->pointermethod = pd_defaultpointer;
    c->floatmethod   = pd_defaultfloat;
    c->symbolmethod  = pd_defaultsymbol;
    c->listmethod    = pd_defaultlist;
    c->anymethod     = pd_defaultanything;
    c->firstin = ((flags & CLASS_NOINLET) == 0);
    c->firsttip = gensym("?");
    c->fields = (t_symbol **)malloc(sizeof(t_symbol *)*31);
    c->nfields = 0;
    c->patchable = (typeflag == CLASS_PATCHABLE);
    c->gobj = (typeflag >= CLASS_GOBJ);
    c->drawcommand = 0;
    c->floatsignalin = 0;
    c->externdir = class_extern_dir;
    c->savefn = (typeflag == CLASS_PATCHABLE ? text_save : class_nosavefn);
#ifdef QUALIFIED_NAME
    c->helpname = gensym(ss);
    // like a class_addcreator
    if (pd_library_name && newmethod)
      class_addmethod2(pd_objectmaker._class, (t_method)newmethod, ss, sig);
#endif
    c->onsubscribe = gobj_onsubscribe;
    class_table->set(c->name, c);
    return c;
}

/* add a creation method, which is a function that returns a Pd object
   suitable for putting in an object box.  We presume you've got a class it
   can belong to, but this won't be used until the newmethod is actually
   called back (and the new method explicitly takes care of this.) */
void class_addcreator2(const char *ss, t_newmethod newmethod, const char *sig) {
    t_symbol *s = gensym(ss);
    class_addmethod2(pd_objectmaker._class, (t_method)newmethod, ss, sig);
#ifdef QUALIFIED_NAME
    class_addmethod2(pd_objectmaker._class, (t_method)newmethod, pd_library_name ? qualified_name(s)->name : ss, sig);
#endif
    class_table->set(s,0);
}

void class_addmethod2(t_class *c, t_method fn, const char *ss, const char *fmt) {
    t_symbol *sel = gensym(ss);
    t_methodentry *m;
    int argtype = *fmt++;
    /* "signal" method specifies that we take audio signals but
       that we don't want automatic float to signal conversion.  This
       is obsolete; you should now use the CLASS_MAINSIGNALIN macro. */
    if (sel == &s_signal) {
        if (c->floatsignalin) post("warning: signal method overrides class_mainsignalin");
        c->floatsignalin = -1;
    }
    /* check for special cases.  "Pointer" is missing here so that
       pd_objectmaker's pointer method can be typechecked differently.  */
    /* is anyone actually using those five cases? */
    if      (sel==&s_bang)     {if (argtype)            goto phooey; class_addbang(    c,fn);}
    else if (sel==&s_float)    {if (argtype!='f'||*fmt) goto phooey; class_doaddfloat( c,fn);}
    else if (sel==&s_symbol)   {if (argtype!='s'||*fmt) goto phooey; class_addsymbol(  c,fn);}
    else if (sel==&s_list)     {if (argtype!='*')       goto phooey; class_addlist(    c,fn);}
    else if (sel==&s_anything) {if (argtype!='*')       goto phooey; class_addanything(c,fn);}
    else {
	/* SLOW, especially for [objectmaker] */
        c->methods = (t_methodentry *)realloc(c->methods, (c->nmethod+1) * sizeof(*c->methods));
        m = c->methods + c->nmethod;
        c->nmethod++;
        m->me_name = sel;
        m->me_fun = (t_gotfn)fn;
        int nargs = 0;
        while (argtype && nargs < MAXPDARG) {
	    t_atomtype t;
	    switch(argtype) {
		case 'f': t=A_FLOAT;    break;
		case 's': t=A_SYMBOL;   break;
		case 'p': t=A_POINTER;  break;
		case ';': t=A_SEMI;     break;
		case ',': t=A_COMMA;    break;
		case 'F': t=A_DEFFLOAT; break;
		case 'S': t=A_DEFSYMBOL;break;
		case '$': t=A_DOLLAR;   break;
		case '@': t=A_DOLLSYM;  break;
		case '*': t=A_GIMME;    break;
		case '!': t=A_CANT;     break;
		default: goto phooey;
	    };
            m->me_arg[nargs++] = t;
            argtype = *fmt++;
        }
        if (argtype) error("%s_%s: only 5 arguments are typecheckable; use A_GIMME aka '*'", c->name->name, sel->name);
        m->me_arg[nargs] = A_NULL;
    }
    return;
phooey:
    bug("class_addmethod: %s_%s: bad argument types", c->name->name, sel->name);
}

t_class *class_new(t_symbol *s, t_newmethod newmethod, t_method freemethod,
size_t size, int flags, t_atomtypearg arg1, ...) {
    char fmt[42],*f=fmt; va_list ap; va_start(ap,arg1); int t=arg1;
    while(t) {
	if (t>A_CANT) {error("class_new: ARRGH! t=%d",t); return 0;}
	*f++ = " fsp;,FS$@*!"[t];
	t=(t_atomtype)va_arg(ap,int);
    }
    *f=0; va_end(ap); return class_new2(s->name,newmethod,freemethod,size,flags,fmt);
}
void class_addcreator(t_newmethod newmethod, t_symbol *s, t_atomtypearg arg1, ...) {
    char fmt[42],*f=fmt; va_list ap; va_start(ap,arg1); int t=arg1;
    while(t) {
	if (t>A_CANT) {error("class_addcreator: ARRGH! t=%d",t); return;}
	*f++ = " fsp;,FS$@*!"[t];
	t=(t_atomtype)va_arg(ap,int);
    }
    *f=0; va_end(ap); class_addcreator2(s->name,newmethod,fmt);
}
void class_addmethod(t_class *c, t_method fn, t_symbol *sel, t_atomtypearg arg1, ...) {
    char fmt[42],*f=fmt; va_list ap; va_start(ap,arg1); int t=arg1;
    while(t) {
	if (t>A_CANT) {error("class_addmethod: ARRGH! t=%d",t); return;}
	*f++ = " fsp;,FS$@*!"[t];
	t=(t_atomtype)va_arg(ap,int);
    }
    *f=0; va_end(ap); class_addmethod2(c,fn,sel->name,fmt);
}

/* see also the "class_addfloat", etc.,  macros in m_pd.h */
#undef class_addbang
#undef class_addpointer
#undef class_addsymbol
#undef class_addlist
#undef class_addanything
void class_addbang(    t_class *c, t_method fn) {c->   bangmethod =    (t_bangmethod)fn;}
void class_addpointer( t_class *c, t_method fn) {c->pointermethod = (t_pointermethod)fn;}
void class_doaddfloat( t_class *c, t_method fn) {c->  floatmethod =   (t_floatmethod)fn;}
void class_addsymbol(  t_class *c, t_method fn) {c-> symbolmethod =  (t_symbolmethod)fn;}
void class_addlist(    t_class *c, t_method fn) {c->   listmethod =    (t_listmethod)fn;}
void class_addanything(t_class *c, t_method fn) {c->    anymethod =     (t_anymethod)fn;}

char *class_getname(t_class *c)     {return c->name->name;}
char *class_gethelpname(t_class *c) {return c->helpname->name;}
void class_sethelpsymbol(t_class *c, t_symbol *s) {c->helpname = s;}
void class_setdrawcommand(t_class *c) {c->drawcommand = 1;}
int  class_isdrawcommand( t_class *c) {return c->drawcommand;}
void class_setnotice(     t_class *c, t_notice      notice     ) {c->notice      = notice     ;}
void class_setonsubscribe(t_class *c, t_onsubscribe onsubscribe) {c->onsubscribe = onsubscribe;}

static void pd_floatforsignal(t_pd *x, t_float f) {
    int offset = x->_class->floatsignalin;
    if (offset > 0)
        *(t_sample *)(((char *)x) + offset) = f;
    else
        error("%s: float unexpected for signal input", x->_class->name->name);
}

void class_domainsignalin(t_class *c, int onset) {
    if (onset <= 0) onset = -1;
    else {
        if (c->floatmethod != pd_defaultfloat)
            post("warning: %s: float method overwritten", c->name->name);
        c->floatmethod = (t_floatmethod)pd_floatforsignal;
    }
    c->floatsignalin = onset;
}

void class_set_extern_dir(t_symbol *s) {class_extern_dir = s;}
char *class_gethelpdir(t_class *c) {return c->externdir->name;}

static void class_nosavefn(t_gobj *z, t_binbuf *b) {
    bug("save function called but not defined");
}

void class_setsavefn(t_class *c, t_savefn f) {c->savefn = f;}
t_savefn class_getsavefn(t_class *c) {return  c->savefn;}

/* ---------------- the symbol table ------------------------ */

/* tb: new 16 bit hash table: multiplication hash */
#ifndef NEWHASH
#define HASHSIZE 1024
#else
#define HASHSIZE 65536
#define HASHFACTOR 40503 /* donald knuth: (sqrt(5) - 1)/2*pow(2,16) */
#endif

#ifdef NEWHASH
static short hash(const char *s, size_t n) {
    unsigned short hash1 = 0, hash2 = 0;
#else
static int hash(const char *s, size_t n) {
    unsigned int hash1 = 0, hash2 = 0;
#endif
    const char *s2 = s;
    while (n) {
        hash1 += *s2;
        hash2 += hash1;
        s2++;
	n--;
    }
    return hash2;
}

/* tb: made dogensym() threadsafe
 * supported by vibrez.net */
t_symbol *dogensym(const char *s, size_t n, t_symbol *oldsym) {
    static t_symbol *symhash[HASHSIZE];
#ifdef THREADSAFE_GENSYM
    static pthread_mutex_t hash_lock = PTHREAD_MUTEX_INITIALIZER;
#endif
    t_symbol **sym1, *sym2;
#ifdef NEWHASH
    unsigned short hash2 = hash(s,n);
#else
    unsigned int   hash2 = hash(s,n);
#endif
#ifdef NEWHASH
	hash2 = hash2 * HASHFACTOR;
	sym1 = symhash + hash2;
#else
	sym1 = symhash + (hash2 & (HASHSIZE-1));
#endif
    while ((sym2 = *sym1)) {
        if (!strcmp(sym2->name, s)) return sym2;
        sym1 = &sym2->next;
    }
#ifdef THREADSAFE_GENSYM
    pthread_mutex_lock(&hash_lock);
    /* tb: maybe another thread added the symbol to the hash table; double check */
    while (sym2 = *sym1) {
        if (!strcmp(sym2->name, s)) {
	    pthread_mutex_unlock(&hash_lock);
	    return sym2;
	}
        sym1 = &sym2->next;
    }
#endif

    if (oldsym) sym2 = oldsym;
    else {
        sym2 = (t_symbol *)malloc(sizeof(*sym2));
        sym2->name = (char *)malloc(n+1);
        sym2->next = 0;
        sym2->thing = 0;
        memcpy(sym2->name, s, n);
	sym2->name[n]=0;
	sym2->n=n;
    }
    *sym1 = sym2;
#ifdef THREADSAFE_GENSYM
	pthread_mutex_unlock(&hash_lock);
#endif
    return sym2;
}

t_symbol *gensym( const char *s) {return dogensym(s,strlen(s),0);}
t_symbol *gensym2(const char *s, size_t n) {return dogensym(s,n,0);}
extern "C" t_symbol *symprintf(const char *s, ...) {
    char *buf;
    va_list args;
    va_start(args,s);
    vasprintf(&buf,s,args);
    va_end(args);
    t_symbol *r = gensym(buf);
    free(buf);
    return r;
}

static int tryingalready;
extern "C" void canvas_popabstraction(t_canvas *x);
extern t_pd *newest;
t_symbol* pathsearch(t_symbol *s,char* ext);
int pd_setloadingabstraction(t_symbol *sym);

/* this routine is called when a new "object" is requested whose class Pd
   doesn't know.  Pd tries to load it as an extern, then as an abstraction. */
void new_anything(void *dummy, t_symbol *s, int argc, t_atom *argv) {
    int fd;
    char *dirbuf, *nameptr;
    if (tryingalready) return;
    newest = 0;
    class_loadsym = s;
    if (sys_load_lib(canvas_getcurrent(), s->name)) {
        tryingalready = 1;
        typedmess((t_pd *)dummy, s, argc, argv);
        tryingalready = 0;
        return;
    }
    class_loadsym = 0;
    t_pd *current = s__X.thing;
    if ((fd = canvas_open2(canvas_getcurrent(), s->name, ".pd",  &dirbuf, &nameptr, 0)) >= 0 ||
        (fd = canvas_open2(canvas_getcurrent(), s->name, ".pat", &dirbuf, &nameptr, 0)) >= 0) {
        close(fd);
        if (!pd_setloadingabstraction(s)) {
            canvas_setargs(argc, argv); /* bug fix by Krzysztof Czaja */
            binbuf_evalfile(gensym(nameptr), gensym(dirbuf));
            if (s__X.thing != current) canvas_popabstraction((t_canvas *)s__X.thing);
            canvas_setargs(0, 0);
        } else error("%s: can't load abstraction within itself", s->name);
        free(dirbuf);
    } else newest = 0;
}

#define MAKESYM(CSYM,S) t_symbol CSYM = {(char *)(S),0,0,1,0xdeadbeef};
MAKESYM(s_pointer ,"pointer")
MAKESYM(s_float   ,"float")
MAKESYM(s_symbol  ,"symbol")
MAKESYM(s_bang    ,"bang")
MAKESYM(s_list    ,"list")
MAKESYM(s_anything,"anything")
MAKESYM(s_signal  ,"signal")
MAKESYM(s__N      ,"#N")
MAKESYM(s__X      ,"#X")
MAKESYM(s_x       ,"x")
MAKESYM(s_y       ,"y")
MAKESYM(s_        ,"")

static t_symbol *symlist[] = { &s_pointer, &s_float, &s_symbol, &s_bang,
    &s_list, &s_anything, &s_signal, &s__N, &s__X, &s_x, &s_y, &s_};

t_pd *newest;

/* This is externally available, but note that it might later disappear; the
whole "newest" thing is a hack which needs to be redesigned. */
t_pd *pd_newest () {return newest;}

    /* horribly, we need prototypes for each of the artificial function
    calls in typedmess(), to keep the compiler quiet. */
typedef t_pd *(*t_newgimme)(t_symbol *s, int argc, t_atom *argv);
typedef void(*t_messgimme)(t_pd *x, t_symbol *s, int argc, t_atom *argv);

#define REST t_floatarg d1, t_floatarg d2, t_floatarg d3, t_floatarg d4, t_floatarg d5
typedef t_pd *(*t_fun0)(REST);
typedef t_pd *(*t_fun1)(t_int i1, REST);
typedef t_pd *(*t_fun2)(t_int i1, t_int i2, REST);
typedef t_pd *(*t_fun3)(t_int i1, t_int i2, t_int i3, REST);
typedef t_pd *(*t_fun4)(t_int i1, t_int i2, t_int i3, t_int i4, REST);
typedef t_pd *(*t_fun5)(t_int i1, t_int i2, t_int i3, t_int i4, t_int i5, REST);
typedef t_pd *(*t_fun6)(t_int i1, t_int i2, t_int i3, t_int i4, t_int i5, t_int i6, REST);
#undef REST

void pd_typedmess_2(t_pd *x, t_symbol *s, int argc, t_atom *argv) {
    t_class *c = x->_class;
    t_atomtype *wp, wanttype;
    t_int      ai[MAXPDARG+1], *ap = ai;
    t_floatarg ad[MAXPDARG+1], *dp = ad;
    int narg = 0;
    /* check for messages that are handled by fixed slots in the class structure.  We don't catch "pointer"
       though so that sending "pointer" to pd_objectmaker doesn't require that we supply a pointer value. */
    if (s == &s_float) {
        if (!argc) c->floatmethod(x, 0.);
        else if (argv->a_type == A_FLOAT) c->floatmethod(x, argv->a_float);
        else error("expected one float, in class [%s]", c->name->name);
        return;
    }
    if (s == &s_bang) {c->bangmethod(x); return;}
    if (s == &s_list) {c->listmethod(x,s,argc,argv); return;}
    if (s == &s_symbol) {c->symbolmethod(x, argc && argv->a_type==A_SYMBOL ? argv->a_symbol : &s_); return;}
    t_methodentry *m = c->methods;
    for (int i = c->nmethod; i--; m++) if (m->me_name == s) {
        wp = m->me_arg;
        if (*wp == A_GIMME) {
            if (x == &pd_objectmaker) pd_set_newest(((t_newgimme)(m->me_fun))(  s,argc,argv));
            else                                   ((t_messgimme)(m->me_fun))(x,s,argc,argv);
            return;
        }
        if (argc > MAXPDARG) argc = MAXPDARG;
        if (x != &pd_objectmaker) *(ap++) = (t_int)x, narg++;
        while ((wanttype = *wp++)) {
            switch (wanttype) {
            case A_POINTER:
                if (!argc) goto badarg;
                if (argv->a_type!=A_POINTER) goto badarg;
                *ap = t_int(argv->a_gpointer);
                argc--; argv++;
                narg++;
                ap++;
                break;
            case A_FLOAT:    if (!argc) goto badarg;
            case A_DEFFLOAT: if (!argc) *dp = 0;
                else {
                    if (argv->a_type!=A_FLOAT) goto badarg;
                    *dp = argv->a_float;
                    argc--; argv++;
                }
                dp++;
                break;
            case A_SYMBOL: if (!argc) goto badarg;
            case A_DEFSYM: if (!argc) *ap = t_int(&s_);
                else {
                    if (argv->a_type == A_SYMBOL) *ap = t_int(argv->a_symbol);
                    /* if it's an unfilled "dollar" argument it appears as zero here; cheat and bash it to the null
                       symbol.  Unfortunately, this lets real zeros pass as symbols too, which seems wrong... */
                    else if (x == &pd_objectmaker && argv->a_type == A_FLOAT && argv->a_float == 0)
                        *ap = t_int(&s_);
                    else goto badarg;
                    argc--; argv++;
                }
                narg++;
                ap++;
            default: {}
            }
        }
        t_pd *bonzo;
        switch (narg) {
#define REST ad[0],ad[1],ad[2],ad[3],ad[4]
        case 0 : bonzo = ((t_fun0)(m->me_fun))(                                    REST); break;
        case 1 : bonzo = ((t_fun1)(m->me_fun))(ai[0],                              REST); break;
        case 2 : bonzo = ((t_fun2)(m->me_fun))(ai[0],ai[1],                        REST); break;
        case 3 : bonzo = ((t_fun3)(m->me_fun))(ai[0],ai[1],ai[2],                  REST); break;
        case 4 : bonzo = ((t_fun4)(m->me_fun))(ai[0],ai[1],ai[2],ai[3],            REST); break;
        case 5 : bonzo = ((t_fun5)(m->me_fun))(ai[0],ai[1],ai[2],ai[3],ai[4],      REST); break;
        case 6 : bonzo = ((t_fun6)(m->me_fun))(ai[0],ai[1],ai[2],ai[3],ai[4],ai[5],REST); break;
        default: bonzo = 0;
        }
        if (x == &pd_objectmaker) pd_set_newest(bonzo);
        return;
    }
    c->anymethod(x, s, argc, argv);
    return;
badarg:
    error("Bad arguments for message '%s' to object '%s'", s->name, c->name->name);
}

void pd_typedmess(t_pd *x, t_symbol *s, int argc, t_atom *argv) {
	ENTER(s); pd_typedmess_2(x,s,argc,argv); LEAVE;
}

void pd_vmess(t_pd *x, t_symbol *sel, const char *fmt, ...) {
    va_list ap;
    t_atom arg[MAXPDARG], *at =arg;
    int nargs = 0;
    const char *fp = fmt;
    va_start(ap, fmt);
    while (1) {
        if (nargs > MAXPDARG) {
            error("pd_vmess: only %d allowed", MAXPDARG);
            break;
        }
        switch(*fp++) {
        case 'f': SETFLOAT(at,   va_arg(ap, double)); break;
        case 's': SETSYMBOL(at,  va_arg(ap, t_symbol *)); break;
        case 'i': SETFLOAT(at,   va_arg(ap, t_int)); break;
        case 'p': SETPOINTER(at, va_arg(ap, t_gpointer *)); break;
        default: goto done;
        }
        at++;
        nargs++;
    }
done:
    va_end(ap);
    typedmess(x, sel, nargs, arg);
}

void pd_forwardmess(t_pd *x, int argc, t_atom *argv) {
    if (argc) {
        t_atomtype t = argv->a_type;
        if      (t == A_SYMBOL)   pd_typedmess(x, argv->a_symbol, argc-1, argv+1);
        else if (t == A_POINTER) {if (argc==1) pd_pointer(x, argv->a_gpointer); else pd_list(x, &s_list, argc, argv);}
	else if (t == A_FLOAT)   {if (argc==1) pd_float(  x, argv->a_float);    else pd_list(x, &s_list, argc, argv);}
        else bug("pd_forwardmess");
    }
}

void nullfn () {}

t_gotfn getfn(t_pd *x, t_symbol *s) {
    t_class *c = x->_class;
    t_methodentry *m = c->methods;
    for (int i=c->nmethod; i--; m++) if (m->me_name == s) return m->me_fun;
    error("%s: no method for message '%s'", c->name->name, s->name);
    return (t_gotfn)nullfn;
}

t_gotfn zgetfn(t_pd *x, t_symbol *s) {
    t_class *c = x->_class;
    t_methodentry *m = c->methods;
    for (int i=c->nmethod; i--; m++) if (m->me_name == s) return m->me_fun;
    return 0;
}

void class_settip(t_class *x,t_symbol* s) {x->firsttip = s;}

/* must be called only once */
void class_setfieldnames(t_class *x, const char *s) {
    char foo[64];
    while (*s) {
	char *t = strchr(s,' ');
	int i = t-s;
	if (!t) return;
	memcpy(foo,s,i);
	foo[i]=0;
	x->fields[x->nfields++] = gensym(foo);
	s=s+i+1;
    }
}

int class_getfieldindex(t_class *x, const char *s) {
	t_symbol *sy = gensym((char *)s);
	for (int i=0; i<x->nfields; i++) if (x->fields[i]==sy) return i;
	return -1;
}

/* O(n) asymptotic time :-} */
/* only looks for already loaded classes though. */

t_class *class_find (t_symbol *s) {return (t_class *)class_table->get(s);}

void glob_update_class_info (t_pd *bogus, t_symbol *s, t_symbol *cb_recv, t_symbol *cb_sel) {
    t_class *c = class_find(s);
    if (!c) { post("class not found!"); return; }
    sys_vgui("global class_info; set class_info(%s) [list "
                "helpname \"%s\" externdir \"%s\" size \"%d\" "
/*
    t_methodentry *c_methods; int c_nmethod;
    t_method c_freemethod;
    t_savefn c_savefn;
    int c_floatsignalin;
*/
    "gobj \"%d\" patchable \"%d\" firstin \"%d\" "
    "firsttip \"%s\" methods {",s->name,c->helpname->name,c->externdir->name,
	c->size,c->gobj,c->patchable,c->firstin,c->firsttip->name);
    if (c->   bangmethod != pd_defaultbang)     sys_vgui("<bang> ");
    if (c->pointermethod != pd_defaultpointer)  sys_vgui("<pointer> ");
    if (c->  floatmethod != pd_defaultfloat)    sys_vgui("<float> ");
    if (c-> symbolmethod != pd_defaultsymbol)   sys_vgui("<symbol> ");
    if (c->   listmethod != pd_defaultlist)     sys_vgui("<list> ");
    if (c->    anymethod != pd_defaultanything) sys_vgui("<any> ");
    for (int i=0; i<c->nmethod; i++) sys_vgui("%s ",c->methods[i].me_name->name);
    sys_vgui("}]; %s %s %s\n",cb_recv->name, cb_sel->name, s->name);
}

t_class *binbuf_class;

t_binbuf *binbuf_new () {
    t_binbuf *x = (t_binbuf *)pd_new(binbuf_class);
    x->n = 0;
    x->capa = 1;
    x->v = (t_atom *)malloc(1*sizeof(t_atom));
    return x;
}

/* caution: capa >= x->n and capa >= 1 too */
static void binbuf_capa(t_binbuf *x, int capa) {
    x->v = (t_atom *)realloc(x->v, capa*sizeof(*x->v));
    x->capa = capa;
}

void binbuf_free(t_binbuf *x) {pd_free(x);}
void binbuf_free2(t_binbuf *x) {free(x->v);}

t_binbuf *binbuf_duplicate(t_binbuf *y) {
    t_binbuf *x = (t_binbuf *)malloc(sizeof(*x));
    x->capa = x->n = y->n;
    x->v = (t_atom *)malloc(x->n * sizeof(*x->v));
    memcpy(x->v,y->v,x->n*sizeof(*x->v));
    return x;
}

void binbuf_clear(t_binbuf *x) {
    x->n = 0;
    x->v = (t_atom *)realloc(x->v,4);
    x->capa = 4;
}

/* called just after a doublequote in version 1 parsing */
const char *binbuf_text_quoted(t_binbuf *x, const char *t, char *end) {
	ostringstream buf;
	while (t!=end) {
		char c = *t++;
		if (c=='"') break;
		if (c!='\\') {buf << c; continue;}
		c = *t++;
		if (c=='a') {buf << '\a'; continue;}
		if (c=='b') {buf << '\b'; continue;}
		if (c=='f') {buf << '\f'; continue;}
		if (c=='n') {buf << '\n'; continue;}
		if (c=='r') {buf << '\r'; continue;}
		if (c=='v') {buf << '\v'; continue;}
		if (c=='t') {buf << '\t'; continue;}
		if (c=='"') {buf << '\"'; continue;}
		if (c=='\\'){buf << '\\'; continue;}
		if (c=='\n'){continue;}
		/* if (c=='u') ... */
		/* if (c=='x') ... */
		/* if (isdigit(c)) ... */
		buf << c; /* ignore syntax error (should it?) */
	}
	binbuf_addv(x,"t",buf.str().data());
	return t; /* ignore syntax error (should it?) */
}

/* find the first atom in text, in any, and add it to this binbuf;
   returns pointer to end of atom text */
/* this one is for pd format version 1 */
/* TODO: double-quotes, braces, test backslashes&dollars */
const char *binbuf_text_matju(t_binbuf *x, const char *t, const char *end) {
	int doll=0;
	while (t!=end && isspace(*t)) t++;
	if (t==end) return t;
	if (*t==';') {binbuf_addv(x,";"); return t+1;}
	if (*t==',') {binbuf_addv(x,","); return t+1;}
	/* if (*t=='"') return binbuf_text_quoted(x,t,end); */
	if (*t=='+' || *t=='-' || *t=='.' || isdigit(*t)) {
	    char *token;
	    double v = strtod(t,&token);
	    if (t==end || isspace(*token)) {binbuf_addv(x,"f",v); return token;}
	}
	ostringstream buf;
	for (; t!=end && *t!=',' && *t!=';' && !isspace(*t); ) {
		doll |= t[0]=='$' && t+1!=end && isdigit(t[1]);
		if (*t=='\\') t++;
		if (t!=end) buf << *t++;
	}
	if (doll) {
		const char *b = buf.str().data();
		if (b[0]!='$') doll=0;
		for (b++; *b; b++) if (!isdigit(*b)) doll=0;
		if (doll) binbuf_addv(x,"$",atoi(buf.str().data()+1));
		else      binbuf_addv(x,"&",gensym(buf.str().data()));
	} else            binbuf_addv(x,"t",buf.str().data());
	return t;
}

/* this one is for pd format version 0 */
const char *binbuf_text_miller(t_binbuf *x, const char *t, const char *end) {
    ostringstream buf;
    /* it's an atom other than a comma or semi */
    int q = 0, slash = 0, lastslash = 0, dollar = 0;
    /* skip leading space */
    while (t!=end && isspace(*t)) t++;
    if (t==end) return t;
    if (*t==';') {binbuf_addv(x,";"); return t+1;}
    if (*t==',') {binbuf_addv(x,","); return t+1;}
    do {
	char c = *t++;
	lastslash = slash;
	slash = c=='\\';
	if (q >= 0) {
	  int digit = isdigit(c), dot=c=='.', minus=c=='-', plusminus=minus||c=='+', expon=c=='e'||c=='E';
	  if      (q==0) { /* beginning */ if (minus) q=1; else if (digit) q=2; else if (dot) q=3; else q=-1;}
	  else if (q==1) { /* got minus */                      if (digit) q=2; else if (dot) q=3; else q=-1;}
	  else if (q==2) { /* got digits */               if (dot) q=4; else if (expon) q=6; else if (!digit) q=-1;}
	  else if (q==3) { /* got '.' without digits */ if (digit) q=5; else q=-1;}
	  else if (q==4) { /* got '.'   after digits */ if (digit) q=5; else if (expon) q=6; else q=-1;}
	  else if (q==5) { /* got digits after . */                          if (expon) q=6; else if (!digit) q=-1;}
	  else if (q==6) { /* got 'e' */ if (plusminus) q=7; else if (digit) q=8; else q=-1;}
	  else if (q==7) { /* got plus or minus */ if (digit) q=8; else q=-1;}
	  else if (q==8) { /* got digits */ if (!digit) q=-1;}
	}
	if (!lastslash && c == '$' && t!=end && isdigit(*t)) dollar = 1;
#if 1
	if (slash&&lastslash) slash=0;
#endif
	if (!slash) buf << c;
    } while (t!=end && (slash || !strchr(" \n\r\t,;",*t)));
    if (q == 2 || q == 4 || q == 5 || q == 8) {binbuf_addv(x,"f",atof(buf.str().data())); return t;}
    /* LATER try to figure out how to mix "$" and "\$" correctly; here, the backslashes were already
       stripped so we assume all "$" chars are real dollars.  In fact, we only know at least one was. */
    if (dollar) {
        const char *b = buf.str().data();
        //printf("b=%s\n",b);
	if (*b != '$') dollar = 0;
	for (b++; *b; b++) if (!isdigit(*b)) dollar = 0;
	if (dollar) binbuf_addv(x,"$",atoi(buf.str().data()+1));
	else        binbuf_addv(x,"&",gensym(buf.str().data()));
    } else          binbuf_addv(x,"t",buf.str().data());
    return t;
}

int sys_syntax = 0;

void binbuf_text(t_binbuf *x, const char *t, size_t size) {
	const char *end=t+size;
	binbuf_clear(x);
	while (t!=end) t = sys_syntax ? binbuf_text_matju(x,t,end) : binbuf_text_miller(x,t,end);
	binbuf_capa(x,x->n);
}

void pd_eval_text(const char *t, size_t size) {
	t_binbuf *x = binbuf_new();
	const char *end = t+size;
	while (t!=end) {
		t = sys_syntax ? binbuf_text_matju(x,t,end) : binbuf_text_miller(x,t,end);
		if (x->n && x->v[x->n-1].a_type == A_SEMI) {
			binbuf_eval(x,0,0,0);
			binbuf_clear(x);
		}
	}
	binbuf_free(x);
}

void voprintf(ostream &buf, const char *s, va_list args) {
    char *b;
    vasprintf(&b,s,args);
    buf << b;
    free(b);
}
void oprintf(ostream &buf, const char *s, ...) {
    va_list args;
    va_start(args,s);
    voprintf(buf,s,args);
    va_end(args);
}

/* convert a binbuf to text; no null termination. */
void binbuf_gettext(t_binbuf *x, char **bufp, int *lengthp) {
    ostringstream buf;
    t_atom *ap = x->v;
    char nextdelim=0;
    for (int i=x->n; i--; ap++) {
    	if (ap->a_type != A_SEMI && ap->a_type != A_COMMA && nextdelim) buf << (char)nextdelim;
        atom_ostream(ap,buf);
        nextdelim = ap->a_type == A_SEMI ? '\n' : ' ';
    }
    //if (nextdelim) buf << (char)nextdelim;
    *bufp = strdup(buf.str().data());
    *lengthp = buf.str().size();// - (nextdelim == ' ');
}

/* convert a binbuf to text with null termination, as return value */
char *binbuf_gettext2(t_binbuf *x) {
  char *buf; int n;
  binbuf_gettext(x,&buf,&n);
  buf[n] = 0;
  return (char *)realloc(buf,n+1);
}

/* Miller said: fix this so that writing to file doesn't buffer everything together. */
/* matju said: make this use vector size doubling as it used to be in binbuf_text */
void binbuf_add(t_binbuf *x, int argc, t_atom *argv) {
    int newsize = x->n + argc;
    t_atom *ap = (t_atom *)realloc(x->v,newsize*sizeof(*x->v));
    x->v = ap;
    ap += x->n;
    for (int i = argc; i--; ap++) *ap = *(argv++);
    x->capa = x->n = newsize;
}

#define MAXADDMESSV 100
void binbuf_addv(t_binbuf *x, const char *fmt, ...) {
    va_list ap;
    t_atom arg[MAXADDMESSV], *at =arg;
    int nargs = 0;
    const char *fp = fmt;
    va_start(ap, fmt);
    while (1) {
        if (nargs >= MAXADDMESSV) {
            error("binbuf_addmessv: only %d allowed", MAXADDMESSV);
            break;
        }
        switch(*fp++) {
        case 'i': SETFLOAT(at, va_arg(ap, int)); break;
        case 'f': SETFLOAT(at, va_arg(ap, double)); break;
        case 's': SETSYMBOL(at, va_arg(ap, t_symbol *)); break;
        case 't': SETSYMBOL(at, gensym(va_arg(ap, char *))); break;
        case ';': SETSEMI(at); break;
        case ',': SETCOMMA(at); break;
	case '$': SETDOLLAR(at, va_arg(ap, int)); break;
	case '&': SETDOLLSYM(at, va_arg(ap, t_symbol *)); break;
        default: goto done;
        }
        at++;
        nargs++;
    }
done:
    va_end(ap);
    binbuf_add(x, nargs, arg);
}

/* add a binbuf to another one for saving.  Semicolons and commas go to
symbols ";", "'",; the symbol ";" goes to "\;", etc. */

void binbuf_addbinbuf(t_binbuf *x, t_binbuf *y) {
    t_binbuf *z = binbuf_new();
    binbuf_add(z, y->n, y->v);
    t_atom *ap = z->v;
    for (size_t i=0; i < z->n; i++, ap++) {
        switch (ap->a_type) {
        case A_FLOAT:   break;
        case A_SEMI:    SETSYMBOL(ap, gensym(";")); break;
        case A_COMMA:   SETSYMBOL(ap, gensym(",")); break;
        case A_DOLLAR:  SETSYMBOL(ap, symprintf("$%ld", ap->a_index)); break;
        case A_DOLLSYM: {
        	ostringstream b;
        	atom_ostream(ap,b);
        	SETSYMBOL(ap, gensym(b.str().data()));} break;
        case A_SYMBOL:
            /* FIXME make this general */
            if      (!strcmp(ap->a_symbol->name, ";")) SETSYMBOL(ap, gensym(";"));
            else if (!strcmp(ap->a_symbol->name, ",")) SETSYMBOL(ap, gensym(","));
            break;
        default:
            //bug("binbuf_addbinbuf: stray atom of type %d",ap->a_type);
            //abort();
            ;
        }
    }
    binbuf_add(x, z->n, z->v);
}

void binbuf_addsemi(t_binbuf *x) {
    t_atom a;
    SETSEMI(&a);
    binbuf_add(x, 1, &a);
}

/* Supply atoms to a binbuf from a message, making the opposite changes
from binbuf_addbinbuf.  The symbol ";" goes to a semicolon, etc. */

void binbuf_restore(t_binbuf *x, int argc, t_atom *argv) {
    int newsize = x->n + argc;
    t_atom *ap = (t_atom *)realloc(x->v,(newsize+1)*sizeof(*x->v));
    if (!ap) {error("binbuf_addmessage: out of space"); return;}
    x->v = ap;
    ap = x->v + x->n;
    for (int i = argc; i--; ap++) {
        if (argv->a_type == A_SYMBOL) {
            char *str = argv->a_symbol->name, *str2;
            if (!strcmp(str, ";")) SETSEMI(ap);
            else if (!strcmp(str, ",")) SETCOMMA(ap);
            else if ((str2 = strchr(str, '$')) && isdigit(str2[1])) {
                int dollsym = 0;
                if (*str != '$') dollsym = 1;
                else for (str2 = str + 1; *str2; str2++) if (!isdigit(*str2)) {
                    dollsym = 1;
                    break;
                }
                if (dollsym) SETDOLLSYM(ap, gensym(str));
                else {
                    int dollar = 0;
                    sscanf(argv->a_symbol->name + 1, "%d", &dollar);
                    SETDOLLAR(ap, dollar);
                }
            } else *ap = *argv;
            argv++;
        } else *ap = *(argv++);
    }
    x->n = newsize;
}

#define MSTACKSIZE 2048

void binbuf_print(t_binbuf *x) {
    int startedpost = 0, newline = 1;
    for (size_t i=0; i < x->n; i++) {
        if (newline) {
            if (startedpost) endpost();
            startpost("");
            startedpost = 1;
        }
        postatom(1, x->v + i);
        newline = !! x->v[i].a_type == A_SEMI;
    }
    if (startedpost) endpost();
}

int binbuf_getnatom(t_binbuf *x) {return x->n;}
t_atom *binbuf_getvec(t_binbuf *x) {return x->v;}

int canvas_getdollarzero ();

/* JMZ:
 * s points to the first character after the $
 * (e.g. if the org.symbol is "$1-bla", then s will point to "1-bla")
 * (e.g. org.symbol="hu-$1mu", s="1mu")
 * LATER: think about more complex $args, like ${$1+3}
 *
 * the return value holds the length of the $arg (in most cases: 1)
 * buf holds the expanded $arg
 *
 * if some error occurred, "-1" is returned
 *
 * e.g. "$1-bla" with list "10 20 30"
 * s="1-bla"
 * buf="10"
 * return value = 1; (s+1=="-bla")
 */
static int binbuf_expanddollsym(char *s, std::ostream &buf, t_atom dollar0, int ac, t_atom *av, int tonew) {
  int argno=atol(s);
  int arglen=0;
  char*cs=s;
  char c=*cs;
  while (c && isdigit(c)) {
    c=*cs++;
    arglen++;
  }
  /* invalid $-expansion (like "$bla") */
  if (cs==s) {buf << "$"; return 0;}
  if (argno < 0 || argno > ac) { /* undefined argument */
      if(!tonew) return 0;
      buf << "$" << argno;
  } else if (argno == 0) { /* $0 */
    atom_ostream(&dollar0, buf);
  } else { /* fine! */
    atom_ostream(av+(argno-1), buf);
  }
  return arglen-1;
}

/* LATER remove the dependence on the current canvas for $0; should be another argument. */
t_symbol *binbuf_realizedollsym(t_symbol *s, int ac, t_atom *av, int tonew) {
    ostringstream buf2;
    char *str=s->name;
    t_atom dollarnull;
    SETFLOAT(&dollarnull, canvas_getdollarzero());
    /* JMZ: currently, a symbol is detected to be A_DOLLSYM if it starts with '$'
     * the leading $ is stripped and the rest stored in "s". i would suggest to NOT strip the leading $
     * and make everything a A_DOLLSYM that contains(!) a $ whenever this happened, enable this code */
    char *substr=strchr(str, '$');
    if(!substr) return s;
    oprintf(buf2,"%.*s",substr-str,str);
    str=substr+1;
    for (;;) {
        std::ostringstream buf;
        int next = binbuf_expanddollsym(str, buf, dollarnull, ac, av, tonew);
        if (next<0) break;
    /* JMZ: i am not sure what this means, so i might have broken it. it seems like that if "tonew" is
        set and the $arg cannot be expanded (or the dollarsym is in reality a A_DOLLAR).
        0 is returned from binbuf_realizedollsym; this happens when expanding in a message-box,
        but does not happen when the A_DOLLSYM is the name of a subpatch */
    /* JMZ: this should mimick the original behaviour */
 	if(!tonew && !next && buf.str().size()==0) return 0;
	buf2 << buf;
	str+=next;
	substr=strchr(str, '$');
	if(substr) {
	  oprintf(buf2,"%.*s",substr-str,str);
	  str=substr+1;
	} else {
	  buf2 << str;
	  return gensym(buf2.str().data());
	}
    }
    return gensym(buf2.str().data());
}

void binbuf_eval(t_binbuf *x, t_pd *target, int argc, t_atom *argv) {
    static t_atom mstack[MSTACKSIZE], *msp = mstack, *ems = mstack+MSTACKSIZE;
    t_atom *stackwas = msp;
    t_atom *at = x->v;
    int ac = x->n;
    int nargs;
    while (1) {
        t_pd *nexttarget;
        while (!target) {
            t_symbol *s;
            while (ac && (at->a_type == A_SEMI || at->a_type == A_COMMA)) {ac--; at++;}
            if (!ac) break;
            if (at->a_type == A_DOLLAR) {
                if (at->a_index <= 0 || at->a_index > argc)  {error("$%d: not enough arguments supplied", at->a_index); goto cleanup;}
                else if (argv[at->a_index-1].a_type != A_SYMBOL) {error("$%d: symbol needed as receiver", at->a_index); goto cleanup;}
                else s = argv[at->a_index-1].a_symbol;
            } else if (at->a_type == A_DOLLSYM) {
                s = binbuf_realizedollsym(at->a_symbol, argc, argv, 0);
                if (!s) {error("$%s: not enough arguments supplied", at->a_symbol->name); goto cleanup;}
            } else s = atom_getsymbol(at);
	    target = s->thing;
	    /* IMPD: allows messages to unbound objects, via pointers */
	    if (!target) {
		if (!sscanf(s->name,".x%lx",(long*)&target)) target=0;
		if (target) {
			if (!object_table->exists(target) || !object_table->get(target)) {
				error("%s target is not a currently valid pointer",s->name);
				return;
			}
		}
	    }
            if (!target) {error("%s: no such object", s->name); goto cleanup;}
            at++;
            ac--;
            break;
            cleanup:
                do {at++; ac--;} while (ac && at->a_type != A_SEMI); /* is this the correct thing to do? */
                continue;
        }
        if (!ac) break;
        nargs = 0;
        nexttarget = target;
        while (1) {
            if (!ac) goto gotmess;
            if (msp >= ems) {error("message too long"); goto broken;}
            switch (at->a_type) {
            /* semis and commas in new message just get bashed to a symbol.  This is needed so you can pass them to "expr." */
            case A_SEMI:  if (target == &pd_objectmaker) {SETSYMBOL(msp, gensym(";")); break;} else {nexttarget = 0; goto gotmess;}
            case A_COMMA: if (target == &pd_objectmaker) {SETSYMBOL(msp, gensym(",")); break;} else                  goto gotmess;
            case A_FLOAT:
            case A_SYMBOL:
                *msp = *at;
                break;
            case A_DOLLAR:
                if (at->a_index > 0 && at->a_index <= argc) *msp = argv[at->a_index-1];
                else if (at->a_index == 0) SETFLOAT(msp, canvas_getdollarzero());
                else {
                    SETFLOAT(msp, 0);
                    if (target != &pd_objectmaker) error("$%d: argument number out of range", at->a_index);
                }
                break;
            case A_DOLLSYM: {
                t_symbol *s9 = binbuf_realizedollsym(at->a_symbol, argc, argv, target == &pd_objectmaker);
                if (!s9) {
                    error("%s: argument number out of range", at->a_symbol->name);
                    SETSYMBOL(msp, at->a_symbol);
                } else SETSYMBOL(msp, s9);
                break;}
            default:
                bug("bad item in binbuf");
                goto broken;
            }
            msp++;
            ac--;
            at++;
            nargs++;
        }
    gotmess:
        if (nargs) {
            switch (stackwas->a_type) {
            case A_SYMBOL: typedmess(target, stackwas->a_symbol, nargs-1, stackwas+1); break;
            case A_FLOAT:  if (nargs == 1) pd_float(target, stackwas->a_float); else pd_list(target, 0, nargs, stackwas); break;
            default: {}
            }
        }
        msp = stackwas;
        if (!ac) break;
        target = nexttarget;
        at++;
        ac--;
    }
    return;
broken:
    msp = stackwas;
}

static int binbuf_doopen(char *s, int mode) {
    char namebuf[strlen(s)+1];
#ifdef MSW
    mode |= O_BINARY;
#endif
    sys_bashfilename(s, namebuf);
    return open(namebuf, mode);
}

static FILE *binbuf_dofopen(const char *s, const char *mode) {
    char namebuf[strlen(s)+1];
    sys_bashfilename(s, namebuf);
    return fopen(namebuf, mode);
}

int binbuf_read(t_binbuf *b, const char *filename, const char *dirname, int flags) {
    long length;
    char *buf;
    char *namebuf=0;
    if (*dirname) asprintf(&namebuf,"%s/%s",dirname,filename);
    else          asprintf(&namebuf,   "%s",        filename);
    int fd = binbuf_doopen(namebuf, 0);
    if (fd < 0) {error("open: %s: %s",namebuf,strerror(errno)); return 1;}
    if ((length = lseek(fd, 0, SEEK_END)) < 0 || lseek(fd, 0, SEEK_SET) < 0 || !(buf = (char *)malloc(length))) {
        error("lseek: %s: %s",namebuf,strerror(errno));
        close(fd); free(namebuf);
        return 1;
    }
    int readret = read(fd, buf, length);
    if (readret < length) {
        error("read (%d %ld) -> %d; %s: %s", fd, length, readret, namebuf, strerror(errno));
        close(fd); free(namebuf); free(buf);
        return 1;
    }
    if (flags&1) for (int i=0; i<length; i++) if (buf[i]=='\n') buf[i] = ';';
    if (flags&2) pd_eval_text(buf,length); else binbuf_text(b, buf, length);
    close(fd); free(namebuf); free(buf);
    return 0;
}

/* read a binbuf from a file, via the search patch of a canvas */
int binbuf_read_via_canvas(t_binbuf *b, const char *filename, t_canvas *canvas, int flags) {
    char *buf, *bufptr;
    int fd = canvas_open2(canvas, filename, "", &buf, &bufptr, 0);
    if (fd<0) {error("%s: can't open", filename); return 1;}
    close(fd); free(buf);
    return !!binbuf_read(b, bufptr, buf, flags);
}

/* old version */
int binbuf_read_via_path(t_binbuf *b, char const *filename, const char *dirname, int flags) {
    char *buf, *bufptr;
    int fd = open_via_path2(dirname, filename, "", &buf, &bufptr, 0);
    if (fd<0) {error("%s: can't open", filename); return 1;}
    close(fd);
    bool r = binbuf_read(b, bufptr, buf, flags);
    free(buf);
    return r;
}

#define WBUFSIZE 4096
static t_binbuf *binbuf_convert(t_binbuf *oldb, int maxtopd);

/* write a binbuf to a text file.  If "crflag" is set we suppress semicolons. */
int binbuf_write(t_binbuf *x, const char *filename, const char *dir, int crflag) {
    char sbuf[WBUFSIZE];
    ostringstream fbuf;
    char *bp = sbuf, *ep = sbuf + WBUFSIZE;
    int indx; bool deleteit = 0;
    int ncolumn = 0;
    if (*dir) fbuf << dir << "/";
    fbuf << filename;
    if (!strcmp(filename + strlen(filename) - 4, ".pat")) {
        x = binbuf_convert(x, 0);
        deleteit = 1;
    }
    FILE *f = binbuf_dofopen(fbuf.str().data(), "w");
    if (!f) {error("open: %s: %s",fbuf.str().data(),strerror(errno)); goto fail;}
    indx = x->n;
    for (t_atom *ap = x->v; indx--; ap++) {
        /* estimate how many characters will be needed.  Printing out symbols may need extra characters for inserting backslashes. */
        int length = (ap->a_type == A_SYMBOL || ap->a_type == A_DOLLSYM) ? 80 + strlen(ap->a_symbol->name) : 40;
        if (ep - bp < length) {
            if (fwrite(sbuf, bp-sbuf, 1, f) < 1) {error("write: %s: %s",fbuf.str().data(),strerror(errno)); goto fail;}
            bp = sbuf;
        }
        if ((ap->a_type == A_SEMI || ap->a_type == A_COMMA) && bp > sbuf && bp[-1] == ' ') bp--;
        if (!crflag || ap->a_type != A_SEMI) {
            atom_string(ap, bp, (ep-bp)-2);
            length = strlen(bp);
            bp += length;
            ncolumn += length;
        }
        if (ap->a_type == A_SEMI || (!crflag && ncolumn > 65)) {
            *bp++ = '\n';
            ncolumn = 0;
        } else {
            *bp++ = ' ';
            ncolumn++;
        }
    }
    if (fwrite(sbuf, bp-sbuf, 1, f) < 1) {error("write: %s: %s",fbuf.str().data(),strerror(errno)); goto fail;}
    if (deleteit) binbuf_free(x);
    fclose(f);
    return 0;
fail:
    if (deleteit) binbuf_free(x);
    if (f) fclose(f);
    return 1;
}

/* The following routine attempts to convert from max to pd or back.  The max to pd direction is working OK
   but you will need to make lots of abstractions for objects like "gate" which don't exist in Pd. Conversion
   from Pd to Max hasn't been tested for patches with subpatches yet!  */
#define MAXSTACK 1000
#define ISSYMBOL(a, b) ((a)->a_type == A_SYMBOL && !strcmp((a)->a_symbol->name, (b)))
#define GETF(i) atom_getfloatarg(i,natom,nextmess)
static t_binbuf *binbuf_convert(t_binbuf *oldb, int maxtopd) {
    t_binbuf *newb = binbuf_new();
    t_atom *vec = oldb->v;
    t_int n = oldb->n, nextindex, stackdepth = 0, stack[MAXSTACK], nobj = 0;
    t_atom outmess[MAXSTACK], *nextmess;
    if (!maxtopd) binbuf_addv(newb,"tt;","max","v2");
    for (nextindex = 0; nextindex < n; ) {
        int endmess, natom;
        for (endmess = nextindex; endmess < n && vec[endmess].a_type != A_SEMI; endmess++) {}
        if (endmess == n) break;
        if (endmess == nextindex || endmess == nextindex + 1
          || vec[nextindex].a_type != A_SYMBOL || vec[nextindex+1].a_type != A_SYMBOL) {
            nextindex = endmess + 1;
            continue;
        }
        natom = endmess - nextindex;
        if (natom > MAXSTACK-10) natom = MAXSTACK-10;
        nextmess = vec + nextindex;
        char *first  =  nextmess   ->a_symbol->name;
        char *second = (nextmess+1)->a_symbol->name;
        if (maxtopd) { /* case 1: importing a ".pat" file into Pd. */
            /* dollar signs in file translate to symbols */
            for (int i=0; i<natom; i++) {
                if (nextmess[i].a_type == A_DOLLAR) {
                    SETSYMBOL(nextmess+i, symprintf("$%ld",nextmess[i].a_index));
                } else if (nextmess[i].a_type == A_DOLLSYM) {
                    SETSYMBOL(nextmess+i, gensym(nextmess[i].a_symbol->name));
                }
            }
            if (!strcmp(first, "#N")) {
                if (!strcmp(second, "vpatcher")) {
                    if (stackdepth >= MAXSTACK) {
                        post("too many embedded patches");
                        return newb;
                    }
                    stack[stackdepth] = nobj;
                    stackdepth++;
                    nobj = 0;
                    binbuf_addv(newb,"ttfffff;","#N","canvas", GETF(2), GETF(3), GETF(4)-GETF(2), GETF(5)-GETF(3), 10.);
                }
            }
            if (!strcmp(first, "#P")) {
                /* drop initial "hidden" flag */
                if (!strcmp(second, "hidden")) {
                    nextmess++;
                    natom--;
                    second = (nextmess+1)->a_symbol->name;
                }
                if (natom >= 7 && !strcmp(second, "newobj")
                  && (ISSYMBOL(&nextmess[6], "patcher") || ISSYMBOL(&nextmess[6], "p"))) {
                    binbuf_addv(newb,"ttffts;","#X","restore", GETF(2), GETF(3),
                        "pd", atom_getsymbolarg(7, natom, nextmess));
                    if (stackdepth) stackdepth--;
                    nobj = stack[stackdepth];
                    nobj++;
                } else if (!strcmp(second, "newex") || !strcmp(second, "newobj")) {
                    t_symbol *classname = atom_getsymbolarg(6, natom, nextmess);
                    if (classname == gensym("trigger") || classname == gensym("t")) {
                        for (int i=7; i<natom; i++)
                            if (nextmess[i].a_type == A_SYMBOL && nextmess[i].a_symbol == gensym("i"))
                                    nextmess[i].a_symbol = gensym("f");
                    }
                    if (classname == gensym("table")) classname = gensym("TABLE");
                    SETSYMBOL(outmess, gensym("#X"));
                    SETSYMBOL(outmess + 1, gensym("obj"));
                    outmess[2] = nextmess[2];
                    outmess[3] = nextmess[3];
                    SETSYMBOL(outmess+4, classname);
                    for (int i=7; i<natom; i++) outmess[i-2] = nextmess[i];
                    SETSEMI(outmess + natom - 2);
                    binbuf_add(newb, natom - 1, outmess);
                    nobj++;
                } else if (!strcmp(second, "message") || !strcmp(second, "comment")) {
                    SETSYMBOL(outmess, gensym("#X"));
                    SETSYMBOL(outmess + 1, gensym((char *)(strcmp(second, "message") ? "text" : "msg")));
                    outmess[2] = nextmess[2];
                    outmess[3] = nextmess[3];
                    for (int i=6; i<natom; i++) outmess[i-2] = nextmess[i];
                    SETSEMI(outmess + natom - 2);
                    binbuf_add(newb, natom - 1, outmess);
                    nobj++;
                } else if (!strcmp(second, "button")) {
                    binbuf_addv(newb,"ttfft;","#X","obj",GETF(2),GETF(3),"bng");
                    nobj++;
                } else if (!strcmp(second, "number") || !strcmp(second, "flonum")) {
                    binbuf_addv(newb,"ttff;","#X","floatatom",GETF(2),GETF(3));
                    nobj++;
                } else if (!strcmp(second, "slider")) {
                    float inc = GETF(7);
                    if (inc <= 0) inc = 1;
                    binbuf_addv(newb, "ttfftfffffftttfffffffff;","#X","obj",
                        GETF(2), GETF(3), "vsl", GETF(4), GETF(5), GETF(6), GETF(6)+(GETF(5)-1)*inc,
                        0., 0., "empty", "empty", "empty", 0., -8., 0., 8., -262144., -1., -1., 0., 1.);
                    nobj++;
                } else if (!strcmp(second, "toggle")) {
                    binbuf_addv(newb,"ttfft;","#X","obj",GETF(2),GETF(3),"tgl");
                    nobj++;
                } else if (!strcmp(second, "inlet")) {
                    binbuf_addv(newb,"ttfft;","#X","obj",GETF(2),GETF(3), natom > 5 ? "inlet~" : "inlet");
                    nobj++;
                } else if (!strcmp(second, "outlet")) {
                    binbuf_addv(newb,"ttfft;","#X","obj",GETF(2),GETF(3), natom > 5 ? "outlet~" : "outlet");
                    nobj++;
                } else if (!strcmp(second, "user")) {
                    binbuf_addv(newb,"ttffs;","#X","obj", GETF(3), GETF(4), atom_getsymbolarg(2, natom, nextmess));
                    nobj++;
                } else if (!strcmp(second, "connect") || !strcmp(second, "fasten")) {
                    binbuf_addv(newb,"ttffff;","#X","connect", nobj-GETF(2)-1, GETF(3), nobj-GETF(4)-1, GETF(5));
                }
            }
        } else { /* Pd to Max */
            if (!strcmp(first, "#N")) {
                if (!strcmp(second, "canvas")) {
                    if (stackdepth >= MAXSTACK) {
                        post("too many embedded patches");
                        return newb;
                    }
                    stack[stackdepth] = nobj;
                    stackdepth++;
                    nobj = 0;
                    binbuf_addv(newb,"ttffff;","#N","vpatcher", GETF(2), GETF(3), GETF(4), GETF(5));
                }
            }
            if (!strcmp(first, "#X")) {
                if (natom >= 5 && !strcmp(second, "restore") && (ISSYMBOL (&nextmess[4], "pd"))) {
                    binbuf_addv(newb,"tt;","#P","pop");
                    binbuf_addv(newb,"ttffffts;","#P","newobj", GETF(2), GETF(3), 50., 1.,
                        "patcher", atom_getsymbolarg(5, natom, nextmess));
                    if (stackdepth) stackdepth--;
                    nobj = stack[stackdepth];
                    nobj++;
                } else if (!strcmp(second, "obj")) {
                    t_symbol *classname = atom_getsymbolarg(4, natom, nextmess);
                    if      (classname == gensym("inlet"))    binbuf_addv(newb,"ttfff;","#P","inlet", GETF(2), GETF(3), 15.);
                    else if (classname == gensym("inlet~"))   binbuf_addv(newb,"ttffff;","#P","inlet", GETF(2), GETF(3), 15., 1.);
                    else if (classname == gensym("outlet"))   binbuf_addv(newb,"ttfff;","#P","outlet", GETF(2), GETF(3), 15.);
                    else if (classname == gensym("outlet~"))  binbuf_addv(newb,"ttffff;","#P","outlet", GETF(2), GETF(3), 15., 1.);
                    else if (classname == gensym("bng"))      binbuf_addv(newb,"ttffff;","#P","button", GETF(2), GETF(3), GETF(5), 0.);
                    else if (classname == gensym("tgl"))      binbuf_addv(newb,"ttffff;","#P","toggle", GETF(2), GETF(3), GETF(5), 0.);
                    else if (classname == gensym("vsl"))      binbuf_addv(newb,"ttffffff;","#P","slider",
                    	GETF(2), GETF(3), GETF(5), GETF(6), (GETF(8)-GETF(7)) / (GETF(6)==1?1:GETF(6)-1), GETF(7));
                    else {
                    	binbuf_addv(newb,"ttffff","#P","newex", GETF(2), GETF(3), 50., 1.);
                        for (int i=4; i<natom; i++) outmess[i-4] = nextmess[i];
                        binbuf_add(newb, natom-4, outmess);
                        binbuf_addv(newb,";");
                    }
                    nobj++;
                } else if (!strcmp(second, "msg") || !strcmp(second, "text")) {
                    binbuf_addv(newb,"ttffff","#P",strcmp(second, "msg") ? "comment" : "message",GETF(2),GETF(3),50.,1.);
                    for (int i=4; i<natom; i++) outmess[i-4] = nextmess[i];
                    binbuf_add(newb, natom-4, outmess);
                    binbuf_addv(newb,";");
                    nobj++;
                } else if (!strcmp(second, "floatatom")) {
                    binbuf_addv(newb, "ttfff;", "#P", "flonum", GETF(2), GETF(3), 35);
                    nobj++;
                } else if (!strcmp(second, "connect")) {
                    binbuf_addv(newb, "ttffff;", "#P", "connect", nobj-GETF(2)-1, GETF(3), nobj-GETF(4)-1, GETF(5));
                }
            }
        }
        nextindex = endmess + 1;
    }
    if (!maxtopd) binbuf_addv(newb, "tt;", "#P", "pop");
#if 0
    binbuf_write(newb, "import-result.pd", "/tmp", 0);
#endif
    return newb;
}

/* function to support searching */
int binbuf_match(t_binbuf *inbuf, t_binbuf *searchbuf) {
    for (size_t indexin = 0; indexin <= inbuf->n - searchbuf->n; indexin++) {
        for (size_t nmatched = 0; nmatched < searchbuf->n; nmatched++) {
            t_atom *a1 = &inbuf->v[indexin + nmatched], *a2 = &searchbuf->v[nmatched];
            if (a1->a_type != a2->a_type ||
                a1->a_type == A_SYMBOL  && a1->a_symbol != a2->a_symbol ||
                a1->a_type == A_FLOAT   && a1->a_float  != a2->a_float  ||
                a1->a_type == A_DOLLAR  && a1->a_index  != a2->a_index  ||
                a1->a_type == A_DOLLSYM && a1->a_symbol != a2->a_symbol) goto nomatch;
        }
        return 1;
    nomatch: ;
    }
    return 0;
}

/* LATER figure out how to log errors */
void binbuf_evalfile(t_symbol *name, t_symbol *dir) {
    t_binbuf *b = binbuf_new();
    int import = !strcmp(name->name + strlen(name->name) - 4, ".pat");
    /* set filename so that new canvases can pick them up */
    int dspstate = canvas_suspend_dsp();
    glob_setfilename(0, name, dir);
    if (import) {
	if (binbuf_read(b, name->name, dir->name, 0)) {perror(name->name); goto bye;}
	t_binbuf *newb = binbuf_convert(b, 1);
	binbuf_free(b);
	b = newb;
    } else {
	if (binbuf_read(b, name->name, dir->name, 2)) perror(name->name);
    }
bye:
    glob_setfilename(0, &s_, &s_);      /* bug fix by Krzysztof Czaja */
    binbuf_free(b);
    canvas_resume_dsp(dspstate);
}

void glob_evalfile(t_pd *ignore, t_symbol *name, t_symbol *dir) {
    /* even though binbuf_evalfile appears to take care of dspstate, we have to do it again here, because
       canvas_startdsp() assumes that all toplevel canvases are visible. LATER: check if this is still necessary (probably not) */
    int dspstate = canvas_suspend_dsp();
    binbuf_evalfile(name, dir);
    t_pd *x = 0;
    while ((x != s__X.thing) && (x = s__X.thing)) vmess(x, gensym("pop"), "i", 1);
    if (lastpopped) pd_vmess(lastpopped, gensym("loadbang"), "");
    lastpopped = 0;
    canvas_resume_dsp(dspstate);
}

//copied from m_pd.h
#define class_new2(NAME,NU,FREE,SIZE,FLAGS,SIG) class_new2(NAME,(t_newmethod)NU,(t_method)FREE,SIZE,FLAGS,SIG)

extern "C" {
void conf_init();
void glob_init();
void boxes_init();
void garray_init();
void pd_init() {
    object_table = new t_hash<t_pd *,long>(127);
    bindlist_class = class_new(gensym("bindlist"), 0, 0, sizeof(t_bindlist), CLASS_PD, 0);
    class_addbang(bindlist_class, (t_method)bindlist_bang);
    class_addfloat(bindlist_class, (t_method)bindlist_float);
    class_addsymbol(bindlist_class, (t_method)bindlist_symbol);
    class_addpointer(bindlist_class, (t_method)bindlist_pointer);
    class_addlist(bindlist_class, (t_method)bindlist_list);
    class_addanything(bindlist_class, (t_method)bindlist_anything);
    binbuf_class = class_new2("__list", binbuf_new, binbuf_free2, sizeof(t_binbuf), CLASS_PD,   "*");
    wire_class   = class_new2("__wire",   wire_new,   wire_free,  sizeof(t_wire),   CLASS_GOBJ, "*");
    class_setsavefn(wire_class,(t_savefn)wire_save);
    if (pd_objectmaker._class) bug("ARGH");
    for (size_t i=0; i<sizeof(symlist)/sizeof(*symlist); i++) {
        symlist[i]->n = strlen(symlist[i]->name);
        dogensym(symlist[i]->name, symlist[i]->n, symlist[i]); /* why does this take three args? */
    }
    pd_objectmaker._class = class_new2("objectmaker", 0, 0, sizeof(t_pd), CLASS_DEFAULT, "");
    pd_canvasmaker._class = class_new2("canvasmaker", 0, 0, sizeof(t_pd), CLASS_DEFAULT, "");
    pd_bind(&pd_canvasmaker, &s__N);
    class_addanything(pd_objectmaker._class, (t_method)new_anything);
    obj_init();
    conf_init();
    glob_init();
    boxes_init();
    garray_init();
}
};

#ifndef HAVE_ASPRINTF
int asprintf(char **str, const char *fmt, ...)
{
        va_list ap;
        int ret;
        *str = NULL;
        va_start(ap, fmt);
        ret = vasprintf(str, fmt, ap);
        va_end(ap);

        return ret;
}
#endif /* HAVE_ASPRINTF */
#ifndef HAVE_VASPRINTF
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>

#ifndef VA_COPY
# ifdef HAVE_VA_COPY
#  define VA_COPY(dest, src) va_copy(dest, src)
# else
#  ifdef HAVE___VA_COPY
#   define VA_COPY(dest, src) __va_copy(dest, src)
#  else
#   define VA_COPY(dest, src) (dest) = (src)
#  endif
# endif
#endif

#define INIT_SZ 128

int vasprintf(char **str, const char *fmt, va_list ap)
{
        int ret = -1;
        va_list ap2;
        char *string, *newstr;
        size_t len;

        VA_COPY(ap2, ap);
        if ((string = (char *)malloc(INIT_SZ)) == NULL)
                goto fail;

        ret = vsnprintf(string, INIT_SZ, fmt, ap2);
        if (ret >= 0 && ret < INIT_SZ) { /* succeeded with initial alloc */
                *str = string;
        } else if (ret == INT_MAX || ret < 0) { /* Bad length */
                goto fail;
        } else {        /* bigger than initial, realloc allowing for nul */
                len = (size_t)ret + 1;
                if ((newstr = (char *)realloc(string, len)) == NULL) {
                        free(string);
                        goto fail;
                } else {
                        va_end(ap2);
                        VA_COPY(ap2, ap);
                        ret = vsnprintf(newstr, len, fmt, ap2);
                        if (ret >= 0 && (size_t)ret < len) {
                                *str = newstr;
                        } else { /* failed with realloc'ed string, give up */
                                free(newstr);
                                goto fail;
                        }
                }
        }
        va_end(ap2);
        return (ret);

fail:
        *str = NULL;
        errno = ENOMEM;
        va_end(ap2);
        return (-1);
}
#endif /* HAVE_VASPRINTF */
