/* plugin~, a Pd tilde object for hosting LADSPA/VST plug-ins
   Copyright (C) 2001 Jarno Seppänen
   Copyright (C) 2000 Richard W.E. Furse
   $Id: vstutils.c,v 1.1 2002-11-19 09:51:40 ggeiger Exp $

   This file is part of plugin~.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */

#ifdef WIN32 /* currently for Windows only */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vst/AEffect.h" /* VST-specific stuff */
#include <windows.h> /* LoadLibrary, FreeLibrary etc. Windows-specific */
#include "vstutils.h"
#include "win/vitunmsvc.h" /* strncasecmp */

/* OS-specific things */
#ifdef WIN32
#define CHR_DIR_SLASH '\\'
#define CHR_PATH_DELIM ';'
#define STR_DIR_SLASH "\\"
#define STR_LIB_SUFFIX ".dll"
#endif
#ifdef __linux__ /* FIXME? */
#define CHR_DIR_SLASH '/'
#define CHR_PATH_DELIM ':'
#define STR_DIR_SLASH "/"
#define STR_LIB_SUFFIX ".so"
#endif

void*
vstutils_load_vst_plugin_dll (const char* plugin_dll_filename)
{
    HINSTANCE* dll = NULL;

    char* buf = NULL;
    size_t fnlength = 0;
    const size_t suffixlength = strlen (STR_LIB_SUFFIX);

    /* precondition */
    assert (plugin_dll_filename != NULL);

    dll = (HINSTANCE*)calloc (1, sizeof (HINSTANCE));
    fnlength = strlen (plugin_dll_filename);

    /* assure the file name ends in ".so", ".dll" or equivalent */
    if (fnlength > suffixlength) {
	int has_lib_suffix = 0;
	has_lib_suffix
	    = (strncasecmp (&plugin_dll_filename[fnlength - suffixlength],
			    STR_LIB_SUFFIX,
			    suffixlength + 1)
	       == 0);
	if (!has_lib_suffix) {
	    buf = calloc (fnlength + suffixlength + 1, sizeof (char));
	    strcpy (buf, plugin_dll_filename);
	    strcat (buf, STR_LIB_SUFFIX);
	    plugin_dll_filename = buf;
	    fnlength += suffixlength;
	}
    }

    /* First we just try loading by absolute directory name or via
       default system paths. */
    *dll = LoadLibrary (plugin_dll_filename);
    if (*dll != NULL) {
	goto VSTUTILS_LOAD_DLL_RETURN_DLL;
    }

    /* If the filename is not absolute then we wish to check along the
       VST_PATH to see if we can find the file there. */
    if (plugin_dll_filename[0] != CHR_DIR_SLASH) {
	const char* vst_path = getenv ("VST_PATH");
	if (vst_path != NULL) {
	    const char* dir_start = NULL;
	    const char* dir_end = NULL;
	    dir_start = vst_path;
	    while (*dir_start != '\0') {
		dir_end = dir_start;
		while (*dir_end != CHR_PATH_DELIM
		       && *dir_end != '\0') {
		    dir_end++;
		}
		if (dir_end > dir_start) {
		    char* absfn = NULL; /* absolute filename */
		    absfn = calloc (dir_end - dir_start /* directory */
				    + 1 /* slash */
				    + fnlength /* basename */
				    + 1, /* null */
				    sizeof (char));
		    /* start with directory */
		    strncpy (absfn, dir_start, dir_end - dir_start);
		    /* append slash if necessary */
		    if (*(dir_end - 1) != CHR_DIR_SLASH) {
			strcat (absfn, STR_DIR_SLASH);
		    }
		    /* finally append dll filename */
		    strcat (absfn, plugin_dll_filename);
#if 0
		    fprintf (stderr, "plugin~_vst: attempting \"%s\"\n", absfn);
#endif
		    *dll = LoadLibrary (absfn);
		    free (absfn);
		    if (*dll != NULL) {
			goto VSTUTILS_LOAD_DLL_RETURN_DLL;
		    }
		}
		/* if not found, try next directory from path */
		dir_start = dir_end;
		if (*dir_start == CHR_PATH_DELIM) {
		    dir_start++;
		}
	    }
	} else {
	    fputs ("warning: You haven't specified the VST_PATH environment variable and didn't specify an absolute path to the plug-in.\n"
		   "Please set the VST_PATH variable to point to your VST plug-in directories (eg. \"set VST_PATH=C:\\Program Files\\Steinberg\\Vstplugins\").\n", stderr);
	}
    }

    /* No luck */
    assert (*dll == NULL);
    free (dll);
    /* remember to check and free buf! */
    if (buf != NULL) {
	free (buf);
	buf = NULL;
    }
    return NULL;
    
 VSTUTILS_LOAD_DLL_RETURN_DLL:
    assert (*dll != NULL);
    /* remember to check and free buf! */
    if (buf != NULL) {
	free (buf);
	buf = NULL;
    }
    return dll;
}

void
vstutils_unload_vst_plugin_dll (void* plugin_dll)
{
    HINSTANCE* dll = plugin_dll;
    if (dll != NULL) {
	FreeLibrary (*dll);
	*dll = NULL;
	free (plugin_dll);
    }
}

AEffect*
vstutils_init_vst_plugin (void* plugin_dll,
			  const char* plugin_dll_filename,
			  audioMasterCallback am)
{
    typedef AEffect* (*vst_main_func)(audioMasterCallback am);
    HINSTANCE* dll = plugin_dll;
    vst_main_func vstmain;
    AEffect* plugin = NULL;

    if (plugin_dll == NULL
	|| *dll == NULL) {
	return NULL;
    }

    vstmain = (vst_main_func)GetProcAddress (*dll, "main");
    if (vstmain == NULL) {
	char* error = NULL;
	FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER
		       | FORMAT_MESSAGE_FROM_SYSTEM
		       | FORMAT_MESSAGE_IGNORE_INSERTS,
		       NULL,
		       GetLastError (),
		       MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
		       error,
		       0,
		       NULL);
	fprintf (stderr,
		 "Unable to find main() function in VST plugin "
		 "DLL file \"%s\": %s.\n"
		 "Are you sure this is a VST plugin file?\n",
		 plugin_dll_filename, error);
	LocalFree (error);
    }
    /* Initialize plugin */
    plugin = vstmain (am);

    /* Check that what we have loaded really IS a VST plugin */
    if (plugin->magic != kEffectMagic) {
	fprintf (stderr,
		 "DLL file \"%s\" is not a genuine VST plugin.\n",
		 plugin_dll_filename);
	return NULL;
    }
    return plugin;
}

#endif /* WIN32 */

/* EOF */
