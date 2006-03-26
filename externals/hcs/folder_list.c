#include <m_pd.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <stdio.h>
#else
#include <stdlib.h>
#include <glob.h>
#endif

static char *version = "$Revision: 1.6 $";

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
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	DWORD errorNumber;
	LPVOID lpErrorMessage;
	
	hFind = FindFirstFile(x->x_pattern->s_name, &FindFileData);
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
	   outlet_symbol( x->x_obj.ob_outlet, gensym(FindFileData.cFileName) );
	} while (FindNextFile(hFind, &FindFileData) != 0);
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
	
	post("[folder_list] %s",version);  
	post("\twritten by Hans-Christoph Steiner <hans@at.or.at>");
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

