/*
 * Readanysf  PD object for reading and playing multiple soundfile types
 * from disk and from the web using gmerlin_avdecode
 *
 * Copyright (C) 2003-9 August Black
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * readanysf.cpp
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>  //IMPORTANT bool
#include <stdlib.h>   // for malloc
#include <stdio.h>	//sprintf
#include <string.h>  //strcmp
#include <math.h>   // ceil

#include "m_pd.h"
#include "Readsf.h"

#define MAXSFCHANS 64	// got this from d_soundfile.c in pd/src


static t_class *readanysf_class;

typedef struct readanysf {
	t_object x_obj;
	t_sample *(x_outvec[MAXSFCHANS]);
	int frames ;   // this should be block size
	int num_channels;
	int num_frames_in_fifo;
	int num_samples_per_frame;
	int buffersize;
	unsigned int tick;  // how often to send outlet info
	bool play;
	t_sample *buf;
	unsigned int count;
	t_outlet *outinfo;
	Readsf *rdsf;
} t_readanysf;


void m_play(t_readanysf *x) {
	if (x->rdsf->isReady() ) {	
		x->play = true;
	} else {
		if (x->rdsf->isOpening() ) {
			post("Current file is still starting.");
			post("This probably means that it is a stream and it needs to buffer in from the network.");
		} else {
			post("Current file is either invalid or an unsupported codec.");
		}
	}
}

void readanysf_bang(t_readanysf *x) {
	m_play(x);
}


void m_pause(t_readanysf *x) {
	x->play = false;
}

void m_pcm_seek(t_readanysf *x, float f) {
	if (! x->rdsf->PCM_seek( (long)f) )		
		post("can't seek on this file.");

}	

void m_time_seek(t_readanysf *x, float f) {
	if (! x->rdsf->TIME_seek( (double)f) )		
		post("can't seek on this file.");

}	

void m_tick(t_readanysf *x, float f) {
	if (f >= 0.0) {
		x->tick = (unsigned int) f ;
	}
}	

void m_stop(t_readanysf *x) {
	x->play = false;
	x->rdsf->Rewind();
}

void m_open_callback( void * data) {
	t_atom lst;
	t_readanysf * x = (t_readanysf *)data;

	if (x->rdsf->isOpen()) {	
		SETFLOAT(&lst, (float)x->rdsf->get_input_samplerate() );
		outlet_anything(x->outinfo, gensym("samplerate"), 1, &lst);
		
		SETFLOAT(&lst, x->rdsf->getLengthInSeconds() );
		outlet_anything(x->outinfo, gensym("length"), 1, &lst);

		outlet_float(x->outinfo, 0.0);

		// ready should be last	
		SETFLOAT(&lst, 1.0 );
		outlet_anything(x->outinfo, gensym("ready"), 1, &lst);
		// set time to 0 again here just to be sure
	} else {
		SETFLOAT(&lst, 0.0 );
		outlet_anything(x->outinfo, gensym("samplerate"), 1, &lst);
		SETFLOAT(&lst, 0.0 );
		outlet_anything(x->outinfo, gensym("length"), 1, &lst);
		SETFLOAT(&lst, 0.0 );
		outlet_anything(x->outinfo, gensym("ready"), 1, &lst);
		outlet_float(x->outinfo, 0.0);
		post("Invalid file or unsupported codec.");
	}
}

void m_open(t_readanysf *x, t_symbol *s) {

	t_atom lst;
	SETFLOAT(&lst, 0.0 );
	outlet_anything(x->outinfo, gensym("ready"), 1, &lst);

	SETFLOAT(&lst, 0.0 );
	outlet_anything(x->outinfo, gensym("length"), 1, &lst);
	
	outlet_float(x->outinfo, 0.0);

	x->play = false;
	x->rdsf->Open( s->s_name);
}

void m_speed(t_readanysf *x, float f) {
	x->rdsf->setSpeed( f );
}

void m_loop(t_readanysf *x, float f) {
	if ( f == 0)
		x->rdsf->doLoop( false );
	else 
		x->rdsf->doLoop( true );
	post("looping = %d", x->rdsf->doLoop());
}


static void *readanysf_new(t_float f, t_float f2, t_float f3 ) {
  
  int nchannels = (int)f;
  int nframes = (int)f2;
  int nsamples = (int)f3;
  int i;
  t_atom lst;

  // if the external is created without any options
  if (nchannels <=0)
	  nchannels = 2;

  if (nframes <=0)
	  nframes = 24;

  if (nsamples <=0)
	  nsamples = sys_getblksize();



  t_readanysf *x = (t_readanysf *)pd_new(readanysf_class);
  x->num_channels = nchannels;
  x->num_frames_in_fifo = nframes;
  x->num_samples_per_frame = nsamples;
  x->count = 0;
  x->tick = 1000;
  x->buffersize =0;
  x->rdsf = NULL; 
  x->play =false; 
  for (i=0; i < nchannels; i++) {
  	outlet_new(&x->x_obj,  gensym("signal"));
  }
  x->outinfo = outlet_new(&x->x_obj, &s_anything);
  SETFLOAT(&lst, 0.0 );
  outlet_anything(x->outinfo, gensym("ready"), 1, &lst);
  
  // set time to 0.0
  outlet_float(x->outinfo, 0.0);
	if (x->rdsf == NULL) {
		x->rdsf = new Readsf ( (int)sys_getsr(), x->num_channels, x->num_frames_in_fifo, x->num_samples_per_frame);
		post("Created new readanysf~ with %d channels and internal buffer of %d * %d = %d", x->num_channels,
				x->num_frames_in_fifo, x->num_samples_per_frame, x->num_frames_in_fifo *  x->num_samples_per_frame);
	}
	x->rdsf->setOpenCallback( m_open_callback, (void *)x); 

  return (void *)x;
}

static t_int *readanysf_perform(t_int *w) {
	t_readanysf *x = (t_readanysf *) (w[1]);
	int i=0,j=0, k=0;
	int samples_returned = 0;
	int blocksize = x->frames; //sys_getblksize();
	t_atom lst;

	if (x->play ) {
		samples_returned = x->rdsf->Decode( (char *) x->buf, (unsigned int)blocksize);
		if (samples_returned == 0 && x->rdsf->getEOF() == true) {
			// if loop?
			m_stop(x);
			outlet_bang(x->outinfo);
		}
		for (i = 0; i < x->num_channels; i++) {
			k =0;
			for( j=0; j< samples_returned; j++) {
				x->x_outvec[i][j] = x->buf[ k + i  ];
				k = k + x->num_channels;
			}

		}
		// this could happen if 1) the file has ended or 
		// 	2) we are decoding frames faster than what can be read 
		// 	from disk or from the net
		//if (samples_returned != x->frames)
		//	printf("%d != %d\n", x->frames, samples_returned);
	} 

	for (i = 0; i < x->num_channels; i++) {
		for (j = samples_returned; j < x->frames;  j++) {
			x->x_outvec[i][j] = 0.0;
		}
	}
	if ( ++x->count > x->tick ) {
		SETFLOAT (&lst, x->rdsf->getFifoSizePercentage() );
		//post("cache: %f", x->rdsf->getFifoSizePercentage());
		outlet_anything(x->outinfo, gensym("cache"), 1, &lst);
		if (x->play) {
			outlet_float(x->outinfo, x->rdsf->getTimeInSeconds());
		}
		x->count = 0;
	}
	
	return (w+2);	
}

void readanysf_dsp(t_readanysf *x, t_signal **sp) {

  int i, tmpbufsize;

  x->frames = sp[0]->s_n;
  tmpbufsize = x->frames * x->num_channels  * sizeof(t_sample);
	//x->rdsf = new Readsf ( (int)sys_getsr(), x->num_channels, (x->frames + 1) * SRC_MAX );
  // only malloc the buffer if the frame size changes.
  if(  x->buffersize < tmpbufsize ) { 
  	x->buffersize = tmpbufsize;
  	free(x->buf);
		x->buf = (t_sample *)malloc( x->buffersize  );
  	//post("frames=%d, buffersize = %d, spf=%d", x->frames, x->buffersize, x->samples_per_frame);
  }
  for (i = 0; i < x->num_channels; i++)
	  x->x_outvec[i] = sp[i]->s_vec;
		    
  dsp_add(readanysf_perform, 1, x);
}

static void readanysf_free(t_readanysf *x) {
	// delete the readany objs
	delete x->rdsf;
	x->rdsf = NULL;
}

//extern "C" void readanysf_tilde_setup(void) {
extern "C" void readanysf_tilde_setup(void) {

  readanysf_class = class_new(gensym("readanysf~"), (t_newmethod)readanysf_new,
  	(t_method)readanysf_free, sizeof(t_readanysf), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);

  class_addmethod(readanysf_class, (t_method)readanysf_dsp, gensym("dsp"), A_NULL);
  class_addmethod(readanysf_class, (t_method)m_open, gensym("open"), A_SYMBOL, A_NULL);
  class_addmethod(readanysf_class, (t_method)m_play, gensym("play"),  A_NULL);
  class_addmethod(readanysf_class, (t_method)m_pause, gensym("pause"),  A_NULL);
  class_addmethod(readanysf_class, (t_method)m_stop, gensym("stop"),  A_NULL);
  class_addmethod(readanysf_class, (t_method)m_tick, gensym("tick"), A_FLOAT, A_NULL);
  class_addmethod(readanysf_class, (t_method)m_speed, gensym("speed"), A_FLOAT, A_NULL);
  class_addmethod(readanysf_class, (t_method)m_loop, gensym("loop"), A_FLOAT, A_NULL);
  class_addmethod(readanysf_class, (t_method)m_pcm_seek, gensym("pcm_seek"), A_FLOAT, A_NULL);
  class_addmethod(readanysf_class, (t_method)m_time_seek, gensym("time_seek"), A_FLOAT, A_NULL);
  class_addbang(readanysf_class, readanysf_bang);

}
