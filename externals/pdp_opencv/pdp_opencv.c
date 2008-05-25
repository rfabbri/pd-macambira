/*
 *   Pure Data Packet system implementation: setup code
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

#include <stdio.h>
#include <m_pd.h>

static int pdp_opencv_initialized = 0;

/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif

void pdp_opencv_edge_setup(void);
void pdp_opencv_threshold_setup(void);
void pdp_opencv_bgsubstract_setup(void);
void pdp_opencv_distrans_setup(void);
void pdp_opencv_laplace_setup(void);
void pdp_opencv_motempl_setup(void);
void pdp_opencv_morphology_setup(void);
void pdp_opencv_haarcascade_setup(void);
void pdp_opencv_floodfill_setup(void);
void pdp_opencv_contours_convexity_setup(void);
void pdp_opencv_contours_boundingrect_setup(void);
void pdp_opencv_lk_setup(void);


/* library setup routine */
void pdp_opencv_setup(void){
    
    if (pdp_opencv_initialized) return;

    /* babble */
#ifdef PDP_CV_VERSION	
    post("PDP_opencv: pure data packet openCV wrapper version " PDP_CV_VERSION );
#else
    post ("PDP_opencv: pure data packet openCV wrapper");
#endif


    /* setup pdp_opencv system */
    pdp_opencv_edge_setup();
    pdp_opencv_threshold_setup();
    pdp_opencv_bgsubstract_setup();
    pdp_opencv_distrans_setup();
    pdp_opencv_laplace_setup();
    pdp_opencv_motempl_setup();
    pdp_opencv_morphology_setup();
    pdp_opencv_haarcascade_setup();
    pdp_opencv_floodfill_setup();
    pdp_opencv_contours_convexity_setup();
    pdp_opencv_contours_boundingrect_setup();
    pdp_opencv_lk_setup();


    pdp_opencv_initialized++;


}

#ifdef __cplusplus
}
#endif
