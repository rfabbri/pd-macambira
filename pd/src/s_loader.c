/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifdef DL_OPEN
#include <dlfcn.h>
#endif
#ifdef UNIX
#include <stdlib.h>
#include <unistd.h>
#endif
#ifdef NT
#include <io.h>
#include <windows.h>
#endif
#ifdef MACOSX
#include <mach-o/dyld.h> 
#endif
#include <string.h>
#include "m_imp.h"
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
#ifdef NT
    ".dll";
#endif


int sys_load_lib(char *dirname, char *classname)
{
    char symname[MAXPDSTRING], filename[MAXPDSTRING], dirbuf[MAXPDSTRING],
    	*nameptr, *lastdot;
    void *dlobj;
    t_xxx makeout;
    int fd;
#ifdef NT
    HINSTANCE ntdll;
#endif
#if 0
    fprintf(stderr, "lib %s %s\n", dirname, classname);
#endif
    if ((fd = open_via_path(dirname, classname, sys_dllextent,
    	dirbuf, &nameptr, MAXPDSTRING, 1)) < 0)
    {
    	return (0);
    }
    else
    {
    	close(fd);
    	    /* refabricate the pathname */
	strcpy(filename, dirbuf);
	strcat(filename, "/");
	strcat(filename, nameptr);
    	    /* extract the setup function name */
    	if (lastdot = strrchr(nameptr, '.'))
	    *lastdot = 0;

#ifdef MACOSX
    	strcpy(symname, "_");
    	strcat(symname, nameptr);
#else
    	strcpy(symname, nameptr);
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
	    return (0);
	}
	makeout = (t_xxx)dlsym(dlobj,  symname);
#endif
#ifdef NT
	sys_bashfilename(filename, filename);
    	ntdll = LoadLibrary(filename);
    	if (!ntdll)
    	{
	    post("%s: couldn't load", filename);
	    return (0);
	}
    	makeout = (t_xxx)GetProcAddress(ntdll, symname);  
#endif
#ifdef MACOSX
        {
            NSObjectFileImage image; 
            void *ret;
            NSSymbol s; 
            if ( NSCreateObjectFileImageFromFile( filename, &image) != NSObjectFileImageSuccess )
            {
                post("%s: couldn't load", filename);
                return 0;
            }
            ret = NSLinkModule( image, filename, NSLINKMODULE_OPTION_BINDNOW); 
            
            s = NSLookupSymbolInModule(ret, symname); 
        
            if (s)
                makeout = (t_xxx)NSAddressOfSymbol( s);
            else makeout = 0;
        }
#endif
    }
    if (!makeout)
    {
    	post("load_object: Symbol \"%s\" not found", symname);
    	return 0;
    }
    (*makeout)();
    return (1);
}









