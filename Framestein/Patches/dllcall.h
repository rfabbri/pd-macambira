#ifndef __DLLCALL_H
#define __DLLCALL_H

#include "windows.h"

void loadeffect(HMODULE *lib, FARPROC *proc, char *name)
{
	if(*lib) FreeLibrary(*lib);

	*lib = LoadLibrary(name);
	if(!*lib)
	{
		post("loadeffect: failed to load %s", name);
		return;
	}
	*proc = GetProcAddress(*lib, "perform_effect");
	if(!*proc)
	{
		post("loadeffect: failed to get perform_effect from %s", name);
		FreeLibrary(*lib);
		*lib=NULL;
		return;
	}
}

void loadcopy(HMODULE *lib, FARPROC *proc, char *name)
{
	if(*lib) FreeLibrary(*lib);

	*lib = LoadLibrary(name);
	if(!*lib)
	{
		post("loadcopy: failed to load %s", name);
		return;
	}
	*proc = GetProcAddress(*lib, "perform_copy");
	if(!*proc)
	{
		post("loadcopy: failed to get perform_copy from %s", name);
		FreeLibrary(*lib);
		*lib=NULL;
		return;
	}
}

#endif
