/******************************************************
 *
 * espeak - implementation file
 *
 * (c) copyright 2011 IOhannes m zmölnig
 * (c) copyright 2011 Matthias Kronlachner
 * (c) copyright 2011 Marian Weger
 *
 *  within the course: Künstlerisches Gestalten mit Klang 2010/2011
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

#include "m_pd.h"
#include <espeak/speak_lib.h>
#include <string.h>

/* general */

static int espeak_rate=0;
#define ESPEAK_BUFFER 50000


typedef struct _espeak
{
  t_object x_obj;

  short*x_buffer;        /* buffer of synth'ed samples */
  unsigned int x_buflen; /* length of the buffer */

  unsigned int x_position; /* playback position for perform-routine */

  int x_ready;           /* flag: if true, start playback */


  t_outlet*x_infoout;
} t_espeak;

/* ------------------------ espeak~ ----------------------------- */ 

static t_class *espeak_class;

static t_int *espeak_perform(t_int *w){
  t_espeak *x = (t_espeak *)(w[1]);
  t_sample *out = (t_sample *)(w[2]);
  int n = (int)(w[3]);

  if(x->x_ready) {
    // got data to play back
    n=n/2;

    short*in=x->x_buffer+x->x_position;

    x->x_position+=n;
    if(x->x_position>=x->x_buflen)
      x->x_ready=0;


    while(n--) {
      t_sample f=(*in++)/32768.;
      *out++ = f;
      *out++ = f;
    }

  } else {
    while(n--) {
      *out++=0.f;
    }
  }

  return (w+4);
}

static void espeak_dsp(t_espeak *x, t_signal **sp){

  t_signal*sig=sp[0];


  dsp_add(espeak_perform, 3, 
	  x, 
	  sp[0]->s_vec, 
	  sp[0]->s_n);
}

static int espeak_callback(short *wav, int numsamples, espeak_EVENT *events) {
  int i=0;

  verbose(1, "get %d samples", numsamples);

  if(numsamples) {
    t_espeak*x = (t_espeak*)events[0].user_data;
    if(NULL==x)
      return 1;
    
    memset(x->x_buffer, 0, x->x_buflen*sizeof(short));
    memcpy(x->x_buffer, wav, numsamples*sizeof(short));
    x->x_ready = 1;
    x->x_position = 0;
  }

  return 0;
}

static void espeak_text(t_espeak *x, t_symbol*s, int argc, t_atom*argv) {
  t_binbuf*bb=binbuf_new();
  int size=0;
  char*text=NULL;

  binbuf_add(bb, argc, argv);
  binbuf_gettext(bb, &text, &size);
  binbuf_free(bb);

  text[size]=0;

  verbose(1, "speak '%s'", text);

  espeak_Synth(text,
	       strlen(text),
	       0,
	       POS_CHARACTER,
	       0,
	       espeakCHARS_AUTO,
	       NULL,
	       x);
}


static void espeak_info(t_espeak*x){
  const espeak_VOICE**voices=espeak_ListVoices(NULL);
  int i=0;

  while(voices[i]) {
    
    t_atom ap[3];
    SETSYMBOL(ap+0, gensym(voices[i]->name));
    SETSYMBOL(ap+1, gensym(voices[i]->languages));
    SETSYMBOL(ap+2, gensym(voices[i]->identifier));

    outlet_anything(x->x_infoout, gensym("voice"), 3, ap);

    i++;
  }
}


static void espeak_voice(t_espeak*x, t_symbol*s){
  espeak_ERROR err= espeak_SetVoiceByName(s->s_name);
}

static void espeak_free(t_espeak*x){
  freebytes(x->x_buffer, x->x_buflen*sizeof(short));
  if(x->x_infoout)
    outlet_free(x->x_infoout);

}

static void *espeak_new(void){
  t_espeak *x = (t_espeak *)pd_new(espeak_class);

  x->x_buflen = espeak_rate*ESPEAK_BUFFER/1000;
  x->x_buffer = (short*)getbytes(x->x_buflen*sizeof(short));

  outlet_new(&x->x_obj, gensym("signal"));
  x->x_infoout=outlet_new(&x->x_obj, NULL);

  return (x);
}

void espeak_tilde_setup(void){

  int result=espeak_Initialize(AUDIO_OUTPUT_RETRIEVAL, 
			       ESPEAK_BUFFER,
			       NULL,
			       0);


  if(result<0) {
    error("couldn't initialize eSpeak");
    return;
  }

  espeak_SetSynthCallback(espeak_callback);
  espeak_rate=result;

  post("eSpeak running at %d Hz", result);

  espeak_class = class_new(gensym("espeak~"), (t_newmethod)espeak_new, 0, sizeof(t_espeak), 0, 0);

  class_addmethod(espeak_class, (t_method)espeak_dsp, gensym("dsp"), 0);

  class_addmethod(espeak_class, (t_method)espeak_text, gensym("speak"), A_GIMME, 0);


  class_addmethod(espeak_class, (t_method)espeak_voice, gensym("voice"), A_SYMBOL, 0);


  class_addbang(espeak_class, (t_method)espeak_info);

}
