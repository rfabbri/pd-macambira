/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifdef DL_OPEN
#include <dlfcn.h>
#endif
#ifdef UNISTD
#include <stdlib.h>
#include <unistd.h>
#endif
#ifdef MSW
#include <io.h>
#include <windows.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#include <string.h>
#define PD_PLUSPLUS_FACE
#include "desire.h"
#include "s_stuff.h"
#include <stdio.h>
#include <sstream>
using namespace std;

typedef void (*t_xxx)();

/* naming convention for externs.  The names are kept distinct for those
who wich to make "fat" externs compiled for many platforms.  Less specific
fallbacks are provided, primarily for back-compatibility; these suffice if
you are building a package which will run with a single set of compiled
objects.  The specific name is the letter b, l, d, or m for  BSD, linux,
darwin, or microsoft, followed by a more specific string, either "fat" for
a fat binary or an indication of the instruction set. */

#ifdef __FreeBSD__
static char sys_dllextent[] = ".b_i386", sys_dllextent2[] = ".pd_freebsd";
#endif
#ifdef __linux__
#ifdef __ia64__
static char sys_dllextent[] = ".l_ia64", sys_dllextent2[] = ".pd_linux";
#else
static char sys_dllextent[] = ".l_i386", sys_dllextent2[] = ".pd_linux";
#endif
#endif
#ifdef __APPLE__
#ifndef MACOSX3
static char sys_dllextent[] = ".d_fat", sys_dllextent2[] = ".pd_darwin";
#else
static char sys_dllextent[] = ".d_ppc", sys_dllextent2[] = ".pd_darwin";
#endif
#endif
#ifdef MSW
static char sys_dllextent[] = ".m_i386", sys_dllextent2[] = ".dll";
#endif

/* maintain list of loaded modules to avoid repeating loads */
struct t_loadlist {
    t_loadlist *ll_next;
    t_symbol *ll_name;
};

static t_loadlist *sys_loaded;
/* return true if already loaded */
int sys_onloadlist(char *classname) {
    t_symbol *s = gensym(classname);
    t_loadlist *ll;
    for (ll = sys_loaded; ll; ll = ll->ll_next)
        if (ll->ll_name == s)
            return 1;
    return 0;
}

/* add to list of loaded modules */
void sys_putonloadlist(char *classname) {
    t_loadlist *ll = (t_loadlist *)getbytes(sizeof(*ll));
    ll->ll_name = gensym(classname);
    ll->ll_next = sys_loaded;
    sys_loaded = ll;
    /* post("put on list %s", classname); */
}

static char *make_setup_name(const char *s) {
    bool hexmunge=0;
    ostringstream buf;
    for (; *s; s++) {
        char c = *s;
        if ((c>='0' && c<='9') || (c>='A' && c<='Z') || (c>='a' && c<='z' )|| c == '_') {
            buf << c;
        } else if (c=='~' && s[1]==0) { /* trailing tilde becomes "_tilde" */
            buf << "_tilde";
        } else { /* anything you can't put in a C symbol is sprintf'ed in hex */
	    // load_object: symbol "setup_0xbf861ee4" not found
            oprintf(buf,"0x%02x",c);
            hexmunge = 1;
        }
    }
    char *r;
    if (hexmunge) asprintf(&r,"setup_%s",buf.str().data());
    else          asprintf(&r,"%s_setup",buf.str().data());
    return r;
}

static int sys_do_load_lib(t_canvas *canvas, char *objectname) {
    char *filename=0, *dirbuf, *classname, *nameptr;
    t_xxx makeout = NULL;
    int fd;
    if ((classname = strrchr(objectname, '/'))) classname++;
    else classname = objectname;
    if (sys_onloadlist(objectname)) {
        post("%s: already loaded", objectname);
        return 1;
    }
    char *symname = make_setup_name(classname);
    /* try looking in the path for (objectname).(sys_dllextent) ... */
    if ((fd = canvas_open2(canvas, objectname, sys_dllextent , &dirbuf, &nameptr, 1)) >= 0) goto gotone;
    /* same, with the more generic sys_dllextent2 */
    if ((fd = canvas_open2(canvas, objectname, sys_dllextent2, &dirbuf, &nameptr, 1)) >= 0) goto gotone;
    /* next try (objectname)/(classname).(sys_dllextent) ... */
    asprintf(&filename,"%s/%s",objectname,classname);
    if ((fd = canvas_open2(canvas, filename, sys_dllextent , &dirbuf, &nameptr, 1)) >= 0) goto gotone;
    if ((fd = canvas_open2(canvas, filename, sys_dllextent2, &dirbuf, &nameptr, 1)) >= 0) goto gotone;
    return 0;
gotone:
    close(fd);
    class_set_extern_dir(gensym(dirbuf));
    /* rebuild the absolute pathname */
    free(filename);
    /* extra nulls are a workaround for a dlopen bug */
    asprintf(&filename,"%s/%s%c%c%c%c",dirbuf,nameptr,0,0,0,0);
//    filename = realloc(filename,);
#ifdef DL_OPEN
    void *dlobj = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
    if (!dlobj) {
        post("%s: %s", filename, dlerror());
        goto forgetit;
    }
    makeout = (t_xxx)dlsym(dlobj, symname);
#endif
#ifdef MSW
    sys_bashfilename(filename, filename);
    HINSTANCE ntdll = LoadLibrary(filename);
    if (!ntdll) {
        post("%s: couldn't load", filename);
        goto forgetit;
    }
    makeout = (t_xxx)GetProcAddress(ntdll);
#endif
    if (!makeout) {
        post("%s: can't find symbol '%s' in library", filename, symname);
        goto forgetit;
    }
    makeout();
    class_set_extern_dir(&s_);
    sys_putonloadlist(objectname);
    free(filename); free(symname);
    return 1;
forgetit:
    class_set_extern_dir(&s_);
    free(filename); free(symname); free(dirbuf);
    return 0;
}

/* callback type definition */
typedef int (*t_loader)(t_canvas *canvas, char *classname);

/* linked list of loaders */
typedef struct t_loader_queue {
    t_loader loader;
    t_loader_queue *next;
};

static t_loader_queue loaders = {sys_do_load_lib, NULL};

/* register class loader function */
void sys_register_loader(t_loader loader) {
    t_loader_queue *q = &loaders;
    while (1) {
        if (q->next) q = q->next;
        else {
            q->next = (t_loader_queue *)getbytes(sizeof(t_loader_queue));
            q->next->loader = loader;
            q->next->next = NULL;
            break;
        }
    }
}

int sys_load_lib(t_canvas *canvas, char *classname) {
    int dspstate = canvas_suspend_dsp();
    int ok = 0;
    for(t_loader_queue *q = &loaders; q; q = q->next) if ((ok = q->loader(canvas, classname))) break;
    canvas_resume_dsp(dspstate);
    return ok;
}
