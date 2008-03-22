#include <jni.h>
#include <m_pd.h>

#ifdef DEBUG
   #define ASSERT(v) { if ( v == NULL ) {bug("ouch, assertion failed %s:%d\n", __FILE__, __LINE__);}}
   #define JASSERT(v) { if ( v == NULL ) {(*env)->ExceptionDescribe(env);bug("ouch, jni assertion failed %s:%d\n", __FILE__, __LINE__);}}
   #undef  DEBUG
   #define DEBUG(X) {X};
#else 
   #define ASSERT(v)
   #undef  DEBUG
   #define DEBUG(X)
   #define JASSERT(v)
#endif

#define SHOWEXC { if ((*env)->ExceptionOccurred(env)) (*env)->ExceptionDescribe(env); } 

#ifdef PROFILER
	#define PROF(v) { v }
#else
	#define PROF(v)
#endif

#ifdef MSW
	#define DIR_SEP "\\"
	#define PATH_SEP ";"
#else 
	#define DIR_SEP "/"
	#define PATH_SEP ":"
#endif

#ifdef __LP64__
	#define JPOINTER_CAST (unsigned long)
#else 
	#define JPOINTER_CAST (unsigned int)
#endif

// the JVM takes 50M; I don't care taking 4K...
#define BUFFER_SIZE 4096

// MAXIMUM atom[x] size for type casting from jatoms to atoms
#define MAX_ATOMS_STACK 32

typedef struct PdjCaching {
	jclass      cls_Atom;
	jclass		cls_AtomString;
	jclass		cls_AtomFloat;
	jclass      cls_MaxClock;
	jclass		cls_MaxObject;
	jclass      cls_MSPObject;
	jclass		cls_MSPSignal;
	jmethodID   MIDAtom_newAtom_String;
	jmethodID   MIDAtom_newAtom_Float;
	jmethodID   MIDMaxObject_trySetter;
	jmethodID	MIDMSPObject_dspinit;
	jmethodID	MIDMSPObject_emptyPerformer;
	jfieldID	FIDAtom_type;
	jfieldID    FIDMaxObject_pdobj_ptr;
	jfieldID	FIDMaxObject_activity_inlet;
	jfieldID    FIDMaxClock_clock_ptr;
	jfieldID	FIDAtomFloat_value;
	jfieldID    FIDAtomString_value;
	jfieldID    FIDMSPObject_used_inputs;
	jfieldID    FIDMSPObject_used_outputs;
	jfieldID	FIDMSPSignal_vec;
} PdjCaching;
extern PdjCaching pdjCaching;

typedef struct _pdjcached_sym {
	t_symbol *sym;
	jmethodID mid;
	int arged;
	struct _pdjcached_sym *next;
} t_pdjcached_sym;

extern t_class *pdj_class;
typedef struct _pdj {
    t_object x_obj;
    char *jobject_name;
    int nb_inlet;
    char *patch_path;
    
    /* already resolved symbol to method id */
    t_pdjcached_sym *cache;
    
    /* java object instance and class definition */
    jobject obj;
    jclass  cls;
    
    /* object method binder */
    jmethodID MIDbang;
    jmethodID MIDfloat;
    jmethodID MIDint;
    jmethodID MIDlist;
    jmethodID MIDanything;
} t_pdj;

extern t_class *pdj_tilde_class;
typedef struct _pdj_tilde {
	t_pdj    pdj;
	t_sample _dummy_f;    

	/* performer method */	
    jmethodID performer;
    
    /* pointer to private field _used_inputs/outputs */
    jobject	_used_inputs;
    jobject	_used_outputs;

    /* C array to the java float vector */
    jobject	*ins;
    jobject	*outs;

    int	ins_count;
    int	outs_count;
    
    /* number of arguments sended to the performer */
    int	argc;
} t_pdj_tilde;

extern t_class *inlet_proxy;
typedef struct _inlet_proxy {
	t_object x_obj;
	t_pdj *peer;
	int idx;
} t_inlet_proxy;


typedef int JNICALL JNI_CreateJavaVM_func(JavaVM**, JNIEnv**, JavaVMInitArgs*);
int getuglylibpath(char *path);
JNI_CreateJavaVM_func *linkjvm(char *vm_type);
char *pdj_getProperty(char *name);

JNIEnv *init_jvm();

void *pdj_new(t_symbol *s, int argc, t_atom *argv);
void pdj_free(t_pdj *obj);

void pdj_tilde_dsp(t_pdj_tilde *obj, t_signal **sp);
void *pdj_tilde_new(t_symbol *s, int argc, t_atom *argv);
void pdj_tilde_free(t_pdj_tilde *pdjt);

JNIEnv *pdjAttachVM();
void pdjDetachVM(JNIEnv *env);

extern int REDIRECT_PD_IO;
extern JavaVM *jni_jvm; 
