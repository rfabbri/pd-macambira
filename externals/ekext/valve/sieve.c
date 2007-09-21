/* takes a map like 0 1 3 4 7 and only returns the number if it is present */
/* in the map, or returns the closest, or the next up or down (wrapped)*/
#include "m_pd.h"
#include <math.h>
#include <string.h>
#define MAXENTRIES 512
#define LASTENTRY 511

static t_class *sieve_class;

/* mode = 0 : block when absent, 1: nearest when absent, 2: shunt when absent */
typedef struct _map
{
  t_atom map[MAXENTRIES];
  t_atom nomap[MAXENTRIES];
} t_map;

typedef struct _sieve
{
  t_object x_obj;
  t_map x_map;
  t_float input, mode, max, outmap;
  t_outlet *mapped, *value, *mapout, *inst;
} t_sieve;

void sieve_float(t_sieve *x, t_floatarg fin)
{
  int i, ip, in, arg, arga, argb, argaout, argbout, argxa, argxb, itest, itesta, itestb, iresult;
  itest = itesta = itestb = iresult = arga = argb = arg = 0;
  float test, testa, testb, fresult;
  test = testa = testb = fresult = 0;
  x->input = arg = fin;// < 0 ? 0 : fin > LASTENTRY ? LASTENTRY : (int)fin;
  if (x->mode == 0)
    {
      test = fin < 0 ? 0 : atom_getfloatarg(arg, MAXENTRIES, x->x_map.map);
      if(test!=0) 
	{
	  outlet_bang(x->inst);
	  outlet_float(x->value, test);
	  outlet_float(x->mapped, arg);
	}
    }
  else if (x->mode == 1)
    {
      test =  fin < 0 ? 0 : atom_getfloatarg(arg, MAXENTRIES, x->x_map.map);
      if(test!=0)
	{
	  outlet_bang(x->inst);
	  outlet_float(x->value, test);
	  outlet_float(x->mapped, arg);
	}
      else
	{
	  arga = argb = arg;
	  while(itest == 0 && (arga > -1 || argb < MAXENTRIES))
	    {
	      arga--;
	      argb++;
	      argxa = arga >= 0 ? arga : 0;
	      argxb = argb <= LASTENTRY ? argb : LASTENTRY;
	      testa = atom_getfloatarg(argxa, MAXENTRIES, x->x_map.map);
	      testb = atom_getfloatarg(argxb, MAXENTRIES, x->x_map.map);
	      itesta = testa != 0 ? 1 : 0;
	      itestb = testb != 0 ? 1 : 0;
	      itest =  fin < 0 ? 0 : itesta + itestb;
	    }
	  switch(itest)
	    {
	    case 2:
	      if (x->mode == 1)
		{
		  outlet_float(x->value, testb);
		  outlet_float(x->mapped, argb);
		}
	      else
		{
		  outlet_float(x->value, testa);
		  outlet_float(x->mapped, arga);
		}
	    case 1:
	      iresult = itesta == 1 ? arga : argb;
	      fresult = itesta == 1 ? testa : testb;
	      outlet_float(x->value, fresult);
	      outlet_float(x->mapped, iresult);
	    case 0:
	      break;
	    }
	}
    }
  else if (x->mode==2)
    {
      itest = 0;
      test =  fin < 0 ? 0 : atom_getfloatarg(arg, MAXENTRIES, x->x_map.map);
      if(test!=0)
	{
	  outlet_bang(x->inst);
	  outlet_float(x->value, test);
	  outlet_float(x->mapped, arg);
	}
      else
	{
	  arga =  arg;
	  while(itest == 0 && (x->max > 0))
	    {
	      arga = (arga + 1) <= LASTENTRY ? (arga + 1) : 0;
	      testa = atom_getfloatarg(arga, MAXENTRIES, x->x_map.map);
	      itest = testa != 0 ? 1 : 0;
	    }
	  if(x->max > 0 && fin >= 0)
	    {
	      outlet_float(x->value, testa);
	      outlet_float(x->mapped, arga);
	    }
	}
    }
  else if (x->mode == 3)
    {
      itest = 0;
      test =  fin < 0 ? 0 : atom_getfloatarg(arg, MAXENTRIES, x->x_map.map);
      if(test!=0)
	{
	  outlet_bang(x->inst);
	  outlet_float(x->value, test);
	  outlet_float(x->mapped, arg);
	}
      else
	{
	  arga =  arg;
	  while(itest == 0 && (x->max > 0))
	    {
	      argb = arga - 1;
	      arga = argb >= 0 ? argb : LASTENTRY;
	      testa = atom_getfloatarg(arga, MAXENTRIES, x->x_map.map);
	      itest = testa != 0 ? 1 : 0;
	    }
	}
      outlet_float(x->value, testa);
      outlet_float(x->mapped, arga);
    }
}

void sieve_set(t_sieve *x, t_floatarg fmap, t_floatarg fval)
{
  float fvaller;
  if(fmap < MAXENTRIES && fmap >= 0)
    {
      int imap = (int)fmap;
      fvaller = fval != 0 ? 0 : 1;
      SETFLOAT(&x->x_map.map[imap], fval);
      SETFLOAT(&x->x_map.nomap[imap], fvaller);
      x->max = fmap > x->max ? fmap : x->max;
    }
}

void sieve_get(t_sieve *x, t_floatarg inv)
{
  if(inv!=0) 
    {
      outlet_list(x->mapout, gensym("list"), x->max+1, x->x_map.nomap);
    }
  else outlet_list(x->mapout, gensym("list"), x->max+1, x->x_map.map);
  x->outmap = inv;
}

void sieve_clear(t_sieve *x)
{
  //memset(x->x_map.map, 0, MAXENTRIES);
  int i;
  for(i=0;i<MAXENTRIES;i++) 
    {
      SETFLOAT(&x->x_map.map[i], 0);
      SETFLOAT(&x->x_map.nomap[i], 1);
    }
  //memset(x->x_map.nomap, 1, MAXENTRIES);
  x->max = 0;
}

void sieve_map(t_sieve *x, t_symbol *s, int argc, t_atom *argv)
{
  //memset(x->x_map.map, 0, MAXENTRIES);
  //memset(x->x_map.nomap, 1, MAXENTRIES);
  int i;
  for(i=0;i<MAXENTRIES;i++) 
    {
      SETFLOAT(x->x_map.map+i, 0);
      SETFLOAT(x->x_map.nomap+i, 1);
    }
  x->max = 0;
  float arg;
  for(i=0;i<argc;i++) 
    {
      arg = atom_getfloat(argv+i);
      if(arg != 0)
	{
	  SETFLOAT(&x->x_map.map[i], arg);
	  SETFLOAT(&x->x_map.nomap[i], 0);
	  x->max = i;
	}
    }
  if (x->max > 0 && x->outmap == 0)
    {
      outlet_list(x->mapout, gensym("list"), x->max+1, x->x_map.map);
    }
  else if (x->max > 0 && x->outmap == 1)
    {
      outlet_list(x->mapout, gensym("list"), x->max+1, x->x_map.nomap);
    }
}

void sieve_mode(t_sieve *x, t_floatarg fmode)
{
  x->mode = fmode < 0 ? 0 : fmode > 3 ? 3 : fmode;
}

void sieve_debug(t_sieve *x)
{
  float ele0, ele1, ele2, ele3, ele4, ele5, ele6, ele7, ele8, ele9;
  float nle0, nle1, nle2, nle3, nle4, nle5, nle6, nle7, nle8, nle9;
  ele0 = atom_getfloatarg(0, MAXENTRIES, x->x_map.map);
  ele1 = atom_getfloatarg(1, MAXENTRIES, x->x_map.map);
  ele2 = atom_getfloatarg(2, MAXENTRIES, x->x_map.map);
  ele3 = atom_getfloatarg(3, MAXENTRIES, x->x_map.map);
  ele4 = atom_getfloatarg(4, MAXENTRIES, x->x_map.map);
  ele5 = atom_getfloatarg(5, MAXENTRIES, x->x_map.map);
  ele6 = atom_getfloatarg(6, MAXENTRIES, x->x_map.map);
  ele7 = atom_getfloatarg(7, MAXENTRIES, x->x_map.map);
  ele8 = atom_getfloatarg(8, MAXENTRIES, x->x_map.map);
  ele9 = atom_getfloatarg(9, MAXENTRIES, x->x_map.map);
  nle0 = atom_getfloatarg(0, MAXENTRIES, x->x_map.nomap);
  nle1 = atom_getfloatarg(1, MAXENTRIES, x->x_map.nomap);
  nle2 = atom_getfloatarg(2, MAXENTRIES, x->x_map.nomap);
  nle3 = atom_getfloatarg(3, MAXENTRIES, x->x_map.nomap);
  nle4 = atom_getfloatarg(4, MAXENTRIES, x->x_map.nomap);
  nle5 = atom_getfloatarg(5, MAXENTRIES, x->x_map.nomap);
  nle6 = atom_getfloatarg(6, MAXENTRIES, x->x_map.nomap);
  nle7 = atom_getfloatarg(7, MAXENTRIES, x->x_map.nomap);
  nle8 = atom_getfloatarg(8, MAXENTRIES, x->x_map.nomap);
  nle9 = atom_getfloatarg(9, MAXENTRIES, x->x_map.nomap);
  /*	  post("blocksize = %d, scales = %d, vectorsize = %d, offset = %d", 
	  x->N, x->scales, x->vecsize, x->offset); */
  post("mode = %d, max = %d", x->mode, x->max);
  post("first 10 elements = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", ele0, ele1, ele2, ele3, ele4, ele5, ele6, ele7, ele8, ele9);
  post("first 10 elements = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", nle0, nle1, nle2, nle3, nle4, nle5, nle6, nle7, nle8, nle9);
}  

void *sieve_new(t_floatarg f) 
{
  t_sieve *x = (t_sieve *)pd_new(sieve_class);
  x->mode = f;
  x->max = 0;
  x->outmap = 0;
  //memset(x->x_map.map, 0, MAXENTRIES);
  //memset(x->x_map.nomap, 1, MAXENTRIES);
  int i;
  for(i=0;i<MAXENTRIES;i++) 
    {
      SETFLOAT(x->x_map.map+i, 0);
      SETFLOAT(x->x_map.nomap+i, 1);
    }
  x->mapped = outlet_new(&x->x_obj, &s_float);
  x->value = outlet_new(&x->x_obj, &s_float);
  x->mapout = outlet_new(&x->x_obj, &s_list);
  x->inst = outlet_new(&x->x_obj, &s_bang);
  return (void *)x;
}

void sieve_setup(void) 
{

  sieve_class = class_new(gensym("sieve"),
  (t_newmethod)sieve_new,
  0, sizeof(t_sieve),
  0, A_DEFFLOAT, 0);
  post("|^^^^^^^^^^^^^sieve^^^^^^^^^^^^^|");
  post("|->^^^integer map to floats^^^<-|");
  post("|^^^^^^^Edward Kelly 2006^^^^^^^|");
  class_sethelpsymbol(sieve_class, gensym("help-sieve"));
  class_addfloat(sieve_class, sieve_float);
  class_addmethod(sieve_class, (t_method)sieve_set, gensym("set"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_map, gensym("map"), A_GIMME, 0);
  class_addmethod(sieve_class, (t_method)sieve_clear, gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_get, gensym("get"), A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_mode, gensym("mode"), A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_debug, gensym("debug"), A_DEFFLOAT, 0);
}
