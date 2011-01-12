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
#include <stdio.h>


void sys_get_audio_apis(char*buf);

#ifdef _WIN32
/* we have to implement the not exported functions for w32... */
#endif


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



t_as_apis*as_apiparse_42(t_as_apis*apis, const char*buf) {
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


t_as_apis*as_apiparse(t_as_apis*apis, const char*buf) {
  return as_apiparse_42(apis, buf);
}

static t_as_apis*APIS=NULL;

typedef struct _audiosettings
{
  t_object x_obj;
  t_outlet*x_info;
} t_audiosettings;


static void audiosettings_listdevices(t_audiosettings *x, t_symbol *s)
{

}

/*
 */
static void audiosettings_bang(t_audiosettings *x)
{

  t_as_apis*api=NULL;

  for(api=APIS; api; api=api->next) {
    t_atom ap[1];
    SETSYMBOL(ap, api->name);
    outlet_anything(x->x_info, gensym("driver"), 1, ap);    
  }

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
  class_addmethod(audiosettings_class, (t_method)audiosettings_listdevices, gensym("listdevices"), A_GIMME);
}

