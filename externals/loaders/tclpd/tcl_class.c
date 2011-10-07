#include "tcl_extras.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CLASS_TABLE_SIZE (1 << 7)
#define OBJECT_TABLE_SIZE (1 << 8)

static inline uint32_t hash_str(const char *s)
{
	const unsigned char *p = (const unsigned char *)s;
	uint32_t h = 5381;

	while (*p) {
		h *= 33;
		h ^= *p++;
	}

	return h ^ (h >> 16);
}

typedef struct list_node
{
	const char* k;
	void* v;
	struct list_node* next;
} list_node_t;

static inline list_node_t* list_add(list_node_t* head, const char* k, void* v)
{
	list_node_t* n = (list_node_t*)malloc(sizeof(list_node_t));
	n->next = head;
	n->k = strdup(k);
	n->v = v;
	return n;
}

static inline list_node_t* list_remove(list_node_t* head, const char* k)
{
	list_node_t* tmp;

	// head remove
	while(head && strcmp(head->k, k) == 0)
	{
		tmp = head;
		head = head->next;
		free(tmp->k);
		free(tmp);
	}

	list_node_t* p = head;

	// normal (non-head) remove
	while(p->next)
	{
		if(strcmp(p->next->k, k) == 0)
		{
			tmp = p->next;
			p->next = p->next->next;
			free(tmp);
			continue;
		}
		p = p->next;
	}

	return head;
}

static inline void* list_get(list_node_t* head, const char* k)
{
	while(head)
	{
		if(strcmp(head->k, k) == 0)
		{
			return head->v;
		}
		head = head->next;
	}
#ifdef DEBUG
	debug_print("list_get(%lx, %s) = NULL\n", head, k);
#endif
	return (void*)0;
}

static list_node_t* class_tbl[CLASS_TABLE_SIZE];
static list_node_t* object_tbl[OBJECT_TABLE_SIZE];

static inline void class_table_add(const char* name, t_class* c)
{
	uint32_t h = hash_str(name) % CLASS_TABLE_SIZE;
	class_tbl[h] = list_add(class_tbl[h], name, (void*)c);
}

static inline void class_table_remove(const char* name)
{
	uint32_t h = hash_str(name) % CLASS_TABLE_SIZE;
	class_tbl[h] = list_remove(class_tbl[h], name);
}

static inline t_class* class_table_get(const char* name)
{
	uint32_t h = hash_str(name) % CLASS_TABLE_SIZE;
	return (t_class*)list_get(class_tbl[h], name);
}

static inline void object_table_add(const char* name, t_pd* o)
{
	uint32_t h = hash_str(name) % OBJECT_TABLE_SIZE;
	object_tbl[h] = list_add(object_tbl[h], name, (void*)o);
}

static inline void object_table_remove(const char* name)
{
	uint32_t h = hash_str(name) % OBJECT_TABLE_SIZE;
	object_tbl[h] = list_remove(object_tbl[h], name);
}

static inline t_pd* object_table_get(const char* name)
{
	uint32_t h = hash_str(name) % OBJECT_TABLE_SIZE;
	return (t_pd*)list_get(object_tbl[h], name);
}

static unsigned long objectSequentialId = 0;

static list_node_t* class_tbl[CLASS_TABLE_SIZE];
static list_node_t* object_tbl[OBJECT_TABLE_SIZE];

/* set up the class that handles loading of tcl classes */
t_class* tclpd_class_new(const char* name, int flags) {
    t_class* c = class_new(gensym(name), (t_newmethod)tclpd_new,
        (t_method)tclpd_free, sizeof(t_tcl), flags, A_GIMME, A_NULL);

    if(!class_table_get(name))
	    class_table_add(name, c);

    class_addanything(c, tclpd_anything);
    class_addmethod(c, (t_method)tclpd_loadbang, gensym("loadbang"), A_NULL);
    
    // always set save function. it will call the default if
    // none exists in tcl space.
    class_setsavefn(c, tclpd_save);

    // check if properties function exists in tcl space.
    char buf[80];
    int res_i;
    snprintf(buf, 80, "llength [info procs ::%s_object_properties]", name);
    if(Tcl_Eval(tcl_for_pd, buf) == TCL_OK) {
        Tcl_Obj* res = Tcl_GetObjResult(tcl_for_pd);
        if(Tcl_GetIntFromObj(tcl_for_pd, res, &res_i) == TCL_OK) {
            if(res_i) {
                class_setpropertiesfn(c, tclpd_properties);
            }
#ifdef DEBUG
            else {
                post("tclpd_class_new: propertiesfn does not exist", buf);
            }
#endif
        }
#ifdef DEBUG
        else {
            post("tclpd_class_new: Tcl_GetIntFromObj returned an error");
        }
#endif
    }
#ifdef DEBUG
    else {
        post("tclpd_class_new: [info procs] returned an error");
    }
#endif
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
    // lookup in class table
    const char* name = classsym->s_name;
    t_class* qlass = class_table_get(name);

    t_tcl* x = (t_tcl*)pd_new(qlass);
    x->ninlets = 1 /* qlass->c_firstin ??? */;

    x->classname = Tcl_NewStringObj(name, -1);
    Tcl_IncrRefCount(x->classname);

    char s[64];
    snprintf(s, 64, "tclpd:%s:x%lx", name, objectSequentialId++);
    x->self = Tcl_NewStringObj(s, -1);
    Tcl_IncrRefCount(x->self);

    x->x_glist = (t_glist*)canvas_getcurrent();

    // store in object table (for later lookup)
    if(!object_table_get(s))
	    object_table_add(s, x);

    // build constructor command
    Tcl_Obj *av[ac+2]; InitArray(av, ac+2, NULL);
    av[0] = x->classname;
    Tcl_IncrRefCount(av[0]);
    av[1] = x->self;
    Tcl_IncrRefCount(av[1]);
    for(int i=0; i<ac; i++) {
        if(pd_to_tcl(&at[i], &av[2+i]) == TCL_ERROR) {
#ifdef DEBUG
            post("pd_to_tcl: tclpd_new: failed during conversion. check memory leaks!");
#endif
            goto error;
        }
    }
    // call constructor
    if(Tcl_EvalObjv(tcl_for_pd, ac+2, av, 0) != TCL_OK) {
        goto error;
    }

    for(int i = 0; i < (ac+2); i++)
        Tcl_DecrRefCount(av[i]);

    return x;

error:
    tclpd_interp_error(TCL_ERROR);
    for(int i = 0; i < (ac+2); i++) {
        if(!av[i]) break;
        Tcl_DecrRefCount(av[i]);
    }
    pd_free((t_pd*)x);
    return 0;
}

void tclpd_free(t_tcl* x) {
    // build destructor command
    Tcl_Obj *sym = Tcl_NewStringObj(Tcl_GetStringFromObj(x->classname, NULL), -1);
    Tcl_AppendToObj(sym, "_destructor", -1);
    Tcl_Obj *av[2]; InitArray(av, 2, NULL);
    av[0] = sym;
    Tcl_IncrRefCount(av[0]);
    av[1] = x->self;
    Tcl_IncrRefCount(av[1]);
    // call destructor
    if(Tcl_EvalObjv(tcl_for_pd, 2, av, 0) != TCL_OK) {
#ifdef DEBUG
        post("tclpd_free: failed");
#endif
    }
    Tcl_DecrRefCount(av[0]);
    Tcl_DecrRefCount(av[1]);

    Tcl_DecrRefCount(x->self);
    Tcl_DecrRefCount(x->classname);
#ifdef DEBUG
    post("tclpd_free called");
#endif
}

void tclpd_anything(t_tcl* x, t_symbol* s, int ac, t_atom* at) {
    tclpd_inlet_anything(x, 0, s, ac, at);
}

void tclpd_inlet_anything(t_tcl* x, int inlet, t_symbol* s, int ac, t_atom* at) {
    // proxy method - format: <self> <inlet#> <selector> ...
    Tcl_Obj* av[ac+3]; InitArray(av, ac+3, NULL);
    int result;

    av[0] = x->self;
    Tcl_IncrRefCount(av[0]);
    av[1] = Tcl_NewIntObj(inlet);
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj(s->s_name, -1);
    Tcl_IncrRefCount(av[2]);
    for(int i=0; i<ac; i++) {
        if(pd_to_tcl(&at[i], &av[3+i]) == TCL_ERROR) {
#ifdef DEBUG
            post("pd_to_tcl: tclpd_inlet_anything: failed during conversion. check memory leaks!");
#endif
            goto error;
        }
    }
    result = Tcl_EvalObjv(tcl_for_pd, ac+3, av, 0);
    if(result != TCL_OK) {
        goto error;
    }

    for(int i=0; i < (ac+3); i++)
        Tcl_DecrRefCount(av[i]);

    // OK
    return;

error:
    tclpd_interp_error(TCL_ERROR);
    for(int i=0; i < (ac+3); i++) {
        if(!av[i]) break;
        Tcl_DecrRefCount(av[i]);
    }
    return;
}

void tclpd_loadbang(t_tcl* x) {
    tclpd_inlet_anything(x, 0, gensym("loadbang"), 0, NULL);
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
    return (t_tcl*)object_table_get(objectSequentialId);
}

t_pd* tclpd_get_instance_pd(const char* objectSequentialId) {
    return (t_pd*)object_table_get(objectSequentialId);
}

t_object* tclpd_get_object(const char* objectSequentialId) {
    t_tcl* x = tclpd_get_instance(objectSequentialId);
    return &x->o;
}

t_pd* tclpd_get_object_pd(const char* objectSequentialId) {
    t_object* o = tclpd_get_object(objectSequentialId);
    return &o->ob_pd;
}

t_glist* tclpd_get_glist(const char* objectSequentialId) {
    t_tcl* x = tclpd_get_instance(objectSequentialId);
    return x->x_glist;
}

t_symbol* null_symbol() {
    return (t_symbol*)0;
}

void poststring2 (const char *s) {
    post("%s", s);
}

void tclpd_save(t_gobj* z, t_binbuf* b) {
    Tcl_Obj* av[3]; InitArray(av, 3, NULL);
    Tcl_Obj* res;

    t_tcl* x = (t_tcl*)z;

    av[0] = x->self;
    Tcl_IncrRefCount(av[0]);
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
    Tcl_DecrRefCount(av[0]);
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
}

void tclpd_properties(t_gobj* z, t_glist* owner) {
    Tcl_Obj* av[3]; InitArray(av, 3, NULL);
    Tcl_Obj* res;

    t_tcl* x = (t_tcl*)z;

    av[0] = x->self;
    Tcl_IncrRefCount(av[0]);
    av[1] = Tcl_NewStringObj("object", -1);
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("properties", -1);
    Tcl_IncrRefCount(av[2]);
    int result = Tcl_EvalObjv(tcl_for_pd, 3, av, 0);
    if(result != TCL_OK) {
        //res = Tcl_GetObjResult(tcl_for_pd);
        pd_error(x, "Tcl: object properties: failed");
        tclpd_interp_error(result);
    }
    Tcl_DecrRefCount(av[0]);
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
}
