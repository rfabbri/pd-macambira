/******************************************************
 *
 * midisettings - get/set midi preferences from within Pd-patches
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
#ifndef MAXMIDIINDEV
# define MAXMIDIINDEV 4
#endif
#ifndef MAXMIDIOUTDEV
# define MAXMIDIOUTDEV 4
#endif
extern int sys_midiapi;
static t_class *midisettings_class;

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

typedef struct _midisettings
{
  t_object x_obj;
  t_outlet*x_info;
} t_midisettings;


static void midisettings_listdevices(t_midisettings *x)
{
  int i;

  char indevlist[MAXNDEV][DEVDESCSIZE], outdevlist[MAXNDEV][DEVDESCSIZE];
  int indevs = 0, outdevs = 0;

  t_atom atoms[3];
 
  sys_get_midi_devs((char*)indevlist, &indevs, 
                     (char*)outdevlist, &outdevs, 
                     MAXNDEV, DEVDESCSIZE);

  SETSYMBOL (atoms+0, gensym("api"));
  SETSYMBOL (atoms+1, as_getapiname(sys_midiapi));
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

static void midisettings_listparams(t_midisettings *x) {
  int i;
  t_atom atoms[4];

  int nmidiindev=1, midiindev[MAXMIDIINDEV];
  int nmidioutdev, midioutdev[MAXMIDIOUTDEV];
  sys_get_midi_params(&nmidiindev, midiindev, 
                      &nmidioutdev, midioutdev);
  
  SETSYMBOL(atoms+0, gensym("in"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)nmidiindev);
  outlet_anything(x->x_info, gensym("params"), 3, atoms);

  for(i=0; i<nmidiindev; i++) {
    SETFLOAT (atoms+1, (t_float)midiindev[i]);
    outlet_anything(x->x_info, gensym("params"), 2, atoms);
  }

  SETSYMBOL(atoms+0, gensym("out"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)nmidioutdev);
  outlet_anything(x->x_info, gensym("params"), 3, atoms);

  for(i=0; i<nmidioutdev; i++) {
    SETFLOAT (atoms+1, (t_float)midioutdev[i]);
    outlet_anything(x->x_info, gensym("params"), 2, atoms);
  }
}

/*
 * TODO: provide a nicer interface than the "pd audio-dialog"
 */
static void midisettings_set(t_midisettings *x, 
                             t_symbol*s, 
                             int argc, t_atom*argv) {
/*
     "pd audio-dialog ..."
     #00: indev[0]
     #01: indev[1]
     #02: indev[2]
     #03: indev[3]
     #04: outdev[0]
     #05: outdev[1]
     #06: outdev[2]
     #07: outdev[3]
     #08: alsadevin
     #09: alsadevout
*/
/*
  PLAN:
    several messages that accumulate to a certain settings, and then "apply" them
*/

       
}



static void midisettings_testdevices(t_midisettings *x)
{
  int i;

  char indevlist[MAXNDEV][DEVDESCSIZE], outdevlist[MAXNDEV][DEVDESCSIZE];
  int indevs = 0, outdevs = 0;
 
  sys_get_midi_devs((char*)indevlist, &indevs, (char*)outdevlist, &outdevs, MAXNDEV, DEVDESCSIZE);

  post("%d midi indevs", indevs);
  for(i=0; i<indevs; i++)
    post("\t#%02d: %s", i, indevlist[i]);

  post("%d midi outdevs", outdevs);
  for(i=0; i<outdevs; i++)
    post("\t#%02d: %s", i, outdevlist[i]);

  endpost();
  int nmidiindev, midiindev[MAXMIDIINDEV];
  int nmidioutdev, midioutdev[MAXMIDIOUTDEV];
  sys_get_midi_params(&nmidiindev, midiindev,
                       &nmidioutdev, midioutdev);
  
  post("%d midiindev (parms)", nmidiindev);
  for(i=0; i<nmidiindev; i++) {
    post("\t#%02d: %d %d", i, midiindev[i]);
  }
  post("%d midioutdev (parms)", nmidioutdev);
  for(i=0; i<nmidioutdev; i++) {
    post("\t#%02d: %d %d", i, midioutdev[i]);
  }
}
/*
 */
static void midisettings_listapis(t_midisettings *x)
{

  t_as_apis*api=NULL;

  for(api=APIS; api; api=api->next) {
    t_atom ap[1];
    SETSYMBOL(ap, api->name);
    outlet_anything(x->x_info, gensym("driver"), 1, ap);    
  }
}

static void midisettings_free(t_midisettings *x){

}


static void *midisettings_new(void)
{
  t_midisettings *x = (t_midisettings *)pd_new(midisettings_class);
  x->x_info=outlet_new(&x->x_obj, 0);

  char buf[MAXPDSTRING];
  sys_get_midi_apis(buf);

  APIS=as_apiparse(APIS, buf);

  //  sys_get_midi_apis(buf);
  //  APIS=as_apiparse(APIS, buf);


  return (x);
}


void midisettings_setup(void)
{
  post("midisettings: midi settings manager");
  post("          Copyright (C) 2010 IOhannes m zmoelnig");
  post("          for the IntegraLive project");
  post("          institute of electronic music and acoustics (iem)");
  post("          published under the GNU General Public License version 3 or later");
#ifdef MIDISETTINGS_VERSION
  startpost("          version:"MIDISETTINGS_VERSION);
#endif
  post("\tcompiled: "__DATE__"");

  midisettings_class = class_new(gensym("midisettings"), (t_newmethod)midisettings_new, (t_method)midisettings_free,
			     sizeof(t_midisettings), 0, 0);

  class_addbang(midisettings_class, (t_method)midisettings_listapis);
  class_addmethod(midisettings_class, (t_method)midisettings_listapis, gensym("listapis"), A_NULL);
  class_addmethod(midisettings_class, (t_method)midisettings_listdevices, gensym("listdevices"), A_NULL);
 class_addmethod(midisettings_class, (t_method)midisettings_listparams, gensym("listparams"), A_NULL);


  class_addmethod(midisettings_class, (t_method)midisettings_testdevices, gensym("testdevices"), A_NULL);

}

