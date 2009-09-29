
static t_class *pak_class;
static t_class *pak_inlet_class;
struct _param_inlet2;

#define PAK_MAX_INLETS 100

typedef struct _pak
{
  t_object                     x_obj;
  struct _pak_inlet		       inlets[PAK_MAX_INLETS];
  int 						   number;
} t_pak;

typedef struct _pak_inlet
{
  t_object        x_obj;
  t_pak  *p_owner;
} t_pak_inlet;





// CONSTRUCTOR
static void *param_new(t_symbol *s, int ac, t_atom *av)
{
  t_param *x = (t_param *)pd_new(param_class);
  t_param_inlet2 *p = (t_param_inlet2 *)pd_new(param_inlet2_class);
   
  // Stuff
  x->s_set = gensym("set");
  x->s_PARAM = gensym("PARAM");
  
  // Set up second inlet proxy
  x->x_param_inlet2 = p;
  p->p_owner = x;
  
 

  
  return (x);
}


void pak_setup(void)
{
  pak_class = class_new(gensym("pak"),
    (t_newmethod)pak_new, (t_method)pak_free,
    sizeof(t_param), 0, A_GIMME, 0);


  //class_addanything(param_class, param_anything);
  //class_addbang(param_class, param_bang);
  
  //class_addmethod(param_class, (t_method)param_loadbang, gensym("loadbang"), 0);
  
  pak_inlet_class = class_new(gensym("_pak_inlet"),
    0, 0, sizeof(t_pak_inlet), CLASS_PD | CLASS_NOINLET, 0);
	
  //class_addanything(pak_inlet_class, pak_inlet_anything);
  
}


///////////////// PACK ///////////////

/*
static t_class *pack_class;

typedef struct _pack
{
    t_object x_obj;
    t_int x_n;              // number of args
    t_atom *x_vec;          // input values
    t_int x_nptr;           // number of pointers
    t_gpointer *x_gpointer; // the pointers
    t_atom *x_outvec;       // space for output values
} t_pack;

static void *pack_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pack *x = (t_pack *)pd_new(pack_class);
    t_atom defarg[2], *ap, *vec, *vp;
    t_gpointer *gp;
    int nptr = 0;
    int i;
    if (!argc)
    {
        argv = defarg;
        argc = 2;
        SETFLOAT(&defarg[0], 0);
        SETFLOAT(&defarg[1], 0);
    }

    x->x_n = argc;
    vec = x->x_vec = (t_atom *)getbytes(argc * sizeof(*x->x_vec));
    x->x_outvec = (t_atom *)getbytes(argc * sizeof(*x->x_outvec));

    for (i = argc, ap = argv; i--; ap++)
        if (ap->a_type == A_SYMBOL && *ap->a_w.w_symbol->s_name == 'p')
            nptr++;

    gp = x->x_gpointer = (t_gpointer *)t_getbytes(nptr * sizeof (*gp));
    x->x_nptr = nptr;

    for (i = 0, vp = x->x_vec, ap = argv; i < argc; i++, ap++, vp++)
    {
        if (ap->a_type == A_FLOAT)
        {
            *vp = *ap;
            if (i) floatinlet_new(&x->x_obj, &vp->a_w.w_float);
        }
        else if (ap->a_type == A_SYMBOL)
        {
            char c = *ap->a_w.w_symbol->s_name;
            if (c == 's')
            {
                SETSYMBOL(vp, &s_symbol);
                if (i) symbolinlet_new(&x->x_obj, &vp->a_w.w_symbol);
            }
            else if (c == 'p')
            {
                vp->a_type = A_POINTER;
                vp->a_w.w_gpointer = gp;
                gpointer_init(gp);
                if (i) pointerinlet_new(&x->x_obj, gp);
                gp++;
            }
            else
            {
                if (c != 'f') pd_error(x, "pack: %s: bad type",
                    ap->a_w.w_symbol->s_name);
                SETFLOAT(vp, 0);
                if (i) floatinlet_new(&x->x_obj, &vp->a_w.w_float);
            }
        }
    }
    outlet_new(&x->x_obj, &s_list);
    return (x);
}

static void pack_bang(t_pack *x)
{
    int i, reentered = 0, size = x->x_n * sizeof (t_atom);
    t_gpointer *gp;
    t_atom *outvec;
    for (i = x->x_nptr, gp = x->x_gpointer; i--; gp++)
        if (!gpointer_check(gp, 1))
    {
        pd_error(x, "pack: stale pointer");
        return;
    }
        // reentrancy protection.  The first time through use the pre-allocated
        // x_outvec; if we're reentered we have to allocate new memory.
    if (!x->x_outvec)
    {
            // LATER figure out how to deal with reentrancy and pointers... 
        if (x->x_nptr)
            post("pack_bang: warning: reentry with pointers unprotected");
        outvec = t_getbytes(size);
        reentered = 1;
    }
    else
    {
        outvec = x->x_outvec;
        x->x_outvec = 0;
    }
    memcpy(outvec, x->x_vec, size);
    outlet_list(x->x_obj.ob_outlet, &s_list, x->x_n, outvec);
    if (reentered)
        t_freebytes(outvec, size);
    else x->x_outvec = outvec;
}

static void pack_pointer(t_pack *x, t_gpointer *gp)
{
    if (x->x_vec->a_type == A_POINTER)
    {
        gpointer_unset(x->x_gpointer);
        *x->x_gpointer = *gp;
        if (gp->gp_stub) gp->gp_stub->gs_refcount++;
        pack_bang(x);
    }
    else pd_error(x, "pack_pointer: wrong type");
}

static void pack_float(t_pack *x, t_float f)
{
    if (x->x_vec->a_type == A_FLOAT)
    {
        x->x_vec->a_w.w_float = f;
        pack_bang(x);
    }
    else pd_error(x, "pack_float: wrong type");
}

static void pack_symbol(t_pack *x, t_symbol *s)
{
    if (x->x_vec->a_type == A_SYMBOL)
    {
        x->x_vec->a_w.w_symbol = s;
        pack_bang(x);
    }
    else pd_error(x, "pack_symbol: wrong type");
}

static void pack_list(t_pack *x, t_symbol *s, int ac, t_atom *av)
{
    obj_list(&x->x_obj, 0, ac, av);
}

static void pack_anything(t_pack *x, t_symbol *s, int ac, t_atom *av)
{
    t_atom *av2 = (t_atom *)getbytes((ac + 1) * sizeof(t_atom));
    int i;
    for (i = 0; i < ac; i++)
        av2[i + 1] = av[i];
    SETSYMBOL(av2, s);
    obj_list(&x->x_obj, 0, ac+1, av2);
    freebytes(av2, (ac + 1) * sizeof(t_atom));
}

static void pack_free(t_pack *x)
{
    t_gpointer *gp;
    int i;
    for (gp = x->x_gpointer, i = x->x_nptr; i--; gp++)
        gpointer_unset(gp);
    freebytes(x->x_vec, x->x_n * sizeof(*x->x_vec));
    freebytes(x->x_outvec, x->x_n * sizeof(*x->x_outvec));
    freebytes(x->x_gpointer, x->x_nptr * sizeof(*x->x_gpointer));
}

static void pack_setup(void)
{
    pack_class = class_new(gensym("pack"), (t_newmethod)pack_new,
        (t_method)pack_free, sizeof(t_pack), 0, A_GIMME, 0);
    class_addbang(pack_class, pack_bang);
    class_addpointer(pack_class, pack_pointer);
    class_addfloat(pack_class, pack_float);
    class_addsymbol(pack_class, pack_symbol);
    class_addlist(pack_class, pack_list);
    class_addanything(pack_class, pack_anything);
}
*/

//////////////// MULTIPLEX //////////////


/*
 


static t_class *mux_class;
static t_class *muxproxy_class;

typedef struct _mux
{
  t_object x_obj;
  struct _muxproxy  **x_proxy;

  int i_count;
  int i_selected;
  t_inlet **in;
} t_mux;


typedef struct _muxproxy
{
  t_pd  p_pd;
  t_mux    *p_master;
  int id;
} t_muxproxy;

static void mux_select(t_mux *x, t_float f)
{
  x->i_selected=f;
}

static void mux_anything(t_muxproxy *y, t_symbol *s, int argc, t_atom *argv)
{
  t_mux*x=y->p_master;
  if(y->id==x->i_selected)
    outlet_anything(x->x_obj.ob_outlet, s, argc, argv);
}

static void *mux_new(t_symbol *s, int argc, t_atom *argv)
{
  int n = (argc < 2)?2:argc;
  t_mux *x = (t_mux *)pd_new(mux_class);

  x->i_selected=0;
  x->i_count = n;
  x->in = (t_inlet **)getbytes(x->i_count * sizeof(t_inlet *));
  x->x_proxy = (t_muxproxy**)getbytes(x->i_count * sizeof(t_muxproxy*));

  for (n = 0; n<x->i_count; n++) {
    x->x_proxy[n]=(t_muxproxy*)pd_new(muxproxy_class);
    x->x_proxy[n]->p_master = x;
    x->x_proxy[n]->id=n;
    x->in[n] = inlet_new ((t_object*)x, (t_pd*)x->x_proxy[n], 0,0);
  }

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym(""));

  outlet_new(&x->x_obj, 0);
  return (x);
}

static void mux_free(t_mux*x){
  const int count = x->i_count;

  if(x->in && x->x_proxy){
    int n=0;
    for(n=0; n<count; n++){
      if(x->in[n]){
        inlet_free(x->in[n]);
      }
      x->in[n]=0;
      if(x->x_proxy[n]){
        t_muxproxy *y=x->x_proxy[n];
        y->p_master=0;
        y->id=0;
        pd_free(&y->p_pd);        
      }
      x->x_proxy[n]=0;      
    }
    freebytes(x->in, x->i_count * sizeof(t_inlet *));
    freebytes(x->x_proxy, x->i_count * sizeof(t_muxproxy*));
  }
  
  // pd_free(&y->p_pd);
}

void multiplex_setup(void)
{
  mux_class = class_new(gensym("multiplex"), (t_newmethod)mux_new,
			(t_method)mux_free, sizeof(t_mux), CLASS_NOINLET, A_GIMME,  0);
  class_addcreator((t_newmethod)mux_new, gensym("mux"), A_GIMME, 0);
 
  class_addmethod   (mux_class, (t_method)mux_select, gensym(""), A_DEFFLOAT, 0);

  muxproxy_class = class_new(0, 0, 0,
			    sizeof(t_muxproxy),
			    CLASS_PD | CLASS_NOINLET, 0);
  class_addanything(muxproxy_class, mux_anything);


  zexy_register("multiplex");
}

void mux_setup(void)
{
  multiplex_setup();
}
*/
