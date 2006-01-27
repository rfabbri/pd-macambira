// mono
extern "C" {
#include <mono/jit/jit.h>
#include <mono/metadata/object.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/class.h>
#include <mono/metadata/metadata.h>
}

#ifdef _MSC_VER
#pragma warning(disable: 4091)
#endif
#include <m_pd.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <io.h> // for _close

#include <map>

#if 0

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// OLD VERSION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <time.h>
#include <math.h>


#define MAX_SELECTORS 256
#define MAX_OUTLETS 32

void *clr_new(t_symbol *s, int argc, t_atom *argv);

typedef struct _clr
{
    t_object x_obj; // myself
	t_outlet *l_out;
	t_symbol *assemblyname;
	t_symbol *filename;
	int loaded;
	// mono stuff
	MonoAssembly *assembly;
	MonoObject *obj;
	MonoImage *image;
	MonoMethod *method, *setUp, *manageBang, *manageSymbol, *manageFloat, *manageList;
	MonoClass *klass;
	int n;

	// TODO: dynamic memory allocation
	selectorList selectors[MAX_SELECTORS];
	outletList outlets[MAX_OUTLETS];

} t_clr;

// list of mono methods
typedef struct
{
	char *sel; // the selector
	MonoMethod *func; // the function
	int type;
//	selectorList *next; // next element of the list
} selectorList;

// list of outlets
typedef struct 
{
	t_outlet *outlet_pointer;
//	outletList *next; // next element of the list
} outletList;

// simplyfied atom
typedef enum
{
    A_S_NULL=0,
    A_S_FLOAT=1,
    A_S_SYMBOL=2,
}  t_atomtype_simple;

typedef struct 
{
	//t_atomtype_simple a_type;
	int a_type;
	float float_value;
	MonoString *string_value;

} atom_simple;




// mono functions

static void mono_clean(t_clr *x)
{
	// clean up stuff
	//mono_jit_cleanup (x->domain);
	//mono_domain_free(x->domain);
}

void registerMonoMethod(void *x, MonoString *selectorString, MonoString *methodString, int type);
void createInlet(void *x1, MonoString *selectorString, int type);
void createOutlet(void *x1, int type);
void out2outlet(void *x1, int outlet, int atoms_length, MonoArray *array);
void post2pd(MonoString *mesString);
void error2pd(MonoString *mesString);


// load the variables and init mono
static void mono_load(t_clr *x, int argc, t_atom *argv)
{
//	const char *file="D:\\Davide\\cygwin\\home\\Davide\\externalTest1.dll";
	//const char *file="External.dll";
	
	MonoMethod *m = NULL, *ctor = NULL, *fail = NULL, *mvalues;
	MonoClassField *classPointer;
	gpointer iter;
	gpointer *args;
	int val;
	int i,j;
	int params;
	t_symbol *strsymbol;
	atom_simple *listparams;

	if (x->loaded == 0)
	{
		error("assembly not specified");
		return;
	}

	// prepare the selectors list
	for (i=0; i<MAX_SELECTORS; i++)
	{
		x->selectors[i].sel = 0;
		x->selectors[i].func = 0;
	}
	// and the outlets list
	for (i=0; i<MAX_OUTLETS; i++)
	{
		x->outlets[i].outlet_pointer = 0;
	}

	//mono_set_dirs (NULL, NULL);
	//mono_config_parse (NULL);

	//mono_jit_init (random_name_str);
//	x->domain = mono_domain_get();
	// all done without errors
	x->loaded = 1;

}

/*
// call the method
static void clr_bang(t_clr *x) {

	gpointer args [2];
	int val;
	MonoObject *result;

	val = x->n;
	args [0] = &val;

	if (x->method)
	{
		result = mono_runtime_invoke (x->method, x->obj, args, NULL);
		val = *(int*)mono_object_unbox (result);
		x->n = val;
	//	outlet_float(x->l_out, (float) x->n);
	}


}
*/

// finds a method from its name
void findMonoMethod( MonoClass *klass, char *function_name, MonoMethod **met)
{
	int trovato;
	MonoMethod *m = NULL;
	gpointer iter;
	iter = NULL;
	while (m = mono_class_get_methods (klass, &iter)) 
	{
		if (strcmp (mono_method_get_name (m), function_name) == 0) 
		{
			*met = m;
//printf("%s trovata\n", function_name);
			return;
		} 	
	}
}

void clr_free(t_clr *x)
{
	mono_clean(x);
}

static void clr_method_bang(t_clr *x) 
{
	if (x->loaded == 0)
	{
		error("assembly not specified");
		return;
	}
	if (x->manageBang)
	{
		mono_runtime_invoke (x->manageBang, x->obj, NULL, NULL);
	}
}

static void clr_method_symbol(t_clr *x, t_symbol *sl) 
{
	gpointer args [1];
	MonoString *strmono;
	strmono = mono_string_new (monodomain, sl->s_name);
	args[0] = &strmono;
	if (x->loaded == 0)
	{
		error("assembly not specified");
		return;
	}
	if (x->manageSymbol)
	{
		mono_runtime_invoke (x->manageSymbol, x->obj, args, NULL);
	}
}

static void clr_method_float(t_clr *x, float f) 
{
	gpointer args [1];
	args [0] = &f;
	if (x->loaded == 0)
	{
		error("assembly not specified");
		return;
	}
	if (x->manageFloat)
	{
		mono_runtime_invoke (x->manageFloat, x->obj, args, NULL);
	}
}

// here i look for the selector and call the right mono method
static void clr_method_list(t_clr *x, t_symbol *sl, int argc, t_atom *argv)
{
	gpointer args [2];
	int valInt;
	float valFloat;

	int i;
	if (x->loaded == 0)
	{
		error("assembly not specified");
		return;
	}

	// check if a selector is present !
	//printf("clr_manage_list, got symbol = %s\n", sl->s_name);
	if (strcmp("list", sl->s_name) == 0)
	{
		printf("lista senza selector");
		if (x->manageList)
		{
			gpointer args [1];
			MonoString *stringtmp;
			double *floattmp;
			t_atomtype_simple *typetmp;
			t_symbol *strsymbol;
			MonoArray *atoms;
			atom_simple *atom_array;
			int j;		
			char strfloat[256], strnull[256];
			sprintf(strfloat, "float");
			sprintf(strnull, "null");
			atom_array = malloc(sizeof(atom_simple)*argc);
			atoms = mono_array_new (monodomain, clr_atom, argc);
			for (j=0; j<argc; j++)
			{
				switch ((argv+j)->a_type)
				{
				case A_FLOAT:
					atom_array[j].a_type =  A_S_FLOAT;
					atom_array[j].float_value = (double) atom_getfloat(argv+j);
					atom_array[j].string_value = mono_string_new (monodomain, strfloat);
					break;
				case A_SYMBOL:
					atom_array[j].a_type =  A_S_SYMBOL;
					strsymbol = atom_getsymbol(argv+j);
					atom_array[j].string_value = mono_string_new (monodomain, strsymbol->s_name);
					atom_array[j].float_value = 0;
					break;
				default:
					atom_array[j].a_type =  A_S_NULL;
					atom_array[j].float_value = 0;
					atom_array[j].string_value = mono_string_new (monodomain, strnull);
				}
				mono_array_set (atoms, atom_simple , j, atom_array[j]);
			}

			args[0] = atoms;
			mono_runtime_invoke (x->manageList, x->obj, args, NULL);
			return;
		} else
		{
			error("you did not specified a function to call for lists without selectors");
			return;
		}
	}
	for (i=0; i<MAX_SELECTORS; i++)
	{
		if (strcmp(x->selectors[i].sel, sl->s_name) == 0)
		{
			// I've found the selector!
			if (x->selectors[i].func)
			{
				// ParametersType {None = 0, Float=1, Symbol=2, List=3};
				switch (x->selectors[i].type)
				{
				case 0:
					mono_runtime_invoke (x->selectors[i].func, x->obj, NULL, NULL);
					break;
				case 1:
					{
						gpointer args [1];
						float val = atom_getfloat(argv);
						args[0] = &val;
						mono_runtime_invoke (x->selectors[i].func, x->obj, args, NULL);
						break;
					}
				case 2:
					{
						gpointer args [1];
						t_symbol *strsymbol;
						MonoString *strmono;
						strsymbol = atom_getsymbol(argv);
						strmono = mono_string_new (monodomain, strsymbol->s_name);
						args[0] = &strmono;
						mono_runtime_invoke (x->selectors[i].func, x->obj, args, NULL);
						// memory leak ?
						break;
					}
				case 3:
					{
						gpointer args [1];
						MonoString *stringtmp;
						double *floattmp;
						t_atomtype_simple *typetmp;
						t_symbol *strsymbol;
						MonoArray *atoms;
						atom_simple *atom_array;
						int j;		
						char strfloat[256], strnull[256];
						sprintf(strfloat, "float");
						sprintf(strnull, "null");
						atom_array = malloc(sizeof(atom_simple)*argc);
						atoms = mono_array_new (monodomain, clr_atom, argc);
						for (j=0; j<argc; j++)
						{
							switch ((argv+j)->a_type)
							{
							case A_FLOAT:
								atom_array[j].a_type =  A_S_FLOAT;
								atom_array[j].float_value = (double) atom_getfloat(argv+j);
								atom_array[j].string_value = mono_string_new (monodomain, strfloat);
								break;
							case A_SYMBOL:
								atom_array[j].a_type =  A_S_SYMBOL;
								strsymbol = atom_getsymbol(argv+j);
								atom_array[j].string_value = mono_string_new (monodomain, strsymbol->s_name);
								atom_array[j].float_value = 0;
								break;
							default:
								atom_array[j].a_type =  A_S_NULL;
								atom_array[j].float_value = 0;
								atom_array[j].string_value = mono_string_new (monodomain, strnull);
							}
							mono_array_set (atoms, atom_simple , j, atom_array[j]);
						}

						args[0] = atoms;
						mono_runtime_invoke (x->selectors[i].func, x->obj, args, NULL);
						break;
					}
				}
				return;
			}
		}
		if (x->selectors[i].func == 0)
			i = MAX_SELECTORS;
	}
	error("clr: selector not recognized");
}

// this function is called by mono when it wants to register a selector callback
void registerMonoMethod(void *x1, MonoString *selectorString, MonoString *methodString, int type)
{
	
	char *selCstring, *metCstring;
	MonoMethod *met;
	t_clr *x;
	int this_selector, i;
	//int ret;

	
    selCstring = mono_string_to_utf8 (selectorString);
	metCstring = mono_string_to_utf8 (methodString);
	
	x = (t_clr *)x1;

	//ret = 0;
	met = 0;
	findMonoMethod( x->klass, metCstring, &met);

	if (!met)
	{
		error("method not found!");
		return;
	}

	//post("registerMonoMethod: associating %s to %s", selCstring, metCstring);

	if (selectorString->length == 0)
	{
		switch (type)
		{
		case 1: // float
			x->manageFloat = met;
			break;
		case 2: // string
			x->manageSymbol = met;
			break;
		case 3: // list
			x->manageList = met;
			break;
		case 4: // bang
			x->manageBang = met;
			break;
		}
	}

	this_selector = MAX_SELECTORS;
	for (i = 0; i < MAX_SELECTORS; i++)
	{
		if (x->selectors[i].func == 0)
		{
			// i found the first empty space
			this_selector = i;
			i = MAX_SELECTORS;
		}
		
	}
	if (this_selector == MAX_SELECTORS)
	{
		error("not enough space for selectors! MAX_SELECTORS = %i", MAX_SELECTORS);
		return;
	}

	x->selectors[this_selector].sel = selCstring;
	x->selectors[this_selector].func = met;
	x->selectors[this_selector].type = type;

	class_addmethod(clr_class, (t_method)clr_method_list, gensym(selCstring), A_GIMME, 0);


}

// this function is called by mono when it wants to create a new inlet
void createInlet(void *x1, MonoString *selectorString, int type)
{
	// public enum ParametersType {None = 0, Float=1, Symbol=2, List=3, Bang=4, Generic=5};
	char *selCstring;
	char typeString[256];
	t_clr *x;

    selCstring = mono_string_to_utf8 (selectorString);	
	x = (t_clr *)x1;
	if (x->loaded == 0)
	{
		error("assembly not specified");
		return;
	}
	switch (type)
	{
	case 1:
		sprintf(typeString, "float");
		break;
	case 2:
		sprintf(typeString, "symbol");
		break;
	case 3:
		sprintf(typeString, "list");
		break;
	case 4:
		sprintf(typeString, "bang");
		break;
	default:
		sprintf(typeString, "");
		break;
	}
	// create the inlet
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym(typeString), gensym(selCstring));
}

// this function is called by mono when it wants to create a new outlet
void createOutlet(void *x1, int type)
{
	t_clr *x;
	int i = 0;
	char typeString[256];
	x = (t_clr *)x1;
	if (x->loaded == 0)
	{
		error("assembly not specified");
		return;
	}
	// public enum ParametersType {None = 0, Float=1, Symbol=2, List=3, Bang=4, Generic=5};
	switch (type)
	{
	case 1:
		sprintf(typeString, "float");
		break;
	case 2:
		sprintf(typeString, "symbol");
		break;
	case 3:
		sprintf(typeString, "list");
		break;
	case 4:
		sprintf(typeString, "bang");
		break;
	default:
		sprintf(typeString, "");
		break;
	}

	while (i < MAX_OUTLETS)
	{
		if (x->outlets[i].outlet_pointer == 0)
		{
			// empty space found!
			x->outlets[i].outlet_pointer = outlet_new(&x->x_obj, gensym(typeString));
			return;
		}
		i++;
	}
	error("the maximum number of outlets has been reached (%i)", MAX_OUTLETS);
	
}

// out to outlet
void out2outlet(void *x1, int outlet, int atoms_length, MonoArray *array)
{
	t_clr *x;
	t_atom *lista;
	atom_simple *atoms;
	int n;
	x = (t_clr *)x1;
	if (x->loaded == 0)
	{
		error("assembly not specified");
		return;
	}
	if ((outlet>MAX_OUTLETS) || (outlet<0))
	{
		error("outlet number out of range, max is %i", MAX_OUTLETS);
		return;
	}
	if (x->outlets[outlet].outlet_pointer == 0)
	{
		error("outlet %i not registered", outlet);
		return;
	}
	lista = (t_atom *) malloc(sizeof(t_atom) * atoms_length);
	atoms = (atom_simple *) malloc(sizeof(atom_simple) * atoms_length);
	for (n=0; n<atoms_length; n++)
	{
		char *mesCstring;
		atoms[n] = mono_array_get(array, atom_simple, n);
		switch (atoms[n].a_type)
		{
			case A_S_NULL:
				SETFLOAT(lista+n, (float) 0);
				break;
			case A_S_FLOAT:
				SETFLOAT(lista+n, (float) atoms[n].float_value);
				break;
			case A_S_SYMBOL:
				mesCstring = mono_string_to_utf8 (atoms[n].string_value);
				SETSYMBOL(lista+n, gensym(mesCstring));
				break;
		}

	}
	outlet_anything(x->outlets[outlet].outlet_pointer,
					gensym("list") ,
					atoms_length, 
					lista);
	free(lista);
}

// this function is called by mono when it wants post messages to pd
void post2pd(MonoString *mesString)
{
	
	char *mesCstring;
    mesCstring = mono_string_to_utf8 (mesString);
	post(mesCstring);
	
}

// this function is called by mono when it wants to post errors to pd
void error2pd(MonoString *mesString)
{
	
	char *mesCstring;
    mesCstring = mono_string_to_utf8 (mesString);
	error(mesCstring);
}
/*
//void ext_class_addbang(const char *funcName)
void ext_class_addbang(t_method funcName)
{
//	class_addbang(clr_class, (t_method)clr_bang);
	class_addbang(clr_class, funcName);
}*/

void *clr_new(t_symbol *s, int argc, t_atom *argv)
{

	int i;
	time_t a;
    t_clr *x = (t_clr *)pd_new(clr_class);

	char strtmp[256];
	x->manageBang = 0;
	x->manageSymbol = 0;
	x->manageFloat = 0;
	x->manageList = 0;
	
	if (argc==0)
	{
		x->loaded = 0;
		error("clr: you must provide a class name as the first argument");
		return (x);
	}
	x->loaded = 1;
	x->assemblyname = atom_gensym(argv);
	
	if (argc<2)
	{
		// only main class passed
		// filename by default
		sprintf(strtmp, "%s.dll", x->assemblyname->s_name);
		x->filename = gensym(strtmp);
        printf(" used did not specified filename, I guess it is %s\n", strtmp);
	} else
	{
		x->filename = atom_gensym(argv+1);
	}

    x->obj = mono_object_new (monodomain, x->klass);

	// load mono, init the needed vars
//	mono_load(x, argc, argv);
	// now call the class constructor
	if (ctor)
	{
		args = malloc(sizeof(gpointer)*params);
		listparams = malloc(sizeof(atom_simple)*params);
		for (j=2; j<argc; j++)
		{
			switch ((argv+j)->a_type)
			{
			case A_FLOAT:
				listparams[j-2].a_type =  A_S_FLOAT;
				listparams[j-2].float_value = (double) atom_getfloat(argv+j);
				args[j-2] = &(listparams[j-2].float_value);
				break;
			case A_SYMBOL:
				listparams[j-2].a_type =  A_S_SYMBOL;
				strsymbol = atom_getsymbol(argv+j);
				listparams[j-2].string_value = mono_string_new (monodomain, strsymbol->s_name);
				args[j-2] = listparams[j-2].string_value;
				break;
			}
		}
		mono_runtime_invoke (ctor, x->obj, args, NULL);
		free(listparams);
		free(args);
	} else
	{
		error("%s doesn't have a constructor with %i parameters!",x->assemblyname->s_name, params);
	}


    return (x);
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// NEW VERSION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static MonoDomain *monodomain;
static MonoClass *clr_atom,*clr_symbol;
static MonoClassField *clr_symbol_ptr;
//static MonoMethod *clr_symbol_ctor;
static MonoMethodDesc *clr_desc_main,*clr_desc_ctor;
static MonoMethodDesc *clr_desc_bang,*clr_desc_float,*clr_desc_symbol,*clr_desc_pointer,*clr_desc_list,*clr_desc_anything;

typedef std::map<t_symbol *,MonoMethod *> ClrMethodMap;

// this is the class structure
// holding the pd and mono class
// and our CLR method pointers
typedef struct
{
    t_class *pd_class;
    MonoClass *mono_class;
    MonoClassField *mono_obj_field;
    MonoMethod *mono_ctor;
    MonoMethod *method_bang,*method_float, *method_symbol,*method_list,*method_pointer,*method_anything;
//    ClrMethodMap *methods;
} t_clr_class;

typedef std::map<t_symbol *,t_clr_class *> ClrMap;
// this holds the class name to class structure association
static ClrMap clr_map;

// this is the class to be setup (while we are in the CLR static Main method)
static t_clr_class *clr_setup_class = NULL;

// this is the class instance object structure
typedef struct
{
    t_object pd_obj; // myself
    t_clr_class *clr_clss; // pointer to our class structure
    MonoObject *mono_obj;
} t_clr;



static void clr_method_bang(t_clr *x) 
{
    assert(x && x->clr_clss && x->clr_clss->method_bang);
    mono_runtime_invoke(x->clr_clss->method_bang,x->mono_obj,NULL,NULL);
}

static void clr_method_float(t_clr *x,t_float f) 
{
    assert(x && x->clr_clss && x->clr_clss->method_float);
	gpointer args = &f;
    mono_runtime_invoke(x->clr_clss->method_float,x->mono_obj,&args,NULL);
}

static void clr_method_symbol(t_clr *x,t_symbol *s) 
{
    assert(x && x->clr_clss && x->clr_clss->method_symbol);
    MonoObject *symobj = mono_object_new(monodomain,clr_symbol);
//	gpointer args = &s;
//    mono_runtime_invoke(clr_symbol_ctor,symobj,&args,NULL);
    // setting the field directly doesn't seem to work?!
    mono_field_set_value(symobj,clr_symbol_ptr,s);
	gpointer args = symobj;
    mono_runtime_invoke(x->clr_clss->method_symbol,x->mono_obj,&args,NULL);
}

static void clr_method_list(t_clr *x,int argc, t_atom *argv)
{
    assert(x && x->clr_clss && x->clr_clss->method_list);
}

static void clr_method_pointer(t_clr *x,t_gpointer *ptr)
{
    assert(x && x->clr_clss && x->clr_clss->method_pointer);
}

static void clr_method_anything(t_clr *x,t_symbol *sl, int argc, t_atom *argv)
{
#if 0
    assert(x && x->clr_clss);
    ClrMethodMap *methods = x->clr_clss->methods;
    if(methods) {
        ClrMethodMap::iterator it = methods->find(sl);
        if(it != methods->end()) {
            // \TODO call m
            return;
        }
    }
    if(x->clr_clss->method_anything) {
         // \TODO call methodanything
    }
    else
        post("CLR - no method for %s found",sl->s_name);
#else
    assert(x && x->clr_clss && x->clr_clss->method_anything);
#endif
}


// this function is called by mono when it wants post messages to pd
static void PD_Post(MonoString *str)
{
	post("%s",mono_string_to_utf8(str));	
}

// this function is called by mono when it wants post messages to pd
static void PD_PostError(MonoString *str)
{
	error("%s",mono_string_to_utf8(str));	
}

// this function is called by mono when it wants post messages to pd
static void PD_PostBug(MonoString *str)
{
	bug("%s",mono_string_to_utf8(str));	
}

// this function is called by mono when it wants post messages to pd
static void PD_PostVerbose(int lvl,MonoString *str)
{
	verbose(lvl,"%s",mono_string_to_utf8(str));	
}

static void *PD_GenSym(MonoString *str)
{
	return gensym(mono_string_to_utf8(str));	
}

static MonoString *PD_EvalSym(MonoObject *symobj)
{
    t_symbol *sym;
    mono_field_get_value(symobj,clr_symbol_ptr,&sym);
    return mono_string_new(monodomain,sym->s_name);
}


#if 0
// this function is called by mono when it wants post messages to pd
static void PD_AddMethod(MonoObject *symobj,MonoObject *obj)
{
    assert(clr_setup_class);

//    char *tag = mono_string_to_utf8(str);
//	post("register method %s",tag);
    t_symbol *sym;
    mono_field_get_value(symobj,clr_symbol_ptr,&sym);

    // \TODO convert from obj
    MonoMethod *m = NULL;

    if(sym == &s_bang) {
        if(!clr_setup_class->method_bang)
    	    class_addbang(clr_setup_class->pd_class,clr_method_bang);
        clr_setup_class->method_bang = m;
    }
    else if(sym == &s_float) {
        if(!clr_setup_class->method_bang)
    	    class_addfloat(clr_setup_class->pd_class,clr_method_float);
        clr_setup_class->method_bang = m;
    }
    else if(sym == &s_symbol) {
        if(!clr_setup_class->method_symbol)
    	    class_addsymbol(clr_setup_class->pd_class,clr_method_symbol);
        clr_setup_class->method_symbol = m;
    }
    else if(sym == &s_list) {
        if(!clr_setup_class->method_list)
    	    class_addlist(clr_setup_class->pd_class,clr_method_list);
        clr_setup_class->method_list = m;
    }
    else if(sym == &s_pointer) {
        if(!clr_setup_class->method_pointer)
        	class_addpointer(clr_setup_class->pd_class,clr_method_pointer);
        clr_setup_class->method_pointer = m;
    }
    else if(sym == &s_) {
        if(!clr_setup_class->method_anything && !clr_setup_class->methods) // only register once!
	        class_addanything(clr_setup_class->pd_class,clr_method_anything);
        clr_setup_class->method_anything = m;
    }
    else {
        if(!clr_setup_class->methods) {
            // no methods yet
            clr_setup_class->methods = new ClrMethodMap;
            if(!clr_setup_class->method_anything) // only register once!
    	        class_addanything(clr_setup_class->pd_class,clr_method_anything);
        }

        // add tag to map
        (*clr_setup_class->methods)[sym] = m;
    }
}
#endif

void *clr_new(t_symbol *classname, int argc, t_atom *argv)
{
    // find class name in map
    ClrMap::iterator it = clr_map.find(classname);
    if(it == clr_map.end()) {
        error("CLR class %s not found",classname->s_name);
        return NULL;
    }

    t_clr_class *clss = it->second;

    // make instance
    t_clr *x = (t_clr *)pd_new(clss->pd_class);
    x->mono_obj = mono_object_new (monodomain,clss->mono_class);
    if(!x->mono_obj) {
        pd_free((t_pd *)x);
        error("CLR class %s could not be instantiated",classname->s_name);
        return NULL;
    }

    // store class pointer
    x->clr_clss = clss;

    // store our object pointer in External::ptr member
    mono_field_set_value(x->mono_obj,clss->mono_obj_field,x);

    // ok, we have an object - look for constructor
	if(clss->mono_ctor) {
        // call static Main method
        MonoObject *ret = mono_runtime_invoke(clss->mono_ctor,x->mono_obj,NULL,NULL);
        if(ret) {
            post("Warning: returned value from %s::.ctor ignored",classname->s_name);
            // ??? do we have to mark ret as free?
        }
    }
    else
        post("Warning: no %s.Main method found",classname);

    return x;
}

void clr_free(t_clr *x)
{
}

static int classloader(char *dirname, char *classname)
{
    t_clr_class *clr_class = NULL;
    t_symbol *classsym;
    MonoAssembly *assembly;
    MonoImage *image;
    MonoMethod *method;

    char dirbuf[MAXPDSTRING],*nameptr;
    // search for classname.dll in the PD path
    int fd;
    if ((fd = open_via_path(dirname, classname, ".dll", dirbuf, &nameptr, MAXPDSTRING, 1)) < 0)
        // not found
        goto bailout;

    // found
    _close(fd);

    clr_class = (t_clr_class *)getbytes(sizeof(t_clr_class));

//    clr_class->methods = NULL;

    // try to load assembly
    char path[MAXPDSTRING];
    strcpy(path,dirname);
    strcat(path,"/");
//    strcat(path,dirbuf);
//    strcat(path,"/");
    strcat(path,nameptr);

    assembly = mono_domain_assembly_open(monodomain,path);
	if(!assembly) {
		error("clr: file %s couldn't be loaded!",path);
		goto bailout;
	}

    image = mono_assembly_get_image(assembly);
    assert(image);

    // try to find class
    // "" means no namespace
	clr_class->mono_class = mono_class_from_name(image,"",classname);
	if(!clr_class->mono_class) {
		error("Can't find %s class in %s\n",classname,mono_image_get_filename(image));
		goto bailout;
	}

    clr_class->mono_obj_field = mono_class_get_field_from_name(clr_class->mono_class,"ptr");
    assert(clr_class->mono_obj_field);

    // ok

	post("CLR class %s loaded",classname);

    // make new class (with classname)
    classsym = gensym(classname);
    clr_class->pd_class = class_new(classsym,(t_newmethod)clr_new,(t_method)clr_free, sizeof(t_clr), CLASS_DEFAULT, A_GIMME, 0);

    // find static Main method

    method = mono_method_desc_search_in_class(clr_desc_main,clr_class->mono_class);
	if(method) {
        // set current class
        clr_setup_class = clr_class;

        // call static Main method
        MonoObject *ret = mono_runtime_invoke(method,NULL,NULL,NULL);

        // unset current class
        clr_setup_class = NULL;

        if(ret) {
            post("CLR - Warning: returned value from %s.Main ignored",classname);
            // ??? do we have to mark ret as free?
        }
    }
    else
        post("CLR - Warning: no %s.Main method found",classname);

    // find and save constructor
    clr_class->mono_ctor = mono_method_desc_search_in_class(clr_desc_ctor,clr_class->mono_class);

    // find && register methods
    if((clr_class->method_bang = mono_method_desc_search_in_class(clr_desc_bang,clr_class->mono_class)) != NULL)
        class_addbang(clr_class->pd_class,clr_method_bang);
    if((clr_class->method_float = mono_method_desc_search_in_class(clr_desc_float,clr_class->mono_class)) != NULL)
        class_addfloat(clr_class->pd_class,clr_method_float);
    if((clr_class->method_symbol = mono_method_desc_search_in_class(clr_desc_symbol,clr_class->mono_class)) != NULL)
        class_addsymbol(clr_class->pd_class,clr_method_symbol);
//    if((clr_class->method_pointer = mono_class_get_method_from_name(clr_class->mono_class,"MethodPointer",1)) != NULL)
    if((clr_class->method_pointer = mono_method_desc_search_in_class(clr_desc_pointer,clr_class->mono_class)) != NULL)
        class_addpointer(clr_class->pd_class,clr_method_pointer);
    if((clr_class->method_list = mono_method_desc_search_in_class(clr_desc_list,clr_class->mono_class)) != NULL)
        class_addlist(clr_class->pd_class,clr_method_list);
    if((clr_class->method_anything = mono_method_desc_search_in_class(clr_desc_anything,clr_class->mono_class)) != NULL)
        class_addanything(clr_class->pd_class,clr_method_anything);


    // put into map
    clr_map[classsym] = clr_class;

    post("Loaded class %s OK",classname);

    return 1;

bailout:
    if(clr_class) freebytes(clr_class,sizeof(t_clr_class));

    return 0;
}

extern "C"
#ifdef _MSC_VER
__declspec(dllexport) 
#endif
void clr_setup(void)
{
#ifdef _WIN32
    // set mono paths
    const char *monopath = getenv("MONO_PATH");
    if(!monopath) {
        error("CLR - Please set the MONO_PATH environment variable to the folder of your MONO installation - CLR not loaded!");
        return;
    }
    
    char tlib[256],tconf[256];
    strcpy(tlib,monopath);
    strcat(tlib,"/lib");
    strcpy(tconf,monopath);
    strcat(tconf,"/etc");
    mono_set_dirs(tlib,tconf);
#endif

    monodomain = mono_jit_init("PureData");
	if(monodomain) {
    /*	
	    // add mono to C hooks
	    mono_add_internal_call ("PureData.pd::RegisterSelector", registerMonoMethod);
	    mono_add_internal_call ("PureData.pd::ToOutlet", out2outlet);
	    mono_add_internal_call ("PureData.pd::PostMessage", post2pd);
	    mono_add_internal_call ("PureData.pd::ErrorMessage", error2pd);
	    mono_add_internal_call ("PureData.pd::CreateOutlet", createOutlet);
	    mono_add_internal_call ("PureData.pd::CreateInlet", createInlet);
    */
//	    mono_add_internal_call ("PureData.Core::AddMethod", PD_AddMethod);

        mono_add_internal_call("PureData.Core::Post",(const void *)PD_Post);
	    mono_add_internal_call("PureData.Core::PostError",(const void *)PD_PostError);
	    mono_add_internal_call("PureData.Core::PostBug",(const void *)PD_PostBug);
	    mono_add_internal_call("PureData.Core::PostVerbose",(const void *)PD_PostVerbose);

	    mono_add_internal_call("PureData.Core::GenSym", PD_GenSym);
	    mono_add_internal_call("PureData.Core::EvalSym", PD_EvalSym);


        MonoAssembly *assembly = mono_domain_assembly_open (monodomain, "PureData.dll");
	    if(!assembly) {
		    error("clr: assembly PureData.dll not found!");
		    return;
	    }

	    MonoImage *image = mono_assembly_get_image(assembly);
        assert(image);

        // load important classes
        clr_atom = mono_class_from_name(image,"PureData","Atom");
        assert(clr_atom);
        clr_symbol = mono_class_from_name(image,"PureData","Symbol");
        assert(clr_symbol);
        clr_symbol_ptr = mono_class_get_field_from_name(clr_symbol,"ptr");
        assert(clr_symbol_ptr);
//        MonoMethodDesc *d = mono_method_desc_new("::.ctor(System.IntPtr)",FALSE);
//        assert(d);
//        clr_symbol_ctor = mono_method_desc_search_in_class(d,clr_symbol);
//        assert(clr_symbol_ctor);

        clr_desc_main = mono_method_desc_new("::Main",FALSE);
        assert(clr_desc_main);
        clr_desc_ctor = mono_method_desc_new("::.ctor",FALSE);
        assert(clr_desc_ctor);

        clr_desc_bang = mono_method_desc_new(":MethodBang",FALSE);
        assert(clr_desc_bang);
        clr_desc_float = mono_method_desc_new(":MethodFloat",FALSE);
        assert(clr_desc_float);
        clr_desc_symbol = mono_method_desc_new(":MethodSymbol",FALSE);
        assert(clr_desc_symbol);
        clr_desc_pointer = mono_method_desc_new(":MethodPointer",TRUE);
        assert(clr_desc_pointer);
        clr_desc_list = mono_method_desc_new(":MethodList",TRUE);
        assert(clr_desc_list);
        clr_desc_anything = mono_method_desc_new(":MethodAnything",TRUE);
        assert(clr_desc_anything);

        // install loader hook
        sys_loader(classloader);

        // ready!
	    post("CLR - (c) Davide Morelli, Thomas Grill");
    }
    else
		error("clr: mono domain couldn't be initialized!");
}
