/* 
just a dummy clr patch 

*/

#include "m_pd.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>

// mono
#include <mono/jit/jit.h>
#include <mono/metadata/object.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SELECTORS 256
#define MAX_OUTLETS 32

void *clr_new(t_symbol *s, int argc, t_atom *argv);

// list of mono methods
typedef struct selectorList selectorList;
typedef struct selectorList
{
	char *sel; // the selector
	MonoMethod *func; // the function
	int type;
//	selectorList *next; // next element of the list
};

// list of outlets
typedef struct outletList outletList;
typedef struct outletList
{
	t_outlet *outlet_pointer;
//	outletList *next; // next element of the list
};

// simplyfied atom
typedef struct atom_simple atom_simple;
typedef enum
{
    A_S_NULL=0,
    A_S_FLOAT=1,
    A_S_SYMBOL=2,
}  t_atomtype_simple;
typedef struct atom_simple
{
	//t_atomtype_simple a_type;
	int a_type;
	//MonoString *a_type;
	//union{
		float float_value;
		MonoString *string_value;
	//} stuff;
};


static t_class *clr_class;

typedef struct _clr
{
    t_object x_obj; // myself
	t_outlet *l_out;
	// mono stuff
	MonoDomain *domain;
	MonoAssembly *assembly;
	MonoObject *obj;
	MonoImage *image;
	MonoMethod *method, *setUp;
	MonoClass *klass;
	int n;

	// TODO: dynamic memory allocation
	selectorList selectors[MAX_SELECTORS];
	outletList outlets[MAX_OUTLETS];

} t_clr;


// mono functions

static void mono_clean(t_clr *x)
{
	// clean up stuff
	mono_jit_cleanup (x->domain);

}

void registerMonoMethod(void *x, MonoString *selectorString, MonoString *methodString, int type);
void createInlet(void *x1, MonoString *selectorString, int type);
void createOutlet(void *x1, int type);
void out2outlet(void *x, int outlet, int type, unsigned char  *val  /* TODO */);
void post2pd(MonoString *mesString);
void error2pd(MonoString *mesString);

// load the variables and init mono
static void mono_load(t_clr *x)
{
//	const char *file="D:\\Davide\\cygwin\\home\\Davide\\externalTest1.dll";
	const char *file="PureData.dll";
	
	MonoMethod *m = NULL, *ctor = NULL, *fail = NULL, *mvalues;
	gpointer iter;
	gpointer args [1];
	int val;
	int i;
	double rnd;
	int random_name_int;
	char random_name_str[256];
	srand( (unsigned)time( NULL ) );
	rnd = rand()/((double)RAND_MAX + 1);
	random_name_int =(int) (rnd * RAND_MAX);
	sprintf(random_name_str, "%s-%i",file, random_name_int);

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

printf("will load %s, random_name %s\n", file, random_name_str);

	x->domain = mono_jit_init (random_name_str);

		// add mono to C hooks
	mono_add_internal_call ("PureData.pd::RegisterSelector", registerMonoMethod);
	mono_add_internal_call ("PureData.pd::ToOutlet", out2outlet);
	mono_add_internal_call ("PureData.pd::PostMessage", post2pd);
	mono_add_internal_call ("PureData.pd::ErrorMessage", error2pd);
	mono_add_internal_call ("PureData.pd::CreateOutlet", createOutlet);
	mono_add_internal_call ("PureData.pd::CreateInlet", createInlet);

	x->assembly = mono_domain_assembly_open (x->domain, file);
	if (!x->assembly)
	{
		error("clr: assembly not found!");
	}

	x->image = mono_assembly_get_image (x->assembly);

	
	x->klass = mono_class_from_name (x->image, "PureData", "External");
	if (!x->klass) {
		error("Can't find MyType in assembly %s\n", mono_image_get_filename (x->image));
		//exit (1);
	}
	x->obj = mono_object_new (x->domain, x->klass);
	mono_runtime_object_init (x->obj);

	/* retrieve all the methods we need */
	iter = NULL;
	while ((m = mono_class_get_methods (x->klass, &iter))) {
		if (strcmp (mono_method_get_name (m), "test") == 0) {
			x->method = m;
		} else if (strcmp (mono_method_get_name (m), "SetUp") == 0) {
			x->setUp = m;
		} else if (strcmp (mono_method_get_name (m), "Fail") == 0) {
			fail = m;
		} else if (strcmp (mono_method_get_name (m), "Values") == 0) {
			mvalues = m;
		} else if (strcmp (mono_method_get_name (m), ".ctor") == 0) {
			/* Check it's the ctor that takes two args:
			 * as you see a contrsuctor is a method like any other.
			 */
			MonoMethodSignature * sig = mono_method_signature (m);
			if (mono_signature_get_param_count (sig) == 2) {
				ctor = m;
			}
		}
	}

	// call the base functions
	if (x->setUp)
	{
		val = x;
		args [0] = &val;
		mono_runtime_invoke (x->setUp, x->obj, args, NULL);
		post("SetUp() invoked");

	} else
	{
		error("clr: the provided assembly is not valid! the SetUp function is missing");
	}


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

// here i look for the selector and call the right mono method
void clr_manage_list(t_clr *x, t_symbol *sl, int argc, t_atom *argv)
{
	gpointer args [2];
	int valInt;
	float valFloat;

	int i;
	// first i extract the first atom which should be a symbol
	//post("clr_manage_list, got symbol = %s", sl->s_name);

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
						strmono = mono_string_new (x->domain, strsymbol->s_name);
						args[0] = &strmono;
						mono_runtime_invoke (x->selectors[i].func, x->obj, args, NULL);
						// memory leak ?
						break;
					}
				case 3:
					{
						gpointer args [1];
						MonoString *strmono;
						t_symbol *strsymbol;
						float ftmp;
						char *str;
						char strtmp[265], strtmp2[256];
						MonoArray * arystr;
						int j;		
						atom_simple *atmp;
						atom_simple *atmp2, *atmp3;

							int *tipo;
							float *fp;

						MonoClass *c = mono_class_from_name (x->image, "PureData", "Atom");
						arystr = mono_array_new (x->domain, c /*mono_get_string_class ()*/, argc);
						//arystr = mono_array_new (x->domain, c, 2);
						/*
						for (j=0; j<argc; j++)
						{
							atmp = malloc(sizeof(atom_simple));
							switch ((argv+j)->a_type)
							{
							case A_FLOAT:
post("setting type float in position %i", j);
								//atmp->a_type = 1;
								sprintf(strtmp, "1");
						//		atmp->a_type = mono_string_new (x->domain, strtmp);
								ftmp = atom_getfloat(argv+j);
								atmp->float_value = ftmp;
								break;
							case A_SYMBOL:
post("setting type symbol in position %i", j);
								//atmp->a_type = 2;
								sprintf(strtmp, "2");
							//	atmp->a_type = mono_string_new (x->domain, strtmp);
								strsymbol = atom_getsymbol(argv+j);
								atmp->string_value = mono_string_new (x->domain, strsymbol->s_name);
								break;
							default:
post("setting type null in position %i", j);
								//atmp->a_type = 0;
								sprintf(strtmp, "0");
							//	atmp->a_type = mono_string_new (x->domain, strtmp);
							}
						//	mono_array_set (arystr, MonoString *, j, atmp->a_type);
							mono_array_set (arystr, atom_simple *, j, atmp);
							//int * ftmp = malloc(sizeof(int));
							//*ftmp =  j;
							//float ftmp = atom_getfloat(argv+j);
							//strsymbol = atom_getsymbol(argv+j);
							//MonoString *arg = mono_string_new (x->domain, strsymbol->s_name);
							//mono_array_set (arystr, MonoString *, j, arg);
							// gpointer
							
							//mono_array_set (arystr, gint32 , j, *ftmp);
							//mono_array_set (arystr, gint32 , j, ftmp);

						}
						*/

						// debug:
						// send just 1 atom
						
							atmp2 = malloc(sizeof(atom_simple));
							atmp2->a_type = 121;
							atmp2->float_value = atom_getfloat(argv);
							strsymbol = atom_getsymbol(argv);
							atmp2->string_value = mono_string_new (x->domain, strsymbol->s_name);
						args[0] = atmp2;
						
						
						/*
						// a list of atoms
						for (j=0; j<argc; j++)
						{
							atmp2 = malloc(sizeof(atom_simple));
							atmp2->a_type = 4;
							atmp2->float_value = atom_getfloat(argv+j);
							strsymbol = atom_getsymbol(argv+j);
							atmp2->string_value = mono_string_new (x->domain, strsymbol->s_name);
							mono_array_set (arystr, atom_simple *, j, atmp2);
						}
						*/


						/*
						atmp3 = malloc(sizeof(atom_simple));
						atmp3->a_type = 2;
						atmp3->float_value = 0.5;
						sprintf(strtmp2, "abracadabra");
						atmp3->string_value = mono_string_new (x->domain, strtmp2);
						mono_array_set (arystr, atom_simple *, 1, atmp3);
						*/
					//	args[0] = arystr;
						
						//args[0] = strings;
						mono_runtime_invoke (x->selectors[i].func, x->obj, args, NULL);
						break;
					}
				}
				
				//result = mono_runtime_invoke (x->selectors[i].func, x->obj, args, NULL);
				//val = *(int*)mono_object_unbox (result);
				//x->n = val;
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

	post("registerMonoMethod: associating %s to %s", selCstring, metCstring);

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

	class_addmethod(clr_class, (t_method)clr_manage_list, gensym(selCstring), A_GIMME, 0);


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
void out2outlet(void *x1, int outlet, int type, unsigned char *val /* TODO */)
{
	t_clr *x;
	x = (t_clr *)x1;
	t_atom *lista;

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
printf("ricevuto %i %i\n", val[0], val[1]);
if (val[0]==0x01)
	printf("val[0]==0x00");

	lista = (t_atom *) malloc(sizeof(t_atom) * 2);
	SETFLOAT(lista, (float) val[0]);
	SETFLOAT(lista+1, (float) val[1]);
	outlet_anything(x->outlets[outlet].outlet_pointer,
					gensym("list") ,
					2, 
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
//	x->l_out = outlet_new(&x->x_obj, &s_list);
//	x->l_out = outlet_new(&x->x_obj, gensym("float"));
	


	x->n = 0;
	// load mono, init the needed vars
	mono_load(x);

    return (x);
}

void clr_setup(void)
{
    clr_class = class_new(gensym("clr"), (t_newmethod)clr_new,
        (t_method)clr_free, sizeof(t_clr), CLASS_DEFAULT, A_GIMME, 0);
    //class_addbang(clr_class, (t_method)clr_bang);
//	ext_class_addbang((t_method) clr_bang);

	class_addlist(clr_class, (t_method)clr_manage_list);

}
