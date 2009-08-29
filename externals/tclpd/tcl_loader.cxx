#include "tcl_extras.h"
#include <string.h>
#include <unistd.h>

extern "C" int tclpd_do_load_lib(t_canvas *canvas, char *objectname) {
#ifdef DEBUG
    post("Tcl loader: registering tcl class loader mechanism");
#endif
    char filename[MAXPDSTRING], dirbuf[MAXPDSTRING],
        *classname, *nameptr;
    int fd;

    if ((classname = strrchr(objectname, '/')) != NULL)
        classname++;
    else
        classname = objectname;

    if(sys_onloadlist(objectname)) {
        post("%s: already loaded", objectname);
        return (1);
    }

        /* try looking in the path for (objectname).(tcl) ... */
    if ((fd = canvas_open(canvas, objectname, ".tcl",
        dirbuf, &nameptr, MAXPDSTRING, 1)) >= 0)
            goto gotone;

        /* next try (objectname)/(classname).(tcl) ... */
    strncpy(filename, objectname, MAXPDSTRING);
    filename[MAXPDSTRING-2] = 0;
    strcat(filename, "/");
    strncat(filename, classname, MAXPDSTRING-strlen(filename));
    filename[MAXPDSTRING-1] = 0;
    if ((fd = canvas_open(canvas, filename, ".tcl",
        dirbuf, &nameptr, MAXPDSTRING, 1)) >= 0)
            goto gotone;

    return 0;

gotone:
    close(fd);
    class_set_extern_dir(gensym(dirbuf));
        /* rebuild the absolute pathname */
    strncpy(filename, dirbuf, MAXPDSTRING);
    filename[MAXPDSTRING-2] = 0;
    strcat(filename, "/");
    strncat(filename, nameptr, MAXPDSTRING-strlen(filename));
    filename[MAXPDSTRING-1] = 0;

    // load tcl:
    char b[MAXPDSTRING+10];
    snprintf(&b[0], MAXPDSTRING+10, "source %s", filename);
    int result = Tcl_Eval(tcl_for_pd, b);
    if(result == TCL_OK) {
        post("Tcl_loader: loaded %s", filename);
    } else {
        post("Tcl_loader: error trying to load %s", filename);
        post("Error: %s", Tcl_GetStringResult(tcl_for_pd));
        post("(see stderr for details)");

        fprintf(stderr, "------------------- Tcl error: -------------------\n");
        Tcl_Obj* dict = Tcl_GetReturnOptions(tcl_for_pd, result);
        Tcl_Obj* errorInfo = NULL;
        Tcl_Obj* errorInfoK = Tcl_NewStringObj("-errorinfo", -1);
        Tcl_IncrRefCount(errorInfoK);
        Tcl_DictObjGet(tcl_for_pd, dict, errorInfoK, &errorInfo);
        Tcl_DecrRefCount(errorInfoK);
        fprintf(stderr, "%s\n", Tcl_GetStringFromObj(errorInfo, 0));
        fprintf(stderr, "--------------------------------------------------\n");
        return 0;
    }

    class_set_extern_dir(&s_);
    sys_putonloadlist(objectname);
    return 1;
}

