/*
 *   Pure Data Packet. c wrapper for mmx image processing routines.
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


/* this is a c wrapper around platform specific (mmx) code */
#include <stdlib.h>
#include "pdp_mmx.h"
#include "pdp_imageproc.h"

// utility stuff
inline static s16 float2fixed(float f)
{
    if (f > 1) f = 1;
    if (f < -1) f = -1;
    f *= 0x7fff;
    return (s16)f;
}

inline static void setvec(s16 *v, float f)
{
    s16 a = float2fixed(f);
    v[0] = a;
    v[1] = a;
    v[2] = a;
    v[3] = a;
}



// add two images
void pdp_imageproc_add_process(s16 *image, s16 *image2,  u32 width, u32 height)
{
    unsigned int totalnbpixels = width * height;
    pixel_add_s16(image, image2, totalnbpixels>>2);
}

// mul two images
void pdp_imageproc_mul_process(s16 *image, s16 *image2,  u32 width, u32 height)
{
    unsigned int totalnbpixels = width * height;
    pixel_mul_s16(image, image2, totalnbpixels>>2);
}

// mix 2 images
void *pdp_imageproc_mix_new(void){return malloc(8*sizeof(s16));}
void pdp_imageproc_mix_delete(void *x) {free (x);}
void pdp_imageproc_mix_setleftgain(void *x, float gain){setvec((s16 *)x, gain);}
void pdp_imageproc_mix_setrightgain(void *x, float gain){setvec((s16 *)x + 4, gain);}
void pdp_imageproc_mix_process(void *x, s16 *image, s16 *image2, u32 width, u32 height)
{
    s16 *d = (s16 *)x;
    unsigned int totalnbpixels = width * height;
    pixel_mix_s16(image, image2, totalnbpixels>>2, d, d+4);
}


// random mix 2 images
void *pdp_imageproc_randmix_new(void){return malloc(8*sizeof(s16));}
void pdp_imageproc_randmix_delete(void *x) {free (x);}
void pdp_imageproc_randmix_setthreshold(void *x, float threshold){setvec((s16 *)x, 2*threshold-1);}
void pdp_imageproc_randmix_setseed(void *x, float seed)
{
    s16 *d = (s16 *)x;
    srandom((u32)seed);
    d[4] = (s16)random();
    d[5] = (s16)random();
    d[6] = (s16)random();
    d[7] = (s16)random();
    
}
void pdp_imageproc_randmix_process(void *x, s16 *image, s16 *image2, u32 width, u32 height)
{
    s16 *d = (s16 *)x;
    unsigned int totalnbpixels = width * height;
    pixel_randmix_s16(image, image2, totalnbpixels>>2, d+4, d);
}

// affine transformation (applies gain + adds offset)
void *pdp_imageproc_affine_new(void){return malloc(8*sizeof(s16));}
void pdp_imageproc_affine_delete(void *x){free(x);}
void pdp_imageproc_affine_setgain(void *x, float gain){setvec((s16 *)x, gain);}
void pdp_imageproc_affine_setoffset(void *x, float offset){setvec((s16 *)x+4, offset);}
void pdp_imageproc_affine_process(void *x, s16 *image, u32 width, u32 height)
{
    s16 *d = (s16 *)x;
    pixel_affine_s16(image, (width*height)>>2, d, d+4);
}

// 3x1 or 1x3 in place convolution
// orientation
void *pdp_imageproc_conv_new(void){return(malloc(16*sizeof(s16)));}
void pdp_imageproc_conv_delete(void *x){free(x);}
void pdp_imageproc_conv_setmin1(void *x, float val){setvec((s16 *)x, val);}
void pdp_imageproc_conv_setzero(void *x, float val){setvec((s16 *)x+4, val);}
void pdp_imageproc_conv_setplus1(void *x, float val){setvec((s16 *)x+8, val);}
void pdp_imageproc_conv_setbordercolor(void *x, float val){setvec((s16 *)x+12, val);}
void pdp_imageproc_conv_process(void *x, s16 *image, u32 width, u32 height, u32 orientation, u32 nbp)
{
    s16 *d = (s16 *)x;
    u32 i,j;

    if (orientation == PDP_IMAGEPROC_CONV_HORIZONTAL)
    {
	for(i=0; i<width*height; i+=width)
	    for (j=0; j<nbp; j++)
		pixel_conv_hor_s16(image+i, width>>2, d+12, d);
    }

    else
    {
	for (j=0; j<nbp; j++)
	    for(i=0; i<width; i +=4) pixel_conv_ver_s16(image+i,  height, width, d+12, d);
    }

	
	
}

// apply a gain to an image
void *pdp_imageproc_gain_new(void){return(malloc(8*sizeof(s16)));}
void pdp_imageproc_gain_delete(void *x){free(x);}
void pdp_imageproc_gain_setgain(void *x, float gain)
{
    /* convert float to s16 + shift */
    s16 *d = (s16 *)x;
    s16 g;
    int i;
    float sign;
    int shift = 0;
    
    sign = (gain < 0) ? -1 : 1;
    gain *= sign;

    /* max shift = 16 */
    for(i=0; i<=16; i++){
	if (gain < 0x4000){
	    gain *= 2;
	    shift++;
	}
	else break;
    }

    gain *= sign;
    g = (s16) gain;

    //g = 0x4000;
    //shift = 14;

    d[0]=g;
    d[1]=g;
    d[2]=g;
    d[3]=g;
    d[4]=(s16)shift;
    d[5]=0;
    d[6]=0;
    d[7]=0;
}
void pdp_imageproc_gain_process(void *x, s16 *image, u32 width, u32 height)
{
    s16 *d = (s16 *)x;
    pixel_gain_s16(image, (width*height)>>2, d, (u64 *)(d+4));
}

// colour rotation for 2 colour planes
void *pdp_imageproc_crot2d_new(void){return malloc(16*sizeof(s16));}
void pdp_imageproc_crot2d_delete(void *x){free(x);}
void pdp_imageproc_crot2d_setmatrix(void *x, float *matrix)
{
    s16 *d = (s16 *)x;
    setvec(d, matrix[0]);
    setvec(d+4, matrix[1]);
    setvec(d+8, matrix[2]);
    setvec(d+12, matrix[3]);
}
void pdp_imageproc_crot2d_process(void *x, s16 *image, u32 width, u32 height)
{
    s16 *d = (s16 *)x;
    pixel_crot2d_s16(image, width*height >> 2, d);
}

// biquad and biquad time
void *pdp_imageproc_bq_new(void){return malloc((5+2+2)*4*sizeof(s16));}//5xcoef, 2xstate, 2xsavestate
void pdp_imageproc_bq_delete(void *x){free(x);}
void pdp_imageproc_bq_setcoef(void *x, float *coef) // a0,-a1,-a2,b0,b1,b2,u0,u1
{
    s16 *d = (s16 *)x;
    float ia0 = 1.0f / coef[0];

    /* all coefs are s1.14 fixed point */
    /* representing values -2 < x < 2  */
    /* so scale down before using the ordinary s0.15 float->fixed routine */

    ia0 *= 0.5f;

    // coef
    setvec(d, ia0*coef[1]);
    setvec(d+4, ia0*coef[2]);
    setvec(d+8, ia0*coef[3]);
    setvec(d+12, ia0*coef[4]);
    setvec(d+16, ia0*coef[5]);

    // state to reset too
    setvec(d+28, coef[6]);
    setvec(d+32, coef[7]);

}
void pdp_imageproc_bqt_process(void *x, s16 *image, s16 *state0, s16 *state1, u32 width, u32 height)
{
    s16 *d = (s16 *)x;
    pixel_biquad_time_s16(image, state0, state1, d, (width*height)>>2);
}

void pdp_imageproc_bq_process(void *x, s16 *image, u32 width, u32 height, u32 direction, u32 nbp)
{
    s16 *d = (s16 *)x;
    unsigned int i,j;



    /* VERTICAL */

    if ((direction & PDP_IMAGEPROC_BIQUAD_TOP2BOTTOM)
	&& (direction &  PDP_IMAGEPROC_BIQUAD_BOTTOM2TOP)){

	for(i=0; i<width; i +=4){
	    for (j=0; j<nbp; j++){
		pixel_biquad_vertb_s16(image+i,    height>>2, width, d, d + (5*4));
		pixel_biquad_verbt_s16(image+i,    height>>2, width, d, d + (5*4));
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_TOP2BOTTOM){
	for(i=0; i<width; i +=4){
	    for (j=0; j<nbp; j++){
		pixel_biquad_vertb_s16(image+i,    height>>2, width, d, d + (5*4));
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_BOTTOM2TOP){
	for(i=0; i<width; i +=4){
	    for (j=0; j<nbp; j++){
		pixel_biquad_verbt_s16(image+i,    height>>2, width, d, d + (5*4));
	    }
	}
    }

    /* HORIZONTAL */

    if ((direction & PDP_IMAGEPROC_BIQUAD_LEFT2RIGHT)
	&& (direction & PDP_IMAGEPROC_BIQUAD_RIGHT2LEFT)){

	for(i=0; i<(width*height); i +=(width<<2)){
	    for (j=0; j<nbp; j++){
		pixel_biquad_horlr_s16(image+i,    width>>2, width, d, d + (5*4));
		pixel_biquad_horrl_s16(image+i,    width>>2, width, d, d + (5*4));
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_LEFT2RIGHT){
	for(i=0; i<(width*height); i +=(width<<2)){
	    for (j=0; j<nbp; j++){
		pixel_biquad_horlr_s16(image+i,    width>>2, width, d, d + (5*4));
	    }
	}
    }

    else if (direction & PDP_IMAGEPROC_BIQUAD_RIGHT2LEFT){
	for(i=0; i<(width*height); i +=(width<<2)){
	    for (j=0; j<nbp; j++){
		pixel_biquad_horrl_s16(image+i,    width>>2, width, d, d + (5*4));
	    }
	}
    }

}

// produce a random image
// note: random number generator can be platform specific
// however, it should be seeded. (same seed produces the same result)
void *pdp_imageproc_random_new(void){return malloc(4*sizeof(s16));}
void pdp_imageproc_random_delete(void *x){free(x);}
void pdp_imageproc_random_setseed(void *x, float seed)
{
    s16 *d = (s16 *)x;
    srandom((u32)seed);
    d[0] = (s16)random();
    d[1] = (s16)random();
    d[2] = (s16)random();
    d[3] = (s16)random();
    
}
void pdp_imageproc_random_process(void *x, s16 *image, u32 width, u32 height)
{
    s16 *d = (s16 *)x;
    unsigned int totalnbpixels = width * height;
    pixel_rand_s16(image, totalnbpixels>>2, d);
}


