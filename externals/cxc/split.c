#include <m_pd.h>
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* this is taken from ggee, where the file was hanging around but the object was not
   funtional. i keep it here for reference since i dont wnat to fix it over and over ;)
   but its not included in the makefile to avoid namespace clash with ggee.prepend
   anyhow, i ll just rename it cxc.prepend
*/

/* ------------------------ split ----------------------------- */

static t_class *split_class;


typedef struct _split
{
     t_object x_obj;
     t_symbol* x_splitter;
} t_split;


void split_symbol(t_split *x, t_symbol *s)
{
  t_atom* a;
  split_anything(x, s, 0, a);
}

void split_anything(t_split *x,t_symbol* s,t_int argc,t_atom* argv)
{
     int i = argc; int j;
     t_symbol* cur;
     t_atom a_out[256];
     int    c_out = 0;
     t_atom* a = a_out;
     char u[MAXPDSTRING]; char v[MAXPDSTRING];
     u[0] = '\0';
     v[0] = '\0';
     int isnum = 1;

     for(j=0; j<strlen(s->s_name); j++) {
       u[0] = s->s_name[j];
       if(u[0] == x->x_splitter->s_name[0]) {
	 // post("found split: %d", v);
	 post("found split: %d", (int)v);
/* 	 if(isnum) */
/* 	   SETFLOAT(a, (int)v); */
/* 	 else */
	   SETSYMBOL(a, gensym(v));
	 a++; c_out++;
	 // reset stuff
	 v[0] = '\0';
	 isnum = 1;
       } else {
	 if(!(48 <= u[0] <= 57)) {
	   isnum = 0;
	 }
	 strncat(v, u, 1);
       }
     }
     // post("found split: %s", v);
     SETSYMBOL(a, gensym(v));
     a++, c_out++;

/* #if 1 */
/*      //     post("sym: %s",s->s_name); */
/*      SETSYMBOL(a,s); */
/*      a++; */
/*      c_out++; */
/* #endif */

/*      while (i--) { */
/*        switch( argv->a_type) { */
/*        case A_FLOAT: */
/* 	 //	 post("flo: %f",atom_getfloat(argv)); */
/* 	 SETFLOAT(a,atom_getfloat(argv)); */
/* 	 a++; */
/* 	 c_out++; */
/* 	 break; */
/*        case A_SYMBOL: */
/* 	 //	 post("sym: %s",atom_getsymbol(argv)->s_name); */
/* 	 SETSYMBOL(a,atom_getsymbol(argv)); */
/* 	 a++; */
/* 	 c_out++; */
/* 	 break; */
/*        default: */
/* 	 post("split.c: unknown type"); */
/*        } */
/*        argv++; */
/*      } */
     
       outlet_list(x->x_obj.ob_outlet, &s_list, c_out, (t_atom*)&a_out);
       // outlet_anything(x->x_obj.ob_outlet,gensym("list"),c_out,(t_atom*)&a_out);
/*      //post("done"); */
}

void split_list(t_split *x,t_symbol* s,t_int argc,t_atom* argv)
{
     int i = argc;
     t_symbol* cur;
     t_atom a_out[256];
     int    c_out = 0;
     t_atom* a = a_out;

     while (i--) {
       switch( argv->a_type) {
       case A_FLOAT:
	 //	 post("flo: %f",atom_getfloat(argv));
	 SETFLOAT(a,atom_getfloat(argv));
	 a++;
	 c_out++;
	 break;
       case A_SYMBOL:
	 //	 post("sym: %s",atom_getsymbol(argv)->s_name);
	 SETSYMBOL(a,atom_getsymbol(argv));
	 a++;
	 c_out++;
	 break;
       default:
	 post("split.c: unknown type");
       }
       argv++;
     }
     
     outlet_anything(x->x_obj.ob_outlet,x->x_splitter,c_out,(t_atom*)&a_out);
     //post("done");
}

static void *split_new(t_symbol* s)
{
    t_split *x = (t_split *)pd_new(split_class);
    outlet_new(&x->x_obj, &s_float);
    if (s != &s_)
	 x->x_splitter = s;
    else
	 x->x_splitter = gensym("cxc.split");
    return (x);
}

static void split_set(t_split *x, t_symbol *s)
{
  t_symbol *t;
  // init temp splitter
  char u[1]; u[0] = '\0';

  if(strlen(s->s_name) > 1) {
    // t = gensym((char*)s->s_name[0]);
    // post("%d", s->s_name[0]);
    strncat(u, s->s_name, 1);
    t = gensym(u);
  } else 
    t = s;
  x->x_splitter = t;
}

void split_setup(void)
{
    split_class = class_new(gensym("cxc.split"), (t_newmethod)split_new, 0,
				sizeof(t_split), 0,A_DEFSYM,NULL);
    class_addlist(split_class, split_list);
    class_addanything(split_class,split_anything);
    class_addmethod(split_class, (t_method)split_set, gensym("set"), A_SYMBOL, 0);
    class_addsymbol(split_class, split_symbol);
}
