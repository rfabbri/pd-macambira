#include <stdio.h>
#include  "pdp.h"


/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif

    void pdp_intrusion_setup(void);
    void pdp_simura_setup(void);
    void pdp_underwatch_setup(void);
    void pdp_vertigo_setup(void);
    void pdp_yvu2rgb_setup(void);
    void pdp_yqt_setup(void);
    void pdp_fqt_setup(void);
    void pdp_fcqt_setup(void);
    void pdp_lens_setup(void);
    void pdp_baltan_setup(void);
    void pdp_aging_setup(void);
    void pdp_ripple_setup(void);
    void pdp_warp_setup(void);
    void pdp_rev_setup(void);
    void pdp_mosaic_setup(void);
    void pdp_edge_setup(void);
    void pdp_spiral_setup(void);
    void pdp_radioactiv_setup(void);
    void pdp_warhol_setup(void);
    void pdp_nervous_setup(void);
    void pdp_quark_setup(void);
    void pdp_spigot_setup(void);
    void pdp_rec_tilde_setup(void);
    void pdp_o_setup(void);
    void pdp_i_setup(void);
    void pdp_mgrid_setup(void);
    void pdp_ctrack_setup(void);
    void pdp_cycle_setup(void);
    void pdp_transform_setup(void);
    void pdp_shagadelic_setup(void);
    void pdp_dice_setup(void);
    void pdp_puzzle_setup(void);
    void pdp_text_setup(void);
    void pdp_form_setup(void);
    void pdp_compose_setup(void);
    void pdp_cmap_setup(void);
    void pdp_aa_setup(void);
    void pdp_ascii_setup(void);
    void pdp_ffmpeg_tilde_setup(void);
    void pdp_live_tilde_setup(void);
    void pdp_segsnd_tilde_setup(void);
    void pdp_noquark_setup(void);
    void pdp_juxta_setup(void);
    void pdp_capture_setup(void);
    void pdp_smuck_setup(void);
    void pdp_lumafilt_setup(void);
    void pdp_transition_setup(void);
    void pdp_imgloader_setup(void);
    void pdp_imgsaver_setup(void);
    void pdp_cache_setup(void);
    void pdp_canvas_setup(void);
    void pdp_pen_setup(void);
    void pdp_shape_setup(void);


/* library setup routine */
void pidip_setup(void){
    
    post ("PiDiP : additional video processing objects for PDP : version " PDP_PIDIP_VERSION " ( ydegoyon@free.fr )\n");

    pdp_intrusion_setup();
    pdp_yqt_setup();
    pdp_fqt_setup();
    pdp_fcqt_setup();
    pdp_simura_setup();
    pdp_underwatch_setup();
    pdp_vertigo_setup();
    pdp_yvu2rgb_setup();
    pdp_lens_setup();
    pdp_baltan_setup();
    pdp_aging_setup();
    pdp_ripple_setup();
    pdp_warp_setup();
    pdp_rev_setup();
    pdp_mosaic_setup();
    pdp_edge_setup();
    pdp_spiral_setup();
    pdp_radioactiv_setup();
    pdp_warhol_setup();
    pdp_nervous_setup();
    pdp_quark_setup();
    pdp_spigot_setup();
    pdp_rec_tilde_setup();
    pdp_o_setup();
    pdp_i_setup();
    pdp_mgrid_setup();
    pdp_ctrack_setup();
    pdp_cycle_setup();
    pdp_transform_setup();
    pdp_shagadelic_setup();
    pdp_dice_setup();
    pdp_puzzle_setup();
    pdp_text_setup();
    pdp_form_setup();
    pdp_compose_setup();
    pdp_cmap_setup();
    pdp_aa_setup();
    pdp_ascii_setup();
    pdp_ffmpeg_tilde_setup();
    pdp_live_tilde_setup();
    pdp_segsnd_tilde_setup();
    pdp_noquark_setup();
    pdp_juxta_setup();
    pdp_capture_setup();
    pdp_smuck_setup();
    pdp_lumafilt_setup();
    pdp_transition_setup();
    pdp_imgloader_setup();
    pdp_imgsaver_setup();
    pdp_cache_setup();
    pdp_canvas_setup();
    pdp_pen_setup();
    pdp_shape_setup();

}

#ifdef __cplusplus
}
#endif
