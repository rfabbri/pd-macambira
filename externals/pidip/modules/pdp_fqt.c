/*
 *   Pure Data Packet module.
 *   Copyright (c) by Yves Degoyon <ydegoyon@free.fr>
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
#include "pdp_llconv.h"
#include <quicktime/lqt.h>
#include <quicktime/colormodels.h>

typedef struct pdp_fqt_data
{
    short int gain[4];
} t_pdp_fqt_data;

typedef struct pdp_fqt_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;

    int packet0;
    bool initialized;

    unsigned int x_vwidth;
    unsigned int x_vheight;

    bool loop;

    unsigned char * qt_rows[3];

    unsigned char * qt_frame;
    quicktime_t *qt;
    int qt_cmodel;

    t_pdp_fqt_data *state_data;

} t_pdp_fqt;



static void pdp_fqt_close(t_pdp_fqt *x)
{
    if (x->initialized){
	quicktime_close(x->qt);
	free(x->qt_frame);
	x->initialized = false;
    }

}

static void pdp_fqt_open(t_pdp_fqt *x, t_symbol *name)
{
    unsigned int size;

    post("pdp_fqt: opening %s", name->s_name);

    pdp_fqt_close(x);

    x->qt = quicktime_open(name->s_name, 1, 0);

    if (!(x->qt)){
	post("pdp_fqt: error opening qt file");
	x->initialized = false;
	return;
    }

    if (!quicktime_has_video(x->qt)) {
	post("pdp_fqt: no video stream");
	quicktime_close(x->qt);
	x->initialized = false;
	return;
	
    }
    else if (!quicktime_supported_video(x->qt,0)) {
	post("pdp_fqt: unsupported video codec\n");
	quicktime_close(x->qt);
	x->initialized = false;
	return;
    }
    else 
    {
	x->qt_cmodel = BC_YUV420P;
	x->x_vwidth  = quicktime_video_width(x->qt,0);
	x->x_vheight = quicktime_video_height(x->qt,0);
	x->qt_frame = (unsigned char*)malloc(x->x_vwidth*x->x_vheight*4);
	size = x->x_vwidth * x->x_vheight;
	x->qt_rows[0] = &x->qt_frame[0];
	x->qt_rows[2] = &x->qt_frame[size];
	x->qt_rows[1] = &x->qt_frame[size + (size>>2)];
    
	quicktime_set_cmodel(x->qt, x->qt_cmodel);
	x->initialized = true;
	outlet_float(x->x_outlet2, (float)quicktime_video_length(x->qt,0));

    }
}


static void pdp_fqt_bang(t_pdp_fqt *x)
{
  unsigned int w, h, nbpixels, packet_size;
  int object, length, pos, i, j;
  short int* data;
  t_pdp* header;

  static short int gain[4] = {0x7fff, 0x7fff, 0x7fff, 0x7fff};

    if (!(x->initialized)){
	//post("pdp_fqt: no qt file opened");
	return;
    }

    w = x->x_vwidth;
    h = x->x_vheight;
    nbpixels = w * h;
    packet_size = (nbpixels + (nbpixels >> 1)) << 1;

    object = pdp_packet_new_image_YCrCb( x->x_vwidth, x->x_vheight );
    header = pdp_packet_header(object);
    data = (short int *) pdp_packet_data(object);

    header->info.image.encoding = PDP_IMAGE_YV12;
    header->info.image.width = w;
    header->info.image.height = h;

    length = quicktime_video_length(x->qt,0);
    pos = quicktime_video_position(x->qt,0);
    // post("pdp_fqt : video position : %d length =%d", pos, length );

    if (pos >= length){
	pos = (x->loop) ? 0 : length - 1;
        // post("pdp_fqt : setting video position to %d", pos);
	quicktime_set_video_position(x->qt, pos, 0);
    }

    lqt_decode_video(x->qt, x->qt_rows, 0);

    switch(x->qt_cmodel){
    case BC_YUV420P:
        pdp_llconv(x->qt_frame, RIF_YVU__P411_U8, data, RIF_YVU__P411_S16, x->x_vwidth, x->x_vheight);
        break;

    case BC_YUV422:
        pdp_llconv(x->qt_frame, RIF_YUYV_P____U8, data, RIF_YVU__P411_S16, x->x_vwidth, x->x_vheight);
        break;

    case BC_RGB888:
        pdp_llconv(x->qt_frame, RIF_RGB__P____U8, data, RIF_YVU__P411_S16, x->x_vwidth, x->x_vheight);
        break;

    default:
        post("pdp_fqt : error on decode: unkown colour model");
        break;
    }
    
    outlet_float(x->x_outlet1, (float)pos);
    pdp_packet_pass_if_valid(x->x_outlet0, &object);

}

static void pdp_fqt_loop(t_pdp_fqt *x, t_floatarg loop)
{
    int loopi = (int)loop;
    x->loop = !(loopi == 0);
}

static void pdp_fqt_frame_cold(t_pdp_fqt *x, t_floatarg frameindex)
{
    int frame = (int)frameindex;
    int length;


    if (!(x->initialized)) return;

    length = quicktime_video_length(x->qt,0);

    frame = (frame >= length) ? length-1 : frame;
    frame = (frame < 0) ? 0 : frame;

    // post("pdp_fqt : frame cold : setting video position to : %d", frame );
    quicktime_set_video_position(x->qt, frame, 0);
}

static void pdp_fqt_frame(t_pdp_fqt *x, t_floatarg frameindex)
{
    // pdp_fqt_frame_cold(x, frameindex);
    pdp_fqt_bang(x);
}

static void pdp_fqt_gain(t_pdp_fqt *x, t_floatarg f)
{
    int i;
    short int g;
    float bound = (float)0x7fff;

    f *= (float)0x7fff;

    f = (f>bound) ? bound : f;
    f = (f<-bound) ? -bound : f;

    g = (short int)f;

    for (i=0; i<4; i++) x->state_data->gain[i] = g;
}

static void pdp_fqt_free(t_pdp_fqt *x)
{
    free (x->state_data);
    pdp_fqt_close(x);
}

t_class *pdp_fqt_class;

void *pdp_fqt_new(void)
{
    t_pdp_fqt *x = (t_pdp_fqt *)pd_new(pdp_fqt_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("frame_cold"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("gain"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);

    x->packet0 = -1;

    x->initialized = false;

    x->loop = false;

    x->state_data = (t_pdp_fqt_data *)malloc(sizeof(t_pdp_fqt_data));
    pdp_fqt_gain(x, 1.0f);

    return (void *)x;
}

#ifdef __cplusplus
extern "C"
{
#endif


void pdp_fqt_setup(void)
{
    pdp_fqt_class = class_new(gensym("pdp_fqt"), (t_newmethod)pdp_fqt_new,
    	(t_method)pdp_fqt_free, sizeof(t_pdp_fqt), 0, A_NULL);

    class_addmethod(pdp_fqt_class, (t_method)pdp_fqt_bang, gensym("bang"), A_NULL);
    class_addmethod(pdp_fqt_class, (t_method)pdp_fqt_close, gensym("close"), A_NULL);
    class_addmethod(pdp_fqt_class, (t_method)pdp_fqt_open, gensym("open"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_fqt_class, (t_method)pdp_fqt_loop, gensym("loop"), A_DEFFLOAT, A_NULL);
    class_addfloat (pdp_fqt_class, (t_method)pdp_fqt_frame);
    class_addmethod(pdp_fqt_class, (t_method)pdp_fqt_frame_cold, gensym("frame_cold"), A_FLOAT, A_NULL);
    class_addmethod(pdp_fqt_class, (t_method)pdp_fqt_gain, gensym("gain"), A_FLOAT, A_NULL);
    class_addmethod(pdp_fqt_class, nullfn, gensym("signal"), 0);
    class_sethelpsymbol( pdp_fqt_class, gensym("pdp_fqt.pd") );

}

#ifdef __cplusplus
}
#endif
