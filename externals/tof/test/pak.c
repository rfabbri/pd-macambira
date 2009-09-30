static t_class *pak_class;
static t_class *pak_inlet_class;

typedef struct _pak
{
    t_object 			x_obj;
    t_int 				x_n;              // number of args
    t_atom 				*x_vec;          // input values
    t_int 				x_nptr;           // number of pointers
    t_gpointer 			*x_gpointer; // the pointers
    t_atom 				*x_outvec;       // space for output values
	int					count;
	struct _pak_inlet  **proxy;
	t_inlet 			**in;
} t_pak;



typedef struct _pak_inlet
{
  t_pd  		p_pd;
  t_pack    	*master;
  int 			id;
  t_symbol*		type;
} t_pak_inlet;


static void *pak_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pak *x = (t_pak *)pd_new(pak_class);
    t_atom defarg[2], *vap, *vec, *vp;
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


    // Create inlet proxys
	x->count = argc - 1;
	if (x->count < 0) x->count = 0;
	x->in = (t_inlet **)getbytes(x->count * sizeof(t_inlet *));
	x->proxy = (t_pak_inlet**)getbytes(x->count * sizeof(t_pak_inlet*));
    //

    /*
	 
	for (n = 0; n < x->count; n++) {
		x->proxy[n]=(t_pak_inlet*)pd_new(pak_inlet_class);
		x->proxy[n]->master = x;
		x->proxy[n]->id=n;
		x->in[n] = inlet_new ((t_object*)x, (t_pd*)x->proxy[n], 0,0);
	}
	 
	 */

    int n;
    for (i = 0, vp = x->x_vec, ap = argv; i < argc; i++, ap++, vp++)
    {
	  n = i-1;	
        if (ap->a_type == A_FLOAT)
        {
            *vp = *ap;
            if (i) {
				//floatinlet_new(&x->x_obj, &vp->a_w.w_float);
			x->proxy[n]=(t_pak_inlet*)pd_new(pak_inlet_class);
			x->proxy[n]->master = x;
			x->proxy[n]->id=n;
			x->proxy[n]->typr = &s_float;
			x->in[n] = inlet_new ((t_object*)x, (t_pd*)x->proxy[n], 0,0);
			}
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
                if (c != 'f') pd_error(x, "pak: %s: bad type",
                    ap->a_w.w_symbol->s_name);
                SETFLOAT(vp, 0);
                if (i) floatinlet_new(&x->x_obj, &vp->a_w.w_float);
            }
        }
    }
	
	
	 
	
	 
	 
	 
    outlet_new(&x->x_obj, &s_list);
    return (x);
}

static void pak_bang(t_pak *x)
{
    int i, reentered = 0, size = x->x_n * sizeof (t_atom);
    t_gpointer *gp;
    t_atom *outvec;
    for (i = x->x_nptr, gp = x->x_gpointer; i--; gp++)
        if (!gpointer_check(gp, 1))
    {
        pd_error(x, "pak: stale pointer");
        return;
    }
        // reentrancy protection.  The first time through use the pre-allocated
        // x_outvec; if we're reentered we have to allocate new memory.
    if (!x->x_outvec)
    {
            // LATER figure out how to deal with reentrancy and pointers... 
        if (x->x_nptr)
            post("pak_bang: warning: reentry with pointers unprotected");
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

static void pak_pointer(t_pak *x, t_gpointer *gp)
{
    if (x->x_vec->a_type == A_POINTER)
    {
        gpointer_unset(x->x_gpointer);
        *x->x_gpointer = *gp;
        if (gp->gp_stub) gp->gp_stub->gs_refcount++;
        pak_bang(x);
    }
    else pd_error(x, "pak_pointer: wrong type");
}

static void pak_float(t_pak *x, t_float f)
{
    if (x->x_vec->a_type == A_FLOAT)
    {
        x->x_vec->a_w.w_float = f;
        pak_bang(x);
    }
    else pd_error(x, "pak_float: wrong type");
}

static void pak_symbol(t_pak *x, t_symbol *s)
{
    if (x->x_vec->a_type == A_SYMBOL)
    {
        x->x_vec->a_w.w_symbol = s;
        pak_bang(x);
    }
    else pd_error(x, "pak_symbol: wrong type");
}

static void pak_list(t_pak *x, t_symbol *s, int ac, t_atom *av)
{
    obj_list(&x->x_obj, 0, ac, av);
}

static void pak_anything(t_pak *x, t_symbol *s, int ac, t_atom *av)
{
    t_atom *av2 = (t_atom *)getbytes((ac + 1) * sizeof(t_atom));
    int i;
    for (i = 0; i < ac; i++)
        av2[i + 1] = av[i];
    SETSYMBOL(av2, s);
    obj_list(&x->x_obj, 0, ac+1, av2);
    freebytes(av2, (ac + 1) * sizeof(t_atom));
}


static void pak_inlet(t_pak  *y, t_symbol *s, int argc, t_atom *argv)
{
  t_mux*x=y->p_master;
  if(y->id==x->i_selected)
    outlet_anything(x->x_obj.ob_outlet, s, argc, argv);
}


static void pak_free(t_pak *x)
{
    t_gpointer *gp;
    int i;
    for (gp = x->x_gpointer, i = x->x_nptr; i--; gp++)
        gpointer_unset(gp);
    freebytes(x->x_vec, x->x_n * sizeof(*x->x_vec));
    freebytes(x->x_outvec, x->x_n * sizeof(*x->x_outvec));
    freebytes(x->x_gpointer, x->x_nptr * sizeof(*x->x_gpointer));
	
	// Free proxys
	const int count = x->count;

	  if(x->in && x->proxy){
		int n=0;
		for(n=0; n<count; n++){
		  if(x->in[n]){
			inlet_free(x->in[n]);
		  }
		  x->in[n]=0;
		  if(x->proxy[n]){
			t_pak_inlet *y=x->proxy[n];
			y->master=0;
			y->id=0;
			pd_free(&y->p_pd);        
		  }
		  x->proxy[n]=0;      
		}
		freebytes(x->in, x->count * sizeof(t_inlet *));
		freebytes(x->proxy, x->count * sizeof(t_pak_inlet*));
	  }
	
	
}

static void pak_setup(void)
{
    pak_class = class_new(gensym("pak"), (t_newmethod)pak_new,
        (t_method)pak_free, sizeof(t_pak), 0, A_GIMME, 0);
    class_addbang(pak_class, pak_bang);
    class_addpointer(pak_class, pak_pointer);
    class_addfloat(pak_class, pak_float);
    class_addsymbol(pak_class, pak_symbol);
    class_addlist(pak_class, pak_list);
    class_addanything(pak_class, pak_anything);
	
	// Setup proxies
	
	pak_inlet_class = class_new(0, 0, 0, sizeof(t_pak_inlet),CLASS_PD | CLASS_NOINLET, 0);
	class_addanything(pak_inlet_class, pak_inlet);
	
}



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
