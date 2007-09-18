%module tclpd
%include exception.i
%include cpointer.i

/* functions that are in m_pd.h but don't exist in modern versions of pd */
%ignore pd_getfilename;
%ignore pd_getdirname;
%ignore pd_anything;
%ignore class_parentwidget;
%ignore sys_isreadablefile;
%ignore garray_get;
%ignore c_extern;
%ignore c_addmess;

/* functions that are only in Miller's pd, not in devel_0_39/DesireData */
%ignore sys_idlehook;

/* functions that are not supported by DesireData */
%ignore class_getpropertiesfn;
%ignore class_setpropertiesfn;
%ignore class_getwidget;
%ignore class_setwidget;
%ignore sys_fontwidth;
%ignore sys_fontheight;
%ignore sys_queuegui;
%ignore sys_unqueuegui;
%ignore sys_pretendguibytes;
%ignore class_setparentwidget;
%ignore pd_getparentwidget;
%ignore getzbytes;
%ignore gfxstub_new;
%ignore gfxstub_deleteforkey;
%ignore glist_grab;

/* functions that we can't auto-wrap, because they have varargs */
%ignore post;
%ignore class_new;

/* end of ignore-list */

%include "m_pd.h"
%include "tcl_extras.h"

%{
#include "m_pd.h"

typedef t_atom t_atom_array;
%}

%name(outlet_list) EXTERN void outlet_list(t_outlet *x, t_symbol *s, int argc, t_atom_array *argv);

%pointer_class(t_float, t_float)
%pointer_class(t_symbol, t_symbol)

%{
#include "tcl_extras.h"

#include <unistd.h>
#include "config.h"

Tcl_Interp *tcl_for_pd = 0;

extern "C" SWIGEXPORT int Tclpd_SafeInit(Tcl_Interp *interp);

extern "C" void tcl_setup (void) {
  /* Pd initialization */

  if (tcl_for_pd) {
    post("Tcl: already loaded");
    return;
  }
  post("Tcl external v0.1-alpha - 09.2007");
  post("by Federico Ferri <mescalinum@gmail.com>, Mathieu Bouchard <matju@artengine.ca>");
  tcl_for_pd = Tcl_CreateInterp();
  Tcl_Init(tcl_for_pd);
  Tclpd_SafeInit(tcl_for_pd);

  char *dirname   = new char[242];
  char *dirresult = new char[242];
  /* nameresult is only a pointer in dirresult space so don't delete[] it. */
  char *nameresult;
  if (getcwd(dirname,242)<0) {post("AAAARRRRGGGGHHHH!"); exit(69);}
  int       fd=open_via_path(dirname,"gridflow/tcl",PDSUF,dirresult,&nameresult,242,1);
  if (fd<0) fd=open_via_path(dirname,         "tcl",PDSUF,dirresult,&nameresult,242,1);
  if (fd>=0) {
    close(fd);
  } else {
    post("%s was not found via the -path!","tcl"PDSUF);
  }
  Tcl_SetVar(tcl_for_pd,"DIR",dirresult,0);
  Tcl_Eval(tcl_for_pd,"set auto_path [concat [list $DIR/.. $DIR $DIR/optional/rblti] $auto_path]");

  if (Tcl_Eval(tcl_for_pd,"source $DIR/tcl.tcl") == TCL_OK)
    post("Tcl: loaded %s/tcl.tcl", dirresult);

  if (Tcl_Eval(tcl_for_pd,"source $env(HOME)/.pd.tcl") == TCL_OK)
    post("Tcl: loaded ~/.pd.tcl");

  delete[] dirresult;
  delete[] dirname;

  post("Tcl: registering tcl loader");
  sys_register_loader(tclpd_do_load_lib);
}

int tcl_to_pd(Tcl_Obj *input, t_atom *output) {
  int llength;
  if(Tcl_ListObjLength(tcl_for_pd, input, &llength) == TCL_ERROR)
    return TCL_ERROR;
  if(llength != 2)
    /*SWIG_exception(SWIG_ValueError, "Bad t_atom: expeting a 2-elements list.");*/
    return TCL_ERROR;

  int i;
  Tcl_Obj* obj[2];
  for(i = 0; i < 2; i++) Tcl_ListObjIndex(tcl_for_pd, input, i, &obj[i]);
  char* argv0 = Tcl_GetStringFromObj(obj[0], 0);

  if(strcmp(argv0, "float") == 0) {
    double dbl;
    if(Tcl_GetDoubleFromObj(tcl_for_pd, obj[1], &dbl) == TCL_ERROR)
      return TCL_ERROR;
    SETFLOAT(output, dbl);
  } else if(strcmp(argv0, "symbol") == 0) {
    SETSYMBOL(output, gensym(Tcl_GetStringFromObj(obj[1], 0)));
  } else if(strcmp(argv0, "pointer") == 0) {
    // TODO:
  }
  return TCL_OK;
}

int pd_to_tcl(t_atom *input, Tcl_Obj **output) {
  Tcl_Obj* tcl_t_atom[2];
  /*post("pd_to_tcl got an atom of type %d (%s)",
    input->a_type, input->a_type == A_FLOAT ? "A_FLOAT" :
    input->a_type == A_SYMBOL ? "A_SYMBOL" :
    input->a_type == A_POINTER ? "A_POINTER" : "?");*/
  switch (input->a_type) {
    case A_FLOAT: {
      tcl_t_atom[0] = Tcl_NewStringObj("float", -1);
      tcl_t_atom[1] = Tcl_NewDoubleObj(input->a_w.w_float);
      break;
    }
    case A_SYMBOL: {
      tcl_t_atom[0] = Tcl_NewStringObj("symbol", -1);
      tcl_t_atom[1] = Tcl_NewStringObj(input->a_w.w_symbol->s_name, strlen(input->a_w.w_symbol->s_name));
      break;
    }
    case A_POINTER: {
      tcl_t_atom[0] = Tcl_NewStringObj("pointer", -1);
      tcl_t_atom[1] = Tcl_NewDoubleObj((long)input->a_w.w_gpointer);
      break;
    }
    default: {
      tcl_t_atom[0] = Tcl_NewStringObj("?", -1);
      tcl_t_atom[1] = Tcl_NewStringObj("", 0);
      break;
    }
  }
  *output = Tcl_NewListObj(2, &tcl_t_atom[0]);
  return TCL_OK;
}

%}

%typemap(in) t_atom * {
  t_atom *a = (t_atom*)getbytes(sizeof(t_atom));
  if(tcl_to_pd($input, a) == TCL_ERROR)
    return TCL_ERROR;
  $1 = a;
}

%typemap(freearg) t_atom * {
  freebytes($1, sizeof(t_atom));
}

%typemap(out) t_atom* {
  Tcl_Obj* res_obj;
  if(pd_to_tcl($1, &res_obj) == TCL_ERROR)
    return TCL_ERROR;
  Tcl_SetObjResult(tcl_for_pd, res_obj);
}

/* helper functions for t_atom arrays */
%inline %{
t_atom_array *new_atom_array(int size) {
  return (t_atom_array*)getbytes(size*sizeof(t_atom));
}
void delete_atom_array(t_atom_array *a, int size) {
  freebytes(a, size*sizeof(t_atom));
}
t_atom* get_atom_array(t_atom_array *a, int index) {
  return &a[index];
}
void set_atom_array(t_atom_array *a, int index, t_atom *n) {
  memcpy(&a[index], n, sizeof(t_atom));
}
%}


