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
#include <math.h>

/* computes a transfer function:
 *
 *         b0 + b1 z^(-1) + b2 z^(-2)
 *  T(z) = --------------------------
 *         1  + a1 z^(-1) + a2 z^(-2)
 *
 * TODO: think about scaling.
 */


/*
 * TODO:
 * add another time processing class
 * add methods for separate horizontal/vertical filters
 */


typedef struct pdp_bq_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    //_outlet *x_outlet1;
    //t_outlet *x_outlet2;

    /* pass through packet */
    int x_packet0;

    /* state packets for bqt */
    int x_packet1;
    int x_packet2;

    int x_dropped;
    int x_queue_id;

    unsigned int x_nbpasses;

    /* single direction */
    unsigned int x_direction;

    bool x_reset_on_formatchange;


    void *x_biquad;

    float x_coefs_a[3]; // a0, -a1, -a2
    float x_coefs_b[3]; // b0, b1, b2
    float x_state_u[2]; // u0, u1
    float x_state_u_save[2]; // u0, u1 (for reset)

} t_pdp_bq;


/************************* COEFFICIENT METHODS ****************************/

static void pdp_bq_a0(t_pdp_bq *x, t_floatarg f){x->x_coefs_a[0] = f;}
static void pdp_bq_a1(t_pdp_bq *x, t_floatarg f){x->x_coefs_a[1] = -f;}
static void pdp_bq_a2(t_pdp_bq *x, t_floatarg f){x->x_coefs_a[2] = -f;}

static void pdp_bq_b0(t_pdp_bq *x, t_floatarg f){x->x_coefs_b[0] = f;}
static void pdp_bq_b1(t_pdp_bq *x, t_floatarg f){x->x_coefs_b[1] = f;}
static void pdp_bq_b2(t_pdp_bq *x, t_floatarg f){x->x_coefs_b[2] = f;}

static void pdp_bq_u0(t_pdp_bq *x, t_floatarg f){x->x_state_u_save[0] = f;}
static void pdp_bq_u1(t_pdp_bq *x, t_floatarg f){x->x_state_u_save[1] = f;}


static void pdp_bq_setcoefs(t_pdp_bq *x, 
			    float a0, float a1, float a2,
			    float b0, float b1, float b2)
{
    pdp_bq_a0(x,a0);
    pdp_bq_a1(x,a1);
    pdp_bq_a2(x,a2);
    pdp_bq_b0(x,b0);
    pdp_bq_b1(x,b1);
    pdp_bq_b2(x,b2);
    pdp_imageproc_bq_setcoef(x->x_biquad, x->x_coefs_a);
}

static void pdp_bq_setstate(t_pdp_bq *x, float u0, float u1)
{
    pdp_bq_u0(x,u0);
    pdp_bq_u1(x,u1);
    pdp_imageproc_bq_setcoef(x->x_biquad, x->x_coefs_a);
}



/* reso lowpass */
static void pdp_bq_lpf(t_pdp_bq *x, t_floatarg f, t_floatarg Q)
{
    float a0, a1, a2, b0, b1, b2, cs, sn, w, alpha;
    w = 2.0 * M_PI * f;
    cs = cos(w);
    sn = sin(w);
	
    alpha = sn*sinh(1.0f/(2.0f*Q));
    b0 = (1.0 - cs)/2.0;
    b1 =  1.0 - cs;
    b2 = (1.0 - cs)/2.0;    
    a0 = (1.0 + alpha);
    a1 = -2.0*cs;
    a2 =  1.0 - alpha;

    pdp_bq_setcoefs(x, a0, a1, a2, b0, b1, b2);

}

/* reso highpass */
static void pdp_bq_hpf(t_pdp_bq *x, t_floatarg f, t_floatarg Q)
{
    float a0, a1, a2, b0, b1, b2, cs, sn, w, alpha;
    w = 2.0 * M_PI * f;
    cs = cos(w);
    sn = sin(w);
	
    alpha = sn*sinh(1.0f/(2.0f*Q));

    b0 = (1.0 + cs)/2.0;
    b1 = -1.0 - cs;
    b2 = (1.0 + cs)/2.0;    
    a0 = (1.0 + alpha);
    a1 = -2.0*cs;
    a2 =  1.0 - alpha;

    pdp_bq_setcoefs(x, a0, a1, a2, b0, b1, b2);

}


/* reso allpass */
static void pdp_bq_apf(t_pdp_bq *x, t_floatarg f, t_floatarg Q)
{
    float a0, a1, a2, b0, b1, b2, cs, sn, w, alpha;
    w = 2.0 * M_PI * f;
    cs = cos(w);
    sn = sin(w);
	
    alpha = sn*sinh(1.0f/(2.0f*Q));

    b0 = (1.0 - alpha);
    b1 = -2.0 * cs;
    b2 = (1.0 + alpha);    
    a0 =  (1.0 + alpha);
    a1 = -2.0*cs;
    a2 =  1.0 - alpha;

    pdp_bq_setcoefs(x, a0, a1, a2, b0, b1, b2);
}

/* reso band stop (notch) */
static void pdp_bq_bsf(t_pdp_bq *x, t_floatarg f, t_floatarg Q)
{
    float a0, a1, a2, b0, b1, b2, cs, sn, w, alpha;
    w = 2.0 * M_PI * f;
    cs = cos(w);
    sn = sin(w);
	
    alpha = sn*sinh(1.0f/(2.0f*Q));

    b0 = 1.0;
    b1 = -2.0 * cs;
    b2 = 1.0;    
    a0 =  (1.0 + alpha);
    a1 = -2.0*cs;
    a2 =  1.0 - alpha;

    pdp_bq_setcoefs(x, a0, a1, a2, b0, b1, b2);

}

static void pdp_bq_onep(t_pdp_bq *x, t_floatarg f)
{
    float a0,a1,a2,b0,b1,b2;

    if (f>1.0f) f = 1.0f;
    if (f<0.0f) f = 0.0f;

    a0 = 1.0f;
    a1 = -(1.0f - f);
    a2 = 0.0f;
    b0 = f;
    b1 = 0.0f;
    b2 = 0.0f;
    pdp_bq_setcoefs(x, a0, a1, a2, b0, b1, b2);
}

static void pdp_bq_twop(t_pdp_bq *x, t_floatarg f)
{
    float f1;
    float a0,a1,a2,b0,b1,b2;

    if (f>1.0) f = 1.0;
    if (f<0.0) f = 0.0;

    f1 = 1.0 - f;

    a0 = 1.0f;
    a1 = -2.0f*f1;
    a2 = f1*f1;
    b0 = f*f;
    b1 = 0.0f;
    b2 = 0.0f;

    pdp_bq_setcoefs(x, a0, a1, a2, b0, b1, b2);
}





/************************* PROCESS METHODS ****************************/


static void pdp_bqt_process_yv12(t_pdp_bq *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    t_pdp *header2 = pdp_packet_header(x->x_packet2);

    void  *data0   = pdp_packet_data  (x->x_packet0);
    void  *data1   = pdp_packet_data  (x->x_packet1);
    void  *data2   = pdp_packet_data  (x->x_packet2);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    h = h + (h>>1);

    pdp_imageproc_bqt_process(x->x_biquad, (short int*)data0, (short int*)data1, (short int *)data2, w, h);

    return;
}



static void pdp_bqt_process_grey(t_pdp_bq *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    t_pdp *header2 = pdp_packet_header(x->x_packet2);

    void  *data0   = pdp_packet_data  (x->x_packet0);
    void  *data1   = pdp_packet_data  (x->x_packet1);
    void  *data2   = pdp_packet_data  (x->x_packet2);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    pdp_imageproc_bqt_process(x->x_biquad, (short int*)data0, (short int*)data1, (short int *)data2, w, h);

    return;
}




static void pdp_bq_process_yv12(t_pdp_bq *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    void  *data0   = pdp_packet_data  (x->x_packet0);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    unsigned int size = w*h;
    unsigned int v_offset = size;
    unsigned int u_offset = size + (size >> 2);

    unsigned int nbp = x->x_nbpasses;

    short int * idata = (short int *)data0;

    pdp_imageproc_bq_process(x->x_biquad, idata, w, h, x->x_direction, nbp);
    pdp_imageproc_bq_process(x->x_biquad, idata + v_offset, w>>1, h>>1, x->x_direction, nbp);
    pdp_imageproc_bq_process(x->x_biquad, idata + u_offset, w>>1, h>>1, x->x_direction, nbp);

    return;
}
    

static void pdp_bq_process_grey(t_pdp_bq *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    void  *data0   = pdp_packet_data  (x->x_packet0);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    unsigned int nbp = x->x_nbpasses;

    short int * idata = (short int *)data0;
    pdp_imageproc_bq_process(x->x_biquad, idata, w, h, x->x_direction, nbp);

    return;

}


static void pdp_bqt_process(t_pdp_bq *x)
{
    int encoding;

    /* image data packets are compatible, this is ensured in the pdp method  */
    /* when the hot packet is received */

    /* process inputs and write into active inlet */
    switch(pdp_packet_header(x->x_packet0)->info.image.encoding){
       
    case PDP_IMAGE_YV12:
	pdp_bqt_process_yv12(x);
	break;
       
    case PDP_IMAGE_GREY:
	pdp_bqt_process_grey(x);
	break;

    default:
	break;
	/* don't know the type, so dont process */
	    
    }

}


static void pdp_bq_process(t_pdp_bq *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);

    /* check data packets */

    if ((header0) && (PDP_IMAGE == header0->type)){

	/* process inputs and write into active inlet */
	switch(header0->info.image.encoding){

	case PDP_IMAGE_YV12:
	    pdp_bq_process_yv12(x);
	    break;

	case PDP_IMAGE_GREY:
	    pdp_bq_process_grey(x);
	    break;

	default:
	    break;
	    /* don't know the type, so dont process */
	    
	}
    }

}


static void pdp_bqt_reset(t_pdp_bq *x)
{
    t_pdp* header1 = pdp_packet_header(x->x_packet1);
    t_pdp* header2 = pdp_packet_header(x->x_packet2);
    void *data1 = pdp_packet_data(x->x_packet1);
    void *data2 = pdp_packet_data(x->x_packet1);
    unsigned int w,h,nbpixels,count;

    if (!(header1 && header2)) return;
    if (header1->type != PDP_IMAGE) return;

    w = header1->info.image.width;
    h = header1->info.image.height;
    nbpixels = w*h;

    post("pdp_bqt: resetting state");
    switch (header1->info.image.encoding){
    case PDP_IMAGE_YV12:
	count = (nbpixels + nbpixels >> 1) << 1;
	bzero(data1, count);
	bzero(data2, count);
	break;
    case PDP_IMAGE_GREY:
	count = (nbpixels) << 1;
	bzero(data1, count);
	bzero(data2, count);
	break;
    default:
	break;
    }

}



/************************* INPUT HANDLERS ****************************/

static void pdp_bq_sendpacket(t_pdp_bq *x)
{

    /* output state packets for those that are interested */
    //if(x->x_packet2 != -1) outlet_pdp(x->x_outlet2, x->x_packet2);
    //if(x->x_packet1 != -1) outlet_pdp(x->x_outlet1, x->x_packet1);

    /* unregister and propagate if valid packet */
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_bq_input_0(t_pdp_bq *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;
    int passes, i;

    if (s== gensym("register_rw"))  x->x_dropped = pdp_packet_copy_rw_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){

	/* add the process method and callback to the process queue */
	pdp_queue_add(x, pdp_bq_process, pdp_bq_sendpacket, &x->x_queue_id);
    }

}



static void pdp_bqt_input_0(t_pdp_bq *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;
    int passes, i;

    if (s== gensym("register_rw"))  x->x_dropped = pdp_packet_copy_rw_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){

	/* check if state packets are compatible */
	if (!( (pdp_type_compat_image(x->x_packet0, x->x_packet1))
	     && (pdp_packet_header(x->x_packet0)->info.image.encoding 
		 == pdp_packet_header(x->x_packet1)->info.image.encoding))){


	    /* if not, create new state packets by copying the input packets */
	    post("pdp_bqt: created new state packets");
	    pdp_packet_mark_unused(x->x_packet1);
	    pdp_packet_mark_unused(x->x_packet2);
	    x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);
	    x->x_packet2 = pdp_packet_clone_rw(x->x_packet0);

	    /* reset */
	    if (x->x_reset_on_formatchange) pdp_bqt_reset(x);
	}

	/* add the process method and callback to the process queue */
	pdp_queue_add(x, pdp_bqt_process, pdp_bq_sendpacket, &x->x_queue_id);
    }

}



/************************* CONFIG METHODS ****************************/


static void pdp_bq_passes(t_pdp_bq *x, t_floatarg f)
{
    int passes = (int)f;
    passes = passes < 0 ? 0 : passes;
    x->x_nbpasses = passes;

}

static void pdp_bq_lr(t_pdp_bq *x, t_floatarg f)
{
    if (f == 1.0f) x->x_direction |= PDP_IMAGEPROC_BIQUAD_LEFT2RIGHT;
    if (f == 0.0f) x->x_direction &= ~PDP_IMAGEPROC_BIQUAD_LEFT2RIGHT;
}

static void pdp_bq_rl(t_pdp_bq *x, t_floatarg f)
{
    if (f == 1.0f) x->x_direction |= PDP_IMAGEPROC_BIQUAD_RIGHT2LEFT;
    if (f == 0.0f) x->x_direction &= ~PDP_IMAGEPROC_BIQUAD_RIGHT2LEFT;
}
static void pdp_bq_tb(t_pdp_bq *x, t_floatarg f)
{
    if (f == 1.0f) x->x_direction |= PDP_IMAGEPROC_BIQUAD_TOP2BOTTOM;
    if (f == 0.0f) x->x_direction &= ~PDP_IMAGEPROC_BIQUAD_TOP2BOTTOM;
}

static void pdp_bq_bt(t_pdp_bq *x, t_floatarg f)
{
    if (f == 1.0f) x->x_direction |= PDP_IMAGEPROC_BIQUAD_BOTTOM2TOP;
    if (f == 0.0f) x->x_direction &= ~PDP_IMAGEPROC_BIQUAD_BOTTOM2TOP;
}


static void pdp_bq_hor(t_pdp_bq *x, t_floatarg f)
{
    pdp_bq_lr(x, f);
    pdp_bq_rl(x, f);
}
static void pdp_bq_ver(t_pdp_bq *x, t_floatarg f)
{
    pdp_bq_tb(x, f);
    pdp_bq_bt(x, f);
}



/************************* DES/CONSTRUCTORS ****************************/

static void pdp_bq_free(t_pdp_bq *x)
{
    pdp_queue_finish(x->x_queue_id);
    pdp_imageproc_bq_delete(x->x_biquad);
    pdp_packet_mark_unused(x->x_packet0);
    pdp_packet_mark_unused(x->x_packet1);
    pdp_packet_mark_unused(x->x_packet2);

}


void pdp_bq_init(t_pdp_bq *x)
{


    x->x_packet0 = -1;
    x->x_packet1 = -1;
    x->x_packet2 = -1;

    x->x_queue_id = -1;

    x->x_nbpasses = 1;
    x->x_reset_on_formatchange = true;

    x->x_biquad = pdp_imageproc_bq_new();


    pdp_bq_setstate(x, 0.0f, 0.0f);
    pdp_bq_onep(x, 0.1f);

}



/* class pointers */

t_class *pdp_bq_class;   /* biquad spacial processing */
t_class *pdp_bqt_class;  /* biquad time processing */

void *pdp_bq_new(void)
{
    t_pdp_bq *x = (t_pdp_bq *)pd_new(pdp_bq_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("passes"));
    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    pdp_bq_init(x);
    return (void *)x;
}

void *pdp_bqt_new(void)
{
    t_pdp_bq *x = (t_pdp_bq *)pd_new(pdp_bqt_class);
    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    //x->x_outlet1 = outlet_new(&x->x_obj, &s_anything); 
    //x->x_outlet2 = outlet_new(&x->x_obj, &s_anything); 
    pdp_bq_init(x);
    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif






/************************* CLASS CONSTRUCTORS ****************************/


void pdp_bq_setup(void)
{

    /* setup spatial processing class */
    
    pdp_bq_class = class_new(gensym("pdp_bq"), (t_newmethod)pdp_bq_new,
    	(t_method)pdp_bq_free, sizeof(t_pdp_bq), 0, A_NULL);


    class_addmethod(pdp_bq_class, (t_method)pdp_bq_passes, gensym("passes"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_hor, gensym("hor"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_ver, gensym("ver"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_tb, gensym("tb"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_bt, gensym("bt"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_lr, gensym("lr"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_rl, gensym("rl"),  A_FLOAT, A_NULL);   



    /* raw coefficient methods */
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_a1, gensym("a1"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_a2, gensym("a2"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_b0, gensym("b0"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_b1, gensym("b1"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_b2, gensym("b2"),  A_FLOAT, A_NULL);   
    //class_addmethod(pdp_bq_class, (t_method)pdp_bq_u1, gensym("u1"),  A_FLOAT, A_NULL);   
    //class_addmethod(pdp_bq_class, (t_method)pdp_bq_u2, gensym("u2"),  A_FLOAT, A_NULL);

    /* real pole filters */
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_onep, gensym("onep"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_twop, gensym("twop"),  A_FLOAT, A_NULL);   

    /* resonnant pole filters */
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_lpf, gensym("lpf"),  A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_hpf, gensym("hpf"),  A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_apf, gensym("apf"),  A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_bq_class, (t_method)pdp_bq_bsf, gensym("bsf"),  A_FLOAT, A_FLOAT, A_NULL);


    class_addmethod(pdp_bq_class, (t_method)pdp_bq_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);







    /* setup time processing class */
    pdp_bqt_class = class_new(gensym("pdp_bqt"), (t_newmethod)pdp_bqt_new,
    	(t_method)pdp_bq_free, sizeof(t_pdp_bq), 0, A_NULL);


    /* raw coefficient methods */
    class_addmethod(pdp_bqt_class, (t_method)pdp_bq_a1, gensym("a1"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bqt_class, (t_method)pdp_bq_a2, gensym("a2"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bqt_class, (t_method)pdp_bq_b0, gensym("b0"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bqt_class, (t_method)pdp_bq_b1, gensym("b1"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bqt_class, (t_method)pdp_bq_b2, gensym("b2"),  A_FLOAT, A_NULL);   
    //class_addmethod(pdp_bqt_class, (t_method)pdp_bq_u1, gensym("u1"),  A_FLOAT, A_NULL);   
    //class_addmethod(pdp_bqt_class, (t_method)pdp_bq_u2, gensym("u2"),  A_FLOAT, A_NULL);

    /* real pole filters */
    class_addmethod(pdp_bqt_class, (t_method)pdp_bq_onep, gensym("onep"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_bqt_class, (t_method)pdp_bq_twop, gensym("twop"),  A_FLOAT, A_NULL);   

    /* resonnant pole filters */
    class_addmethod(pdp_bqt_class, (t_method)pdp_bq_lpf, gensym("lpf"),  A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_bqt_class, (t_method)pdp_bq_hpf, gensym("hpf"),  A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_bqt_class, (t_method)pdp_bq_apf, gensym("apf"),  A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_bqt_class, (t_method)pdp_bq_bsf, gensym("bsf"),  A_FLOAT, A_FLOAT, A_NULL);

    /* control */
    class_addmethod(pdp_bqt_class, (t_method)pdp_bqt_reset, gensym("reset"), A_NULL);
    


    /* pdp */
    class_addmethod(pdp_bqt_class, (t_method)pdp_bqt_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);






}

#ifdef __cplusplus
}
#endif
