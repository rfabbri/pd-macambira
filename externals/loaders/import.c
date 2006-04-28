/*
 * This object loads libraries and libdirs from within a patch. It is the
 * first small step towards a patch-specific namespace.  Currently, it just
 * adds things to the global path.  It is a reimplementation of a similar/same
 * idea from Guenter Geiger's [using] object.   <hans@at.or.at>
 *
 * This object currently depends on the packages/patches/libdir-0.38-4.patch
 * for sys_load_lib_dir().
 */

#include "m_pd.h"
#include "s_stuff.h"

static t_class *import_class;

typedef struct _import
{
    t_object    x_obj;
    t_atom*     loaded_libs;
    t_atom*     current;
    t_int       num_libs;
} t_import;


static int import_load_lib(char *libname)
{
    if (!sys_load_lib(sys_libdir->s_name, libname))
        if (!sys_load_lib_dir(sys_libdir->s_name, libname)) 
        {
            post("%s: can't load library in %s", libname, sys_libdir->s_name);
            return 0;
        }
    return 1;
}


static void import_load_arguments(t_import *x, int argc, t_atom *argv)
{
    t_symbol *libname;
    
    while (argc--) {
        switch (argv->a_type) {
        case A_FLOAT:
            post("[import] ERROR: floats not supported yet: %f",atom_getfloat(argv));
            break;
        case A_SYMBOL:
            libname = atom_getsymbol(argv);
        
            if (import_load_lib(libname->s_name))
            {
                x->loaded_libs = copybytes(libname, sizeof(t_atom));
                x->current = x->loaded_libs;
                x->num_libs++;
            }
            post("[import] loaded library: %s",libname->s_name);
            break;
        default:
            post("[import] ERROR: Unsupported atom type");
        }
        argv++;
    }
}


static void import_bang(t_import *x)
{
    t_int i = x->num_libs;
    t_atom* libs_list = x->loaded_libs;
    
    post("[import]: %d libs loaded.",x->num_libs);
    while(i--)
    {
        startpost(" %s",(atom_getsymbol(libs_list))->s_name);
        libs_list++;
    }
    endpost();
}


static void *import_new(t_symbol *s, int argc, t_atom *argv)
{
    t_import *x = (t_import *)pd_new(import_class);
    x->loaded_libs = 0;
    x->num_libs = 0;
    import_load_arguments(x,argc,argv);
    return (x);
}


static void import_free(t_import *x)
{

  if (x->loaded_libs) {
    freebytes(x->loaded_libs, x->num_libs * sizeof(t_atom));
    x->loaded_libs = 0;
    x->num_libs = 0;
  }

}


void import_setup(void)
{
    import_class = class_new(gensym("import"), (t_newmethod)import_new,
                             (t_method)import_free,
                             sizeof(t_import), 
                             CLASS_DEFAULT, 
                             A_GIMME, 
                             0);
    class_addbang    (import_class, import_bang);
}
