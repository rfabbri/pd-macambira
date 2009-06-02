/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  These routines build a copy of the DSP portion of a graph, which is
    then sorted into a linear list of DSP operations which are added to
    the DSP duty cycle called by the scheduler.  Once that's been done,
    we delete the copy.  The DSP objects are represented by "ugenbox"
    structures which are parallel to the DSP objects in the graph and
    have vectors of siginlets and sigoutlets which record their
    interconnections.
*/

/* hacked to run subpatches with different power-of-2 samplerates - mfg.gfd.uil IOhannes */

#define PD_PLUSPLUS_FACE
#include "desire.h"
#include <stdlib.h>
#include <stdarg.h>

/* T.Grill - include SIMD functionality */
#include "m_simd.h"

extern t_class *vinlet_class, *voutlet_class, *canvas_class;
t_sample *obj_findsignalscalar(t_object *x, int m);
static int ugen_loud;
static t_int *dsp_chain;
static int dsp_chainsize;
struct t_vinlet;
struct t_voutlet;

void vinlet_dspprolog(t_vinlet *x,  t_signal **parentsigs, int myvecsize, int calcsize, int phase, int period, int frequency,
    int downsample, int upsample, int reblock, int switched);
void voutlet_dspprolog(t_voutlet *x, t_signal **parentsigs, int myvecsize, int calcsize, int phase, int period, int frequency,
    int downsample, int upsample, int reblock, int switched);
void voutlet_dspepilog(t_voutlet *x, t_signal **parentsigs, int myvecsize, int calcsize, int phase, int period, int frequency,
    int downsample, int upsample, int reblock, int switched);

/* zero out a vector */
t_int *zero_perform(t_int *w) {
    t_float *out = (t_float *)w[1];
    int n = int(w[2]);
    while (n--) *out++ = 0;
    return w+3;
}

t_int *zero_perf8(t_int *w) {
    t_float *out = (t_float *)w[1];
    int n = int(w[2]);
    for (; n; n -= 8, out += 8) {
        out[0] = 0; out[1] = 0;
        out[2] = 0; out[3] = 0;
        out[4] = 0; out[5] = 0;
        out[6] = 0; out[7] = 0;
    }
    return w+3;
}

void dsp_add_zero(t_sample *out, int n) {
    if (n&7)                    dsp_add(zero_perform,   2, out, n);
    else if(SIMD_CHECK1(n,out)) dsp_add(zero_perf_simd, 2, out, n);
    else                        dsp_add(zero_perf8,     2, out, n);
}

/* ---------------------------- block~ ----------------------------- */
/* The "block~ object maintains the containing canvas's DSP computation,
calling it at a super- or sub-multiple of the containing canvas's
calling frequency.  The block~'s creation arguments specify block size
and overlap.  Block~ does no "dsp" computation in its own right, but it
adds prolog and epilog code before and after the canvas's unit generators.

A subcanvas need not have a block~ at all; if there's none, its
ugens are simply put on the list without any prolog or epilog code.

Block~ may be invoked as switch~, in which case it also acts to switch the
subcanvas on and off.  The overall order of scheduling for a subcanvas
is thus,

    inlet and outlet prologue code (1)
    block prologue (2)
    the objects in the subcanvas, including inlets and outlets
    block epilogue (2)
    outlet epilogue code (2)

where (1) means, "if reblocked" and  (2) means, "if reblocked or switched".

If we're reblocked, the inlet prolog and outlet epilog code takes care of
overlapping and buffering to deal with vector size changes.  If we're switched
but not reblocked, the inlet prolog is not needed, and the output epilog is
ONLY run when the block is switched off; in this case the epilog code simply
copies zeros to all signal outlets.
*/

static int dsp_phase;
static t_class *block_class;

struct t_block : t_object {
    int vecsize;      /* size of audio signals in this block */
    int calcsize;     /* number of samples actually to compute */
    int overlap;
    int phase;        /* from 0 to period-1; when zero we run the block */
    int period;       /* submultiple of containing canvas */
    int frequency;    /* supermultiple of comtaining canvas */
    int count;        /* number of times parent block has called us */
    int chainonset;   /* beginning of code in DSP chain */
    int blocklength;  /* length of dspchain for this block */
    int epiloglength; /* length of epilog */
    char switched;    /* true if we're acting as a a switch */
    char switchon;    /* true if we're switched on */
    char reblock;     /* true if inlets and outlets are reblocking */
    int upsample;     /* IOhannes: upsampling-factor */
    int downsample;   /* IOhannes: downsampling-factor */
    int x_return;     /* stop right after this block (for one-shots) */
};

static void block_set(t_block *x, t_floatarg fvecsize, t_floatarg foverlap, t_floatarg fupsample);

static void *block_new(t_floatarg fcalcsize, t_floatarg foverlap, t_floatarg fupsample) /* IOhannes */ {
    t_block *x = (t_block *)pd_new(block_class);
    x->phase = 0;
    x->period = 1;
    x->frequency = 1;
    x->switched = 0;
    x->switchon = 1;
    block_set(x, fcalcsize, foverlap, fupsample);
    return x;
}

static void block_set(t_block *x, t_floatarg fcalcsize, t_floatarg foverlap, t_floatarg fupsample) {
    int upsample, downsample; /* IOhannes */
    int calcsize = (int)fcalcsize;
    int overlap = (int)foverlap;
    int dspstate = canvas_suspend_dsp();
    if (overlap < 1) overlap = 1;
    if (calcsize < 0) calcsize = 0;    /* this means we'll get it from parent later. */
    /* IOhannes { */
    if (fupsample <= 0) upsample = downsample = 1;
    else if (fupsample >= 1) {
      upsample = (int)fupsample;
      downsample   = 1;
    } else {
      downsample = int(1.0 / fupsample);
      upsample   = 1;
    }
    /* } IOhannes */
    /* vecsize is smallest power of 2 large enough to hold calcsize */
    int vecsize = 0;
    if (calcsize) {
	vecsize = (1 << ilog2(calcsize));
        if (vecsize != calcsize) vecsize *= 2;
    }
    if (vecsize && vecsize != (1 << ilog2(vecsize))) {error("block~: vector size not a power of 2"); vecsize = 64;}
    if (           overlap != (1 << ilog2(overlap))) {error("block~: overlap not a power of 2");     overlap = 1;}
    /* IOhannes { */
    if (downsample != (1 << ilog2(downsample))) {error("block~: downsampling not a power of 2"); downsample = 1;}
    if (  upsample != (1 << ilog2(  upsample))) {error("block~: upsampling not a power of 2");     upsample = 1;}
    /* } IOhannes */
    x->calcsize = calcsize;
    x->vecsize = vecsize;
    x->overlap = overlap;
    /* IOhannes { */
    x->upsample = upsample;
    x->downsample = downsample;
    /* } IOhannes */
    canvas_resume_dsp(dspstate);
}

static void *switch_new(t_floatarg fvecsize, t_floatarg foverlap, t_floatarg fupsample) /* IOhannes */ {
    t_block *x = (t_block *)(block_new(fvecsize, foverlap, fupsample)); /* IOhannes */
    x->switched = 1;
    x->switchon = 0;
    return x;
}

static void block_float(t_block *x, t_floatarg f) {
    if (x->switched) x->switchon = f!=0;
}

static void block_bang(t_block *x) {
    if (x->switched && !x->switchon) {
        x->x_return = 1;
        for (t_int *ip = dsp_chain + x->chainonset; ip; ) ip = t_perfroutine(*ip)(ip);
        x->x_return = 0;
    } else error("bang to block~ or on-state switch~ has no effect");
}

#define PROLOGCALL 2
#define EPILOGCALL 2

static t_int *block_prolog(t_int *w) {
    t_block *x = (t_block *)w[1];
    int phase = x->phase;
    /* if we're switched off, jump past the epilog code */
    if (!x->switchon) return w+x->blocklength;
    if (phase) {
        phase++;
        if (phase == x->period) phase = 0;
        x->phase = phase;
        return w+x->blocklength;  /* skip block; jump past epilog */
    } else {
        x->count = x->frequency;
        x->phase = (x->period > 1 ? 1 : 0);
        return w+PROLOGCALL;        /* beginning of block is next ugen */
    }
}

static t_int *block_epilog(t_int *w) {
    t_block *x = (t_block *)w[1];
    int count = x->count - 1;
    if (x->x_return) return 0;
    if (!x->reblock) return w+x->epiloglength+EPILOGCALL;
    if (count) {
        x->count = count;
        return w - (x->blocklength - (PROLOGCALL + EPILOGCALL));   /* go to ugen after prolog */
    } else return w+EPILOGCALL;
}

static void block_dsp(t_block *x, t_signal **sp) {/* do nothing here */}

void block_tilde_setup() {
    block_class = class_new2("block~", (t_newmethod)block_new, 0, sizeof(t_block), 0,"FFF");
    class_addcreator2("switch~",(t_newmethod)switch_new,"FFF");
    class_addmethod2(block_class, (t_method)block_set,"set","FFF");
    class_addmethod2(block_class, (t_method)block_dsp,"dsp","");
    class_addfloat(block_class, block_float);
    class_addbang(block_class, block_bang);
}

/* ------------------ DSP call list ----------------------- */

static t_int dsp_done(t_int *w) {return 0;}

void dsp_add(t_perfroutine f, int n, ...) {
    va_list ap;
    va_start(ap, n);
    int newsize = dsp_chainsize + n+1;
    dsp_chain = (t_int *)resizebytes(dsp_chain, dsp_chainsize * sizeof (t_int), newsize * sizeof (t_int));
    dsp_chain[dsp_chainsize-1] = (t_int)f;
    for (int i=0; i<n; i++) dsp_chain[dsp_chainsize + i] = va_arg(ap, t_int);
    dsp_chain[newsize-1] = (t_int)dsp_done;
    dsp_chainsize = newsize;
    va_end(ap);
}

/* at Guenter's suggestion, here's a vectorized version */
void dsp_addv(t_perfroutine f, int n, t_int *vec) {
    int newsize = dsp_chainsize + n+1;
    dsp_chain = (t_int *)resizebytes(dsp_chain, dsp_chainsize * sizeof (t_int), newsize * sizeof (t_int));
    dsp_chain[dsp_chainsize-1] = (t_int)f;
    for (int i=0; i<n; i++) dsp_chain[dsp_chainsize + i] = vec[i];
    dsp_chain[newsize-1] = (t_int)dsp_done;
    dsp_chainsize = newsize;
}

void dsp_tick() {
    if (dsp_chain) {
        for (t_int *ip = dsp_chain; ip; ) ip = ((t_perfroutine)*ip)(ip);
        dsp_phase++;
    }
}

/* ---------------- signals ---------------------------- */

int ilog2(int n) {
    int r = -1;
    if (n <= 0) return 0;
    while (n) {
        r++;
        n >>= 1;
    }
    return r;
}

/* list of signals which can be reused, sorted by buffer size */
static t_signal *signal_freelist[MAXLOGSIG+1];
/* list of reusable "borrowed" signals (which don't own sample buffers) */
static t_signal *signal_freeborrowed;
/* list of all signals allocated (not including "borrowed" ones) */
static t_signal *signal_usedlist;

/* call this when DSP is stopped to free all the signals */
void signal_cleanup() {
    t_signal *sig;
    while ((sig = signal_usedlist)) {
        signal_usedlist = sig->nextused;
	    if (!sig->isborrowed) {
#ifndef VECTORALIGNMENT
                free(sig->v);
#else
                freealignedbytes(sig->v, sig->vecsize * sizeof (*sig->v));
#endif
	    }
        free(sig);
    }
    for (int i=0; i<=MAXLOGSIG; i++) signal_freelist[i] = 0;
    signal_freeborrowed = 0;
}

/* mark the signal "reusable." */
extern "C" void signal_makereusable(t_signal *sig) {
    int logn = ilog2(sig->vecsize);
#if 1
    for (t_signal *s5 = signal_freeborrowed;   s5; s5 = s5->nextfree) {if (s5 == sig) {bug("signal_free 3"); return;}}
    for (t_signal *s5 = signal_freelist[logn]; s5; s5 = s5->nextfree) {if (s5 == sig) {bug("signal_free 4"); return;}}
#endif
    if (ugen_loud) post("free %lx: %d", sig, sig->isborrowed);
    if (sig->isborrowed) {
        /* if the signal is borrowed, decrement the borrowed-from signal's reference count, possibly marking it reusable too */
        t_signal *s2 = sig->borrowedfrom;
        if ((s2 == sig) || !s2) bug("signal_free");
        s2->refcount--;
        if (!s2->refcount) signal_makereusable(s2);
        sig->nextfree = signal_freeborrowed;
        signal_freeborrowed = sig;
    } else {
        /* if it's a real signal (not borrowed), put it on the free list so we can reuse it. */
        if (signal_freelist[logn] == sig) bug("signal_free 2");
        sig->nextfree = signal_freelist[logn];
        signal_freelist[logn] = sig;
    }
}

/* reclaim or make an audio signal.  If n is zero, return a "borrowed"
   signal whose buffer and size will be obtained later via signal_setborrowed(). */
t_signal *signal_new(int n, float sr) {
    int vecsize = 0;
    t_signal *ret, **whichlist;
    int logn = ilog2(n);
    if (n) {
        if ((vecsize = (1<<logn)) != n) vecsize *= 2;
        if (logn > MAXLOGSIG) bug("signal buffer too large");
        whichlist = signal_freelist + logn;
    } else whichlist = &signal_freeborrowed;
    /* first try to reclaim one from the free list */
    ret = *whichlist;
    if (ret) *whichlist = ret->nextfree;
    else {
        /* LATER figure out what to do for out-of-space here! */
        ret = (t_signal *)t_getbytes(sizeof *ret);
        if (n) {
#ifndef VECTORALIGNMENT
            ret->v = (t_sample *)getbytes(vecsize * sizeof (*ret->v));
#else
            /* T.Grill - make signal vectors aligned! */
            ret->v = (t_sample *)getalignedbytes(vecsize * sizeof (*ret->v));
#endif
            ret->isborrowed = 0;
        } else {
            ret->v = 0;
            ret->isborrowed = 1;
        }
        ret->nextused = signal_usedlist;
        signal_usedlist = ret;
    }
    ret->n = n;
    ret->vecsize = vecsize;
    ret->sr = sr;
    ret->refcount = 0;
    ret->borrowedfrom = 0;
    if (ugen_loud) post("new %lx: %d", ret, ret->isborrowed);
    return ret;
}

static t_signal *signal_newlike(const t_signal *sig) {return signal_new(sig->n, sig->sr);}

extern "C" void signal_setborrowed(t_signal *sig, t_signal *sig2) {
    if (!sig->isborrowed || sig->borrowedfrom) bug("signal_setborrowed");
    if (sig == sig2) bug("signal_setborrowed 2");
    sig->borrowedfrom = sig2;
    sig->v = sig2->v;
    sig->n = sig2->n;
    sig->vecsize = sig2->vecsize;
}

int signal_compatible(t_signal *s1, t_signal *s2) {return s1->n == s2->n && s1->sr == s2->sr;}

/* ------------------ ugen ("unit generator") sorting ----------------- */

struct t_ugenbox {
    struct t_siginlet  *in;  int nin;
    struct t_sigoutlet *out; int nout;
    int u_phase;
    t_ugenbox *next;
    t_object *obj;
    int done;
};

struct t_siginlet {
    int nconnect;
    int ngot;
    t_signal *signal;
};

struct t_sigoutconnect {
    t_ugenbox *who;
    int inno;
    t_sigoutconnect *next;
};

struct t_sigoutlet {
    int nconnect;
    int nsent;
    t_signal *signal;
    t_sigoutconnect *connections;
};

struct t_dspcontext {
    t_ugenbox *ugenlist;
    t_dspcontext *parentcontext;
    int ninlets;
    int noutlets;
    t_signal **iosigs;
    float srate;
    int vecsize;     /* vector size, power of two */
    int calcsize;    /* number of elements to calculate */
    char toplevel;   /* true if "iosigs" is invalid. */
    char reblock;    /* true if we have to reblock inlets/outlets */
    char switched;   /* true if we're switched */
};

static int ugen_sortno = 0;
static t_dspcontext *ugen_currentcontext;

void ugen_stop() {
    if (dsp_chain) {
        free(dsp_chain);
        dsp_chain = 0;
    }
    signal_cleanup();
}

void ugen_start() {
    ugen_stop();
    ugen_sortno++;
    dsp_chain = (t_int *)getbytes(sizeof(*dsp_chain));
    dsp_chain[0] = (t_int)dsp_done;
    dsp_chainsize = 1;
    if (ugen_currentcontext) bug("ugen_start");
}

int ugen_getsortno() {return ugen_sortno;}

#if 0
void glob_foo(void *dummy, t_symbol *s, int argc, t_atom *argv) {
    int i, count;
    t_signal *sig;
    for (count = 0, sig = signal_usedlist; sig; count++, sig = sig->nextused) {}
    post("used signals %d", count);
    for (i = 0; i < MAXLOGSIG; i++) {
        for (count = 0, sig = signal_freelist[i]; sig; count++, sig = sig->nextfree) {}
        if (count) post("size %d: free %d", (1 << i), count);
    }
    for (count = 0, sig = signal_freeborrowed; sig; count++, sig = sig->nextfree) {}
    post("free borrowed %d", count);
    ugen_loud = argc;
}
#endif

/* start building the graph for a canvas */
extern "C" t_dspcontext *ugen_start_graph(int toplevel, t_signal **sp, int ninlets, int noutlets) {
    t_dspcontext *dc = (t_dspcontext *)getbytes(sizeof(*dc));
    if (ugen_loud) post("ugen_start_graph...");
    dc->ugenlist = 0;
    dc->toplevel = toplevel;
    dc->iosigs = sp;
    dc->ninlets = ninlets;
    dc->noutlets = noutlets;
    dc->parentcontext = ugen_currentcontext;
    ugen_currentcontext = dc;
    return dc;
}

/* first the canvas calls this to create all the boxes... */
extern "C" void ugen_add(t_dspcontext *dc, t_object *obj) {
    t_ugenbox *x = (t_ugenbox *)getbytes(sizeof *x);
    int i;
    t_sigoutlet *uout;
    t_siginlet *uin;
    x->next = dc->ugenlist;
    dc->ugenlist = x;
    x->obj = obj;
    x->nin = obj_nsiginlets(obj);
    x->in = (t_siginlet *)getbytes(x->nin * sizeof (*x->in));
    for (uin = x->in, i = x->nin; i--; uin++) uin->nconnect = 0;
    x->nout = obj_nsigoutlets(obj);
    x->out = (t_sigoutlet *)getbytes(x->nout * sizeof (*x->out));
    for (uout = x->out, i = x->nout; i--; uout++) uout->connections = 0, uout->nconnect = 0;
}

/* and then this to make all the connections. */
extern "C" void ugen_connect(t_dspcontext *dc, t_object *x1, int outno, t_object *x2, int inno) {
    t_ugenbox *u1, *u2;
    int sigoutno = obj_sigoutletindex(x1, outno);
    int siginno = obj_siginletindex(x2, inno);
    if (ugen_loud) post("%s -> %s: %d->%d", class_getname(x1->ob_pd), class_getname(x2->ob_pd), outno, inno);
    for (u1 = dc->ugenlist; u1 && u1->obj != x1; u1 = u1->next) {}
    for (u2 = dc->ugenlist; u2 && u2->obj != x2; u2 = u2->next) {}
    if (!u1 || !u2 || siginno < 0) {
        pd_error(u1->obj, "signal outlet connect to nonsignal inlet (ignored)");
        return;
    }
    if (sigoutno < 0 || sigoutno >= u1->nout || siginno >= u2->nin) {
        bug("ugen_connect %s %s %d %d (%d %d)",
            class_getname(x1->ob_pd), class_getname(x2->ob_pd), sigoutno, siginno, u1->nout, u2->nin);
    }
    t_sigoutlet *uout = u1->out + sigoutno;
    t_siginlet  * uin = u2->in + siginno;
    /* add a new connection to the outlet's list */
    t_sigoutconnect *oc = (t_sigoutconnect *)getbytes(sizeof *oc);
    oc->next = uout->connections;
    uout->connections = oc;
    oc->who = u2;
    oc->inno = siginno;
    /* update inlet and outlet counts  */
    uout->nconnect++;
    uin->nconnect++;
}

/* get the index of a ugenbox or -1 if it's not on the list */
static int ugen_index(t_dspcontext *dc, t_ugenbox *x) {
    int ret=0;
    for (t_ugenbox *u = dc->ugenlist; u; u = u->next, ret++) if (u == x) return ret;
    return -1;
}

/* put a ugenbox on the chain, recursively putting any others on that this one might uncover. */
static void ugen_doit(t_dspcontext *dc, t_ugenbox *u) {
    t_sigoutlet *uout;
    t_siginlet *uin;
    t_sigoutconnect *oc;
    t_class *klass = pd_class(u->obj);
    int i, n;
    /* suppress creating new signals for the outputs of signal-inlets and subpatches, except in the case we're an inlet
       and "blocking" is set.  We don't yet know if a subcanvas will be "blocking" so there we delay new signal creation,
       which will be handled by calling signal_setborrowed in the ugen_done_graph routine below. When we encounter a
       subcanvas or a signal outlet, suppress freeing the input signals as they may be "borrowed" for the parent or subpatch;
       same exception as above, but also if we're "switched" we have to do a copy rather than a borrow. */
    int nonewsigs  = klass==canvas_class || (klass==vinlet_class &&  ! dc->reblock                 );
    int nofreesigs = klass==canvas_class || (klass==voutlet_class && !(dc->reblock || dc->switched));
    t_signal **insig, **outsig, **sig, *s1, *s2, *s3;
    t_ugenbox *u2;
    if (ugen_loud) post("doit %s %d %d", class_getname(klass), nofreesigs, nonewsigs);
    for (i = 0, uin = u->in; i < u->nin; i++, uin++) {
        if (!uin->nconnect) {
            t_sample *scalar;
            s3 = signal_new(dc->vecsize, dc->srate);
            /* post("%s: unconnected signal inlet set to zero", class_getname(u->obj->ob_pd)); */
            if ((scalar = obj_findsignalscalar(u->obj, i))) dsp_add_scalarcopy(scalar, s3->v, s3->n);
            else dsp_add_zero(s3->v, s3->n);
            uin->signal = s3;
            s3->refcount = 1;
        }
    }
    insig = (t_signal **)getbytes((u->nin + u->nout) * sizeof(t_signal *));
    outsig = insig + u->nin;
    for (sig = insig, uin = u->in, i = u->nin; i--; sig++, uin++) {
        int newrefcount;
        *sig = uin->signal;
        newrefcount = --(*sig)->refcount;
            /* if the reference count went to zero, we free the signal now,
            unless it's a subcanvas or outlet; these might keep the
            signal around to send to objects connected to them.  In this
            case we increment the reference count; the corresponding decrement
            is in sig_makereusable(). */
        if (nofreesigs) (*sig)->refcount++;
        else if (!newrefcount) signal_makereusable(*sig);
    }
    for (sig = outsig, uout = u->out, i = u->nout; i--; sig++, uout++) {
        /* similarly, for outlets of subcanvases we delay creating
           them; instead we create "borrowed" ones so that the refcount
           is known.  The subcanvas replaces the fake signal with one showing
           where the output data actually is, to avoid having to copy it.
           For any other object, we just allocate a new output vector;
           since we've already freed the inputs the objects might get called "in place." */
        *sig = uout->signal = nonewsigs ?
            signal_new(0, dc->srate) :
            signal_new(dc->vecsize, dc->srate);
        (*sig)->refcount = uout->nconnect;
    }
    /* now call the DSP scheduling routine for the ugen.  This routine must fill in "borrowed"
       signal outputs in case it's either a subcanvas or a signal inlet. */
    mess1(u->obj, gensym("dsp"), insig);
    /* if any output signals aren't connected to anyone, free them now; otherwise they'll either
       get freed when the reference count goes back to zero, or even later as explained above. */
    for (sig = outsig, uout = u->out, i = u->nout; i--; sig++, uout++) {
        if (!(*sig)->refcount) signal_makereusable(*sig);
    }
    if (ugen_loud) {
        if      (u->nin+u->nout==0) post("put %s %d",           class_getname(u->obj->ob_pd), ugen_index(dc,u));
        else if (u->nin+u->nout==1) post("put %s %d (%lx)",     class_getname(u->obj->ob_pd), ugen_index(dc,u),sig[0]);
        else if (u->nin+u->nout==2) post("put %s %d (%lx %lx)", class_getname(u->obj->ob_pd), ugen_index(dc,u),sig[0],sig[1]);
        else                post("put %s %d (%lx %lx %lx ...)", class_getname(u->obj->ob_pd), ugen_index(dc,u),sig[0],sig[1],sig[2]);
    }
    /* pass it on and trip anyone whose last inlet was filled */
    for (uout = u->out, i = u->nout; i--; uout++) {
        s1 = uout->signal;
        for (oc = uout->connections; oc; oc = oc->next) {
            u2 = oc->who;
            uin = &u2->in[oc->inno];
            /* if there's already someone here, sum the two */
	    s2 = uin->signal;
            if (s2) {
                s1->refcount--;
                s2->refcount--;
                if (!signal_compatible(s1, s2)) {pd_error(u->obj, "%s: incompatible signal inputs", class_getname(u->obj->ob_pd)); return;}
                s3 = signal_newlike(s1);
                dsp_add_plus(s1->v, s2->v, s3->v, s1->n);
                uin->signal = s3;
                s3->refcount = 1;
                if (!s1->refcount) signal_makereusable(s1);
                if (!s2->refcount) signal_makereusable(s2);
            } else uin->signal = s1;
            uin->ngot++;
            if (uin->ngot < uin->nconnect) goto notyet;
            if (u2->nin > 1) for (uin = u2->in, n = u2->nin; n--; uin++) if (uin->ngot < uin->nconnect) goto notyet;
            ugen_doit(dc, u2);
        notyet: ;
        }
    }
    free(insig);
    u->done = 1;
}

/* once the DSP graph is built, we call this routine to sort it. This routine also deletes the graph; later we might
   want to leave the graph around, in case the user is editing the DSP network, to save having to recreate it all the
   time.  But not today.  */
extern "C" void ugen_done_graph(t_dspcontext *dc) {
    t_sigoutlet *uout;
    t_siginlet *uin;
    int i, n;
    t_block *blk;
    int period, frequency, phase, vecsize, calcsize;
    float srate;
    int reblock = 0, switched;
    int downsample = 1, upsample = 1; /* IOhannes */
    /* debugging printout */
    if (ugen_loud) {
        post("ugen_done_graph...");
        for (t_ugenbox *u = dc->ugenlist; u; u = u->next) {
            post("ugen: %s", class_getname(u->obj->ob_pd));
            for (uout = u->out, i = 0; i < u->nout; uout++, i++)
                for (t_sigoutconnect *oc = uout->connections; oc; oc = oc->next) {
                post("... out %d to %s, index %d, inlet %d", i, class_getname(oc->who->obj->ob_pd), ugen_index(dc, oc->who), oc->inno);
            }
        }
    }
    /* search for an object of class "block~" */
    blk = 0;
    for (t_ugenbox *u = dc->ugenlist; u; u = u->next) {
        t_pd *zz = u->obj;
        if (pd_class(zz) == block_class) {
            if (blk) pd_error(blk, "conflicting block~ objects in same page");
            else blk = (t_block *)zz;
        }
    }
    t_dspcontext *parent_context = dc->parentcontext;
    float parent_srate = parent_context ? parent_context->srate   : sys_getsr();
    int parent_vecsize = parent_context ? parent_context->vecsize : sys_getblksize();
    if (blk) {
        int realoverlap;
        vecsize     = blk->vecsize; if (!vecsize) vecsize = parent_vecsize;
        calcsize    = blk->calcsize;if (!calcsize) calcsize = vecsize;
        realoverlap = blk->overlap; if (realoverlap > vecsize) realoverlap = vecsize;
        /* IOhannes { */
        downsample = blk->downsample;
        upsample   = blk->upsample;
        if (downsample > parent_vecsize) downsample=parent_vecsize;
        period = (vecsize * downsample) / (parent_vecsize * realoverlap * upsample);
        frequency = (parent_vecsize * realoverlap * upsample) / (vecsize * downsample);
        /* } IOhannes*/
        phase = blk->phase;
        srate = parent_srate * realoverlap * upsample / downsample;
        /* IOhannes */
        if (period < 1) period = 1;
        if (frequency < 1) frequency = 1;
        blk->frequency = frequency;
        blk->period = period;
        blk->phase = dsp_phase & (period - 1);
        if (!parent_context || realoverlap!=1 || vecsize!=parent_vecsize || downsample!=1 || upsample!=1) /* IOhannes */
            reblock = 1;
        switched = blk->switched;
    } else {
        srate = parent_srate;
        vecsize = parent_vecsize;
        calcsize = parent_context ? parent_context->calcsize : vecsize;
        downsample = upsample = 1;/* IOhannes */
        period = frequency = 1;
        phase = 0;
        if (!parent_context) reblock = 1;
        switched = 0;
    }
    dc->reblock = reblock;
    dc->switched = switched;
    dc->srate = srate;
    dc->vecsize = vecsize;
    dc->calcsize = calcsize;
    /* if we're reblocking or switched, we now have to create output signals to fill in for the "borrowed" ones we
       have now.  This is also possibly true even if we're not blocked/switched, in the case that there was a
       signal loop.  But we don't know this yet. */
    if (dc->iosigs && (switched || reblock)) {
        t_signal **sigp;
        for (i = 0, sigp = dc->iosigs + dc->ninlets; i < dc->noutlets; i++, sigp++) {
            if ((*sigp)->isborrowed && !(*sigp)->borrowedfrom) {
                signal_setborrowed(*sigp, signal_new(parent_vecsize, parent_srate));
                (*sigp)->refcount++;
                if (ugen_loud) post("set %lx->%lx", *sigp, (*sigp)->borrowedfrom);
            }
        }
    }
    if (ugen_loud) post("reblock %d, switched %d", reblock, switched);
    /* schedule prologs for inlets and outlets.  If the "reblock" flag is set, an inlet will put code on the DSP chain
       to copy its input into an internal buffer here, before any unit generators' DSP code gets scheduled.  If we don't
       "reblock", inlets will need to get pointers to their corresponding inlets/outlets on the box we're inside,
       if any.  Outlets will also need pointers, unless we're switched, in which case outlet epilog code will kick in. */
    for (t_ugenbox *u = dc->ugenlist; u; u = u->next) {
        t_pd *zz = u->obj;
        t_signal **outsigs = dc->iosigs;
        if (outsigs) outsigs += dc->ninlets;
        if (pd_class(zz) == vinlet_class)
            vinlet_dspprolog((t_vinlet *)zz, dc->iosigs, vecsize, calcsize, dsp_phase, period, frequency,
                    downsample, upsample, reblock, switched);
        else if (pd_class(zz) == voutlet_class)
            voutlet_dspprolog((t_voutlet *)zz,  outsigs, vecsize, calcsize, dsp_phase, period, frequency,
                    downsample, upsample, reblock, switched);
    }
    int chainblockbegin = dsp_chainsize; /* DSP chain onset before block prolog code */
    if (blk && (reblock || switched)) {  /* add the block DSP prolog */
        dsp_add(block_prolog, 1, blk);
        blk->chainonset = dsp_chainsize - 1;
    }
    /* Initialize for sorting */
    for (t_ugenbox *u = dc->ugenlist; u; u = u->next) {
        u->done = 0;
        for (uout = u->out, i = u->nout; i--; uout++) uout->nsent = 0;
        for (uin = u->in, i = u->nin; i--; uin++) uin->ngot = 0, uin->signal = 0;
    }
    /* Do the sort */
    for (t_ugenbox *u = dc->ugenlist; u; u = u->next) {
        /* check that we have no connected signal inlets */
        if (u->done) continue;
        for (uin = u->in, i = u->nin; i--; uin++)
        if (uin->nconnect) goto next;
        ugen_doit(dc, u);
    next: ;
    }
    /* check for a DSP loop, which is evidenced here by the presence of ugens not yet scheduled. */
    for (t_ugenbox *u = dc->ugenlist; u; u = u->next) if (!u->done) {
        t_signal **sigp;
        pd_error(u->obj, "DSP loop detected (some tilde objects not scheduled)");
        /* this might imply that we have unfilled "borrowed" outputs which we'd better fill in now. */
        for (i = 0, sigp = dc->iosigs + dc->ninlets; i < dc->noutlets; i++, sigp++) {
            if ((*sigp)->isborrowed && !(*sigp)->borrowedfrom) {
                t_signal *s3 = signal_new(parent_vecsize, parent_srate);
                signal_setborrowed(*sigp, s3);
                (*sigp)->refcount++;
                dsp_add_zero(s3->v, s3->n);
                if (ugen_loud) post("oops, belatedly set %lx->%lx", *sigp, (*sigp)->borrowedfrom);
            }
        }
        break;   /* don't need to keep looking. */
    }
    /* add block DSP epilog */
    if (blk && (reblock || switched)) dsp_add(block_epilog, 1, blk);
    int chainblockend = dsp_chainsize; /* and after block epilog code */
    /* add epilogs for outlets. */
    for (t_ugenbox *u = dc->ugenlist; u; u = u->next) {
        t_pd *zz = u->obj;
        if (pd_class(zz) == voutlet_class) {
            t_signal **iosigs = dc->iosigs;
            if (iosigs) iosigs += dc->ninlets;
            voutlet_dspepilog((t_voutlet *)zz, iosigs, vecsize, calcsize, dsp_phase, period, frequency,
                    downsample, upsample, reblock, switched);
        }
    }
    int chainafterall = dsp_chainsize; /* and after signal outlet epilog */
    if (blk) {
        blk->blocklength = chainblockend - chainblockbegin;
        blk->epiloglength = chainafterall - chainblockend;
        blk->reblock = reblock;
    }
    if (ugen_loud) {
        t_int *ip;
        if (!dc->parentcontext) for (i=dsp_chainsize, ip=dsp_chain; i--; ip++) post("chain %lx", *ip);
        post("... ugen_done_graph done.");
    }
    /* now delete everything. */
    while (dc->ugenlist) {
        for (uout = dc->ugenlist->out, n = dc->ugenlist->nout; n--; uout++) {
            t_sigoutconnect *oc = uout->connections, *oc2;
            while (oc) {
                oc2 = oc->next;
                free(oc);
                oc = oc2;
            }
        }
        free(dc->ugenlist->out);
        free(dc->ugenlist->in);
        t_ugenbox *u = dc->ugenlist;
        dc->ugenlist = u->next;
        free(u);
    }
    if (ugen_currentcontext == dc) ugen_currentcontext = dc->parentcontext; else bug("ugen_currentcontext");
    free(dc);
}

t_signal *ugen_getiosig(int index, int inout) {
    if (!ugen_currentcontext) bug("ugen_getiosig");
    if (ugen_currentcontext->toplevel) return 0;
    if (inout) index += ugen_currentcontext->ninlets;
    return ugen_currentcontext->iosigs[index];
}

/* resampling code originally by Johannes Zmölnig in 2001 */
/* also "block-resampling" added by Johannes in 2004.09 */
/* --------------------- up/down-sampling --------------------- */
/* LATER: add some downsampling-filters for HOLD and LINEAR */

/* up: upsampling factor */
/* down: downsampling factor */
/* parent: original vectorsize */
t_int *downsampling_perform_0(t_int *w) {
  PERFORM4ARGS(t_float *,in, t_float *,out, int,down, int,parent);
  int n=parent/down;
  while(n--) {*out++=*in; in+=down;}
  return w+5;
}
/* the downsampled vector is exactly the first part of the parent vector; the rest of the parent is just skipped
 * cool for FFT-data, where you only want to process the significant (1st) part of the vector */
t_int *downsampling_perform_block(t_int *w) {
  PERFORM4ARGS(t_float *,in, t_float *,out, int,down, int,parent);
  int n=parent/down;
  while(n--) *out++=*in++;
  return w+5;
}
t_int *upsampling_perform_0(t_int *w) {
  PERFORM4ARGS(t_float *,in, t_float *,out, int,up, int,parent);
  int n=parent*up;
  t_float *dummy = out;
  while(n--) *out++=0;
  n = parent;
  out = dummy;
  while(n--) {*out=*in++; out+=up;}
  return w+5;
}
t_int *upsampling_perform_hold(t_int *w) {
  PERFORM4ARGS(t_float *,in, t_float *,out, int,up, int,parent);
  int i=up;
  t_float *dum_out = out;
  t_float *dum_in  = in;
  while (i--) {
    int n = parent;
    out = dum_out+i;
    in  = dum_in;
    while(n--) {*out=*in++; out+=up;}
  }
  return w+5;
}
t_int *upsampling_perform_linear(t_int *w) {
  PERFORM5ARGS(t_resample *,x, t_float *,in, t_float *,out, int,up, int,parent);
  const int length = parent*up;
  t_float a=*x->buffer, b=*in;
  const t_float up_inv = (t_float)1.0/up;
  t_float findex = 0.f;
  for (int n=0; n<length; n++) {
    const int index = int(findex+=up_inv);
    t_float frac=findex-index;
    if(frac==0.)frac=1.;
    *out++ = frac * b + (1.-frac) * a;
    t_float *fp=in+index;
    b=*fp;
    // do we still need the last sample of the previous pointer for interpolation ?
    a=(index)?*(fp-1):a;
  }
  *x->buffer = a;
  return w+6;
}
/* 1st part of the upsampled signal-vector will be the original one; 2nd part of the upsampled signal-vector is just 0
 * cool for FFT-data, where you only want to process the significant (1st) part of the vector */
t_int *upsampling_perform_block(t_int *w) {
  PERFORM4ARGS(t_float *,in, t_float *,out, int,up, int,parent);
  int i=parent;
  int n=parent*(up-1);
  while (i--) *out++=*in++;
  while (n--) *out++=0.f;
  return w+5;
}

/* ----------------------- public -------------------------------- */
/* utils */

void resample_init(t_resample *x) {
  x->method=0;
  x->downsample=x->upsample=1;
  x->n = x->coefsize = x->bufsize = 0;
  x->v = x->coeffs   = x->buffer  = 0;
}
void resample_free(t_resample *x) {
  if (x->n) free(x->v);
  if (x->coefsize) free(x->coeffs);
  if (x->bufsize) free(x->buffer);
  x->n = x->coefsize = x->bufsize = 0;
  x->v = x->coeffs   = x->buffer  = 0;
}
void resample_dsp(t_resample *x, t_sample* in, int insize, t_sample* out, int outsize, int method) {
  if (insize == outsize) {bug("nothing to be done"); return;}
  if (insize > outsize) { /* downsampling */
    if (insize % outsize) {error("bad downsampling factor"); return;}
    switch (method) {
    case RESAMPLE_BLOCK: dsp_add(downsampling_perform_block, 4, in, out, insize/outsize, insize); break;
    default:             dsp_add(downsampling_perform_0,     4, in, out, insize/outsize, insize);
    }
  } else { /* upsampling */
    if (outsize % insize) {error("bad upsampling factor"); return;}
    switch (method) {
    case RESAMPLE_HOLD: dsp_add(upsampling_perform_hold, 4, in, out, outsize/insize, insize); break;
    case RESAMPLE_LINEAR:
      if (x->bufsize != 1) {
        free(x->buffer);
        x->bufsize = 1;
        x->buffer = (t_float *)t_getbytes(x->bufsize*sizeof(*x->buffer));
      }
      dsp_add(upsampling_perform_linear, 5, x, in, out, outsize/insize, insize);
      break;
    case RESAMPLE_BLOCK: dsp_add(upsampling_perform_block, 4, in, out, outsize/insize, insize); break;
    default:             dsp_add(upsampling_perform_0,     4, in, out, outsize/insize, insize);
    }
  }
}
void resamplefrom_dsp(t_resample *x, t_sample *in, int insize, int outsize, int method) {
  if (insize==outsize) {           free(x->v); x->n = 0; x->v = in; return;}
  if (x->n != outsize) {           free(x->v); x->v = (t_float *)t_getbytes(outsize * sizeof(*x->v)); x->n = outsize;}
  resample_dsp(x, in, insize, x->v, x->n, method);
}
void resampleto_dsp(t_resample *x, t_sample *out, int insize, int outsize, int method) {
  if (insize==outsize) {if (x->n) free(x->v); x->n = 0; x->v = out; return;}
  if (x->n != insize) {            free(x->v); x->v = (t_float *)t_getbytes(insize * sizeof(*x->v)); x->n = insize;}
  resample_dsp(x, x->v, x->n, out, outsize, method);
}

/* ------------------------ samplerate~ -------------------------- */

static t_class *samplerate_tilde_class;
struct t_samplerate : t_object {
    t_canvas *canvas;
};
extern "C" void *canvas_getblock(t_class *blockclass, t_canvas **canvasp);
static void samplerate_tilde_bang(t_samplerate *x) {
    float srate = sys_getsr();
    t_canvas *canvas = x->canvas;
    while (canvas) {
        t_block *b = (t_block *)canvas_getblock(block_class, &canvas);
        if (b) srate *= float(b->upsample) / float(b->downsample);
    }
    x->ob_outlet->send(srate);
}
static void *samplerate_tilde_new(t_symbol *s) {
    t_samplerate *x = (t_samplerate *)pd_new(samplerate_tilde_class);
    outlet_new(x,&s_float);
    x->canvas = canvas_getcurrent();
    return x;
}
static void samplerate_tilde_setup() {
    samplerate_tilde_class = class_new2("samplerate~",samplerate_tilde_new,0,sizeof(t_samplerate),0,"");
    class_addbang(samplerate_tilde_class, samplerate_tilde_bang);
}

/* -------------------- setup routine -------------------------- */

void d_ugen_setup () {
    block_tilde_setup();
    samplerate_tilde_setup();
}
