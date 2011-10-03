#include "tcl_extras.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <m_imp.h>

Tcl_Interp *tcl_for_pd = NULL;

void tclpd_setup(void) {
    if(tcl_for_pd) {
        return;
    }

#if PD_MAJOR_VERSION==0 && PD_MINOR_VERSION<43
    post("tclpd loader v" TCLPD_VERSION);
#else
    logpost(NULL, 3, "tclpd loader v" TCLPD_VERSION);
#endif

    proxyinlet_setup();

    tcl_for_pd = Tcl_CreateInterp();
    Tcl_Init(tcl_for_pd);
    Tclpd_SafeInit(tcl_for_pd);

    Tcl_Eval(tcl_for_pd, "package provide Tclpd " TCLPD_VERSION);

    t_class* foo_class = class_new(gensym("tclpd_init"), 0, 0, 0, 0, 0);
    char buf[PATH_MAX];
    
    snprintf(buf, PATH_MAX, "%s/pdlib.tcl", foo_class->c_externdir->s_name);
    if(Tcl_EvalFile(tcl_for_pd, buf) != TCL_OK) {
        error("tclpd loader: error loading %s", buf);
    }

    sys_register_loader(tclpd_do_load_lib);
}

void tclpd_interp_error(int result) {
    error("tclpd error: %s", Tcl_GetStringResult(tcl_for_pd));
    post("  (see stderr for details)");

    fprintf(stderr, "------------------- Tcl error: -------------------\n");

    // Tcl_GetReturnOptions and Tcl_DictObjGet only available in Tcl >= 8.5

#if ((TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION >= 5) || (TCL_MAJOR_VERSION > 8))
    Tcl_Obj* dict = Tcl_GetReturnOptions(tcl_for_pd, result);
    Tcl_Obj* errorInfo = NULL;
    Tcl_Obj* errorInfoK = Tcl_NewStringObj("-errorinfo", -1);
    Tcl_IncrRefCount(errorInfoK);
    Tcl_DictObjGet(tcl_for_pd, dict, errorInfoK, &errorInfo);
    Tcl_DecrRefCount(errorInfoK);
    fprintf(stderr, "%s\n", Tcl_GetStringFromObj(errorInfo, 0));
#else
    fprintf(stderr, "Backtrace not available in Tcl < 8.5. Please upgrade Tcl.\n");
#endif

    fprintf(stderr, "--------------------------------------------------\n");
}
