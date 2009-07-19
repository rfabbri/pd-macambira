#include "native_classes.h"
#include "type_handler.h"
#include "pdj.h"


static t_pdj *getMaxObject(JNIEnv *env, jobject obj) {
  	t_pdj *ret = (t_pdj *) JPOINTER_CAST (*env)->GetLongField(env, obj, 
  			pdjCaching.FIDMaxObject_pdobj_ptr);
  	
  	if ( ret == NULL ) 
		error("pdj: using a native method without pd context");
  	
  	return ret;
}


JNIEXPORT jlong JNICALL Java_com_cycling74_max_MaxObject_newInlet
  (JNIEnv *env, jobject obj, jint type) {
  	t_pdj *pdj = getMaxObject(env, obj); 	
  	t_inlet_proxy *proxy; 

	if ( pdj == NULL )
		return 0;
  	if ( type == com_cycling74_msp_MSPObject_SIGNAL ) {
  		inlet_new(&pdj->x_obj, &pdj->x_obj.ob_pd, &s_signal, 0);	
  		return 0;
  	}

  	if ( inlet_proxy == 0 ) { 
        	bug("the inlet_proxy IS NOT initialized. danke!");
       		return 0;
    	}
  	
  	proxy = (t_inlet_proxy *) pd_new(inlet_proxy);
  	proxy->idx = pdj->nb_inlet++;
  	proxy->peer = pdj;
  		
  	return (jlong) inlet_new(&pdj->x_obj, &proxy->x_obj.ob_pd, 0, 0); 
}


JNIEXPORT jlong JNICALL Java_com_cycling74_max_MaxObject_newOutlet
  (JNIEnv *env, jobject obj, jint type) {
  	t_pdj *pdj = getMaxObject(env, obj); 	

	if ( pdj == NULL )
		return 0;
	  	
  	if ( type == com_cycling74_msp_MSPObject_SIGNAL ) {
  		outlet_new(&pdj->x_obj, &s_signal);
  		return 0;
  	}
  	
  	return (jlong) outlet_new(&pdj->x_obj, NULL); 
}


JNIEXPORT void JNICALL Java_com_cycling74_max_MaxObject_doOutletBang
  (JNIEnv *env, jobject obj, jlong outlet) {
  	t_outlet *x = (t_outlet *) (JPOINTER_CAST outlet);
  	outlet_bang(x);
}


JNIEXPORT void JNICALL Java_com_cycling74_max_MaxObject_doOutletFloat
  (JNIEnv *env, jobject obj, jlong outlet , jfloat value) {
  	t_outlet *x = (t_outlet *) (JPOINTER_CAST outlet);
  	outlet_float(x, value);
}


JNIEXPORT void JNICALL Java_com_cycling74_max_MaxObject_doOutletSymbol
  (JNIEnv *env, jobject obj, jlong outlet, jstring value) {
  	t_outlet *x = (t_outlet *) (JPOINTER_CAST outlet);
  	outlet_symbol(x, jstring2symbol(env, value));
}


JNIEXPORT void JNICALL Java_com_cycling74_max_MaxObject_doOutletAnything
  (JNIEnv *env, jobject obj, jlong outlet, jstring str, jobjectArray value) {
  	t_outlet *x = (t_outlet *) (JPOINTER_CAST outlet);
  	t_atom args[MAX_ATOMS_STACK];
  	int argc;

  	if ( jatoms2atoms(env, value, &argc, args) != 0 )
		return;

  	if ( str == NULL ) {
  		if ( args[0].a_type == A_FLOAT ) {
  			outlet_anything(x, &s_list, argc, args);
  		} else {
	  		t_symbol *sym = atom_getsymbol(&(args[0]));
	  		outlet_anything(x, sym, argc-1, args+1);
  		}
  	} else {
  		outlet_anything(x, jstring2symbol(env, str), argc, args);
  	}
}

JNIEXPORT jstring JNICALL Java_com_cycling74_max_MaxObject_getPatchPath
  (JNIEnv *env, jobject obj) {
  	t_pdj *pdj = getMaxObject(env, obj);
  	
  	if ( pdj == NULL )
  		return NULL;
  		
	return (*env)->NewStringUTF(env, pdj->patch_path);
  	
}

// UGLY UGLY UGLY, but this is used not force the user from using
// a constructor. MaxObject CAN be used outside the pdj object
// but all the natives calls will break. Theses methods are always called
// within a synchronized() block. This is ugly, but less ugly than
// putting the information in a ThreadLocals? ... or using a MaxContext ?
// ===========================================================================
static t_pdj *pdjConstructor = NULL;
JNIEXPORT void JNICALL Java_com_cycling74_max_MaxObject_pushPdjPointer
  (JNIEnv *env, jclass cls, jlong ptr) {
  	pdjConstructor = (t_pdj *) ptr;
}


JNIEXPORT jlong JNICALL Java_com_cycling74_max_MaxObject_popPdjPointer
  (JNIEnv *env, jclass cls) {
  	t_pdj *tmp;
 
 	// the object has been used instanciated without pdj. Return a null
 	// pointer. 	
  	if ( pdjConstructor == NULL ) {
   		return 0;
  	}
  	tmp = pdjConstructor;
  	pdjConstructor = NULL;
  	
  	return tmp;
}
