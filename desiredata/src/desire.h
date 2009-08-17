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
#define DESIRE 1
#endif
#ifndef __DESIRE_H
#define __DESIRE_H

#include "m_pd.h"
#include <stdio.h>

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif

/* ----------------------- START OF FORMER S_STUFF ----------------------------- */

/* from s_path.c */

typedef struct _namelist {   /* element in a linked list of stored strings */
    struct _namelist *nl_next;  /* next in list */
    char *nl_string;            /* the string */
} t_namelist;

t_namelist *namelist_append(t_namelist *listwas, const char *s, int allowdup);
t_namelist *namelist_append_files(t_namelist *listwas, const char *s);
void namelist_free(t_namelist *listwas);
char *namelist_get(t_namelist *namelist, int n);
void sys_setextrapath(const char *p);
extern int sys_usestdpath;
extern t_namelist *sys_externlist;
extern t_namelist *sys_searchpath;
extern t_namelist *sys_helppath;

// IOhannes : added namespace support for libraries
// the define QUALIFIED_NAME both turns on namespaces and sets the library-object delimiter
#define QUALIFIED_NAME "/"
#ifdef QUALIFIED_NAME
void pd_set_library_name(char *libname);
#endif

int sys_open_absolute(                const char *name, const char* ext, char **dirresult, char **nameresult, int bin, int *fdp);
int sys_trytoopenone(const char *dir, const char *name, const char* ext, char **dirresult, char **nameresult, int bin);

/* s_main.c */
extern int sys_debuglevel;
extern int sys_verbose;
extern int sys_noloadbang;
extern int sys_nogui;
extern char *sys_guicmd;
extern int sys_tooltips;
extern int sys_defeatrt;
extern t_symbol *sys_flags;
extern t_symbol *sys_libdir;    /* library directory for auxilliary files */

/* s_loader.c */
int sys_load_lib(t_canvas *canvas, char *filename);
 
/* s_audio.c */
#define SENDDACS_NO 0           /* return values for sys_send_dacs() */
#define SENDDACS_YES 1
#define SENDDACS_SLEPT 2
#define DEFDACBLKSIZE 64        /* the default dac~blocksize */
extern int sys_dacblocksize;    /* the real dac~blocksize */
extern int sys_schedblocksize;  /* audio block size for scheduler */
extern int sys_hipriority;      /* real-time flag, true if priority boosted */
extern t_sample *sys_soundout;
extern t_sample *sys_soundin;
extern int sys_inchannels;
extern int sys_outchannels;
extern int sys_advance_samples; /* scheduler advance in samples */
extern int sys_blocksize;       /* audio I/O block size in sample frames */
extern float sys_dacsr;
extern int sys_schedadvance;
extern int sys_sleepgrain;
extern int sys_callbackscheduler;       /* tb: scheduler to use (0: traditional, 1: callback) */
void sys_open_audio(
int  naudioindev, int * audioindev, int  nchindev, int * chindev,
int naudiooutdev, int *audiooutdev, int nchoutdev, int *choutdev,
int srate, int dacblocksize, int advance, int schedmode, int enable);
void sys_reopen_audio(void);
void sys_close_audio(void);
int sys_send_dacs(void);
void sys_set_priority(int higher);
void sys_audiobuf(int nbufs);
void sys_getmeters(float *inmax, float *outmax);
void sys_listdevs(void);
void sys_setblocksize(int n);
void sys_update_sleepgrain(void); //tb

/* s_midi.c */
#define MAXMIDIINDEV 16         /* max. number of input ports */
#define MAXMIDIOUTDEV 16        /* max. number of output ports */
extern int sys_nmidiin;
extern int sys_nmidiout;
extern int sys_midiindevlist[];
extern int sys_midioutdevlist[];
void sys_open_midi(int nmidiin, int *midiinvec, int nmidiout, int *midioutvec, int enable);
void sys_get_midi_params(int *pnmidiindev, int *pmidiindev, int *pnmidioutdev, int *pmidioutdev);
void sys_get_midi_apis(char *buf);
void sys_reopen_midi( void);
void sys_close_midi( void);
EXTERN void sys_putmidimess(int portno, int a, int b, int c);
EXTERN void sys_putmidibyte(int portno, int a);
EXTERN void sys_poll_midi(void);
EXTERN void sys_setmiditimediff(double inbuftime, double outbuftime);
EXTERN void sys_midibytein(int portno, int byte);
/* implemented in the system dependent MIDI code (s_midi_pm.c, etc.) */
void midi_getdevs(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int maxndev, int devdescsize);
void sys_do_open_midi(int nmidiindev, int *midiindev, int nmidioutdev, int *midioutdev);

#ifdef USEAPI_ALSA
EXTERN void sys_alsa_putmidimess(int portno, int a, int b, int c);
EXTERN void sys_alsa_putmidibyte(int portno, int a);
EXTERN void sys_alsa_poll_midi(void);
EXTERN void sys_alsa_setmiditimediff(double inbuftime, double outbuftime);
EXTERN void sys_alsa_midibytein(int portno, int byte);
EXTERN void sys_alsa_close_midi( void);
/* implemented in the system dependent MIDI code (s_midi_pm.c, etc. ) */
void midi_alsa_getdevs(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int maxndev, int devdescsize);
void sys_alsa_do_open_midi(int nmidiindev, int *midiindev, int nmidioutdev, int *midioutdev);
#endif

/* m_sched.c */
EXTERN void sys_log_error(int type);
#define ERR_NOTHING 0
#define ERR_ADCSLEPT 1
#define ERR_DACSLEPT 2
#define ERR_RESYNC 3
#define ERR_DATALATE 4
#define ERR_XRUN 5
#define ERR_SYSLOCK 6
void sched_set_using_dacs(int flag);
void sys_setscheduler(int scheduler); //tb
int sys_getscheduler(void); //tb

/* s_inter.c */
EXTERN void sys_microsleep(int microsec);
EXTERN void sys_bail(int exitcode);
EXTERN int sys_pollgui(void);
typedef void (*t_socketnotifier)(void *x);
typedef void (*t_socketreceivefn)(void *x, t_binbuf *b);

typedef struct _socketreceiver {
    char *inbuf;
    int inhead;
    int intail;
    void *owner;
    int udp;
    t_socketnotifier notifier;
    t_socketreceivefn socketreceivefn;
/* for sending only: */
    int fd;
    struct _socketreceiver *next;
    char *obuf;
    int ohead, otail, osize;
    int waitingforping;
    int bytessincelastping;
} t_socketreceiver;

EXTERN char pd_version[];
EXTERN t_text *sys_netreceive;
EXTERN t_socketreceiver *sys_socketreceiver;
//EXTERN t_socketreceiver *netreceive_newest_receiver(t_text *x);

EXTERN t_namelist *sys_externlist;
EXTERN t_namelist *sys_openlist;
EXTERN t_namelist *sys_messagelist;

EXTERN t_socketreceiver *socketreceiver_new(t_pd *owner, int fd,
    t_socketnotifier notifier, t_socketreceivefn socketreceivefn, int udp);
EXTERN void socketreceiver_read(t_socketreceiver *x, int fd);
EXTERN void sys_sockerror(const char *s);
EXTERN void sys_closesocket(int fd);

typedef void (*t_fdpollfn)(void *ptr, int fd);
EXTERN void sys_addpollfn(int fd, t_fdpollfn fn, void *ptr);
EXTERN void sys_rmpollfn(int fd);
#ifdef UNIX
void sys_setalarm(int microsec);
void sys_setvirtualalarm(void);
#endif

#define API_NONE 0
#define API_ALSA 1
#define API_OSS 2
#define API_MMIO 3
#define API_PORTAUDIO 4
#define API_JACK 5
#define API_ASIO 7

#if defined(USEAPI_OSS)
#define API_DEFAULT API_OSS
#define API_DEFSTRING "oss"
#elif defined(USEAPI_ALSA)
#define API_DEFAULT API_ALSA
#define API_DEFSTRING "alsa"
#elif defined(USEAPI_JACK)
#define API_DEFAULT API_JACK
#define API_DEFSTRING "jack"
#elif defined(USEAPI_MMIO)
#define API_DEFAULT API_MMIO
#define API_DEFSTRING "mmio"
#elif defined(USEAPI_PORTAUDIO)
#define API_DEFAULT API_PORTAUDIO
#define API_DEFSTRING "portaudio"
#else
#define API_DEFAULT 0
#define API_DEFSTRING "none"
#endif

#define DEFAULTAUDIODEV 0
#define MAXAUDIOINDEV 4
#define MAXAUDIOOUTDEV 4
#define DEFMIDIDEV 0
#define DEFAULTSRATE 44100
#ifdef MSW
#define DEFAULTADVANCE 70
#else
#define DEFAULTADVANCE 50
#endif

struct t_audiodevs {
  int ndev;
  int dev[MAXAUDIOINDEV];
  int chdev[MAXAUDIOINDEV];
#ifdef __cplusplus
  t_audiodevs() : ndev(-1) {}
#endif
};

/* new audio api interface */
typedef struct t_audioapi {
    int (*open_audio)(
       int naudioindev,  int *audioindev,  int nchindev,  int *chindev,
       int naudiooutdev, int *audiooutdev, int nchoutdev, int *choutdev,
       int rate, int schedmode);
    void (*close_audio)(void);
    int (*send_dacs)(void);
    void (*getdevs)(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int *canmulti, int maxndev, int devdescsize);
} t_audioapi;

int pa_open_audio(int inchans, int outchans, int rate, int advance, int indeviceno, int outdeviceno, int schedmode);

/* tb { */
void pa_test_setting (int ac, t_atom *av);
void pa_getcurrent_devices(void);
void pa_getaudiooutdevinfo(t_float f);
void pa_getaudioindevinfo(t_float f);
/* } tb */

int  jack_open_audio(int wantinchans, int wantoutchans, int rate, int scheduler);
void jack_listdevs(void);
void sys_listmididevs(void);
void sys_set_midi_api(int whichapi);
void sys_set_audio_api(int whichapi);
void sys_get_audio_apis(char *buf);
extern int sys_audioapi;
void sys_set_audio_state(int onoff);

/* API dependent audio flags and settings */
void oss_set32bit(void);
void linux_alsa_devname(char *devname);

void  sys_get_audio_params(t_audiodevs *in, t_audiodevs *out, int *prate, int *dacblocksize, int *padvance, int *pscheduler);
void sys_save_audio_params(t_audiodevs *in, t_audiodevs *out, int   rate, int  dacblocksize, int   advance, int   scheduler);

/* s_file.c */
typedef void (*t_printhook)(const char *s);
extern t_printhook sys_printhook;  /* set this to override printing */
extern FILE *sys_printtofh;
#ifdef MSW
#define vsnprintf  _vsnprintf /* jsarlo -- alias this name for msw */
#endif

/* jsarlo { */
EXTERN double sys_time;
EXTERN double sys_time_per_dsp_tick;
EXTERN int sys_externalschedlib;

EXTERN t_sample* get_sys_soundout(void);
EXTERN t_sample* get_sys_soundin(void);
EXTERN int* get_sys_main_advance(void);
EXTERN double* get_sys_time_per_dsp_tick(void);
EXTERN int* get_sys_schedblocksize(void);
EXTERN double* get_sys_time(void);
EXTERN float* get_sys_dacsr(void);
EXTERN int* get_sys_sleepgrain(void);
EXTERN int* get_sys_schedadvance(void);

EXTERN void sys_clearhist(void);
EXTERN void sys_initmidiqueue(void);
EXTERN int sys_addhist(int phase);
EXTERN void sys_setmiditimediff(double inbuftime, double outbuftime);
EXTERN void sched_tick(double next_sys_time);
EXTERN void sys_pollmidiqueue(void);
EXTERN int sys_pollgui(void);
EXTERN void sys_setchsr(int chin, int chout, int sr, int dacblocksize);

EXTERN void inmidi_noteon(        int portno, int channel, int pitch,     int velo);
EXTERN void inmidi_controlchange( int portno, int channel, int ctlnumber, int value);
EXTERN void inmidi_programchange( int portno, int channel,                int value);
EXTERN void inmidi_pitchbend(     int portno, int channel,                int value);
EXTERN void inmidi_aftertouch(    int portno, int channel,                int value);
EXTERN void inmidi_polyaftertouch(int portno, int channel, int pitch,     int value);
/* } jsarlo */

/* functions in x_midi.c */
void inmidi_realtimein(    int portno, int cmd);
void inmidi_byte(          int portno, int byte);
void inmidi_sysex(         int portno, int byte);

/* ----------------------- END OF FORMER S_STUFF ----------------------------- */


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

struct _namelist;

/* LATER consider adding font size to this struct (see canvas_getfont()) */
struct _canvasenvironment {
    t_symbol *dir;   /* directory patch lives in */
    int argc;        /* number of "$" arguments */
    t_atom *argv;    /* array of "$" arguments */
    long dollarzero; /* value of "$0" */
    struct _namelist *path;/* search path (0.40) */
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
// should I remove min,max now? g++ complains about conflict between min and std::min...
// they have the same def. btw i don't know how to refer to my own.
namespace desire {
template <class T> static T min(T a, T b) {return a<b?a:b;}
template <class T> static T max(T a, T b) {return a>b?a:b;}
template <class T> T clip(T a, T b, T c) {return min(max(a,b),c);}
void  oprintf(std::ostream &buf, const char *s, ...);
void voprintf(std::ostream &buf, const char *s, va_list args);
};
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

#define PERFORM1ARGS(A,a)                                                                       A a = (A)w[1];
#define PERFORM2ARGS(A,a,B,b)                         PERFORM1ARGS(A,a)                         B b = (B)w[2];
#define PERFORM3ARGS(A,a,B,b,C,c)                     PERFORM2ARGS(A,a,B,b)                     C c = (C)w[3];
#define PERFORM4ARGS(A,a,B,b,C,c,D,d)                 PERFORM3ARGS(A,a,B,b,C,c)                 D d = (D)w[4];
#define PERFORM5ARGS(A,a,B,b,C,c,D,d,E,e)             PERFORM4ARGS(A,a,B,b,C,c,D,d)             E e = (E)w[5];
#define PERFORM6ARGS(A,a,B,b,C,c,D,d,E,e,F,f)         PERFORM5ARGS(A,a,B,b,C,c,D,d,E,e)         F f = (F)w[6];
#define PERFORM7ARGS(A,a,B,b,C,c,D,d,E,e,F,f,G,g)     PERFORM6ARGS(A,a,B,b,C,c,D,d,E,e,F,f)     G g = (G)w[7];
#define PERFORM8ARGS(A,a,B,b,C,c,D,d,E,e,F,f,G,g,H,h) PERFORM7ARGS(A,a,B,b,C,c,D,d,E,e,F,f,G,g) H h = (H)w[8];

#endif /* __DESIRE_H */
