/* --------------------------------------------------------------------------*/
/*                                                                           */
/* object for getting file listings using wildcard patterns,                 */
/* based on the interface of [textfile]                                      */
/* Written by Hans-Christoph Steiner <hans@at.or.at>                         */
/*                                                                           */
/* Copyright (c) 2010 Hans-Christoph Steiner                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 3            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* See file LICENSE for further informations on licensing terms.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software Foundation,   */
/* Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           */
/*                                                                           */
/* --------------------------------------------------------------------------*/

#include <m_pd.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#else
#include <stdlib.h>
#include <glob.h>
#endif

#include <stdio.h>
#include <string.h>

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *getfilenames_class;

typedef struct _getfilenames {
	t_object            x_obj;
	t_symbol*           x_pattern;
    t_canvas*           x_canvas;    
    int                 isnewpattern;
#ifdef _WIN32
	HANDLE              hFind;
#else
	glob_t              glob_buffer;
    unsigned int        current_glob_position;
#endif
	t_outlet            *data_outlet;
	t_outlet            *status_outlet;
} t_getfilenames;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

// TODO: make FindFirstFile display when its just a dir

static void normalize_path(t_getfilenames* x, char *normalized, const char *original)
{
    char buf[FILENAME_MAX];
    t_symbol *cwd = canvas_getdir(x->x_canvas);
#ifdef _WIN32
    sys_unbashfilename(original, buf);
#else
    strncpy(buf, original, FILENAME_MAX);
#endif
    if(sys_isabsolutepath(buf)) {
        strncpy(normalized, buf, FILENAME_MAX);
        return;
    }
    strncpy(normalized, cwd->s_name, FILENAME_MAX);
    if(normalized[(strlen(normalized)-1)] != '/') {
        strncat(normalized, "/", 1);
    }
    if(buf[0] == '.') {
        if(buf[1] == '/') {
            strncat(normalized, buf + 2, 
                    FILENAME_MAX - strlen(normalized));
        } else if(buf[1] == '.' && buf[2] == '/') {
            strncat(normalized, buf, 
                    FILENAME_MAX - strlen(normalized));
        }
    } else if(buf[0] != '/') {
        strncat(normalized, buf, 
                FILENAME_MAX - strlen(normalized));
    } else {
        strncpy(normalized, buf, FILENAME_MAX);
    }
}

static void getfilenames_rewind(t_getfilenames* x)
{
	DEBUG(post("getfilenames_rewind"););
    char normalized_path[FILENAME_MAX] = "";

    normalize_path(x, normalized_path, x->x_pattern->s_name);
#ifdef _WIN32
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	DWORD errorNumber;
	LPVOID lpErrorMessage;
	char fullPathNameBuffer[FILENAME_MAX] = "";
	char unbashBuffer[FILENAME_MAX] = "";
	char outputBuffer[FILENAME_MAX] = "";
	char *pathBuffer;

// arg, looks perfect, but only in Windows Vista
//	GetFinalPathNameByHandle(hFind,fullPathNameBuffer,FILENAME_MAX,FILE_NAME_NORMALIZED);
    GetFullPathName(normalized_path, FILENAME_MAX, fullPathNameBuffer, NULL);
    sys_unbashfilename(fullPathNameBuffer,unbashBuffer);
	
	hFind = FindFirstFile(fullPathNameBuffer, &findData);
	if (hFind == INVALID_HANDLE_VALUE) 
	{
	   errorNumber = GetLastError();
	   switch (errorNumber)
	   {
       case ERROR_FILE_NOT_FOUND:
       case ERROR_PATH_NOT_FOUND:
           pd_error(x,"[getfilenames] nothing found for \"%s\"",x->x_pattern->s_name);
           break;
       default:
           FormatMessage(
               FORMAT_MESSAGE_ALLOCATE_BUFFER | 
               FORMAT_MESSAGE_FROM_SYSTEM,
               NULL,
               errorNumber,
               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
               (LPTSTR) &lpErrorMessage,
               0, NULL );
           pd_error(x,"[getfilenames] %s", (char *)lpErrorMessage);
	   }
	   return;
	} 
    char* unbashBuffer_position = strrchr(unbashBuffer, '/');
    if(unbashBuffer_position)
    {
        pathBuffer = getbytes(FILENAME_MAX+1);
        strncpy(pathBuffer, unbashBuffer, unbashBuffer_position - unbashBuffer);
    }
	do {
        // skip "." and ".."
        if( strcmp(findData.cFileName, ".") && strcmp(findData.cFileName, "..") ) 
		{
            strncpy(outputBuffer, pathBuffer, FILENAME_MAX);
			strcat(outputBuffer,"/");
			strcat(outputBuffer,findData.cFileName);
			outlet_symbol( x->data_outlet, gensym(outputBuffer) );
		}
	} while (FindNextFile(hFind, &findData) != 0);
	FindClose(hFind);
#else /* systems with glob */
	DEBUG(post("globbing %s",normalized_path););
    x->current_glob_position = 0;
	switch( glob( normalized_path, GLOB_TILDE, NULL, &x->glob_buffer ) )
	{
    case GLOB_NOSPACE: 
        pd_error(x,"[getfilenames] out of memory for \"%s\"",normalized_path); 
        break;
# ifdef GLOB_ABORTED
    case GLOB_ABORTED: 
        pd_error(x,"[getfilenames] aborted \"%s\"",normalized_path); 
        break;
# endif
# ifdef GLOB_NOMATCH
    case GLOB_NOMATCH: 
        pd_error(x,"[getfilenames] nothing found for \"%s\"",normalized_path); 
        break;
# endif
	}
#endif /* _WIN32 */
}


static void getfilenames_set(t_getfilenames* x, t_symbol *s) 
{
	DEBUG(post("getfilenames_set"););
#ifdef _WIN32
    char *patternBuffer;
    char envVarBuffer[FILENAME_MAX];
    if( (s->s_name[0] == '~') && (s->s_name[1] == '/'))
    {
        patternBuffer = getbytes(FILENAME_MAX);
        strcpy(patternBuffer,"%USERPROFILE%");
        strncat(patternBuffer, s->s_name + 1, FILENAME_MAX - 1);
        post("set: %s", patternBuffer);
    }
    else
    {
        patternBuffer = s->s_name;
    }
	ExpandEnvironmentStrings(patternBuffer, envVarBuffer, FILENAME_MAX - 2);
	x->x_pattern = gensym(envVarBuffer);
#else  // UNIX
    // TODO translate env vars to a full path using /bin/sh and exec
	x->x_pattern = s;
#endif /* _WIN32 */
}


static void getfilenames_symbol(t_getfilenames *x, t_symbol *s) 
{
    getfilenames_set(x,s);
    x->isnewpattern = 1;
}

static void getfilenames_bang(t_getfilenames *x) 
{
    if(x->isnewpattern) 
    {
        getfilenames_rewind(x);
        x->isnewpattern = 0;
    }
    if(x->current_glob_position < x->glob_buffer.gl_pathc) 
    {
		outlet_symbol( x->data_outlet, 
                       gensym(x->glob_buffer.gl_pathv[x->current_glob_position]) );
        x->current_glob_position++;
    }
    else
        outlet_bang(x->status_outlet);
}

static void *getfilenames_new(t_symbol *s)
{
	DEBUG(post("getfilenames_new"););

	t_getfilenames *x = (t_getfilenames *)pd_new(getfilenames_class);
	t_symbol *currentdir;
	char buffer[MAXPDSTRING];

    x->x_canvas =  canvas_getcurrent();

    symbolinlet_new(&x->x_obj, &x->x_pattern);
    x->data_outlet = outlet_new(&x->x_obj, &s_symbol);
    x->status_outlet = outlet_new(&x->x_obj, 0);
	
	/* set to the value from the object argument, if that exists */
	if (s != &s_)
	{
		x->x_pattern = s;
	}
	else
	{
		currentdir = canvas_getcurrentdir();
		strncpy(buffer,currentdir->s_name,MAXPDSTRING);
		strncat(buffer,"/*",MAXPDSTRING);
		x->x_pattern = gensym(buffer);
		post("setting pattern to default: *");
	}
    x->isnewpattern = 1;

	return (x);
}

static void getfilenames_free(t_getfilenames *x)
{
	globfree( &(x->glob_buffer) );
}

void getfilenames_setup(void) 
{
	DEBUG(post("getfilenames_setup"););
	getfilenames_class = class_new(gensym("getfilenames"), 
								  (t_newmethod)getfilenames_new, 
								  (t_method)getfilenames_free, 
								  sizeof(t_getfilenames), 
								  0, 
								  A_DEFSYMBOL, 
								  0);
	/* add inlet datatype methods */
	class_addbang(getfilenames_class, (t_method) getfilenames_bang);
	class_addsymbol(getfilenames_class, (t_method) getfilenames_symbol);
	
	/* add inlet message methods */
	class_addmethod(getfilenames_class, (t_method) getfilenames_set,
                    gensym("set"), A_DEFSYMBOL, 0);
	class_addmethod(getfilenames_class, (t_method) getfilenames_rewind,
                    gensym("rewind"), 0);
}

