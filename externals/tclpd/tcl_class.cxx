#include "tcl_extras.h"
#include <map>
#include <string>
#include <string.h>

using namespace std;

static unsigned long objectSequentialId = 0;
map<string,t_class*> class_table;
map<string,t_pd*> object_table;

/* set up the class that handles loading of tcl classes */
t_class* tclpd_class_new(const char* name, int flags) {
    t_class* c = class_new(gensym(name), (t_newmethod)tclpd_new,
        (t_method)tclpd_free, sizeof(t_tcl), flags, A_GIMME, A_NULL);

    class_table[string(name)] = c;
    class_addanything(c, tclpd_anything);

    class_setsavefn(c, tclpd_save);

    return c;
}

t_class* tclpd_guiclass_new(const char* name, int flags) {
    t_class* c = tclpd_class_new(name, flags);
    t_widgetbehavior* wb = (t_widgetbehavior*)getbytes(sizeof(t_widgetbehavior));
    wb->w_getrectfn = tclpd_guiclass_getrect;
    wb->w_displacefn = tclpd_guiclass_displace;
    wb->w_selectfn = tclpd_guiclass_select;
    wb->w_activatefn = NULL;
    wb->w_deletefn = tclpd_guiclass_delete;
    wb->w_visfn = tclpd_guiclass_vis;
    wb->w_clickfn = tclpd_guiclass_click;
    class_setwidget(c, wb);
    return c;
}

t_tcl* tclpd_new(t_symbol* classsym, int ac, t_atom* at) {
    const char* name = classsym->s_name;
    t_class* qlass = class_table[string(name)];
    t_tcl* self = (t_tcl*)pd_new(qlass);
    self->ninlets = 1 /* qlass->c_firstin ??? */;
    char s[64];
    snprintf(s, 64, "tclpd:%s:x%lx", name, objectSequentialId++);
    self->self = Tcl_NewStringObj(s, -1);
    Tcl_IncrRefCount(self->self);
    object_table[string(s)] = (t_pd*)self;
    Tcl_Obj *av[ac+2];
    av[0] = Tcl_NewStringObj(name, -1);
    Tcl_IncrRefCount(av[0]);
    av[1] = self->self;
    for(int i=0; i<ac; i++) {
        if(pd_to_tcl(&at[i], &av[2+i]) == TCL_ERROR) {
            tclpd_interp_error(TCL_ERROR);
            pd_free((t_pd*)self);
            return 0;
        }
    }
    if(Tcl_EvalObjv(tcl_for_pd, ac+2, av, 0) != TCL_OK) {
        tclpd_interp_error(TCL_ERROR);
        pd_free((t_pd*)self);
        return 0;
    }
    return self;
}

void tclpd_free(t_tcl* self) {
#ifdef DEBUG
    post("tclpd_free called");
#endif
}

void tclpd_anything(t_tcl* self, t_symbol* s, int ac, t_atom* at) {
    tclpd_inlet_anything(self, 0, s, ac, at);
}

void tclpd_inlet_anything(t_tcl* self, int inlet, t_symbol* s, int ac, t_atom* at) {
/* proxy method */
    Tcl_Obj* av[ac+3];
    av[0] = self->self;
    av[1] = Tcl_NewIntObj(inlet);
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj(s->s_name, -1);
    Tcl_IncrRefCount(av[2]);
    for(int i=0; i<ac; i++) {
        if(pd_to_tcl(&at[i], &av[3+i]) == TCL_ERROR) {
            tclpd_interp_error(TCL_ERROR);
            return;
        }
    }
    int result = Tcl_EvalObjv(tcl_for_pd, ac+3, av, 0);
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
    if(result != TCL_OK) {
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

void tclpd_save(t_gobj* z, t_binbuf* b) {
    Tcl_Obj* av[3], *res;
    t_tcl* x = (t_tcl*)z;
    av[0] = x->self;
    av[1] = Tcl_NewStringObj("object", -1);
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("save", -1);
    Tcl_IncrRefCount(av[2]);
    int result = Tcl_EvalObjv(tcl_for_pd, 3, av, 0);
    if(result == TCL_OK) {
        res = Tcl_GetObjResult(tcl_for_pd);
        Tcl_IncrRefCount(res);
        int objc;
        Tcl_Obj** objv;
        result = Tcl_ListObjGetElements(tcl_for_pd, res, &objc, &objv);
        if(result == TCL_OK) {
            if(objc == 0 && objv == NULL) {
                // call default savefn
                text_save(z, b);
            } else {
                // do custom savefn
                int i;
                double tmp;
                for(i = 0; i < objc; i++) {
                    result = Tcl_GetDoubleFromObj(tcl_for_pd, objv[i], &tmp);
                    if(result == TCL_OK) {
                        binbuf_addv(b, "f", (t_float)tmp);
                    } else {
                        char* tmps = Tcl_GetStringFromObj(objv[i], NULL);
                        if(!strcmp(tmps, ";")) {
                            binbuf_addv(b, ";");
                        } else {
                            binbuf_addv(b, "s", gensym(tmps));
                        }
                    }
                }
            }
        } else {
            pd_error(x, "Tcl: object save: failed");
            tclpd_interp_error(result);
        }
        Tcl_DecrRefCount(res);
    } else {
        pd_error(x, "Tcl: object save: failed");
        tclpd_interp_error(result);
    }
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
}
