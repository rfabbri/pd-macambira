/*
 *   Pure Data Packet module.
 *   Copyright (c) by Tom Schouten <pdp@zzz.kotnet.org>
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



#include "pdp.h"
#include "pdp_mmx.h"


typedef struct pdp_conv_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;


    int x_packet0;
    int x_dropped;
    int x_queue_id;

    unsigned int x_nbpasses;
    bool x_horizontal;
    bool x_vertical;

    void *x_convolver_hor;
    void *x_convolver_ver;

} t_pdp_conv;



static void pdp_conv_process_yv12(t_pdp_conv *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    void  *data0   = pdp_packet_data  (x->x_packet0);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    unsigned int size = w*h;
    unsigned int v_offset = size;
    unsigned int u_offset = size + (size >> 2) ;
    

    unsigned int i,j;
    unsigned int nbpasses = x->x_nbpasses;

    short int * idata = (short int *)data0;

    int orient;

    


    orient = PDP_IMAGEPROC_CONV_VERTICAL;
    if (x->x_vertical){
	pdp_imageproc_conv_process(x->x_convolver_ver, idata, w, h, orient, nbpasses);
	pdp_imageproc_conv_process(x->x_convolver_ver, idata+v_offset, w>>1, h>>1, orient, nbpasses);
	pdp_imageproc_conv_process(x->x_convolver_ver, idata+u_offset, w>>1, h>>1, orient, nbpasses);
    }



    orient = PDP_IMAGEPROC_CONV_HORIZONTAL;
    if (x->x_horizontal){
	pdp_imageproc_conv_process(x->x_convolver_hor, idata, w, h, orient, nbpasses);
	pdp_imageproc_conv_process(x->x_convolver_hor, idata+v_offset, w>>1, h>>1, orient, nbpasses);
	pdp_imageproc_conv_process(x->x_convolver_hor, idata+u_offset, w>>1, h>>1, orient, nbpasses);
    }


    return;


}
static void pdp_conv_process_grey(t_pdp_conv *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    void  *data0   = pdp_packet_data  (x->x_packet0);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    unsigned int size = w*h;
    

    unsigned int i,j;
    unsigned int nbpasses = x->x_nbpasses;

    short int * idata = (short int *)data0;

    int orient;

    if (x->x_vertical){
	orient = PDP_IMAGEPROC_CONV_VERTICAL;
	pdp_imageproc_conv_process(x->x_convolver_ver, idata, w, h, orient, nbpasses);
    }

    if (x->x_horizontal){
	orient = PDP_IMAGEPROC_CONV_HORIZONTAL;
	pdp_imageproc_conv_process(x->x_convolver_hor, idata, w, h, orient, nbpasses);
    }


}

static void pdp_conv_process(t_pdp_conv *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);

    /* check data packets */

    if ((header0) && (PDP_IMAGE == header0->type)){

	/* pdp_conv_process inputs and write into active inlet */
	switch(header0->info.image.encoding){

	case PDP_IMAGE_YV12:
	    pdp_conv_process_yv12(x);
	    break;

	case PDP_IMAGE_GREY:
	    pdp_conv_process_grey(x);
	    break;

	default:
	    break;
	    /* don't know the type, so dont process */
	    
	}
    }

}


static void pdp_conv_sendpacket(t_pdp_conv *x)
{
    /* unregister and propagate if valid packet */
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_conv_input_0(t_pdp_conv *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;
    int passes, i;

    if (s== gensym("register_rw"))  x->x_dropped = pdp_packet_copy_rw_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){

	/* add the process method and callback to the process queue */

	pdp_queue_add(x, pdp_conv_process, pdp_conv_sendpacket, &x->x_queue_id);
    }

}




static void pdp_conv_passes(t_pdp_conv *x, t_floatarg f)
{
    int passes = (int)f;
    passes = passes < 0 ? 0 : passes;
    x->x_nbpasses = passes;

}
static void pdp_conv_hor(t_pdp_conv *x, t_floatarg f)
{
    int hor = (int)f;
    x->x_horizontal = (hor != 0);

}
static void pdp_conv_ver(t_pdp_conv *x, t_floatarg f)
{
    int ver = (int)f;
    x->x_vertical = (ver != 0);
}
static void pdp_conv_free(t_pdp_conv *x)
{
    pdp_queue_finish(x->x_queue_id);
    pdp_imageproc_conv_delete(x->x_convolver_hor);
    pdp_imageproc_conv_delete(x->x_convolver_ver);
    pdp_packet_mark_unused(x->x_packet0);

}

/* setup hmask */

static void pdp_conv_hleft(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setmin1(x->x_convolver_hor, f);

}
static void pdp_conv_hmiddle(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setzero(x->x_convolver_hor, f);
}
static void pdp_conv_hright(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setplus1(x->x_convolver_hor, f);
}

static void pdp_conv_hmask(t_pdp_conv *x, t_floatarg l, t_floatarg m, t_floatarg r)
{
  pdp_conv_hleft(x, l);
  pdp_conv_hmiddle(x, m);
  pdp_conv_hright(x, r);
}

static void pdp_conv_vtop(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setmin1(x->x_convolver_ver, f);
}
static void pdp_conv_vmiddle(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setzero(x->x_convolver_ver, f);

}
static void pdp_conv_vbottom(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setplus1(x->x_convolver_ver, f);
}

static void pdp_conv_vmask(t_pdp_conv *x, t_floatarg l, t_floatarg m, t_floatarg r)
{
  pdp_conv_vtop(x, l);
  pdp_conv_vmiddle(x, m);
  pdp_conv_vbottom(x, r);
}


static void pdp_conv_mask(t_pdp_conv *x, t_floatarg l, t_floatarg m, t_floatarg r)
{
  pdp_conv_hmask(x, l, m, r);
  pdp_conv_vmask(x, l, m, r);
}

t_class *pdp_conv_class;



void *pdp_conv_new(void)
{
    t_pdp_conv *x = (t_pdp_conv *)pd_new(pdp_conv_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("passes"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_queue_id = -1;
    x->x_nbpasses = 1;
    x->x_horizontal = true;
    x->x_vertical = true;

    x->x_convolver_hor = pdp_imageproc_conv_new();
    x->x_convolver_ver = pdp_imageproc_conv_new();

    pdp_imageproc_conv_setbordercolor(x->x_convolver_hor, 0);
    pdp_imageproc_conv_setbordercolor(x->x_convolver_ver, 0);

    pdp_conv_mask(x, .25,.5,.25);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_conv_setup(void)
{


    pdp_conv_class = class_new(gensym("pdp_conv"), (t_newmethod)pdp_conv_new,
    	(t_method)pdp_conv_free, sizeof(t_pdp_conv), 0, A_NULL);


    class_addmethod(pdp_conv_class, (t_method)pdp_conv_passes, gensym("passes"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_hor, gensym("hor"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_ver, gensym("ver"),  A_DEFFLOAT, A_NULL);   

    class_addmethod(pdp_conv_class, (t_method)pdp_conv_hleft, gensym("hleft"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_hmiddle, gensym("hmiddle"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_hright, gensym("hright"),  A_DEFFLOAT, A_NULL);   

    class_addmethod(pdp_conv_class, (t_method)pdp_conv_vtop, gensym("vtop"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_vmiddle, gensym("vmiddle"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_vbottom, gensym("vbottom"),  A_DEFFLOAT, A_NULL);   

    class_addmethod(pdp_conv_class, (t_method)pdp_conv_vmask, gensym("vmask"),  A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_hmask, gensym("hmask"),  A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_mask, gensym("mask"),  A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);   

    class_addmethod(pdp_conv_class, (t_method)pdp_conv_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
