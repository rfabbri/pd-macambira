/* (C) 2010 Federico Ferri <mescalinum@gmail.com>
 * this software is gpl'ed software, read the file "README.txt" for details
 */

#include "tms5220/tms5220.c"

#include "m_pd.h"

static t_class *tms5220_tilde_class;

typedef struct _tms5220_tilde {
	t_object x_obj;

	// status outlets
	t_int status;
	t_outlet *x_status;
	t_int ready;
	t_outlet *x_ready;
	t_int interrupt;
	t_outlet *x_interrupt;

	t_float dummy;
} t_tms5220_tilde;

void tms5220_tilde_reset(t_tms5220_tilde *x) {
	tms5220_reset();
}

void *tms5220_tilde_new(t_symbol *s, int argc, t_atom *argv) {
	t_tms5220_tilde *x = (t_tms5220_tilde *)pd_new(tms5220_tilde_class);

	x->status = x->ready = x->interrupt = 0;

	//inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	//floatinlet_new(&x->x_obj, &x->f_x);

	outlet_new(&x->x_obj, &s_signal);
	x->x_status = outlet_new(&x->x_obj, &s_float);
	x->x_ready = outlet_new(&x->x_obj, &s_float);
	x->x_interrupt = outlet_new(&x->x_obj, &s_float);

	tms5220_tilde_reset(x);

	return (void *)x;
}

void tms5220_tilde_free(t_tms5220_tilde *x) {
}

void tms5220_tilde_write(t_tms5220_tilde *x, t_floatarg data) {
	tms5220_data_write((int)data);
}

void tms5220_tilde_update_status(t_tms5220_tilde *x) {
	t_int new_status, new_ready, new_interrupt;

	new_status = tms5220_status_read();
	new_ready = tms5220_ready_read();
	new_interrupt = tms5220_int_read();

	if(new_interrupt != x->interrupt) {
		outlet_float(x->x_interrupt, new_interrupt);
		x->interrupt = new_interrupt;
	}

	if(new_ready != x->ready) {
		outlet_float(x->x_ready, new_ready);
		x->ready = new_ready;
	}

	if(new_status != x->status) {
		outlet_float(x->x_status, new_status);
		x->status = new_status;
	}
}

t_int *tms5220_tilde_perform(t_int *w) {
	t_tms5220_tilde *x = (t_tms5220_tilde *)(w[1]);
	t_sample       *in =        (t_sample *)(w[2]);
	t_sample      *out =        (t_sample *)(w[3]);
	int              n =               (int)(w[4]);

	//TODO: figure out proper resampling here:
#define RESAMPLE_FACTOR 8
	int rc = 0; // resample counter

	unsigned char *bytebuf = (unsigned char *)malloc(sizeof(unsigned char)*n/RESAMPLE_FACTOR);

	if(!bytebuf) {error("FATAL: cannot allocate signal buffer"); return w;}

	tms5220_process(bytebuf, n/RESAMPLE_FACTOR);
	unsigned char *pb = bytebuf;

	while (n--) {
		//FIXME: resampling without alias, please
		*out++ = (0.5 + ((t_sample) *pb)) / 127.5;
		if(!(rc = ((rc + 1) % RESAMPLE_FACTOR))) pb++;
	}

	free(bytebuf);

	tms5220_tilde_update_status(x);
	
	return (w + 5);
}

void tms5220_tilde_dsp(t_tms5220_tilde *x, t_signal **sp) {
	dsp_add(tms5220_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void tms5220_tilde_setup(void) {
	tms5220_tilde_class = class_new(gensym("tms5220~"),
		(t_newmethod)tms5220_tilde_new,
		(t_method)tms5220_tilde_free,
		sizeof(t_tms5220_tilde),
		CLASS_DEFAULT, A_GIMME, 0);

	CLASS_MAINSIGNALIN(tms5220_tilde_class, t_tms5220_tilde, dummy);

	class_addmethod(tms5220_tilde_class, (t_method)tms5220_tilde_dsp, gensym("dsp"), 0);
	//class_addfloat(tms5220_tilde_class, (t_method)tms5220_tilde_write);
	class_addmethod(tms5220_tilde_class, (t_method)tms5220_tilde_write, gensym("write"), A_DEFFLOAT, 0);
	class_addmethod(tms5220_tilde_class, (t_method)tms5220_tilde_reset, gensym("reset"), 0);

	post("tms5220~: TSM5220 IC emulation");
	post("tms5220~:   external by Federico Ferri <mescalinum@gmail.com>");
	post("tms5220~:   based on code by Frank Palazzolo <palazzol@tir.com>");
}
