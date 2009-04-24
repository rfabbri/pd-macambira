/* $Id$
   This file is part of DesireData.

   Copyright (c) 2006,2007,2009 Mathieu Bouchard.
   Copyright (c) 1997-1999 Miller Puckette.
   For information on usage and redistribution, and for a DISCLAIMER OF ALL
   WARRANTIES, see the file, "LICENSE.txt", in this distribution.
*/
/*
   PD_PLUSPLUS_FACE is not considered as part of the main interface for externals,
   even though it has become the main interface for internals. please don't rely on
   it outside of the desiredata source code (yet).
*/

#ifndef __m_pd_h_

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
#include <iostream>

#ifdef __cplusplus
template <class K, class V> class t_hash /* : t_pd */ {
	struct entry {
		K k;
		V v;
		struct entry *next;
	};
	long capa;
	long n;
	entry **tab;
/* when iterating: */
	long i;
	entry *m_next;
public:
	t_hash(long capa_) : capa(capa_), n(0), i(0) {
		tab = new entry *[capa];
		for (long j=0; j<capa; j++) tab[j]=0;
	}
	~t_hash() {
		for (long j=0; j<capa; j++) while (tab[j]) del(tab[j]->k);
		delete[] tab;
	}
	size_t size() {return n;}
	V get(K k) {
		long h = hash(k);
		for (entry *e=tab[h]; e; e=e->next) {if (e->k==k) return e->v;}
		return 0;
	}
	void set(K k, V v) {
		long h = hash(k);
		for (entry *e=tab[h]; e; e=e->next) {if (e->k==k) {e->v=v; return;}}
		entry *nu = new entry; nu->k=k; nu->v=v; nu->next=tab[h];
		n++;
		tab[h] = nu;
	}
	V del(K k) {
		long h = hash(k);
		for (entry **ep=&tab[h]; *ep; ep=&(*ep)->next) {
			if ((*ep)->k==k) {
				V v=(*ep)->v;
				entry *next=(*ep)->next;
				delete *ep; n--;
				*ep = next;
				return v;
			}
		}
		return 0;
	}
	int    exists(K k) {
		long h = hash(k);
		for (entry *e=tab[h]; e; e=e->next) {if (e->k==k) return 1;}
		return 0;
	}
	void start() {i=0; m_next=0;}
	int  next(K *kp, V *vp) {
		while (!m_next && i<capa) m_next = tab[i++];
		if (m_next) {
			*kp = m_next->k;
			*vp = m_next->v;
			m_next = m_next->next;
			return 1;
		}
		return 0;
	}
	#define hash_foreach(k,v,h) for (h->start(); h->next(&k,&v); )
	long hash(K k) {return (((long)k*0x54321) & 0x7FFFFFFF)%capa;}
};
#endif

extern "C" {
#endif

#define PD_MAJOR_VERSION 1
#define PD_MINOR_VERSION 0
#define PD_DEVEL_VERSION 0
#define PD_TEST_VERSION ""

/* old name for "MSW" flag -- we have to take it for the sake of many old
"nmakefiles" for externs, which will define NT and not MSW */
#if (defined(_WIN32) || defined(NT)) && !defined(MSW)
#define MSW
#endif

#ifdef __CYGWIN__
#define UNISTD
#endif

#ifdef _MSC_VER
/* #pragma warning( disable : 4091 ) */
#pragma warning( disable : 4305 )  /* uncast const double to float */
#pragma warning( disable : 4244 )  /* uncast float/int conversion etc. */
#pragma warning( disable : 4101 )  /* unused automatic variables */
#endif /* _MSC_VER */

    /* the external storage class is "extern" in GCC; in MS-C it's ugly. */
#if defined(MSW) && !defined (__GNUC__)
#ifdef PD_INTERNAL
#define EXTERN __declspec(dllexport) extern
#else
#define EXTERN __declspec(dllimport) extern
#endif /* PD_INTERNAL */
#else
#define EXTERN extern
#endif /* MSW */

#if !defined(_SIZE_T) && !defined(_SIZE_T_)
#include <stddef.h>
#endif

#include <stdarg.h>

#define MAXPDSTRING 1000        /* use this for anything you want */
#define MAXPDARG 5              /* max number of args we can typecheck today */

/* signed and unsigned integer types the size of a pointer:  */
/* GG: long is the size of a pointer */
typedef long t_int;

typedef float t_float;  /* a floating-point number at most the same size */
typedef float t_floatarg;  /* floating-point type for function calls */

typedef struct _class t_class;
typedef struct _outlet t_outlet;
typedef struct _inlet t_inlet;
typedef struct _clock t_clock;
typedef struct _glist t_glist, t_canvas;

#ifdef PD_PLUSPLUS_FACE
struct t_pd {
	t_class *_class;
};
#else
typedef t_class *t_pd;      /* pure datum: nothing but a class pointer */
#endif

typedef struct _symbol {
    char *name;           /* the const string that represents this symbol */
    t_pd *thing;          /* pointer to the target of a receive-symbol or to the bindlist of several targets */
    struct _symbol *next; /* brochette of all symbols (only for permanent symbols) */
    size_t refcount;      /* refcount<0 means that the symbol is permanent */
    size_t n;             /* size of name (support for NUL characters) */
} t_symbol;
#define s_name  name
#define s_thing thing
#define s_next  next

#ifdef PD_PLUSPLUS_FACE
typedef struct t_list : t_pd {
#else
typedef struct t_list {
    t_pd *pd;
#endif
    struct _atom *v;
    size_t capa;
    size_t refcount;
    size_t n;
} t_list, t_binbuf;

typedef struct _array t_array;   /* g_canvas.h */

/* pointers to glist and array elements go through a "stub" which sticks
around after the glist or array is freed.  The stub itself is deleted when
both the glist/array is gone and the refcount is zero, ensuring that no
gpointers are pointing here. */

#ifdef PD_PLUSPLUS_FACE
typedef struct _gpointer {
    union {
        struct _scalar *scalar;  /* scalar we're in (if glist) */
        union word *w;           /* raw data (if array) */
    };
    union {
        struct _glist *canvas;
        struct _array *array;
        struct t_text *o;
    };
    size_t dummy;
    size_t refcount;
//#define gs_refcount refcount
#define gs_refcount canvas->refcount
} t_gpointer;
#else
typedef struct _gpointer {
    union {
        struct _scalar *gp_scalar;  /* scalar we're in (if glist) */
        union word *gp_w;           /* raw data (if array) */
    } gp_un;
    union {
        struct _glist *canvas;
        struct _array *array;
        struct _text *o;
    } un;
    size_t dummy;
    size_t refcount;
//#define gs_refcount un.canvas->gl_obj.refcount
#define gs_refcount refcount
} t_gpointer;
#endif

#define w_list w_canvas
typedef union word {
    t_float w_float;        /* A_FLOAT   */
    t_symbol *w_symbol;     /* A_SYMBOL or A_DOLLSYM  */
    t_gpointer *w_gpointer; /* A_POINTER */
    t_array *w_array;       /* DT_ARRAY */
    struct _glist *w_canvas; /* DT_LIST */
    t_int w_index;          /* A_SEMI or A_COMMA or A_DOLLAR */
} t_word;

typedef enum {
    A_NULL, /* non-type: represents end of typelist */
    A_FLOAT, A_SYMBOL, A_POINTER, /* stable elements */
    A_SEMI, A_COMMA, /* radioactive elements of the first kind */
    A_DEFFLOAT, A_DEFSYM, /* pseudo-types for optional (DEFault) arguments */
    A_DOLLAR, A_DOLLSYM, /* radioactive elements of the second kind */
    A_GIMME,   /* non-type: represents varargs */
    A_CANT,    /* bottom type: type constraint is impossible */
/* regular pd stops here, before #12 */
    A_ATOM,    /* top type:    type constraint doesn't constrain */
    A_LIST,    /* t_list *, the 4th stable element (future use) */
    A_GRID,    /* reserved for GridFlow */
    A_GRIDOUT, /* reserved for GridFlow */
}  t_atomtype;

#define A_DEFSYMBOL A_DEFSYM    /* better name for this */

typedef struct _atom {
    t_atomtype a_type;
    union word a_w;
#ifdef __cplusplus
    operator float     () {return a_w.w_float;}
    operator t_symbol *() {return a_w.w_symbol;}
    //bool operator == (_atom &o) {return a_type==o.a_type && a_w.w_index==o.a_w.w_index;}
    //bool operator != (_atom &o) {return a_type!=o.a_type || a_w.w_index!=o.a_w.w_index;}
#endif
} t_atom;

struct t_arglist {long c; t_atom v[0];};

typedef unsigned long long uint64;

/* t_appendix is made of the things that logically ought to be in t_gobj but have been put in a
   separate memory space because this allows most externals to work unmodified on both DesireData
   and non-DesireData systems. The equivalent in the Tcl side is really part of every view object. */
typedef struct t_appendix {
	t_canvas *canvas; /* the holder of this object */
/* actual observable */
	size_t nobs;           /* number of spies */
	struct _gobj **obs; /* I spy with my little I */
/* miscellaneous */
#ifdef __cplusplus
	t_hash<t_symbol *, t_arglist *> *visual;
#else
	void *visual;
#endif
	long index; /* index of an object within its canvas scope. */
	uint64 elapsed;
} t_appendix;
t_appendix *appendix_new (struct _gobj *master);
void appendix_free(struct _gobj *self);
void appendix_save(struct _gobj *master, t_binbuf *b);

#ifdef PD_PLUSPLUS_FACE
#define g_pd _class
typedef struct _gobj : t_pd {
#else
typedef struct _gobj        /* a graphical object */
{
    t_pd g_pd;              /* pure datum header (class) */
#endif
    t_appendix *dix;
#define g_adix dix
#ifdef PD_PLUSPLUS_FACE
    _gobj *next();
#endif
} t_gobj;

#ifdef PD_PLUSPLUS_FACE
typedef struct _outconnect : _gobj {
    struct _outconnect *next;
    t_pd *oc_to;
    struct t_text *from;
    struct t_text *to;
    short outlet, inlet;
} t_outconnect, t_wire;
#define oc_next next
#else
typedef struct _outconnect t_outconnect;
#endif

#ifdef PD_PLUSPLUS_FACE
typedef struct _scalar : t_gobj {
#else
typedef struct _scalar      /* a graphical object holding data */
{
    t_gobj sc_gobj;         /* header for graphical object */
#endif
    t_symbol *t; /* template name (LATER replace with pointer) */
    t_word v[1]; /* indeterminate-length array of words */
} t_scalar;

#define sc_template t
#define sc_vec v

#ifdef PD_PLUSPLUS_FACE
typedef struct t_text : t_gobj {
#else
typedef struct _text     /* patchable object - graphical, with text */
{
    t_gobj te_g;         /* header for graphical object */
#endif
    t_binbuf *binbuf;    /* holder for the text */
    t_outlet *outlet;    /* linked list of outlets */
    t_inlet *inlet;      /* linked list of inlets */
    short x,y;           /* x&y location (within the toplevel) */
    int refcount;        /* there used to be a bitfield here, which may be a problem with ms-bitfields (?) */
#ifdef PD_PLUSPLUS_FACE
    t_inlet  * in(int n);
    t_outlet *out(int n);
#endif
} t_text, t_object;

#define te_binbuf binbuf
#define te_outlet outlet
#define te_inlet  inlet
#define te_xpix   x
#define te_ypix   y
#define te_width  width
#define te_type   type

#define ob_outlet te_outlet
#define ob_inlet te_inlet
#define ob_binbuf te_binbuf
#ifdef PD_PLUSPLUS_FACE
#define te_pd g_pd
#define ob_pd g_pd
#else
#define te_pd te_g.g_pd
#define ob_pd te_g.g_pd
#endif
#define ob_g te_g

/* don't take those three types for cash (on average). they're lying because the type system can't express what there is to be expressed. */
typedef void (*t_method)(void);
typedef void *(*t_newmethod)(void);
typedef void (*t_gotfn)(void *x, ...);

/* ---------------- pre-defined objects and symbols --------------*/
EXTERN t_pd pd_objectmaker;     /* factory for creating "object" boxes */
EXTERN t_pd pd_canvasmaker;     /* factory for creating canvases */
EXTERN t_symbol s_pointer, s_float, s_symbol;
EXTERN t_symbol s_bang, s_list, s_anything, s_signal;
EXTERN t_symbol s__N, s__X, s_x, s_y, s_;

/* --------- central message system ----------- */
EXTERN void pd_typedmess(t_pd *x, t_symbol *s, int argc, t_atom *argv);
EXTERN void pd_forwardmess(t_pd *x, int argc, t_atom *argv);
EXTERN t_symbol *gensym(const char *s);
EXTERN t_symbol *gensym2(const char *s, size_t n);
EXTERN t_symbol *symprintf(const char *s, ...);
EXTERN t_gotfn  getfn(t_pd *x, t_symbol *s);
EXTERN t_gotfn zgetfn(t_pd *x, t_symbol *s);
EXTERN void nullfn(void);
EXTERN void pd_vmess(t_pd *x, t_symbol *s, const char *fmt, ...);
#define mess0(x,s)           (getfn((x),(s))((x)))
#define mess1(x,s,a)         (getfn((x),(s))((x),(a)))
#define mess2(x,s,a,b)       (getfn((x),(s))((x),(a),(b)))
#define mess3(x,s,a,b,c)     (getfn((x),(s))((x),(a),(b),(c)))
#define mess4(x,s,a,b,c,d)   (getfn((x),(s))((x),(a),(b),(c),(d)))
#define mess5(x,s,a,b,c,d,e) (getfn((x),(s))((x),(a),(b),(c),(d),(e)))
EXTERN void obj_list(t_object *x, t_symbol *s, int argc, t_atom *argv);
EXTERN t_pd *pd_newest(void); /* multiclient race conditions? */

/* --------------- memory management -------------------- */
EXTERN void *getbytes(size_t nbytes); /* deprecated, use malloc() */
EXTERN void *copybytes(void *src, size_t nbytes);
EXTERN void freebytes(void *x, size_t nbytes); /* deprecated, use free() */
EXTERN void *resizebytes(void *x, size_t oldsize, size_t newsize); /* deprecated, use realloc() */

/* T.Grill - functions for aligned memory (according to CPU SIMD architecture) */
EXTERN void *getalignedbytes(size_t nbytes);
EXTERN void *copyalignedbytes(void *src, size_t nbytes);
EXTERN void freealignedbytes(void *x,size_t nbytes);
EXTERN void *resizealignedbytes(void *x,size_t oldsize, size_t newsize);
/* -------------------- atoms ----------------------------- */

#define atom_decref(atom) (void)42
#define atom_incref(atom) (void)42
#define SETATOM(atom,type,field,value) (atom_decref(atom), (atom)->a_type=type, (atom)->a_w.field=value, atom_incref(atom))
#define SETSEMI(atom)        SETATOM(atom,A_SEMI,   w_index,0)
#define SETCOMMA(atom)       SETATOM(atom,A_COMMA,  w_index,0)
#define SETPOINTER(atom, gp) SETATOM(atom,A_POINTER,w_gpointer,(gp)) /* looks unsafe... */
#define SETFLOAT(atom, f)    SETATOM(atom,A_FLOAT,  w_float,(f))
#define SETSYMBOL(atom, s)   SETATOM(atom,A_SYMBOL, w_symbol,(s))
#define SETSTRING(atom, s)   SETATOM(atom,A_SYMBOL, w_symbol,gensym(s))
#define SETDOLLAR(atom, n)   SETATOM(atom,A_DOLLAR, w_index,(n))
#define SETDOLLSYM(atom, s)  SETATOM(atom,A_DOLLSYM,w_symbol,(s))

EXTERN t_float     atom_getfloat( t_atom *a);
EXTERN t_int       atom_getint(   t_atom *a);
EXTERN t_symbol   *atom_getsymbol(t_atom *a); /* this call causes a t_symbol to become æternal */
EXTERN const char *atom_getstring(t_atom *a); /* the return value should be used only immediately */
EXTERN t_float     atom_getfloatarg( int which, int argc, t_atom *argv);
EXTERN t_int       atom_getintarg(   int which, int argc, t_atom *argv);
EXTERN t_symbol   *atom_getsymbolarg(int which, int argc, t_atom *argv);
EXTERN const char *atom_getstringarg(int which, int argc, t_atom *argv); /* see above */

EXTERN t_symbol   *atom_gensym(   t_atom *a);

/* this function should produce a literal, whereas getstring gives the exact string */
EXTERN void atom_string(t_atom *a, char *buf, unsigned int bufsize);
#ifdef __cplusplus
EXTERN void atom_ostream(t_atom *a, std::ostream &buf);
inline std::ostream &operator <<(std::ostream &buf, t_atom *a) {atom_ostream(a,buf); return buf;}
#endif

/* goes with desiredata's CLASS_NEWATOM */
EXTERN void atom_init(t_atom *a, size_t n);
EXTERN void atom_copy(t_atom *a, t_atom *b, size_t n);
EXTERN void atom_delete(t_atom *a, size_t n);

/* ------------------  binbufs --------------- */

EXTERN t_binbuf *binbuf_new(void);
EXTERN void binbuf_free(t_binbuf *x);
EXTERN t_binbuf *binbuf_duplicate(t_binbuf *y);

EXTERN void binbuf_text(t_binbuf *x, const char *text, size_t size);
EXTERN void binbuf_gettext(t_binbuf *x, char **bufp, int *lengthp);
EXTERN char *binbuf_gettext2(t_binbuf *x);
EXTERN void binbuf_clear(t_binbuf *x);
EXTERN void binbuf_add(t_binbuf *x, int argc, t_atom *argv);
EXTERN void binbuf_addv(t_binbuf *x, const char *fmt, ...);
EXTERN void binbuf_addbinbuf(t_binbuf *x, t_binbuf *y);
EXTERN void binbuf_addsemi(t_binbuf *x);
EXTERN void binbuf_restore(t_binbuf *x, int argc, t_atom *argv);
EXTERN void binbuf_print(t_binbuf *x);
EXTERN int binbuf_getnatom(t_binbuf *x);
EXTERN t_atom *binbuf_getvec(t_binbuf *x);
EXTERN void binbuf_eval(t_binbuf *x, t_pd *target, int argc, t_atom *argv);
EXTERN int binbuf_read(           t_binbuf *b, char *filename, char *dirname,    int flags);
EXTERN int binbuf_read_via_canvas(t_binbuf *b, char *filename, t_canvas *canvas, int flags);
EXTERN int binbuf_read_via_path(  t_binbuf *b, char *filename, char *dirname,    int flags);
EXTERN int binbuf_write(          t_binbuf *x, char *filename, char *dir,        int flags);
EXTERN void binbuf_evalfile(t_symbol *name, t_symbol *dir);
EXTERN t_symbol *binbuf_realizedollsym(t_symbol *s, int ac, t_atom *av, int tonew);

/* ------------------  clocks --------------- */

EXTERN t_clock *clock_new(void *owner, t_method fn);
EXTERN void clock_set(t_clock *x, double systime);
EXTERN void clock_delay(t_clock *x, double delaytime);
EXTERN void clock_unset(t_clock *x);
EXTERN double clock_getlogicaltime(void);
EXTERN double clock_getsystime(void); /* OBSOLETE; use clock_getlogicaltime() */
EXTERN double clock_gettimesince(double prevsystime);
EXTERN double clock_getsystimeafter(double delaytime);
EXTERN void clock_free(t_clock *x);

/* ----------------- pure data ---------------- */
EXTERN t_pd *pd_new(t_class *cls);
EXTERN void pd_free(t_pd *x);
EXTERN void pd_bind(  t_pd *x, t_symbol *s);
EXTERN void pd_unbind(t_pd *x, t_symbol *s);
EXTERN t_pd *pd_findbyclass(t_symbol *s, t_class *c);
EXTERN void pd_pushsym(t_pd *x);
EXTERN void pd_popsym( t_pd *x);
EXTERN t_symbol *pd_getfilename(void);
EXTERN t_symbol *pd_getdirname(void);
EXTERN void pd_bang(    t_pd *x);
EXTERN void pd_pointer( t_pd *x, t_gpointer *gp);
EXTERN void pd_float(   t_pd *x, t_float f);
EXTERN void pd_symbol(  t_pd *x, t_symbol *s);
EXTERN void pd_string(  t_pd *x, const char *s); /* makes a refcounted symbol (copying s) */
EXTERN void pd_list(    t_pd *x, t_symbol *s, int argc, t_atom *argv);

#ifdef PD_PLUSPLUS_FACE
#define pd_class(x) ((x)->_class)
#else
#define pd_class(x) (*(x))
#endif

EXTERN void gobj_subscribe   (t_gobj *self, t_gobj *observer);
EXTERN void gobj_unsubscribe (t_gobj *self, t_gobj *observer);
EXTERN void gobj_changed     (t_gobj *self, const char *k);
EXTERN void gobj_changed2    (t_gobj *self, int argc, t_atom *argv);
EXTERN void gobj_changed3    (t_gobj *self, t_gobj *origin, int argc, t_atom *argv);

/* ----------------- pointers ---------------- */
EXTERN void gpointer_init(t_gpointer *gp);
EXTERN void gpointer_copy(const t_gpointer *gpfrom, t_gpointer *gpto);
EXTERN void gpointer_unset(t_gpointer *gp);
EXTERN int gpointer_check(const t_gpointer *gp, int headok);

/* ----------------- patchable "objects" -------------- */
/* already defined
EXTERN_STRUCT _inlet;
#define t_inlet struct _inlet
EXTERN_STRUCT _outlet;
#define t_outlet struct _outlet
*/
EXTERN t_inlet *inlet_new(t_object *owner, t_pd *dest, t_symbol *s1, t_symbol *s2);
EXTERN t_inlet *pointerinlet_new(t_object *owner, t_gpointer *gp);
EXTERN t_inlet *floatinlet_new(t_object *owner, t_float *fp);
EXTERN t_inlet *symbolinlet_new(t_object *owner, t_symbol **sp);
EXTERN t_inlet *stringinlet_new(t_object *owner, t_symbol **sp); /* for future use */
EXTERN t_inlet *signalinlet_new(t_object *owner, t_float f);
EXTERN void inlet_free(t_inlet *x);

EXTERN t_outlet *outlet_new(t_object *owner, t_symbol *s);
EXTERN void outlet_bang(    t_outlet *x);
EXTERN void outlet_pointer( t_outlet *x, t_gpointer *gp);
EXTERN void outlet_float(   t_outlet *x, t_float f);
EXTERN void outlet_symbol(  t_outlet *x, t_symbol *s);
EXTERN void outlet_string(  t_outlet *x, const char *s); /* makes a refcounted symbol (copying s) */
EXTERN void outlet_atom(    t_outlet *x, t_atom *a);
EXTERN void outlet_list(    t_outlet *x, t_symbol *s, int argc, t_atom *argv);
EXTERN void outlet_anything(t_outlet *x, t_symbol *s, int argc, t_atom *argv);
EXTERN t_symbol *outlet_getsymbol(t_outlet *x);
EXTERN void outlet_free(t_outlet *x);
EXTERN t_object *pd_checkobject(t_pd *x);

/* -------------------- canvases -------------- */

EXTERN void glob_setfilename(void *dummy, t_symbol *name, t_symbol *dir);
EXTERN void canvas_setargs(int  argc,  t_atom * argv );
EXTERN void canvas_getargs(int *argcp, t_atom **argvp);
EXTERN t_symbol *canvas_getcurrentdir(void);
EXTERN t_glist *canvas_getcurrent(void);
/* if result==0 then it will allocate a result and return it */
EXTERN char *canvas_makefilename(t_glist *c, char *file, char *result, int resultsize);
EXTERN t_symbol *canvas_getdir(t_glist *x);
EXTERN void canvas_dataproperties(t_glist *x, t_scalar *sc, t_binbuf *b);
EXTERN int canvas_open( t_canvas *x, const char *name, const char *ext, char * dirresult, char **nameresult, unsigned int size, int bin);
EXTERN int canvas_open2(t_canvas *x, const char *name, const char *ext, char **dirresult, char **nameresult,                    int bin);

/* -------------------- classes -------------- */

#define CLASS_DEFAULT 0         /* flags for new classes below */
#define CLASS_PD 1
#define CLASS_GOBJ 2
#define CLASS_PATCHABLE 3
#define CLASS_NOINLET 8
#define CLASS_NEWATOMS 16
#define CLASS_TYPEMASK 3

#ifdef __cplusplus
typedef int t_atomtypearg;
#else
typedef t_atomtype t_atomtypearg;
#endif

EXTERN t_class *class_find (t_symbol *s);
EXTERN t_class *class_new( t_symbol *name,t_newmethod nu,t_method freem,size_t size,int flags,t_atomtypearg arg1, ...);
EXTERN t_class *class_new2(const char *name,t_newmethod nu,t_method freem,size_t size,int flags,const char *sig);
EXTERN void class_addcreator( t_newmethod nu, t_symbol *sel, t_atomtypearg arg1, ...);
EXTERN void class_addcreator2(const char *name, t_newmethod nu, const char *sig);
EXTERN void class_addmethod(  t_class *c, t_method fn, t_symbol *sel, t_atomtypearg arg1, ...);
EXTERN void class_addmethod2( t_class *c, t_method fn, const char *sel, const char *sig);
#define class_new2(NAME,NU,FREE,SIZE,FLAGS,SIG) class_new2(NAME,(t_newmethod)NU,(t_method)FREE,SIZE,FLAGS,SIG)
#define class_addcreator2(NAME,NU,SIG) class_addcreator2(NAME,(t_newmethod)NU,SIG)
#define class_addmethod2(NAME,METH,SEL,SIG) class_addmethod2(NAME,(t_method)METH,SEL,SIG)

EXTERN void class_addbang(    t_class *c, t_method fn);
EXTERN void class_addpointer( t_class *c, t_method fn);
EXTERN void class_doaddfloat( t_class *c, t_method fn);
EXTERN void class_addsymbol(  t_class *c, t_method fn);
EXTERN void class_addstring(  t_class *c, t_method fn); /* for future use */
EXTERN void class_addlist(    t_class *c, t_method fn);
EXTERN void class_addanything(t_class *c, t_method fn);
EXTERN void class_sethelpsymbol(t_class *c, t_symbol *s);
EXTERN char *class_getname(t_class *c);
EXTERN char *class_gethelpname(t_class *c);
EXTERN void class_setdrawcommand(t_class *c);
EXTERN int class_isdrawcommand(t_class *c);
EXTERN void class_domainsignalin(t_class *c, int onset);
#define CLASS_MAINSIGNALIN(c, type, field) \
    class_domainsignalin(c, (char *)(&((type *)0)->field) - (char *)0)

/* in "class" observer: observable calls this to notify of a change */
typedef void (*t_notice)(     t_gobj *x, t_gobj *origin, int argc, t_atom *argv);
EXTERN void class_setnotice(t_class *c, t_notice notice);

/* in "class" observable: observable sends initial data to observer */
/* this is called by gobj_subscribe to find out all the indirect subscriptions of aggregators. */
/* every class that redefines this is an aggregator; every aggregator should redefine this. */
/* "calling super" is done by calling gobj_onsubscribe with same args */
typedef void (*t_onsubscribe)(t_gobj *x, t_gobj *observer);
EXTERN void class_setonsubscribe(t_class *c, t_onsubscribe onsubscribe);
EXTERN void gobj_onsubscribe(t_gobj *x, t_gobj *observer); /* default handler, that you may inherit from */

/* prototype for functions to save Pd's to a binbuf */
typedef void (*t_savefn)(t_gobj *x, t_binbuf *b);
EXTERN void class_setsavefn(t_class *c, t_savefn f);
EXTERN t_savefn class_getsavefn(t_class *c);

#ifndef PD_CLASS_DEF
#define class_addbang(x, y)     class_addbang(    (x), (t_method)(y))
#define class_addpointer(x, y)  class_addpointer( (x), (t_method)(y))
#define class_addfloat(x, y)    class_doaddfloat( (x), (t_method)(y))
#define class_addsymbol(x, y)   class_addsymbol(  (x), (t_method)(y))
#define class_addstring(x, y)   class_addstring(  (x), (t_method)(y)) /* for future use */
#define class_addlist(x, y)     class_addlist(    (x), (t_method)(y))
#define class_addanything(x, y) class_addanything((x), (t_method)(y))
#endif

EXTERN void class_settip(t_class *x,t_symbol* s);
EXTERN void inlet_settip(t_inlet* i,t_symbol* s);
EXTERN void class_setfieldnames(t_class *x, const char *s); /* where s is split on spaces and tokenized */
EXTERN  int class_getfieldindex(t_class *x, const char *s);
typedef int (*t_loader)(t_canvas *canvas, char *classname);
EXTERN void sys_register_loader(t_loader loader);

/* ------------   printing --------------------------------- */
EXTERN void post(const char *fmt, ...);
EXTERN void startpost(const char *fmt, ...);
EXTERN void poststring(const char *s);
EXTERN void postfloat(float f);
EXTERN void postatom(int argc, t_atom *argv);
EXTERN void endpost(void);
EXTERN void error(const char *fmt, ...);
EXTERN void verror(const char *fmt, va_list args);
EXTERN void verbose(int level, const char *fmt, ...);
EXTERN void bug(const char *fmt, ...);
EXTERN void pd_error(void *object, const char *fmt, ...) /*__attribute__ ((deprecated))*/;
EXTERN void sys_logerror(const char *object, const char *s);
EXTERN void sys_unixerror(const char *object);
EXTERN void sys_ouch(void);

/* ------------  system interface routines ------------------- */
EXTERN int sys_isreadablefile(const char *name);
EXTERN void sys_bashfilename(const char *from, char *to);
EXTERN void sys_unbashfilename(const char *from, char *to);
EXTERN int open_via_path(const char *name, const char *ext, const char *dir,
    char *dirresult, char **nameresult, unsigned int size, int bin);
EXTERN int open_via_path2(const char *dir, const char *name, const char *ext, char **dirresult, char **nameresult, int bin);

EXTERN int sched_geteventno(void);
EXTERN double sys_getrealtime(void);


/* ------------  threading ------------------- */ 
/* T.Grill - see m_sched.c */
EXTERN void sys_lock(void);
EXTERN void sys_unlock(void);
EXTERN int sys_trylock(void);
EXTERN int sys_timedlock(int microsec);
/* tb: to be called at idle time */
EXTERN void sys_callback(t_int (*callback) (t_int* argv), t_int* argv, t_int argc);

/* --------------- signals ----------------------------------- */

typedef float t_sample;
#define MAXLOGSIG 32
#define MAXSIGSIZE (1 << MAXLOGSIG)

/* this doesn't really have to do with C++, just with getting rid of prefixes */
#ifdef PD_PLUSPLUS_FACE
struct t_signal {
    int n;
    t_sample *v;
    float sr;
    int refcount;
    int isborrowed;
    t_signal *borrowedfrom;
    t_signal *nextfree;
    t_signal *nextused;
    int vecsize;
};
#else
typedef struct _signal {
    int s_n;            /* number of points in the array */
    t_sample *s_vec;    /* the array */
    float s_sr;         /* sample rate */
    int s_refcount;     /* number of times used */
    int s_isborrowed;   /* whether we're going to borrow our array */
    struct _signal *s_borrowedfrom;     /* signal to borrow it from */
    struct _signal *s_nextfree;         /* next in freelist */
    struct _signal *s_nextused;         /* next in used list */
    int s_vecsize;      /* allocated size of array in points */
} t_signal;
#endif

typedef t_int *(*t_perfroutine)(t_int *args);

/* tb: exporting basic arithmetic dsp functions { 
 * for (n % 8) != 0 */
EXTERN t_int *zero_perform(t_int *args);
EXTERN t_int *copy_perform(t_int *args);
EXTERN t_int *plus_perform( t_int *args); EXTERN t_int *scalarplus_perform( t_int *args);
EXTERN t_int *minus_perform(t_int *args); EXTERN t_int *scalarminus_perform(t_int *args);
EXTERN t_int *times_perform(t_int *args); EXTERN t_int *scalartimes_perform(t_int *args);
EXTERN t_int *over_perform( t_int *args); EXTERN t_int *scalarover_perform( t_int *args);
EXTERN t_int *max_perform(  t_int *args); EXTERN t_int *scalarmax_perform(  t_int *args);
EXTERN t_int *min_perform(  t_int *args); EXTERN t_int *scalarmin_perform(  t_int *args);
EXTERN t_int *sig_tilde_perform(t_int *args);
EXTERN t_int *clip_perform(t_int *args);

/* for (n % 8) == 0 */
EXTERN t_int *zero_perf8(t_int *args);
EXTERN t_int *copy_perf8(t_int *args);
EXTERN t_int *plus_perf8( t_int *args); EXTERN t_int *scalarplus_perf8( t_int *args);
EXTERN t_int *minus_perf8(t_int *args); EXTERN t_int *scalarminus_perf8(t_int *args);
EXTERN t_int *times_perf8(t_int *args); EXTERN t_int *scalartimes_perf8(t_int *args);
EXTERN t_int *over_perf8( t_int *args); EXTERN t_int *scalarover_perf8( t_int *args);
EXTERN t_int *max_perf8(  t_int *args); EXTERN t_int *scalarmax_perf8(  t_int *args);
EXTERN t_int *min_perf8(  t_int *args); EXTERN t_int *scalarmin_perf8(  t_int *args);
EXTERN t_int *sqr_perf8(t_int *args);
EXTERN t_int *sig_tilde_perf8(t_int *args);

/* for (n % 8) == 0 && aligned signal vectors
 * check with simd_checkX functions !!! */
EXTERN t_int *zero_perf_simd(t_int *args);
EXTERN t_int *copy_perf_simd(t_int *args);
EXTERN t_int *plus_perf_simd( t_int *args); EXTERN t_int *scalarplus_perf_simd( t_int *args);
EXTERN t_int *minus_perf_simd(t_int *args); EXTERN t_int *scalarminus_perf_simd(t_int *args);
EXTERN t_int *times_perf_simd(t_int *args); EXTERN t_int *scalartimes_perf_simd(t_int *args);
EXTERN t_int *over_perf_simd( t_int *args); EXTERN t_int *scalarover_perf_simd( t_int *args);
EXTERN t_int *max_perf_simd(  t_int *args); EXTERN t_int *scalarmax_perf_simd(  t_int *args);
EXTERN t_int *min_perf_simd(  t_int *args); EXTERN t_int *scalarmin_perf_simd(  t_int *args);
EXTERN t_int *sqr_perf_simd(t_int *args);
EXTERN t_int *sig_tilde_perf_simd(t_int *args);
EXTERN t_int *clip_perf_simd(t_int *args);
/* } tb */

EXTERN void dsp_add_plus(t_sample *in1, t_sample *in2, t_sample *out, int n);
EXTERN void dsp_add_copy(t_sample *in, t_sample *out, int n);
EXTERN void dsp_add_scalarcopy(t_sample *in, t_sample *out, int n);
EXTERN void dsp_add_zero(t_sample *out, int n);

EXTERN int sys_getblksize(void);
EXTERN float sys_getsr(void);
EXTERN int sys_get_inchannels(void);
EXTERN int sys_get_outchannels(void);

EXTERN void dsp_add(t_perfroutine f, int n, ...);
EXTERN void dsp_addv(t_perfroutine f, int n, t_int *vec);
EXTERN void pd_fft(float *buf, int npoints, int inverse);
EXTERN int ilog2(int n);

EXTERN void mayer_fht(float *fz, int n);
EXTERN void mayer_fft(int n, float *real, float *imag);
EXTERN void mayer_ifft(int n, float *real, float *imag);
EXTERN void mayer_realfft(int n, float *real);
EXTERN void mayer_realifft(int n, float *real);

EXTERN float *cos_table;
#define LOGCOSTABSIZE 9
#define COSTABSIZE (1<<LOGCOSTABSIZE)

EXTERN int canvas_suspend_dsp(void);
EXTERN void canvas_resume_dsp(int oldstate);
EXTERN void canvas_update_dsp(void);
EXTERN int canvas_dspstate;

/* IOhannes { (up/downsampling) */
/* use zero-padding to generate samples inbetween */
#define RESAMPLE_ZERO 0
/* use sample-and-hold to generate samples inbetween */
#define RESAMPLE_HOLD 1 
/* use linear interpolation to generate samples inbetween */
#define RESAMPLE_LINEAR 2
/* not a real up/downsampling:
 * upsampling: copy the original vector to the first part of the upsampled vector; the rest is zero
 * downsampling: take the first part of the original vector as the downsampled vector
 * WHAT FOR: we often want to process only the first half of an FFT-signal (the rest is redundant)
 */
#define RESAMPLE_BLOCK 3

#define RESAMPLE_DEFAULT RESAMPLE_ZERO

typedef struct _resample {
  int method;       /* up/downsampling method ID */
  t_int downsample; /* downsampling factor */
  t_int upsample;   /* upsampling factor */
#ifdef PD_PLUSPLUS_FACE
  t_float *v;   /* here we hold the resampled data */
  int n;
#else
  t_float *vec;
  int      s_n;
#endif
  t_float *coeffs;  /* coefficients for filtering... */
  int      coefsize;
  t_float *buffer;  /* buffer for filtering */
  int      bufsize;
} t_resample;

EXTERN void resample_init(t_resample *x);
EXTERN void resample_free(t_resample *x);
EXTERN void resample_dsp(t_resample *x, t_sample *in, int insize, t_sample *out, int outsize, int method);
EXTERN void resamplefrom_dsp(t_resample *x, t_sample *in, int insize, int outsize, int method);
EXTERN void resampleto_dsp(t_resample *x, t_sample *out, int insize, int outsize, int method);
/* } IOhannes */

/* tb: exporting basic simd coded dsp functions { */

/* vectorized, not simd functions*/
EXTERN void zerovec_8(t_float *dst,int n);
EXTERN void setvec_8(t_float *dst,t_float v,int n);
EXTERN void copyvec_8(t_float *dst,const t_float *src,int n);
EXTERN void addvec_8(t_float *dst,const t_float *src,int n);
EXTERN void testcopyvec_8(t_float *dst,const t_float *src,int n);
EXTERN void testaddvec_8(t_float *dst,const t_float *src,int n);
/* EXTERN float sumvec_8(t_float* in, t_int n); */

/* vectorized, simd functions *
 * dst and src are assumed to be aligned */
EXTERN void zerovec_simd(t_float *dst,int n);
EXTERN void setvec_simd(t_float *dst,t_float v,int n);
EXTERN void copyvec_simd(t_float *dst,const t_float *src,int n);
EXTERN void addvec_simd(t_float *dst,const t_float *src,int n);
EXTERN void testcopyvec_simd(t_float *dst,const t_float *src,int n);
EXTERN void testaddvec_simd(t_float *dst,const t_float *src,int n);
/* EXTERN float sumvec_simd(t_float* in, t_int n); */
EXTERN void copyvec_simd_unalignedsrc(t_float *dst,const t_float *src,int n);

/* not vectorized, not simd functions */
EXTERN void copyvec(t_float *dst,const t_float *src,int n);
EXTERN void addvec(t_float *dst,const t_float *src,int n);
EXTERN void zerovec(t_float *dst, int n);

EXTERN int simd_runtime_check(void);
EXTERN int simd_check1(t_int n, t_float* ptr1);
EXTERN int simd_check2(t_int n, t_float* ptr1, t_float* ptr2);
EXTERN int simd_check3(t_int n, t_float* ptr1, t_float* ptr2, t_float* ptr3);

/* } tb */


/* ----------------------- utility functions for signals -------------- */
EXTERN float mtof(float);
EXTERN float ftom(float);
EXTERN float rmstodb(float);
EXTERN float powtodb(float);
EXTERN float dbtorms(float);
EXTERN float dbtopow(float);

EXTERN float q8_sqrt(float);
EXTERN float q8_rsqrt(float);
#ifndef N32     
EXTERN float qsqrt(float);  /* old names kept for extern compatibility */
EXTERN float qrsqrt(float);
#endif
/* --------------------- data --------------------------------- */

/* graphical arrays */
typedef struct _garray t_garray;

EXTERN t_class *garray_class;
EXTERN int garray_getfloatarray(t_garray *x, int *size, t_float **vec);
EXTERN float garray_get(t_garray *x, t_symbol *s, t_int indx);
EXTERN void garray_redraw(t_garray *x);
EXTERN int garray_npoints(t_garray *x);
EXTERN char *garray_vec(t_garray *x);
EXTERN void garray_resize(t_garray *x, t_floatarg f);
EXTERN void garray_usedindsp(t_garray *x);
EXTERN void garray_setsaveit(t_garray *x, int saveit);
EXTERN double garray_updatetime(t_garray *x);

EXTERN t_class *scalar_class;

EXTERN t_float *value_get(t_symbol *s);
EXTERN void value_release(t_symbol *s);
EXTERN int value_getfloat(t_symbol *s, t_float *f);
EXTERN int value_setfloat(t_symbol *s, t_float f);

/* ------- GUI interface - functions to send strings to TK --------- */
EXTERN void sys_vgui(const char *fmt, ...);
EXTERN void sys_gui(const char *s);

extern t_class *glob_pdobject;  /* object to send "pd" messages */

/*-------------  Max 0.26 compatibility --------------------*/

#define t_getbytes getbytes
#define t_freebytes freebytes
#define t_resizebytes resizebytes
#define typedmess pd_typedmess
#define vmess pd_vmess

/* A definition to help gui objects straddle 0.34-0.35 changes.  If this is
defined, there is a "te_xpix" field in objects, not a "te_xpos" as before: */

#define PD_USE_TE_XPIX

#ifdef __i386__
/* a test for NANs and denormals.  Should only be necessary on i386. */
#define PD_BADFLOAT(f) ((((*(unsigned int*)&(f))&0x7f800000)==0) || \
    (((*(unsigned int*)&(f))&0x7f800000)==0x7f800000))
/* more stringent test: anything not between 1e-19 and 1e19 in absolute val */
#define PD_BIGORSMALL(f) ((((*(unsigned int*)&(f))&0x60000000)==0) || \
    (((*(unsigned int*)&(f))&0x60000000)==0x60000000))
#else
#define PD_BADFLOAT(f) 0
#define PD_BIGORSMALL(f) 0
#endif

/* tb: wrapper for PD_BIGORSMALL macro */
EXTERN void testcopyvec(t_float *dst,const t_float *src,int n);
EXTERN void testaddvec(t_float *dst,const t_float *src,int n);

/* tb's fifos */
typedef struct _fifo t_fifo;
/* function prototypes */
EXTERN t_fifo * fifo_init(void);
EXTERN void fifo_destroy(t_fifo*);
/* fifo_put() and fifo_get are the only threadsafe functions!!! */
EXTERN void fifo_put(t_fifo*, void*);
EXTERN void* fifo_get(t_fifo*);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif

#define __m_pd_h_
#endif /* __m_pd_h_ */

/* removed functions:
   sys_fontwidth, sys_fontheight, t_widgetbehavior, class_setproperties,
   class_setwidget, class_setparentwidget, class_parentwidget, pd_getparentwidget,
   sys_pretendguibytes, sys_queuegui, sys_unqueuegui, getzbytes,
   gfxstub_new, gfxstub_deleteforkey
   removed fields: te_width, te_type.
 */
/* externs that use g_next directly can't work with desiredata: gui/state, clone, dyn, dynext, toxy, cyclone */
