#include "tcl_extras.h"
#include <unistd.h>
#include <limits.h>

Tcl_Interp *tcl_for_pd = NULL;

extern "C" void tcl_setup(void) {
    tclpd_setup();
}

void tclpd_setup(void) {
    if(tcl_for_pd) {
        return;
    }

    post("Tcl loader v0.1.1 - 08.2009");

    proxyinlet_setup();

    tcl_for_pd = Tcl_CreateInterp();
    Tcl_Init(tcl_for_pd);
    Tclpd_SafeInit(tcl_for_pd);

    char *dirname   = new char[PATH_MAX];
    char *dirresult = new char[PATH_MAX];
    /* nameresult is only a pointer in dirresult space so don't delete[] it. */
    char *nameresult;
    if(getcwd(dirname, PATH_MAX) < 0) {
        post("Tcl loader: FATAL: cannot get current dir");
        /* exit(69); */ return;
    }

    int fd = open_via_path(dirname, "tcl", PDSUF, dirresult, &nameresult, PATH_MAX, 1);
    if(fd >= 0) {
        close(fd);
    } else {
        post("Tcl loader: %s was not found via the -path!", "tcl" PDSUF);
    }

    Tcl_SetVar(tcl_for_pd, "DIR", dirresult, 0);
    Tcl_Eval(tcl_for_pd, "set auto_path [linsert $auto_path $DIR]");

    if(Tcl_Eval(tcl_for_pd, "source $DIR/tcl.tcl") == TCL_OK) {
        post("Tcl loader: loaded %s/tcl.tcl", dirresult);
    }

    if(Tcl_Eval(tcl_for_pd,"source $env(HOME)/.pd.tcl") == TCL_OK) {
        post("Tcl loader: loaded ~/.pd.tcl");
    }

    delete[] dirresult;
    delete[] dirname;

    sys_register_loader(tclpd_do_load_lib);
}

void tclpd_interp_error(int result) {
    post("Tcl error: %s", Tcl_GetStringResult(tcl_for_pd));
    post("  (see stderr for details)");

    fprintf(stderr, "------------------- Tcl error: -------------------\n");
    Tcl_Obj* dict = Tcl_GetReturnOptions(tcl_for_pd, result);
    Tcl_Obj* errorInfo = NULL;
    Tcl_Obj* errorInfoK = Tcl_NewStringObj("-errorinfo", -1);
    Tcl_IncrRefCount(errorInfoK);
    Tcl_DictObjGet(tcl_for_pd, dict, errorInfoK, &errorInfo);
    Tcl_DecrRefCount(errorInfoK);
    fprintf(stderr, "%s\n", Tcl_GetStringFromObj(errorInfo, 0));
    fprintf(stderr, "--------------------------------------------------\n");
}
