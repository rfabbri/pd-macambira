/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* sampling */

#include "iem16_table.h"
static void table16_const(t_table16*x, t_float f);

static void *table16_new(t_symbol *s, t_float f){
  t_table16 *x = (t_table16*)pd_new(table16_class);
  int i=f;
  if(i<1)i=100;
  x->x_tablename=s;
  x->x_size=i;
  x->x_table=getbytes(x->x_size*sizeof(t_iem16_16bit));
  x->x_usedindsp=0;
  pd_bind(&x->x_obj.ob_pd, x->x_tablename);

  table16_const(x, 0);
  return(x);
}

static void table16_free(t_table16 *x){
  if(x->x_table)freebytes(x->x_table, x->x_size*sizeof(t_iem16_16bit));
  pd_unbind(&x->x_obj.ob_pd, x->x_tablename);
}

int table16_getarray16(t_table16*x, int*size,t_iem16_16bit**vec){
  *size=x->x_size;
  *vec =x->x_table;
  return 1;
}
void table16_usedindsp(t_table16*x){
  x->x_usedindsp=1;
}
static void table16_resize(t_table16*x, t_float f){
  int i=f;
  int was=x->x_size;
  if (i<1){
    error("can only resize to sizes >0");
    return;
  }
  x->x_table=resizebytes(x->x_table, was*sizeof(t_iem16_16bit), i*sizeof(t_iem16_16bit));
  if(i>was)memset(x->x_table+was, 0, (i-was)*sizeof(t_iem16_16bit));
  x->x_size  =i;
  if (x->x_usedindsp) canvas_update_dsp();
}

static void table16_const(t_table16*x, t_float f){
  t_iem16_16bit s = (t_iem16_16bit)f;
  int i = x->x_size;
  t_iem16_16bit*buf=x->x_table;
  while(i--)*buf++=s;
}


static void table16_from(t_table16*x, t_symbol*s, int argc, t_atom*argv){
  float scale=IEM16_SCALE_UP;
  int resize=0;
  int startfrom=0, startto=0, endfrom=0, endto=x->x_size;
  t_garray *a=0;
  int npoints;
  t_float *vec, *src;
  t_iem16_16bit   *dest;

  int i,length=0;

  if(argc<1 || argv->a_type!=A_SYMBOL){
    error("you have to specify the from-table !");
    return;
  }
  s=atom_getsymbol(argv);  argc--;argv++;
  if (!(a = (t_garray *)pd_findbyclass(s, garray_class))){
    error("%s: no such array", s->s_name);
    return;
  } else if (!garray_getfloatarray(a, &npoints, &vec)){
    error("%s: bad template for tabread4", s->s_name);
    return;
  }

  if(argc>0 && atom_getsymbol(argv+argc-1)==gensym("resize")){
    resize=1;
    argc--;
  }
  endfrom=npoints;

  switch(argc){
  case 0:break;
  case 4:
    endto    =atom_getfloat(argv+3);
  case 3:
    startto  =atom_getfloat(argv+2);
  case 2:
    endfrom  =atom_getfloat(argv+1);
  case 1:
    startfrom=atom_getfloat(argv);
    break;
  default:
    error("table16: from <tablename> [<startfrom> [<endfrom> [<startto> [<endto>]]]] [resize]");
    return;
  }
  if(startfrom<0)startfrom=0;
  if  (startto<0)startto=0;
  if(endfrom<=startfrom)return;
  if(endto  <=startto)  return;

  length=endfrom-startfrom;
  if(resize){
    if(x->x_size < (startto+length))table16_resize(x, startto+length);    
  } else{
    if(x->x_size < (startto+length))length=x->x_size-startto;
  }
  endfrom=startfrom+length;
  endto  =startto+length;

  dest=x->x_table+startto;
  src =vec+startfrom;
  i=length;
  while(i--)*dest++=(*src++)*scale;
  post("from %s (%d, %d) --> (%d, %d)\tresize=%s", s->s_name, startfrom, endfrom, startto, endto, (resize)?"yes":"no");
}

 
static void table16_setup(void){
  table16_class = class_new(gensym("table16"),
			    (t_newmethod)table16_new, (t_method)table16_free,
			    sizeof(t_table16), 0, A_DEFSYM, A_DEFFLOAT, 0);
  class_addmethod(table16_class, (t_method)table16_resize, gensym("resize"), A_DEFFLOAT);
  class_addmethod(table16_class, (t_method)table16_const, gensym("const"), A_DEFFLOAT);
  class_addmethod(table16_class, (t_method)table16_from, gensym("from"), A_GIMME);
}


void iem16_table_setup(void)
{
    table16_setup();
}

