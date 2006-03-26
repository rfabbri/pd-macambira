#include <m_pd.h>
#include <stdlib.h>
#include <glob.h>

static char *version = "$Revision: 1.2 $";

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *folder_list_class;

typedef struct _folder_list {
	t_object            x_obj;
	t_symbol            *x_pattern;
	glob_t              x_glob;
} t_folder_list;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */


static void folder_list_output(t_folder_list* x)
{
	DEBUG(post("folder_list_output"););
	t_int i;

	DEBUG(post("globbing %s",x->x_pattern->s_name););
	switch( glob( x->x_pattern->s_name, GLOB_TILDE, NULL, &(x->x_glob) ) )
	{
	case GLOB_NOSPACE: 
		error("[folder_list] out of memory"); break;
	case GLOB_ABORTED: 
		error("[folder_list] aborted"); break;
	case GLOB_NOMATCH: 
		error("[folder_list] no match"); break;
	}
	for(i = 0; i < x->x_glob.gl_matchc; i++)
		outlet_symbol( x->x_obj.ob_outlet, gensym(x->x_glob.gl_pathv[i]) );
}


static void folder_list_symbol(t_folder_list *x, t_symbol *s) 
{
	x->x_pattern = s;
	folder_list_output(x);
}


static void folder_list_set(t_folder_list* x, t_symbol *s) 
{
	DEBUG(post("folder_list_set"););
	x->x_pattern = s;
}


static void folder_list_free(t_folder_list* x) 
{
	DEBUG(post("folder_list_free"););

	globfree( &(x->x_glob) );
}


static void *folder_list_new(t_symbol *s) 
{
	DEBUG(post("folder_list_new"););

	t_folder_list *x = (t_folder_list *)pd_new(folder_list_class);
	
	post("[folder_list] %s, written by Hans-Christoph Steiner <hans@at.or.at>",version);  
	/* set HOME as default */
	x->x_pattern = gensym(getenv("HOME"));

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
								  (t_method)folder_list_free,
								  sizeof(t_folder_list), 
								  0, 
								  A_DEFSYM, 
								  0);
	/* add inlet datatype methods */
//	class_addfloat(folder_list_class,(t_method) glob_float);
	class_addbang(folder_list_class,(t_method) folder_list_output);
	class_addsymbol(folder_list_class,(t_method) folder_list_symbol);
	
	/* add inlet message methods */
	class_addmethod(folder_list_class,(t_method) folder_list_set,gensym("set"), 
					A_DEFSYM, 0);
}

