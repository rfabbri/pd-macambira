/*
 *   Pure Data Packet module.
 *   Copyright (c) 2003 by Tom Schouten <pdp@zzz.kotnet.org>
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
 *
 */


#include <math.h>

#include "pdp.h"
#include "pdp_base.h"


typedef struct _pdp_array
{
    t_object x_obj;
    t_symbol *x_array_sym;
    t_outlet *x_outlet; // for array->pdp
    t_int x_rows;

    /* the packet */
    int x_packet0;

} t_pdp_array;


static void pdp_array_bang(t_pdp_array *x)
{
    post("not implemented");
}


static void pdp_array_input_0(t_pdp_array *x, t_symbol *s, t_floatarg f)
{
    int packet = (int)f;


    /* register */
    if (s == gensym("register_ro")){
	/* replace if not compatible or we are not interpolating */
	pdp_packet_mark_unused(x->x_packet0);
	x->x_packet0 = pdp_packet_convert_ro(packet, pdp_gensym("image/grey/*"));
    }

    /* process */
    if (s == gensym("process")){
	float *vec;
	int nbpoints;
	t_garray *a;
	t_pdp *header = pdp_packet_header(x->x_packet0);
	short int *data = pdp_packet_data(x->x_packet0);
	if (!header || !data) return;

	/* dump to array if possible */
	if (!x->x_array_sym){
	}

	/* check if array is valid */
	else if (!(a = (t_garray *)pd_findbyclass(x->x_array_sym, garray_class))){
	    post("pdp_array: %s: no such array", x->x_array_sym->s_name);
	}
	/* get data */
	else if (!garray_getfloatarray(a, &nbpoints, &vec)){
	    post("pdp_array: %s: bad template", x->x_array_sym->s_name);
	}
	/* scale and dump in array */
	else{
	    int i;
	    int w = header->info.image.width;
	    int h = header->info.image.height;
	    int N = w*h;
	    N = (nbpoints < N) ? nbpoints : N;

	    /* scan rows */
	    if (x->x_rows){
		for (i=0; i<N; i++) 
		    vec[i] = (float)data[i] * (1.0f / (float)0x8000);
	    }
	    /* scan columns */
	    else{
		for (i=0; i<N; i++) {
		    int x = i / h;
		    int y = i % h;
		    vec[i] = (float)data[x+(h-y-1)*w] * (1.0f / (float)0x8000);
		}
	    }
	    //garray_redraw(a);
	}

    }
}

static void pdp_array_array(t_pdp_array *x, t_symbol *s)
{
    //post("setting symbol %x", s);
    x->x_array_sym = s;
    x->x_packet0 = -1;
}


static void pdp_array_free(t_pdp_array *x)
{
    pdp_packet_mark_unused(x->x_packet0);
}


t_class *pdp_array2grey_class;
t_class *pdp_grey2array_class;



void *pdp_array2grey_new(t_symbol *s, t_symbol *r)
{
    t_pdp_array *x = (t_pdp_array *)pd_new(pdp_array2grey_class);
    pdp_array_array(x, s);
    return (void *)x;
}

void *pdp_grey2array_new(t_symbol *s, t_symbol *r)
{
    t_pdp_array *x = (t_pdp_array *)pd_new(pdp_grey2array_class);
    pdp_array_array(x, s);
    if (r == gensym("rows")){
	x->x_rows = 1;
	post("pdp_grey2array: scanning rows");
    }
    else {
	x->x_rows = 0;
	post("pdp_grey2array: scanning columns");
    }
    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_array_setup(void)
{

    pdp_array2grey_class = class_new(gensym("pdp_array2grey"), 
				     (t_newmethod)pdp_array2grey_new,
				     (t_method)pdp_array_free, 
				     sizeof(t_pdp_array), 
				     0, A_DEFSYMBOL, A_DEFSYMBOL, A_NULL);

    pdp_grey2array_class = class_new(gensym("pdp_grey2array"), 
				     (t_newmethod)pdp_grey2array_new,
				     (t_method)pdp_array_free, 
				     sizeof(t_pdp_array), 
				     0, A_DEFSYMBOL, A_DEFSYMBOL, A_NULL);


    /* packet input */
    class_addmethod(pdp_grey2array_class, 
		    (t_method)pdp_array_input_0, 
		    gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

    /* bang method */
    class_addmethod(pdp_array2grey_class, 
		    (t_method)pdp_array_bang, gensym("bang"), A_NULL);


    /* bookkeeping */
    class_addmethod(pdp_array2grey_class, (t_method)pdp_array_array, 
		    gensym("array"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_grey2array_class, (t_method)pdp_array_array, 
		    gensym("array"), A_SYMBOL, A_NULL);

}

#ifdef __cplusplus
}
#endif



