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
#include <string.h>

#define MAXNDEV 20
#define DEVDESCSIZE 80

#ifndef MAXMIDIINDEV
# define MAXMIDIINDEV 4
#endif
#ifndef MAXMIDIOUTDEV
# define MAXMIDIOUTDEV 4
#endif

#if MAXMIDIINDEV > MAXMIDIOUTDEV
# define MAXMIDIDEV MAXMIDIINDEV
#else
# define MAXMIDIDEV MAXMIDIOUTDEV
#endif



extern int sys_midiapi;
static t_class *midisettings_class;

static t_symbol*s_pdsym=NULL;

typedef struct _ms_symkeys {
  t_symbol*name;
  int      id;

  struct _ms_symkeys *next;
} t_ms_symkeys;

typedef t_ms_symkeys t_ms_drivers ;


static const char*ms_defaultdrivername(const int id) {
  switch (id) {
  case API_NONE:
    return NULL;
  case API_ALSA:
    return "ALSA-MIDI";
  default:
    return "default-MIDI"; /* such a stupid name! */
  }
  return NULL;
}

static t_ms_symkeys*ms_symkeys_find(t_ms_symkeys*symkeys, const t_symbol*name) {
  while(symkeys) {
    if(name==symkeys->name)return symkeys;
    symkeys=symkeys->next;
  }
  return NULL;
}

static t_ms_symkeys*ms_symkeys_findid(t_ms_symkeys*symkeys, const int id) {
  while(symkeys) {
    if(id==symkeys->id)return symkeys;
    symkeys=symkeys->next;
  }
  return NULL;
}

static t_ms_symkeys*ms_symkeys_add(t_ms_symkeys*symkeys, t_symbol*name, int id, int overwrite) {
  t_ms_symkeys*symkey=ms_symkeys_find(symkeys, name);

  if(symkey) {
    if(overwrite) {
      symkey->name=name;
      symkey->id  =id;
    }
    return symkeys;
  }

  symkey=(t_ms_symkeys*)getbytes(sizeof(t_ms_symkeys));
  symkey->name=name;
  symkey->id=id;
  symkey->next=symkeys;

  return symkey;
}

static void ms_symkeys_clear(t_ms_symkeys*symkeys) {
  t_ms_symkeys*symkey=symkeys;
  while(symkey) {
    t_ms_symkeys*next=symkey->next;

    symkey->name=NULL;
    symkey->id=0;
    symkey->next=NULL;

    freebytes(symkey, sizeof(t_ms_symkeys));

    symkey=next;
  }
}

static t_symbol*ms_symkeys_getname(t_ms_symkeys*symkeys, const int id) {
  t_ms_symkeys*driver=ms_symkeys_findid(symkeys, id);
  if(driver)
    return driver->name;
  return NULL;
}
static int ms_symkeys_getid(t_ms_symkeys*symkeys, const t_symbol*name) {
  t_ms_symkeys*symkey=ms_symkeys_find(symkeys, name);
  if(symkey) {
    return symkey->id;
  }
  return -1; /* unknown */
}


t_ms_symkeys*ms_driverparse(t_ms_symkeys*drivers, const char*buf) {
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
        char drivername[MAXPDSTRING];
        int  driverid;
        int length=stop-start;
        if(length>=MAXPDSTRING)length=MAXPDSTRING-1;
        snprintf(substring, length, "%s", buf+start+1);
        
        if(2==sscanf(substring, "%s %d", drivername, &driverid)) {
          drivers=ms_symkeys_add(drivers, gensym(drivername), driverid, 0);
        } else {
          if((start+1)!=(stop)) /* empty APIs string */
            post("unparseable: '%s'", substring);
        }
      }
      start=-1;
      stop=-1;
    }
  }

  return drivers;
}

static t_ms_symkeys*DRIVERS=NULL;

static t_symbol*ms_getdrivername(const int id) {
  t_symbol*s=ms_symkeys_getname(DRIVERS, id);
  if(s)
    return s;
  else {
    const char*name=ms_defaultdrivername(id);
    if(name)
      return gensym(name);
  }
  return gensym("<unknown>");
}

static int ms_getdriverid(const t_symbol*id) {
  return ms_symkeys_getid(DRIVERS, id);
}



typedef struct _ms_params {
  int indev[MAXMIDIINDEV], outdev[MAXMIDIOUTDEV];
  int inchannels, outchannels;
} t_ms_params;

static void ms_params_print(t_ms_params*parms) {
  int i=0;
#if 0
  const int maxin =MAXMIDIINDEV;
  const int maxout=MAXMIDIOUTDEV;
#else
  const int maxin =parms->inchannels; 
  const int maxout=parms->outchannels;
#endif

  post("\n=================================<");
  if(API_ALSA == sys_midiapi) {
    post("alsamidi: %d %d", parms->inchannels, parms->outchannels);
  } else {
    for(i=0; i<maxin; i++) {
      post("indev[%d]: %d", i, parms->indev[i]);
    }
    for(i=0; i<maxout; i++) {
      post("outdev[%d]: %d", i, parms->outdev[i]);
    }
  }

  post(">=================================\n");

}
static void ms_params_get(t_ms_params*parms) {
  memset(parms, 0, sizeof(t_ms_params));

  sys_get_midi_params(&parms->inchannels, parms->indev,
                      &parms->outchannels, parms->outdev);

  ms_params_print(parms);
}





typedef struct _midisettings
{
  t_object x_obj;
  t_outlet*x_info;

  t_ms_params x_params;
} t_midisettings;

static void midisettings_params_init(t_midisettings*x) {
  int i=0;
  for(i=0; i<MAXMIDIINDEV ; i++) x->x_params.indev [i]=0;
  for(i=0; i<MAXMIDIOUTDEV; i++) x->x_params.outdev[i]=0;

  x->x_params.inchannels = x->x_params.outchannels = 0;
  
  ms_params_get(&x->x_params);
}


static void midisettings_listdevices(t_midisettings *x)
{
  int i, count;

#define MS_ALSADEV_FORMAT "ALSA-%02d"

  char indevlist[MAXNDEV][DEVDESCSIZE], outdevlist[MAXNDEV][DEVDESCSIZE];
  int indevs = 0, outdevs = 0;

  t_atom atoms[MAXMIDIDEV+1];
 
  sys_get_midi_devs((char*)indevlist, &indevs, 
                     (char*)outdevlist, &outdevs, 
                     MAXNDEV, DEVDESCSIZE);

  ms_params_get(&x->x_params);

  SETSYMBOL(atoms+0, gensym("in"));
  count=0;
  for(i=0; i<x->x_params.inchannels; i++) {
    char dummy[MAXPDSTRING];
    char*devname=NULL;
    if(API_ALSA == sys_midiapi) {
      snprintf(dummy, MAXPDSTRING, MS_ALSADEV_FORMAT, i);
      dummy[MAXPDSTRING-1]=0;
      devname=dummy;
    } else {
      devname=indevlist[x->x_params.indev[i]];
    }
    post("indev%d: '%s'", i, devname);
    if(devname) {
      SETSYMBOL(atoms+count+1, gensym(devname));
      count++;
    }
  }
  outlet_anything(x->x_info, gensym("device"), count+1, atoms);


 SETSYMBOL(atoms+0, gensym("out"));
  count=0;
  for(i=0; i<x->x_params.outchannels; i++) {
    char dummy[MAXPDSTRING];
    char*devname=NULL;
    if(API_ALSA == sys_midiapi) {
      snprintf(dummy, MAXPDSTRING, MS_ALSADEV_FORMAT, i);
      dummy[MAXPDSTRING-1]=0;
      devname=dummy;
    } else {
      devname=outdevlist[x->x_params.outdev[i]];
    }
    post("outdev%d: '%s'", i, devname);
    if(devname) {
      SETSYMBOL(atoms+count+1, gensym(devname));
      count++;
    }
  }
  outlet_anything(x->x_info, gensym("device"), count+1, atoms);
 
  SETSYMBOL(atoms+0, gensym("in"));
  if(API_ALSA == sys_midiapi) {
    char dummy[MAXPDSTRING];
    indevs=MAXMIDIINDEV;
    SETFLOAT (atoms+1, (t_float)indevs);
    outlet_anything(x->x_info, gensym("devicelist"), 2, atoms);
    for(i=0; i<indevs; i++) {
      snprintf(dummy, MAXPDSTRING, MS_ALSADEV_FORMAT, i);
      dummy[MAXPDSTRING-1]=0;
      SETSYMBOL(atoms+1, gensym(dummy));
      SETFLOAT (atoms+2, (t_float)i);
      outlet_anything(x->x_info, gensym("devicelist"), 3, atoms);
    }
  } else {
    SETFLOAT (atoms+1, (t_float)indevs);
    outlet_anything(x->x_info, gensym("devicelist"), 2, atoms);
    for(i=0; i<indevs; i++) {
      SETSYMBOL(atoms+1, gensym(indevlist[i]));
      SETFLOAT (atoms+2, (t_float)i);
      outlet_anything(x->x_info, gensym("devicelist"), 3, atoms);
    }
  }

  SETSYMBOL(atoms+0, gensym("out"));
  if(API_ALSA == sys_midiapi) {
    char dummy[MAXPDSTRING];
    outdevs=MAXMIDIOUTDEV;
    SETFLOAT (atoms+1, (t_float)outdevs);
    outlet_anything(x->x_info, gensym("devicelist"), 2, atoms);
    for(i=0; i<outdevs; i++) {
      snprintf(dummy, MAXPDSTRING, MS_ALSADEV_FORMAT, i);
      dummy[MAXPDSTRING-1]=0;
      SETSYMBOL(atoms+1, gensym(dummy));
      SETFLOAT (atoms+2, (t_float)i);
      outlet_anything(x->x_info, gensym("devicelist"), 3, atoms);
    }
  } else {
    SETFLOAT (atoms+1, (t_float)outdevs);
    outlet_anything(x->x_info, gensym("devicelist"), 2, atoms);
    for(i=0; i<outdevs; i++) {
      SETSYMBOL(atoms+1, gensym(outdevlist[i]));
      SETFLOAT (atoms+2, (t_float)i);
      outlet_anything(x->x_info, gensym("devicelist"), 3, atoms);
    }
  }
}

/*
 * TODO: provide a nicer interface than the "pd midi-dialog"
 */
static void midisettings_set(t_midisettings *x, 
                             t_symbol*s, 
                             int argc, t_atom*argv) {
/*
     "pd midi-dialog ..."
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




static void midisettings_params_apply(t_midisettings*x) {
/*
     "pd midi-dialog ..."
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

  int alsamidi=0;

  t_atom argv [MAXMIDIINDEV+MAXMIDIOUTDEV+2];
  int    argc= MAXMIDIINDEV+MAXMIDIOUTDEV+2;

  int i=0;

  ms_params_print(&x->x_params);


  for(i=0; i<MAXMIDIINDEV; i++) {
    SETFLOAT(argv+i+0*MAXMIDIINDEV, (t_float)(alsamidi?0:x->x_params.indev[i]));
  }
  for(i=0; i<MAXMIDIOUTDEV; i++) {
    SETFLOAT(argv+i+1*MAXMIDIINDEV, (t_float)(alsamidi?0:x->x_params.outdev[i]));
  }

  
  SETFLOAT(argv+2*MAXMIDIINDEV+2*MAXMIDIOUTDEV+0,(t_float)(alsamidi?x->x_params.inchannels:0));
  SETFLOAT(argv+2*MAXMIDIINDEV+2*MAXMIDIOUTDEV+1,(t_float)(alsamidi?x->x_params.outchannels:0));

  if (s_pdsym->s_thing) typedmess(s_pdsym->s_thing, 
				  gensym("midi-dialog"), 
				  argc,
				  argv);
}


/* find the beginning of the next parameter in the list */
typedef enum {
  PARAM_INPUT,
  PARAM_OUTPUT,
  PARAM_INVALID
} t_paramtype;
static t_paramtype midisettings_setparams_id(t_symbol*s) {
  if(gensym("@input")==s) {
    return PARAM_INPUT;
  } else if(gensym("@output")==s) {
    return PARAM_OUTPUT;
  }
  return PARAM_INVALID;
}

/* find the beginning of the next parameter in the list */
static int midisettings_setparams_next(int argc, t_atom*argv) {
  int i=0;
  for(i=0; i<argc; i++) {
    if(A_SYMBOL==argv[i].a_type) {
      t_symbol*s=atom_getsymbol(argv+i);
      if('@'==s->s_name[0])
	return i;
    }
  }
  return i;
}

/* [<device1> [<deviceN>]*] ... */
static int midisettings_setparams_input(t_midisettings*x, int argc, t_atom*argv) {
  int length=midisettings_setparams_next(argc, argv);
  int i;
  int dev=0;

  if(length>MAXMIDIINDEV)
    length=MAXMIDIINDEV;

  x->x_params.inchannels = length;

  for(i=0; i<length; i++) {
    switch(argv[2*i].a_type) {
    case A_FLOAT:
      dev=atom_getint(argv);
      break;
    case A_SYMBOL:
    default:
      continue;
    }
    x->x_params.indev[i]=dev;
  }


  return length;
}

static int midisettings_setparams_output(t_midisettings*x, int argc, t_atom*argv) {
  int length=midisettings_setparams_next(argc, argv);
  int i;
  int numpairs=length/2;

  if(length%2)return length;

  if(numpairs>MAXMIDIOUTDEV)
    numpairs=MAXMIDIOUTDEV;

  for(i=0; i<numpairs; i++) {
    int dev=0;
    int ch=0;

    if(A_FLOAT==argv[2*i+0].a_type) {
      dev=atom_getint(argv);
    } else if (A_SYMBOL==argv[2*i+0].a_type) {
      // LATER: get the device-id from the device-name
      continue;
    } else {
      continue;
    }
    ch=atom_getint(argv+2*i+1);

    x->x_params.outdev[i]=dev;
  }

  return length;
}

static void midisettings_setparams(t_midisettings *x, t_symbol*s, int argc, t_atom*argv) {
/*
 * PLAN:
 *   several messages that accumulate to a certain settings, and then "apply" them
 *
 * normal midi: list up to four devices (each direction)
 * alsa   midi: choose number of ports (each direction)
    
*/
  int apply=1;
  int advance=0;
  t_paramtype param=PARAM_INVALID;

  advance=midisettings_setparams_next(argc, argv);
  while((argc-=advance)>0) {
    argv+=advance;
    s=atom_getsymbol(argv);
    param=midisettings_setparams_id(s);

    argv++;
    argc--;

    switch(param) {
    case PARAM_INPUT:
      advance=midisettings_setparams_input(x, argc, argv);
      break;
    case PARAM_OUTPUT:
      advance=midisettings_setparams_output(x, argc, argv);
      break;
    default:
      pd_error(x, "unknown parameter"); postatom(1, argv);endpost();
      break;
    }

    argc-=advance;
    argv+=advance;
    advance=midisettings_setparams_next(argc, argv);
  }
  if(apply) {
    //    midisettings_params_apply(x);
    midisettings_params_init (x); /* re-initialize to what we got */
  }
}

static void midisettings_listdrivers(t_midisettings *x);
static void midisettings_setdriver(t_midisettings *x, t_symbol*s, int argc, t_atom*argv) {
  int id=-1;
  s=gensym("<unknown>"); /* just re-use the argument, which is not needed anyhow */
  switch(argc) {
  case 0:
    midisettings_listdrivers(x);
    return;
  case 1:
    if(A_FLOAT==argv->a_type) {
      s=ms_getdrivername(atom_getint(argv));
      break;
    } else if (A_SYMBOL==argv->a_type) {
      s=atom_getsymbol(argv);
      break;
    }
  }

  if(NULL==DRIVERS) {
    id=sys_midiapi;
  } else {
    id=ms_getdriverid(s);
    if(id<0) {
      pd_error(x, "invalid driver '%s'", s->s_name);
      return;
    }
    verbose(1, "setting driver '%s' (=%d)", s->s_name, id);
  }
#ifdef HAVE_SYS_CLOSE_MIDI
  sys_close_midi();
  sys_set_midi_api(id);
  sys_reopen_midi();
#else
  if (s_pdsym->s_thing) {
    t_atom ap[1];
    SETFLOAT(ap, id);
    typedmess(s_pdsym->s_thing, 
              gensym("midi-setapi"), 
              1,
              ap);
  }
#endif
}




/*
 */
static void midisettings_listdrivers(t_midisettings *x)
{
  t_ms_drivers*driver=NULL;
  t_atom ap[2];
  size_t count=0;

  SETSYMBOL(ap+0, ms_getdrivername(sys_midiapi));
  outlet_anything(x->x_info, gensym("driver"), 1, ap);    


  for(driver=DRIVERS; driver; driver=driver->next) {
    count++;
  }


  SETFLOAT(ap+0, count);
  outlet_anything(x->x_info, gensym("driverlist"), 1, ap);

  for(driver=DRIVERS; driver; driver=driver->next) {
    SETSYMBOL(ap+0, driver->name);
    SETFLOAT (ap+1, (t_float)(driver->id));
    outlet_anything(x->x_info, gensym("driverlist"), 2, ap);    
  }
}

static void midisettings_bang(t_midisettings *x) {
  midisettings_listdrivers(x);
  midisettings_listdevices(x);
}


static void midisettings_free(t_midisettings *x){

}


static void *midisettings_new(void)
{
  t_midisettings *x = (t_midisettings *)pd_new(midisettings_class);
  x->x_info=outlet_new(&x->x_obj, 0);

  char buf[MAXPDSTRING];
  sys_get_midi_apis(buf);
  DRIVERS=ms_driverparse(DRIVERS, buf);

  midisettings_params_init (x); /* re-initialize to what we got */

  return (x);
}

static void midisettings_testdevices(t_midisettings *x);

void midisettings_setup(void)
{
  s_pdsym=gensym("pd");

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

  class_addbang(midisettings_class, (t_method)midisettings_bang);
  class_addmethod(midisettings_class, (t_method)midisettings_listdrivers, gensym("listdrivers"), A_NULL);
  class_addmethod(midisettings_class, (t_method)midisettings_listdevices, gensym("listdevices"), A_NULL);

  class_addmethod(midisettings_class, (t_method)midisettings_setdriver, gensym("driver"), A_GIMME);

  class_addmethod(midisettings_class, (t_method)midisettings_testdevices, gensym("testdevices"), A_NULL);
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
