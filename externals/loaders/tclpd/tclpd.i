%module tclpd

%{
#undef EXTERN
#include "tclpd.h"
#define __attribute__(x)
%}

%include exception.i
%include cpointer.i
%include carrays.i
%include typemaps.i

%pointer_functions(t_atom, atom)
%pointer_functions(t_symbol, symbol)

%array_functions(t_atom, atom_array)
/*
Creates four functions.

type *new_name(int nelements)
type *delete_name(type *ary)
type name_getitem(type *ary, int index)
void name_setitem(type *ary, int index, type value)
 */

%typemap(in) (int argc, t_atom *argv) {
    if(Tcl_ListObjLength(interp, $input, &$1) == TCL_ERROR) {
        SWIG_exception(SWIG_RuntimeError, "failed to get list length");
        SWIG_fail;
    }
    $2 = (t_atom *)getbytes(sizeof(t_atom) * $1);
    int i;
    Tcl_Obj *oi;
    for(i = 0; i < $1; i++) {
        if(Tcl_ListObjIndex(interp, $input, i, &oi) == TCL_ERROR) {
            SWIG_exception(SWIG_RuntimeError, "failed to access list element");
            SWIG_fail;
        }
        if(tcl_to_pdatom(oi, &$2[i]) == TCL_ERROR) {
            SWIG_exception(SWIG_RuntimeError, "failed tcl_to_pdatom conversion");
            SWIG_fail;
        }
    }
}

%typemap(freearg) (int argc, t_atom *argv) {
    if($2) freebytes($2, sizeof(t_atom) * $1);
}

%typemap(in) t_atom * {
    $1 = (t_atom *)getbytes(sizeof(t_atom));
    if(tcl_to_pdatom($input, $1) == TCL_ERROR) {
        SWIG_exception(SWIG_RuntimeError, "failed tcl_to_pdatom conversion");
        SWIG_fail;
    }
}

%typemap(freearg) t_atom * {
    freebytes($1, sizeof(t_atom));
}

%typemap(out) t_atom * {
    Tcl_Obj *lst;
    if(pdatom_to_tcl($1, &lst) == TCL_ERROR) {
        SWIG_exception(SWIG_RuntimeError, "failed pdatom_to_tcl conversion");
        SWIG_fail;
    }
    Tcl_SetObjResult(interp, lst);
}

%typemap(in) t_symbol * {
    if(tcl_to_pdsymbol($input, &$1) == TCL_ERROR) {
        SWIG_exception(SWIG_RuntimeError, "failed tcl_to_pdsymbol conversion");
        SWIG_fail;
    }
}

%typemap(out) t_symbol * {
    Tcl_Obj *lst;
    if(pdsymbol_to_tcl($1, &lst) == TCL_ERROR) {
        SWIG_exception(SWIG_RuntimeError, "failed pdsymbol_to_tcl conversion");
        SWIG_fail;
    }
    Tcl_SetObjResult(interp, lst);
}

%typemap(in) t_tcl * {
    const char *str = Tcl_GetStringFromObj($input, NULL);
    $1 = object_table_get(str);
    if(!$1) {
        SWIG_exception(SWIG_RuntimeError, "not a t_tcl * instance");
        SWIG_fail;
    }
}

%typemap(in) t_text * {
    const char *str = Tcl_GetStringFromObj($input, NULL);
    $1 = object_table_get(str);
    if(!$1) {
        SWIG_exception(SWIG_RuntimeError, "not a t_text * instance");
        SWIG_fail;
    }
}

%typemap(in) t_pd * {
    const char *str = Tcl_GetStringFromObj($input, NULL);
    $1 = object_table_get(str);
    if(!$1) {
        SWIG_exception(SWIG_RuntimeError, "not a t_pd * instance");
        SWIG_fail;
    }
}

%typemap(in) t_object * {
    const char *str = Tcl_GetStringFromObj($input, NULL);
    t_tcl *x = object_table_get(str);
    if(!x) {
        SWIG_exception(SWIG_RuntimeError, "not a t_tcl * instance");
        SWIG_fail;
    }
    $1 = &x->o;
}

/* functions that are in m_pd.h but don't exist in modern versions of pd */
%ignore pd_getfilename;
%ignore pd_getdirname;
%ignore pd_anything;
%ignore class_parentwidget;
%ignore sys_isreadablefile;
%ignore garray_get;
%ignore c_extern;
%ignore c_addmess;

/* functions that we can't auto-wrap, because they have varargs */
%ignore post;
%ignore class_new;

/* functions that we can't auto-wrap, because <insert reason here> */
%ignore glist_new;
%ignore canvas_zapallfortemplate;
%ignore canvas_fattenforscalars;
%ignore canvas_visforscalars;
%ignore canvas_clicksub;
%ignore text_xcoord;
%ignore text_ycoord;
%ignore canvas_getglistonsuper;
%ignore canvas_getfont;
%ignore canvas_setusedastemplate;
%ignore canvas_vistext;
%ignore rtext_remove;
%ignore canvas_recurapply;
%ignore gobj_properties;

/* function that we don't want to wrap, because they are internal */
%ignore tclpd_setup;
%ignore tclpd_interp_error;
%ignore tcl_to_pdatom;
%ignore tcl_to_pdsymbol;
%ignore pdatom_to_tcl;
%ignore pdsymbol_to_tcl;
%ignore class_table_add;
%ignore class_table_remove;
%ignore class_table_get;
%ignore object_table_add;
%ignore object_table_remove;
%ignore object_table_get;

/* not needed - typemaps take care of this */
%ignore gensym;

/* end of ignore-list */

%include "m_pd.h"
%include "g_canvas.h"
%include "tclpd.h"

/* this does the trick of solving
 TypeError in method 'xyz', argument 4 of type 't_atom *' */
/*%name(outlet_list) EXTERN void outlet_list(t_outlet *x, t_symbol *s, int argc, t_atom_array *argv);
%name(outlet_anything) EXTERN void outlet_anything(t_outlet *x, t_symbol *s, int argc, t_atom_array *argv);
*/

