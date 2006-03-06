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

#ifdef _WIN32
#include <io.h> // for _close
#define close _close
#else
#include <unistd.h>
#endif

#include <map>


static MonoDomain *monodomain;
static MonoClass *clr_symbol,*clr_pointer,*clr_atom,*clr_atomlist,*clr_external;
static MonoMethodDesc *clr_desc_main,*clr_desc_ctor;
static MonoMethodDesc *clr_desc_bang,*clr_desc_float,*clr_desc_symbol,*clr_desc_pointer,*clr_desc_list,*clr_desc_anything;
static MonoMethod *ext_method_bang,*ext_method_float,*ext_method_symbol,*ext_method_list,*ext_method_pointer,*ext_method_anything;

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
    assert(x && x->clr_clss);
    MonoMethod *m = mono_object_get_virtual_method(x->mono_obj,ext_method_bang);
    assert(m);
    MonoObject *exc;
    mono_runtime_invoke(m,x->mono_obj,NULL,&exc);
    if(exc) {
        error("Exception raised");
    }
}

static void clr_method_float(t_clr *x,t_float f) 
{
    assert(x && x->clr_clss);
    MonoMethod *m = mono_object_get_virtual_method(x->mono_obj,ext_method_float);
    assert(m);
	gpointer args = &f;
    MonoObject *exc;
    mono_runtime_invoke(m,x->mono_obj,&args,&exc);
    if(exc) {
        error("Exception raised");
    }
}

static void clr_method_symbol(t_clr *x,t_symbol *s) 
{
    assert(x && x->clr_clss);
    MonoMethod *m = mono_object_get_virtual_method(x->mono_obj,ext_method_symbol);
    assert(m);
#if 0
    MonoObject *symobj = mono_value_box(monodomain,clr_symbol,&s);
    MonoObject *o = (MonoObject *)mono_object_unbox(symobj);
	gpointer args = o;
#else
	gpointer args = &s;
#endif
    MonoObject *exc;
    mono_runtime_invoke(m,x->mono_obj,&args,&exc);
    if(exc) {
        error("Exception raised");
    }
}

struct AtomList
{
    int argc;
    t_atom *argv;

    AtomList(int c,t_atom *v): argc(c),argv(v) {}
};

static MonoObject *new_AtomList(int argc,t_atom *argv)
{
	AtomList al(argc,argv);
    MonoObject *lstobj = mono_value_box(monodomain,clr_atomlist,&al);
    MonoObject *o = (MonoObject *)mono_object_unbox(lstobj);
    return o;
}

static MonoArray *new_Atoms(int argc,t_atom *argv)
{
    MonoArray *lstobj = mono_array_new(monodomain,clr_atom,argc);
    t_atom *lstptr = mono_array_addr(lstobj,t_atom,0);
    memcpy(lstptr,argv,argc*sizeof(t_atom));
    return lstobj;
}

static void clr_method_list(t_clr *x,t_symbol *l, int argc, t_atom *argv)
{
    assert(x && x->clr_clss);
    MonoMethod *m = mono_object_get_virtual_method(x->mono_obj,ext_method_list);
    assert(m);
#if 1
    // make PureData.AtomList value type
//    MonoObject *lstobj = new_AtomList(argc,argv);
//	gpointer args = lstobj;
	AtomList al(argc,argv);
	gpointer args = &al;
#else
    // make PureData.Atom[] array - copy data
    MonoArray *lstobj = new_Atoms(argc,argv);
	gpointer args = lstobj;
#endif
    MonoObject *exc;
    mono_runtime_invoke(m,x->mono_obj,&args,&exc);
    if(exc) {
        error("Exception raised");
    }
}

static void clr_method_pointer(t_clr *x,t_gpointer *p)
{
    assert(x && x->clr_clss);
    MonoMethod *m = mono_object_get_virtual_method(x->mono_obj,ext_method_pointer);
    assert(m);
#if 0
    MonoObject *ptrobj = mono_value_box(monodomain,clr_pointer,&p);
    MonoObject *o = (MonoObject *)mono_object_unbox(ptrobj);
	gpointer args = o;
#else
	gpointer args = &p;
#endif
    MonoObject *exc;
    mono_runtime_invoke(m,x->mono_obj,&args,&exc);
    if(exc) {
        error("Exception raised");
    }
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
    assert(x && x->clr_clss);
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

static void *PD_SymGen(MonoString *str)
{
    assert(str);
	t_symbol *s = gensym(mono_string_to_utf8(str));	
    return s;
}

static MonoString *PD_SymEval(t_symbol *sym)
{
    assert(sym);
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
    mono_field_set_value(x->mono_obj,clss->mono_obj_field,&x);

    // ok, we have an object - look for constructor
	if(clss->mono_ctor) {
#if 1
    	AtomList al(argc,argv);
	    gpointer args = &al;
#else
        MonoObject *lstobj = new_AtomList(argc,argv);
    	gpointer args = lstobj;
#endif
        MonoObject *exc;

        // call static constructor
        MonoObject *ret = mono_runtime_invoke(clss->mono_ctor,x->mono_obj,&args,&exc);
        if(ret) {
            post("Warning: returned value from %s::.ctor ignored",classname->s_name);
            // ??? do we have to mark ret as free?
        }

        if(exc) {
            pd_free((t_pd *)x);
            error("CLR class %s - exception raised in constructor",classname->s_name);
            return NULL;
        }
    }
    else
        post("Warning: no constructor for class %s found",classname->s_name);

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
    close(fd);

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

#if 0
    // find && register methods
    if((clr_class->method_bang = mono_method_desc_search_in_class(clr_desc_bang,clr_class->mono_class)) != NULL)
        class_addbang(clr_class->pd_class,clr_method_bang);
    if((clr_class->method_float = mono_method_desc_search_in_class(clr_desc_float,clr_class->mono_class)) != NULL)
        class_addfloat(clr_class->pd_class,clr_method_float);
    if((clr_class->method_symbol = mono_method_desc_search_in_class(clr_desc_symbol,clr_class->mono_class)) != NULL)
        class_addsymbol(clr_class->pd_class,clr_method_symbol);
    if((clr_class->method_pointer = mono_method_desc_search_in_class(clr_desc_pointer,clr_class->mono_class)) != NULL)
        class_addpointer(clr_class->pd_class,clr_method_pointer);
    if((clr_class->method_list = mono_method_desc_search_in_class(clr_desc_list,clr_class->mono_class)) != NULL)
        class_addlist(clr_class->pd_class,clr_method_list);
    if((clr_class->method_anything = mono_method_desc_search_in_class(clr_desc_anything,clr_class->mono_class)) != NULL)
        class_addanything(clr_class->pd_class,clr_method_anything);
#else
    // register methods
    if(ext_method_bang) 
        class_addbang(clr_class->pd_class,clr_method_bang);
    if(ext_method_float) 
        class_addfloat(clr_class->pd_class,clr_method_float);
    if(ext_method_symbol) 
        class_addsymbol(clr_class->pd_class,clr_method_symbol);
    if(ext_method_pointer) 
        class_addpointer(clr_class->pd_class,clr_method_pointer);
    if(ext_method_list) 
        class_addlist(clr_class->pd_class,clr_method_list);
    if(ext_method_anything) 
        class_addanything(clr_class->pd_class,clr_method_anything);
#endif

    // put into map
    clr_map[classsym] = clr_class;

//    post("Loaded class %s OK",classname);

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

    try { 
        monodomain = mono_jit_init("PureData"); 
    }
    catch(...) {
        monodomain = NULL;
    }

	if(monodomain) {
	    // add mono to C hooks
        mono_add_internal_call("PureData.Core::Post",(const void *)PD_Post);
	    mono_add_internal_call("PureData.Core::PostError",(const void *)PD_PostError);
	    mono_add_internal_call("PureData.Core::PostBug",(const void *)PD_PostBug);
	    mono_add_internal_call("PureData.Core::PostVerbose",(const void *)PD_PostVerbose);

	    mono_add_internal_call("PureData.Core::SymGen", (const void *)PD_SymGen);
	    mono_add_internal_call("PureData.Core::SymEval", (const void *)PD_SymEval);


        MonoAssembly *assembly = mono_domain_assembly_open (monodomain, "PureData.dll");
	    if(!assembly) {
		    error("clr: assembly PureData.dll not found!");
		    return;
	    }

	    MonoImage *image = mono_assembly_get_image(assembly);
        assert(image);

        // load important classes
        clr_symbol = mono_class_from_name(image,"PureData","Symbol");
        assert(clr_symbol);
        clr_pointer = mono_class_from_name(image,"PureData","Pointer");
        assert(clr_pointer);
        clr_atom = mono_class_from_name(image,"PureData","Atom");
        assert(clr_atom);
        clr_atomlist = mono_class_from_name(image,"PureData","AtomList");
        assert(clr_atomlist);
        clr_external = mono_class_from_name(image,"PureData","External");
        assert(clr_external);

        clr_desc_main = mono_method_desc_new("::Main()",FALSE);
        assert(clr_desc_main);
        clr_desc_ctor = mono_method_desc_new("::.ctor(AtomList)",FALSE);
        assert(clr_desc_ctor);

        clr_desc_bang = mono_method_desc_new("::MethodBang()",FALSE);
        assert(clr_desc_bang);
        clr_desc_float = mono_method_desc_new("::MethodFloat(single)",FALSE);
        assert(clr_desc_float);
        clr_desc_symbol = mono_method_desc_new("::MethodSymbol(Symbol)",FALSE);
        assert(clr_desc_symbol);
        clr_desc_pointer = mono_method_desc_new("::MethodPointer(Pointer)",FALSE);
        assert(clr_desc_pointer);
        clr_desc_list = mono_method_desc_new("::MethodList(AtomList)",FALSE);
        assert(clr_desc_list);
        clr_desc_anything = mono_method_desc_new("::MethodAnything(Symbol,AtomList)",FALSE);
        assert(clr_desc_anything);

        // find abstract methods
        ext_method_bang = mono_method_desc_search_in_class(clr_desc_bang,clr_external);
        ext_method_float = mono_method_desc_search_in_class(clr_desc_float,clr_external);
        ext_method_symbol = mono_method_desc_search_in_class(clr_desc_symbol,clr_external);
        ext_method_pointer = mono_method_desc_search_in_class(clr_desc_pointer,clr_external);
        ext_method_list = mono_method_desc_search_in_class(clr_desc_list,clr_external);
        ext_method_anything = mono_method_desc_search_in_class(clr_desc_anything,clr_external);

        // install loader hook
        sys_loader(classloader);

        // ready!
	    post("CLR - (c)2006 Davide Morelli, Thomas Grill");
    }
    else
		error("clr: mono domain couldn't be initialized!");
}
