#include <m_pd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <fnmatch.h>
#include <string.h>
#include <sys/stat.h>

static char *version = "$Revision: 1.1 $";

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *folder_list_class;

typedef struct _folder_list {
	t_object            x_obj;
	t_symbol            *x_folder_name;
	t_symbol            *x_pattern;
	DIR                 *x_folder_pointer;
} t_folder_list;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */


static t_int folder_list_open(t_folder_list* x) 
{
	DEBUG(post("folder_list_rewind"););

	if((x->x_folder_pointer = opendir(x->x_folder_name->s_name)) == NULL) {
        error("cannot open directory: %s\n", x->x_folder_name->s_name);
        return(0);
    }
	return(1);
}


static void folder_list_close(t_folder_list* x) 
{
	DEBUG(post("folder_list_close"););

	closedir(x->x_folder_pointer);
}

/*
static void folder_list_rewind(t_folder_list* x) 
{
	DEBUG(post("folder_list_rewind"););
	rewinddir(x->x_folder_pointer);
}
*/

static void folder_list_output(t_folder_list* x)
{
	DEBUG(post("folder_list_output"););
	struct dirent *entry;

	if( !folder_list_open(x) ) return;
    while( ( entry = readdir( x->x_folder_pointer ) ) != NULL) 
	{
		if( fnmatch( x->x_folder_name, entry->d_name, 0 ) )
			if( strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") ) 
				outlet_symbol(x->x_obj.ob_outlet, gensym(entry->d_name));
    }
	folder_list_close(x);
}


static void folder_list_symbol(t_folder_list *x, t_symbol *s) 
{
	x->x_folder_name = s;
	folder_list_output(x);
}


static void folder_list_set(t_folder_list* x, t_symbol *s) 
{
	DEBUG(post("folder_list_set"););
	x->x_folder_name = s;
}


static void folder_list_free(t_folder_list* x) 
{
	DEBUG(post("folder_list_free"););

}


static void *folder_list_new(t_symbol *s) 
{
	DEBUG(post("folder_list_new"););

	t_folder_list *x = (t_folder_list *)pd_new(folder_list_class);
	
	post("[folder_list] %s, written by Hans-Christoph Steiner <hans@at.or.at>",version);  
	
	/* init vars */
	x->x_folder_name = gensym(getenv("HOME"));

    symbolinlet_new(&x->x_obj, &x->x_folder_name);
	outlet_new(&x->x_obj, &s_symbol);
	
	/* set to the value from the object argument, if that exists */
	if (s != &s_)
		x->x_folder_name = s;
	
	return (x);
}

void folder_list_setup(void) 
{
	DEBUG(post("folder_list_setup"););
	folder_list_class = class_new(gensym("folder_list"), 
								  (t_newmethod)folder_list_new, 
								  (t_method)folder_list_free,
								  sizeof(t_folder_list), 
								  0, 
								  A_DEFSYM, 
								  0);
	/* add inlet datatype methods */
//	class_addfloat(folder_list_class,(t_method) folder_list_float);
	class_addbang(folder_list_class,(t_method) folder_list_output);
	class_addsymbol(folder_list_class,(t_method) folder_list_symbol);
	
	/* add inlet message methods */
/*	class_addmethod(folder_list_class, 
					(t_method) folder_list_rewind, 
					gensym("rewind"), 
					0);
	class_addmethod(folder_list_class,(t_method) folder_list_set,gensym("set"), 
  A_DEFSYM, 0);*/
}



/*
 * WINDOWS EXAMPLE
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnucmg/html/UCMGch09.asp

#include <windows.h>
#include <stdio.h>

void ScanDir(char *dirname, int indent)
{
    BOOL            fFinished;
    HANDLE          hList;
    TCHAR           szDir[MAX_PATH+1];
    TCHAR           szSubDir[MAX_PATH+1];
    WIN32_FIND_DATA FileData;

    // Get the proper directory path
    sprintf(szDir, "%s\\*", dirname);

    // Get the first file
    hList = FindFirstFile(szDir, &FileData);
    if (hList == INVALID_HANDLE_VALUE)
    { 
        printf("No files found\n\n");
    }
    else
    {
        // Traverse through the directory structure
        fFinished = FALSE;
        while (!fFinished)
        {
            // Check the object is a directory or not
            if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((strcmp(FileData.cFileName, ".") != 0) &&
(strcmp(FileData.cFileName, "..") != 0))
                {
                    printf("%*s%s\\\n", indent, "",
                      FileData.cFileName);

                    // Get the full path for sub directory
                    sprintf(szSubDir, "%s\\%s", dirname,
                      FileData.cFileName);

                    ScanDir(szSubDir, indent + 4);
                }
            }
            else
                printf("%*s%s\n", indent, "", FileData.cFileName);


            if (!FindNextFile(hList, &FileData))
            {
                if (GetLastError() == ERROR_NO_MORE_FILES)
                {
                    fFinished = TRUE;
                }
            }
        }
    }

    FindClose(hList);
}

*/
