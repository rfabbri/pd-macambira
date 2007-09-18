#include "tcl_extras.h"
#include <string.h>
#include <unistd.h>

extern "C" int tclpd_do_load_lib(t_canvas *canvas, char *objectname)
{
    char filename[MAXPDSTRING], dirbuf[MAXPDSTRING],
        *classname, *nameptr;
    int fd;
    if (classname = strrchr(objectname, '/'))
        classname++;
    else classname = objectname;
    if (sys_onloadlist(objectname))
    {
        post("%s: already loaded", objectname);
        return (1);
    }
        /* try looking in the path for (objectname).(tcl) ... */
    if ((fd = canvas_open(canvas, objectname, ".tcl",
        dirbuf, &nameptr, MAXPDSTRING, 1)) >= 0)
            goto gotone;
        /* next try (objectname)/(classname).(sys_dllextent) ... */
    strncpy(filename, objectname, MAXPDSTRING);
    filename[MAXPDSTRING-2] = 0;
    strcat(filename, "/");
    strncat(filename, classname, MAXPDSTRING-strlen(filename));
    filename[MAXPDSTRING-1] = 0;
    if ((fd = canvas_open(canvas, filename, ".tcl",
        dirbuf, &nameptr, MAXPDSTRING, 1)) >= 0)
            goto gotone;
    //post("Tcl_loader: tried and failed");
    return (0);
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
    if (Tcl_Eval(tcl_for_pd,b) == TCL_OK)
        post("Tcl_loader: loaded %s", b);
    else
        post("Tcl_loader: error trying to load %s", b);

    class_set_extern_dir(&s_);
    sys_putonloadlist(objectname);
    return (1);
}

