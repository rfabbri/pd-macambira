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
    char s[32];
    sprintf(s, "tclpd:%s:x%lx", name, objectSequentialId++);
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

void tclpd_guiclass_getrect(t_gobj* z, t_glist* owner, int* xp1, int* yp1, int* xp2, int* yp2) {
    Tcl_Obj* av[5], *o, *theList;
    int tmp[4], i, length;
    t_tcl* x = (t_tcl*)z;
    av[0] = x->self;
    av[1] = Tcl_NewStringObj("widgetbehavior", -1);
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("getrect", -1);
    Tcl_IncrRefCount(av[2]);
    av[3] = Tcl_NewIntObj(text_xpix(&x->o, owner));
    Tcl_IncrRefCount(av[3]);
    av[4] = Tcl_NewIntObj(text_xpix(&x->o, owner));
    Tcl_IncrRefCount(av[4]);
    int result = Tcl_EvalObjv(tcl_for_pd, 5, av, 0);
    if(result != TCL_OK) {
        tclpd_interp_error(result);
        goto error;
    }
    theList = Tcl_GetObjResult(tcl_for_pd);
    length = 0;
    //result = Tcl_ListObjGetElements(tcl_for_pd, theList, @, @);
    result = Tcl_ListObjLength(tcl_for_pd, theList, &length);
    if(result != TCL_OK) {
        tclpd_interp_error(result);
        goto error;
    }
    if(length != 4) {
        pd_error(x, "widgetbehavior getrect: must return a list of 4 integers");
        goto error;
    }
    o = NULL;
    for(i = 0; i < 4; i++) {
        result = Tcl_ListObjIndex(tcl_for_pd, theList, i, &o);
        if(result != TCL_OK) {
            tclpd_interp_error(result);
            goto error;
        }
        result = Tcl_GetIntFromObj(tcl_for_pd, o, &tmp[i]);
        if(result != TCL_OK) {
            tclpd_interp_error(result);
            goto error;
        }
    }
    *xp1 = tmp[0]; *yp1 = tmp[1]; *xp2 = tmp[2]; *yp2 = tmp[3];
    goto cleanup;
error:
cleanup:
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
    Tcl_DecrRefCount(av[3]);
    Tcl_DecrRefCount(av[4]);
}

void tclpd_guiclass_displace(t_gobj* z, t_glist* glist, int dx, int dy) {
    Tcl_Obj* av[5];
    t_tcl* x = (t_tcl*)z;
    av[0] = x->self;
    av[1] = Tcl_NewStringObj("widgetbehavior", -1);
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("displace", -1);
    Tcl_IncrRefCount(av[2]);
    av[3] = Tcl_NewIntObj(dx);
    Tcl_IncrRefCount(av[3]);
    av[4] = Tcl_NewIntObj(dy);
    Tcl_IncrRefCount(av[4]);
    int result = Tcl_EvalObjv(tcl_for_pd, 5, av, 0);
    if(result != TCL_OK) {
        tclpd_interp_error(result);
        goto error;
    }
error:
cleanup:
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
    Tcl_DecrRefCount(av[3]);
    Tcl_DecrRefCount(av[4]);
}

void tclpd_guiclass_select(t_gobj* z, t_glist* glist, int selected) {
    Tcl_Obj* av[4];
    t_tcl* x = (t_tcl*)z;
    av[0] = x->self;
    av[1] = Tcl_NewStringObj("widgetbehavior", -1);
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("select", -1);
    Tcl_IncrRefCount(av[2]);
    av[3] = Tcl_NewIntObj(selected);
    Tcl_IncrRefCount(av[3]);
    int result = Tcl_EvalObjv(tcl_for_pd, 5, av, 0);
    if(result != TCL_OK) {
        tclpd_interp_error(result);
        goto error;
    }
error:
cleanup:
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
    Tcl_DecrRefCount(av[3]);
}

void tclpd_guiclass_activate(t_gobj* z, t_glist* glist, int state) {
    Tcl_Obj* av[4];
    t_tcl* x = (t_tcl*)z;
    av[0] = x->self;
    av[1] = Tcl_NewStringObj("widgetbehavior", -1);
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("activate", -1);
    Tcl_IncrRefCount(av[2]);
    av[3] = Tcl_NewIntObj(state);
    Tcl_IncrRefCount(av[3]);
    int result = Tcl_EvalObjv(tcl_for_pd, 5, av, 0);
    if(result != TCL_OK) {
        tclpd_interp_error(result);
        goto error;
    }
error:
cleanup:
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
    Tcl_DecrRefCount(av[3]);
}

void tclpd_guiclass_delete(t_gobj* z, t_glist* glist) {
    /* will this be ever need to be accessed in Tcl land? */
    canvas_deletelinesfor(glist_getcanvas(glist), (t_text*)z);
}

void tclpd_guiclass_vis(t_gobj* z, t_glist* glist, int vis) {
    Tcl_Obj* av[4];
    t_tcl* x = (t_tcl*)z;
    av[0] = x->self;
    av[1] = Tcl_NewStringObj("widgetbehavior", -1);
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("vis", -1);
    Tcl_IncrRefCount(av[2]);
    av[3] = Tcl_NewIntObj(vis);
    Tcl_IncrRefCount(av[3]);
    int result = Tcl_EvalObjv(tcl_for_pd, 5, av, 0);
    if(result != TCL_OK) {
        tclpd_interp_error(result);
        goto error;
    }
error:
cleanup:
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
    Tcl_DecrRefCount(av[3]);
}

int tclpd_guiclass_click(t_gobj* z, t_glist* glist, int xpix, int ypix, int shift, int alt, int dbl, int doit) {
    Tcl_Obj* av[9], *o;
    int i = 0;
    t_tcl* x = (t_tcl*)z;
    av[0] = x->self;
    av[1] = Tcl_NewStringObj("widgetbehavior", -1);
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("click", -1);
    Tcl_IncrRefCount(av[2]);
    av[3] = Tcl_NewIntObj(xpix);
    Tcl_IncrRefCount(av[3]);
    av[4] = Tcl_NewIntObj(ypix);
    Tcl_IncrRefCount(av[4]);
    av[5] = Tcl_NewIntObj(shift);
    Tcl_IncrRefCount(av[5]);
    av[6] = Tcl_NewIntObj(alt);
    Tcl_IncrRefCount(av[6]);
    av[7] = Tcl_NewIntObj(dbl);
    Tcl_IncrRefCount(av[7]);
    av[8] = Tcl_NewIntObj(doit);
    Tcl_IncrRefCount(av[8]);
    int result = Tcl_EvalObjv(tcl_for_pd, 9, av, 0);
    if(result != TCL_OK) {
        tclpd_interp_error(result);
        goto error;
    }
    o = Tcl_GetObjResult(tcl_for_pd);
    result = Tcl_GetIntFromObj(tcl_for_pd, o, &i);
    if(result != TCL_OK) {
        tclpd_interp_error(result);
        goto error;
    }
    goto cleanup;
error:
cleanup:
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
    Tcl_DecrRefCount(av[3]);
    Tcl_DecrRefCount(av[4]);
    Tcl_DecrRefCount(av[5]);
    Tcl_DecrRefCount(av[6]);
    Tcl_DecrRefCount(av[7]);
    Tcl_DecrRefCount(av[8]);
    return i;
}
