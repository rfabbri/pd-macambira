#include "m_pd.h"
#include <tcl.h>

typedef struct t_tcl {
  t_object o;
  Tcl_Obj *self;
} t_tcl;

void         poststring2          (const char* s);

t_class*     tclpd_class_new      (char* name, int flags);
t_pd*        tclpd_get_instance   (const char* cereal);
t_object*    tclpd_get_object     (const char* cereal);
t_pd*        tclpd_get_object_pd  (const char* cereal);

int          pd_to_tcl            (t_atom* input, Tcl_Obj** output);
int          tcl_to_pd            (Tcl_Obj* input, t_atom* output);

extern Tcl_Interp *tcl_for_pd;

/* tcl loader */
typedef int (*loader_t)(t_canvas *canvas, char *classname);
extern "C" void sys_register_loader(loader_t loader);
extern "C" int sys_onloadlist(char *classname);
extern "C" void sys_putonloadlist(char *classname);
extern "C" void class_set_extern_dir(t_symbol *s);
extern "C" int tclpd_do_load_lib    (t_canvas *canvas, char *objectname);

