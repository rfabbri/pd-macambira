/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2005 */

#include "m_pd.h"
#include "iemlib.h"


static t_class *iemlib2_class;

static void *iemlib2_new(void)
{
  t_object *x = (t_object *)pd_new(iemlib2_class);
  
  return (x);
}

void add2_comma_setup(void);
void bpe_setup(void);
void dollarg_setup(void);
void exp_inc_setup(void);
void float24_setup(void);
void iem_anything_setup(void);
void iem_append_setup(void);
void iem_i_route_setup(void);
void iem_pbank_csv_setup(void);
void iem_prepend_setup(void);
void iem_receive_setup(void);
void iem_route_setup(void);
void iem_sel_any_setup(void);
void iem_send_setup(void);
void init_setup(void);
void list2send_setup(void);
void mergefilename_setup(void);
void modulo_counter_setup(void);
void parentdollarzero_setup(void);
void post_netreceive_setup(void);
void pre_inlet_setup(void);
void prepend_ascii_setup(void);
void protect_against_open_setup(void);
void receive2list_setup(void);
void round_zero_setup(void);
void sigfade_setup(void);
void sigiem_blocksize_setup(void);
void sigiem_samplerate_setup(void);
void sigLFO_noise_setup(void);
void sigm2f_setup(void);
void speedlim_setup(void);
void splitfilename_setup(void);
void stripfilename_setup(void);
void toggle_mess_setup(void);
void transf_fader_setup(void);
void unsymbol_setup(void);
void wrap_setup(void);

/* ------------------------ setup routine ------------------------- */

void iemlib2_setup(void)
{
  iemlib2_class = class_new(gensym("iemlib2"), iemlib2_new, 0, sizeof(t_object), CLASS_NOINLET, 0);
  
  add2_comma_setup();
  bpe_setup();
  dollarg_setup();
  exp_inc_setup();
  float24_setup();
  iem_anything_setup();
  iem_append_setup();
  iem_i_route_setup();
  iem_pbank_csv_setup();
  iem_prepend_setup();
  iem_receive_setup();
  iem_route_setup();
  iem_sel_any_setup();
  iem_send_setup();
  init_setup();
  list2send_setup();
  mergefilename_setup();
  modulo_counter_setup();
  parentdollarzero_setup();
  post_netreceive_setup();
  pre_inlet_setup();
  prepend_ascii_setup();
  protect_against_open_setup();
  receive2list_setup();
  round_zero_setup();
  sigfade_setup();
  sigiem_blocksize_setup();
  sigiem_samplerate_setup();
  sigLFO_noise_setup();
  sigm2f_setup();
  speedlim_setup();
  splitfilename_setup();
  stripfilename_setup();
  toggle_mess_setup();
  transf_fader_setup();
  unsymbol_setup();
  wrap_setup();
    
  post("iemlib2 (R-1.16) library loaded!   (c) Thomas Musil 05.2005");
	post("   musil%ciem.at iem KUG Graz Austria", '@');
}
