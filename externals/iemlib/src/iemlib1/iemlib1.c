/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2003 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include "m_pd.h"
#include "iemlib.h"

static t_class *iemlib1_class;

static void *iemlib1_new(void)
{
	t_object *x = (t_object *)pd_new(iemlib1_class);

	return (x);
}

void biquad_freq_resp_setup(void);
void db2v_setup(void);
void f2note_setup(void);
void forpp_setup(void);
void gate_setup(void);
void sigfilter_setup(void);
void sigFIR_setup(void);
void sighml_shelf_setup(void);
void sigiem_cot4_setup(void);
void sigiem_delay_setup(void);
void sigiem_sqrt4_setup(void);
void sigiem_pow4_setup(void);
void siglp1_t_setup(void);
void sigmov_avrg_kern_setup(void);
void sigpara_bp2_setup(void);
void sigpeakenv_setup(void);
void sigprvu_setup(void);
void sigpvu_setup(void);
void sigrvu_setup(void);
void sigsin_phase_setup(void);
void sigvcf_filter_setup(void);
void soundfile_info_setup(void);
void split_setup(void);
void v2db_setup(void);

/* ------------------------ setup routine ------------------------- */

void iemlib1_setup(void)
{
	iemlib1_class = class_new(gensym("iemlib1"), iemlib1_new, 0,
		sizeof(t_object), CLASS_NOINLET, 0);

	biquad_freq_resp_setup();
	db2v_setup();
	f2note_setup();
	forpp_setup();
	gate_setup();
	sigfilter_setup();
	sigFIR_setup();
	sighml_shelf_setup();
	sigiem_cot4_setup();
	sigiem_delay_setup();
	sigiem_sqrt4_setup();
	sigiem_pow4_setup();
	siglp1_t_setup();
	sigmov_avrg_kern_setup();
	sigpara_bp2_setup();
	sigpeakenv_setup();
	sigprvu_setup();
	sigpvu_setup();
	sigrvu_setup();
	sigsin_phase_setup();
	sigvcf_filter_setup();
	soundfile_info_setup();
	split_setup();
	v2db_setup();

	post("iemlib1 (R-1.15) library loaded!");
}
