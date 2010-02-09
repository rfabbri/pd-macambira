#ifndef VERSION
#define VERSION "0.03"
#endif

#include <m_pd.h>


typedef struct _sigpack
{
     t_object x_obj;
} t_sigpack;

static t_class* sigpack_class;

void chop_tilde_setup();
void decimate_tilde_setup();
void diode_tilde_setup();
void foldback_tilde_setup();
void foldover_tilde_setup();
void freqdiv_tilde_setup();
void freqshift_tilde_setup();
void harmgen_tilde_setup();
void rectify_tilde_setup();
void round_tilde_setup();
void saturate_tilde_setup();
void sieve_tilde_setup();
void split_tilde_setup();
void ustep_tilde_setup();
void vowel_tilde_setup();

static void* sigpack_new(t_symbol* s) {
    t_sigpack *x = (t_sigpack *)pd_new(sigpack_class);
    return (x);
}

void sigpack_setup(void) 
{
    sigpack_class = class_new(gensym("sIgpAck"), (t_newmethod)sigpack_new, 0,
    	sizeof(t_sigpack), 0,0);

	 chop_tilde_setup();
	 decimate_tilde_setup();
	 diode_tilde_setup();
	 foldback_tilde_setup();
	 foldover_tilde_setup();
	 freqdiv_tilde_setup();
	 freqshift_tilde_setup();
	 harmgen_tilde_setup();
	 rectify_tilde_setup();
	 round_tilde_setup();
	 saturate_tilde_setup();
	 sieve_tilde_setup();
	 split_tilde_setup();
	 ustep_tilde_setup();
	 vowel_tilde_setup();
     
     post("sIgpAck"" "VERSION " weiss www.weiss-archiv.de");
}
