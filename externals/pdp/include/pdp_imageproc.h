
/*
 *   Pure Data Packet. Header file for image processing routines (used in modules).
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


#ifndef PDP_IMAGEPROC_H
#define PDP_IMAGEPROC_H

/*
#ifdef __cplusplus
extern "C"
{
#endif
*/

/****************************** 16 bit signed (pixel) routines ***************************************/

#include "pdp_types.h"
//typedef unsigned long u32;
//typedef unsigned long long u64;
//typedef short s16;
//typedef long  s32;

// mix 2 images
void *pdp_imageproc_mix_new(void);
void pdp_imageproc_mix_delete(void *x);
void pdp_imageproc_mix_setleftgain(void *x, float gain);
void pdp_imageproc_mix_setrightgain(void *x, float gain);
void pdp_imageproc_mix_process(void *x, s16 *image, s16 *image2, u32 width, u32 height);

// random mix 2 images
// note: random number generator can be platform specific
// however, it should be seeded. (same seed produces the same result)
// threshold = 0 -> left image
// threshold = 1 -> right image

void *pdp_imageproc_randmix_new(void);
void pdp_imageproc_randmix_delete(void *x);
void pdp_imageproc_randmix_setthreshold(void *x, float threshold);
void pdp_imageproc_randmix_setseed(void *x, float seed);
void pdp_imageproc_randmix_process(void *x, s16 *image, s16 *image2, u32 width, u32 height);


// produce a random image
// note: random number generator can be platform specific
// however, it should be seeded. (same seed produces the same result)
void *pdp_imageproc_random_new(void);
void pdp_imageproc_random_delete(void *x);
void pdp_imageproc_random_setseed(void *x, float seed);
void pdp_imageproc_random_process(void *x, s16 *image, u32 width, u32 height);


// apply a gain to an image
void *pdp_imageproc_gain_new(void);
void pdp_imageproc_gain_delete(void *x);
void pdp_imageproc_gain_setgain(void *x, float gain);
void pdp_imageproc_gain_process(void *x, s16 *image, u32 width, u32 height);



// add two images
void pdp_imageproc_add_process(s16 *image, s16 *image2, u32 width, u32 height);

// mul two images
void pdp_imageproc_mul_process(s16 *image, s16 *image2, u32 width, u32 height);


// affine transformation (applies gain + adds offset)
void *pdp_imageproc_affine_new(void);
void pdp_imageproc_affine_delete(void *x);
void pdp_imageproc_affine_setgain(void *x, float gain);
void pdp_imageproc_affine_setoffset(void *x, float offset);
void pdp_imageproc_affine_process(void *x, s16 *image, u32 width, u32 height);

// 3x1 or 1x3 in place convolution
// orientation
#define PDP_IMAGEPROC_CONV_HORIZONTAL 0
#define PDP_IMAGEPROC_CONV_VERTICAL   1
void *pdp_imageproc_conv_new(void);
void pdp_imageproc_conv_delete(void *x);
void pdp_imageproc_conv_setmin1(void *x, float val);
void pdp_imageproc_conv_setzero(void *x, float val);
void pdp_imageproc_conv_setplus1(void *x, float val);
void pdp_imageproc_conv_setbordercolor(void *x, float intensity);
void pdp_imageproc_conv_process(void *x, s16 *image, u32 width, u32 height, u32 orientation, u32 nbpasses);


// colour rotation for 2 colour planes
// matrix is column encoded
void *pdp_imageproc_crot2d_new(void);
void pdp_imageproc_crot2d_delete(void *x);
void pdp_imageproc_crot2d_setmatrix(void *x, float *matrix);
void pdp_imageproc_crot2d_process(void *x, s16 *image, u32 width, u32 height);

// colour rotation for 3 colour planes
void *pdp_imageproc_crot3d_new(void);
void pdp_imageproc_crot3d_delete(void *x);
void pdp_imageproc_crot3d_setmatrix(void *x, float *matrix);
void pdp_imageproc_crot3d_process(void *x, s16 *image, u32 width, u32 height);




// biquad space

// directions
#define PDP_IMAGEPROC_BIQUAD_TOP2BOTTOM (1<<0)
#define PDP_IMAGEPROC_BIQUAD_BOTTOM2TOP (1<<1)
#define PDP_IMAGEPROC_BIQUAD_LEFT2RIGHT (1<<2)
#define PDP_IMAGEPROC_BIQUAD_RIGHT2LEFT (1<<3)
void *pdp_imageproc_bq_new(void);
void pdp_imageproc_bq_delete(void *x);
void pdp_imageproc_bq_setcoef(void *x, float *coef); // a0,a1,a2,b0,b1,b2
void pdp_imageproc_bq_process(void *x, s16 *image, u32 width, u32 height, u32 direction, u32 nbpasses);


// biquad time
void *pdp_imageproc_bqt_new(void);
void pdp_imageproc_bqt_delete(void *x);
void pdp_imageproc_bqt_setcoef(void *x, float *coef); // a0,a1,a2,b0,b1,b2
void pdp_imageproc_bqt_process(void *x, s16 *image, s16 *state0, s16 *state1, u32 width, u32 height);



/*
#ifdef __cplusplus
}
#endif
*/

#endif //PDP_IMAGEPROC_H
