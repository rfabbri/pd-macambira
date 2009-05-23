/*
  This file is part of PureData
  Copyright 2004-2006 by Mathieu Bouchard
  Copyright 2000-2001 IEM KUG Graz Austria (Thomas Musil)
  Copyright (c) 1997-1999 Miller Puckette.
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "LICENSE.txt", in this distribution.

  this file declares the C interface for the second half of DesireData.
  this is where most of the differences between DesireData and PureData lie.

  SYNONYMS:
    1. glist = graph = canvas = patcher; those were not always synonymous.
       however, graph sometimes means a canvas that is displaying an array.
    2. outconnect = connection = patchcord = wire

  canvases have a few flags that determine their appearance:
    gl_havewindow: should open its own window (only if mapped? or...)
    gl_isgraph:    the GOP flag: show as graph-on-parent, vs TextBox.
    gl_owner==0:   it's a root canvas, should be in canvas_list.
                   In this case "gl_havewindow" is always set.

  canvas_list is a list of root windows only, which can be traversed using canvases_each.

  If a canvas has a window it may still not be "mapped."  Miniaturized
  windows aren't mapped, for example, but a window is also not mapped
  immediately upon creation.  In either case gl_havewindow is true but
  gl_mapped is false.

  Closing a non-root window makes it invisible; closing a root destroys it.
  A canvas that's just a text object on its parent is always "toplevel."  An
  embedded canvas can switch back and forth to appear as a toplevel by double-
  clicking on it.  Single-clicking a text box makes the toplevel become visible
  and raises the window it's in.

  If a canvas shows up as a graph on its parent, the graph is blanked while the
  canvas has its own window, even if miniaturized.
*/

#ifndef DESIRE
#define DESIRE
#endif
#ifndef __DESIRE_H
#define __DESIRE_H

#include "m_pd.h"
#include "s_stuff.h"

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
//#include <map>
extern "C" {
#endif

/* ----------------------- (v)asprintf missing on mingw... ---------------------------------------------------*/

#ifndef HAVE_VASPRINTF
extern int vasprintf(char **str, const char *fmt, va_list ap) throw ();
#endif

#ifndef HAVE_ASPRINTF
extern int asprintf(char **str, const char *fmt, ...) throw ();
#endif

/* ----------------------- m_imp.h ---------------------------------------------------*/

typedef struct _methodentry {
    t_symbol *me_name;
    t_gotfn me_fun;
    t_atomtype me_arg[MAXPDARG+1];
} t_methodentry;

typedef void (*t_bangmethod)   (t_pd *x);
typedef void (*t_pointermethod)(t_pd *x, t_gpointer *gp);
typedef void (*t_floatmethod)  (t_pd *x, t_float f);
typedef void (*t_symbolmethod) (t_pd *x, t_symbol *s);
typedef void (*t_stringmethod) (t_pd *x, const char *s);
typedef void (*t_listmethod)   (t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef void (*t_anymethod)    (t_pd *x, t_symbol *s, int argc, t_atom *argv);

t_pd *pd_new2(int argc, t_atom *argv);
t_pd *pd_new3(const char *s);

struct _class {
    t_symbol *name;                   /* name (mostly for error reporting) */
    t_symbol *helpname;               /* name of help file */
    t_symbol *externdir;              /* directory extern was loaded from */
    size_t size;                      /* size of an instance */
    t_methodentry *methods;           /* methods other than bang, etc below */
    int nmethod;                      /* number of methods */
    t_method        freemethod;       /* function to call before freeing */
    t_bangmethod    bangmethod;       /* common methods */
    t_pointermethod pointermethod;
    t_floatmethod   floatmethod;
    t_symbolmethod  symbolmethod; /* or t_stringmethod, but only C++ has anonymous unions, so... */
    t_listmethod    listmethod;
    t_anymethod     anymethod;
    t_savefn savefn;           /* function to call when saving */
    int floatsignalin;         /* onset to float for signal input */
    unsigned gobj:1;           /* true if is a gobj */
    unsigned patchable:1;      /* true if we have a t_object header */
    unsigned firstin:1;        /* if patchable, true if draw first inlet */
    unsigned drawcommand:1;    /* a drawing command for a template */
    unsigned newatoms:1;         /* can handle refcounting of atoms (future use) */
    unsigned use_stringmethod:1; /* the symbolmethod slot holds a stringmethod instead */
    t_symbol *firsttip;
    t_symbol **fields; /* names of fields aka attributes, and I don't mean the #V attributes. */
    int nfields; /* ... and how many of them */
    t_notice notice;             /* observer method */
    t_onsubscribe onsubscribe; /* observable method */
};

//#define c_methods methods /* for Cyclone */
//#define c_nmethod nmethod /* for Cyclone */
//#define c_externdir externdir /* for PDDP */
//#define c_name name /* for Cyclone,Creb,Pidip */
//#define c_size size /* for Cyclone,Flext */
//#define me_name name /* for Cyclone */
//#define me_fun fun /* for Cyclone */
//#define me_arg arg /* for Cyclone */

/* m_obj.c */
EXTERN int obj_noutlets(t_object *x);
EXTERN int obj_ninlets(t_object *x);
EXTERN t_outconnect *obj_starttraverseoutlet(t_object *x, t_outlet **op, int nout);
EXTERN t_outconnect *obj_nexttraverseoutlet(t_outconnect *lastconnect, t_object **destp, t_inlet **inletp, int *whichp);
EXTERN t_outconnect *obj_connect(t_object *source, int outno, t_object *sink, int inno);
EXTERN void obj_disconnect(t_object *source, int outno, t_object *sink, int inno);
EXTERN void outlet_setstacklim(void);
EXTERN int obj_issignalinlet(t_object *x, int m);
EXTERN int obj_issignaloutlet(t_object *x, int m);
EXTERN int obj_nsiginlets(t_object *x);
EXTERN int obj_nsigoutlets(t_object *x);
EXTERN int obj_siginletindex(t_object *x, int m);
EXTERN int obj_sigoutletindex(t_object *x, int m);

/* misc */
EXTERN void glob_evalfile(t_pd *ignore, t_symbol *name, t_symbol *dir);
EXTERN void glob_initfromgui(void *dummy, t_symbol *s);
EXTERN void glob_quit(void *dummy);

/* ----------------------- g_canvas.h ------------------------------------------------*/

/* i don't know whether this is currently used at all in DesireData. -- matju 2006.09 */
#ifdef GARRAY_THREAD_LOCK
#include <pthread.h> /* TB: for t_garray */
#endif

typedef struct t_gtemplate t_gtemplate;
typedef struct _canvasenvironment t_canvasenvironment;
typedef struct _slot t_slot;

/* the t_tick structure describes where to draw x and y "ticks" for a canvas */
typedef struct _tick {  /* where to put ticks on x or y axes */
    float point;      /* one point to draw a big tick at */
    float inc;        /* x or y increment per little tick */
    int lperb;        /* little ticks per big; 0 if no ticks to draw */
} t_tick;

typedef struct t_boxes t_boxes;

/* the t_canvas structure, which describes a list of elements that live on an area of a window.*/
#ifdef PD_PLUSPLUS_FACE
struct _glist : t_object {
#else
struct _glist {
    t_object gl_obj;         /* header in case we're a [pd] or abstraction */
#endif
    int pixwidth, pixheight; /* width in pixels (on parent, if a graph) */
    float x1,y1,x2,y2;       /* bounding rectangle in our own coordinates */
    int screenx1, screeny1, screenx2, screeny2; /* screen coordinates when toplevel */
    int xmargin, ymargin;    /* origin for GOP rectangle */
    /* ticks and tick labels */
    t_tick xtick; int nxlabels; t_symbol **xlabel; float xlabely;
    t_tick ytick; int nylabels; t_symbol **ylabel; float ylabelx;
    t_symbol *name;            /* symbol bound here */
    int font;                  /* nominal font size in points, e.g., 10 */
    t_canvasenvironment *env;  /* one of these per $0; env=0 for subpatches */
    unsigned int havewindow:1; /* this indicates whether we publish to the manager */
    unsigned int gop:1;
    unsigned int goprect:1;    /* gop version >= 0.39 */
    unsigned int hidetext:1;   /* hide object-name + args when doing graph on parent */
    long next_o_index;         /* next object index. to be incremented on each use */
    long next_w_index;         /* next wire   index. to be incremented on each use */
    t_boxes *boxes;
};

/* LATER consider adding font size to this struct (see canvas_getfont()) */
struct _canvasenvironment {
    t_symbol *dir;   /* directory patch lives in */
    int argc;        /* number of "$" arguments */
    t_atom *argv;    /* array of "$" arguments */
    long dollarzero; /* value of "$0" */
    t_namelist *path;/* search path (0.40) */
};

/* a data structure to describe a field in a pure datum */
#define DT_FLOAT 0
#define DT_SYMBOL 1
#define DT_CANVAS 2
#define DT_ARRAY 3

typedef struct t_dataslot {
    int type;
    t_symbol *name;
    t_symbol *arraytemplate;     /* filled in for arrays only */
} t_dataslot;

#ifdef PD_PLUSPLUS_FACE
typedef struct t_template : t_pd {
#else
typedef struct t_template {
    t_pd t_pdobj;               /* header */
#endif
    t_gtemplate *list;  /* list of "struct"/gtemplate objects */
    t_symbol *sym;            /* name */
    int n;                    /* number of dataslots (fields) */
    t_dataslot *vec;          /* array of dataslots */
} t_template;

/* this is not really a t_object, but it needs to be observable and have a refcount, so... */
#ifdef PD_PLUSPLUS_FACE
struct _array : t_object {
#else
struct _array {
    t_gobj gl_obj;
#endif
    int n;            /* number of elements */
    int elemsize;     /* size in bytes; LATER get this from template */
    char *vec;        /* array of elements */
    union {
      t_symbol *tsym;    /* template for elements */
      t_symbol *templatesym; /* alias */
    };
    t_gpointer gp;    /* pointer to scalar or array element we're in */
};

#ifdef PD_PLUSPLUS_FACE
struct _garray : t_gobj {
#else
struct _garray {
    t_gobj x_gobj;
#endif
    t_scalar *scalar;     /* scalar "containing" the array */
    t_canvas *canvas;     /* containing canvas */
    t_symbol *name;       /* unexpanded name (possibly with leading '$') */
    t_symbol *realname;   /* expanded name (symbol we're bound to) */
    unsigned int usedindsp:1;   /* true if some DSP routine is using this */
    unsigned int saveit:1;      /* true if we should save this with parent */
    unsigned int listviewing:1; /* true if list view window is open */
    unsigned int hidename:1;    /* don't print name above graph */
};

t_array *garray_getarray(t_garray *x);

/* structure for traversing all the connections in a canvas */
typedef struct t_linetraverser {
    t_canvas *canvas;
    t_object *from; int nout; int outlet; t_outlet *outletp;
    t_object *to;   int nin;  int inlet;  t_inlet  *inletp;
    t_outconnect *next;
    int nextoutno;
#ifdef __cplusplus
    t_linetraverser() {}
    t_linetraverser(t_canvas *canvas);
#endif
} t_linetraverser;

extern t_canvas *canvas_list; /* list of all root canvases */
extern t_class *vinlet_class, *voutlet_class, *canvas_class;
extern int canvas_valid;         /* incremented when pointers might be stale */
EXTERN int sys_noloadbang;
EXTERN t_symbol *s_empty;

#define PLOTSTYLE_POINTS 0     /* plotting styles for arrays */
#define PLOTSTYLE_POLY 1
#define PLOTSTYLE_BEZ 2

/* from kernel.c */
EXTERN void gobj_save(t_gobj *x, t_binbuf *b);
EXTERN void pd_eval_text(const char *t, size_t size);
EXTERN int sys_syntax;

/* from desire.c */
EXTERN int pd_scanargs(int argc, t_atom *argv, const char *fmt, ...);
EXTERN int pd_saveargs(t_binbuf *b, const char *fmt, ...);
EXTERN void pd_upload(t_gobj *self);
EXTERN void sys_mgui(void *self,   const char *sel, const char *fmt, ...);
EXTERN void canvas_add(t_canvas *x, t_gobj *g, int index=-1);
EXTERN void canvas_delete(t_canvas *x, t_gobj *y);
EXTERN void canvas_deletelinesfor(t_canvas *x, t_text *text);
EXTERN void canvas_deletelinesforio(t_canvas *x, t_text *text, t_inlet *inp, t_outlet *outp);
EXTERN int canvas_isvisible(t_canvas *x);
EXTERN int canvas_istoplevel(t_canvas *x);
EXTERN int canvas_istable(t_canvas *x);
EXTERN int canvas_isabstraction(t_canvas *x);
EXTERN t_canvas *canvas_getcanvas(t_canvas *x);
EXTERN t_canvas *canvas_getrootfor(t_canvas *x);
EXTERN t_canvas *canvas_getcurrent(void);
EXTERN t_canvasenvironment *canvas_getenv(t_canvas *x);
EXTERN int canvas_getdollarzero(void);
EXTERN t_symbol *canvas_realizedollar(t_canvas *x, t_symbol *s);
EXTERN t_symbol *canvas_makebindsym(t_symbol *s);

EXTERN void linetraverser_start(t_linetraverser *t, t_canvas *x);
EXTERN t_outconnect *linetraverser_next(t_linetraverser *t);

/* -------------------- TO BE SORTED OUT --------------------- */
EXTERN void canvas_redrawallfortemplatecanvas(t_canvas *x, int action);
EXTERN void array_resize(t_array *x, int n);
EXTERN void word_init(t_word *wp, t_template *tmpl, t_gpointer *gp);
EXTERN void word_restore(t_word *wp, t_template *tmpl, int argc, t_atom *argv);
EXTERN t_scalar *scalar_new(t_canvas *owner, t_symbol *templatesym);
EXTERN void word_free(t_word *wp, t_template *tmpl);
EXTERN void scalar_redraw(t_scalar *x, t_canvas *canvas);
//EXTERN int pd_pickle(t_foo *foo, const char *fmt, ...);
EXTERN void pd_set_newest (t_pd *x);
EXTERN const char *inlet_tip(t_inlet* i,int num);

extern t_hash<t_pd *,long> *object_table;
extern t_hash<t_symbol *,t_class *> *class_table;

/* some kernel.c stuff that wasn't in any header, when shifting to C++. */
void obj_moveinletfirst(t_object *x, t_inlet *i);
void obj_moveoutletfirst(t_object *x, t_outlet *o);
int inlet_getsignalindex(t_inlet *x);
int outlet_getsignalindex(t_outlet *x);
void text_save(t_gobj *z, t_binbuf *b);
t_sample *obj_findsignalscalar(t_object *x, int m);
void class_set_extern_dir(t_symbol *s);
void glob_update_class_info (t_pd *bogus, t_symbol *s, t_symbol *cb_recv, t_symbol *cb_sel);
void pd_free_zombie(t_pd *x);

/* some other stuff that wasn't in any header */
void glob_watchdog(t_pd *dummy);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif

#ifdef __cplusplus
#include<iostream>
template <class T> static T min(T a, T b) {return a<b?a:b;}
template <class T> static T max(T a, T b) {return a>b?a:b;}
template <class T> T clip(T a, T b, T c) {return min(max(a,b),c);}
void  oprintf(std::ostream &buf, const char *s, ...);
void voprintf(std::ostream &buf, const char *s, va_list args);
EXTERN std::ostringstream lost_posts;
#endif

#define L post("%s:%d in %s",__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define LS(self) post("%s:%d in %s (self=%lx class=%s)",__FILE__,__LINE__,__PRETTY_FUNCTION__,(long)self, \
	((t_gobj *)self)->_class->name->name);

#define STACKSIZE 1024
struct t_call {
	t_pd *self;  /* receiver */
	t_symbol *s; /* selector */
	/* insert temporary profiling variables here */
};

EXTERN t_call pd_stack[STACKSIZE];
EXTERN int pd_stackn;

EXTERN int gstack_empty(); /* that's a completely different stack: see pd_pushsym,pd_popsym */

class Error {};
class VeryUnlikelyError : Error {};
inline static int throw_if_negative(int n) {if (n<0) throw VeryUnlikelyError(); else return n;}
#define  asprintf(ARGS...) throw_if_negative( asprintf(ARGS))
#define vasprintf(ARGS...) throw_if_negative(vasprintf(ARGS))

#endif /* __DESIRE_H */
