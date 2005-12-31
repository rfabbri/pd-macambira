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
#ifdef MACOSX
#include <mach-o/dyld.h> 
#endif
#include <string.h>
#include "m_pd.h"
#include "s_stuff.h"
#include <stdio.h>

typedef void (*t_xxx)(void);

static char sys_dllextent[] = 
#ifdef __FreeBSD__
    ".pd_freebsd";
#endif
#ifdef IRIX
#ifdef N32
    ".pd_irix6";
#else
    ".pd_irix5";
#endif
#endif
#ifdef __linux__
    ".pd_linux";
#endif
#ifdef MACOSX
    ".pd_darwin";
#endif
#ifdef MSW
    ".dll";
#endif

void class_set_extern_dir(t_symbol *s);

static int sys_load_lib_alt(char *dirname, char *classname, char *altname)
{
    char symname[MAXPDSTRING], filename[MAXPDSTRING], dirbuf[MAXPDSTRING],
      classname2[MAXPDSTRING], *nameptr, *lastdot, 
      altsymname[MAXPDSTRING];
    void *dlobj;
    t_xxx makeout = NULL;
    int fd;
#ifdef MSW
    HINSTANCE ntdll;
#endif
#if 0
    fprintf(stderr, "lib %s %s\n", dirname, classname);
#endif
        /* try looking in the path for (classname).(sys_dllextent) ... */
    if ((fd = open_via_path(dirname, classname, sys_dllextent,
        dirbuf, &nameptr, MAXPDSTRING, 1)) < 0)
    {
            /* next try (classname)/(classname).(sys_dllextent) ... */
        strncpy(classname2, classname, MAXPDSTRING);
        filename[MAXPDSTRING-2] = 0;
        strcat(classname2, "/");
        strncat(classname2, classname, MAXPDSTRING-strlen(classname2));
        filename[MAXPDSTRING-1] = 0;
        if ((fd = open_via_path(dirname, classname2, sys_dllextent,
            dirbuf, &nameptr, MAXPDSTRING, 1)) < 0)
        {
          /* next try (alternative_classname).(sys_dllextent) */
          if(altname)
            {
              if ((fd = open_via_path(dirname, altname, sys_dllextent,
                                      dirbuf, &nameptr, MAXPDSTRING, 1)) < 0)

                /* next try (alternative_classname)/(alternative_classname).(sys_dllextent) ... */
                strncpy(classname2, altname, MAXPDSTRING);
              filename[MAXPDSTRING-2] = 0;
              strcat(classname2, "/");
              strncat(classname2, altname, MAXPDSTRING-strlen(classname2));
              filename[MAXPDSTRING-1] = 0;
              if ((fd = open_via_path(dirname, classname2, sys_dllextent,
                                      dirbuf, &nameptr, MAXPDSTRING, 1)) < 0)
                {
                  return 0;
                } 
            }
          else
            return (0);
        }
    }


    close(fd);
    class_set_extern_dir(gensym(dirbuf));

        /* refabricate the pathname */
    strncpy(filename, dirbuf, MAXPDSTRING);
    filename[MAXPDSTRING-2] = 0;
    strcat(filename, "/");
    strncat(filename, nameptr, MAXPDSTRING-strlen(filename));
    filename[MAXPDSTRING-1] = 0;
        /* extract the setup function name */
    if (lastdot = strrchr(nameptr, '.'))
        *lastdot = 0;

#ifdef MACOSX
    strcpy(symname, "_");
    strcat(symname, nameptr);
    if(altname)
      {
        strcpy(altsymname, "_setup_");
        strcat(symname, altname);
      }
#else
    strcpy(symname, nameptr);
    if(altname)
      {
        strcpy(altsymname, "setup_");
        strcat(altsymname, altname);
      }
#endif

        /* if the last character is a tilde, replace with "_tilde" */
    if (symname[strlen(symname) - 1] == '~')
        strcpy(symname + (strlen(symname) - 1), "_tilde");
        /* and append _setup to form the C setup function name */
    strcat(symname, "_setup");
#ifdef DL_OPEN
    dlobj = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
    if (!dlobj)
    {
        post("%s: %s", filename, dlerror());
        class_set_extern_dir(&s_);
        return (0);
    }
    makeout = (t_xxx)dlsym(dlobj,  symname);
    if(!makeout)makeout = (t_xxx)dlsym(dlobj,  altsymname);
#endif
#ifdef MSW
    sys_bashfilename(filename, filename);
    ntdll = LoadLibrary(filename);
    if (!ntdll)
    {
        post("%s: couldn't load", filename);
        class_set_extern_dir(&s_);
        return (0);
    }
    makeout = (t_xxx)GetProcAddress(ntdll, symname);  
    if(!makeout)makeout = (t_xxx)GetProcAddress(ntdll, altsymname);  
#endif
#if defined(MACOSX) && !defined(DL_OPEN)
    {
        NSObjectFileImage image; 
        void *ret;
        NSSymbol s; 
        if ( NSCreateObjectFileImageFromFile( filename, &image) != NSObjectFileImageSuccess )
        {
            post("%s: couldn't load", filename);
            class_set_extern_dir(&s_);
            return 0;
        }
        ret = NSLinkModule( image, filename, 
               NSLINKMODULE_OPTION_BINDNOW |
               NSLINKMODULE_OPTION_RETURN_ON_ERROR);
               
        if (ret == NULL) {
                int err;
                const char *fname, *errt;
                NSLinkEditErrors c;
                NSLinkEditError(&c, &err, &fname, &errt);
                post("link error %d %s %s", err, fname, errt);
                return 0;
        }
        s = NSLookupSymbolInModule(ret, symname); 

        if(!s)s=NSLookupSymbolInModule(ret, altsymname); 

        if (s)
            makeout = (t_xxx)NSAddressOfSymbol( s);
        else makeout = 0;
    }
#endif

    if (!makeout)
    {
        post("load_object: Symbol \"%s\" not found", symname);
        if(altname)
          post("load_object: Symbol \"%s\" not found", altsymname);
        class_set_extern_dir(&s_);
        return 0;
    }
    (*makeout)();
    class_set_extern_dir(&s_);
    return (1);
}

/* callback type definition */
typedef int (*loader_t)(char *dirname, char *classname, char *altname);

/* linked list of loaders */
typedef struct loader_queue {
    loader_t loader;
    struct loader_queue *next;
} loader_queue_t;

static loader_queue_t loaders = {sys_load_lib_alt, NULL};

/* register class loader function */
void sys_register_loader(loader_t loader)
{
    loader_queue_t *q = &loaders;
    while (1)
    {
        if (q->next) 
            q = q->next;
        else
        {
            q->next = (loader_queue_t *)getbytes(sizeof(loader_queue_t));
            q->next->loader = loader;
            q->next->next = NULL;
            break;
        }
    }   
}

int sys_load_lib(char *dirname, char *classname, char *altname)
{
    int dspstate = canvas_suspend_dsp();
    int ok = 0;
    loader_queue_t *q;
    for(q = &loaders; q; q = q->next)
        if(ok = q->loader(dirname, classname, altname)) break;
    canvas_resume_dsp(dspstate);
    return ok;
}



