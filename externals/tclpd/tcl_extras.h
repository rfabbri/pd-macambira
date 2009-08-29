#include "m_pd.h"
#include <tcl.h>

/* PATH_MAX is not defined in limits.h on some platforms */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct _t_tcl {
    t_object o;
    Tcl_Obj *self;
} t_tcl;

typedef struct _t_proxyinlet {
    t_pd pd;
    int argc;
    t_atom* argv;
} t_proxyinlet;

/* tcl_proxyinlet.cxx */
void proxyinlet_init(t_proxyinlet* x);
void proxyinlet_clear(t_proxyinlet* x);
void proxyinlet_list(t_proxyinlet* x, t_symbol* s, int argc, t_atom* argv);
void proxyinlet_anything(t_proxyinlet* x, t_symbol* s, int argc, t_atom* argv);
void proxyinlet_clone(t_proxyinlet* x, t_proxyinlet* y);
void proxyinlet_setup(void);

/* tcl_wrap.cxx */
extern "C" int Tclpd_SafeInit(Tcl_Interp *interp);

/* tcl_typemap.cxx */
int          pd_to_tcl            (t_atom* input, Tcl_Obj** output);
int          tcl_to_pd            (Tcl_Obj* input, t_atom* output);

/* tcl_setup.cxx */
extern Tcl_Interp *tcl_for_pd;
extern "C" void tclpd_setup(void);
void tclpd_interp_error(int result);

/* tcl_class.cxx */
t_class* tclpd_class_new(char *name, int flags);
t_tcl* tclpd_new(t_symbol *classsym, int ac, t_atom *at);
void tclpd_free (t_tcl *self);
void tclpd_anything(t_tcl *self, t_symbol *s, int ac, t_atom *at);
t_pd* tclpd_get_instance(const char* objectSequentialId);
t_object* tclpd_get_object(const char* objectSequentialId);
t_pd* tclpd_get_object_pd(const char* objectSequentialId);
void poststring2 (const char *s);

/* tcl_loader.cxx */
extern "C" int tclpd_do_load_lib (t_canvas *canvas, char *objectname);
/* pd loader private stuff: */
typedef int (*loader_t)(t_canvas *canvas, char *classname);
extern "C" void sys_register_loader(loader_t loader);
extern "C" int sys_onloadlist(char *classname);
extern "C" void sys_putonloadlist(char *classname);
extern "C" void class_set_extern_dir(t_symbol *s);
