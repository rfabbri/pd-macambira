/* --------------------------- k_sc~  ----------------------------------- */
/*   ;; Kjetil S. Matheussen, 2004.                                             */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* ---------------------------------------------------------------------------- */



#include "fromto.c"

#include <stdio.h>
#include <stdbool.h>
#include <jack/jack.h>


static int num_ins=0;
static int num_outs=0;

static int getnumjackchannels(jack_client_t *client,char *regstring){
  int lokke=0;
  const char **ports=jack_get_ports(client,regstring,"",0);
  if(ports==NULL) return 0;
  while(ports[lokke]!=NULL){
    lokke++;
  }
  return lokke;
}

static void setupjack(void){
  static bool inited=false;
  int lokke;
  int num_sc_in,num_sc_out;

  jack_client_t *client;

  if(inited==true) return;

  if(sys_audioapi!=API_JACK){
    post("Error. k_sc~ will not work without jack as the sound API.");
    goto apiwasnotjack;
  }

  client=jack_client_new("k_sc_tilde");

  num_sc_in=getnumjackchannels(client,"SuperCollider:in_*");
  num_sc_out=getnumjackchannels(client,"SuperCollider:out_*");

  if(num_sc_in==0 || num_sc_out==0){
    post("Error. No Supercollider jack ports found.");
    goto nosupercolliderportsfound;
  }

  num_ins=sys_get_inchannels();
  num_outs=sys_get_outchannels();

  {
    int t1[1]={0};
    int t2[1]={0};
    int t3[1]={num_sc_out+num_ins};
    int t4[1]={num_sc_in+num_outs};
    sys_close_audio();
    sys_open_audio(1,t1,
		   1,t3,
		   1,t2,
		   1,t4,
		   sys_getsr(),sys_schedadvance/1000,1);
  }

  for(lokke=0;lokke<num_sc_in;lokke++){
    char temp[500];
    char temp2[500];
    sprintf(temp,"pure_data_0:output%d",lokke+num_outs);
    sprintf(temp2,"SuperCollider:in_%d",lokke+1);
    jack_connect(client,temp,temp2);
    sprintf(temp,"alsa_pcm:capture_%d",lokke+1);
    jack_disconnect(client,temp,temp2);
  }
  for(lokke=0;lokke<num_sc_out;lokke++){
    char temp[500];
    char temp2[500];
    sprintf(temp,"pure_data_0:input%d",lokke+num_ins);
    sprintf(temp2,"SuperCollider:out_%d",lokke+1);
    jack_connect(client,temp2,temp);
    sprintf(temp,"alsa_pcm:playback_%d",lokke+1);
    jack_disconnect(client,temp2,temp);
  }

  inited=true;

 nosupercolliderportsfound:
  jack_client_close(client);


 apiwasnotjack:
  return;
}

static void *from_sc_newnew(t_symbol *s, int argc, t_atom *argv){
  int lokke;
  t_from_sc *x;
  setupjack();
  x=from_sc_new(s,argc,argv);

  for(lokke=0;lokke<x->x_n;lokke++){
    x->x_vec[lokke]+=num_outs;
  }
  return x;
}

static void *to_sc_newnew(t_symbol *s, int argc, t_atom *argv){
  int lokke;
  t_to_sc *x;
  setupjack();
  x=to_sc_new(s,argc,argv);

  for(lokke=0;lokke<x->x_n;lokke++){
    x->x_vec[lokke]+=num_ins;
  }
  return x;
}


static void k_sc_tilde_setup(void){
  d_to_sc_setup();
}


void from_sc_tilde_setup(void){
  k_sc_tilde_setup();
}

void to_sc_tilde_setup(void){
  k_sc_tilde_setup();
}
