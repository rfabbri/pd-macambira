/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* a header for canvas objects that hold lists of t_gobjs, such as
*  canvases themselves or graphs
*/

/* --------------------- geometry ---------------------------- */
#define IOWIDTH 7
#define IOMIDDLE 3
#define IOTOPMARGIN 1
#define IOBOTTOMMARGIN 1

/* ----------------------- data ------------------------------- */

typedef struct _updateheader
{
    struct _updateheader *upd_next;
    unsigned int upd_array:1;	    /* true if array, false if glist */
    unsigned int upd_queued:1;	    /* true if we're queued */
} t_updateheader;

typedef void (*t_glistmotionfn)(void *z, t_floatarg dx, t_floatarg dy);
typedef void (*t_glistkeyfn)(void *z, t_floatarg key);

EXTERN_STRUCT _rtext;
#define t_rtext struct _rtext

EXTERN_STRUCT _gtemplate;
#define t_gtemplate struct _gtemplate

EXTERN_STRUCT _guiconnect;
#define t_guiconnect struct _guiconnect

EXTERN_STRUCT _subcanvas;
#define t_subcanvas struct _subcanvas

EXTERN_STRUCT _tscalar;
#define t_tscalar struct _tscalar

typedef struct _selection
{
    t_gobj *sel_what;
    struct _selection *sel_next;
} t_selection;

    /* this structure is instantiated whenever a glist becomes visible. */
typedef struct _editor
{
    t_updateheader e_upd;   	    /* update header structure */
    t_selection *e_updlist; 	    /* list of objects to update */
    t_rtext *e_rtext;	    	    /* text responder linked list */
    t_selection *e_selection;  	    /* head of the selection list */
    t_rtext *e_textedfor;   	    /* the rtext if any that we are editing */
    t_gobj *e_grab;	    	    /* object being "dragged" */
    t_glistmotionfn e_motionfn;     /* ... motion callback */
    t_glistkeyfn e_keyfn;	    /* ... keypress callback */
    t_binbuf *e_connectbuf;	    /* connections to deleted objects */
    t_binbuf *e_deleted;    	    /* last stuff we deleted */
    t_guiconnect *e_guiconnect;     /* GUI connection for filtering messages */
    struct _glist *e_glist;	    /* glist which owns this */
    int e_xwas;   	    	    /* xpos on last mousedown or motion event */
    int e_ywas;   	    	    /* ypos, similarly */
    unsigned int e_onmotion: 3;     /* action to take on motion */
    unsigned int e_lastmoved: 1;    /* one if mouse has moved since click */
    unsigned int e_textdirty: 1;    /* one if e_textedfor has changed */
} t_editor;

#define MA_NONE    0 	/* e_onmotion: do nothing on mouse motion */
#define MA_MOVE    1	/* drag the selection around */
#define MA_CONNECT 2	/* make a connection */
#define MA_REGION  3	/* selection region */
#define MA_PASSOUT 4	/* send on to e_grab */
#define MA_DRAGTEXT 5	/* drag in text editor to alter selection */

/* editor structure for "garrays".  We don't bother to delete and regenerate
this structure when the "garray" becomes invisible or visible, although we
could do so if the structure gets big (like the "editor" above.) */
    
typedef struct _arrayvis
{
    t_updateheader av_upd;   	    /* update header structure */
    t_garray *av_garray;    	    /* owning structure */    
} t_arrayvis;

typedef struct _tick	/* where to put ticks on x or y axes */
{
    float k_point;  	/* one point to draw a big tick at */
    float k_inc;    	/* x or y increment per little tick */
    int k_lperb;    	/* little ticks per big; 0 if no ticks to draw */
} t_tick;

typedef struct _glist
{  
    t_gobj gl_gobj; 	    	/* header in case we're a glist */
    t_gobj *gl_list;	    	/* the actual data */
    struct _gstub *gl_stub; 	/* safe pointer handler */
    int gl_valid;   	    	/* incremented when pointers might be stale */
    struct _glist *gl_owner;	/* parent glist or 0 if none */
    float gl_px1;   	    	/* bounding rectangle in parent's coords */
    float gl_py1;
    float gl_px2;
    float gl_py2;
    float gl_x1;    	    	/* ... and in our own coordinates */
    float gl_y1;
    float gl_x2;
    float gl_y2;
    t_tick gl_xtick; 	    	/* ticks marking X values */    
    t_tick gl_ytick;	    	    /* ... and Y values */
    int gl_nxlabels;	    	/* X coordinate labels */
    t_symbol **gl_xlabel;
    float gl_xlabely;	    	    /* ... and their Y coordinates */
    int gl_nylabels;    	/* Y coordinate labels */
    t_symbol **gl_ylabel;
    float gl_ylabelx;
    t_editor *gl_editor;    	/* editor structure when visible */
    t_symbol *gl_name;	    	/* symbol bound here */
} t_glist;

#define gl_pd gl_gobj.g_pd

/* a data structure to describe a field in a pure datum */

#define DT_FLOAT 0
#define DT_SYMBOL 1
#define DT_LIST 2
#define DT_ARRAY 3

typedef struct _dataslot
{
    int ds_type;
    t_symbol *ds_name;
    t_symbol *ds_arraytemplate;	    /* filled in for arrays only */
} t_dataslot;

typedef struct _template
{
    int t_n;
    t_dataslot *t_vec;
} t_template;

struct _array
{
    int a_n;
    int a_elemsize; 	/* LATERd just look this up from template... */
    t_word *a_vec;
    t_symbol *a_templatesym;	/* template for elements */
    int a_valid;    	/* protection against stale pointers into array */
    t_gpointer a_gp;	/* pointer to scalar or array element we're in */
    t_gstub *a_stub;
};


/* function types used to define graphical behavior for gobjs, a bit like X
widgets.  These aren't Pd methods because Pd's typechecking can't specify the
types of pointer arguments.  Also it's more convenient this way, since
every "patchable" object can just get the "text" behaviors. */

    	/* Call this to get a gobj's bounding rectangle in pixels */
typedef void (*t_getrectfn)(t_gobj *x, struct _glist *glist,
    int *x1, int *y1, int *x2, int *y2);
    	/* and this to displace a gobj: */
typedef void (*t_displacefn)(t_gobj *x, struct _glist *glist, int dx, int dy);
    	/* change color to show selection: */
typedef void (*t_selectfn)(t_gobj *x, struct _glist *glist, int state);
    	/* change appearance to show activation/deactivation: */
typedef void (*t_activatefn)(t_gobj *x, struct _glist *glist, int state);
    	/* warn a gobj it's about to be deleted */
typedef void (*t_deletefn)(t_gobj *x, struct _glist *glist);
    	/*  making visible or invisible */
typedef void (*t_visfn)(t_gobj *x, struct _glist *glist, int flag);
    	/* field a mouse click (when not in "edit" mode) */
typedef int (*t_clickfn)(t_gobj *x, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit);
    	/*  save to a binbuf */
typedef void (*t_savefn)(t_gobj *x, t_binbuf *b);
    	/*  open properties dialog */
typedef void (*t_propertiesfn)(t_gobj *x, struct _glist *glist);
    	/* ... and later, resizing; getting/setting color... */

struct _widgetbehavior
{
    t_getrectfn w_getrectfn;
    t_displacefn w_displacefn;
    t_selectfn w_selectfn;
    t_activatefn w_activatefn;
    t_deletefn w_deletefn;
    t_visfn w_visfn;
    t_clickfn w_clickfn;
    t_savefn w_savefn;
    t_propertiesfn w_propertiesfn;
};

/* -------- behaviors for scalars defined by objects in template --------- */
/* these are set by "drawing commands" in g_template.c which add appearance to
scalars, which live in some other window.  If the scalar is just included
in a canvas the "parent" is a misnomer.  There is also a text scalar object
which really does draw the scalar on the parent window; see g_scalar.c. */

/* note how the click function wants the whole scalar, not the "data", so
doesn't work on array elements... is this a problem??? */

    	/* bounding rectangle: */
typedef void (*t_parentgetrectfn)(t_gobj *x, struct _glist *glist,
    t_word *data, t_canvas *template, float basex, float basey,
    int *x1, int *y1, int *x2, int *y2);
    	/* displace it */
typedef void (*t_parentdisplacefn)(t_gobj *x, struct _glist *glist, 
    t_word *data, t_canvas *template, float basex, float basey,
    int dx, int dy);
    	/* change color to show selection */
typedef void (*t_parentselectfn)(t_gobj *x, struct _glist *glist,
    t_word *data, t_canvas *template, float basex, float basey,
    int state);
    	/* change appearance to show activation/deactivation: */
typedef void (*t_parentactivatefn)(t_gobj *x, struct _glist *glist,
    t_word *data, t_canvas *template, float basex, float basey,
    int state);
    	/*  making visible or invisible */
typedef void (*t_parentvisfn)(t_gobj *x, struct _glist *glist,
    t_word *data, t_canvas *template, float basex, float basey,
    int flag);
    	/*  field a mouse click */
typedef int (*t_parentclickfn)(t_gobj *x, struct _glist *glist,
    t_scalar *sc, t_canvas *template, float basex, float basey,
    int xpix, int ypix, int shift, int alt, int dbl, int doit);

struct _parentwidgetbehavior
{
    t_parentgetrectfn w_parentgetrectfn;
    t_parentdisplacefn w_parentdisplacefn;
    t_parentselectfn w_parentselectfn;
    t_parentactivatefn w_parentactivatefn;
    t_parentvisfn w_parentvisfn;
    t_parentclickfn w_parentclickfn;
};

    /* cursor definitions; used as return value for t_parentclickfn */
#define CURSOR_RUNMODE_NOTHING 0
#define CURSOR_RUNMODE_CLICKME 1
#define CURSOR_RUNMODE_THICKEN 2
#define CURSOR_RUNMODE_ADDPOINT 3
#define CURSOR_EDITMODE_NOTHING 4
#define CURSOR_EDITMODE_CONNECT 5
#define CURSOR_EDITMODE_DISCONNECT 6

extern t_canvas *canvas_editing;    /* last canvas to start text edting */ 
extern t_class *vinlet_class, *voutlet_class;
extern t_class *tscalar_class;
extern t_class *subcanvas_class;

/* ------------------- functions on any gobj ----------------------------- */
EXTERN void gobj_getrect(t_gobj *x, t_glist *owner, int *x1, int *y1,
    int *x2, int *y2);
EXTERN void gobj_displace(t_gobj *x, t_glist *owner, int dx, int dy);
EXTERN void gobj_select(t_gobj *x, t_glist *owner, int state);
EXTERN void gobj_activate(t_gobj *x, t_glist *owner, int state);
EXTERN void gobj_delete(t_gobj *x, t_glist *owner);
EXTERN void gobj_vis(t_gobj *x, t_glist *glist, int flag);
EXTERN int gobj_click(t_gobj *x, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit);
EXTERN void gobj_save(t_gobj *x, t_binbuf *b);
EXTERN void gobj_properties(t_gobj *x, struct _glist *glist);

/* -------------------- functions on glists --------------------- */
EXTERN t_glist *glist_new( void);
EXTERN void glist_init(t_glist *x);
EXTERN void glist_add(t_glist *x, t_gobj *g);
EXTERN void glist_clear(t_glist *x);
EXTERN t_canvas *glist_getcanvas(t_glist *x);
EXTERN int glist_isselected(t_glist *x, t_gobj *y);
EXTERN void glist_select(t_glist *x, t_gobj *y);
EXTERN void glist_deselect(t_glist *x, t_gobj *y);
EXTERN void glist_noselect(t_glist *x);
EXTERN void glist_selectall(t_glist *x);
EXTERN void glist_delete(t_glist *x, t_gobj *y);
EXTERN void glist_retext(t_glist *x, t_text *y);
EXTERN void glist_grab(t_glist *x, t_gobj *y, t_glistmotionfn motionfn,
    t_glistkeyfn keyfn, int xpos, int ypos);
EXTERN int  glist_isvisible(t_glist *x);
EXTERN t_glist *glist_findgraph(t_glist *x);
EXTERN int glist_getfont(t_glist *x);
EXTERN void glist_sort(t_glist *canvas);
EXTERN void glist_read(t_glist *x, t_symbol *filename, t_symbol *format);
EXTERN void glist_write(t_glist *x, t_symbol *filename, t_symbol *format);

EXTERN float glist_pixelstox(t_glist *x, float xpix);
EXTERN float glist_pixelstoy(t_glist *x, float ypix);
EXTERN float glist_xtopixels(t_glist *x, float xval);
EXTERN float glist_ytopixels(t_glist *x, float yval);

EXTERN void glist_redrawitem(t_glist *owner, t_gobj *gobj);
EXTERN void glist_getnextxy(t_glist *gl, int *xval, int *yval);
EXTERN void glist_glist(t_glist *g, t_symbol *s, int argc, t_atom *argv);
EXTERN t_glist *glist_addglist(t_glist *g, t_symbol *sym,
    float x1, float y1, float x2, float y2,
    float px1, float py1, float px2, float py2);
EXTERN void glist_arraydialog(t_glist *parent, t_symbol *name,
    t_floatarg size, t_floatarg saveit, t_floatarg newgraph);
EXTERN t_binbuf *glist_writetobinbuf(t_glist *x, int wholething);
EXTERN void glist_cleanup(t_glist *x);
EXTERN void glist_free(t_glist *x);

/* -------------------- functions on texts ------------------------- */
EXTERN void text_setto(t_text *x, t_glist *glist, char *buf, int bufsize);
EXTERN void text_drawborder(t_text *x, t_glist *glist, char *tag,
    int width, int height, int firsttime);
EXTERN void text_eraseborder(t_text *x, t_glist *glist, char *tag);

/* -------------------- functions on rtexts ------------------------- */
#define RTEXT_DOWN 1
#define RTEXT_DRAG 2
#define RTEXT_DBL 3
#define RTEXT_SHIFT 4

EXTERN t_rtext *rtext_new(t_glist *glist, t_text *who, t_rtext *next);
EXTERN t_rtext *rtext_remove(t_rtext *first, t_rtext *x);
EXTERN t_rtext *glist_findrtext(t_glist *gl, t_text *who);
EXTERN int rtext_height(t_rtext *x);
EXTERN void rtext_displace(t_rtext *x, int dx, int dy);
EXTERN void rtext_select(t_rtext *x, int state);
EXTERN void rtext_activate(t_rtext *x, int state);
EXTERN void rtext_free(t_rtext *x);
EXTERN void rtext_key(t_rtext *x, int n, t_symbol *s);
EXTERN void rtext_mouse(t_rtext *x, int xval, int yval, int flag);
EXTERN void rtext_retext(t_rtext *x);
EXTERN int rtext_width(t_rtext *x);
EXTERN int rtext_height(t_rtext *x);
EXTERN char *rtext_gettag(t_rtext *x);
EXTERN void rtext_gettext(t_rtext *x, char **buf, int *bufsize);

/* -------------------- functions on canvases ------------------------ */
EXTERN t_class *graph_class, *canvas_class;

EXTERN t_canvas *canvas_new(t_symbol *sel, int argc, t_atom *argv);
EXTERN t_symbol *canvas_makebindsym(t_symbol *s);
EXTERN void canvas_vistext(t_canvas *x, t_text *y);
EXTERN void canvas_fixlinesfor(t_canvas *x, t_text *text);
EXTERN void canvas_deletelinesfor(t_canvas *x, t_text *text);
EXTERN void canvas_stowconnections(t_canvas *x);
EXTERN void canvas_restoreconnections(t_canvas *x);
EXTERN t_template *canvas_gettemplate(t_canvas *x);
EXTERN t_template *canvas_gettemplatebyname(t_symbol *s);
EXTERN t_template *gtemplate_get(t_gtemplate *x);
EXTERN int template_find_field(t_template *x, t_symbol *name, int *p_onset,
    int *p_type, t_symbol **p_arraytype);
EXTERN t_float canvas_getfloat(t_canvas *x, t_symbol *fieldname, t_word *wp,
    int loud);
EXTERN void canvas_setfloat(t_canvas *x, t_symbol *fieldname, t_word *wp,
    t_float f, int loud);
EXTERN t_symbol *canvas_getsymbol(t_canvas *x, t_symbol *fieldname, t_word *wp,
    int loud);
EXTERN void canvas_setsymbol(t_canvas *x, t_symbol *fieldname, t_word *wp,
    t_symbol *s, int loud);

EXTERN t_inlet *canvas_addinlet(t_canvas *x, t_pd *who, t_symbol *sym);
EXTERN void canvas_rminlet(t_canvas *x, t_inlet *ip);
EXTERN t_outlet *canvas_addoutlet(t_canvas *x, t_pd *who, t_symbol *sym);
EXTERN void canvas_rmoutlet(t_canvas *x, t_outlet *op);
EXTERN void canvas_redrawallfortemplate(t_canvas *template);
EXTERN void canvas_zapallfortemplate(t_canvas *template);
EXTERN void canvas_setusedastemplate(t_canvas *x);
EXTERN t_canvas *canvas_getcurrent(void);
EXTERN void canvas_setcurrent(t_canvas *x);
EXTERN void canvas_unsetcurrent(t_canvas *x);
EXTERN t_canvas *canvas_getrootfor(t_canvas *x);
EXTERN void canvas_dirty(t_canvas *x, t_int n);
EXTERN int canvas_isvisible(t_canvas *x);
EXTERN int canvas_getfont(t_canvas *x);
typedef int (*t_canvasapply)(t_canvas *x, t_int x1, t_int x2, t_int x3);

EXTERN t_int *canvas_recurapply(t_canvas *x, t_canvasapply *fn,
    t_int x1, t_int x2, t_int x3);

EXTERN void canvas_resortinlets(t_canvas *x);
EXTERN void canvas_resortoutlets(t_canvas *x);
EXTERN void canvas_free(t_canvas *x);

/* ---------------------- functions on subcanvases --------------------- */

EXTERN void subcanvas_fattenforscalars(t_subcanvas *x,
    int *x1, int *y1, int *x2, int *y2);
EXTERN void subcanvas_visforscalars(t_subcanvas *x, t_glist *glist, int vis);
EXTERN int subcanvas_click(t_subcanvas *x, int xpix, int ypix, int shift,
    int alt, int dbl, int doit);
EXTERN t_glist *canvas_getglistonsuper(void);

/* --------------------- functions on tscalars --------------------- */

EXTERN void tscalar_getrect(t_tscalar *x, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2);
EXTERN void tscalar_vis(t_tscalar *x, t_glist *owner, int flag);
EXTERN int tscalar_click(t_tscalar *x, int xpix, int ypix, int shift,
    int alt, int dbl, int doit);

/* --------- functions on garrays (graphical arrays) -------------------- */

EXTERN t_template *garray_template(t_garray *x);

/* -------------------- arrays --------------------- */
EXTERN t_garray *graph_array(t_glist *gl, t_symbol *s, t_symbol *template,
    t_floatarg f, t_floatarg saveit);
EXTERN t_array *array_new(t_symbol *templatesym, t_gpointer *parent);
EXTERN void array_resize(t_array *x, t_template *template, int n);
EXTERN void array_free(t_array *x);

/* --------------------- gpointers and stubs ---------------- */
EXTERN t_gstub *gstub_new(t_glist *gl, t_array *a);
EXTERN void gstub_cutoff(t_gstub *gs);
EXTERN void gpointer_setglist(t_gpointer *gp, t_glist *glist, t_scalar *x);

/* --------------------- scalars ------------------------- */
EXTERN void word_init(t_word *wp, t_template *template, t_gpointer *gp);
EXTERN void word_restore(t_word *wp, t_template *template,
    int argc, t_atom *argv);
EXTERN t_scalar *scalar_new(t_glist *owner,
    t_symbol *templatesym);
EXTERN void scalar_getbasexy(t_scalar *x, float *basex, float *basey);

/* --------------------- templates ------------------------- */
EXTERN t_template *template_new(int argc, t_atom *argv);
EXTERN void template_free(t_template *x);
EXTERN int template_match(t_template *x1, t_template *x2);

/* ----------------------- guiconnects, g_guiconnect.c --------- */
EXTERN t_guiconnect *guiconnect_new(t_pd *who, t_symbol *sym);
EXTERN void guiconnect_notarget(t_guiconnect *x, double timedelay);
