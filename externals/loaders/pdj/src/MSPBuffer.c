#include "native_classes.h"
#include "type_handler.h"
#include "pdj.h"


static t_garray *getArray(JNIEnv *env, jstring name) {
	t_symbol *s = jstring2symbol(env, name);
	t_garray *ret = (t_garray *)pd_findbyclass(s, garray_class);

  	if ( ret == NULL ) {
  		post("pdj: array %s not found", s->s_name);
  		return NULL;
  	}

	return ret;	
}


JNIEXPORT jlong JNICALL Java_com_cycling74_msp_MSPBuffer_getSize
  (JNIEnv *env, jclass cls, jstring name) {
	t_garray *array = getArray(env, name);  	
  	
  	if ( array == NULL )
  		return -1;

	return garray_npoints(array);	
}


JNIEXPORT jfloatArray JNICALL Java_com_cycling74_msp_MSPBuffer_peek
  (JNIEnv *env , jclass cls, jstring name) {
	t_garray *array = getArray(env, name);  	
	t_sample *vec;
	jfloatArray ret;
  	int size;
  	
  	if ( array == NULL )
  		return NULL;
  	
  	if ( ! garray_getfloatarray(array, &size, &vec) )
  		return NULL;
  	
	ret = (*env)->NewFloatArray(env, size);
	
  	if ( ret == NULL ) {
  		SHOWEXC;
  		return NULL;
  	}
  		
  	(*env)->SetFloatArrayRegion(env, ret, 0, size, (jfloat *) vec);

  	return ret;
}


JNIEXPORT void JNICALL Java_com_cycling74_msp_MSPBuffer_poke
  (JNIEnv *env, jclass cls, jstring name, jfloatArray values) {
	t_garray *array = getArray(env, name);  	
	t_sample *vec;
	jsize jarray_length;
  	int size;

	if ( array == NULL )
		return;
		
  	if ( ! garray_getfloatarray(array, &size, &vec) )
  		return;
	 	
	jarray_length = (*env)->GetArrayLength(env, values);
	
	if ( jarray_length < size ) {
		error("pdj: warning: array too short");
	} else {
		size = jarray_length;
	}
	
	(*env)->GetFloatArrayRegion(env, values, 0, size, vec);
	garray_redraw(array);
}


JNIEXPORT void JNICALL Java_com_cycling74_msp_MSPBuffer_setSize
  (JNIEnv *env, jclass cls, jstring name, jint channel, jlong size) {
  	t_garray *array = getArray(env, name);
  	
  	if ( array == NULL )
  		return;
  	
	garray_resize(array, size);  	
}


JNIEXPORT jfloatArray JNICALL Java_com_cycling74_msp_MSPBuffer_getArray
  (JNIEnv *env, jclass cls, jstring name, jlong from, jlong arraySize) {
	t_garray *array = getArray(env, name);  	
	t_sample *vec;
	jfloatArray ret;
  	int size, ifrom = (int) from, iarraySize = (int) arraySize;
  	
  	if ( array == NULL )
  		return NULL;
  	
  	if ( ! garray_getfloatarray(array, &size, &vec) )
  		return NULL;
  	
  	if ( iarraySize != -1 ) {
	  	if ( size < ifrom ) {
	  		error("pdj: array is shorter than the starting point");
	  		return NULL;
	  	}
  	
	  	if ( size < ifrom + iarraySize ) {
	  		error("pdj: array is not big enough to fill the array");
	  		return NULL;
	  	}
	  	size = iarraySize;
  	}
  	
	ret = (*env)->NewFloatArray(env, size);
  	if ( ret == NULL ) {
  		SHOWEXC;
  		return NULL;
  	}
  	
  	vec += ifrom;
  	(*env)->SetFloatArrayRegion(env, ret, 0, size, (jfloat *) vec);

  	return ret;
}


JNIEXPORT void JNICALL Java_com_cycling74_msp_MSPBuffer_setArray
  (JNIEnv *env, jclass cls, jstring name, jlong from, jfloatArray values) {
	t_garray *array = getArray(env, name);  	
	t_sample *vec;
	jsize jarray_length;
  	int size, ifrom = from;

	if ( array == NULL )
		return;
		
  	if ( ! garray_getfloatarray(array, &size, &vec) )
  		return;
  		
	if ( size < from ) {
		error("pdj: array too short from starting point");
		return;
	}
	jarray_length = (*env)->GetArrayLength(env, values);
	
	if ( jarray_length + ifrom > size ) {
		error("pdj: warning: array too short from java array size");
		return;
	} 

	vec += from;	
	(*env)->GetFloatArrayRegion(env, values, 0, jarray_length, vec);
	garray_redraw(array);
}
