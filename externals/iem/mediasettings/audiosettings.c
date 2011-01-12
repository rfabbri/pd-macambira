/******************************************************
 *
 * audiosettings - get/set audio preferences from within Pd-patches
 * Copyright (C) 2010 IOhannes m zmölnig
 *
 *   forum::für::umläute
 *
 *   institute of electronic music and acoustics (iem)
 *   university of music and dramatic arts, graz (kug)
 *
 *
 ******************************************************
 *
 * license: GNU General Public License v.3 or later
 *
 ******************************************************/
#include "m_pd.h"
#include "s_stuff.h"
#include <stdio.h>

#define MAXNDEV 20
#define DEVDESCSIZE 80
#define MAXAUDIOINDEV 4
#define MAXAUDIOOUTDEV 4

static t_class *audiosettings_class;

typedef struct _as_apis {
  t_symbol*name;
  int      id;

  struct _as_apis *next;
} t_as_apis;


t_as_apis*as_findapi(t_as_apis*apis, const t_symbol*name) {
  while(apis) {
    if(name==apis->name)return apis;
    apis=apis->next;
  }
  return NULL;
}

t_as_apis*as_findapiid(t_as_apis*apis, const int id) {
  while(apis) {
    if(id==apis->id)return apis;
    apis=apis->next;
  }
  return NULL;
}

t_as_apis*as_addapi(t_as_apis*apis, t_symbol*name, int id, int overwrite) {
  t_as_apis*api=as_findapi(apis, name);

  if(api) {
    if(overwrite) {
      api->name=name;
      api->id  =id;
    }
    return apis;
  }

  api=(t_as_apis*)getbytes(sizeof(t_as_apis));
  api->name=name;
  api->id=id;
  api->next=apis;

  return api;
}

t_as_apis*as_apiparse(t_as_apis*apis, const char*buf) {
  t_as_apis*api=apis;

  int start=-1;
  int stop =-1;

  unsigned int index=0;
  int depth=0;
  const char*s;
  char substring[MAXPDSTRING];

  for(index=0, s=buf; 0!=*s; s++, index++) {
    if('{'==*s) {
      start=index;
      depth++;
    }
    if('}'==*s) {
      depth--;
      stop=index;

      if(start>=0 && start<stop) {
        char apiname[MAXPDSTRING];
        int  apiid;
        int length=stop-start;
        if(length>=MAXPDSTRING)length=MAXPDSTRING-1;
        snprintf(substring, length, "%s", buf+start+1);
        
        if(2==sscanf(substring, "%s %d", apiname, &apiid)) {
          apis=as_addapi(apis, gensym(apiname), apiid, 0);
        } else {
          post("unparseable: '%s'", substring);
        }
      }
      start=-1;
      stop=-1;
    }
  }

  return apis;
}

static t_as_apis*APIS=NULL;

static t_symbol*as_getapiname(const int id) {
  t_as_apis*api=as_findapiid(APIS, id);
  if(api) {
    return api->name;
  } else {
    return gensym("<unknown>");
  }
}

static int as_getapiid(const t_symbol*id) {
  t_as_apis*api=as_findapi(APIS, id);
  if(api) {
    return api->id;
  }
  return -1; /* unknown */
}

typedef struct _audiosettings
{
  t_object x_obj;
  t_outlet*x_info;
} t_audiosettings;


static void audiosettings_listparams(t_audiosettings *x);
static void audiosettings_listdevices(t_audiosettings *x)
{
  int i;

  char indevlist[MAXNDEV][DEVDESCSIZE], outdevlist[MAXNDEV][DEVDESCSIZE];
  int indevs = 0, outdevs = 0, canmulti = 0, cancallback = 0;

  t_atom atoms[3];
 
  sys_get_audio_devs((char*)indevlist, &indevs, 
                     (char*)outdevlist, &outdevs, 
                     &canmulti, &cancallback, 
                     MAXNDEV, DEVDESCSIZE);

  SETSYMBOL (atoms+0, gensym("api"));
  SETSYMBOL (atoms+1, as_getapiname(sys_audioapi));
  outlet_anything(x->x_info, gensym("device"), 2, atoms);

  SETSYMBOL (atoms+0, gensym("multi"));
  SETFLOAT (atoms+1, (t_float)canmulti);
  outlet_anything(x->x_info, gensym("device"), 2, atoms);

  SETSYMBOL (atoms+0, gensym("callback"));
  SETFLOAT (atoms+1, (t_float)cancallback);
  outlet_anything(x->x_info, gensym("device"), 2, atoms);

  SETSYMBOL(atoms+0, gensym("in"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)indevs);
  outlet_anything(x->x_info, gensym("device"), 3, atoms);

  for(i=0; i<indevs; i++) {
    SETFLOAT (atoms+1, (t_float)i);
    SETSYMBOL(atoms+2, gensym(indevlist[i]));
    outlet_anything(x->x_info, gensym("device"), 3, atoms);
  }

  SETSYMBOL(atoms+0, gensym("out"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)outdevs);
  outlet_anything(x->x_info, gensym("device"), 2, atoms);

  for(i=0; i<outdevs; i++) {
    SETFLOAT (atoms+1, (t_float)i);
    SETSYMBOL(atoms+2, gensym(outdevlist[i]));
    outlet_anything(x->x_info, gensym("device"), 3, atoms);
  }
}
static void audiosettings_listparams(t_audiosettings *x) {
  int i;
  t_atom atoms[4];

  int naudioindev=1, audioindev[MAXAUDIOINDEV], chindev[MAXAUDIOINDEV];
  int naudiooutdev, audiooutdev[MAXAUDIOOUTDEV], choutdev[MAXAUDIOOUTDEV];
  int rate, advance, callback;
  sys_get_audio_params(&naudioindev, audioindev, chindev,
                       &naudiooutdev, audiooutdev, choutdev, 
                       &rate, &advance, &callback);
  

  SETSYMBOL (atoms+0, gensym("rate"));
  SETFLOAT (atoms+1, (t_float)rate);
  outlet_anything(x->x_info, gensym("params"), 2, atoms);

  SETSYMBOL (atoms+0, gensym("advance"));
  SETFLOAT (atoms+1, (t_float)advance);
  outlet_anything(x->x_info, gensym("params"), 2, atoms);

  SETSYMBOL (atoms+0, gensym("callback"));
  SETFLOAT (atoms+1, (t_float)callback);
  outlet_anything(x->x_info, gensym("params"), 2, atoms);

  SETSYMBOL(atoms+0, gensym("in"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)naudioindev);
  outlet_anything(x->x_info, gensym("params"), 3, atoms);

  for(i=0; i<naudioindev; i++) {
    SETFLOAT (atoms+1, (t_float)audioindev[i]);
    SETFLOAT (atoms+2, (t_float)chindev[i]);
    outlet_anything(x->x_info, gensym("params"), 3, atoms);
  }

  SETSYMBOL(atoms+0, gensym("out"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)naudiooutdev);
  outlet_anything(x->x_info, gensym("params"), 3, atoms);

  for(i=0; i<naudiooutdev; i++) {
    SETFLOAT (atoms+1, (t_float)audiooutdev[i]);
    SETFLOAT (atoms+2, (t_float)choutdev[i]);
    outlet_anything(x->x_info, gensym("params"), 3, atoms);
  }
}

/*
 * TODO: provide a nicer interface than the "pd audio-dialog"
 */
static void audiosettings_set(t_audiosettings *x, t_symbol*s, int argc, t_atom*argv) {
/*
     "pd audio-dialog ..."
     #00: indev[0]
     #01: indev[1]
     #02: indev[2]
     #03: indev[3]
     #04: inchan[0]
     #05: inchan[1]
     #06: inchan[2]
     #07: inchan[3]
     #08: outdev[0]
     #09: outdev[1]
     #10: outdev[2]
     #11: outdev[3]
     #12: outchan[0]
     #13: outchan[1]
     #14: outchan[2]
     #15: outchan[3]
     #16: rate
     #17: advance
     #18: callback
*/
/*
  PLAN:
    several messages that accumulate to a certain settings, and then "apply" them
*/

       
}



static void audiosettings_testdevices(t_audiosettings *x)
{
  int i;

  char indevlist[MAXNDEV][DEVDESCSIZE], outdevlist[MAXNDEV][DEVDESCSIZE];
  int indevs = 0, outdevs = 0, canmulti = 0, cancallback = 0;
 
  sys_get_audio_devs((char*)indevlist, &indevs, (char*)outdevlist, &outdevs, &canmulti,
                &cancallback, MAXNDEV, DEVDESCSIZE);

  post("%d indevs", indevs);
  for(i=0; i<indevs; i++)
    post("\t#%02d: %s", i, indevlist[i]);

  post("%d outdevs", outdevs);
  for(i=0; i<outdevs; i++)
    post("\t#%02d: %s", i, outdevlist[i]);

  post("multi: %d\tcallback: %d", canmulti, cancallback);

  endpost();

  int naudioindev, audioindev[MAXAUDIOINDEV], chindev[MAXAUDIOINDEV];
  int naudiooutdev, audiooutdev[MAXAUDIOOUTDEV], choutdev[MAXAUDIOOUTDEV];
  int rate, advance, callback;
  sys_get_audio_params(&naudioindev, audioindev, chindev,
                       &naudiooutdev, audiooutdev, choutdev, 
                       &rate, &advance, &callback);
  
  post("%d audioindev (parms)", naudioindev);
  for(i=0; i<naudioindev; i++) {
    post("\t#%02d: %d %d", i, audioindev[i], chindev[i]);
  }
  post("%d audiooutdev (parms)", naudiooutdev);
  for(i=0; i<naudiooutdev; i++) {
    post("\t#%02d: %d %d", i, audiooutdev[i], choutdev[i]);
  }
  post("rate=%d\tadvance=%d\tcallback=%d", rate, advance, callback);

}
/*
 */
static void audiosettings_listapis(t_audiosettings *x)
{

  t_as_apis*api=NULL;

  for(api=APIS; api; api=api->next) {
    t_atom ap[1];
    SETSYMBOL(ap, api->name);
    outlet_anything(x->x_info, gensym("driver"), 1, ap);    
  }
}

static void audiosettings_setapi(t_audiosettings *x, t_symbol*s) {
  int id=as_getapiid(s);
  if(id<0) {
    pd_error(x, "invalid API: '%s'", s->s_name);
  }
  verbose(1, "setting API: %d", id);
  sys_set_audio_api(id);
  sys_reopen_audio();
}

static void audiosettings_bang(t_audiosettings *x) {
  audiosettings_listapis(x);
  audiosettings_listdevices(x);
  audiosettings_listparams(x);
}


static void audiosettings_free(t_audiosettings *x){

}


static void *audiosettings_new(void)
{
  t_audiosettings *x = (t_audiosettings *)pd_new(audiosettings_class);
  x->x_info=outlet_new(&x->x_obj, 0);

  char buf[MAXPDSTRING];
  sys_get_audio_apis(buf);

  APIS=as_apiparse(APIS, buf);

  return (x);
}


void audiosettings_setup(void)
{
  post("audiosettings: audio settings manager");
  post("          Copyright (C) 2010 IOhannes m zmoelnig");
  post("          for the IntegraLive project");
  post("          institute of electronic music and acoustics (iem)");
  post("          published under the GNU General Public License version 3 or later");
#ifdef AUDIOSETTINGS_VERSION
  startpost("          version:"AUDIOSETTINGS_VERSION);
#endif
  post("\tcompiled: "__DATE__"");

  audiosettings_class = class_new(gensym("audiosettings"), (t_newmethod)audiosettings_new, (t_method)audiosettings_free,
			     sizeof(t_audiosettings), 0, 0);

  class_addbang(audiosettings_class, (t_method)audiosettings_bang);
  class_addmethod(audiosettings_class, (t_method)audiosettings_listapis, gensym("listapis"), A_NULL);
  class_addmethod(audiosettings_class, (t_method)audiosettings_listdevices, gensym("listdevices"), A_NULL);
  class_addmethod(audiosettings_class, (t_method)audiosettings_listparams, gensym("listparams"), A_NULL);


  class_addmethod(audiosettings_class, (t_method)audiosettings_setapi, gensym("setapi"), A_SYMBOL);


  class_addmethod(audiosettings_class, (t_method)audiosettings_testdevices, gensym("testdevices"), A_NULL);

}

