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

#include "pdp_config.h"
#include "pdp.h"
#include <stdio.h>

/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif

/* module setup declarations (all C-style) */

/* pdp system / internal stuff */
void pdp_list_setup(void);
void pdp_sym_setup(void);
void pdp_forth_setup(void);
void pdp_type_setup(void);
void pdp_packet_setup(void);
void pdp_ut_setup(void);
void pdp_queue_setup(void);
void pdp_control_setup(void);
void pdp_image_setup(void);
void pdp_bitmap_setup(void);
void pdp_matrix_setup(void);


/* pdp modules */
void pdp_xv_setup(void);
void pdp_add_setup(void);
void pdp_mul_setup(void);
void pdp_mix_setup(void);
void pdp_randmix_setup(void);
void pdp_qt_setup(void);
void pdp_v4l_setup(void);
void pdp_reg_setup(void);
void pdp_conv_setup(void);
void pdp_bq_setup(void);
void pdp_del_setup(void);
void pdp_snap_setup(void);
void pdp_trigger_setup(void);
void pdp_route_setup(void);
void pdp_noise_setup(void);
void pdp_gain_setup(void);
void pdp_chrot_setup(void);
void pdp_scope_setup(void);
void pdp_scale_setup(void);
void pdp_zoom_setup(void);
void pdp_scan_setup(void);
void pdp_scanxy_setup(void);
void pdp_sdl_setup(void);
void pdp_cheby_setup(void);
void pdp_grey2mask_setup(void);
void pdp_constant_setup(void);
void pdp_logic_setup(void);
void pdp_glx_setup(void);
void pdp_loop_setup(void);
void pdp_description_setup(void);
void pdp_convert_setup(void);
void pdp_stateless_setup(void);
void pdp_mat_mul_setup(void);
void pdp_mat_lu_setup(void);
void pdp_mat_vec_setup(void);
void pdp_plasma_setup(void);
void pdp_cog_setup(void);
void pdp_histo_setup(void);
void pdp_array_setup(void);

/* hacks */
void pdp_inspect_setup(void);
void pdp_slice_cut_setup(void);
void pdp_slice_glue_setup(void);

/* testing */
void pdp_dpd_test_setup(void);
void pdp_forthproc_setup(void);



/* library setup routine */
void pdp_setup(void){
    
    /* babble */
    post ("PDP: pure data packet");

#ifdef PDP_VERSION	
    fprintf(stderr, "PDP: version " PDP_VERSION "\n");
#endif


    /* setup pdp system */
    pdp_list_setup();
    pdp_type_setup();
    pdp_sym_setup();
    pdp_packet_setup();
    pdp_forth_setup();
    pdp_control_setup();

    pdp_image_setup();
    pdp_bitmap_setup();
    pdp_matrix_setup();

    pdp_queue_setup();

    /* setup utility toolkit */
    pdp_ut_setup();

    /* setup pdp modules*/
    pdp_add_setup();
    pdp_mul_setup();
    pdp_mix_setup();
    pdp_randmix_setup();
    pdp_reg_setup();
    pdp_conv_setup();
    pdp_bq_setup();
    pdp_del_setup();
    pdp_snap_setup();
    pdp_trigger_setup();
    pdp_route_setup();
    pdp_noise_setup();
    pdp_plasma_setup();
    pdp_gain_setup();
    pdp_chrot_setup();
    pdp_scope_setup();
    pdp_scale_setup();
    pdp_zoom_setup();
    pdp_scan_setup();
    pdp_scanxy_setup();
    pdp_cheby_setup();
    pdp_grey2mask_setup();
    pdp_constant_setup();
    pdp_logic_setup();
    pdp_loop_setup();
    pdp_description_setup();
    pdp_convert_setup();
    pdp_stateless_setup();
    pdp_mat_mul_setup();
    pdp_mat_lu_setup();
    pdp_mat_vec_setup();
    pdp_cog_setup();
    pdp_histo_setup();
    pdp_array_setup();

    /* experimental stuff */
    pdp_slice_cut_setup();
    pdp_slice_glue_setup();
    pdp_inspect_setup();

    /* testing */
    //pdp_dpd_test_setup();
    pdp_forthproc_setup();

    /* optional modules */

#ifdef HAVE_PDP_QT
    pdp_qt_setup();
#endif

#ifdef HAVE_PDP_XV
    pdp_xv_setup();
#endif

#ifdef HAVE_PDP_SDL
    pdp_sdl_setup();
#endif

#ifdef HAVE_PDP_V4L
    pdp_v4l_setup();
#endif

#ifdef HAVE_PDP_GLX
    pdp_glx_setup();
#endif

}

#ifdef __cplusplus
}
#endif
