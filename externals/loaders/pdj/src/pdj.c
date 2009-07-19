#include <stdlib.h>
#include "pdj.h"
#include "type_handler.h"

JavaVM *jni_jvm = NULL;
t_class *inlet_proxy;
t_class *pdj_class;

#define PDTHREAD_STACKSIZE 1000000
static JNIEnv *pdthread_jnienv = NULL; 
static char *pdthread_stackaddr;

#define PROF_MIN 999

double prof_max = 0, prof_min = PROF_MIN, prof_tot = 0, prof_nb = 0, prof_tmp;

JNIEnv *pdjAttachVM() {
	const int N_REFS = 16;
	JNIEnv *env;
	char stack_pos; /* the position of this variable is relative to the 
	                   position in the stack of the main thread */
	
	PROF(prof_tmp = sys_getrealtime(););
	
	/* this avoids getting the JNIEnv when we are in the main thread. so
	 * if the current stack is more far than 1 meg of distance between the 
	 * main thread stack when pdj was initialized, it is considered a 
	 * "external thread"
	 */
	if ( abs(pdthread_stackaddr - (&stack_pos)) > PDTHREAD_STACKSIZE ) {
		(*jni_jvm)->AttachCurrentThread(jni_jvm, (void **)&env, NULL);	
		ASSERT(env);
	} else {
		env = pdthread_jnienv;
	}
	
	if ( (*env)->PushLocalFrame(env, N_REFS) < 0 ) {
		SHOWEXC;
		bug("pdj: java: out of memory!?!");
	}
	
	return env;
}


void pdjDetachVM(JNIEnv *env) {
#ifdef DEBUG
	if ( (*env)->ExceptionOccurred(env) ) {
		error("pdj: unhandled exception in JNI interface:");
		(*env)->ExceptionDescribe(env);
	}
#endif
	(*env)->PopLocalFrame(env, NULL);
	
	PROF(prof_tmp = sys_getrealtime() - prof_tmp;);
	PROF(prof_max = prof_tmp > prof_max ? prof_tmp : prof_max;);
	PROF(prof_min = prof_tmp < prof_min ? prof_tmp : prof_min;);
	PROF(prof_nb++;);
	PROF(prof_tot += prof_tmp;);
}


static void pdj_mapmethods(JNIEnv *env, t_pdj *pdj) {
	jclass base = pdjCaching.cls_MaxObject;
	jmethodID idBase, id;
	JASSERT(base);	
	
	id     = (*env)->GetMethodID(env, pdj->cls, "bang", "()V");
	idBase = (*env)->GetMethodID(env, base, "bang", "()V");
	(*env)->ExceptionClear(env);	
	if ( id != idBase ) {
		pdj->MIDbang = id;
	} else {
		pdj->MIDbang = NULL;
	}

	id     = (*env)->GetMethodID(env, pdj->cls, "inlet", "(F)V");
	idBase = (*env)->GetMethodID(env, base, "inlet", "(F)V");
	(*env)->ExceptionClear(env);		
	if ( id != idBase ) {
		pdj->MIDfloat = id;
	} else {
		pdj->MIDfloat = NULL;
		id     = (*env)->GetMethodID(env, pdj->cls, "inlet", "(I)V");
		idBase = (*env)->GetMethodID(env, base, "inlet", "(I)V");
		(*env)->ExceptionClear(env);		
		if ( id != idBase ) {
			pdj->MIDint = id;
		} else {
			pdj->MIDint = NULL;
		}		
	}

	id     = (*env)->GetMethodID(env, pdj->cls, "list", "([com/cycling74/max/Atom;)V");	
	idBase = (*env)->GetMethodID(env, base, "list", "([com/cycling74/max/Atom;)V");	
	(*env)->ExceptionClear(env);	
	if ( id != idBase ) {
		pdj->MIDlist = id;
	} else {
		pdj->MIDlist = NULL;
	}
	
	idBase = (*env)->GetMethodID(env, pdj->cls, "anything", "(Ljava/lang/String;[Lcom/cycling74/max/Atom;)V");	
	pdj->MIDanything = idBase;
}


static jobject init_pdj_class(JNIEnv *env, char *name, long cobj, int argc, t_atom *argv) {
	char *sig = "(Ljava/lang/String;J[Lcom/cycling74/max/Atom;)Lcom/cycling74/max/MaxObject;";
	jstring objectName;
	jmethodID id;
	jobject newMaxObject;
	jobjectArray args;
	jlong jval = cobj;
	
	id = (*env)->GetStaticMethodID(env, pdjCaching.cls_MaxObject, "registerObject", sig);
	JASSERT(id);

	objectName = (*env)->NewStringUTF(env, name);
	JASSERT(objectName);
	
	args = atoms2jatoms(env, argc, argv);
	JASSERT(args);

	newMaxObject = (*env)->CallStaticObjectMethod(env, pdjCaching.cls_MaxObject, id, objectName, jval, args);
	if ( (*env)->ExceptionOccurred(env) ) {
		(*env)->ExceptionDescribe(env);
	}
	if ( newMaxObject != NULL ) {
        jobject global = (*env)->NewGlobalRef(env, newMaxObject);
		(*env)->DeleteLocalRef(env, newMaxObject);        
		return global;
	}
	return NULL;
}


void *pdj_new(t_symbol *s, int argc, t_atom *argv) {
	JNIEnv *env = NULL; 
    t_pdj *x;

	if ( argc < 1 ) { 
		post("pdj: no class specified");
		return NULL;
	}

	if ( argv[0].a_type != A_SYMBOL ) {
		post("pdj: first argument must be a class name");
		return NULL;
	}
	
	if ( jni_jvm == NULL ) {
		env = init_jvm();	
		if ( env == NULL )
			return NULL;
	} else {
		(*jni_jvm)->AttachCurrentThread(jni_jvm, (void **)&env, NULL);
		ASSERT(env);
	}

	pdthread_jnienv = env;

	if ( s == gensym("pdj~") ) {
		x = (t_pdj *)pd_new(pdj_tilde_class);
	} else {
		x = (t_pdj *)pd_new(pdj_class);
	}
	ASSERT(x);
	x->jobject_name = argv[0].a_w.w_symbol->s_name;
	x->nb_inlet = 0;
    x->patch_path = canvas_getcurrentdir()->s_name;

    x->cache = NULL;
	x->obj = init_pdj_class(env, x->jobject_name, (long) x, argc, argv);

	if ( x->obj != NULL ) {
		jclass cls = (*env)->GetObjectClass(env, x->obj);
        JASSERT(cls);
        
        x->cls = (*env)->NewGlobalRef(env, cls); 
        pdj_mapmethods(env, x);
		return x;
	}
 	
	return NULL;
}


static void pdj_profiler(t_pdj *pdj) {
	if ( prof_nb != 0 )
		post("pdj-profiler: %f min %f max %f avg", prof_min, prof_max, prof_tot / prof_nb);
	prof_min = PROF_MIN;
	prof_max = 0;
	prof_tot = 0;
	prof_nb  = 0;
}


void pdj_free(t_pdj *pdj) {
	JNIEnv *env = pdjAttachVM();
	jmethodID id = (*env)->GetMethodID(env, pdj->cls, "notifyDeleted", "()V");
	JASSERT(id);
	
	(*env)->CallVoidMethod(env, pdj->obj, id);
	SHOWEXC;

	PROF(pdj_profiler(pdj););
	
	while (pdj->cache != NULL) {
		t_pdjcached_sym *next = pdj->cache->next;
		free(pdj->cache);
		pdj->cache = next;		
	}
	
	(*env)->DeleteGlobalRef(env, pdj->cls);
	(*env)->DeleteGlobalRef(env, pdj->obj);

	pdjDetachVM(env);	
}


static void pdj_addcache_sym(t_pdj *pdj, t_symbol *s, jmethodID id, int arged) {
	t_pdjcached_sym *n = malloc(sizeof(t_pdjcached_sym));
	
	n->sym = s;
	n->mid = id;
	n->arged = arged;
	n->next = pdj->cache;

	pdj->cache = n;	
}


static void pdj_process_inlet(int idx, t_pdj *pdj, t_symbol *s, int argc, t_atom atoms[]){
	JNIEnv *env = pdjAttachVM();
	jobjectArray args = NULL;
	t_pdjcached_sym *cache;
	jmethodID id;
	jboolean rc;
	jstring s_name;
	
	(*env)->SetIntField(env, pdj->obj, pdjCaching.FIDMaxObject_activity_inlet, idx);
	
	if ( s == &s_bang ) {
		if ( pdj->MIDbang != NULL ) {
			(*env)->CallVoidMethod(env, pdj->obj, pdj->MIDbang);
			SHOWEXC;
			pdjDetachVM(env);
			return;
		}
	} else if ( s == &s_float ) {
		if ( pdj->MIDfloat != NULL ) {
			(*env)->CallVoidMethod(env, pdj->obj, pdj->MIDfloat, atom_getfloatarg(0, argc, atoms));
			SHOWEXC;
			pdjDetachVM(env);
			return;
		}
		if ( pdj->MIDint != NULL ) {
			(*env)->CallVoidMethod(env, pdj->obj, pdj->MIDint, atom_getintarg(0, argc, atoms));
			SHOWEXC;
			pdjDetachVM(env);
			return;
		}
	} else if ( s == &s_list ) {
		if ( pdj->MIDlist != NULL ) {
			args = atoms2jatoms(env, argc, atoms);
			JASSERT(args);
			(*env)->CallVoidMethod(env, pdj->obj, pdj->MIDlist, args);
			SHOWEXC;
			pdjDetachVM(env);
			return;
		}
	}

	/* is the symbol in cache ? */
	cache = pdj->cache;
	while( cache != NULL ) {
		if ( cache->sym == s ) {
			if ( ! cache->arged ) {	
				(*env)->CallVoidMethod(env, pdj->obj, cache->mid, NULL);
			} else {
				args = atoms2jatoms(env, argc, atoms);
				JASSERT(args);
				(*env)->CallVoidMethod(env, pdj->obj, cache->mid, args);
			}
			SHOWEXC;
			pdjDetachVM(env);
			return;
		}
		cache = cache->next;
	}

	/* the last tries will require a Atom[] arguments */
	args = atoms2jatoms(env, argc, atoms);
	JASSERT(args);
		
	/* try with [name](Atom []) */
	id = (*env)->GetMethodID(env, pdj->cls, s->s_name, "([Lcom/cycling74/max/Atom;)V");
	(*env)->ExceptionClear(env);
	if ( id != NULL ) {
		(*env)->CallVoidMethod(env, pdj->obj, id, args);
		SHOWEXC;
		pdjDetachVM(env);
		pdj_addcache_sym(pdj, s, id, 1);
		return;
	}

	/* try again with [name]() */ 
	id = (*env)->GetMethodID(env, pdj->cls, s->s_name, "()V");
	(*env)->ExceptionClear(env);
	if ( id != NULL ) {
		(*env)->CallVoidMethod(env, pdj->obj, id, NULL);
		SHOWEXC;
		pdjDetachVM(env);
		pdj_addcache_sym(pdj, s, id, 0);
		return;
	}
	
	s_name = (*env)->NewStringUTF(env, s->s_name);
	JASSERT(s_name);			
	
	/* try with the setter */		
	rc = (*env)->CallBooleanMethod(env, pdj->obj, pdjCaching.MIDMaxObject_trySetter, s_name, args);
	if ( (*env)->ExceptionCheck(env) == 1 ) {
		/* we got an exception, the class do have a setter, but it trowed an
		 * exception: log it and don't try with anything() 
		 */
		(*env)->ExceptionDescribe(env);
		rc = 1;	
	}
	
	if ( rc == 0 ) {
		/* nothing... call the anything method anything... */
		(*env)->CallVoidMethod(env, pdj->obj, pdj->MIDanything, s_name, args);
		SHOWEXC;
	}
	
	// TODO: setters should be cached too
	pdjDetachVM(env);
}


static void pdj_anything(t_pdj *pdj, t_symbol *s, int argc, t_atom atoms[]){
	pdj_process_inlet(0, pdj, s, argc, atoms);
}


static void inlet_proxy_anything(t_inlet_proxy *proxy, t_symbol *s, int argc, t_atom atoms[]){
	pdj_process_inlet(proxy->idx, proxy->peer, s, argc, atoms);
}


static void pdj_loadbang(t_pdj *pdj) {
	JNIEnv *env = pdjAttachVM();
	jmethodID id = (*env)->GetMethodID(env, pdj->cls, "loadbang", "()V");
	JASSERT(id);
	
	(*env)->CallVoidMethod(env, pdj->obj, id, NULL);
	SHOWEXC;

	pdjDetachVM(env);
}


DLLEXPORT void pdj_setup(void) {
	char stack_pos;
	
    pdj_class = class_new(gensym("pdj"), 
        (t_newmethod)pdj_new, (t_method)pdj_free,
        sizeof(t_pdj), CLASS_DEFAULT|CLASS_NOINLET, A_GIMME, 0);
        
	class_addmethod(pdj_class, (t_method)pdj_loadbang, gensym("loadbang"), 
		A_CANT, A_NULL);
	class_addanything(pdj_class, (t_method)pdj_anything);
	
	/* pdj~ things */
    pdj_tilde_class = class_new(gensym("pdj~"), 
        (t_newmethod)pdj_tilde_new, (t_method)pdj_tilde_free,
        sizeof(t_pdj_tilde), CLASS_DEFAULT|CLASS_NOINLET, A_GIMME, 0);
        
	class_addmethod(pdj_tilde_class, (t_method)pdj_loadbang, gensym("loadbang"), 
		A_CANT, A_NULL);
	class_addanything(pdj_tilde_class, (t_method)pdj_anything);
    class_addmethod(pdj_tilde_class, (t_method)pdj_tilde_dsp, gensym("dsp"), 0);
    CLASS_MAINSIGNALIN(pdj_tilde_class, t_pdj_tilde, _dummy_f);
    
    /* inlet_proxy: we create a dummy class to catch all messages from cold
     * inlets */
	inlet_proxy = class_new(gensym("pdj_inlet_proxy"),
		NULL,NULL, sizeof(t_inlet_proxy),
		CLASS_PD|CLASS_NOINLET, A_NULL);
	class_addanything(inlet_proxy, (t_method)inlet_proxy_anything);

	/* main thread stack address */	
	pdthread_stackaddr = (void *) &(stack_pos);
}
