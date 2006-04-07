/* flib - PD library for feature extraction 
   Copyright (C) 2005  Jamie Bullock

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
   */


/*Calculate the (non optimized) cross correlation of two signal vectors*/
/*Based on code by Phil Bourke */


#include "flib.h"
#define SQ(a) (a * a)

static t_class *cross_class;

typedef struct _cross {
    t_object  x_obj;
    t_float f;
    t_int delay;
} t_cross;

static t_int *cross_perform(t_int *w)
{
    t_sample *x = (t_sample *)(w[1]);
    t_sample *y = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    t_int N = (t_int)(w[4]),
	  i, j, delay;
    t_int maxdelay = (t_int)(w[5]);
    t_float mx, my, sx, sy, sxy, denom, r;
  
   if(maxdelay > N * .5){
	maxdelay = N * .5;
	post("cross~: invalid maxdelay, must be <= blocksize/2");
   } 
   else if (!maxdelay){
       maxdelay = N * .5;
   }
 
   /* Calculate the mean of the two series x[], y[] */
   mx = 0;
   my = 0;   
   for (i=0;i<N;i++) {
      mx += x[i];
      my += y[i];
   }
   mx /= N;
   my /= N;

   /* Calculate the denominator */
   sx = 0;
   sy = 0;
   for (i=0;i<N;i++) {
      sx += (x[i] - mx) * (x[i] - mx);
      sy += (y[i] - my) * (y[i] - my);
   }
   denom = sqrt(sx*sy);

   /* Calculate the correlation series */
   for (delay=-maxdelay;delay<maxdelay;delay++) {
      sxy = 0;
      for (i=0;i<N;i++) {
         j = i + delay;

	/* circular correlation */
	 while (j < 0)
            j += N;
         j %= N;
         sxy += (x[i] - mx) * (y[j] - my);

     }
      r = sxy / denom;
	*out++ = r;
      /* r is the correlation coefficient at "delay" */

   }

  return (w+6);

}

static void cross_dsp(t_cross *x, t_signal **sp)
{
    dsp_add(cross_perform, 5,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n, x->delay);
}

static void *cross_new(t_floatarg f)
{
    t_cross *x = (t_cross *)pd_new(cross_class);
    x->delay = (t_int)f;
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    return (void *)x;
}


void cross_tilde_setup(void) {
    cross_class = class_new(gensym("cross~"),
	    (t_newmethod)cross_new,
	    0, sizeof(t_cross),
	    CLASS_DEFAULT, A_DEFFLOAT, 0);

    class_addmethod(cross_class,
	    (t_method)cross_dsp, gensym("dsp"), 0);
    CLASS_MAINSIGNALIN(cross_class, t_cross,f);
    class_sethelpsymbol(cross_class, gensym("help-flib"));
}
