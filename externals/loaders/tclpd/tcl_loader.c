#include "tcl_extras.h"
#include <string.h>
#include <unistd.h>

extern int tclpd_do_load_lib(t_canvas *canvas, char *objectname) {
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

    int result;

    // create the required tcl namespace for the class
    tclpd_class_namespace_init(classname);

    // load tcl external:
    result = Tcl_EvalFile(tcl_for_pd, filename);
    if(result == TCL_OK) {
        post("Tcl loader: loaded %s", filename);
    } else {
        post("Tcl loader: error trying to load %s", filename);
        tclpd_interp_error(NULL, result);
        return 0;
    }

#ifdef TCLPD_CALL_SETUP
    // call the setup method:
    char cmd[64];
    snprintf(cmd, 64, "::%s::setup", classname);
    result = Tcl_Eval(tcl_for_pd, cmd);
    if(result == TCL_OK) {
    } else {
        post("Tcl loader: error in %s %s::setup", filename, classname);
        tclpd_interp_error(NULL, result);
        return 0;
    }
#endif // TCLPD_CALL_SETUP

    class_set_extern_dir(&s_);
    sys_putonloadlist(objectname);
    return 1;
}

