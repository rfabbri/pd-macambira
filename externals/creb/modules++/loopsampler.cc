/*
 *   loopsampler.cc  -  a sampler object for looping
 *   Copyright (c) 2000-2003 by Tom Schouten
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "m_pd.h"
#include <math.h>


/* "soundfiler" object */

#define QUEUE_LENGTH 64;
typedef struct
{
} t_audiobuffer;



typedef struct
{
    t_soundfiler_queue q[QUEUE_LENGTH];

} t_soundfiler;





/* sample player class */


typedef struct loopsampler_struct
{
    t_object x_obj;
    t_float x_f;

    t_symbol *x_soundfile;
    t_int x_channels;
    t_int x_frames;
    t_float *x_buffer;

} t_loopsampler;

void loopsampler_bang(t_loopsampler *x)
{

}


static t_int *loopsampler_perform(t_int *w)
{


  t_loopsampler *x = (t_loopsampler *)(w[1]);
  t_int n          = (t_int)(w[2]);
  t_float *inL      = (float *)(w[3]);
  t_float *inR      = (float *)(w[4]);
  t_float *outL    = (float *)(w[5]);
  t_float *outR    = (float *)(w[6]);
  t_int i;
  t_float x;

  if (!x->x_buffer) goto nofile;




 nofile:
  while (n--){
      *outL++ = 0.0f;
      *outR++ = 0.0f;
  }

  return (w+7);
}

static void loopsampler_dsp(t_loopsampler *x, t_signal **sp)
{
    dsp_add(loopsampler_perform, 6, x->loopsampler, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);

}                                  
void loopsampler_free(void)
{

}

t_class *loopsampler_class;

void *loopsampler_new(t_floatarg fsections)
{
    t_loopsampler *x = (t_loopsampler *)pd_new(loopsampler_class);


    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  
    outlet_new(&x->x_obj, gensym("signal")); 
    outlet_new(&x->x_obj, gensym("signal")); 

    x->x_buffer = 0;
    x->x_soundfile = 0;
    x->x_channels = 0;
    x->x_frames = 0;

    return (void *)x;
}


extern "C" {

void loopsampler_tilde_setup(void)
{
    //post("loopsampler~ v0.1");

    loopsampler_class = class_new(gensym("loopsampler~"), (t_newmethod)loopsampler_new,
    	(t_method)loopsampler_free, sizeof(t_loopsampler), 0, A_DEFFLOAT, 0);

    CLASS_MAINSIGNALIN(loopsampler_class, t_loopsampler, x_f); 
    class_addmethod(loopsampler_class, (t_method)loopsampler_bang, gensym("bang"), (t_atomtype)0);
    class_addmethod(loopsampler_class, (t_method)loopsampler_dsp, gensym("dsp"), (t_atomtype)0); 


}

}
