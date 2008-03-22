#include <stdlib.h>
#include "pdj.h"
#include "type_handler.h"

int jatom2atom(JNIEnv *env, jobject jatom, t_atom *atom) {
	int type = (*env)->GetIntField(env, jatom, pdjCaching.FIDAtom_type);
	const jbyte *symbolValue;
	jstring value;
	
	switch(type) {
		case DataTypes_INT: 
		case DataTypes_FLOAT:
			atom->a_type = A_FLOAT;
			atom->a_w.w_float = (*env)->GetFloatField(env, jatom, pdjCaching.FIDAtomFloat_value);
			return 0;

		case DataTypes_MESSAGE :
			atom->a_type = A_SYMBOL;
			value = (*env)->GetObjectField(env, jatom, pdjCaching.FIDAtomString_value);
			symbolValue = (*env)->GetStringUTFChars(env, value, NULL);
			atom->a_w.w_symbol = gensym((char *)symbolValue);
			(*env)->ReleaseStringUTFChars(env, value, symbolValue);
			return 0;
			
		default:
			error("pdj: java unhandled datatype: %d ", type);
			atom->a_type = A_CANT;
			return 1;
	}
}


int jatoms2atoms(JNIEnv *env, jobjectArray jatoms, int *nb_atoms, t_atom *atoms) {
	jobject obj;
	int i, rc = 0;
	
	*nb_atoms = (*env)->GetArrayLength(env, jatoms);
	
	if ( *nb_atoms >= MAX_ATOMS_STACK ) {
		error("pdj: array of atoms truncated, original size: %d, new size: %d",
		         *nb_atoms, MAX_ATOMS_STACK);
		*nb_atoms = MAX_ATOMS_STACK;
	}
	
	for(i=0;i<*nb_atoms;i++) {
		obj = (*env)->GetObjectArrayElement(env, jatoms, i);
		rc |= jatom2atom(env, obj, atoms+i);
	}
	
	return rc;
}


jobject atom2jatom(JNIEnv *env, t_atom *atom) {
	jstring arg;
	jobject ret;

	ASSERT(atom);	
	switch(atom->a_type) {
		case A_NULL:
			ret = (*env)->CallStaticObjectMethod(env, pdjCaching.cls_Atom, 
					pdjCaching.MIDAtom_newAtom_String, NULL);
			if ( ret == NULL ) {
				SHOWEXC;
				return NULL;
			}
			return ret;
		
		case A_SYMBOL: 
			arg = (*env)->NewStringUTF(env, atom->a_w.w_symbol->s_name);
			JASSERT(arg);			
			ret = (*env)->CallStaticObjectMethod(env, pdjCaching.cls_Atom, 
					pdjCaching.MIDAtom_newAtom_String, arg);
			if ( ret == NULL ) {
				SHOWEXC;
				return NULL;
			}
			return ret;
			
		case A_FLOAT:
			ret = (*env)->CallStaticObjectMethod(env, pdjCaching.cls_Atom, 
					pdjCaching.MIDAtom_newAtom_Float, atom->a_w.w_float);
			if ( ret == NULL ) {
				SHOWEXC;
				return NULL;
			}
			return ret;
		
		default:
			error("pdj: don't know how to handle atom! type=%d", atom->a_type);
			return NULL;
	}
}


jobjectArray atoms2jatoms(JNIEnv *env, int argc, t_atom *argv) {
	jobjectArray ret;
	int i;

	ret = (*env)->NewObjectArray(env, argc, pdjCaching.cls_Atom, NULL);
	JASSERT(ret);

	for(i=0;i<argc;i++) {
		jobject elem = atom2jatom(env, &(argv[i]));
		
		if ( elem == NULL ) {
			(*env)->DeleteLocalRef(env, ret);
			return NULL;
		}
		
		(*env)->SetObjectArrayElement(env, ret, i, elem);
		if ( (*env)->ExceptionOccurred(env) ) {
			(*env)->ExceptionDescribe(env);
			(*env)->DeleteLocalRef(env, ret);
			return NULL;
		}		
	}
	
	return ret;
}


t_symbol *jstring2symbol(JNIEnv *env, jstring strvalue) {
	const jbyte *tmp = (*env)->GetStringUTFChars(env, strvalue, NULL);
	t_symbol *value;
	
	JASSERT(tmp);
	// TODO : do we have a valid symbol name ???
	value = gensym((char *) tmp);
	(*env)->ReleaseStringUTFChars(env, strvalue, tmp);	
	
	return value;
}


/**
 * Returns the first elements with symbol name 
 */
t_pd *findPDObject(t_symbol *name) {
	void *bindlist_class = gensym("bindlist")->s_thing;
    
    if ( !name->s_thing ) 
    		return NULL;
    if ( *name->s_thing != bindlist_class ) 
    		return name->s_thing;	
    	
    // TODO: work with bindlist to find first object
    error("pdj: duplicate named objects not supported: %s", name->s_name);
    
    return NULL;
}
