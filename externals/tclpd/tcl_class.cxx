#include "tcl_extras.h"
#include <map>
#include <string>
#include <string.h>

using namespace std;

static long objectSequentialId = 0;
map<string,t_class*> class_table;
map<string,t_pd*> object_table;

/* set up the class that handles loading of tcl classes */
t_class* tclpd_class_new(char *name, int flags) {
    t_class *c = class_new(gensym(name), (t_newmethod)tclpd_new,
        (t_method)tclpd_free, sizeof(t_tcl), flags, A_GIMME, A_NULL);
    class_table[string(name)] = c;
    class_addanything(c, tclpd_anything);
    return c;
}

t_tcl* tclpd_new(t_symbol *classsym, int ac, t_atom *at) {
    const char* name = classsym->s_name;
    t_class* qlass = class_table[string(name)];
    t_tcl* self = (t_tcl*)pd_new(qlass);
    self->ninlets = 1 /* qlass->c_firstin ??? */;
    char s[32];
    sprintf(s, "pd%06lx", objectSequentialId++);
    self->self = Tcl_NewStringObj(s, -1);
    Tcl_IncrRefCount(self->self);
    object_table[string(s)] = (t_pd*)self;
    Tcl_Obj *av[ac+2];
    av[0] = Tcl_NewStringObj(name, -1);
    Tcl_IncrRefCount(av[0]);
    av[1] = self->self;
    for(int i=0; i<ac; i++) {
        if(pd_to_tcl(&at[i], &av[2+i]) == TCL_ERROR) {
            //post("tcl error: %s\n", Tcl_GetStringResult(tcl_for_pd));
            tclpd_interp_error(TCL_ERROR);
            pd_free((t_pd *)self);
            return 0;
        }
    }
    if(Tcl_EvalObjv(tcl_for_pd,ac+2,av,0) != TCL_OK) {
        //post("tcl error: %s\n", Tcl_GetStringResult(tcl_for_pd));
        tclpd_interp_error(TCL_ERROR);
        pd_free((t_pd *)self);
        return 0;
    }
    return self;
}

void tclpd_free(t_tcl *self) {
#ifdef DEBUG
    post("tclpd_free called");
#endif
}

void tclpd_anything(t_tcl *self, t_symbol *s, int ac, t_atom *at) {
    tclpd_inlet_anything(self, 0, s, ac, at);
}

void tclpd_inlet_anything(t_tcl *self, int inlet, t_symbol *s, int ac, t_atom *at) {
/* proxy method */
    Tcl_Obj *av[ac+2];
    av[0] = self->self;
    av[1] = Tcl_NewIntObj(inlet);
    Tcl_AppendToObj(av[1],"_",1);
    Tcl_AppendToObj(av[1],s->s_name,strlen(s->s_name)); // selector
    Tcl_IncrRefCount(av[1]);
    for(int i=0; i<ac; i++) {
        if(pd_to_tcl(&at[i], &av[2+i]) == TCL_ERROR) {
            tclpd_interp_error(TCL_ERROR);
            return;
        }
    }
    int result = Tcl_EvalObjv(tcl_for_pd,ac+2,av,0);
    Tcl_DecrRefCount(av[1]);
    if (result != TCL_OK) {
        tclpd_interp_error(TCL_ERROR);
    }
}

/* Tcl glue: */

t_proxyinlet* tclpd_add_proxyinlet(t_tcl* x) {
    t_proxyinlet* proxy = (t_proxyinlet*)pd_new(proxyinlet_class);
    proxyinlet_init(proxy);
    proxy->target = x;
    proxy->ninlet = x->ninlets++;
    inlet_new(&x->o, &proxy->obj.ob_pd, 0, 0);
    return proxy;
}

t_tcl* tclpd_get_instance(const char* objectSequentialId) {
    return (t_tcl*)object_table[objectSequentialId];
}

t_object* tclpd_get_object(const char* objectSequentialId) {
    t_tcl* x = tclpd_get_instance(objectSequentialId);
    return &x->o;
}

t_pd* tclpd_get_object_pd(const char* objectSequentialId) {
    t_object* o = tclpd_get_object(objectSequentialId);
    return &o->ob_pd;
}

void poststring2 (const char *s) {
    post("%s", s);
}
