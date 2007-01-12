/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* tab16write~, tab16play~, tab16read~, tab16read4~, tab16send~, tab16receive~ */

#include "iem16_table.h"

/* ------------------------- tab16write~ -------------------------- */

static t_class *tab16write_tilde_class;

typedef struct _tab16write_tilde {
  t_object x_obj;
  int x_phase;
  int x_nsampsintab;
  short *x_vec;
  t_symbol *x_arrayname;
  float x_f;
} t_tab16write_tilde;

static void *tab16write_tilde_new(t_symbol *s) {
  t_tab16write_tilde *x = (t_tab16write_tilde *)pd_new(tab16write_tilde_class);
  x->x_phase = 0x7fffffff;
  x->x_arrayname = s;
  x->x_f = 0;
  return (x);
}

static t_int *tab16write_tilde_perform(t_int *w) {
  t_tab16write_tilde *x = (t_tab16write_tilde *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  int n = (int)(w[3]), phase = x->x_phase, endphase = x->x_nsampsintab;
  if (!x->x_vec) goto bad;
    
  if (endphase > phase)    {
    int nxfer = endphase - phase;
    t_iem16_16bit *fp = x->x_vec + phase;
    if (nxfer > n) nxfer = n;
    phase += nxfer;
    while (nxfer--)*fp++ = *in++*IEM16_SCALE_UP;
    x->x_phase = phase;
  }
 bad:
  return (w+4);
}

void tab16write_tilde_set(t_tab16write_tilde *x, t_symbol *s){
  t_table16 *a;

  x->x_arrayname = s;
  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))    {
    if (*s->s_name) pd_error(x, "tab16write~: %s: no such array",
			     x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else if (!table16_getarray16(a, &x->x_nsampsintab, &x->x_vec))    {
    error("%s: bad template for tab16write~", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else table16_usedindsp(a);
}

static void tab16write_tilde_dsp(t_tab16write_tilde *x, t_signal **sp){
  tab16write_tilde_set(x, x->x_arrayname);
  dsp_add(tab16write_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void tab16write_tilde_bang(t_tab16write_tilde *x){
  x->x_phase = 0;
}

static void tab16write_tilde_stop(t_tab16write_tilde *x){}

static void tab16write_tilde_free(t_tab16write_tilde *x){}

void tab16write_tilde_setup(void){
  tab16write_tilde_class = class_new(gensym("tab16write~"),
				     (t_newmethod)tab16write_tilde_new, (t_method)tab16write_tilde_free,
				     sizeof(t_tab16write_tilde), 0, A_DEFSYM, 0);
  CLASS_MAINSIGNALIN(tab16write_tilde_class, t_tab16write_tilde, x_f);
  class_addmethod(tab16write_tilde_class, (t_method)tab16write_tilde_dsp,
		  gensym("dsp"), 0);
  class_addmethod(tab16write_tilde_class, (t_method)tab16write_tilde_set,
		  gensym("set"), A_SYMBOL, 0);
  class_addmethod(tab16write_tilde_class, (t_method)tab16write_tilde_stop,
		  gensym("stop"), 0);
  class_addbang(tab16write_tilde_class, tab16write_tilde_bang);
}

/* ------------ tab16play~ - non-transposing sample playback --------------- */

static t_class *tab16play_tilde_class;

typedef struct _tab16play_tilde{
  t_object x_obj;
  t_outlet *x_bangout;
  int x_phase;
  int x_nsampsintab;
  int x_limit;
  t_iem16_16bit *x_vec;
  t_symbol *x_arrayname;
} t_tab16play_tilde;

static void *tab16play_tilde_new(t_symbol *s){
  t_tab16play_tilde *x = (t_tab16play_tilde *)pd_new(tab16play_tilde_class);
  x->x_phase = 0x7fffffff;
  x->x_limit = 0;
  x->x_arrayname = s;
  outlet_new(&x->x_obj, &s_signal);
  x->x_bangout = outlet_new(&x->x_obj, &s_bang);
  return (x);
}

static t_int *tab16play_tilde_perform(t_int *w){
  t_tab16play_tilde *x = (t_tab16play_tilde *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_iem16_16bit *fp;
  int n = (int)(w[3]), phase = x->x_phase,
    endphase = (x->x_nsampsintab < x->x_limit ?
		x->x_nsampsintab : x->x_limit), nxfer, n3;
  if (!x->x_vec || phase >= endphase)	goto zero;
    
  nxfer = endphase - phase;
  fp = x->x_vec + phase;
  if (nxfer > n)
    nxfer = n;
  n3 = n - nxfer;
  phase += nxfer;
  while (nxfer--) *out++ = *fp++*IEM16_SCALE_DOWN;
  if (phase >= endphase)  {
    x->x_phase = 0x7fffffff;
    while (n3--) *out++ = 0;
  }
  else x->x_phase = phase;
    
  return (w+4);
 zero:
  while (n--) *out++ = 0;
  return (w+4);
}

void tab16play_tilde_set(t_tab16play_tilde *x, t_symbol *s){
  t_table16 *a;

  x->x_arrayname = s;
  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))    {
    if (*s->s_name) pd_error(x, "tab16play~: %s: no such array",
			     x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else if (!table16_getarray16(a, &x->x_nsampsintab, &x->x_vec))    {
    error("%s: bad template for tab16play~", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else table16_usedindsp(a);
}

static void tab16play_tilde_dsp(t_tab16play_tilde *x, t_signal **sp){
  tab16play_tilde_set(x, x->x_arrayname);
  dsp_add(tab16play_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void tab16play_tilde_list(t_tab16play_tilde *x, t_symbol *s,
				 int argc, t_atom *argv){
  long start = atom_getfloatarg(0, argc, argv);
  long length = atom_getfloatarg(1, argc, argv);
  if (start < 0) start = 0;
  if (length <= 0)x->x_limit = 0x7fffffff;
  else	    x->x_limit = start + length;
  x->x_phase = start;
}

static void tab16play_tilde_stop(t_tab16play_tilde *x){
  x->x_phase = 0x7fffffff;
}

static void tab16play_tilde_free(t_tab16play_tilde *x){}

void tab16play_tilde_setup(void){
  tab16play_tilde_class = class_new(gensym("tab16play~"),
				    (t_newmethod)tab16play_tilde_new, (t_method)tab16play_tilde_free,
				    sizeof(t_tab16play_tilde), 0, A_DEFSYM, 0);
  class_addmethod(tab16play_tilde_class, (t_method)tab16play_tilde_dsp,
		  gensym("dsp"), 0);
  class_addmethod(tab16play_tilde_class, (t_method)tab16play_tilde_stop,
		  gensym("stop"), 0);
  class_addmethod(tab16play_tilde_class, (t_method)tab16play_tilde_set,
		  gensym("set"), A_DEFSYM, 0);
  class_addlist(tab16play_tilde_class, tab16play_tilde_list);
}

/* ------------------------ tab16send~ ------------------------- */

static t_class *tab16send_class;

typedef struct _tab16send{
  t_object x_obj;
  t_iem16_16bit *x_vec;
  int x_graphperiod;
  int x_graphcount;
  t_symbol *x_arrayname;
  float x_f;
} t_tab16send;

static void *tab16send_new(t_symbol *s){
  t_tab16send *x = (t_tab16send *)pd_new(tab16send_class);
  x->x_graphcount = 0;
  x->x_arrayname = s;
  x->x_f = 0;
  return (x);
}

static t_int *tab16send_perform(t_int *w){
  t_tab16send *x = (t_tab16send *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  int n = w[3];
  t_iem16_16bit *dest = x->x_vec;
  int i = x->x_graphcount;
  if (!x->x_vec) goto bad;

  while (n--)	*dest = *in++*IEM16_SCALE_UP;
  if (!i--)i = x->x_graphperiod;
  x->x_graphcount = i;
 bad:
  return (w+4);
}

static void tab16send_dsp(t_tab16send *x, t_signal **sp){
  int vecsize;
  t_table16 *a;

  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))    {
    if (*x->x_arrayname->s_name)
      error("tab16send~: %s: no such array", x->x_arrayname->s_name);
  }
  else if (!table16_getarray16(a, &vecsize, &x->x_vec))
    error("%s: bad template for tab16send~", x->x_arrayname->s_name);
  else    {
    int n = sp[0]->s_n;
    int ticksper = sp[0]->s_sr/n;
    if (ticksper < 1) ticksper = 1;
    x->x_graphperiod = ticksper;
    if (x->x_graphcount > ticksper) x->x_graphcount = ticksper;
    if (n < vecsize) vecsize = n;
    table16_usedindsp(a);
    dsp_add(tab16send_perform, 3, x, sp[0]->s_vec, vecsize);
  }
}

static void tab16send_free(t_tab16send *x){}

static void tab16send_setup(void){
  tab16send_class = class_new(gensym("tab16send~"), (t_newmethod)tab16send_new,
			      (t_method)tab16send_free, sizeof(t_tab16send), 0, A_DEFSYM, 0);
  CLASS_MAINSIGNALIN(tab16send_class, t_tab16send, x_f);
  class_addmethod(tab16send_class, (t_method)tab16send_dsp, gensym("dsp"), 0);
}

// G.Holzmann: for PD-extended build system
void tab16send_tilde_setup(void)
{
  tab16send_setup();
}

/* ------------------------ tab16receive~ ------------------------- */

static t_class *tab16receive_class;

typedef struct _tab16receive{
  t_object x_obj;
  t_iem16_16bit *x_vec;
  t_symbol *x_arrayname;
} t_tab16receive;

static t_int *tab16receive_perform(t_int *w){
  t_tab16receive *x = (t_tab16receive *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = w[3];
  t_iem16_16bit *from = x->x_vec;
  if (from) while (n--) *out++ = *from++*IEM16_SCALE_DOWN;
  else while (n--) *out++ = 0;
  return (w+4);
}

static void tab16receive_dsp(t_tab16receive *x, t_signal **sp){
  t_table16 *a;
  int vecsize;
    
  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class))) {
    if (*x->x_arrayname->s_name)
      error("tab16send~: %s: no such array", x->x_arrayname->s_name);
  }
  else if (!table16_getarray16(a, &vecsize, &x->x_vec))
    error("%s: bad template for tab16receive~", x->x_arrayname->s_name);
  else     {
    int n = sp[0]->s_n;
    if (n < vecsize) vecsize = n;
    table16_usedindsp(a);
    dsp_add(tab16receive_perform, 3, x, sp[0]->s_vec, vecsize);
  }
}

static void *tab16receive_new(t_symbol *s){
  t_tab16receive *x = (t_tab16receive *)pd_new(tab16receive_class);
  x->x_arrayname = s;
  outlet_new(&x->x_obj, &s_signal);
  return (x);
}

static void tab16receive_setup(void){
  tab16receive_class = class_new(gensym("tab16receive~"),
				 (t_newmethod)tab16receive_new, 0,
				 sizeof(t_tab16receive), 0, A_DEFSYM, 0);
  class_addmethod(tab16receive_class, (t_method)tab16receive_dsp,
		  gensym("dsp"), 0);
}

// G.Holzmann: for PD-extended build system
void tab16receive_tilde_setup(void)
{
  tab16receive_setup();
}

/******************** tab16read~ ***********************/

static t_class *tab16read_tilde_class;

typedef struct _tab16read_tilde{
  t_object x_obj;
  int x_npoints;
  t_iem16_16bit *x_vec;
  t_symbol *x_arrayname;
  float x_f;
} t_tab16read_tilde;

static void *tab16read_tilde_new(t_symbol *s){
  t_tab16read_tilde *x = (t_tab16read_tilde *)pd_new(tab16read_tilde_class);
  x->x_arrayname = s;
  x->x_vec = 0;
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_f = 0;
  return (x);
}

static t_int *tab16read_tilde_perform(t_int *w){
  t_tab16read_tilde *x = (t_tab16read_tilde *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);    
  int maxindex;
  t_iem16_16bit *buf = x->x_vec;
  int i;
    
  maxindex = x->x_npoints - 1;
  if (!buf) goto zero;

  for (i = 0; i < n; i++)    {
    int index = *in++;
    if (index < 0) index = 0;
    else if (index > maxindex) index = maxindex;
    *out++ = buf[index]*IEM16_SCALE_DOWN;
  }
  return (w+5);
 zero:
  while (n--) *out++ = 0;

  return (w+5);
}

void tab16read_tilde_set(t_tab16read_tilde *x, t_symbol *s){
  t_table16 *a;
    
  x->x_arrayname = s;
  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))  {
    if (*s->s_name)
      error("tab16read~: %s: no such array", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else if (!table16_getarray16(a, &x->x_npoints, &x->x_vec)) {
    error("%s: bad template for tab16read~", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else table16_usedindsp(a);
}

static void tab16read_tilde_dsp(t_tab16read_tilde *x, t_signal **sp){
  tab16read_tilde_set(x, x->x_arrayname);

  dsp_add(tab16read_tilde_perform, 4, x,
	  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void tab16read_tilde_free(t_tab16read_tilde *x){}

void tab16read_tilde_setup(void){
  tab16read_tilde_class = class_new(gensym("tab16read~"),
				    (t_newmethod)tab16read_tilde_new, (t_method)tab16read_tilde_free,
				    sizeof(t_tab16read_tilde), 0, A_DEFSYM, 0);
  CLASS_MAINSIGNALIN(tab16read_tilde_class, t_tab16read_tilde, x_f);
  class_addmethod(tab16read_tilde_class, (t_method)tab16read_tilde_dsp,
		  gensym("dsp"), 0);
  class_addmethod(tab16read_tilde_class, (t_method)tab16read_tilde_set,
		  gensym("set"), A_SYMBOL, 0);
}

/******************** tab16read4~ ***********************/

static t_class *tab16read4_tilde_class;

typedef struct _tab16read4_tilde{
  t_object x_obj;
  int x_npoints;
  t_iem16_16bit *x_vec;
  t_symbol *x_arrayname;
  float x_f;
} t_tab16read4_tilde;

static void *tab16read4_tilde_new(t_symbol *s){
  t_tab16read4_tilde *x = (t_tab16read4_tilde *)pd_new(tab16read4_tilde_class);
  x->x_arrayname = s;
  x->x_vec = 0;
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_f = 0;
  return (x);
}

static t_int *tab16read4_tilde_perform(t_int *w){
  t_tab16read4_tilde *x = (t_tab16read4_tilde *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);    
  int maxindex;
  t_iem16_16bit *buf = x->x_vec;
  t_iem16_16bit *fp;
  int i;
    
  maxindex = x->x_npoints - 3;

  if (!buf) goto zero;

  for (i = 0; i < n; i++)    {
    float findex = *in++;
    int index = findex;
    float frac,  a,  b,  c,  d, cminusb;
    if (index < 1)   index = 1, frac = 0;
    else if (index > maxindex)  index = maxindex, frac = 1;
    else frac = findex - index;
    fp = buf + index;
    a = fp[-1]*IEM16_SCALE_DOWN;
    b = fp[0]*IEM16_SCALE_DOWN;
    c = fp[1]*IEM16_SCALE_DOWN;
    d = fp[2]*IEM16_SCALE_DOWN;
    cminusb = c-b;
    *out++ = b + frac * (
			 cminusb - 0.5f * (frac-1.) * (
				(a - d + 3.0f * cminusb) * frac + (b - a - cminusb)
				)
			 );
  }
  return (w+5);
 zero:
  while (n--) *out++ = 0;
  
  return (w+5);
}

void tab16read4_tilde_set(t_tab16read4_tilde *x, t_symbol *s){
  t_table16 *a;
    
  x->x_arrayname = s;
  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class))) {
    if (*s->s_name)
      error("tab16read4~: %s: no such array", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else if (!table16_getarray16(a, &x->x_npoints, &x->x_vec))  {
    error("%s: bad template for tab16read4~", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else table16_usedindsp(a);
}

static void tab16read4_tilde_dsp(t_tab16read4_tilde *x, t_signal **sp){
  tab16read4_tilde_set(x, x->x_arrayname);

  dsp_add(tab16read4_tilde_perform, 4, x,
	  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void tab16read4_tilde_setup(void){
  tab16read4_tilde_class = class_new(gensym("tab16read4~"),
				     (t_newmethod)tab16read4_tilde_new, 0,
				     sizeof(t_tab16read4_tilde), 0, A_DEFSYM, 0);
  CLASS_MAINSIGNALIN(tab16read4_tilde_class, t_tab16read4_tilde, x_f);
  class_addmethod(tab16read4_tilde_class, (t_method)tab16read4_tilde_dsp,
		  gensym("dsp"), 0);
  class_addmethod(tab16read4_tilde_class, (t_method)tab16read4_tilde_set,
		  gensym("set"), A_SYMBOL, 0);
}
/* ------------------------ global setup routine ------------------------- */

void iem16_array_tilde_setup(void){
  tab16write_tilde_setup();
  tab16play_tilde_setup();
  tab16read_tilde_setup();
  tab16read4_tilde_setup();
  tab16send_setup();
  tab16receive_setup();
}

