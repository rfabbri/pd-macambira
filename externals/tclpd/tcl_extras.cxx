#include "tcl_extras.h"
#include <map>
#include <string>

using namespace std;

static long cereal=0;
map<string,t_class*> class_table;
map<string,t_pd*> object_table;

void poststring2 (const char *s) {post("%s",s);}

static void *tclpd_init (t_symbol *classsym, int ac, t_atom *at) {
  const char *name = classsym->s_name;
  t_class *qlass = class_table[string(name)];
  t_tcl *self = (t_tcl *)pd_new(qlass);
  char s[32];
  sprintf(s,"pd%06lx",cereal++);
  self->self = Tcl_NewStringObj(s,strlen(s));
  object_table[string(s)] = (t_pd*)self;
  Tcl_IncrRefCount(self->self);
  Tcl_Obj *av[ac+2];
  av[0] = Tcl_NewStringObj(name,strlen(name));
  av[1] = self->self;
  for(int i=0; i<ac; i++) {
    if(pd_to_tcl(&at[i], &av[2+i]) == TCL_ERROR) {
      post("tcl error: %s\n", Tcl_GetString(Tcl_GetObjResult(tcl_for_pd)));
      pd_free((t_pd *)self);
      return 0;
    }
  }
  if (Tcl_EvalObjv(tcl_for_pd,ac+2,av,0) != TCL_OK) {
    post("tcl error: %s\n", Tcl_GetString(Tcl_GetObjResult(tcl_for_pd)));
    pd_free((t_pd *)self);
    return 0;
  }
  return self;
}

t_pd* tclpd_get_instance(const char* cereal) {
  return object_table[cereal];
}

t_object* tclpd_get_object(const char* cereal) {
  t_tcl* x = (t_tcl*)tclpd_get_instance(cereal);
  return &x->o;
}

t_pd* tclpd_get_object_pd(const char* cereal) {
  t_object* o = tclpd_get_object(cereal);
  return &o->ob_pd;
}

static void tclpd_anything (t_tcl *self, t_symbol *s, int ac, t_atom *at) {
  /* proxy method */
  Tcl_Obj *av[ac+2];
  av[0] = self->self;
  av[1] = Tcl_NewIntObj(0); // TODO: 0 -> outlet_number
  Tcl_AppendToObj(av[1],"_",1);
  Tcl_AppendToObj(av[1],s->s_name,strlen(s->s_name)); // selector
  for(int i=0; i<ac; i++) {
    if(pd_to_tcl(&at[i], &av[2+i]) == TCL_ERROR) {
      post("tcl error: %s\n", Tcl_GetString(Tcl_GetObjResult(tcl_for_pd)));
      return;
    }
  }
  if (Tcl_EvalObjv(tcl_for_pd,ac+2,av,0) != TCL_OK)
    post("tcl error: %s\n", Tcl_GetString(Tcl_GetObjResult(tcl_for_pd)));
}

static void tclpd_free (t_tcl *self) {
  post("tclpd_free called");
}

t_class *tclpd_class_new (char *name, int flags) {
  t_class *qlass = class_new(gensym(name), (t_newmethod) tclpd_init,
    (t_method) tclpd_free, sizeof(t_tcl), flags, A_GIMME, A_NULL);
  class_table[string(name)] = qlass;
  class_addanything(qlass,tclpd_anything);
  return qlass;
}
