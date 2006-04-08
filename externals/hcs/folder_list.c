/* --------------------------------------------------------------------------*/
/*                                                                           */
/* object for getting file listings using wildcard patterns                  */
/* Written by Hans-Christoph Steiner <hans@at.or.at>                         */
/*                                                                           */
/* Copyright (c) 2006 Hans-Christoph Steiner                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 2            */
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
#include <stdio.h>
#else
#include <stdlib.h>
#include <glob.h>
#endif

static char *version = "$Revision: 1.9 $";

t_int folder_list_instance_count;

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *folder_list_class;

typedef struct _folder_list {
	  t_object            x_obj;
      t_symbol            *x_pattern;
} t_folder_list;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

// TODO: regexp ~ to USERPROFILE for Windows
// TODO: make FindFirstFile display when its just a dir

static void folder_list_output(t_folder_list* x)
{
	DEBUG(post("folder_list_output"););

#ifdef _WIN32
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	DWORD errorNumber;
	LPVOID lpErrorMessage;
	char fullPathNameBuffer[MAX_PATH+1] = "";
	char unbashBuffer[MAX_PATH+1] = "";
	char pathBuffer[MAX_PATH+1] = "";
	int length;

// arg, looks perfect, but only in Windows Vista
//	GetFinalPathNameByHandle(hFind,fullPathNameBuffer,MAX_PATH,FILE_NAME_NORMALIZED);
	GetFullPathName(x->x_pattern->s_name,MAX_PATH,fullPathNameBuffer,NULL);
	sys_unbashfilename(fullPathNameBuffer,unbashBuffer);
	
	hFind = FindFirstFile(x->x_pattern->s_name, &findData);
	if (hFind == INVALID_HANDLE_VALUE) 
	{
	   errorNumber = GetLastError();
	   switch (errorNumber)
	   {
		  case ERROR_FILE_NOT_FOUND:
		  case ERROR_PATH_NOT_FOUND:
			 error("[folder_list] nothing found for \"%s\"",x->x_pattern->s_name);
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
			 error("[folder_list] %s", lpErrorMessage);
	   }
	   return;
	} 
	do {
		if( strcmp(findData.cFileName, ".") && strcmp(findData.cFileName, "..") ) 
		{
			length = strlen(unbashBuffer);
			do 
			{
				length--;
			} while ( *(unbashBuffer + length) == '/' );
			strncpy(pathBuffer, unbashBuffer, length);
			pathBuffer[length] = '\0';
			strcat(pathBuffer,findData.cFileName);
			outlet_symbol( x->x_obj.ob_outlet, gensym(pathBuffer) );
		}
	} while (FindNextFile(hFind, &findData) != 0);
	FindClose(hFind);
#else
	unsigned int i;
	glob_t glob_buffer;
	
	DEBUG(post("globbing %s",x->x_pattern->s_name););
	switch( glob( x->x_pattern->s_name, GLOB_TILDE, NULL, &glob_buffer ) )
	{
	   case GLOB_NOSPACE: 
		  error("[folder_list] out of memory for \"%s\"",x->x_pattern->s_name); 
		  break;
	   case GLOB_ABORTED: 
		  error("[folder_list] aborted \"%s\"",x->x_pattern->s_name); 
		  break;
	   case GLOB_NOMATCH: 
		  error("[folder_list] nothing found for \"%s\"",x->x_pattern->s_name); 
		  break;
	}
	for(i = 0; i < glob_buffer.gl_pathc; i++)
		outlet_symbol( x->x_obj.ob_outlet, gensym(glob_buffer.gl_pathv[i]) );
	globfree( &(glob_buffer) );
#endif
}


static void folder_list_set(t_folder_list* x, t_symbol *s) 
{
	DEBUG(post("folder_list_set"););
#ifdef _WIN32
	char string_buffer[MAX_PATH];
	ExpandEnvironmentStrings(s->s_name, string_buffer, MAX_PATH);
	x->x_pattern = gensym(string_buffer);
#else
	x->x_pattern = s;
#endif	
}


static void folder_list_symbol(t_folder_list *x, t_symbol *s) 
{
   folder_list_set(x,s);
   folder_list_output(x);
}


static void *folder_list_new(t_symbol *s) 
{
	DEBUG(post("folder_list_new"););

	t_folder_list *x = (t_folder_list *)pd_new(folder_list_class);
	
	if(!folder_list_instance_count) 
	{
		post("[folder_list] %s",version);  
		post("\twritten by Hans-Christoph Steiner <hans@at.or.at>");
	}
	folder_list_instance_count++;

	/* TODO set current dir of patch as default */
#ifdef _WIN32
	x->x_pattern = gensym(getenv("USERPROFILE"));
#else
	x->x_pattern = gensym(getenv("HOME"));
#endif

    symbolinlet_new(&x->x_obj, &x->x_pattern);
	outlet_new(&x->x_obj, &s_symbol);
	
	/* set to the value from the object argument, if that exists */
	if (s != &s_)
		x->x_pattern = s;
	
	return (x);
}

void folder_list_setup(void) 
{
	DEBUG(post("folder_list_setup"););
	folder_list_class = class_new(gensym("folder_list"), 
								  (t_newmethod)folder_list_new, 
								  0,
								  sizeof(t_folder_list), 
								  0, 
								  A_DEFSYM, 
								  0);
	/* add inlet datatype methods */
	class_addbang(folder_list_class,(t_method) folder_list_output);
	class_addsymbol(folder_list_class,(t_method) folder_list_symbol);
	
	/* add inlet message methods */
	class_addmethod(folder_list_class,(t_method) folder_list_set,gensym("set"), 
					A_DEFSYM, 0);
}

